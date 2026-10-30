#!/usr/bin/env python3
"""Draws graphics/types/fairy.png, the FAIRY move-type label, in the style of the other labels: PSYCHIC's frame in
palette 2's pinks, and the letters cut from FIRE (F, I, R), DRAGON (A) and FLYING (Y).

    python3 tools/moemon_gen58/draw_fairy_type.py
"""
import os
from PIL import Image

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
T = os.path.join(REPO, 'graphics', 'types')
TEXT = (14, 15)   # shadow, letter


def glyph(name, x0, x1):
    """Columns x0..x1 of a label's letters (the font's letters touch, so they are cut at known columns)."""
    im = Image.open(os.path.join(T, name + '.png'))
    return [[im.getpixel((c, y)) if im.getpixel((c, y)) in TEXT else None for y in range(4, 12)] for c in range(x0, x1 + 1)]


def main():
    word = [glyph('fire', 7, 11), glyph('dragon', 11, 15), glyph('fire', 12, 15), glyph('fire', 16, 20),
            glyph('flying', 12, 15)]   # F A I R Y
    base = Image.open(os.path.join(T, 'psychic.png'))
    out = base.copy()
    remap = {8: 12, 10: 11}
    for y in range(16):
        for x in range(32):
            p = base.getpixel((x, y))
            if p in TEXT:
                p = 8
            out.putpixel((x, y), remap.get(p, p))
    width = sum(len(g) for g in word)
    x = (32 - width) // 2
    for g in word:
        for col in g:
            for dy, p in enumerate(col):
                if p is not None:
                    out.putpixel((x, 4 + dy), p)
            x += 1
    out.save(os.path.join(T, 'fairy.png'))
    print('graphics/types/fairy.png')


if __name__ == '__main__':
    main()
