#!/usr/bin/env python3
"""The BAG screen from the user's design (docs/user_images/31.png, 256x192, HGSS-style), fitted to 240x160.

Writes src/mp/mp_bag_gfx.h:
- the background in full color (a table of the design's own colors, in 15-bit color, and an index per pixel), drawn
  under the game's layers (mp_bagui.c MP_BagUI_Composite), not bound by the GBA's 16-color palettes;
- the eight pocket icons (10x10: 0 the cell, 1 dark, 2 light) and where their row is, drawn over it with the pocket
  shown picked out (the design's red frame and red icon).

What comes from where (the design's pixels, moved; nothing redrawn):
- the left column (x 0..101): the BAG plate and the light blue stripes (rows 0-65, the design's own bag painted out with
  each row's stripe), then the orange panel from its top edge to the row under the pocket name plate (the design's
  rows 83-124: the pocket icon row and the plate, its text "ITEMS" painted out). The bag rows 66-82 and the "POCKET"
  tab below the plate (rows 125-140) are left out to make room for the description box;
- the list (x 104..239): the design's list frame, its text and red cursor painted out, 16 columns and 31 rows taken out
  of its plain white inside;
- the description box (rows 108-159): the design's rows 140-191, 16 columns out of its plain purple middle, the text and
  the REPEL icon painted out (the game prints the description and puts the item's icon there);
- a straight edge between the left column and the green behind the list (the design's edge is slanted).

usage: tools/moemon_bag/gen_bag.py [--preview OUT.png]  (from the repo root)
"""
import os, sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, '..', '..'))
SRC = os.path.join(REPO, 'docs', 'user_images', '31.png')
OUT = os.path.join(REPO, 'src', 'mp', 'mp_bag_gfx.h')

W, H = 240, 160
EDGE = 102                       # the grey line between the left column and the green (ours)
LIST_CUT_X = (170, 186)          # the design's list columns taken out (plain white inside)
LIST_CUT_Y = (40, 71)            # ... and rows (five list rows of 16 fit)
DESC_CUT_X = (120, 136)          # the description box's columns taken out (plain purple)
DESC_TOP = 108                   # ours: the description box from here down (the design's row 140)

WHITE = (248, 248, 248)
PURPLE = (112, 80, 240)
GREY_EDGE = (144, 144, 144)
GREEN = (136, 224, 96)
RED = (248, 0, 0)
GRAY = (168, 184, 184)           # (the list text's shadow: the frame on an item being moved)

# the pocket icon row (the design's): cell k's inside is x 7+11k .. 16+11k, rows 91-100; border rows 90 and 101
CELL_X0, CELL_PITCH, CELL_W = 7, 11, 10
STRIP_ROW = 91
ICON_DARK, ICON_LIGHT = (88, 88, 176), (168, 168, 216)
SEL_DARK, SEL_LIGHT = (208, 80, 80), (240, 168, 144)
STRIP_BORDER = (112, 120, 144)


