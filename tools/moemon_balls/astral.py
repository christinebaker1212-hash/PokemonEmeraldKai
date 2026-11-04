#!/usr/bin/env python3
"""The ASTRAL BALL's art from the user's pictures (docs/user_images 69 and 70).

    python3 tools/moemon_balls/astral.py      (from anywhere)

69.png is a battle sheet in the HEARTGOLD / SOULSILVER sheet's column layout (16 wide: rows 0-7 the ball spinning,
row 8 squashed at y 128, rows 9-10 the open ball at y 144-167), so it becomes graphics/balls/astral.png the same way
hgss_balls.py makes the other balls (eleven 16x32 frames: closed, open, squashed, spin x8). 70.png (32x32) becomes the
BAG icon graphics/items/icons/moemon_astral_ball.png (its 24x24 middle, as convert.py does) and its palette.
Both are cut to 15 colors (index 0 clear) on GBA colors (5 bits a channel).
"""
import os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, '..', '..'))
SHEET = os.path.join(REPO, 'docs', 'user_images', '69.png')
ICON = os.path.join(REPO, 'docs', 'user_images', '70.png')
FRAMES, FH = 11, 32
REST, SQUISH, OPEN = (0, 16, 8), (128, 16, 7), (144, 24, 0)   # (source top, height, y in the 16x32 frame)
SPIN = [(r * 16, 16, 8) for r in range(8)]


def quantize(pixels, n=15):
    """{(x, y): rgb} -> {(x, y): rgb} with at most n colors, GBA-rounded"""
    gba = {p: tuple(v & 0xF8 for v in c) for p, c in pixels.items()}
    if len(set(gba.values())) <= n:
        return gba
    keys = list(gba)
    strip = Image.new('RGB', (len(keys), 1))
    for i, k in enumerate(keys):
        strip.putpixel((i, 0), gba[k])
    q = strip.quantize(colors=n, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = q.getpalette()
    return {k: tuple(v & 0xF8 for v in pal[q.getpixel((i, 0)) * 3:q.getpixel((i, 0)) * 3 + 3]) for i, k in enumerate(keys)}


def write(pixels, size, path, pal_path=None):
    def lum(c):
        return c[0] * 3 + c[1] * 6 + c[2]
    cols = sorted(set(pixels.values()), key=lum)
    pal = [(0, 0, 0)] + cols + [(0, 0, 0)] * (15 - len(cols))
    out = Image.new('P', size, 0)
    out.putpalette([v for c in pal for v in c])
    for (x, y), c in pixels.items():
        out.putpixel((x, y), pal.index(c))
    out.save(path, transparency=0)
    if pal_path:
        with open(pal_path, 'w', newline='\r\n') as f:
            f.write('JASC-PAL\n0100\n16\n')
            for c in pal:
                f.write('%d %d %d\n' % c)
    print('%s: %d colors' % (os.path.relpath(path, REPO), len(cols)))


def main():
    src = Image.open(SHEET).convert('RGBA')
    clear = Image.open(SHEET).info.get('transparency')
    raw = Image.open(SHEET)
    canvas = {}
    for f, (top, h, dy) in enumerate([REST, OPEN, SQUISH] + SPIN):
        for y in range(h):
            for x in range(16):
                if raw.getpixel((x, top + y)) == clear or src.getpixel((x, top + y))[3] == 0:
                    continue
                canvas[(x, f * FH + dy + y)] = src.getpixel((x, top + y))[:3]
    write(quantize(canvas), (16, FRAMES * FH), os.path.join(REPO, 'graphics', 'balls', 'astral.png'))

    icon = Image.open(ICON)
    iclear = icon.info.get('transparency')
    rgba = icon.convert('RGBA')
    opaque = {(x, y): rgba.getpixel((x, y))[:3] for y in range(32) for x in range(32)
              if icon.getpixel((x, y)) != iclear and rgba.getpixel((x, y))[3] >= 128}
    xs = [p[0] for p in opaque]
    ys = [p[1] for p in opaque]
    assert min(xs) >= 4 and max(xs) < 28 and min(ys) >= 4 and max(ys) < 28, (min(xs), max(xs), min(ys), max(ys))
    crop = {(x - 4, y - 4): c for (x, y), c in opaque.items()}
    write(quantize(crop), (24, 24), os.path.join(REPO, 'graphics', 'items', 'icons', 'moemon_astral_ball.png'),
          os.path.join(REPO, 'graphics', 'items', 'icon_palettes', 'moemon_astral_ball.pal'))


if __name__ == '__main__':
    main()
