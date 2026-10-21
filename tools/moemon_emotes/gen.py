#!/usr/bin/env python3
"""Moemon Co-op: the extra emote icons (EMOTES 2-4 in the quick pick).

Writes src/mp/mp_emote_gfx.h: for each emote one or two 16x16 4bpp frames in the
style of the game's !/?/heart icons (white rounded box, black inner border, the
symbol inside) and the 10x10 symbol alone for battle speech bubbles. Colors are
the co-op overlay palette (mp_emote.c sBubblePal): 1 white, 3 light grey,
4 dark grey, 10 red, 11 blue, 12 green, 13 yellow, 14 black, 15 pink.

usage: tools/moemon_emotes/gen.py  (from the repo root)
"""
import os

COL = {'.': 1, 'w': 1, 'l': 3, 'd': 4, 'r': 10, 'b': 11, 'g': 12, 'y': 13, 'k': 14, 'p': 15}

# name, frames (10x10 symbols; a second frame animates: the two alternate)
EMOTES = [
    ("ANGRY", [[
        "..........",
        ".rr....rr.",
        ".r.r..r.r.",
        "..rr..rr..",
        "..........",
        "..........",
        "..rr..rr..",
        ".r.r..r.r.",
        ".rr....rr.",
        "..........",
    ], [
        "..........",
        "..........",
        ".rr....rr.",
        ".r.r..r.r.",
        "..rr..rr..",
        "..rr..rr..",
        ".r.r..r.r.",
        ".rr....rr.",
        "..........",
        "..........",
    ]]),
    ("SWEAT", [[
        "......b...",
        ".....bb...",
        "....bbbb..",
        "...bbbbbb.",
        "...bwbbbb.",
        "...bwbbbb.",
        "...bbbbbb.",
        "....bbbb..",
        "..........",
        "..........",
    ], [
        "..........",
        "......b...",
        ".....bb...",
        "....bbbb..",
        "...bbbbbb.",
        "...bwbbbb.",
        "...bwbbbb.",
        "...bbbbbb.",
        "....bbbb..",
        "..........",
    ]]),
    ("SLEEPY", [[
        "....kkkkk.",
        ".......k..",
        "......k...",
        ".....k....",
        "....kkkkk.",
        "..........",
        "kkk.......",
        "..k.......",
        ".k........",
        "kkk.......",
    ], [
        "...kkkkk..",
        "......k...",
        ".....k....",
        "....k.....",
        "...kkkkk..",
        "..........",
        ".kkk......",
        "...k......",
        "..k.......",
        ".kkk......",
    ]]),
    ("LAUGH", [[
        "..kkkkkk..",
        ".kyyyyyyk.",
        "kyykyykyyk",
        "kykykkykyk",
        "kyyyyyyyyk",
        "kykkkkkkyk",
        "kyykrrkyyk",
        ".kyykkyyk.",
        "..kkkkkk..",
        "..........",
    ]]),
    ("IDEA", [[
        "...kkkk...",
        "..kyyyyk..",
        ".kyywyyyk.",
        ".kywyyyyk.",
        ".kyyyyyyk.",
        "..kyyyyk..",
        "...kyyk...",
        "...kllk...",
        "...kddk...",
        "....kk....",
    ]]),
    ("SPARKLE", [[
        "....y.....",
        "....y.....",
        "...yyy....",
        "yyyyyyyyy.",
        "..yyyyy...",
        "...yyy....",
        "..yy.yy...",
        "..y...y..y",
        "........yy",
        ".........y",
    ], [
        "y...y.....",
        "....y.....",
        "...yyy....",
        ".yyyyyyy..",
        "..yyyyy...",
        "...yyy....",
        "..yy.yy...",
        "..y...y...",
        "..........",
        "........y.",
    ]]),
    ("SAD", [[
        "..kkkkkk..",
        ".kyyyyyyk.",
        "kyyyyyyyyk",
        "kykkyykkyk",
        "kbyyyyyybk",
        "kbyykkyybk",
        "kyykyykyyk",
        ".kyyyyyyk.",
        "..kkkkkk..",
        "..........",
    ]]),
    ("DOTS", [[
        "..........",
        "..........",
        "..........",
        "..........",
        ".kk.kk.kk.",
        ".kk.kk.kk.",
        "..........",
        "..........",
        "..........",
        "..........",
    ], [
        "..........",
        "..........",
        "..........",
        "..........",
        ".kk.kk....",
        ".kk.kk....",
        "..........",
        "..........",
        "..........",
        "..........",
    ]]),
    ("FIRE", [[
        "....r.....",
        "...rr.....",
        "...rrr..r.",
        "..rrrr.rr.",
        "..rryrrrr.",
        ".rryyyrrr.",
        ".ryyyyyrr.",
        ".ryyyyyyr.",
        ".rryyyyrr.",
        "..rrrrrr..",
    ], [
        ".....r....",
        ".....rr...",
        ".r..rrr...",
        ".rr.rrrr..",
        ".rrrryrr..",
        ".rrryyyrr.",
        ".rryyyyyr.",
        ".ryyyyyyr.",
        ".rryyyyrr.",
        "..rrrrrr..",
    ]]),
    ("CROWN", [[
        "..........",
        ".k..k...k.",
        ".kk.kk.kk.",
        ".kykyykyk.",
        ".kyyyyyyk.",
        ".kyryyryk.",
        ".kyyyyyyk.",
        ".kkkkkkkk.",
        "..........",
        "..........",
    ]]),
    ("BALL", [[
        "...kkkk...",
        ".kkrrrrkk.",
        ".krwrrrrk.",
        "krrrkkrrrk",
        "kkkk..kkkk",
        "k..k..k..k",
        "k...kk...k",
        ".k......k.",
        ".kk....kk.",
        "...kkkk...",
    ]]),
    ("SKULL", [[
        "..kkkkkk..",
        ".k......k.",
        "k........k",
        "k.kk..kk.k",
        "k.kk..kk.k",
        "k........k",
        ".k..kk..k.",
        "..k....k..",
        "..k.k.kk..",
        "...kkkk...",
    ]]),
]

