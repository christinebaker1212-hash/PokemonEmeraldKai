#!/usr/bin/env python3
"""SINNOH's gyms that move (src/sinnoh/s3d_gyms.c): what they need from Platinum's data, as numbers, written to
src/data/sinnoh_gyms.h. Platinum's code for them is overlay008/gym_features.c; nothing of it is copied, only where
its parts lie: the models it loads at run time (tools/sinnoh/gymprops.py: their numbers), and per gym the tiles that
matter (from the blocks' terrain attributes and props).

    python3 -I tools/sinnoh/gen_gyms.py POKEPLATINUM_DIR      (run from the repository; safe to run again)

PASTORIA: the pool's tiles (behavior 0x59: deep water where the water stands above the floor), its gates (0x56 / 0x57
/ 0x58: open at the water's low / middle / high level) and its buttons (the blue, green and orange button props: the
water to high / middle / low), map tiles.
SUNYSHORE: each room's gears (model, tile, start quarter turn, turning way, upright), its collision regions and which
of them block at each of the four turns (Platinum's tables, read from its gym_features.c), and its buttons (the
rooms' coord events: a quarter turn, back a quarter, or a half turn).
"""
import json
import re
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from source import open_source  # noqa: E402
from gen_maps import places  # noqa: E402
from platinum import Land  # noqa: E402
import gymprops  # noqa: E402


def blocks(src, p):
    """[(map x0, y0 of the block, Land)] of a place"""
    mt = src.matrix(p['matrix'])
    out = []
    for cz in range(p['z0'], p['z1'] + 1):
        for cx in range(p['x0'], p['x1'] + 1):
            out.append(((cx - p['x0']) * 32, (cz - p['z0']) * 32, Land(src.land(mt.cell(cx, cz)[0]))))
    return out


