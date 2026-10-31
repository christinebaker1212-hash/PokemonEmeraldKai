#!/usr/bin/env python3
"""Which tiles are trees for HEADBUTT on JOHTO's / KANTO's maps.

    python3 tools/quetzal_port/gen_headbutt_trees.py [--show OUT.png]

Quetzal's maps have a tree / rock table (src/data/wild_headbutt_qz.h) but no HEADBUTT in the field, and its trees have
no tile behavior of their own. For each map with a table this draws every metatile the map uses where the player can't
walk (the block's collision bits) and keeps the ones drawn mostly in greens with dark green in them: trees, and the
odd bush. Writes src/data/qz_headbutt_trees.h: per map, its tree metatile ids (field_control_avatar.c asks
MP_Headbutt_IsTree). --show draws what was picked, one row per map, to check by eye.
"""
import json
import os
import re
import struct
import sys

from PIL import Image

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
RIP = os.path.join(REPO, 'quetzal_rip')
OUTDOORS = ('MAP_TYPE_TOWN', 'MAP_TYPE_CITY', 'MAP_TYPE_ROUTE', 'MAP_TYPE_OCEAN_ROUTE')


class Tiles:
    def __init__(self, kind, name):
        d = os.path.join(RIP, 'tilesets', kind, name)
        self.ok = os.path.exists(os.path.join(d, 'metatiles.bin'))
        if not self.ok:
            return
        self.img = Image.open(os.path.join(d, 'tiles.png'))
        self.px = self.img.load()
        self.w = self.img.size[0] // 8
        self.count = self.w * (self.img.size[1] // 8)
        self.mt = open(os.path.join(d, 'metatiles.bin'), 'rb').read()
        self.pals = []
        for i in range(16):
            p = os.path.join(d, 'palettes', '%02d.pal' % i)
            rows = open(p).read().split('\n')[3:19] if os.path.exists(p) else []
            self.pals.append([tuple(int(v) for v in r.split()) for r in rows if r.strip()] or [(255, 0, 255)] * 16)


def load_pair(prim, sec):
    p = Tiles('primary', prim[len('gTileset_'):])
    s = Tiles('secondary', sec[len('gTileset_'):]) if sec else None
    return p, s


def render(m, p, s):
    frlg = len(p.mt) // 16 > 512
    msplit, tsplit, psplit = (640, 640, 7) if frlg else (512, 512, 6)
    src = p if m < msplit else s
    if src is None or not src.ok:
        return None
    idx = m if m < msplit else m - msplit
    if idx * 16 + 16 > len(src.mt):
        return None
    im = Image.new('RGB', (16, 16), (0, 0, 0))
    bp = im.load()
    for layer in range(2):
        for k in range(4):
            v = struct.unpack_from('<H', src.mt, idx * 16 + layer * 8 + k * 2)[0]
            t, hf, vf, pal = v & 0x3FF, (v >> 10) & 1, (v >> 11) & 1, v >> 12
            tiles = p if t < tsplit else s
            if tiles is None or not tiles.ok:
                continue
            t = t if t < tsplit else t - tsplit
            if t >= tiles.count:
                continue
            pals = (p if pal < psplit else (s if s and s.ok else p)).pals
            tx, ty = (t % tiles.w) * 8, (t // tiles.w) * 8
            for yy in range(8):
                for xx in range(8):
                    c = tiles.px[tx + xx, ty + yy]
                    if c == 0 and layer == 1:
                        continue
                    col = pals[pal][c] if c < len(pals[pal]) else (255, 0, 255)
                    bp[(k % 2) * 8 + (7 - xx if hf else xx), (k // 2) * 8 + (7 - yy if vf else yy)] = col
    return im


def is_tree(im):
    green = dark = 0
    for r, g, b in im.get_flattened_data() if hasattr(im, 'get_flattened_data') else im.getdata():
        if g > r + 15 and g + 30 >= b:   # (JOHTO's third tileset draws them blue-green)
            green += 1
        if g > r and g + 20 > b and r + g + b < 240:
            dark += 1
    return green >= 0.72 * 256 and dark >= 0.1 * 256


def main():
    show = sys.argv[sys.argv.index('--show') + 1] if '--show' in sys.argv else None
    tables = re.findall(r'\{(MAP_\w+), \d+,', open(os.path.join(REPO, 'src/data/wild_headbutt_qz.h')).read())
    groups = json.load(open(os.path.join(REPO, 'data/maps/map_groups.json')))
    by_id = {}
    for g in groups['group_order']:
        for name in groups[g]:
            by_id[json.load(open(os.path.join(REPO, 'data/maps', name, 'map.json')))['id']] = name
    layouts = {l['id']: l for l in json.load(open(os.path.join(RIP, 'layouts', 'layouts.json')))['layouts'] if 'id' in l}
    out, rows, pairs = [], [], {}
    for mid in tables:
        name = by_id.get(mid)
        if not name:
            continue
        m = json.load(open(os.path.join(REPO, 'data/maps', name, 'map.json')))
        if m.get('map_type') not in OUTDOORS:   # (caves' greenish walls aren't trees)
            continue
        lay = layouts.get(m['layout'])
        if not lay:
            continue
        b = open(os.path.join(REPO, lay['blockdata_filepath']), 'rb').read()
        used = sorted({v & 0x3FF for (v,) in struct.iter_unpack('<H', b) if (v >> 10) & 3})
        key = (lay['primary_tileset'], lay['secondary_tileset'])
        if key not in pairs:
            pairs[key] = load_pair(*key)
        p, s = pairs[key]
        trees = []
        for mt in used:
            im = render(mt, p, s)
            if im is not None and is_tree(im):
                trees.append((mt, im))
        if trees:
            out.append('static const u16 sTrees_%s[] = {%s};' % (name, ', '.join(str(t) for t, _ in trees)))
            rows.append((name, mid, len(trees), trees))
    text = ['// Generated by tools/quetzal_port/gen_headbutt_trees.py: the tree metatiles of JOHTO / KANTO maps with a',
            '// tree table (wild_headbutt_qz.h), for HEADBUTT (wild_encounter.c MP_Headbutt_IsTree)'] + out
    text.append('static const struct {u16 map; u8 count; const u16 *ids;} sHeadbuttTrees[] = {')
    text += ['    {%s, %d, sTrees_%s},' % (mid, n, name) for name, mid, n, _ in rows]
    text += ['};', '']
    open(os.path.join(REPO, 'src/data/qz_headbutt_trees.h'), 'w').write('\n'.join(text))
    print('%d maps with trees (of %d with a table)' % (len(rows), len(tables)))
    if show:
        w = max(n for _, _, n, _ in rows)
        img = Image.new('RGB', (w * 18, len(rows) * 18), (255, 255, 255))
        for y, (_, _, _, trees) in enumerate(rows):
            for x, (_, im) in enumerate(trees):
                img.paste(im, (x * 18, y * 18))
        img.resize((img.size[0] * 2, img.size[1] * 2), Image.NEAREST).save(show)


if __name__ == '__main__':
    main()
