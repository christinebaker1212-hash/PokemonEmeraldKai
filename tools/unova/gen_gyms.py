#!/usr/bin/env python3
"""The gyms' gimmicks that follow Black's rails (src/unova/u3d_gyms.h, read by src/unova/u3d_vm.c):

- BRYCEN's gym (ICIRRUS, zone 114, Black's gimmick ov045 field_gimmick_switch_toggle.c): an ice rink whose curves,
  ramps and way down are rails (Black's hybrid field: FIELDMAP_CTRL_HYBRID). Their own attributes (Black's rail
  attribute file, the rail's file + half the archive: RAIL_ATTR_*) make them forced rides: ice and forced moves from a
  jump-start tile (attribute 0xA3, "the blue tile") to the one at the far end, where the player takes their way again.
  - the curves start where Black checks the player's facing (a pos event of check type 1-4: ov010
    EVENTDATA_SYSTEM_DirToPosEventType, up / down / left / right); their scripts hand the player to the rail when the
    switch's requirement holds (FIELD_GIMMICK_SWITCH_TOGGLE_ReplacePlayerIfRequirementMet: ov045's table
    sSwitchToggleRequirementSwitchIndex / State, read from the overlay), else the player skates straight on
  - the ramps over the ravines and the slide down from BRYCEN start on the grid's own jump-start tiles, walked into
    (FIELDMAP_CTRL_HYBRID_UpdateGrid: on a jump start, the way ahead blocked: onto the rail); the ramps both ways, in
    each of their three lanes
  Every tile of these rails is left out of the walkable grid (tools/unova/build_pack.py lay_rails), so the switches
  decide the way as Black's do.

- DRAYDEN's gym (OPELUCID, zone 121, Black's gimmick ov046 field_gimmick_rotating_room.c): three dragon statues, each
  turned by two levers to one of four poses (FIELD_GIMMICK_ROTATING_ROOM_CalcUnitPose); a pose opens a pair of the
  dragon's rail lines (sRotatingRoomPoseLines, read from the overlay: the first two of a row, none in pose 1) and
  closes the others (ApplyPoseLines). Here each pair's tiles that aren't floor otherwise are walkable only while the
  pose has them open (u3d.c U3D_StepBlocked, u3d_vm.c U3D_Vm_DragonBlocked)

    python3 -I tools/unova/gen_gyms.py       (run from the repository; safe to run again)
"""
import math
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402
import rails  # noqa: E402
import events as ev  # noqa: E402
import scrcmd  # noqa: E402
import scripts as sc  # noqa: E402
import terrain  # noqa: E402
import matrix as mxr  # noqa: E402
import zones as zr  # noqa: E402

ICE_ZONE = 114
REQ_AT, REQ_COUNT = 0x021F3B60, 8          # (ov045: u16 switch, u16 state, a requirement)
NO_REQUIREMENT = 255
JUMP_START = 0xA3
CELL = 16.0
DIRS = {(0, 1): 'DIR_SOUTH', (0, -1): 'DIR_NORTH', (-1, 0): 'DIR_WEST', (1, 0): 'DIR_EAST'}
CHECK_DIR = {1: 'DIR_NORTH', 2: 'DIR_SOUTH', 3: 'DIR_WEST', 4: 'DIR_EAST'}


def requirements():
    ram, d = scrcmd.ov_image(45)
    return [struct.unpack_from('<HH', d, REQ_AT - ram + 4 * i) for i in range(REQ_COUNT)]


def requirement_of(zone, script_id):
    """the requirement a curve's script asks of the switches (its ReplacePlayerIfRequirementMet's argument)"""
    f, n, _, _ = sc.script_file(zone, script_id)
    cmds, _, _ = sc.decode(f, [sc.entries(f)[n]])
    for _op, name, args, _next in cmds.values():
        if name.endswith('ReplacePlayerIfRequirementMet'):
            return args[0]
    raise ValueError('script %d hands no player to a rail' % script_id)


