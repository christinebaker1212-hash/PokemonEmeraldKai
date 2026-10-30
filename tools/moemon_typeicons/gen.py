#!/usr/bin/env python3
"""Moemon Co-op: the battle type icons (OPTIONS > TYPE ICONS).

Writes src/mp/mp_typeicon_gfx.h: one 11x11 icon per type in the style of the
modern type chart (a colored rounded square with a white symbol), as GBA
BGR555 colors with bit 15 set where the icon is drawn (0 = see-through).
mp_typeicons.c draws them over the frame next to each healthbox, so they
use no sprites or palettes. ??? gets a question mark; FAIRY (the Gen 5-8 fork) a sparkle.

usage: tools/moemon_typeicons/gen.py  (from the repo root)
"""
import os

# type order = include/constants/pokemon.h (TYPE_NORMAL 0 .. TYPE_DARK 17)
# name, color (RGB), 7x7 symbol ('#' = white)
TYPES = [
    ("NORMAL", (0x9F, 0xA1, 0x9F), [
        "..###..",
        ".#...#.",
        "#.....#",
        "#.....#",
        "#.....#",
        ".#...#.",
        "..###..",
    ]),
    ("FIGHTING", (0xFF, 0x80, 0x00), [
        ".#.#.#.",
        "#######",
        "#######",
        "#######",
        ".######",
        "..####.",
        "..####.",
    ]),
    ("FLYING", (0x81, 0xB9, 0xEF), [
        "#......",
        "##.....",
        "###....",
        ".####..",
        ".######",
        "..#####",
        "....##.",
    ]),
    ("POISON", (0x91, 0x41, 0xCB), [
        ".#####.",
        "#######",
        "#..#..#",
        "#######",
        ".##.##.",
        ".#####.",
        ".#.#.#.",
    ]),
    ("GROUND", (0x91, 0x51, 0x21), [
        "...#...",
        "..###..",
        ".#####.",
        "#######",
        ".......",
        "#######",
        ".......",
    ]),
    ("ROCK", (0xAF, 0xA9, 0x81), [
        "..##...",
        ".####..",
        ".#####.",
        "###.###",
        "####.##",
        "#######",
        ".#####.",
    ]),
    ("BUG", (0x91, 0xA1, 0x19), [
        "#.....#",
        ".#...#.",
        ".#####.",
        "###.###",
        ".##.##.",
        "###.###",
        ".##.##.",
    ]),
    ("GHOST", (0x70, 0x41, 0x70), [
        ".#####.",
        "#######",
        "#.###.#",
        "#.###.#",
        "#######",
        "#######",
        "#.#.#.#",
    ]),
    ("STEEL", (0x60, 0xA1, 0xB8), [
        ".#.#.#.",
        "#######",
        ".##.##.",
        "##...##",
        ".##.##.",
        "#######",
        ".#.#.#.",
    ]),
    ("???", (0x68, 0xA0, 0x90), [
        ".####..",
        "##..##.",
        "....##.",
        "...##..",
        "..##...",
        ".......",
        "..##...",
    ]),
    ("FIRE", (0xE6, 0x28, 0x29), [
        "..#....",
        "..##...",
        ".####..",
        ".####.#",
        "#######",
        "###.###",
        ".#####.",
    ]),
    ("WATER", (0x29, 0x80, 0xEF), [
        "...#...",
        "...#...",
        "..###..",
        ".#####.",
        "#######",
        "#.#####",
        ".#####.",
    ]),
    ("GRASS", (0x3F, 0xA1, 0x29), [
        "....###",
        "..#####",
        ".####.#",
        ".###.##",
        ".##.##.",
        ".#.##..",
        "#......",
    ]),
    ("ELECTRIC", (0xFA, 0xC0, 0x00), [
        "...###.",
        "..###..",
        ".###...",
        ".#####.",
        "...###.",
        "..###..",
        ".##....",
    ]),
    ("PSYCHIC", (0xEF, 0x41, 0x79), [
        ".......",
        "..###..",
        ".#...#.",
        "#..#..#",
        ".#...#.",
        "..###..",
        ".......",
    ]),
    ("ICE", (0x3D, 0xCE, 0xF3), [
        "...#...",
        ".#.#.#.",
        "..###..",
        "#######",
        "..###..",
        ".#.#.#.",
        "...#...",
    ]),
    ("DRAGON", (0x50, 0x60, 0xE1), [
        "#.....#",
        "##...##",
        ".#####.",
        "##.#.##",
        ".#####.",
        "..###..",
        "..#.#..",
    ]),
    ("DARK", (0x62, 0x4D, 0x4E), [
        "..####.",
        ".###...",
        "###....",
        "###....",
        "###....",
        ".###...",
        "..####.",
    ]),
    # Moemon Kai (the Gen 5-8 fork): FAIRY, a four-pointed sparkle
    ("FAIRY", (0xEF, 0x70, 0xEF), [
        "...#...",
        "...#...",
        "..###..",
        "#######",
        "..###..",
        "...#...",
        "...#...",
    ]),
]