def main():
    m = Image.open(SRC).convert('RGB')
    assert m.size == (256, 192)
    M = [[m.getpixel((x, y)) for x in range(256)] for y in range(192)]

    # --- paint out what the game draws itself
    # the design's bag (rows 18-82): each row's own stripe, left of the column's edge (the grey line or the green)
    for y in range(18, 83):
        stripe = M[y][1]
        for x in range(4, 100):
            if M[y][x] in (GREY_EDGE, GREEN):
                break
            M[y][x] = stripe
    # the pocket name ("ITEMS") inside the plate: rows 106-121 between its light inner edges
    for y in range(106, 122):
        for x in range(5, 95):
            M[y][x] = WHITE
    # the list's text and red cursor: everything inside the frame below its header band is white
    for y in range(15, 126):
        for x in range(110, 246):
            M[y][x] = WHITE
        for x in range(100, 256):
            if M[y][x] == RED:
                M[y][x] = M[60][x]   # (the cursor's sides over the frame: the frame as on a plain row)
    # the description's text (rows 143-189 right of the icon box) and the icon in its box
    for y in range(144, 190):
        for x in range(36, 252):
            if M[y][x] not in (PURPLE, (160, 168, 240), (80, 96, 152), (88, 88, 176)):
                M[y][x] = PURPLE
    for y in range(153, 181):
        for x in range(4, 32):
            M[y][x] = WHITE

    # --- the pocket icons (cell 0 is drawn picked out in the design: its colors map back to the others')
    icons = []
    for k in range(8):
        cell = []
        for yy in range(10):
            row = []
            for xx in range(CELL_W):
                c = M[STRIP_ROW + yy][CELL_X0 + CELL_PITCH * k + xx]
                row.append(1 if c in (ICON_DARK, SEL_DARK) else 2 if c in (ICON_LIGHT, SEL_LIGHT) else 0)
            cell.append(row)
        icons.append(cell)
    # the strip without the red frame: every cell plain, the grey border and separators all round
    for y in range(89, 103):
        for x in range(4, 20):
            if M[y][x] == RED:
                M[y][x] = M[y][2] if y in (89, 102) else WHITE
    for x in range(6, 95):
        M[90][x] = STRIP_BORDER
        M[101][x] = STRIP_BORDER
    for k in range(9):
        for y in range(90, 102):
            M[y][CELL_X0 - 1 + CELL_PITCH * k] = STRIP_BORDER
    for k in range(8):
        for yy in range(10):
            for xx in range(CELL_W):
                v = icons[k][yy][xx]
                M[STRIP_ROW + yy][CELL_X0 + CELL_PITCH * k + xx] = WHITE if v == 0 else ICON_DARK if v == 1 else ICON_LIGHT
    # (the corners just outside the frame, the orange around it)
    for y in (89, 102):
        for x in range(4, 20):
            if M[y][x] == WHITE:
                M[y][x] = M[y][2]

    # --- compose ours
    out = [[None] * W for _ in range(H)]
    # left column: design rows for our rows 0..107
    left_rows = list(range(0, 66)) + list(range(83, 125))
    assert len(left_rows) == DESC_TOP
    for y, sy in enumerate(left_rows):
        bg = M[sy][70] if sy < 66 else M[sy][1]   # (the stripes right of the BAG plate; the orange panel's own)
        for x in range(EDGE):
            c = M[sy][x]
            if (x >= 90 and c in (GREY_EDGE, GREEN, (88, 200, 80))) or (sy < 66 and x >= 58):
                c = bg   # (the design's slanted edge: the column runs straight to ours; nothing else right of the plate)
            out[y][x] = c
        out[y][EDGE] = GREY_EDGE
    # right: the list frame, rows and columns taken out of its inside
    right_rows = [r for r in range(0, 141) if not (LIST_CUT_Y[0] <= r < LIST_CUT_Y[1])]
    right_rows = right_rows[:DESC_TOP]
    right_cols = [c for c in range(EDGE - 2, 256) if not (LIST_CUT_X[0] <= c < LIST_CUT_X[1])]
    for y, sy in enumerate(right_rows):
        for i, x in enumerate(range(EDGE + 1, W)):
            sx = right_cols[i + 3]
            c = M[sy][sx]
            if c == GREY_EDGE and sx < 108:
                c = GREEN   # (the design's slanted edge)
            out[y][x] = c
    # the description box
    desc_cols = [c for c in range(256) if not (DESC_CUT_X[0] <= c < DESC_CUT_X[1])]
    desc_rows = list(range(141, 167)) + list(range(166, 192))   # (from its dark top edge; one plain row twice)
    assert len(desc_rows) == H - DESC_TOP
    for y in range(DESC_TOP, H):
        sy = desc_rows[y - DESC_TOP]
        for x in range(W):
            out[y][x] = M[sy][desc_cols[x]]
    for y in range(H):
        for x in range(W):
            assert out[y][x] is not None, (x, y)

    # strip and cells in ours (rows moved by the left column's mapping)
    strip_y = left_rows.index(STRIP_ROW)
    colors = sorted({c for row in out for c in row} | {SEL_DARK, SEL_LIGHT, RED, GRAY, ICON_DARK, ICON_LIGHT, WHITE})
    index = {c: i for i, c in enumerate(colors)}
    assert len(colors) <= 256

    if '--preview' in sys.argv:
        im = Image.new('RGB', (W, H))
        for y in range(H):
            for x in range(W):
                im.putpixel((x, y), out[y][x])
        # the first pocket picked out, as the game draws it
        sel = 0
        x0 = CELL_X0 + CELL_PITCH * sel
        for yy in range(10):
            for xx in range(CELL_W):
                v = icons[sel][yy][xx]
                im.putpixel((x0 + xx, strip_y + yy), WHITE if v == 0 else SEL_DARK if v == 1 else SEL_LIGHT)
        for t in range(2):
            for xx in range(x0 - 2, x0 + CELL_W + 2):
                im.putpixel((xx, strip_y - 2 + t), RED)
                im.putpixel((xx, strip_y + 10 + t), RED)
            for yy in range(strip_y - 2, strip_y + 12):
                im.putpixel((x0 - 2 + t, yy), RED)
                im.putpixel((x0 + CELL_W + t, yy), RED)
        im.save(sys.argv[sys.argv.index('--preview') + 1])

    def c555(c):
        return (c[0] >> 3) | ((c[1] >> 3) << 5) | ((c[2] >> 3) << 10)

    with open(OUT, 'w') as f:
        f.write('// Generated by tools/moemon_bag/gen_bag.py from docs/user_images/31.png: do not edit.\n')
        f.write('#define BAGUI_STRIP_Y %d      // the pocket icon cells\' first row\n' % strip_y)
        f.write('#define BAGUI_CELL_X0 %d\n#define BAGUI_CELL_PITCH %d\n#define BAGUI_CELL_W %d\n' % (CELL_X0, CELL_PITCH, CELL_W))
        f.write('#define BAGUI_DESC_TOP %d\n' % DESC_TOP)
        f.write('#define BAGUI_C_WHITE %d\n#define BAGUI_C_DARK %d\n#define BAGUI_C_LIGHT %d\n' % (index[WHITE], index[ICON_DARK], index[ICON_LIGHT]))
        f.write('#define BAGUI_C_SEL_DARK %d\n#define BAGUI_C_SEL_LIGHT %d\n#define BAGUI_C_RED %d\n#define BAGUI_C_GRAY %d\n'
                % (index[SEL_DARK], index[SEL_LIGHT], index[RED], index[GRAY]))
        f.write('static const u16 sBagUI_Colors[%d] = {\n' % len(colors))
        for i in range(0, len(colors), 12):
            f.write('    ' + ', '.join('0x%04X' % c555(c) for c in colors[i:i + 12]) + ',\n')
        f.write('};\n')
        f.write('static const u8 sBagUI_Bg[%d][%d] = {\n' % (H, W))
        for y in range(H):
            f.write('    {' + ','.join(str(index[c]) for c in out[y]) + '},\n')
        f.write('};\n')
        f.write('static const u8 sBagUI_Icons[8][10][%d] = {\n' % CELL_W)
        for k in range(8):
            f.write('    {' + ', '.join('{' + ','.join(str(v) for v in row) + '}' for row in icons[k]) + '},\n')
        f.write('};\n')
    print('wrote %s (%d colors, strip at %d)' % (OUT, len(colors), strip_y))


if __name__ == '__main__':
    main()
