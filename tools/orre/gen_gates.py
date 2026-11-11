#!/usr/bin/env python3
"""ORRE's story gates: the collision objects Colosseum's room scripts switch on and off (scriptSetCol, native 0x76:
GScolsys2SetObjEnable; tools/orre/colo_rooms.py Room.gates). The maps are built with them open (tools/orre/maps.py
build_grid); here each gate's walls are laid on its room's grid: the walkable tiles they cross (at a person's height),
which src/orre/orre_vm.c makes impassable while a script has the gate on, as src/data/orre_gates.h.

    python3 -I tools/orre/gen_gates.py [UNPACKED]     (default ~/.cache/moemon/orre/unpacked)

A row: the room (Colosseum's id), the gate (its collision object), the first tile in sOrreGateTiles and how many, and
whether it starts closed: Colosseum's collision objects start on, but a gate its room's scripts also switch on starts
open here (which story state switched it on can't be told apart from a fresh load); one they only ever open starts
closed (PHENAC's fence round RUI's sack).
"""
import os
import struct
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import grid  # noqa: E402
import maps  # noqa: E402

OUT = os.path.join(ROOT, 'src', 'data', 'orre_gates.h')


def gate_walls(data, index):
    """The gate's wall triangles (n, 3, 3) in the map's world"""
    off, n = struct.unpack_from('>II', data, 0)
    if index >= n:
        return np.zeros((0, 3, 3))
    o = off + index * 0x40
    rot, t = grid._mtx(struct.unpack_from('>9f', data, o))
    p = struct.unpack_from('>I', data, o + 0x28)[0]
    if not p:
        return np.zeros((0, 3, 3))
    tris, cnt = struct.unpack_from('>II', data, p)
    if cnt == 0 or tris + cnt * 0x34 > len(data):
        return np.zeros((0, 3, 3))
    T = np.array([struct.unpack_from('>9f', data, tris + k * 0x34) for k in range(cnt)]).reshape(-1, 3, 3)
    return T @ rot.T + t


def gate_tiles(g, walls):
    """The walkable tiles of grid G a gate's walls cross where a person stands"""
    out = []
    tile = g['tile']
    segs = []
    for T in walls:
        ys = T[:, 1]
        pts = T[:, [0, 2]]
        for a, b in ((0, 1), (1, 2), (2, 0)):
            segs.append((pts[a], pts[b], ys.min(), ys.max()))
    for ty in range(g['h']):
        for tx in range(g['w']):
            if not g['walk'][ty, tx] or not g['heights'][ty][tx]:
                continue
            h = g['heights'][ty][tx][0]
            cx, cz = g['x0'] + (tx + 0.5) * tile, g['z0'] + (ty + 0.5) * tile
            r = tile / 2 - 0.5   # (the tile itself: a wall crossing it closes it, one beside it leaves it)
            for p0, p1, y0, y1 in segs:
                if y1 >= h + 1.0 and y0 <= h + grid.PERSON and grid._seg_hits_box(p0, p1, cx - r, cz - r, cx + r, cz + r):
                    out.append((tx, ty))
                    break
    # a wall between two tiles (Emerald's grid has no walls on its edges): the tile farther from the gate's middle
    # closes (a fence round something blocks its outside, never the inside it fences)
    if segs:
        mid = np.mean([s_[0] for s_ in segs] + [s_[1] for s_ in segs], axis=0)
        closed = set(out)

        def centre(x, y):
            return np.array([g['x0'] + (x + 0.5) * tile, g['z0'] + (y + 0.5) * tile])

        for ty in range(g['h']):
            for tx in range(g['w']):
                for nx, ny in ((tx + 1, ty), (tx, ty + 1)):
                    if nx >= g['w'] or ny >= g['h'] or not g['walk'][ty, tx] or not g['walk'][ny, nx]:
                        continue
                    if (tx, ty) in closed or (nx, ny) in closed or not g['heights'][ty][tx] or not g['heights'][ny][nx]:
                        continue
                    h = min(g['heights'][ty][tx][0], g['heights'][ny][nx][0])
                    a, b = centre(tx, ty), centre(nx, ny)
                    for p0, p1, y0, y1 in segs:
                        if y1 >= h + 1.0 and y0 <= h + grid.PERSON and grid._seg_cross(a, b, p0, p1):
                            far = (tx, ty) if np.linalg.norm(a - mid) > np.linalg.norm(b - mid) else (nx, ny)
                            closed.add(far)
                            out.append(far)
                            break
    return out


