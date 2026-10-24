#!/usr/bin/env python3
"""Moemon Co-op: the Pokedex info page's top bar with a TEXT tab.

The stock bar has four 7-tile buttons (AREA CRY SIZE CANCEL). This builds five buttons in the
30 visible tiles, from the stock bar's own pixels: TEXT 6 tiles, AREA 6, CRY 5, SIZE 6 and the
stock CANCEL button (7). Buttons are narrowed by dropping plain middle columns; the labels are
the stock ones, and TEXT is drawn in the same 5x9 letter style (E from AREA; T and X drawn here,
shadowed with the stock rule: a shadow pixel right of, below, or down-right of a letter pixel).

Usage: gen_bar.py [REPO [OUT_H [PREVIEW_PNG]]]
Writes REPO/src/data/pokemon/mp_dex_bar.h (60 tiles: row 0 then row 1, 30 tiles each).
"""
import os
import struct
import sys

REPO = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
OUT = sys.argv[2] if len(sys.argv) > 2 else os.path.join(REPO, 'src/data/pokemon/mp_dex_bar.h')
PREVIEW = sys.argv[3] if len(sys.argv) > 3 else None

# The slots: (first column, width in tiles). Must match sMpDexBarSlots in pokedex.c.
SLOTS = [(0, 6), (6, 6), (12, 5), (17, 6), (23, 7)]
LETTER, SHADOW, FILL = 15, 13, 12


def lz77(d):
    assert d[0] == 0x10
    n = d[1] | d[2] << 8 | d[3] << 16
    o = bytearray()
    i = 4
    while len(o) < n:
        f = d[i]
        i += 1
        for b in range(8):
            if len(o) >= n:
                break
            if f & (0x80 >> b):
                x = d[i] << 8 | d[i + 1]
                i += 2
                for _ in range((x >> 12) + 3):
                    o.append(o[-((x & 0xfff) + 1)])
            else:
                o.append(d[i])
                i += 1
    return bytes(o)


