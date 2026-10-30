#!/usr/bin/env python3
"""POKé BALL decorations (mp_ballstyle.c): small emblems drawn into a POKé BALL.

    decorations.py                 writes src/mp/mp_balldeco_data.h
    decorations.py --preview OUT   also renders a review sheet on the real art

Each decoration has two drawings, both drawn straight into the game's own pixels:
  icon  - up to 5x5, on the 24x24 POKé BALL item icon (the summary's POKé BALL
          page), centered at (9,9) on the dome next to the shine
  ball  - up to 3x3 (3x4 for letters and numbers), on the 16x16 battle / summary
          ball, first frame only, centered at (7,5)
Art is text, one character per pixel, '.' = untouched. Icon colors: any of
COLORS (at most 4 per emblem: they go into icon palette slots 12-15). Ball colors:
only BALL_COLORS (fixed slots 9-15 of the ball palette).
Letters and numbers use a 5x5 font whose top row bends in by a pixel, as if
the letter were wrapped over the dome.
"""
import os, sys

COLORS = {  # GBA RGB (0-31)
    'W': (31, 31, 31), 'K': (5, 5, 8),    'Y': (31, 30, 8),  'R': (25, 3, 3),
    'B': (11, 21, 31), 'b': (4, 10, 26),  'G': (10, 27, 8),  'g': (3, 15, 4),
    'P': (31, 14, 23), 'O': (31, 19, 4),  'V': (21, 11, 29), 'v': (11, 4, 17),
    'N': (17, 10, 4),  'S': (16, 16, 19), 'L': (26, 26, 28), 'C': (6, 26, 27),
}
# The battle ball's palette slots 9-15 (its art uses 1-8)
BALL_COLORS = ['W', 'Y', 'B', 'G', 'P', 'O', 'V']
BALL_FIRST_SLOT = 9

def A(s):
    return [r for r in s.strip('\n').split('\n')]

# name, icon art, ball art
SHAPES = [
    ("STAR",     A("..W..\nWWWWW\n.WWW.\n.W.W."),             A(".W.\nWWW\nW.W")),
    ("HEART",    A("WW.WW\nWWWWW\n.WWW.\n..W.."),             A("W.W\nWWW\n.W.")),
    ("DIAMOND",  A("..B..\n.BWB.\nBBBBB\n.BBB.\n..B.."),      A(".B.\nBWB\n.B.")),
    ("RING",     A(".WWW.\nW...W\nW...W\nW...W\n.WWW."),      A(".W.\nW.W\n.W.")),
    ("TRIANGLE", A("..G..\n.GGG.\nGGGGG"),                    A(".G.\nGGG")),
    ("PLUS",     A("..W..\n..W..\nWWWWW\n..W..\n..W.."),      A(".W.\nWWW\n.W.")),
    ("SPARKLE",  A("..Y..\n.YWY.\n..Y.."),                    A(".Y.\nYWY\n.Y.")),
    ("STRIPE",   A("WWWWW\nWWWWW"),                           A("WWW")),
    ("DOTS",     A("W..W.\n...W.\n.W...\n....W"),             A("W.W\n...\n.W.")),
    ("MOON",     A(".YYY.\nYY...\nYY...\nYY...\n.YYY."),      A(".YY\nY..\n.YY")),
    ("NOTE",     A("..WWW\n..W.W\n..W.W\nWW.WW\nWW.WW"),      A(".WW\n.W.\nWW.")),
    ("CROWN",    A("Y.Y.Y\nYYYYY\nYYYYY"),                    A("Y.Y\nYYY")),
    ("FLOWER",   A(".P.P.\nPPYPP\n.PPP.\n..G.."),             A(".P.\nPYP\n.P.")),
    ("CLOVER",   A(".G.G.\nGGGGG\n.GGG.\n..G.."),             A("G.G\n.G.\n.G.")),
    ("SMILE",    A(".YYY.\nYKYKY\nYYYYY\nYKKKY\n.YYY."),      A("W.W\n...\nWWW")),
]