def set_calls(unpacked, file):
    """{gate: {0, 1}}: the values the room's scripts switch each gate to (tools/orre/gsvm.py's listing)"""
    import re
    import subprocess
    out = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'gsvm.py'),
                          os.path.join(unpacked, file, file + '.scd')], capture_output=True, text=True).stdout
    calls = {}
    for m in re.finditer(r'scriptSetCol\((\d+), (\d+)\)', out):
        calls.setdefault(int(m.group(1)), set()).add(int(m.group(2)))
    return calls


def main():
    unpacked = sys.argv[1] if len(sys.argv) > 1 else os.path.expanduser('~/.cache/moemon/orre/unpacked')
    rows, tiles, doors = [], [], []
    for m in maps.load(unpacked):
        for d in m['doors']:
            doors.append('    {%d, %d, %d, %d, %d, %d, %d},   // %s' % (m['room'], d[0], d[1], d[2], d[3], d[4], d[5], m['file']))
        objects = sorted(set(m['gates']) | maps.door_objects(m))
        if not objects:
            continue
        p = os.path.join(unpacked, m['file'], m['file'] + '.col')
        data = open(p, 'rb').read()
        g = maps.build_grid(unpacked, m)
        calls = set_calls(unpacked, m['file'])
        for gate in objects:
            ts = gate_tiles(g, gate_walls(data, gate))
            if not ts:
                continue
            # (a door's leaf is shut or open by its own flag when the room starts: orre_vm.c InitDoors)
            closed = calls.get(gate) == {0} and gate not in maps.door_objects(m)
            rows.append('    {%d, %d, %d, %d, %d},   // %s' % (m['room'], gate, len(tiles), len(ts), closed, m['file']))
            tiles.extend(ts)
    lines = ['// ORRE\'s story gates: the walkable tiles each collision object a room\'s scripts switch (scriptSetCol) closes',
             '// (generated by tools/orre/gen_gates.py from the disc\'s collision; do not edit): src/orre/orre_vm.c', '',
             '#define ORRE_GATE_COUNT %d' % len(rows), '#define ORRE_GATE_TILE_COUNT %d' % len(tiles), '',
             'static const struct OrreGate sOrreGates[ORRE_GATE_COUNT] = {'] + rows + ['};', '',
             'static const u8 sOrreGateTiles[ORRE_GATE_TILE_COUNT][2] = {']
    for i in range(0, len(tiles), 12):
        lines.append('    ' + ' '.join('{%d, %d},' % t for t in tiles[i:i + 12]))
    lines += ['};', '',
              '// Colosseum\'s doors (its door table: tools/orre/colo_rooms.py Room.doors): {room, the door\'s number in the',
              '// table, its leaf\'s collision object (-1 none), the flag set while it\'s open, the flag that unlocks it (0 none),',
              '// its type, whether walking up to it opens it (the shared script 0x05960005 on a touch trigger)}', '',
              '#define ORRE_DOOR_COUNT %d' % len(doors), '',
              'static const struct OrreDoor sOrreDoors[ORRE_DOOR_COUNT] = {'] + doors + ['};', '']
    open(OUT, 'w').write('\n'.join(lines))
    print('%s: %d gates, %d tiles, %d doors' % (OUT, len(rows), len(tiles), len(doors)))


if __name__ == '__main__':
    main()
