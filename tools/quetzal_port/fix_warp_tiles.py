#!/usr/bin/env python3
"""Door mats that only work on one half: Johto / Kanto's FireRed-style mats are two tiles with a warp on each, but in
Quetzal's data only one of the two metatiles is an arrow warp (MB_SOUTH_ARROW_WARP...); the other is plain floor, so
pressing toward the door on that half does nothing. This gives such a metatile its neighbour's arrow behaviour (in its
tileset's metatile_attributes.bin), for every warp on plain floor beside an arrow-warp tile of a warp to the same map.

    python3 tools/quetzal_port/fix_warp_tiles.py      (also run by import.py)
"""
import glob
import json
import os
import re
import struct

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
ARROWS = range(0x62, 0x66)   # MB_EAST / WEST / NORTH / SOUTH_ARROW_WARP


def main():
    names = re.findall(r'^\s+(MB_\w+)', open(os.path.join(REPO, 'include/constants/metatile_behaviors.h')).read(), re.M)
    assert names[0x62] == 'MB_EAST_ARROW_WARP' and names[0x65] == 'MB_SOUTH_ARROW_WARP', 'behaviour numbers moved'
    layouts = {l['id']: l for l in json.load(open(os.path.join(REPO, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
    attr_path = {}
    for f in glob.glob(os.path.join(REPO, 'src/data/tilesets/*.h')):
        for m in re.finditer(r'gMetatileAttributes_(\w+)\[\] = \w+\("([^"]+)"\)', open(f).read()):
            attr_path['gTileset_' + m.group(1)] = os.path.join(REPO, m.group(2))
    attrs = {}

    def attr_file(ts):
        if ts not in attrs:
            attrs[ts] = bytearray(open(attr_path[ts], 'rb').read())
        return attrs[ts]

    changed = {}
    for mj in sorted(glob.glob(os.path.join(REPO, 'data/maps/Johto_*/map.json')) +
                     glob.glob(os.path.join(REPO, 'data/maps/Kanto_*/map.json'))):
        m = json.load(open(mj))
        lay = layouts.get(m.get('layout'))
        if not lay or not m.get('warp_events'):
            continue
        frlg = lay['primary_tileset'] in attr_path and 'Johto' in lay['primary_tileset'] or 'Kanto' in lay['primary_tileset']
        blocks = open(os.path.join(REPO, lay['blockdata_filepath']), 'rb').read()
        w, h = lay['width'], lay['height']
        n_primary = 640 if frlg else 512

        def tile(x, y):
            if not (0 <= x < w and 0 <= y < h):
                return None
            mt = struct.unpack_from('<H', blocks, (y * w + x) * 2)[0] & 0x3FF
            ts, i = (lay['primary_tileset'], mt) if mt < n_primary else (lay['secondary_tileset'], mt - n_primary)
            if ts not in attr_path:
                return None
            a = attr_file(ts)
            if 2 * i + 2 > len(a):
                return None
            return ts, i, struct.unpack_from('<H', a, 2 * i)[0]

        warps = {(wp['x'], wp['y']): wp['dest_map'] for wp in m['warp_events']}
        for (x, y), dest in warps.items():
            t = tile(x, y)
            if not t or (t[2] & 0xFF) != 0:
                continue
            for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                if warps.get((x + dx, y + dy)) != dest:
                    continue
                n = tile(x + dx, y + dy)
                if n and (n[2] & 0xFF) in ARROWS:
                    ts, i, v = t
                    nv = (v & 0xFF00) | (n[2] & 0xFF)
                    struct.pack_into('<H', attr_file(ts), 2 * i, nv)
                    changed[(ts, i)] = (os.path.basename(os.path.dirname(mj)), x, y, names[n[2] & 0xFF])
                    break
    for ts, a in attrs.items():
        open(attr_path[ts], 'wb').write(a)
    for (ts, i), (mp, x, y, b) in sorted(changed.items()):
        print('  %s metatile %d -> %s (%s %d,%d)' % (ts, i, b, mp, x, y))
    print('warp tiles: %d metatiles made arrow warps' % len(changed))


if __name__ == '__main__':
    main()