TYPES = [
    ("NORMAL",   A(".WWW.\nWWWWW\nWWWWW\n.WWW."),             A("WW\nWW")),
    ("FIRE",     A("..R..\n.RYR.\nRYWYR\nRYYYR\n.RRR."),      A(".O.\nOYO\nOYO")),
    ("WATER",    A("..B..\n.BBB.\nBBWBB\nBBBBB\n.BBB."),      A(".B.\nBBB\nBBB")),
    ("GRASS",    A("...GG\n..GGG\n.GGg.\nGGg..\ng...."),      A("..G\n.GG\nG..")),
    ("ELECTRIC", A("..YY.\n.YY..\nYYYY.\n..YY.\n.YY.."),      A(".YY\n.Y.\nYY.")),
    ("ICE",      A("B.B.B\n.BBB.\nBBWBB\n.BBB.\nB.B.B"),      A("B.B\n.W.\nB.B")),
    ("FIGHTING", A(".WWW.\nWKWKW\nWWWWW\nKWWWW\n.WWW."),      A("WW.\nWWW\n.WW")),
    ("POISON",   A(".VVV.\nVWVV.\nVVVV.\n.VV.V\n...V."),      A("VV.\nVV.\n..V")),
    ("GROUND",   A("..N..\n.NNN.\nNNNNN"),                    A(".O.\nOOO")),
    ("FLYING",   A("BB...\nWBBB.\n.WBBB\n..WBB\n...WW"),      A("B..\nBB.\n.BB")),
    ("PSYCHIC",  A(".PPP.\nPWKWP\n.PPP."),                    A(".P.\nPWP\n.P.")),
    ("BUG",      A("G...G\n.GGG.\nGgGgG\n.GGG."),             A("G.G\nGGG\n.G.")),
    ("ROCK",     A(".SS..\nSSSS.\nSLSSS\n.SSS."),             A("OO.\nOOO")),
    ("GHOST",    A(".VVV.\nVWVWV\nVVVVV\nV.V.V"),             A(".V.\nVVV\nV.V")),
    ("DRAGON",   A("B...B\nBB.BB\n.BBB.\n..B.."),             A("B.B\n.B.\n.B.")),
    ("DARK",     A(".KKK.\nKK...\nKK...\n.KKK."),             A("VV.\nV..\nVV.")),
    ("STEEL",    A(".L.L.\nLLLLL\n.LSL.\nLLLLL\n.L.L."),      A("W.W\n.W.\nW.W")),
]

# 5x5 letters for the icon, top row bent in by a pixel (wrapped over the dome)
DOME = {
 'A': ".###./#...#/#####/#...#/#...#", 'B': ".##../#..#./####./#...#/####.",
 'C': ".###./#..../#..../#..../.####", 'D': ".##../#..#./#...#/#...#/####.",
 'E': ".###./#..../####./#..../#####", 'F': ".###./#..../####./#..../#....",
 'G': ".###./#..../#.###/#...#/.###.", 'H': ".#.#./#...#/#####/#...#/#...#",
 'I': ".###./..#../..#../..#../#####", 'J': "..##./...#./...#./#..#./.##..",
 'K': ".#.#./#.#../##.../#.#../#..#.", 'L': ".#.../#..../#..../#..../#####",
 'M': ".#.#./##.##/#.#.#/#...#/#...#", 'N': ".#.#./##..#/#.#.#/#..##/#...#",
 'O': ".###./#...#/#...#/#...#/.###.", 'P': ".##../#..#./####./#..../#....",
 'Q': ".###./#...#/#...#/#..#./.##.#", 'R': ".##../#..#./####./#..#./#...#",
 'S': ".###./#..../.###./....#/####.", 'T': ".###./..#../..#../..#../..#..",
 'U': ".#.#./#...#/#...#/#...#/.###.", 'V': ".#.#./#...#/.#.#./.#.#./..#..",
 'W': ".#.#./#...#/#.#.#/##.##/#...#", 'X': ".#.#./.#.#./..#../.#.#./#...#",
 'Y': ".#.#./.#.#./..#../..#../..#..", 'Z': ".###./...#./..#../.#.../#####",
 '0': ".###./#..##/#.#.#/##..#/.###.", '1': "..#../.##../..#../..#../.###.",
 '2': ".###./#...#/..##./.#.../#####", '3': ".###./....#/..##./....#/####.",
 '4': ".#.#./#..#./#####/...#./...#.", '5': ".###./#..../####./....#/####.",
 '6': ".###./#..../####./#...#/.###.", '7': ".###./....#/...#./..#../..#..",
 '8': ".###./#...#/.###./#...#/.###.", '9': ".###./#...#/.####/....#/.###.",
}
# 3x4 letters for the battle ball
SMALL = {
 'A': ".#./#.#/###/#.#", 'B': "##./###/#.#/##.", 'C': ".##/#../#../.##", 'D': "##./#.#/#.#/##.",
 'E': "###/##./#../###", 'F': "###/##./#../#..", 'G': ".##/#../#.#/.##", 'H': "#.#/###/#.#/#.#",
 'I': "###/.#./.#./###", 'J': "..#/..#/#.#/.#.", 'K': "#.#/##./#.#/#.#", 'L': "#../#../#../###",
 'M': "#.#/###/###/#.#", 'N': "##./#.#/#.#/#.#", 'O': ".#./#.#/#.#/.#.", 'P': "##./#.#/##./#..",
 'Q': ".#./#.#/#.#/.##", 'R': "##./#.#/##./#.#", 'S': ".##/#../..#/##.", 'T': "###/.#./.#./.#.",
 'U': "#.#/#.#/#.#/###", 'V': "#.#/#.#/#.#/.#.", 'W': "#.#/#.#/###/#.#", 'X': "#.#/.#./.#./#.#",
 'Y': "#.#/#.#/.#./.#.", 'Z': "###/..#/#../###",
 '0': "###/#.#/#.#/###", '1': ".#./##./.#./###", '2': "##./..#/.#./###", '3': "##./.##/..#/##.",
 '4': "#.#/#.#/###/..#", '5': "###/##./..#/##.", '6': ".##/#../###/###", '7': "###/..#/.#./.#.",
 '8': "###/###/#.#/###", '9': "###/###/..#/##.",
}

