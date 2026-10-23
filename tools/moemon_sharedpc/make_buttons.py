#!/usr/bin/env python3
# Moemon Co-op: the shared PC box screen's PARTY / MY PC / CLOSE buttons (graphics/moemon/sharedpc/btn_*.png).
#
# Built from the stock box screen's PARTY POKeMON (green) and CLOSE BOX (blue) buttons, as the game draws them
# (box_screen_ref.png: a 240x160 capture of the box screen): their caps and fill make the new, narrower buttons and
# their letters spell the new labels. MY PC is the green button turned yellow. Each output is a 64x32 indexed PNG
# (color 0 transparent) with the 48x16 button in its top-left corner.
#
#   python3 tools/moemon_sharedpc/make_buttons.py [box_screen_ref.png] [out_dir]
import colorsys, os, sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ref = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "box_screen_ref.png")
out_dir = sys.argv[2] if len(sys.argv) > 2 else os.path.join(HERE, "..", "..", "graphics", "moemon", "sharedpc")
im = Image.open(ref).convert("RGB")
px = im.load()
W, H = 48, 16

def grab(x0, x1, y0, y1):
    return [[px[x, y] for x in range(x0, x1 + 1)] for y in range(y0, y1 + 1)]

# The stock buttons: (left, right, top) and their text / shadow / fill colors
GREEN = dict(rows=grab(81, 150, 0, 15), text=(255, 255, 255), shadow=(115, 189, 98), fill=(164, 238, 148))
BLUE = dict(rows=grab(169, 238, 1, 16), text=(255, 255, 255), shadow=(123, 180, 213), fill=(164, 205, 246))

def glyphs(btn):
    """The letters of a button, left to right: lists of columns of 'T'/'S'/None, and where they sit (row offset)"""
    rows = btn["rows"]
    w = len(rows[0])
    cols = []
    for x in range(w):
        col = []
        for y in range(len(rows)):
            c = rows[y][x]
            col.append("T" if c == btn["text"] else "S" if c == btn["shadow"] else None)
        cols.append(col)
    # letters are the runs of columns with text; each keeps the shadow column right after it
    out, cur = [], []
    x = 3
    while x < w - 3:
        if "T" in cols[x]:
            cur.append(cols[x])
        elif cur:
            if any(v == "S" for v in cols[x]):
                cur.append([v if v == "S" else None for v in cols[x]])
            out.append(cur)
            cur = []
        x += 1
    if cur:
        out.append(cur)
    return out

def blank(btn, width):
    """The button's caps and fill at a new width (text taken out)"""
    rows = btn["rows"]
    res = []
    for y, r in enumerate(rows):
        clean = [btn["fill"] if c in (btn["text"], btn["shadow"]) and 3 <= x < len(r) - 3 else c for x, c in enumerate(r)]
        mid = clean[len(clean) // 2]
        res.append(clean[:4] + [mid] * (width - 8) + clean[-4:])
    return res

G = glyphs(GREEN)   # P A R T Y P O K e M O N
B = glyphs(BLUE)    # C L O S E B O X
assert len(G) == 12 and len(B) == 8, (len(G), len(B))
P, A, R, T, Y, O, K, M = G[0], G[1], G[2], G[3], G[4], G[6], G[7], G[9]
C, L, O2, S, E = B[0], B[1], B[2], B[3], B[4]

def make(btn, letters, colors):
    base = blank(btn, W)
    seq = []
    for g in letters:
        if g is None:
            seq.append(None)
        else:
            seq.append(g)
    width = sum(4 if g is None else len(g) for g in seq)
    x = (W - width) // 2
    for g in seq:
        if g is None:
            x += 4
            continue
        for cx, col in enumerate(g):
            for y, v in enumerate(col):
                if v == "T":
                    base[y][x + cx] = colors["text"]
                elif v == "S":
                    base[y][x + cx] = colors["shadow"]
        x += len(g)
    return base

def recolor(rows, f):
    return [[f(c) for c in r] for r in rows]

def to_yellow(c):
    r, g, b = [v / 255 for v in c]
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    if s > 0.15:
        h = (h - 60 / 360) % 1.0   # green -> yellow
    r, g, b = colorsys.hsv_to_rgb(h, s, v)
    return (int(round(r * 255)) & ~7, int(round(g * 255)) & ~7, int(round(b * 255)) & ~7)

party = make(GREEN, [P, A, R, T, Y], GREEN)
mypc = recolor(make(GREEN, [M, Y, None, P, C], dict(text=GREEN["text"], shadow=GREEN["shadow"])), to_yellow)
close = make(BLUE, [C, L, O2, S, E], BLUE)

def save(rows, name):
    colors = []
    for r in rows:
        for c in r:
            if c not in colors:
                colors.append(c)
    assert len(colors) <= 15, (name, len(colors))
    pal = [(255, 0, 255)] + colors
    img = Image.new("P", (64, 32), 0)
    flat = []
    for c in pal:
        flat += list(c)
    flat += [0] * (768 - len(flat))
    img.putpalette(flat)
    p = img.load()
    for y, r in enumerate(rows):
        for x, c in enumerate(r):
            p[x, y] = pal.index(c)
    img.save(os.path.join(out_dir, name))
    print(name, len(colors), "colors")

os.makedirs(out_dir, exist_ok=True)
save(party, "btn_party.png")
save(mypc, "btn_mypc.png")
save(close, "btn_close.png")