tiles = lz77(open(os.path.join(REPO, 'graphics/moemon/gPokedexMenu_Gfx.bin'), 'rb').read())
raw = open(os.path.join(REPO, 'graphics/pokedex/screen_select_bar_main.bin'), 'rb').read()
tmap = struct.unpack('<%dH' % (len(raw) // 2), raw)


def tile_pixel(entry, x, y):
    t = entry & 0x3ff
    sx = 7 - x if entry & 0x400 else x
    sy = 7 - y if entry & 0x800 else y
    b = tiles[t * 32 + sy * 4 + sx // 2]
    return (b >> (4 * (sx & 1))) & 15


# The stock bar's pixels, 240 x 16
stock = [[tile_pixel(tmap[(y // 8) * 32 + c], x, y % 8) for c in range(30) for x in range(8)] for y in range(16)]


def stock_slot(k):
    x0 = 8 + 56 * k
    return [row[x0:x0 + 56] for row in stock]


def label_of(slot, letter):
    cols = [x for x in range(len(slot[0])) if any(slot[y][x] == letter for y in range(4, 13))]
    a, b = min(cols), max(cols)
    return [[slot[y][x] == letter for x in range(a, b + 1)] for y in range(4, 13)], a


def shadowed(mask, letter, shadow):
    h, w = len(mask), len(mask[0])
    out = [[None] * (w + 1) for _ in range(h + 1)]

    def on(y, x):
        return 0 <= y < h and 0 <= x < w and mask[y][x]
    for y in range(h + 1):
        for x in range(w + 1):
            if on(y, x):
                out[y][x] = letter
            elif on(y, x - 1) or on(y - 1, x) or on(y - 1, x - 1):
                out[y][x] = shadow
    return out


def glyph(rows):
    return [[c == '#' for c in r] for r in rows]


T = glyph(['#####'] + ['..#..'] * 8)
X = glyph(['#...#', '#...#', '.#.#.', '.#.#.', '..#..', '.#.#.', '.#.#.', '#...#', '#...#'])
area, _ = label_of(stock_slot(0), LETTER)
E = [r[12:17] for r in area]          # A R E A: the E is the third letter
assert E[0] == [True] * 5 and E[8] == [True] * 5


def word(letters):
    rows = []
    for y in range(9):
        r = []
        for i, g in enumerate(letters):
            if i:
                r.append(False)
            r += g[y]
        rows.append(r)
    return rows


def erase_label(slot, fill):
    s = [row[:] for row in slot]
    for y in range(3, 15):
        for x in range(8, 48):
            s[y][x] = fill
    return s


def narrow(slot, width):
    """A width-pixel button from a 56-pixel stock one: drop plain middle columns."""
    return [row[:8] + [row[8]] * (width - 16) + row[48:] for row in slot]


def place(button, label_pixels):
    w = len(button[0])
    lw = len(label_pixels[0])
    x0 = (w - lw) // 2
    for y, r in enumerate(label_pixels):
        for x, v in enumerate(r):
            if v is not None:
                button[4 + y][x0 + x] = v
    return button


labels = [shadowed(word([T, E, X, T]), LETTER, SHADOW)]
for k in range(3):                      # the stock labels, letter and shadow pixels as they are
    sl = stock_slot(k)
    m, a = label_of(sl, LETTER)
    labels.append([[v if v in (LETTER, SHADOW) else None for v in sl[y][a:a + len(m[0]) + 1]] for y in range(4, 14)])

bar = [[None] * 240 for _ in range(16)]
plain = erase_label(stock_slot(0), FILL)
for i, (col, w) in enumerate(SLOTS):
    if i == 4:
        button = stock_slot(3)          # CANCEL as it is
    else:
        button = place(narrow(plain, w * 8), labels[i])
    for y in range(16):
        for x in range(w * 8):
            bar[y][col * 8 + x] = button[y][x]

# Check: the stock labels come out pixel for pixel as in the stock bar
for k, slot in ((1, 0), (2, 1), (3, 2)):
    col, w = SLOTS[k]
    sw, nw = stock_slot(slot), [r[col * 8:col * 8 + w * 8] for r in bar]
    sm, sa = label_of(sw, LETTER)
    nm, na = label_of(nw, LETTER)
    assert sm == nm, 'label %d differs' % k
    for y in range(4, 14):
        assert sw[y][sa:sa + len(sm[0]) + 1] == nw[y][na:na + len(nm[0]) + 1], 'label %d shadow differs' % k

data = bytearray()
for ty in range(2):
    for tx in range(30):
        for y in range(8):
            for x in range(0, 8, 2):
                p = bar[ty * 8 + y][tx * 8 + x]
                q = bar[ty * 8 + y][tx * 8 + x + 1]
                data.append(p | q << 4)
words = struct.unpack('<%dI' % (len(data) // 4), bytes(data))
with open(OUT, 'w') as f:
    f.write('// Moemon Co-op: the Pokedex info page bar with a TEXT tab (tools/moemon_dex/gen_bar.py).\n')
    f.write('// Generated from the stock bar (graphics/pokedex/screen_select_bar_main.bin): do not edit by hand.\n')
    f.write('// 60 4bpp tiles: the top row (30 tiles), then the bottom row.\n\n')
    f.write('static const u32 sMpDexBarTiles[%d] =\n{\n' % len(words))
    for i in range(0, len(words), 8):
        f.write('    ' + ', '.join('0x%08X' % w for w in words[i:i + 8]) + ',\n')
    f.write('};\n')
print('wrote', OUT)

if PREVIEW:
    from PIL import Image
    lines = open(os.path.join(REPO, 'graphics/pokedex/bg_hoenn.pal')).read().split('\n')[3:]
    pal = [tuple(map(int, l.split())) for l in lines if l.strip()]
    im = Image.new('RGB', (240, 40))
    px = im.load()
    for band, pn in ((0, 4), (1, 2)):
        for y in range(16):
            for x in range(240):
                px[x, band * 20 + y] = pal[pn * 16 + bar[y][x]]
    im.resize((960, 160), Image.NEAREST).save(PREVIEW)
    print('preview', PREVIEW)