def glyph(table, ch):
    return [r.replace('#', 'W') for r in table[ch].split('/')]

LETTERS = [("LETTER " + c, glyph(DOME, c), glyph(SMALL, c)) for c in "ABCDEFGHIJKLMNOPQRSTUVWXYZ"]
NUMBERS = [("NUMBER " + c, glyph(DOME, c), glyph(SMALL, c)) for c in "0123456789"]

GROUPS = [("SHAPES", SHAPES), ("TYPES", TYPES), ("LETTERS", LETTERS), ("NUMBERS", NUMBERS)]

ICON_BOX, BALL_W, BALL_H = 5, 3, 4

def center(art, w, h, name):
    aw, ah = max(len(r) for r in art), len(art)
    if aw > w or ah > h:
        raise SystemExit("%s: %dx%d is larger than %dx%d" % (name, aw, ah, w, h))
    ox, oy = (w - aw) // 2, (h - ah) // 2
    grid = [['.'] * w for _ in range(h)]
    for y, r in enumerate(art):
        for x, c in enumerate(r):
            grid[oy + y][ox + x] = c
    return grid

def items():
    out = []
    for g, lst in GROUPS:
        for name, icon, ball in lst:
            ic = center(icon, ICON_BOX, ICON_BOX, name)
            # 3x3 ball art sits in rows 1-3 of the 3x4 box (same center as the letters' lower rows)
            bl = center(ball, BALL_W, BALL_H, name) if len(ball) == 4 else [['.'] * 3] + center(ball, BALL_W, 3, name)
            cols = []
            for r in ic:
                for c in r:
                    if c != '.' and c not in cols:
                        cols.append(c)
                    if c != '.' and c not in COLORS:
                        raise SystemExit("%s: unknown icon color %r" % (name, c))
            if len(cols) > 4:
                raise SystemExit("%s: icon uses %d colors (4 max)" % (name, len(cols)))
            for r in bl:
                for c in r:
                    if c != '.' and c not in BALL_COLORS:
                        raise SystemExit("%s: ball color %r isn't one of %s" % (name, c, BALL_COLORS))
            out.append((g, name, ic, cols, bl))
    return out

def rgb555(c):
    r, g, b = COLORS[c]
    return r | (g << 5) | (b << 10)

