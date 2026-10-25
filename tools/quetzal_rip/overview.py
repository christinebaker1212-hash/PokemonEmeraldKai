"""Stitch a region's connected outdoor maps into one picture (layouts placed by their connections)."""
from collections import deque

from PIL import Image

from render import TilesetCache, render_layout


def place(maps_by_key, start):
    pos = {start: (0, 0)}
    q = deque([start])
    while q:
        k = q.popleft()
        m = maps_by_key[k]
        x, y = pos[k]
        w, h = m['layout']['width'], m['layout']['height']
        for c in m['connections']:
            nk = tuple(c['map'])
            if nk in pos or nk not in maps_by_key:
                continue
            n = maps_by_key[nk]
            nw, nh = n['layout']['width'], n['layout']['height']
            o = c['offset']
            p = {2: (x + o, y - nh), 1: (x + o, y + h), 3: (x - nw, y + o), 4: (x + w, y + o)}.get(c['direction'])
            if p:
                pos[nk] = p
                q.append(nk)
    return pos


def overview(rom, maps, start, scale=0.25, path=None):
    by = {(m['group'], m['num']): m for m in maps}
    pos = place(by, start)
    minx = min(x for x, y in pos.values())
    miny = min(y for x, y in pos.values())
    maxx = max(x + by[k]['layout']['width'] for k, (x, y) in pos.items())
    maxy = max(y + by[k]['layout']['height'] for k, (x, y) in pos.items())
    img = Image.new('RGB', ((maxx - minx) * 16, (maxy - miny) * 16), (0, 0, 0))
    cache = TilesetCache(rom)
    for k, (x, y) in pos.items():
        img.paste(render_layout(rom, by[k]['layout'], cache), ((x - minx) * 16, (y - miny) * 16))
    if scale != 1:
        img = img.resize((int(img.width * scale), int(img.height * scale)), Image.LANCZOS)
    if path:
        img.save(path, optimize=True)
    return img, pos
