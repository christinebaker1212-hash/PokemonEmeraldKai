#!/usr/bin/env python3
"""POKé BALL decorations: 8x8 emblems drawn on a Moemon's ball (mp_ballstyle.c).

    decorations.py                 writes src/mp/mp_balldeco_data.h
    decorations.py --preview OUT   also renders a review sheet (each emblem on red / blue / premier balls)

Art is drawn here as text, one character per pixel ('.' = transparent), in the
colors of PALETTE below (one shared 16-color sprite palette). Letters and digits
come from a 5x6 font and get a dark outline.
"""
import os, sys

def rgb5(r, g, b):
    return (r, g, b)

# key -> (palette index, GBA RGB 0-31)
PALETTE = {
    '.': (0, (0, 0, 0)),
    'W': (1, (31, 31, 31)),   # white
    'K': (2, (3, 3, 6)),      # outline
    'R': (3, (29, 5, 4)),     # red
    'O': (4, (31, 17, 3)),    # orange
    'Y': (5, (31, 27, 5)),    # yellow
    'G': (6, (9, 25, 7)),     # green
    'g': (7, (3, 14, 5)),     # dark green
    'B': (8, (5, 11, 29)),    # blue
    'b': (9, (14, 24, 31)),   # light blue
    'P': (10, (18, 7, 26)),   # purple
    'p': (11, (31, 13, 22)),  # pink
    'N': (12, (17, 10, 4)),   # brown
    'S': (13, (14, 14, 17)),  # gray
    'L': (14, (24, 24, 27)),  # light gray
    'C': (15, (5, 24, 26)),   # cyan
}

# name, art (8 rows of 8)
SHAPES = [
    ("STAR", """
...KK...
...KYK..
KKKYYKKK
KYYYYYYK
.KYYYYK.
.KYKKYK.
KYK..KYK
KK....KK"""),
    ("HEART", """
.KK..KK.
KppKKppK
KpWppppK
KppppppK
.KppppK.
..KppK..
...KK...
........"""),
    ("CIRCLE", """
..KKKK..
.KWWWWK.
KWWKKWWK
KWK..KWK
KWK..KWK
KWWKKWWK
.KWWWWK.
..KKKK.."""),
    ("DIAMOND", """
...KK...
..KbbK..
.KbWbbK.
KbWbbbbK
KbbbbBbK
.KbbBbK.
..KBBK..
...KK..."""),
    ("TRIANGLE", """
........
...KK...
..KGGK..
..KWGK..
.KGGGGK.
.KGGGGK.
KGGGGGGK
KKKKKKKK"""),
    ("SQUARE", """
KKKKKKKK
KOOOOOOK
KOWOOOOK
KOOOOOOK
KOOOOOOK
KOOOOOOK
KOOOOOOK
KKKKKKKK"""),
    ("CROSS", """
..KKKK..
..KRRK..
KKKRRKKK
KRRRRRRK
KRRRRRRK
KKKRRKKK
..KRRK..
..KKKK.."""),
    ("SPARKLE", """
...KK...
...KK...
..KWWK..
KKWWWWKK
KKWWWWKK
..KWWK..
...KK...
...KK..."""),
    ("RING", """
..KKKK..
.KYYYYK.
KYKKKKYK
KYK..KYK
KYK..KYK
KYKKKKYK
.KYYYYK.
..KKKK.."""),
]

