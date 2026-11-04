#!/usr/bin/env python3
"""Take Quetzal's people / things out of a JOHTO / KANTO map for good (docs/REGIONS_AUDIT.md): their object flag
becomes FLAG_KAI_GONE (always set), so the map's local ids stay as they are. Also drops warps / signs / triggers.

    python3 tools/quetzal_port/kai_gone.py MAP obj:X,Y [obj:X,Y ...] [id:N] [warp:X,Y] [bg:X,Y] [coord:X,Y]

A warp is only dropped when it's the map's last one (others are found by their index); otherwise it says so.
"""
import json
import os
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def gone(mapname, items):
    path = os.path.join(REPO, 'data/maps', mapname, 'map.json')
    d = json.load(open(path))
    for it in items:
        kind, arg = it.split(':', 1)
        if kind == 'id':
            d['object_events'][int(arg) - 1]['flag'] = 'FLAG_KAI_GONE'
            continue
        x, y = (int(v) for v in arg.split(','))
        if kind == 'obj':
            hits = [o for o in d['object_events'] if o['x'] == x and o['y'] == y]
            if not hits:
                raise SystemExit('%s: no object at %d,%d' % (mapname, x, y))
            for o in hits:
                o['flag'] = 'FLAG_KAI_GONE'
        elif kind == 'warp':
            ws = d['warp_events']
            i = [k for k, w in enumerate(ws) if w['x'] == x and w['y'] == y]
            if not i:
                raise SystemExit('%s: no warp at %d,%d' % (mapname, x, y))
            if i[0] != len(ws) - 1:
                raise SystemExit('%s: the warp at %d,%d is not the last one (index %d)' % (mapname, x, y, i[0]))
            del ws[i[0]]
        elif kind in ('bg', 'coord'):
            key = 'bg_events' if kind == 'bg' else 'coord_events'
            before = len(d[key])
            d[key] = [e for e in d[key] if not (e['x'] == x and e['y'] == y)]
            if len(d[key]) == before:
                raise SystemExit('%s: no %s event at %d,%d' % (mapname, kind, x, y))
    json.dump(d, open(path, 'w'), indent=2)
    open(path, 'a').write('\n')


if __name__ == '__main__':
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    gone(sys.argv[1], sys.argv[2:])
