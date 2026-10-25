#!/usr/bin/env python3
"""MOeMON GARDEN map: data/layouts/MoemonGarden/{map,border}.bin and src/mp/mp_garden_map.h.

Every POKEMON CENTER's second floor: its escalator leads up into this meadow inside the CENTER, and the escalator
at the Garden's west edge leads back down to the CENTER you came from (mp_garden.c MP_Garden_RedirectWarp). Built
from pieces of Littleroot Town and Petalburg City (General + gTileset_MoemonGarden, which is Petalburg's tileset with
the CENTER's escalator and floor added: gen_tileset.py), so every metatile keeps its own collision and elevation
bits. 40 x 25, one open meadow (cols 2-37, rows 2-23). Growth marks (patches that appear as the journey goes on) are
written by mp_garden.c at load time from the GROWTH_* rectangles here.
"""
import os, random, struct

ROOT = os.path.join(os.path.dirname(__file__), '..', '..')
W, H = 40, 25   # (odd: every forest column ends on a whole tree above the bottom row of tree tops)

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

# Petalburg's dirt path pieces
P_TL, P_T, P_TR = mt(0x118), mt(0x119), mt(0x11a)
P_BL, P_B, P_BR = mt(0x128), mt(0x129), mt(0x12a)

# borders: top forest with its base row, left and right forest, bottom tree tops
for c in range(W):
    g[0][c] = forest_cell(0, c)
    g[1][c] = T_BASE_A if c % 2 == 0 else T_BASE_B
    g[H - 1][c] = T_CAP_A if c % 2 == 0 else T_CAP_B
for r in range(1, H - 1):
    for c in (0, 1, W - 2, W - 1):
        g[r][c] = forest_cell(r, c)
# the way in and out: the POKEMON CENTER's 2F escalator against the west edge, on a landing of the CENTER's floor
# (laid out like the CENTER's 2F: wall, escalator top, the escalator's two moving rows, wall, the wall's corner).
# The landing starts on an even row and ends on an odd one, so the forest above and below keeps whole trees.
PAD = (10, 0, 6, 5)   # row, col, height, width
pc2f = load('PokemonCenter_2F', 14)   # the CENTER's 2F: the pieces keep its collision and elevation bits
FLOOR, WALL, WALL_END = 0x2B0, 0x2B1, 0x2B2   # gen_tileset.py's imports of 0x202, 0x220, 0x228
def pc_mt(r, c, id_=None):
    v = pc2f[r][c]
    return (v & 0xFC00) | (id_ if id_ is not None else v & 0x3FF)
assert pc2f[7][2] & 0x3FF == 0x202 and pc2f[4][0] & 0x3FF == 0x220 and pc2f[9][0] & 0x3FF == 0x228
for r in range(PAD[0], PAD[0] + PAD[2]):
    for c in range(PAD[1], PAD[1] + PAD[3]):
        g[r][c] = pc_mt(7, 2, FLOOR)
ESC = (PAD[0] + 1, 0)   # the escalator's top left: the 2F's rows 5-7, with the floor beside it as it is there (the
for k in range(3):      # tile you step off onto has elevation 0, between the escalator's and the floor's)
    for c in range(4):
        v = pc2f[5 + k][c]
        g[ESC[0] + k][c] = pc_mt(5 + k, c, FLOOR if v & 0x3FF == 0x202 else None)
g[PAD[0]][0] = pc_mt(4, 0, WALL)
g[PAD[0] + 4][0] = pc_mt(8, 0, WALL)
g[PAD[0] + 5][0] = pc_mt(9, 0, WALL_END)
WARP = (ESC[0] + 1, 1)  # the escalator tile you step on (MB_DOWN_ESCALATOR), as on a CENTER's 2F
def stamp(src, sr, sc, h, w, dr, dc):
    for r in range(h):
        for c in range(w):
            g[dr + r][dc + c] = src[sr + r][sc + c]

# a lone tree, three rows (Petalburg's, rows 23-25 cols 12-13): its crown over open ground, then its top and bottom
# halves (people walk behind the crown)
TREE_SET = [[pb[23][12], pb[23][13]], [pb[24][12], pb[24][13]], [pb[25][12], pb[25][13]]]

def tree(r, c):
    for k in range(3):
        g[r + k][c], g[r + k][c + 1] = TREE_SET[k][0], TREE_SET[k][1]

def flowers(r, c, h, w, gaps=()):
    for rr in range(r, r + h):
        for cc in range(c, c + w):
            if (rr - r, cc - c) not in gaps:
                g[rr][cc] = FLOWER

# the pond (Petalburg City's, rows 2-9, cols 19-25)
POND = (3, 18, 8, 7)          # row, col, height, width
stamp(pb, 2, 19, 8, 7, POND[0], POND[1])
SHORE = pb[9][19]   # 0x002: Petalburg's pond ends in stairs (250 251) on its bottom row: plain shore here
for c in range(POND[1], POND[1] + 7):
    if g[POND[0] + 7][c] & 0x3FF in (0x250, 0x251):
        g[POND[0] + 7][c] = SHORE