# The 17 types
ELEMENTS = [
    ("NORMAL", """
..KKKK..
.KLLLLK.
KLWLLLLK
KLLLLLLK
KLLLLLLK
KLLLLSLK
.KLLSSK.
..KKKK.."""),
    ("FIRE", """
...K....
..KRK...
..KROK..
.KROROK.
KROYYORK
KROYWYRK
.KRYYRK.
..KKKK.."""),
    ("WATER", """
...KK...
...KbK..
..KbBK..
.KbBBBK.
KbWBBBBK
KbWBBBBK
.KBBBBK.
..KKKK.."""),
    ("GRASS", """
....KKK.
..KKGGK.
.KGGGgK.
KGGWGgK.
KGGgGK..
KGgGGK..
.KgKK...
KK......"""),
    ("ELECTRIC", """
...KKKK.
..KYYYK.
.KYYYK..
KYYYYKK.
KKKYYYK.
..KYYK..
.KYYK...
.KKK...."""),
    ("ICE", """
K..KK..K
.KKbbKK.
.KbWWbK.
KbWbbWbK
KbWbbWbK
.KbWWbK.
.KKbbKK.
K..KK..K"""),
    ("FIGHTING", """
.KKKKKK.
KOOOOOOK
KOWOOOOK
KOOOOOOK
.KKKKOOK
.KOOOOOK
.KOOOOK.
..KKKK.."""),
    ("POISON", """
.....KK.
....KPPK
.KK.KPPK
KPPK.KK.
KPWPK...
KPPPPK..
.KPPPK..
..KKK..."""),
    ("GROUND", """
........
...KK...
..KNNK..
..KNNNK.
.KNNYNNK
KNNNNNNK
KNNNNNNK
KKKKKKKK"""),
    ("FLYING", """
........
KK......
KbKK....
KbbbKKK.
.KbWbbbK
.KbbbbbK
..KKbbK.
....KK.."""),
    ("PSYCHIC", """
..KKKK..
.KppppK.
KppKKppK
KpK.KKpK
KpK.KpK.
KppK.KK.
.KppKK..
..KKK..."""),
    ("BUG", """
..K..K..
...KK...
.KRKKRK.
KRRKRRRK
KRKRKRRK
KRRKRKRK
.KRKRRK.
..KKKK.."""),
    ("ROCK", """
..KKK...
.KNSSKK.
KNSSSSSK
KSSWSSSK
KSSSSSNK
KNSSSNNK
.KNNNNK.
..KKKK.."""),
    ("GHOST", """
..KKKK..
.KPPPPK.
KPWPPWPK
KPKPPKPK
KPPPPPPK
KPPPPPPK
KPKPPKPK
.K.KK.K."""),
    ("DRAGON", """
KK...KK.
KBK.KBK.
.KBKBK..
.KBBBBK.
KBWBBWBK
KBBBBBBK
.KBKKBK.
..K..K.."""),
    ("DARK", """
..KKKK..
.KSSKKK.
KSSSKKKK
KSWSKKKK
KSSSKKKK
KSSSKKKK
.KSSKKK.
..KKKK.."""),
    ("STEEL", """
...KK...
.KKLLKK.
.KLLLLK.
KLLKKLLK
KLLKKLLK
.KLLLLK.
.KKLLKK.
...KK..."""),
]

SYMBOLS = [
    ("NOTE", """
...KKKK.
...KCCK.
...KKCK.
...K.KK.
.KKK.K..
KCCCKK..
KCCCK...
.KKK...."""),
    ("CROWN", """
K..KK..K
KYKYYKYK
KYYYYYYK
KYRYYRYK
KYYYYYYK
KKKKKKKK
........
........"""),
    ("PAW", """
.KK..KK.
KNNKKNNK
.KK..KK.
K.KKKK.K
.KNNNNK.
KNNNNNNK
KNNNNNNK
.KKKKKK."""),
    ("FLOWER", """
..KKKK..
.KpKKpK.
KppKKppK
KKKYYKKK
KKKYYKKK
KppKKppK
.KpKKpK.
..KKKK.."""),
    ("SUN", """
K..KK..K
.KKYYKK.
.KYYYYK.
KYYWYYYK
KYYYYYYK
.KYYYYK.
.KKYYKK.
K..KK..K"""),
    ("MOON", """
..KKKK..
.KYYKK..
KYYK....
KYK.....
KYK.....
KYYK....
.KYYKK..
..KKKK.."""),
    ("CLOVER", """
.KK..KK.
KGGKKGGK
KGGGGGGK
.KGGGGK.
KGGGGGGK
KGGKKGGK
.KK.KK..
...K...."""),
    ("SMILE", """
..KKKK..
.KYYYYK.
KYKYYKYK
KYYYYYYK
KYKYYKYK
KYYKKYYK
.KYYYYK.
..KKKK.."""),
    ("POKEBALL", """
..KKKK..
.KRRRRK.
KRWRRRRK
KKKWWKKK
KLLWWLLK
KLLLLLLK
.KLLLLK.
..KKKK.."""),
]

