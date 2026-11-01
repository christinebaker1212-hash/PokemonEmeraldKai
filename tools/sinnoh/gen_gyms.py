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


def ident(name):
    return name.replace('.nsbmd', '').upper()


def main():
    root = sys.argv[1]
    src = open_source(root)
    by = {p['name']: p for p in places(src, root)}
    out = ['// Generated by tools/sinnoh/gen_gyms.py from pret/pokeplatinum: SINNOH\'s gyms that move (src/sinnoh/s3d_gyms.c)', '']
    out.append('// Platinum\'s model numbers of the props its gyms load at run time (the pack\'s named meshes)')
    for n in gymprops.model_names():
        out.append('#define S3D_MODEL_%s %d' % (ident(n), src._props.index(n)))
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
    open('src/data/sinnoh_gyms.h', 'w').write('\n'.join(out) + '\n')
    print('%d models; PASTORIA: %d deep tiles, %d gates, %d buttons' % (len(gymprops.model_names()), len(deep), len(gates),
                                                                     len(buttons)))
    return buttons


if __name__ == '__main__':
    main()