# The box around the symbol (the game's icons): white ring, black border, white inside
BOX = [
    "................",
    "..wwwwwwwwwwww..",
    ".wwkkkkkkkkkkww.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wkwwwwwwwwwwkw.",
    ".wwkkkkkkkkkkww.",
    "..wwwwwwwwwwww..",
    "................",
]


def frame16(symbol):
    px = [[0 if c == '.' else COL[c] for c in row] for row in BOX]
    for y, row in enumerate(symbol):
        assert len(row) == 10, row
        for x, c in enumerate(row):
            if c != '.':
                px[3 + y][3 + x] = COL[c]
    return px


def to4bpp(px):
    # 16x16 = 2x2 tiles, 1D mapping: top-left, top-right, bottom-left, bottom-right
    out = []
    for ty in range(2):
        for tx in range(2):
            for y in range(8):
                for x in range(0, 8, 2):
                    a = px[ty * 8 + y][tx * 8 + x]
                    b = px[ty * 8 + y][tx * 8 + x + 1]
                    out.append(a | (b << 4))
    return out


def main():
    root = os.path.join(os.path.dirname(__file__), '..', '..')
    lines = ["// Generated by tools/moemon_emotes/gen.py - do not edit",
             "// The extra emotes: 16x16 4bpp frames (2 per emote; the second animates, or repeats the first)",
             "// and 10x10 symbols for battle bubbles (overlay palette indices, 1 = white = skip).",
             "#define MP_EXTRA_EMOTE_COUNT %d" % len(EMOTES), ""]
    lines.append("static const char *const sExtraEmoteNames[MP_EXTRA_EMOTE_COUNT] = {%s};" %
                 ", ".join('"%s"' % n for n, _ in EMOTES))
    lines.append("static const u8 sExtraEmoteFrameCount[MP_EXTRA_EMOTE_COUNT] = {%s};" %
                 ", ".join(str(len(f)) for _, f in EMOTES))
    lines.append("static const u8 sExtraEmoteGfx[MP_EXTRA_EMOTE_COUNT][2][0x80] = {")
    for name, frames in EMOTES:
        fs = frames + ([frames[0]] if len(frames) == 1 else [])
        lines.append("    {   // %s" % name)
        for f in fs:
            data = to4bpp(frame16(f))
            lines.append("        {" + ", ".join("0x%02X" % b for b in data) + "},")
        lines.append("    },")
    lines.append("};")
    lines.append("static const u8 sExtraEmoteBubble[MP_EXTRA_EMOTE_COUNT][10][10] = {")
    for name, frames in EMOTES:
        rows = ", ".join("{" + ",".join(str(0 if c == '.' else COL[c]) for c in r) + "}" for r in frames[0])
        lines.append("    {%s},   // %s" % (rows, name))
    lines.append("};")
    path = os.path.join(root, 'src', 'mp', 'mp_emote_gfx.h')
    with open(path, 'w') as f:
        f.write("\n".join(lines) + "\n")
    # A preview sheet (both frames, 4x) for eyeballing
    try:
        from PIL import Image
        pal = {0: (112, 160, 112), 1: (255, 255, 255), 3: (165, 165, 173), 4: (74, 74, 90), 10: (255, 66, 66),
               11: (66, 132, 255), 12: (49, 214, 49), 13: (255, 222, 33), 14: (16, 16, 33), 15: (255, 165, 181)}
        im = Image.new('RGB', (len(EMOTES) * 18, 36), (112, 160, 112))
        for i, (_, frames) in enumerate(EMOTES):
            for j, f in enumerate(frames + ([frames[0]] if len(frames) == 1 else [])):
                px = frame16(f)
                for y in range(16):
                    for x in range(16):
                        if px[y][x]:
                            im.putpixel((i * 18 + 1 + x, j * 18 + 1 + y), pal[px[y][x]])
        im = im.resize((im.width * 4, im.height * 4), Image.NEAREST)
        im.save(os.environ.get('PREVIEW', '/dev/null') if os.environ.get('PREVIEW') else os.devnull, 'PNG')
    except ImportError:
        pass


if __name__ == '__main__':
    main()