SIZE = 11


def bgr555(rgb):
    r, g, b = (c >> 3 for c in rgb)
    return 0x8000 | (b << 10) | (g << 5) | r


def shade(rgb, k):
    return tuple(max(0, min(255, int(c * k))) for c in rgb)


def icon(color, sym):
    px = [[0] * SIZE for _ in range(SIZE)]
    fill, rim, hi = bgr555(color), bgr555(shade(color, 0.55)), bgr555(tuple(min(255, c + 40) for c in color))
    white = bgr555((0xFF, 0xFF, 0xFF))
    for y in range(SIZE):
        for x in range(SIZE):
            corner = (x in (0, SIZE - 1)) and (y in (0, SIZE - 1))
            if corner:
                continue
            edge = x in (0, SIZE - 1) or y in (0, SIZE - 1)
            px[y][x] = rim if edge else fill
    for y, row in enumerate(sym):   # the symbol inside a 1-pixel margin
        for x, c in enumerate(row):
            if c == '#':
                px[y + 2][x + 2] = white
    return px


def main():
    out = []
    out.append("// Generated by tools/moemon_typeicons/gen.py - do not edit.")
    out.append("// %dx%d type icons, GBA BGR555 with bit 15 set where drawn (0 = see-through), TYPE_* order" % (SIZE, SIZE))
    out.append("#define MP_TYPEICON_SIZE %d" % SIZE)
    out.append("static const u16 sTypeIcons[%d][%d * %d] = {" % (len(TYPES), SIZE, SIZE))
    for name, color, sym in TYPES:
        assert len(sym) == 7, name
        px = icon(color, sym)
        out.append("    // %s" % name)
        out.append("    {")
        for row in px:
            out.append("        " + ", ".join("0x%04X" % v for v in row) + ",")
        out.append("    },")
    out.append("};")
    path = os.path.join("src", "mp", "mp_typeicon_gfx.h")
    with open(path, "w") as f:
        f.write("\n".join(out) + "\n")
    # preview: tools/moemon_typeicons/preview.png (needs PIL; optional)
    try:
        from PIL import Image
        scale, cols = 8, 6
        rows = (len(TYPES) + cols - 1) // cols
        img = Image.new("RGB", (cols * (SIZE + 3) * scale, rows * (SIZE + 3) * scale), (232, 240, 216))
        for i, (name, color, sym) in enumerate(TYPES):
            px = icon(color, sym)
            ox, oy = (i % cols) * (SIZE + 3) + 1, (i // cols) * (SIZE + 3) + 1
            for y in range(SIZE):
                for x in range(SIZE):
                    v = px[y][x]
                    if v:
                        c = ((v & 31) << 3, ((v >> 5) & 31) << 3, ((v >> 10) & 31) << 3)
                        for dy in range(scale):
                            for dx in range(scale):
                                img.putpixel(((ox + x) * scale + dx, (oy + y) * scale + dy), c)
        img.save(os.environ.get("PREVIEW", "/dev/null.png") if os.environ.get("PREVIEW") else os.path.join("tools", "moemon_typeicons", "preview.png"))
    except ImportError:
        pass


if __name__ == "__main__":
    main()