FONT = {  # 5 wide x 6 tall
    'A': [".###.", "#...#", "#...#", "#####", "#...#", "#...#"],
    'B': ["####.", "#...#", "####.", "#...#", "#...#", "####."],
    'C': [".####", "#....", "#....", "#....", "#....", ".####"],
    'D': ["####.", "#...#", "#...#", "#...#", "#...#", "####."],
    'E': ["#####", "#....", "####.", "#....", "#....", "#####"],
    'F': ["#####", "#....", "####.", "#....", "#....", "#...."],
    'G': [".####", "#....", "#.###", "#...#", "#...#", ".###."],
    'H': ["#...#", "#...#", "#####", "#...#", "#...#", "#...#"],
    'I': ["#####", "..#..", "..#..", "..#..", "..#..", "#####"],
    'J': ["..###", "...#.", "...#.", "...#.", "#..#.", ".##.."],
    'K': ["#...#", "#..#.", "###..", "#..#.", "#...#", "#...#"],
    'L': ["#....", "#....", "#....", "#....", "#....", "#####"],
    'M': ["#...#", "##.##", "#.#.#", "#...#", "#...#", "#...#"],
    'N': ["#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#"],
    'O': [".###.", "#...#", "#...#", "#...#", "#...#", ".###."],
    'P': ["####.", "#...#", "#...#", "####.", "#....", "#...."],
    'Q': [".###.", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"],
    'R': ["####.", "#...#", "#...#", "####.", "#..#.", "#...#"],
    'S': [".####", "#....", ".###.", "....#", "....#", "####."],
    'T': ["#####", "..#..", "..#..", "..#..", "..#..", "..#.."],
    'U': ["#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
    'V': ["#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."],
    'W': ["#...#", "#...#", "#...#", "#.#.#", "##.##", "#...#"],
    'X': ["#...#", ".#.#.", "..#..", "..#..", ".#.#.", "#...#"],
    'Y': ["#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."],
    'Z': ["#####", "...#.", "..#..", ".#...", "#....", "#####"],
    '0': [".###.", "#..##", "#.#.#", "##..#", "#...#", ".###."],
    '1': ["..#..", ".##..", "..#..", "..#..", "..#..", ".###."],
    '2': [".###.", "#...#", "...#.", "..#..", ".#...", "#####"],
    '3': ["####.", "....#", ".###.", "....#", "....#", "####."],
    '4': ["#..#.", "#..#.", "#####", "...#.", "...#.", "...#."],
    '5': ["#####", "#....", "####.", "....#", "....#", "####."],
    '6': [".###.", "#....", "####.", "#...#", "#...#", ".###."],
    '7': ["#####", "....#", "...#.", "..#..", "..#..", "..#.."],
    '8': [".###.", "#...#", ".###.", "#...#", "#...#", ".###."],
    '9': [".###.", "#...#", "#...#", ".####", "....#", ".###."],
}

def glyph(ch):
    rows = FONT[ch]
    grid = [['.'] * 8 for _ in range(8)]
    for y, row in enumerate(rows):
        for x, c in enumerate(row):
            if c == '#':
                grid[y + 1][x + 1] = 'W'
    # outline: any empty pixel touching a letter pixel (8 neighbours)
    out = [r[:] for r in grid]
    for y in range(8):
        for x in range(8):
            if grid[y][x] != '.':
                continue
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    yy, xx = y + dy, x + dx
                    if 0 <= yy < 8 and 0 <= xx < 8 and grid[yy][xx] == 'W':
                        out[y][x] = 'K'
    return "\n".join("".join(r) for r in out)

LETTERS = [("LETTER " + c, glyph(c)) for c in "ABCDEFGHIJKLMNOPQRSTUVWXYZ"]
DIGITS = [("NUMBER " + c, glyph(c)) for c in "0123456789"]

GROUPS = [("SHAPES", SHAPES), ("TYPES", ELEMENTS), ("SYMBOLS", SYMBOLS), ("LETTERS", LETTERS), ("NUMBERS", DIGITS)]

def parse(art, name):
    rows = [r.replace(' ', '.') for r in art.strip('\n').split('\n')]
    if len(rows) != 8 or any(len(r) != 8 for r in rows):
        raise SystemExit("%s: art must be 8x8, got %s" % (name, [len(r) for r in rows]))
    for r in rows:
        for c in r:
            if c not in PALETTE:
                raise SystemExit("%s: unknown color %r" % (name, c))
    return rows

def tile_4bpp(rows):
    out = bytearray()
    for r in rows:
        for x in range(0, 8, 2):
            lo = PALETTE[r[x]][0]
            hi = PALETTE[r[x + 1]][0]
            out.append(lo | (hi << 4))
    return bytes(out)

def all_decos():
    items = []
    for gname, lst in GROUPS:
        for name, art in lst:
            items.append((gname, name, parse(art, name)))
    return items

def write_header(path, items):
    pal = [0] * 16
    for k, (i, (r, g, b)) in PALETTE.items():
        pal[i] = r | (g << 5) | (b << 10)
    lines = ["// Generated by tools/moemon_balls/decorations.py: POKé BALL decorations (8x8 4bpp emblems).",
             "// Edit the art there and run it again.", "",
             "#define MP_DECO_COUNT %d   // plus 0 = none" % len(items), "",
             "static const u16 sDecoPalette[16] = {" + ", ".join("0x%04X" % c for c in pal) + "};", "",
             "static const u32 sDecoTiles[MP_DECO_COUNT][8] = {"]
    for g, name, rows in items:
        t = tile_4bpp(rows)
        words = [int.from_bytes(t[i:i + 4], 'little') for i in range(0, 32, 4)]
        lines.append("    {" + ", ".join("0x%08X" % w for w in words) + "},   // " + name)
    lines.append("};")
    lines.append("")
    lines.append("// ASCII names (shown in the summary's POKé BALL page) and their group")
    lines.append("static const char *const sDecoNames[MP_DECO_COUNT] = {")
    for g, name, rows in items:
        lines.append('    "%s",' % name)
    lines.append("};")
    groups = [g for g, _ in GROUPS]
    lines.append("")
    lines.append("// First decoration (1-based) of each group: L / R jump between groups")
    starts, n = [], 1
    for gname, lst in GROUPS:
        starts.append(n)
        n += len(lst)
    lines.append("static const u8 sDecoGroupStart[] = {" + ", ".join(str(s) for s in starts) + "};")
    lines.append("static const char *const sDecoGroupNames[] = {" + ", ".join('"%s"' % g for g in groups) + "};")
    with open(path, 'w') as f:
        f.write("\n".join(lines) + "\n")

def preview(path, items):
    from PIL import Image, ImageDraw
    rgb = {k: (r * 255 // 31, g * 255 // 31, b * 255 // 31) for k, (i, (r, g, b)) in PALETTE.items()}
    S = 6           # scale
    cell = 16 * S + 8
    balls = [((230, 50, 40), (250, 250, 250)), ((50, 90, 230), (250, 250, 250)), ((250, 250, 250), (250, 250, 250))]
    cols = 10
    rows_n = (len(items) + cols - 1) // cols
    img = Image.new('RGB', (cols * cell * 1, rows_n * (cell + 14) + 4), (36, 40, 52))
    d = ImageDraw.Draw(img)
    for n, (g, name, rows) in enumerate(items):
        cx, cy = (n % cols) * cell, (n // cols) * (cell + 14)
        top, bottom = balls[n % 3]
        # a 16x16 ball: top half colored, bottom white, black band
        for y in range(16):
            for x in range(16):
                dx, dy = x - 7.5, y - 7.5
                if dx * dx + dy * dy > 7.2 * 7.2:
                    continue
                col = top if y < 7 else bottom
                if 7 <= y <= 8 or dx * dx + dy * dy > 6.3 * 6.3:
                    col = (20, 20, 24)
                d.rectangle([cx + 4 + x * S, cy + 4 + y * S, cx + 4 + x * S + S - 1, cy + 4 + y * S + S - 1], fill=col)
        for y in range(8):
            for x in range(8):
                k = rows[y][x]
                if k == '.':
                    continue
                X, Y = cx + 4 + (x + 4) * S, cy + 4 + (y + 4) * S
                d.rectangle([X, Y, X + S - 1, Y + S - 1], fill=rgb[k])
        d.text((cx + 4, cy + cell - 2), name[:16], fill=(230, 230, 230))
    img.save(path)

if __name__ == '__main__':
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.normpath(os.path.join(here, '..', '..'))
    items = all_decos()
    write_header(os.path.join(root, 'src', 'mp', 'mp_balldeco_data.h'), items)
    print("%d decorations" % len(items))
    if len(sys.argv) > 2 and sys.argv[1] == '--preview':
        preview(sys.argv[2], items)
