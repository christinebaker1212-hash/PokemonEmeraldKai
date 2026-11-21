#!/usr/bin/env python3
"""Moemon VMU pet: check the colour file icon the pet drew into its own header (pet/icon.inc) against what it should
be, pixel for pixel: an Egg's design in its colours (eggs.py), a hatched MOeMON's portrait from the PICTURES file
(gen_icons.py) or, without that file, its LCD sprite.

usage: icon_check.py IMAGE [OUT.png]      IMAGE: a VMU image (vmufs.py) holding the pet (MOEKANTO); prints what the icon
                                         shows; exits 1 if it isn't what the header's marks ($25e stage, $25f MOeMON) say
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import vmufs  # noqa: E402


def icon_of(game):
    pal = struct.unpack_from('<16H', game, 0x260)
    px = []
    for b in game[0x280:0x480]:
        px += [b >> 4, b & 15]
    return pal, px


def rgb(c):
    return ((c >> 8 & 15) * 17, (c >> 4 & 15) * 17, (c & 15) * 17, (c >> 12 & 15) * 17)


def expected(img, stage, sp):
    """(palette, pixels) the icon should have, as RGBA per pixel."""
    import eggs
    import gen_edition as ge
    dex = sp + 1
    nums = ge.species_numbers()
    info = ge.infos(nums)
    if stage != 2:
        kind, phase, mirror = eggs.design(dex, info[dex]['types'])
        ol, sh, pt = eggs.masks(eggs.ICON_W, eggs.ICON_H, kind, phase)
        cols = [(0, 0, 0, 0)] + [rgb(eggs.to4444(c)[0]) for c in eggs.palette(*eggs.body_colours(dex))]
        grid = [[0] * 32 for _ in range(32)]
        for y in range(eggs.ICON_H):
            for x in range(eggs.ICON_W):
                v = 1 if ol[y][x] else 3 if pt[y][x] else 2 if sh[y][x] else 0
                grid[y + 2][x + 2] = v
        if mirror:
            grid = [r[::-1] for r in grid]
        return 'EGG of #%d' % dex, [cols[v] for r in grid for v in r]
    pics = None
    for f in img.files():
        if f['type'] == 'DATA':
            d = img.read(f['name'])
            if d[0x280:0x289] == b'MOEPICS\x01\x01':
                pics = d
    if pics:
        pal = [rgb(c) for c in struct.unpack_from('<4H', pics, 0x400 + sp * 8)]
        at = (5 + sp // 2) * 512 + (sp % 2) * 256
        out = []
        for b in pics[at:at + 256]:
            out += [pal[b >> 6], pal[b >> 4 & 3], pal[b >> 2 & 3], pal[b & 3]]
        return 'PORTRAIT of #%d' % dex, out
    from onebit import onebit
    on = onebit(dex, 24)
    h, w = on.shape
    left = (24 - w) // 2
    outline = rgb(eggs.to4444(eggs.palette(*eggs.body_colours(dex))[0])[0])
    grid = [[(0, 0, 0, 0)] * 32 for _ in range(32)]
    for y in range(h):
        for x in range(w):
            if on[y, x]:
                grid[28 - h + y][4 + left + x] = outline
    return 'SPRITE of #%d' % dex, [c for r in grid for c in r]


def main():
    img = vmufs.Image(open(sys.argv[1], 'rb').read())
    game = img.read('MOEKANTO')
    stage, sp = game[0x25e], game[0x25f]
    pal, px = icon_of(game)
    got = [rgb(pal[v]) if (pal[v] >> 12) else (0, 0, 0, 0) for v in px]
    what, want = expected(img, stage, sp)
    want = [c if c[3] else (0, 0, 0, 0) for c in want]
    if len(sys.argv) > 2:
        from PIL import Image
        im = Image.new('RGBA', (32 * 2 + 4, 32))
        a = Image.new('RGBA', (32, 32))
        a.putdata(got)
        b = Image.new('RGBA', (32, 32))
        b.putdata(want)
        im.paste(a, (0, 0))
        im.paste(b, (36, 0))
        im.resize((im.width * 6, im.height * 6), Image.NEAREST).save(sys.argv[2])
    bad = sum(1 for g, w in zip(got, want) if g != w)
    print('%s: %s' % (what, 'the icon matches' if not bad else '%d pixels differ' % bad))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
