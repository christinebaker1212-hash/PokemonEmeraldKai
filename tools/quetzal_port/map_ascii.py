#!/usr/bin/env python3
"""A map as text, for picking spots in Quetzal's redrawn maps (item balls, hidden items, people).

    python3 tools/quetzal_port/map_ascii.py Johto_Route30 [X0 Y0 X1 Y1]

'.' walkable, '#' blocked (collision), '~' water / surf (behavior), 'O' an object (its local id's last digit after
the map: listed below), 'W' a warp, 'S' a sign / bg event. Collision and elevation come from the layout's map.bin.
"""
import json
import os
import struct
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def layout_of(mapname):
    mj = json.load(open(os.path.join(REPO, 'data/maps', mapname, 'map.json')))
    lay = json.load(open(os.path.join(REPO, 'data/layouts/layouts.json')))
    for l in lay['layouts']:
        if l.get('id') == mj['layout']:
            return mj, l
    raise SystemExit('no layout ' + mj['layout'])


def main():
    mapname = sys.argv[1]
    mj, l = layout_of(mapname)
    w, h = l['width'], l['height']
    data = open(os.path.join(REPO, l['blockdata_filepath']), 'rb').read()
    cells = struct.unpack('<%dH' % (w * h), data[:w * h * 2])
    x0, y0, x1, y1 = (int(v) for v in sys.argv[2:6]) if len(sys.argv) >= 6 else (0, 0, w - 1, h - 1)
    grid = [['#' if (cells[y * w + x] >> 10) & 3 else '.' for x in range(w)] for y in range(h)]
    marks = {}
    for i, o in enumerate(mj.get('object_events', [])):
        if o.get('flag') == 'FLAG_KAI_GONE':
            continue
        grid[o['y']][o['x']] = 'O'
        marks[(o['x'], o['y'])] = 'obj %d %s' % (i + 1, o['script'])
    for wv in mj.get('warp_events', []):
        grid[wv['y']][wv['x']] = 'W'
    for b in mj.get('bg_events', []):
        if 0 <= b['y'] < h and 0 <= b['x'] < w:
            grid[b['y']][b['x']] = 'S'
    print('    ' + ''.join(str(x // 10 % 10) for x in range(x0, x1 + 1)))
    print('    ' + ''.join(str(x % 10) for x in range(x0, x1 + 1)))
    for y in range(y0, y1 + 1):
        print('%3d ' % y + ''.join(grid[y][x0:x1 + 1]))
    for (x, y), t in sorted(marks.items(), key=lambda kv: (kv[0][1], kv[0][0])):
        if x0 <= x <= x1 and y0 <= y <= y1:
            print('  %d,%d %s' % (x, y, t))


if __name__ == '__main__':
    main()
