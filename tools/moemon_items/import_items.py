#!/usr/bin/env python3
"""Moemon Kai: item icons from the user's art (items.zip, not in the repo).

Usage: import_items.py SRC_DIR [--preview out.png]
Writes graphics/items/icons/<name>.png (24x24, 16-color indexed, index 0 transparent) and
graphics/items/icon_palettes/<name>.pal for each item in ITEMS. Colors are the GBA's (5 bits a
channel); art wider than 24 loses the columns most like their neighbors (outlines stay whole),
and art with more than 15 colors is reduced (PIL median cut).
"""
import os
import sys

from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
SIZE = 24

# (output name, source file in SRC_DIR)
ITEMS = [
    ('quick_powder', 'QuickPowder.png'),
    ('eviolite', 'Eviolite.png'),
    ('old_gateau', 'Old Gateau.png'),
    ('casteliacone', 'Casteliacone.png'),
    ('lumiose_galette', 'Bag_Lumiose_Galette_Sprite.png'),
    ('shalour_sable', 'Bag_Shalour_Sable_Sprite.png'),
    ('big_malasada', 'Bag_Big_Malasada_Sprite.png'),
    ('pewter_crunchies', 'Bag_Pewter_Crunchies_Sprite.png'),
    ('rage_candy_bar', 'Rage Candy Bar.png'),
    ('candy_jar', 'Bag_Candy_Jar_Sprite.png'),
]


def gba(c):
    return tuple((v >> 3) << 3 for v in c[:3])


def load(path):
    im = Image.open(path).convert('RGBA')
    w, h = im.size
    px = [[None if im.getpixel((x, y))[3] < 128 else gba(im.getpixel((x, y))) for x in range(w)] for y in range(h)]
    return px, w, h


def col_cost(px, h, a, b):
    cost = 0
    for y in range(h):
        p, q = px[y][a], px[y][b]
        if p == q:
            continue
        if p is None or q is None:
            cost += 3 * 255
        else:
            cost += sum(abs(i - j) for i, j in zip(p, q))
    return cost


def narrow(px, w, h):
    # drop empty columns at the edges first, then the columns most like their left neighbor
    while w > SIZE and all(px[y][0] is None for y in range(h)):
        px = [row[1:] for row in px]
        w -= 1
    while w > SIZE and all(px[y][w - 1] is None for y in range(h)):
        px = [row[:-1] for row in px]
        w -= 1
    while w > SIZE:
        best = min(range(1, w), key=lambda x: col_cost(px, h, x - 1, x))
        px = [row[:best] + row[best + 1:] for row in px]
        w -= 1
    return px, w


def reduce_colors(px, w, h):
    colors = sorted({c for row in px for c in row if c is not None})
    if len(colors) <= 15:
        return px
    im = Image.new('RGB', (w, h))
    opaque = [(x, y) for y in range(h) for x in range(w) if px[y][x] is not None]
    for x, y in opaque:
        im.putpixel((x, y), px[y][x])
    # quantize only the opaque pixels: put them in a strip
    strip = Image.new('RGB', (len(opaque), 1))
    for i, (x, y) in enumerate(opaque):
        strip.putpixel((i, 0), px[y][x])
    q = strip.quantize(colors=15, method=Image.Quantize.MEDIANCUT)
    pal = q.getpalette()
    out = [row[:] for row in px]
    for i, (x, y) in enumerate(opaque):
        k = q.getpixel((i, 0))
        out[y][x] = gba(pal[k * 3:k * 3 + 3])
    return out


def to_indexed(px, w, h):
    canvas = [[None] * SIZE for _ in range(SIZE)]
    ox, oy = (SIZE - w) // 2, (SIZE - h) // 2
    for y in range(h):
        for x in range(w):
            if 0 <= y + oy < SIZE:
                canvas[y + oy][x + ox] = px[y][x]
    colors = []
    for row in canvas:
        for c in row:
            if c is not None and c not in colors:
                colors.append(c)
    assert len(colors) <= 15, len(colors)
    pal = [(0x70, 0xC0, 0xA0)] + colors + [(0, 0, 0)] * (15 - len(colors))
    im = Image.new('P', (SIZE, SIZE), 0)
    flat = [v for c in pal for v in c]
    im.putpalette(flat + [0] * (768 - len(flat)))
    for y in range(SIZE):
        for x in range(SIZE):
            c = canvas[y][x]
            im.putpixel((x, y), 0 if c is None else 1 + colors.index(c))
    return im, pal


def write_pal(path, pal):
    with open(path, 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n16\n')
        for c in pal:
            f.write('%d %d %d\n' % c)


def main():
    src = sys.argv[1]
    preview = sys.argv[sys.argv.index('--preview') + 1] if '--preview' in sys.argv else None
    sheet = Image.new('RGBA', (28 * len(ITEMS), 28), (240, 240, 232, 255))
    for i, (name, fn) in enumerate(ITEMS):
        px, w, h = load(os.path.join(src, fn))
        if h > SIZE:
            raise SystemExit('%s: taller than %d' % (fn, SIZE))
        px, w = narrow(px, w, h)
        px = reduce_colors(px, w, h)
        im, pal = to_indexed(px, w, h)
        im.save(os.path.join(ROOT, 'graphics', 'items', 'icons', name + '.png'), transparency=0)
        write_pal(os.path.join(ROOT, 'graphics', 'items', 'icon_palettes', name + '.pal'), pal)
        sheet.alpha_composite(im.convert('RGBA'), (28 * i + 2, 2))
    if preview:
        sheet.resize((sheet.width * 6, sheet.height * 6), Image.NEAREST).save(preview)


if __name__ == '__main__':
    main()
