#!/usr/bin/env python3
"""Renders a map layout (General + a secondary tileset) to a PNG, to look at a map without the game.

Usage: render_map.py LAYOUT_DIR SECONDARY OUT.png [SCALE] [PRIMARY]
  e.g. render_map.py data/layouts/MoemonGarden moemon_garden garden.png 2
"""
import os, struct, sys
from PIL import Image

ROOT = os.path.join(os.path.dirname(__file__), '..', '..')

def pal(path):
    lines = open(path).read().split('\n')[3:19]
    return [tuple(int(v) for v in l.split()[:3]) for l in lines if l.strip()]

def tileset(name, primary):
    d = os.path.join(ROOT, 'data/tilesets', 'primary' if primary else 'secondary', name)
    img = Image.open(os.path.join(d, 'tiles.png'))
    px = img.load()
    w, h = img.size
    tiles = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            tiles.append([[px[tx * 8 + x, ty * 8 + y] & 15 for x in range(8)] for y in range(8)])
    mt = open(os.path.join(d, 'metatiles.bin'), 'rb').read()
    metas = [struct.unpack('<8H', mt[i:i + 16]) for i in range(0, len(mt), 16)]
    pals = [pal(os.path.join(d, 'palettes', '%02d.pal' % i)) for i in range(16)]
    return tiles, metas, pals

def main():
    layout, second, out = sys.argv[1], sys.argv[2], sys.argv[3]
    scale = int(sys.argv[4]) if len(sys.argv) > 4 else 1
    pt, pm, pp = tileset(sys.argv[5] if len(sys.argv) > 5 else 'general', True)
    st, sm, sp = tileset(second, False)
    tiles = pt[:512] + [[[0] * 8] * 8] * (512 - min(512, len(pt))) + st
    palettes = pp[:6] + sp[6:13] + pp[13:16]
    metas = pm[:512] + [(0,) * 8] * (512 - len(pm[:512])) + sm
    d = open(os.path.join(ROOT, layout, 'map.bin'), 'rb').read()
    cells = struct.unpack('<%dH' % (len(d) // 2), d)
    # width from layouts.json
    import json
    lj = json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))
    name = os.path.basename(os.path.normpath(layout))
    w = [l for l in lj['layouts'] if l.get('blockdata_filepath', '').startswith(layout.rstrip('/') + '/') or
         l.get('blockdata_filepath', '').find('/' + name + '/') >= 0][0]['width']
    h = len(cells) // w
    im = Image.new('RGB', (w * 16, h * 16))
    px = im.load()
    for my in range(h):
        for mx in range(w):
            m = metas[cells[my * w + mx] & 0x3FF]
            for layer in range(2):
                for k in range(4):
                    e = m[layer * 4 + k]
                    t = tiles[e & 0x3FF] if (e & 0x3FF) < len(tiles) else [[0] * 8] * 8
                    hf, vf, p = e & 0x400, e & 0x800, (e >> 12) & 15
                    for y in range(8):
                        for x in range(8):
                            c = t[7 - y if vf else y][7 - x if hf else x]
                            if c == 0 and layer:
                                continue
                            px[mx * 16 + (k % 2) * 8 + x, my * 16 + (k // 2) * 8 + y] = palettes[p][c]
    if scale > 1:
        im = im.resize((im.size[0] * scale, im.size[1] * scale), Image.NEAREST)
    im.save(out)
    print('%s: %dx%d' % (out, w, h))

main()
