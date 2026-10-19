#!/usr/bin/env python3
"""Moemon Co-op: contact sheets of every hand-mapped sprite in a few test looks.

Usage: colorcheck.py DUMP_DIR OUT_DIR [kind]
Applies colormaps.txt the same way mp_avatar.c RecolorByMap does (presets read
from mp_avatar.c), so what you see here is what the game shows.
"""
import os
import re
import sys

import numpy as np
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, '..', '..')
sys.path.insert(0, HERE)
from colorsheets import load  # noqa: E402


def rgb5(r, g, b):
    return r | (g << 5) | (b << 10)


def presets():
    src = open(os.path.join(ROOT, 'src/mp/mp_avatar.c')).read()

    def block(name):
        body = re.search(name + r'\[[^\]]*\](?:\[\d\])? = \{(.*?)\n\};', src, re.S).group(1)
        rows = []
        for line in body.split('\n'):
            vals = [rgb5(*map(int, m)) for m in re.findall(r'RGB\((\d+),\s*(\d+),\s*(\d+)\)', line)]
            if vals or re.search(r'\{0, 0', line) or re.match(r'\s*0,', line):
                rows.append(vals or [0, 0, 0])
        return rows
    return block('sSkinPresets'), block('sColorPresets'), block('sHairPresets')


def cdiv(a, b):
    q = abs(a) // abs(b)
    return q if (a >= 0) == (b > 0) else -q


def rgb_to_hsv(c):
    r = (c & 31) * 255 // 31
    g = ((c >> 5) & 31) * 255 // 31
    b = ((c >> 10) & 31) * 255 // 31
    mx, mn = max(r, g, b), min(r, g, b)
    d = mx - mn
    v = mx
    s = d * 255 // mx if mx else 0
    if d == 0:
        h = 0
    elif mx == r:
        h = (cdiv(60 * (g - b), d) + 360) % 360
    elif mx == g:
        h = cdiv(60 * (b - r), d) + 120
    else:
        h = cdiv(60 * (r - g), d) + 240
    return h, s, v


def hsv_to_rgb(h, s, v):
    s = max(0, min(255, s))
    v = max(0, min(255, v))
    h = ((h % 360) + 360) % 360
    f = (h % 60) * 255 // 60
    p = v * (255 - s) // 255
    q = v * (255 - s * f // 255) // 255
    t = v * (255 - s * (255 - f) // 255) // 255
    i = h // 60
    r, g, b = [(v, t, p), (q, v, p), (p, v, t), (p, q, v), (t, p, v), (v, p, q)][i if i < 5 else 5]
    return rgb5(r * 31 // 255, g * 31 // 255, b * 31 // 255)


def recolor_cluster(pal, members, target):
    th, ts, tv = rgb_to_hsv(target)
    maxV, maxS, lightS = 1, 1, 0
    for m in members:
        h, s, v = rgb_to_hsv(pal[m])
        if v > maxV:
            maxV, lightS = v, s
        maxS = max(maxS, s)
    for m in members:
        h, s, v = rgb_to_hsv(pal[m])
        pal[m] = hsv_to_rgb(th, ts if lightS < 80 else ts * s // maxS, v * tv // maxV)


def recolor(pal, cmap, look, P):
    skinP, colorP, hairP = P
    skin, hair, outfit, accent = look
    pal = list(pal)
    mem = lambda role: [i + 1 for i, c in enumerate(cmap) if c == role]
    m = mem('S')
    if skin and m:
        m.sort(key=lambda i: -rgb_to_hsv(pal[i])[2])
        n = len(m)
        for i, idx in enumerate(m):
            pal[idx] = skinP[skin][0 if n == 1 else (i * 2 + (n - 1) // 2) // (n - 1)]
    for role, val, table in (('H', hair, hairP), ('O', outfit, colorP), ('A', accent, colorP)):
        m = mem(role)
        if val and m:
            recolor_cluster(pal, m, table[val][0])
    return pal


LOOKS = [  # skin, hair, outfit, accent
    (5, 10, 2, 4),   # dark, pink, blue, yellow
    (1, 4, 1, 8),    # porcelain, blonde, red, black
    (3, 7, 3, 5),    # tan, silver, green, purple
]


def to_img(idx, pal16, scale):
    h, w = idx.shape
    im = np.full((h, w, 3), 236, np.uint8)
    for c in range(1, 16):
        v = pal16[c]
        im[idx == c] = ((v & 31) << 3, ((v >> 5) & 31) << 3, ((v >> 10) & 31) << 3)
    return Image.fromarray(im).resize((w * scale, h * scale), Image.NEAREST)


def main():
    dump, out = sys.argv[1], sys.argv[2]
    only = sys.argv[3] if len(sys.argv) > 3 else None
    os.makedirs(out, exist_ok=True)
    P = presets()
    maps = {}
    for line in open(os.path.join(HERE, 'colormaps.txt')):
        p = line.split()
        if len(p) == 3 and not line.startswith('#'):
            maps[(p[0], p[1])] = p[2]
    lines = [l for l in open(os.path.join(dump, 'index.txt')).read().split('\n') if l.strip()]
    tiles = []
    for l in lines:
        kind, cfg = l.split()[:2]
        if (kind, cfg) not in maps or (only and kind != only):
            continue
        _, _, frames, cols = load(dump, l)
        pal = [rgb5(int(r) >> 3, int(g) >> 3, int(b) >> 3) for r, g, b in cols]
        scale = 4 if kind == 'ow' else 2
        imgs = [to_img(frames[0], pal, scale)] + [to_img(frames[0], recolor(pal, maps[(kind, cfg)], lk, P), scale) for lk in LOOKS]
        w = sum(i.width for i in imgs) + 3 * len(imgs)
        cell = Image.new('RGB', (max(w, 150), imgs[0].height + 12), (255, 255, 255))
        d = ImageDraw.Draw(cell)
        d.text((1, 0), cfg[:24], fill=(0, 0, 0))
        x = 0
        for i in imgs:
            cell.paste(i, (x, 12))
            x += i.width + 3
        tiles.append(cell)
    if not tiles:
        print('nothing mapped yet')
        return
    per_row = max(1, 2200 // (max(t.width for t in tiles) + 6))
    cw = max(t.width for t in tiles) + 6
    ch = max(t.height for t in tiles) + 6
    page = 0
    rows_per_page = 5
    for start in range(0, len(tiles), per_row * rows_per_page):
        chunk = tiles[start:start + per_row * rows_per_page]
        rows = (len(chunk) + per_row - 1) // per_row
        sheet = Image.new('RGB', (per_row * cw, rows * ch), (160, 160, 170))
        for k, t in enumerate(chunk):
            sheet.paste(t, ((k % per_row) * cw, (k // per_row) * ch))
        sheet.save(os.path.join(out, 'check_%s_%02d.png' % (only or 'all', page)))
        page += 1
    print('%d sprites, %d sheets' % (len(tiles), page))


if __name__ == '__main__':
    main()