def rail_attrs(li, rail_zone=ICE_ZONE):
    """the attributes along line LI's middle lane"""
    a = narc.arc(rails.RAIL_ARC)
    d = a[rails.rail_files()[rail_zone] + len(a) // 2]
    o = 4
    for i in range(struct.unpack_from('<I', d, 0)[0]):
        w, n = struct.unpack_from('<HH', d, o)
        vals = struct.unpack_from('<%dI' % (w * n), d, o + 4)
        if i == li:
            return [vals[w // 2 + w * y] & 0xFFFF for y in range(n)]
        o += 4 + 4 * w * n
    return []


def _tile(p):
    return int(math.floor(p[0] / CELL)), int(math.floor(p[2] / CELL))


def _samples(r, li, side=0):
    n = max(1, r['lines'][li]['length'])
    return [rails.place(r, li, k / 8.0, side) for k in range(n * 8 + 1)]


def chain_forward(r, li):
    out = [li]
    while True:
        end = r['lines'][out[-1]]['end']
        nxt = [l for l in r['points'][end]['lines'] if l < len(r['lines']) and l not in out
               and r['lines'][l]['start'] == end]
        if not nxt:
            return out
        out.append(nxt[0])


def whole_chain(r, li):
    """the lines LI is among, from the first to the last"""
    first = li
    while True:
        start = r['lines'][first]['start']
        prv = [l for l in r['points'][start]['lines'] if l < len(r['lines']) and l != first
               and r['lines'][l]['end'] == start]
        if not prv:
            return chain_forward(r, first)
        first = prv[0]


def _nearest(r, tx, tz):
    best = None
    for li in range(len(r['lines'])):
        for k, p in enumerate(_samples(r, li)):
            d = math.hypot(p[0] / CELL - tx - 0.5, p[2] / CELL - tz - 0.5)
            if best is None or d < best[0]:
                best = (d, li, k)
    return best[1], best[2]


def jump_tile(r, li, which, side=0):
    """the tile of line LI's first (WHICH 0) or last (-1) jump start, in lane SIDE"""
    ks = [k for k, a in enumerate(rail_attrs(li)) if a == JUMP_START]
    return _tile(rails.place(r, li, ks[which] + 0.25, side)) if ks else None


def lane_path(r, lines, forward, start, side, stop):
    """(the tiles from START along LINES, each run forward or back, in lane SIDE, to STOP; their heights)"""
    tiles, heights, begun = [start], {}, False
    for li in lines:
        ss = _samples(r, li, side)
        for p in (ss if forward else ss[::-1]):
            c = _tile(p)
            if not begun:
                begun = c == start   # (the samples before the start: skipped till the path reaches it)
                continue
            if c == tiles[-1]:
                continue
            px, pz = tiles[-1]
            if abs(c[0] - px) == 1 and abs(c[1] - pz) == 1:   # (a slant: through the tile beside, 4-connected)
                mid = (c[0], pz)
                heights.setdefault(mid, p[1])
                tiles.append(mid)
            heights.setdefault(c, p[1])
            tiles.append(c)
            if c == stop:
                return tiles, heights
    raise ValueError('no way from %s to %s' % (start, stop))


def rides():
    """[(script, requirement, x, z, facing, steps, tiles, heights)] of every ride of BRYCEN's gym"""
    r = rails.load(rails.rail_files()[ICE_ZONE])
    out = []

    def add(script, req, path, heights):
        steps = [DIRS[(b[0] - a[0], b[1] - a[1])] for a, b in zip(path, path[1:])]
        out.append((script, req, path[0][0], path[0][1], steps[0], steps, path, heights))

    used = set()
    for q in ev.events(ICE_ZONE)['pos']:     # the curves: a facing check and a switch's requirement
        if not 1 <= q['check'] <= 4:
            continue
        li, _ = _nearest(r, q['x'], q['z'])
        lines = chain_forward(r, li)
        used.update(lines)
        path, hs = lane_path(r, lines, True, (q['x'], q['z']), 0, jump_tile(r, lines[-1], -1))
        assert DIRS[(path[1][0] - path[0][0], path[1][1] - path[0][1])] == CHECK_DIR[q['check']], q
        add(q['script'], requirement_of(ICE_ZONE, q['script']), path, hs)
    for li in range(len(r['lines'])):        # the ramps and the way down: from the grid's jump starts
        if li in used:
            continue
        lines = whole_chain(r, li)
        used.update(lines)
        top = r['points'][r['lines'][lines[0]]['start']]['pos'][1]
        bottom = r['points'][r['lines'][lines[-1]]['end']]['pos'][1]
        for side in (-1, 0, 1):
            sa, sb = jump_tile(r, lines[0], 0, side), jump_tile(r, lines[-1], -1, side)
            path, hs = lane_path(r, lines, True, sa, side, sb)
            add(0, NO_REQUIREMENT, path, hs)
            if abs(top - bottom) < CELL:   # (a ramp over a ravine: both ways; the slide down from BRYCEN only down)
                path, hs = lane_path(r, lines[::-1], False, sb, side, sa)
                add(0, NO_REQUIREMENT, path, hs)
    return out


def gated_tiles():
    """{(tile x, z): height} of BRYCEN's rails (build_pack.py leaves them out of the grid, keeping their heights)"""
    out = {}
    rows = rides()
    for *_x, path, hs in rows:
        for t in path[1:]:
            out[t] = hs.get(t, 0.0)
    for t, hl in rails.cells(rails.rail_files()[ICE_ZONE]).items():   # (their whole width)
        out.setdefault(t, sorted(hl)[0])
    for *_x, path, hs in rows:   # (the jump starts stay the grid's own)
        out.pop(path[0], None)
        out.pop(path[-1], None)
    return out


ROTATE_ANIMS = 0x021F41C4   # (ov046 sRotatingRoomRotateAnimTable)
DRAGON_ZONE = 121
POSE_LINES_AT = 0x021F4430                 # (ov046: int [3 units][4 poses][6 lines])


def pose_lines():
    ram, d = scrcmd.ov_image(46)
    t = [struct.unpack_from('<6i', d, POSE_LINES_AT - ram + 24 * i) for i in range(12)]
    return [[t[u * 4 + p] for p in range(4)] for u in range(3)]


def grid_walkable(zone):
    """the tiles Black's own grid lets the player walk (the zone's matrix, its first layer)"""
    z = zr.zones()[zone]
    mx = mxr.matrix(z['matrix'], zone)
    out = set()
    for by in range(mx['h']):
        for bx in range(mx['w']):
            bi = mx['blocks'][by * mx['w'] + bx]
            if bi == mxr.NONE or mx['zones'][by * mx['w'] + bx] != zone:
                continue
            layers = terrain.layers(narc.arc('a/0/0/8')[bi])
            if not layers:
                continue
            gr = terrain.grid(layers[0])
            for k in range(1024):
                if not (gr[k][0] >> 16) & 1:
                    out.add((bx * 32 + k % 32, by * 32 + k // 32))
    return out


def dragon_tiles():
    """[unit][pose]: the tiles only that pose's pair of lines makes walkable"""
    rail = rails.rail_files()[DRAGON_ZONE]
    table = pose_lines()
    gated = {l for unit in table for row in unit for l in row}
    free = grid_walkable(DRAGON_ZONE) | set(rails.cells(rail, set(range(len(rails.load(rail)['lines']))) - gated))
    out = []
    for unit in table:
        out.append([sorted(set(rails.cells(rail, set(row[:2]))) - free) if p != 1 else [] for p, row in enumerate(unit)])
    return out


def main():
    reqs = requirements()
    rows = rides()
    path = 'src/unova/u3d_gyms.h'
    with open(path, 'w') as f:
        f.write('// Generated by tools/unova/gen_gyms.py. Do not edit.\n')
        f.write("// BRYCEN's gym's rides (Black's rails, gimmick ov045): from a jump start (the blue tile) facing the rail,\n"
                "// to the one at its far end; a curve's own script (0: none) and its switch requirement\n")
        f.write('#define ICE_GYM_ZONE %d\n' % ICE_ZONE)
        f.write("// ov045 sSwitchToggleRequirement*: a requirement holds when switch S is on (1) or off (0)\n")
        f.write('static const u8 sIceRequirement[%d][2] = {%s};\n'
                % (len(reqs), ', '.join('{%d, %d}' % rq for rq in reqs)))
        f.write('#define ICE_RIDE_STEPS %d\n' % max(len(r_[5]) for r_ in rows))
        f.write('#define ICE_NO_REQUIREMENT %d   // (script 0: the ramps and the way down, always open)\n'
                % NO_REQUIREMENT)
        f.write('static const struct { u16 script; u8 req, x, z, facing, count; u8 steps[ICE_RIDE_STEPS]; } '
                'sIceRides[] = {\n')
        for script, req, x, z, facing, steps, tiles, _hs in rows:
            f.write('    {%d, %d, %d, %d, %s, %d, {%s}},   // (to %d, %d)\n'
                    % (script, req, x, z, facing, len(steps), ', '.join(steps), tiles[-1][0], tiles[-1][1]))
        f.write('};\n\n')
        dt = dragon_tiles()
        f.write("// DRAYDEN's gym (Black's ov046): the tiles each dragon's pose opens (its pair of lines), not floor otherwise\n")
        f.write('#define DRAGON_GYM_ZONE %d\n' % DRAGON_ZONE)
        f.write('static const u8 sDragonTiles[][2] = {\n')
        starts, n = [], 0
        for u, unit in enumerate(dt):
            for p, tiles in enumerate(unit):
                starts.append((n, len(tiles)))
                n += len(tiles)
                if tiles:
                    f.write('    %s,   // (dragon %d, pose %d)\n' % (', '.join('{%d, %d}' % t for t in tiles), u, p))
        f.write('};\n')
        f.write('static const u16 sDragonPoseTiles[3][4][2] = {   // (first, count in sDragonTiles)\n')
        for u in range(3):
            f.write('    {%s},\n' % ', '.join('{%d, %d}' % starts[u * 4 + p] for p in range(4)))
        f.write('};\n')
        ram46, d46 = scrcmd.ov_image(46)   # (sRotatingRoomRotateAnimTable: int [old pose][new pose], an animation + 5)
        rot = [[struct.unpack_from('<i', d46, ROTATE_ANIMS - ram46 + 4 * (4 * a + b))[0] - 5 for b in range(4)]
               for a in range(4)]
        f.write("// the dragon's turn from one pose to another: its model's animation (FIELD_GIMMICK_ROTATING_ROOM_"
                "GetRotateAnim)\n")
        f.write('static const s8 sDragonRotateAnim[4][4] = {%s};\n' % ', '.join(
            '{%s}' % ', '.join(str(v) for v in row) for row in rot))
        write_cannons(f)
        write_carts(f)
    print('%s: %d rides, %d dragon tiles' % (path, len(rows), sum(len(t) for unit in dt for t in unit)))


# SKYLA's cannons (ov041 field_gimmick_cannons.c): each cannon's cell and quarter turns, the direction its animation
# takes for a facing (GetCannonDirection), Black's frames of each direction's animation (fire sound, fire done, the
# player shown again, the flight started), and each flight: its ICA path (archive 0x79: u32 frames, flags scale /
# rotation / translation at 4-6, then per frame scale, rotation, translation floats as present) and landing cell
CANNON_CELLS, CANNON_TURNS, CANNON_RING = 0x021F40F0, 0x021F4660, 0x021F4080
CANNON_FRAMES = (0x021F4084, 0x021F408C, 0x021F4094, 0x021F409C)   # (fire sound, fire done, landing, flight start)
CANNON_FLIGHTS, CANNON_ICA_ARC = 0x021F4320, 'a/1/2/1'


def ica_path(d):
    n = struct.unpack_from('<I', d, 0)[0]
    has_s, has_r, has_t = d[4], d[5], d[6]
    step = 12 * (bool(has_s) + bool(has_r) + bool(has_t))
    off = 12 * (bool(has_s) + bool(has_r))
    if not has_t:
        return []
    return [struct.unpack_from('<3f', d, 8 + i * step + off) for i in range(n)]


def write_cannons(f):
    ram, d = scrcmd.ov_image(41)
    u16 = lambda a: struct.unpack_from('<H', d, a - ram)[0]   # noqa: E731
    cells = [tuple(u16(CANNON_CELLS + 6 * c + 2 * k) for k in range(3)) for c in range(10)]
    turns = [d[CANNON_TURNS - ram + c] for c in range(10)]
    ring = [d[CANNON_RING - ram + k] for k in range(4)]
    frames = [[u16(t + 2 * k) for k in range(4)] for t in CANNON_FRAMES]
    arc = narc.arc(CANNON_ICA_ARC)
    paths, flights = [], []
    for c in range(10):
        row = []
        for r in range(4):
            ica, lx, ly, lz, cam = struct.unpack_from('<5i', d, CANNON_FLIGHTS - ram + 20 * (4 * c + r))
            if ica < 0:
                row.append((0, 0, 0, 0, 0, 0))
                continue
            pts = ica_path(arc[ica])
            row.append((len(paths), len(pts), lx, ly, lz, cam))
            paths += pts
        flights.append(row)
    def turned(direction, n):   # (TurnRingDirection)
        return ring[(ring.index(direction) + n) % 4] if direction in ring else direction
    f.write("// SKYLA's cannons (ov041): cells (x, y, z), each facing's (ring: up, down, right, left) animation direction, "
            "Black's\n// frames of a direction's animation, the flights (first point, points, landing x, y, z, camera) "
            "and their paths\n// (ICA translations, an eighth of a unit)\n")
    f.write('static const u16 sCannonCells[10][3] = {%s};\n' % ', '.join('{%d, %d, %d}' % c for c in cells))
    f.write('static const u8 sCannonAnimDir[10][4] = {%s};\n' % ', '.join(
        '{%s}' % ', '.join(str(turned(r, turns[c])) for r in range(4)) for c in range(10)))
    f.write('static const u8 sCannonTurns[10] = {%s};\n' % ', '.join(str(t) for t in turns))
    f.write('enum { CANNON_FIRE_SE, CANNON_FIRE_DONE, CANNON_LANDING, CANNON_FLIGHT_START };\n')
    f.write('static const u16 sCannonFrames[4][4] = {%s};\n' % ', '.join(
        '{%s}' % ', '.join(str(v) for v in t) for t in frames))
    f.write('static const s16 sCannonFlights[10][4][6] = {\n')
    for row in flights:
        f.write('    {%s},\n' % ', '.join('{%s}' % ', '.join(str(v) for v in fl) for fl in row))
    f.write('};\n')
    f.write('static const s16 sCannonPath[][3] = {\n')
    for k in range(0, len(paths), 6):
        f.write('    %s,\n' % ', '.join('{%d, %d, %d}' % tuple(round(v * 8) for v in p) for p in paths[k:k + 6]))
    f.write('};\n')


# ELESA's coasters (ov039 field_gimmick_cart_lanes.c): the nine docks (cells packed x | z << 16) and their lanes, each
# cart's two stops (its animation's frame and the dock there, 255 none), each cart's ride path (archive 0x6B, an ICA
# as the cannons'), Black's ride origin and each lane's offset, the frames each cart's animation has
CART_DOCKS, CART_DOCK_LANES, CART_STOP_FRAMES, CART_STOP_DOCKS = 0x021F4C58, 0x021F4C0C, 0x021F4D78, 0x021F4C28
CART_RIDE_IDS, CART_RIDE_ORIGIN, CART_LANE_OFFSETS, CART_RIDE_ARC = 0x021F4C38, 0x021F4D54, 0x021F4CA0, 'a/1/0/7'


def write_carts(f):
    import gimmicks
    ram, d = scrcmd.ov_image(39)
    i32 = lambda a: struct.unpack_from('<i', d, a - ram)[0]   # noqa: E731
    docks = [(i32(CART_DOCKS + 4 * k) & 0xFFFF, i32(CART_DOCKS + 4 * k) >> 16) for k in range(9)]
    lanes = list(d[CART_DOCK_LANES - ram:CART_DOCK_LANES - ram + 9])
    stops = [[(i32(CART_STOP_FRAMES + 8 * c + 4 * k) >> 12, d[CART_STOP_DOCKS - ram + 2 * c + k]) for k in range(2)]
             for c in range(8)]
    arc = narc.arc(CART_RIDE_ARC)
    paths, rides = [], []
    for c in range(8):
        pts = ica_path(arc[i32(CART_RIDE_IDS + 4 * c)])
        rides.append((len(paths), len(pts)))
        paths += pts
    img = gimmicks.Image(39)
    origin = img.vec(CART_RIDE_ORIGIN)
    offsets = [img.vec(CART_LANE_OFFSETS + 12 * k) for k in range(4)]
    f.write("// ELESA's coasters (ov039): docks (x, z) and lanes, each cart's stops (frame, dock), its ride path (first, points;\n"
            "// ICA translations, an eighth of a unit, from Black's origin plus its lane's offset)\n")
    f.write('static const u8 sCartDocks[9][3] = {%s};\n' % ', '.join(
        '{%d, %d, %d}' % (x, z, ln) for (x, z), ln in zip(docks, lanes)))
    f.write('static const u16 sCartStops[8][2][2] = {%s};\n' % ', '.join(
        '{%s}' % ', '.join('{%d, %d}' % st for st in row) for row in stops))
    f.write('static const u16 sCartRides[8][2] = {%s};\n' % ', '.join('{%d, %d}' % r for r in rides))
    f.write('static const s16 sCartRideOffset[4][3] = {%s};\n' % ', '.join(
        '{%d, %d, %d}' % tuple(round(o[k] + origin[k]) for k in range(3)) for o in offsets))
    f.write('static const s16 sCartPath[][3] = {\n')
    for k in range(0, len(paths), 6):
        f.write('    %s,\n' % ', '.join('{%d, %d, %d}' % tuple(round(v * 8) for v in p) for p in paths[k:k + 6]))
    f.write('};\n')


if __name__ == '__main__':
    main()