# flower beds and a few trees (the Garden)
flowers(3, 32, 2, 5, gaps={(1, 4)})
flowers(20, 31, 2, 6, gaps={(1, 0)})
flowers(15, 21, 2, 3)
flowers(9, 26, 1, 3)
tree(13, 35)
tree(18, 24)
tree(9, 16)
tree(20, 17)
# the west side: flower beds and trees around the landing
flowers(3, 3, 2, 5, gaps={(1, 4)})
flowers(11, 8, 3, 4, gaps={(0, 0), (2, 3)})
flowers(19, 3, 2, 6)
flowers(6, 14, 1, 2)
flowers(17, 13, 2, 2, gaps={(1, 1)})
tree(6, 9)
tree(16, 7)
tree(3, 12)
tree(18, 11)
# the GARDEN BOARD (a sign beside the path: A in front of it opens the Garden menu)
BOARD = (9, 6)
g[BOARD[0]][BOARD[1]] = SIGN

# growth marks (mp_garden.c stamps these when their moment in the journey has come)
GROWTH = {
    'TALL_GRASS': (16, 27, 3, 4, [mt(0x00d)]),                 # Petalburg Woods behind them: foliage
    'PUDDLES':    (15, 17, 3, 3, None),                        # SLATEPORT: a damp corner (the 3x3 puddle set)
    'SAND':       (19, 20, 2, 4, [mt(0x11c)]),                 # the desert: a dig patch
    'GROVE':      (9, 32, 3, 4, None),                         # FORTREE: two trees to perch in
    'STONE':      (16, 34, 1, 1, None),                        # the WEATHER INSTITUTE: a storm stone
}
PUDDLE_SET = [[mt(0x0c8), mt(0x0c9), mt(0x0ca)], [mt(0x0d0), mt(0x0d1), mt(0x0d2)], [mt(0x0d8), mt(0x0d9), mt(0x0da)]]
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
         '#define GARDEN_X0 2', '#define GARDEN_X1 %d' % (W - 3),
         '#define GARDEN_Y0 2', '#define GARDEN_Y1 %d' % (H - 2),
         '#define GARDEN_BOARD_X %d' % BOARD[1], '#define GARDEN_BOARD_Y %d' % BOARD[0],
         '#define GARDEN_WARP_X %d   // the escalator (warp 0): in and out' % WARP[1],
         '#define GARDEN_WARP_Y %d' % WARP[0],
         '#define GARDEN_ENTRY_X %d   // the floor beside it, where the ride up ends' % (WARP[1] + 1),
         '#define GARDEN_ENTRY_Y %d' % WARP[0],
         '#define GARDEN_PAD_X0 %d   // the landing (no Garden Pokemon or decorations on it)' % PAD[1],
         '#define GARDEN_PAD_Y0 %d' % PAD[0],
         '#define GARDEN_PAD_X1 %d' % (PAD[1] + PAD[3] - 1), '#define GARDEN_PAD_Y1 %d' % (PAD[0] + PAD[2] - 1),
         '#define GARDEN_GRASS 0x%04X' % GRASS, '#define GARDEN_FLOWER 0x%04X' % FLOWER, '']
for name, (r, c, h, w, fill) in GROWTH.items():
    lines.append('#define GROWTH_%s_X %d' % (name, c))
    lines.append('#define GROWTH_%s_Y %d' % (name, r))
    lines.append('#define GROWTH_%s_W %d' % (name, w))
    lines.append('#define GROWTH_%s_H %d' % (name, h))
lines.append('#define GARDEN_TALL_GRASS 0x%04X' % mt(0x00d))
lines.append('#define GARDEN_SAND 0x%04X' % mt(0x11c))
lines.append('static const u16 sGardenPuddles[3][3] = %s;' % arr(PUDDLE_SET))
lines.append('static const u16 sGardenTree[3][2] = %s;   // a lone tree: crown, top, bottom' % arr(TREE_SET))
lines += ['', '#endif // GUARD_MP_GARDEN_MAP_H', '']
open(os.path.join(ROOT, 'src/mp/mp_garden_map.h'), 'w').write('\n'.join(lines))
# the map's events: the escalator (warp 0) and the GARDEN BOARD sit where the layout has them
import json
mj = os.path.join(ROOT, 'data/maps/MoemonGarden/map.json')
m = json.load(open(mj))
m['warp_events'] = [{'x': WARP[1], 'y': WARP[0], 'elevation': 0, 'dest_map': 'MAP_OLDALE_TOWN_POKEMON_CENTER_1F', 'dest_warp_id': '2'}]
m['bg_events'][0]['x'], m['bg_events'][0]['y'] = BOARD[1], BOARD[0]
open(mj, 'w').write(json.dumps(m, indent=2) + '\n')
for row in g:
    print(' '.join('%03x%s' % (v & 0x3ff, '#' if v & 0xc00 else '.') for v in row))
