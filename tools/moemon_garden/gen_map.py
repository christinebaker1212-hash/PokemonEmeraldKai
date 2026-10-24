#!/usr/bin/env python3
"""MOeMON GARDEN map: data/layouts/MoemonGarden/{map,border}.bin and src/mp/mp_garden_map.h.

Built from pieces of Littleroot Town and Petalburg City (the same tilesets, General + Petalburg), so every
metatile keeps its own collision and elevation bits. The Garden sits west of Littleroot (a map connection),
34 x 20: the east part (cols 14-31) is the Garden alone; the west part (cols 2-11) is the extra meadow behind a
hedge (cols 12-13) that opens when friends are there. Growth marks (patches that appear as the journey goes on)
are written by mp_garden.c at load time from the GROWTH_* rectangles here.
"""
import os, random, struct

ROOT = os.path.join(os.path.dirname(__file__), '..', '..')
W, H = 34, 20

def load(name, w):
    d = open(os.path.join(ROOT, 'data/layouts/%s/map.bin' % name), 'rb').read()
    v = struct.unpack('<%dH' % (len(d) // 2), d)
    return [list(v[r * w:(r + 1) * w]) for r in range(len(v) // w)]

lr = load('LittlerootTown', 20)
pb = load('PetalburgCity', 30)

GRASS = lr[4][0]          # 0x001 with its bits
FLOWER = lr[8][0]         # 0x004
SIGN = lr[8][7]           # 0x003 (a sign post)
T_TOP_A, T_TOP_B = lr[0][0], lr[0][1]      # 1d4 1d5
T_BOT_A, T_BOT_B = lr[1][0], lr[1][1]      # 1dc 1dd
T_BASE_A, T_BASE_B = lr[1][2], lr[1][3]    # 1e4 1e5 (forest edge above open ground)
T_CAP_A, T_CAP_B = lr[19][2], lr[19][3]    # 1ce 1cf (tree tops over the bottom row)
ELEV3 = GRASS & 0xF000

def mt(id_, collision=0):
    return ELEV3 | (collision << 10) | id_

g = [[GRASS] * W for _ in range(H)]
rnd = random.Random(1212)

def forest_cell(r, c):
    a = (c % 2) == 0
    if r % 2 == 0:
        return T_TOP_A if a else T_TOP_B
    return T_BOT_A if a else T_BOT_B

# borders: top forest with its base row, left forest, bottom tree tops
for c in range(W):
    g[0][c] = forest_cell(0, c)
    g[1][c] = T_BASE_A if c % 2 == 0 else T_BASE_B
    g[H - 1][c] = T_CAP_A if c % 2 == 0 else T_CAP_B
for r in range(1, H):
    for c in (0, 1):
        g[r][c] = forest_cell(r, c)
# the east edge continues Littleroot's west edge (its cols 0-1): open on rows 4-16
for r in range(H):
    for k, c in enumerate((32, 33)):
        g[r][c] = lr[r][k]
# the hedge between the Garden and the extra meadow (opened at run time)
HEDGE = (12, 13)
for r in range(1, H):
    for c in HEDGE:
        g[r][c] = forest_cell(r, c)
g[1][12], g[1][13] = forest_cell(1, 12), forest_cell(1, 13)

# flowers scattered over the open meadow
for r in range(2, H - 1):
    for c in list(range(2, 12)) + list(range(14, 32)):
        if rnd.random() < 0.07:
            g[r][c] = FLOWER

def stamp(src, sr, sc, h, w, dr, dc):
    for r in range(h):
        for c in range(w):
            g[dr + r][dc + c] = src[sr + r][sc + c]

# the pond (Petalburg City's, rows 2-9, cols 19-25), and a second one in the extra meadow
POND = (3, 16, 8, 7)          # row, col, height, width
stamp(pb, 2, 19, 8, 7, POND[0], POND[1])
POND2 = (9, 3, 8, 7)
stamp(pb, 2, 19, 8, 7, POND2[0], POND2[1])
# Petalburg's pond ends in stairs (250 251) on its bottom row: plain shore here
SHORE = pb[9][19]   # 0x002
for (pr, pc, _h, _w) in (POND, POND2):
    for c in range(pc, pc + 7):
        if g[pr + 7][c] & 0x3FF in (0x250, 0x251):
            g[pr + 7][c] = SHORE
# the GARDEN BOARD (a sign: A in front of it opens the Garden menu)
BOARD = (5, 26)
g[BOARD[0]][BOARD[1]] = SIGN
# keep the paths clear of flowers around the board and the entry
for (r, c) in [(6, 26), (5, 27), (5, 25)]:
    g[r][c] = GRASS

# growth marks (mp_garden.c stamps these when their moment in the journey has come)
GROWTH = {
    'TALL_GRASS': (12, 26, 3, 4, [mt(0x00d)]),                 # Petalburg Woods behind them: foliage
    'PUDDLES':    (13, 15, 3, 3, None),                        # SLATEPORT: a damp corner (the 3x3 puddle set)
    'SAND':       (15, 21, 2, 4, [mt(0x11c)]),                 # the desert: a dig patch
    'GROVE':      (8, 28, 2, 4, None),                         # FORTREE: two trees to perch in
    'STONE':      (10, 24, 1, 1, None),                        # the WEATHER INSTITUTE: a storm stone
}
PUDDLE_SET = [[mt(0x0c8), mt(0x0c9), mt(0x0ca)], [mt(0x0d0), mt(0x0d1), mt(0x0d2)], [mt(0x0d8), mt(0x0d9), mt(0x0da)]]
TREE_SET = [[T_CAP_A, T_CAP_B], [mt(0x1d6, 3), mt(0x1d7, 3)]]
for name, (r, c, h, w, _) in GROWTH.items():
    for rr in range(r, r + h):
        for cc in range(c, c + w):
            g[rr][cc] = GRASS   # plain until it grows

def le16(v):
    return struct.pack('<H', v)

os.makedirs(os.path.join(ROOT, 'data/layouts/MoemonGarden'), exist_ok=True)
with open(os.path.join(ROOT, 'data/layouts/MoemonGarden/map.bin'), 'wb') as f:
    for row in g:
        for v in row:
            f.write(le16(v))
with open(os.path.join(ROOT, 'data/layouts/LittlerootTown/border.bin'), 'rb') as f:
    border = f.read()
with open(os.path.join(ROOT, 'data/layouts/MoemonGarden/border.bin'), 'wb') as f:
    f.write(border)

# the C side
def arr(rows):
    return '{' + ', '.join('{' + ', '.join('0x%04X' % v for v in row) + '}' for row in rows) + '}'
lines = ['// Generated by tools/moemon_garden/gen_map.py (the MOeMON GARDEN map\'s layout facts). Do not edit.',
         '#ifndef GUARD_MP_GARDEN_MAP_H', '#define GUARD_MP_GARDEN_MAP_H', '',
         '#define GARDEN_W %d' % W, '#define GARDEN_H %d' % H,
         '#define GARDEN_HEDGE_X %d   // and the next column; rows 2..%d' % (HEDGE[0], H - 2),
         '#define GARDEN_EAST_X0 14', '#define GARDEN_EAST_X1 31', '#define GARDEN_WEST_X0 2', '#define GARDEN_WEST_X1 11',
         '#define GARDEN_Y0 2', '#define GARDEN_Y1 %d' % (H - 2),
         '#define GARDEN_BOARD_X %d' % BOARD[1], '#define GARDEN_BOARD_Y %d' % BOARD[0],
         '#define GARDEN_ENTRY_X 31', '#define GARDEN_ENTRY_Y 10   // where the shortcut puts you',
         '#define GARDEN_GRASS 0x%04X' % GRASS, '#define GARDEN_FLOWER 0x%04X' % FLOWER, '']
hedge_rows = [[g[r][HEDGE[0]], g[r][HEDGE[1]]] for r in range(H)]
lines.append('static const u16 sGardenHedgeOpen[2] = {0x%04X, 0x%04X};   // grass, flowers' % (GRASS, FLOWER))
for name, (r, c, h, w, fill) in GROWTH.items():
    lines.append('#define GROWTH_%s_X %d' % (name, c))
    lines.append('#define GROWTH_%s_Y %d' % (name, r))
    lines.append('#define GROWTH_%s_W %d' % (name, w))
    lines.append('#define GROWTH_%s_H %d' % (name, h))
lines.append('#define GARDEN_TALL_GRASS 0x%04X' % mt(0x00d))
lines.append('#define GARDEN_SAND 0x%04X' % mt(0x11c))
lines.append('static const u16 sGardenPuddles[3][3] = %s;' % arr(PUDDLE_SET))
lines.append('static const u16 sGardenTree[2][2] = %s;' % arr(TREE_SET))
lines += ['', '#endif // GUARD_MP_GARDEN_MAP_H', '']
open(os.path.join(ROOT, 'src/mp/mp_garden_map.h'), 'w').write('\n'.join(lines))
for row in g:
    print(' '.join('%03x%s' % (v & 0x3ff, '#' if v & 0xc00 else '.') for v in row))