def write_header(path, its):
    L = ["// Generated by tools/moemon_balls/decorations.py: POKé BALL decorations. Edit the art there.", "",
         "#define MP_DECO_COUNT %d   // plus 0 = none" % len(its),
         "#define MP_DECO_ICON_SIZE %d" % ICON_BOX,
         "#define MP_DECO_BALL_W %d" % BALL_W, "#define MP_DECO_BALL_H %d" % BALL_H,
         "#define MP_DECO_BALL_FIRST_SLOT %d" % BALL_FIRST_SLOT, "",
         "// Ball palette slots %d-%d" % (BALL_FIRST_SLOT, BALL_FIRST_SLOT + len(BALL_COLORS) - 1),
         "static const u16 sDecoBallColors[%d] = {%s};" % (len(BALL_COLORS), ", ".join("0x%04X" % rgb555(c) for c in BALL_COLORS)), "",
         "struct MpDecoArt",
         "{",
         "    u8 icon[MP_DECO_ICON_SIZE][MP_DECO_ICON_SIZE];   // 0 = keep, 1-4 = iconColors[0-3] (icon palette slots 12-15)",
         "    u16 iconColors[4];",
         "    u8 ball[MP_DECO_BALL_H][MP_DECO_BALL_W];         // 0 = keep, else a ball palette slot",
         "};", "",
         "static const struct MpDecoArt sDecoArt[MP_DECO_COUNT] = {"]
    for g, name, ic, cols, bl in its:
        icon = ", ".join("{" + ", ".join(str(cols.index(c) + 1 if c != '.' else 0) for c in r) + "}" for r in ic)
        colors = ", ".join("0x%04X" % rgb555(c) for c in cols) or "0"
        ball = ", ".join("{" + ", ".join(str(BALL_FIRST_SLOT + BALL_COLORS.index(c) if c != '.' else 0) for c in r) + "}" for r in bl)
        L.append("    {{%s}, {%s}, {%s}},   // %s" % (icon, colors, ball, name))
    L += ["};", "", "static const char *const sDecoNames[MP_DECO_COUNT] = {"]
    L += ['    "%s",' % name for g, name, ic, cols, bl in its]
    starts, n = [], 1
    for g, lst in GROUPS:
        starts.append(n)
        n += len(lst)
    L += ["};", "", "// First decoration of each group (L / R jump between them)",
          "static const u8 sDecoGroupStart[] = {%s};" % ", ".join(map(str, starts))]
    with open(path, 'w') as f:
        f.write("\n".join(L) + "\n")

def preview(path, its, root):
    from PIL import Image, ImageDraw
    def load(p):
        im = Image.open(p); idx = im.load(); rgba = im.convert('RGBA'); px = rgba.load()
        for y in range(im.size[1]):
            for x in range(im.size[0]):
                if idx[x, y] == 0:
                    px[x, y] = (0, 0, 0, 0)
        return rgba
    icon = load(os.path.join(root, 'graphics/items/icons/poke_ball.png'))
    ball = load(os.path.join(root, 'graphics/balls/poke.png')).crop((0, 8, 16, 24))   # the resting frame's middle (rows 8-23 of 32)
    c8 = lambda c: tuple(v * 255 // 31 for v in COLORS[c]) + (255,)
    S, per = 6, 12
    cw, ch = 24 * S + 10, 24 * S + 16 * S + 30
    rows = (len(its) + per - 1) // per
    img = Image.new('RGBA', (per * cw + 10, rows * ch + 10), (40, 44, 56, 255))
    d = ImageDraw.Draw(img)
    for n, (g, name, ic, cols, bl) in enumerate(its):
        a = icon.copy(); p = a.load()
        for y in range(5):
            for x in range(5):
                if ic[y][x] != '.':
                    p[7 + x, 7 + y] = c8(ic[y][x])
        b = ball.copy(); q = b.load()
        for y in range(4):
            for x in range(3):
                if bl[y][x] != '.':
                    q[6 + x, 3 + y] = c8(bl[y][x])
        x0, y0 = 10 + (n % per) * cw, 10 + (n // per) * ch
        A2 = a.resize((24 * S, 24 * S), Image.NEAREST); img.paste(A2, (x0, y0), A2)
        B2 = b.resize((16 * S, 16 * S), Image.NEAREST); img.paste(B2, (x0 + 4 * S, y0 + 24 * S), B2)
        img.paste(a, (x0, y0 + 24 * S + 2), a); img.paste(b, (x0 + 4, y0 + 24 * S + 30), b)
        d.text((x0, y0 + ch - 22), name, fill=(230, 230, 230))
    img.save(path)

if __name__ == '__main__':
    root = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
    its = items()
    write_header(os.path.join(root, 'src', 'mp', 'mp_balldeco_data.h'), its)
    print("%d decorations" % len(its))
    if len(sys.argv) > 2 and sys.argv[1] == '--preview':
        preview(sys.argv[2], its, root)
