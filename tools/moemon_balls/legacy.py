#!/usr/bin/env python3
"""Moemon Co-op: GENESIS / EPOCH BALL art from the supplied sheets (legacy_src/*.png).

Each sheet holds a large ball (18x18, at 1,2) and a small ball (12x12, at 30,5)
in four colors. The pixels are kept as they are (colors rounded to the GBA's 5
bits a channel), only padded:
  graphics/items/icons/moemon_<name>_ball.png + icon_palettes/*.pal   the 24x24 item icon
  graphics/moemon/legacy_balls/<name>_ball_small.png                  the 16x16 ball (summary, battle)

usage: tools/moemon_balls/legacy.py  (from the repo root)
"""
import os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
BALLS = ["genesis", "epoch"]
LARGE = (1, 2, 18, 18)   # x, y, w, h in the sheet
SMALL = (30, 5, 12, 12)


def q5(c):
    return tuple(v & 0xF8 for v in c[:3])


def cut(sheet, box):
    x0, y0, w, h = box
    return [[sheet.getpixel((x0 + x, y0 + y)) for x in range(w)] for y in range(h)]


def save_indexed(path, rows, size, at, palette):
    out = Image.new("P", (size, size), 0)
    flat = []
    for c in palette:
        flat += list(c)
    out.putpalette(flat + [0] * (768 - len(flat)))
    for y, row in enumerate(rows):
        for x, px in enumerate(row):
            if px[3] >= 128:
                out.putpixel((at[0] + x, at[1] + y), palette.index(q5(px)))
    out.save(path, bits=4)


def main():
    root = os.path.normpath(os.path.join(HERE, "..", ".."))
    os.makedirs(os.path.join(root, "graphics", "moemon", "legacy_balls"), exist_ok=True)
    for name in BALLS:
        sheet = Image.open(os.path.join(HERE, "legacy_src", name + "_ball.png")).convert("RGBA")
        large, small = cut(sheet, LARGE), cut(sheet, SMALL)
        cols = []
        for rows in (large, small):
            for row in rows:
                for px in row:
                    if px[3] >= 128 and q5(px) not in cols:
                        cols.append(q5(px))
        palette = [(180, 180, 180)] + cols
        palette += [(0, 0, 0)] * (16 - len(palette))
        base = "moemon_%s_ball" % name
        save_indexed(os.path.join(root, "graphics", "items", "icons", base + ".png"), large, 24, (3, 3), palette)
        with open(os.path.join(root, "graphics", "items", "icon_palettes", base + ".pal"), "w", newline="\r\n") as f:
            f.write("JASC-PAL\n0100\n16\n")
            for c in palette:
                f.write("%d %d %d\n" % c)
        save_indexed(os.path.join(root, "graphics", "moemon", "legacy_balls", name + "_ball_small.png"), small, 16, (2, 2), palette)
        print(name, "colors", cols)


if __name__ == "__main__":
    main()