def tiles_with(src, p, behaviors):
    out = []
    for bx, by, land in blocks(src, p):
        for t in range(1024):
            b = land.attrs[t] & 0xFF
            if b in behaviors:
                out.append((bx + t % 32, by + t // 32, b))
    return out


def props_named(src, p, names):
    """[(map x, y, name)] of the place's props of those models (the tile under the prop's point)"""
    out = []
    for bx, by, land in blocks(src, p):
        for pr in land.props:
            n = src._props[pr.model] if pr.model < len(src._props) else ''
            if n in names:
                out.append((bx + int(pr.pos[0] // 16 + 16), by + int(pr.pos[2] // 16 + 16), n))
    return out


def c_struct_list(text, name):
    """The initializers of C array NAME as dicts of .field = value"""
    m = re.search(r'%s\[[^]]*\]\s*=\s*\{(.*?)\n\};' % re.escape(name), text, re.S)
    out = []
    for body in re.findall(r'\{([^{}]*)\}', m.group(1)):
        out.append({k: v.strip() for k, v in re.findall(r'\.(\w+)\s*=\s*([^,]+)', body)})
    return out


def c_list(text, name):
    m = re.search(r'%s\[\]\s*=\s*\{([^}]*)\}' % re.escape(name), text)
    return [int(v) for v in m.group(1).replace(',', ' ').split()]


SUNYSHORE_BUTTONS = {1: {2: 1}, 2: {2: 1, 3: 3}, 3: {2: 1, 3: 2}}   # (room: script entry -> quarter turns: 1, 3 back, 2)


def sunyshore(root):
    text = open(os.path.join(root, 'src/overlay008/gym_features.c'), encoding='utf-8').read()
    lines = ['', '// SUNYSHORE: per room its gears {model, x, y, z (tiles), start quarter, clockwise, upright}, its regions {x,',
             '// z, w, d (tiles)}, the regions that block at each quarter turn (0-3), its buttons {x, z, quarter turns}']
    for room in (1, 2, 3):
        gears = c_struct_list(text, 'sSunyshoreRoom%dGears' % room)
        regions = c_struct_list(text, 'sSunyshoreRoom%dCollisionRegions' % room)
        lines.append('static const struct SunyshoreGear sSunyshoreGears%d[] = {%s};' % (room, ', '.join(
            '{S3D_MODEL_%s, %s, %s, %s, %s, %s, %s}' % (ident(g['modelID'].replace('_nsbmd', '.nsbmd')), g['x'], g['y'],
                                                        g['z'], g['initialRotation'], int(g['turnsClockwise'] == 'TRUE'),
                                                        int(g['isVertical'] == 'TRUE')) for g in gears)))
        lines.append('static const u8 sSunyshoreRegions%d[][4] = {%s};' % (room, ', '.join(
            '{%s, %s, %s, %s}' % (r['x'], r['z'], r['sizeX'], r['sizeZ']) for r in regions)))
        n = len(c_list(text, 'sSunyshoreRoom%dState0Collisions' % room))
        lines.append('static const u8 sSunyshoreBlock%d[4][%d] = {%s};' % (room, n, ', '.join(
            '{%s}' % ', '.join(str(v) for v in c_list(text, 'sSunyshoreRoom%dState%dCollisions' % (room, st))[:n]) for st in range(4))))
        ev = json.load(open(os.path.join(root, 'res/field/events/events_sunyshore_city_gym_room_%d.json' % room)))
        buttons = [(c['x'], c['z'], SUNYSHORE_BUTTONS[room][c['script']]) for c in ev['coord_events']]
        lines.append('static const u8 sSunyshoreButtons%d[][3] = {%s};' % (room, ', '.join('{%d, %d, %d}' % b for b in buttons)))
    return lines


def eterna(root):
    """ETERNA: the clock's five states (0 at the start, one more for each trainer beaten and the leader): its time, its
    13x13 collision from (5, 7), the fountain row's (y 19, x 1-21), the hour hand's jump tile {x, y, north-south}"""
    text = open(os.path.join(root, 'src/overlay008/gym_features.c'), encoding='utf-8').read()
    times = re.findall(r'\.hour\s*=\s*(\d+),\s*\.minute\s*=\s*(\d+)',
                       re.search(r'sEternaGymClockTimes\[[^]]*\]\s*=\s*\{(.*?)\n\};', text, re.S).group(1))
    coll = re.search(r'sEternaGymClockCollision\[[^]]*\]\[[^]]*\]\s*=\s*\{(.*?)\n\};', text, re.S).group(1)
    states = [[int(v) for v in re.findall(r'\d+', b)] for b in re.findall(r'\{([^{}]*)\}', coll)]
    fount = re.search(r'sEternaGymFountainCollision\[[^]]*\]\[[^]]*\]\s*=\s*\{(.*?)\n\};', text, re.S).group(1)
    frows = [[int(v) for v in re.findall(r'\d+', b)] for b in re.findall(r'\{([^{}]*)\}', fount)]
    jumps = re.search(r'sEternaGymHourHandJumpTiles\[[^]]*\]\s*=\s*\{(.*?)\n\};', text, re.S).group(1)
    jl = [(0, 0, 0)]
    for b in re.findall(r'\{([^{}]*)\}', jumps):
        f = dict(re.findall(r'\.(\w+)\s*=\s*(\w+)', b))
        jl.append((int(f['x']), int(f['z']), int(f['isJumpAxisNorthSouth'] == 'TRUE')))
    assert len(times) == 5 and len(states) == 5 and all(len(t) == 169 for t in states) and len(frows) == 5 and len(jl) == 5
    eterna_check(root, states, frows, jl)
    return ['', '// ETERNA: the clock\'s five states: time {hour, minute}, the clock\'s collision rows (13 bits from x 5, y 7-19),',
            '// the fountain row\'s (21 bits from x 1, y 19), the hour hand\'s jump tile {x, y, north-south} (state 0: none)',
            'static const u8 sEternaTimes[5][2] = {%s};' % ', '.join('{%s, %s}' % t for t in times),
            'static const u16 sEternaClock[5][13] = {%s};' % ', '.join('{%s}' % ', '.join(
                '0x%04X' % sum(1 << x for x in range(13) if st[y * 13 + x]) for y in range(13)) for st in states),
            'static const u32 sEternaFountain[5] = {%s};' % ', '.join('0x%06X' % sum(1 << x for x in range(21) if r[x]) for r in frows),
            'static const u8 sEternaJump[5][3] = {%s};' % ', '.join('{%d, %d, %d}' % j for j in jl)]


def eterna_check(root, states, frows, jumps):
    """Walked as s3d_gyms.c walks it, each state of the clock reaches the next of Platinum's trainers (CAROLINE,
    JENNA, ANGELA, then GARDENIA) from the door: stops the generator if not"""
    from collections import deque
    src = open_source(root)
    p = [q for q in places(src, root) if q['name'] == 'Sinnoh_EternaCityGym'][0]
    (bx, by, land), = blocks(src, p)
    def wall(st, x, y):
        if not (0 <= x < 32 and 0 <= y < 32):
            return True
        if 5 <= x < 18 and 7 <= y < 20:
            return bool(states[st][(y - 7) * 13 + x - 5])
        if y == 19 and 1 <= x <= 21:
            return bool(frows[st][x - 1])
        return bool(land.attrs[y * 32 + x] & 0x8000)
    order = [(14, 22), (20, 17), (2, 7), (11, 3)]
    for st, (tx, ty) in enumerate(order):
        seen, todo = {(11, 26)}, deque([(11, 26)])   # (inside the door, 11, 27)
        while todo:
            x, y = todo.popleft()
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                j = jumps[st]
                if st and (nx, ny) == (j[0], j[1]) and (dy != 0) == bool(j[2]):
                    nx, ny = nx + dx, ny + dy   # (over the hour hand)
                elif wall(st, nx, ny):
                    continue
                if (nx, ny) not in seen and not wall(st, nx, ny):
                    seen.add((nx, ny))
                    todo.append((nx, ny))
        assert any((tx + dx, ty + dy) in seen for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))), \
            'ETERNA: state %d doesn\'t reach %d,%d' % (st, tx, ty)


def canalave(root):
    """CANALAVE: its 24 platforms {model, A x y z, B x y z (tiles), floor A, floor B, starts at B}, each floor's
    collision (Platinum's tables: 1 a wall), each person's floor (the events' heights: 10 tiles a floor)"""
    text = open(os.path.join(root, 'src/overlay008/gym_features.c'), encoding='utf-8').read()
    m = re.search(r'sCanalavePlatformPaths\[[^]]*\]\s*=\s*\{(.*?)\n\};', text, re.S)
    plats = []
    for body in re.findall(r'\{\s*\.modelID(.*?)\n    \}', m.group(1), re.S):
        f = dict(re.findall(r'\.(\w+)\s*=\s*([^,\n]+)', '.modelID' + body))
        a = dict(re.findall(r'\.(\w)\s*=\s*(\d+)', re.search(r'positionA\s*=\s*\{([^}]*)\}', body).group(1)))
        b = dict(re.findall(r'\.(\w)\s*=\s*(\d+)', re.search(r'positionB\s*=\s*\{([^}]*)\}', body).group(1)))
        plats.append((ident(f['modelID'].strip().replace('_nsbmd', '.nsbmd')), a['x'], a['y'], a['z'], b['x'], b['y'], b['z'],
                      f['floorA'].strip(), f['floorB'].strip(), int(f['startInPositionB'].strip() == 'TRUE')))
    cm = re.search(r'sCanalaveGymCollisionMaps\[4\]\[[^]]*\]\s*=\s*\{(.*?)\n\};', text, re.S).group(1)
    floors = [[int(v) for v in re.findall(r'\d+', blk)] for blk in re.findall(r'\{([^{}]*)\}', cm)]
    assert len(floors) == 4 and all(len(f) == 1024 for f in floors), [len(f) for f in floors]
    ev = json.load(open(os.path.join(root, 'res/field/events/events_canalave_city_gym.json')))
    people = [o.get('y', 0) // 10 for o in ev['object_events']]
    lines = ['', '// CANALAVE: its platforms {model, A x y z, B x y z (tiles), floor A, floor B, starts at B}, each floor\'s',
             '// collision as bits (a row a u32, bit x: a wall), each person\'s floor (Platinum\'s object order)']
    lines.append('static const struct CanalavePlatform sCanalavePlatforms[] = {%s};' % ', '.join(
        '{S3D_MODEL_%s, %s, %s, %s, %s, %s, %s, %s, %s, %d}' % p for p in plats))
    lines.append('static const u32 sCanalaveWalls[4][32] = {%s};' % ', '.join(
        '{%s}' % ', '.join('0x%08X' % sum(1 << x for x in range(32) if f[y * 32 + x]) for y in range(32)) for f in floors))
    lines.append('static const u8 sCanalaveFloorOf[] = {%s};' % ', '.join(str(v) for v in people))
    canalave_check(plats, floors, ev['object_events'])
    return lines


def canalave_check(plats, floors, objects):
    """The gym as s3d_gyms.c walks it (each floor's walls; a platform stepped on carries the player to its other end)
    reaches everyone from the door: stops the generator if not"""
    from collections import deque
    pl = [(int(a), int(b), int(c), int(d), int(e), int(f)) for _, a, b, c, d, e, f, _, _, _ in plats]
    start = (16, 26, 0, 0)
    seen, todo = {start}, deque([start])
    while todo:
        x, y, fl, moved = todo.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if not (0 <= nx < 32 and 0 <= ny < 32) or floors[fl][ny * 32 + nx]:
                continue
            st = (nx, ny, fl, moved)
            for i, p in enumerate(pl):
                at_b = plats[i][9] ^ ((moved >> i) & 1)
                px, py, pz = p[3:6] if at_b else p[0:3]
                if (px, pz) == (nx, ny) and int(py) // 10 == fl:
                    ox, oy, oz = p[0:3] if at_b else p[3:6]
                    st = (ox, oz, int(oy) // 10, moved ^ (1 << i))
                    break
            if st not in seen:
                seen.add(st)
                todo.append(st)
    where = {(s[0], s[1], s[2]) for s in seen}
    for o in objects:
        f = o.get('y', 0) // 10
        assert any((o['x'] + dx, o['z'] + dy, f) in where for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))), \
            'CANALAVE: %s at %d,%d (floor %d) out of reach' % (o['graphics_id'], o['x'], o['z'], f)


VEIL_DIRS = ((0, -1), (0, 1), (-1, 0), (1, 0))   # (Platinum's order: north, south, west, east)


def veilstone(root):
    """VEILSTONE: its punching bags and tire stacks (Platinum's tables, as tiles: Platinum places each object two tiles
    north of the table's spot), and how a bag may move on each tile (Platinum's table: 0 free, 1 not there, 2 not
    north-south, 3 not west-east, 4 stops there, 5 not west)"""
    text = open(os.path.join(root, 'src/overlay008/gym_features.c'), encoding='utf-8').read()
    def coords(name):
        m = re.search(r'%s\[[^]]*\]\s*=\s*\{(.*?)\n\};' % name, text, re.S).group(1)
        return [(int(x), int(z) - 2) for x, z in re.findall(r'\.x\s*=\s*(\d+),\s*\.z\s*=\s*(\d+)', m)]
    bags, tires = coords('sPunchingBagPositions'), coords('sTireStackPositions')
    m = re.search(r'sVeilstoneTileBehaviors\[[^]]*\]\s*=\s*\{(.*?)\n\};', text, re.S).group(1)
    track = [int(v) for v in re.findall(r'\d+', m.split('\n', 1)[1] if m.startswith('\n') else m)]
    assert len(bags) == 9 and len(tires) == 11 and len(track) == 1024, (len(bags), len(tires), len(track))
    veilstone_check(root, bags, tires, track)
    return ['', '// VEILSTONE: its punching bags and tire stacks {x, y}, how a bag may move on each tile (a digit a tile:',
            '// 0 free, 1 not there, 2 not north-south, 3 not west-east, 4 stops there, 5 not west)',
            'static const u8 sVeilstoneBags[%d][2] = {%s};' % (len(bags), ', '.join('{%d, %d}' % b for b in bags)),
            'static const u8 sVeilstoneTires[%d][2] = {%s};' % (len(tires), ', '.join('{%d, %d}' % t for t in tires)),
            'static const char sVeilstoneTrack[32][33] = {%s};' % ',\n    '.join(
                '"%s"' % ''.join(str(v) for v in track[y * 32:y * 32 + 32]) for y in range(32))]


def veil_flags(track, tires, x, y, d):
    """Platinum's flags for a bag on X, Y going D: 1 can't, 2 a tire stack ahead, 4 stops here"""
    dx, dy = VEIL_DIRS[d]
    b, f = track[y * 32 + x], 0
    if b == 4:
        f |= 4
    elif (b == 2 and d < 2) or (b == 3 and d >= 2) or (b == 5 and d == 2):
        f |= 1
    if track[(y + dy) * 32 + x + dx] == 1:
        f |= 1
    if (x + dx, y + dy) in tires:
        f |= 2
    return f


def veil_push(track, tires, x, y, d):
    """Where a bag hit on X, Y going D stops, and the tire stack it knocks over (or None)"""
    dx, dy = VEIL_DIRS[d]
    f = veil_flags(track, tires, x, y, d)
    if f not in (0, 4):
        return x, y, None
    while True:
        x, y = x + dx, y + dy
        f = veil_flags(track, tires, x, y, d)
        if f:
            return x, y, ((x + dx, y + dy) if f & 2 else None)


def veilstone_check(root, bags, tires, track):
    """Pushing bags as s3d_gyms.c does reaches each of Platinum's trainers and MAYLENE from the door: stops the
    generator if not"""
    from collections import deque
    src = open_source(root)
    p = [q for q in places(src, root) if q['name'] == 'Sinnoh_VeilstoneCityGym'][0]
    (bx, by, land), = blocks(src, p)
    ev = json.load(open(os.path.join(root, 'res/field/events/events_veilstone_city_gym.json')))
    people = [(o['x'], o['z']) for o in ev['object_events']]
    door = [(w['x'], w['z']) for w in ev['warp_events']][0]
    start = (door[0], door[1] - 1)
    def reach(bs, ts):
        walls = set(bs) | set(ts) | set(people)
        seen, todo = {start}, deque([start])
        while todo:
            x, y = todo.popleft()
            for dx, dy in VEIL_DIRS:
                n = (x + dx, y + dy)
                if 0 <= n[0] < 32 and 0 <= n[1] < 32 and n not in seen and n not in walls \
                        and not land.attrs[n[1] * 32 + n[0]] & 0x8000:
                    seen.add(n)
                    todo.append(n)
        return seen
    first = (tuple(bags), frozenset(tires))
    states, todo, met = {first}, deque([first]), set()
    while todo:
        bs, ts = todo.popleft()
        here = reach(bs, ts)
        met |= {q for q in people if any((q[0] + dx, q[1] + dy) in here for dx, dy in VEIL_DIRS)}
        for i, (x, y) in enumerate(bs):
            for d, (dx, dy) in enumerate(VEIL_DIRS):
                if (x - dx, y - dy) not in here:
                    continue
                nx, ny, tire = veil_push(track, ts, x, y, d)
                if (nx, ny) == (x, y):
                    continue
                st = (bs[:i] + ((nx, ny),) + bs[i + 1:], ts - {tire} if tire else ts)
                if st not in states:
                    states.add(st)
                    todo.append(st)
    out = [q for q in people if q not in met]
    assert not out, 'VEILSTONE: out of reach %s (%d states)' % (out, len(states))
    print('VEILSTONE: everyone reached (%d states of the bags)' % len(states))


def snowpoint(root):
    """SNOWPOINT: its snowballs (Platinum's objects of that look: hidden by gen_maps, drawn and broken here) and its
    fences (behavior 0x49: no step north or south into, out of or across it; 0x4A: none west or east)"""
    src = open_source(root)
    p = [q for q in places(src, root) if q['name'] == 'Sinnoh_SnowpointCityGym'][0]
    ev = json.load(open(os.path.join(root, 'res/field/events/events_snowpoint_city_gym.json')))
    balls = [(o['x'], o['z']) for o in ev['object_events'] if o['graphics_id'] == 'OBJ_EVENT_GFX_SNOWBALL']
    fences = [(x, y, b - 0x49) for x, y, b in tiles_with(src, p, {0x49, 0x4A})]
    assert balls and fences
    slopes = snow_slopes(src, p)
    snowpoint_check(root, src, p, ev, balls, fences, slopes)
    return ['', '// SNOWPOINT: its ice\'s slopes as Platinum sees them half a step ahead (a tile two hex digits: 2 bits a way,',
            '// north, south, west, east from the low bits; 1 up, 2 down)',
            'static const char sSnowpointSlope[32][65] = {%s};' % ',\n    '.join(
                '"%s"' % ''.join('%02X' % slopes[y * 32 + x] for x in range(32)) for y in range(32)),
            '// its snowballs {x, y}, its fences {x, y, 0 no step north-south / 1 no step west-east}',
            'static const u8 sSnowpointBalls[%d][2] = {%s};' % (len(balls), ', '.join('{%d, %d}' % b for b in balls)),
            'static const u8 sSnowpointFences[%d][3] = {%s};' % (len(fences), ', '.join('{%d, %d, %d}' % f for f in fences))]


def snow_slopes(src, p):
    """Per tile, going each way (Platinum's order) whether the floor half a step ahead (4 units, Platinum's look) is
    higher (1) or lower (2) than at the centre: 2 bits a way"""
    from build_pack import heights
    (bx, by, land), = blocks(src, p)
    here = heights(land)
    ahead = [heights(land, dx * 4, dy * 4) for dx, dy in VEIL_DIRS]
    out = []
    for t in range(1024):
        v = 0
        if land.attrs[t] & 0xFF == 0x20 and here[t]:
            for d in range(4):
                if ahead[d][t]:
                    a, h = ahead[d][t][0], here[t][0]
                    v |= (1 if a > h + 0.5 else 2 if a < h - 0.5 else 0) << (2 * d)
        out.append(v)
    return out


def snow_slide(walls, ice, slopes, fences, balls, x, y, d):
    """A slide from X, Y going D (just stepped onto ice), as field_player_avatar.c SnowpointSlip slides: where it stops
    and the snowballs it breaks"""
    dx, dy = VEIL_DIRS[d]
    s, broke, steps = 0, set(), 0

    def blocked(fx, fy, tx, ty):
        a, b = fences.get((fx, fy)), fences.get((tx, ty))
        ns = fx == tx
        return (tx, ty) in walls or ((tx, ty) in balls and (tx, ty) not in broke) \
            or ((a == 0 or b == 0) and ns) or ((a == 1 or b == 1) and not ns)

    while (x, y) in ice and steps < 200:
        steps += 1
        v = (slopes[y * 32 + x] >> (2 * VEIL_DIRS.index((dx, dy)))) & 3
        sl = 1 if v == 1 else -1 if v == 2 else 0
        if s >= 1 and steps > 1 and (x + dx, y + dy) in balls:
            broke.add((x + dx, y + dy))
        back = False
        if blocked(x, y, x + dx, y + dy):
            if sl <= 0:
                break
            back = True
        elif sl > 0:
            if s == 0:
                back = True
            else:
                s -= 1
        elif sl < 0:
            s = min(3, s + 1)
        if back:
            dx, dy, s = -dx, -dy, 0
            if blocked(x, y, x + dx, y + dy):
                break
        x, y = x + dx, y + dy
    return x, y, broke


def snowpoint_check(root, src, p, ev, balls, fences, slopes):
    """Walking and sliding as the game does (snow_slide) reaches each of Platinum's trainers and CANDICE from the door:
    stops the generator if not; prints the way to CANDICE"""
    from collections import deque
    (bx, by, land), = blocks(src, p)
    people = [(o['x'], o['z']) for o in ev['object_events'] if o['graphics_id'] != 'OBJ_EVENT_GFX_SNOWBALL']
    walls = {(x, y) for x in range(32) for y in range(32) if land.attrs[y * 32 + x] & 0x8000} | set(people)
    walls |= {(x, y) for x in range(-1, 33) for y in (-1, 32)} | {(x, y) for x in (-1, 32) for y in range(-1, 33)}
    ice = {(x, y) for x in range(32) for y in range(32) if land.attrs[y * 32 + x] & 0xFF == 0x20}
    fence = {(x, y): k for x, y, k in fences}
    door = [(w['x'], w['z']) for w in ev['warp_events']][0]
    start = ((door[0], door[1] - 1), frozenset())
    seen, todo, met = {start: None}, deque([start]), set()
    while todo:
        (x, y), broken = st = todo.popleft()
        live = set(balls) - broken
        met |= {q for q in people if abs(q[0] - x) + abs(q[1] - y) == 1}
        for d, (dx, dy) in enumerate(VEIL_DIRS):
            a, b = fence.get((x, y)), fence.get((x + dx, y + dy))
            if (x + dx, y + dy) in walls or (x + dx, y + dy) in live or ((a == 0 or b == 0) and dx == 0) \
                    or ((a == 1 or b == 1) and dx != 0):
                continue
            nx, ny, broke = snow_slide(walls, ice, slopes, fence, live, x + dx, y + dy, d)
            nst = ((nx, ny), broken | frozenset(broke))
            if nst not in seen:
                seen[nst] = (st, 'NSWE'[d])
                todo.append(nst)
    out = [q for q in people if q not in met]
    assert not out, 'SNOWPOINT: out of reach %s (%d states)' % (out, len(seen))
    goal = people[0]
    st = min((s for s in seen if abs(s[0][0] - goal[0]) + abs(s[0][1] - goal[1]) == 1), key=lambda s: len(s[1]))
    way = []
    while seen[st]:
        st, m = seen[st]
        way.append(m)
    print('SNOWPOINT: everyone reached (%d states); to CANDICE: %s' % (len(seen), ''.join(reversed(way))))


def hearthome(root):
    """HEARTHOME: its two trainer rooms (Platinum's layouts: the area its clue may lie in, its first door's symbol
    and how many doors), where the clue may lie (Platinum's tables: '0' it may), and each room's doors {x, y, symbol}"""
    text = open(os.path.join(root, 'src/overlay008/gym_features.c'), encoding='utf-8').read()
    syms = {'CIRCLE': 0, 'SQUARE': 1, 'TRIANGLE': 2, 'SUN': 3, 'DONUT': 4, 'MOOM': 5, 'STAR': 6, 'HEART': 7}
    out = ['', '// HEARTHOME: its trainer rooms {area x, y, w, h where the clue may lie, first door\'s symbol, doors}, where in',
           '// each area the clue may lie (\'0\'), each room\'s doors {x, y, symbol: circle, square, triangle, sun, donut, moon,',
           '// star, heart}']
    rooms = []
    for n, (w, h) in ((1, (15, 8)), (2, (27, 20))):
        m = re.search(r'sTrainerRoom%dCluePositions\[[^]]*\]\s*=\s*\{(.*?)\n\};' % n, text, re.S).group(1)
        vals = [int(v) for v in re.findall(r'\d+', m)]
        assert len(vals) == w * h
        out.append('static const char sHearthClue%d[%d][%d] = {%s};' % (n, h, w + 1, ', '.join(
            '"%s"' % ''.join(str(v) for v in vals[y * w:y * w + w]) for y in range(h))))
        d = re.search(r'sTrainerRoom%dDoors\[\]\s*=\s*\{(.*?)\n\};' % n, text, re.S).group(1)
        doors = [(int(x), int(z), syms[i]) for i, x, z in re.findall(
            r'HEARTHOME_DOOR_ID_(\w+), \.x = (\d+), \.z = (\d+)', d) if i != 'EXIT']
        rooms.append((1, 3, w, h, doors[0][2], len(doors)))
        out.append('static const u8 sHearthDoors%d[%d][3] = {%s};' % (n, len(doors), ', '.join('{%d, %d, %d}' % t for t in doors)))
    out.append('static const u8 sHearthRooms[2][6] = {%s};' % ', '.join('{%d, %d, %d, %d, %d, %d}' % r for r in rooms))
    return out


def villa(root):
    """The VILLA's furniture (Platinum's src/overlay005/villa_furniture.c): each slot's furniture, where its model stands
    (16 units a tile, from the map's corner), its collision box (tiles) and the script its tiles run; the furniture's
    flags (FLAG_VILLA_FURNITURE_*, consecutive) start at the game's flag for the TABLE"""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import ids
    path = os.path.join(root, 'src/overlay005/villa_furniture.c')
    if not os.path.exists(path):
        raise SystemExit('%s: fetch_decomp.sh PARTS 19' % path)
    text = open(path).read()
    types = [l.strip() for l in open(os.path.join(root, 'generated/villa_furniture_type.txt')) if l.strip()]
    types = [t for t in types if t != 'VILLA_FURNITURE_MAX']
    coord = lambda macro, n: int(n) * 16 + (8 if 'CENTER' in macro else 0)
    slots = []
    for b in re.findall(r'\{\s*\.furnitureType = (\w+),\s*\.isCollisionEnabled = (\w+),\s*\.position = \{\s*'
                        r'\.x = (\w+)\((\d+)\),\s*\.y = 0,\s*\.z = (\w+)\((\d+)\),\s*\},\s*'
                        r'\.collisionBounds = (COLLISION_BOUNDS_NONE|\{[^}]*\}),', text):
        bounds = [0, 0, 0, 0] if b[6] == 'COLLISION_BOUNDS_NONE' else [int(v) for v in re.findall(r'= (\d+)', b[6])]
        slots.append((types.index(b[0]), 1 if b[1] == 'TRUE' else 0, coord(b[2], b[3]), coord(b[4], b[5])) + tuple(bounds))
    assert len(slots) == 23, len(slots)
    scripts = dict(re.findall(r'\[(VILLA_FURNITURE_\w+)\] = (0x[0-9A-Fa-f]+|SCRIPT_ID_OFFSET_TV_BROADCAST)', text.split(
        'sVillaFurnitureScriptIDs')[-1]))
    entry = [0 if scripts[t] == 'SCRIPT_ID_OFFSET_TV_BROADCAST' else int(scripts[t], 16) for t in types]
    models = ['S3D_MODEL_%s' % t.replace('VILLA_FURNITURE_', 'VILLA_FURNITURE_') for t in types]
    out = ['', '// the VILLA (s3d_villa.c): its furniture flags from FLAG_VILLA_FURNITURE_TABLE on, each furniture\'s model and',
           '// its tiles\' script (Platinum\'s scripts_villa entry; 0: the TV, SINNOH\'s TV broadcast); each slot: furniture,',
           '// collision, x and z of its model (map units), its box left, top, right, bottom (tiles)']
    out.append('#define SINNOH_VILLA_FLAG_FIRST 0x%X' % ids.flag(root, 'FLAG_VILLA_FURNITURE_TABLE'))
    out.append('static const u16 sVillaModels[%d] = {%s};' % (len(types), ', '.join(models)))
    out.append('static const u8 sVillaScripts[%d] = {%s};' % (len(types), ', '.join(str(e) for e in entry)))
    out.append('static const u16 sVillaSlots[%d][8] = {%s};' % (len(slots), ', '.join(
        '{%s}' % ', '.join(str(v) for v in sl) for sl in slots)))
    return out


def ident(name):
    return name.replace('fx:', '').replace('.nsbmd', '').upper()


def main():
    root = sys.argv[1]
    src = open_source(root)
    by = {p['name']: p for p in places(src, root)}
    out = ['// Generated by tools/sinnoh/gen_gyms.py from pret/pokeplatinum: SINNOH\'s gyms that move (src/sinnoh/s3d_gyms.c)', '']
    out.append('// Platinum\'s model numbers of the props its gyms load at run time (the pack\'s named meshes)')
    for n in gymprops.model_names():
        out.append('#define S3D_MODEL_%s %d' % (ident(n), gymprops.number(src, n)))
    out.append('')
    p = by['Sinnoh_PastoriaCityGym']
    deep = [(x, y) for x, y, b in tiles_with(src, p, {0x59})]
    gates = [(x, y, {0x56: 0, 0x57: 1, 0x58: 2}[b]) for x, y, b in tiles_with(src, p, {0x56, 0x57, 0x58})]
    color = {'pastoria_gym_orange_button.nsbmd': 0, 'pastoria_gym_green_button.nsbmd': 1, 'pastoria_gym_blue_button.nsbmd': 2}
    buttons = [(x, y, color[n]) for x, y, n in props_named(src, p, set(color))]
    out.append('// PASTORIA: the deep water\'s tiles, the gates (open at water level 0 low, 1 middle, 2 high), the buttons')
    out.append('// (the water to that level)')
    out.append('static const u8 sPastoriaDeep[][2] = {%s};' % ', '.join('{%d, %d}' % t for t in deep))
    out.append('static const u8 sPastoriaGates[][3] = {%s};' % ', '.join('{%d, %d, %d}' % t for t in gates))
    out.append('static const u8 sPastoriaButtons[][3] = {%s};' % ', '.join('{%d, %d, %d}' % t for t in sorted(buttons)))
    out += sunyshore(root)
    out += canalave(root)
    out += eterna(root)
    out += veilstone(root)
    out += snowpoint(root)
    out += villa(root)
    out += hearthome(root)
    open('src/data/sinnoh_gyms.h', 'w').write('\n'.join(out) + '\n')
    print('%d models; PASTORIA: %d deep tiles, %d gates, %d buttons' % (len(gymprops.model_names()), len(deep), len(gates),
                                                                     len(buttons)))
    return buttons


if __name__ == '__main__':
    main()
