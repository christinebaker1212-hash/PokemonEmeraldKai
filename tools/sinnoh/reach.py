#!/usr/bin/env python3
"""Who can't be reached on SINNOH's maps: walks each map's own collision (Platinum's terrain attributes, as the game
uses them: tools/sinnoh/build_pack.py blocks()) from its doors and its edges, and lists the people with a script (and
the item balls) that no free tile next to them reaches. A counter between counts as next to (it is talked across).
Water counts as walkable (SURF has no badge restriction in this game); objects don't block.

    python3 -I tools/sinnoh/reach.py POKEPLATINUM_DIR [MAP...]      (MAP: Sinnoh_OreburghCityGym...; default all)

Used to find what a gym's puzzle (water levels, lifts, rotating bridges) or a story block leaves out of reach.
"""
import json
import os
import sys
from collections import deque

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from platinum import Land  # noqa: E402
from source import open_source, NONE  # noqa: E402
from gen_maps import places  # noqa: E402

COUNTER = 0x80


def grid(src, p):
    """{(x, y): attribute} of the map (local tiles), None where a cell is missing"""
    mt = src.matrix(p['matrix'])
    w, h = 32 * (p['x1'] - p['x0'] + 1), 32 * (p['z1'] - p['z0'] + 1)
    out = {}
    for cz in range(p['z0'], p['z1'] + 1):
        for cx in range(p['x0'], p['x1'] + 1):
            lid = mt.cell(cx, cz)[0] if cx < mt.width and cz < mt.height else NONE
            land = Land(src.land(lid)) if lid != NONE else None
            for ty in range(32):
                for tx in range(32):
                    x, y = (cx - p['x0']) * 32 + tx, (cz - p['z0']) * 32 + ty
                    out[(x, y)] = land.attrs[ty * 32 + tx] if land else 0x8000
    return out, w, h


def free(a):
    return a is not None and (not (a & 0x8000) or (a & 0xFF) in (0x4B, 0x4C, 0x13))   # (ROCK CLIMB walls, waterfalls)


def check(src, p):
    path = 'data/maps/%s/map.json' % p['name']
    m = json.load(open(path))
    g, w, h = grid(src, p)
    starts = [(e['x'], e['y']) for e in m['warp_events']]
    if p['outdoor']:
        starts += [(x, y) for x in range(w) for y in (0, h - 1)] + [(x, y) for y in range(h) for x in (0, w - 1)]
    seen = set()
    todo = deque(s for s in starts if s in g)
    seen.update(todo)
    while todo:
        x, y = todo.popleft()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if n in g and n not in seen and (free(g[n]) or n in set(starts)):
                seen.add(n)
                todo.append(n)
    lost = []
    for i, o in enumerate(m['object_events']):
        if o['flag'] == '0x3FFF' or o['script'] == '0x0':
            continue
        x, y = o['x'], o['y']
        ok = False
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            if (x + dx, y + dy) in seen:
                ok = True
            elif (g.get((x + dx, y + dy), 0x8000) & 0xFF) == COUNTER and (x + 2 * dx, y + 2 * dy) in seen:
                ok = True
        if not ok:
            lost.append('Person%d %s at %d,%d' % (i + 1, o['script'].split('_EventScript_')[-1], x, y))
    return lost


def region(src, root, start_map, start):
    """Every map a player gets to from start (local tile on start_map) through doors and map edges, and the tiles
    reached on each: {map name: set of tiles}"""
    ps = {p['name']: p for p in places(src, root)}
    ids = {}
    data = {}
    for name in ps:
        m = json.load(open('data/maps/%s/map.json' % name))
        ids[m['id']] = name
        data[name] = m
    grids = {}

    def g(name):
        if name not in grids:
            grids[name] = grid(src, ps[name])
        return grids[name]
    reached = {}
    todo = deque([(start_map, start)])
    while todo:
        name, t = todo.popleft()
        tiles = reached.setdefault(name, set())
        if t in tiles:
            continue
        cells, w, h = g(name)
        if t not in cells:
            continue
        tiles.add(t)
        m = data[name]
        for e in m['warp_events']:
            if (e['x'], e['y']) == t and e['dest_map'] in ids:
                dest = ids[e['dest_map']]
                wid = int(e['dest_warp_id'])
                dw = data[dest]['warp_events']
                if wid < len(dw):
                    todo.append((dest, (dw[wid]['x'], dw[wid]['y'])))
        x, y = t
        for c in m.get('connections') or []:
            if c['map'] not in ids:
                continue
            other = ids[c['map']]
            off = int(c['offset'])
            ow, oh = g(other)[1], g(other)[2]
            if c['direction'] == 'up' and y == 0:
                todo.append((other, (x - off, oh - 1)))
            elif c['direction'] == 'down' and y == h - 1:
                todo.append((other, (x - off, 0)))
            elif c['direction'] == 'left' and x == 0:
                todo.append((other, (ow - 1, y - off)))
            elif c['direction'] == 'right' and x == w - 1:
                todo.append((other, (0, y - off)))
        doors = {(e['x'], e['y']) for e in m['warp_events']}
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (x + dx, y + dy)
            if n in cells and n not in tiles and (free(cells[n]) or n in doors):   # (the game makes doors walkable)
                todo.append((name, n))
    return reached


def main():
    root = sys.argv[1]
    if len(sys.argv) > 2 and sys.argv[2] == '--from':
        # --from MAP X Y: the maps a player can get to from there, and those they can't
        src = open_source(root)
        r = region(src, root, sys.argv[3], (int(sys.argv[4]), int(sys.argv[5])))
        allmaps = sorted(p['name'] for p in places(src, root))
        print('reached %d of %d maps' % (len(r), len(allmaps)))
        print('reached: ' + ' '.join('%s(%d)' % (n[7:], len(t)) for n, t in sorted(r.items())))
        print('not reached: ' + ' '.join(n[7:] for n in allmaps if n not in r))
        return
    only = set(sys.argv[2:])
    src = open_source(root)
    total = 0
    for p in places(src, root):
        if only and p['name'] not in only:
            continue
        lost = check(src, p)
        if lost:
            total += len(lost)
            print('%s: %s' % (p['name'], '; '.join(lost)))
    print('%d out of reach' % total)


if __name__ == '__main__':
    main()
