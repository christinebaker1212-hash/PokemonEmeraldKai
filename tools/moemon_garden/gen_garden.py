#!/usr/bin/env python3
"""MOeMON GARDEN: its map, its tileset and the facts the game needs (src/mp/mp_garden_map.h).

Every POKEMON CENTER's second floor. The escalator on the west edge leads up into it and back down (mp_garden.c).
The Garden has a place for every type, and each badge the group holds adds one (the GROWTH table: the game stamps
those metatiles when the group's badges reach them). Everything is drawn with the original game's own pieces, put
together the way its maps put them together:
  - trees: the 2x2 trees of LITTLEROOT / ROUTE 101-102, joined by the rules those maps follow (TREE_* below);
  - sand paths: the 3x3 path set as ROUTE 117 joins it (an inside corner is plain sand); General's pale grass and
    FORTREE's marsh grass the same way; ponds like ROUTE 117's (rim on the top and sides, a shore lip below);
  - copied whole: ROUTE 117's flower beds and rock outcrop, LAVARIDGE's hot spring, ROUTE 120's marsh ponds and long
    grass, ROUTE 114's mossy boulders, ROUTE 117's dark hollow, ROUTE 101's ledge;
  - changed to fit (CUSTOM): cave crystals and a FALLARBOR rock lifted onto grass, the hot spring's top rim on grass.

Writes data/layouts/MoemonGarden/{map,border}.bin, data/tilesets/secondary/moemon_garden/*, the map's events in
data/maps/MoemonGarden/map.json, the tile count in src/data/tilesets/graphics.h and src/mp/mp_garden_map.h.
Run with --png OUT_DIR to also draw every stage (0-8 badges).
"""
import json, os, re, struct, sys
from collections import Counter, defaultdict
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import garden_lib as L

ROOT = L.ROOT
W, H = 56, 40    # H even: the border repeats the forest by row parity (top halves on even rows)
TW, TH = W // 2, H // 2

# ---------------------------------------------------------------------------
# Default collision / elevation bits of General's metatiles: what the original maps give each one most often

_bits = None
def bits(mid):
    global _bits
    if _bits is None:
        c = defaultdict(Counter)
        for n in L.layouts():
            try:
                g, w, h, p, s = L.load_layout(n)
            except Exception:
                continue
            if p != 'general':
                continue
            for row in g:
                for v in row:
                    if v & 0x3FF < 0x200:
                        c[v & 0x3FF][v & 0xFC00] += 1
        _bits = {m: cc.most_common(1)[0][0] for m, cc in c.items()}
    return _bits[mid]

def gen(mid):
    return (bits(mid), mid)

_sbits = {}
def sec_bits(folder, mid):
    """What the original maps on that secondary tileset give the metatile most often."""
    if folder not in _sbits:
        c = defaultdict(Counter)
        for n in L.layouts():
            try:
                g, w, h, p, s = L.load_layout(n)
            except Exception:
                continue
            if p != 'general' or s != folder:
                continue
            for row in g:
                for v in row:
                    c[v & 0x3FF][v & 0xFC00] += 1
        _sbits[folder] = {m: cc.most_common(1)[0][0] for m, cc in c.items()}
    return _sbits[folder].get(mid)

def SEC(folder, mid, b=None):
    """A secondary piece with the bits its own maps give it (b: checked against them; used for a piece no map has)."""
    got = sec_bits(folder, mid)
    if got is None:
        assert b is not None, '%s 0x%03X: on no map, and no bits given' % (folder, mid)
        got = b
    assert b is None or b == got, '%s 0x%03X: bits 0x%04X, not 0x%04X' % (folder, mid, got, b)
    return (got, (folder, 'general', mid))

# ---------------------------------------------------------------------------
# Pieces

GRASS, FLOWER, SIGN, TALL, LONG, SHORE = 0x001, 0x004, 0x003, 0x00D, 0x015, 0x002
PATH = {  # (N, S, W, E open) -> piece; ROUTE 117 joins it so: an inside corner is plain sand (0x121)
    'TL': 0x118, 'T': 0x119, 'TR': 0x11A, 'L': 0x120, 'C': 0x121, 'R': 0x122, 'BL': 0x128, 'B': 0x129, 'BR': 0x12A}
DIG = 0x124       # sand you can dig in (MB_SAND)
PALE = {'TL': 0x1D0, 'T': 0x1D1, 'TR': 0x1D2, 'L': 0x1D8, 'C': 0x1D9, 'R': 0x1DA, 'BL': 0x1E0, 'B': 0x1E1, 'BR': 0x1E2}
MARSH = {'TL': 0x288, 'T': 0x289, 'TR': 0x28A, 'L': 0x290, 'C': 0x291, 'R': 0x292, 'BL': 0x298, 'B': 0x299, 'BR': 0x29A}
POND = {'TL': 0x0B0, 'T': 0x0B1, 'TR': 0x0B2, 'L': 0x0B8, 'C': 0x0A1, 'R': 0x0BA, 'IN_NE': 0x0C2, 'IN_SW': 0x0CC}
TREE_TOP = {(1, 0): 0x1D4, (1, 1): 0x1D5, (0, 0): 0x1D6, (0, 1): 0x1D7}       # (joined that side?, right half?)
TREE_BOT_ON = (0x1DC, 0x1DD)                                                   # another tree below
TREE_BOT = {(1, 0): 0x1E4, (1, 1): 0x1E5, (0, 0): 0x1E6, (0, 1): 0x1E7}
CROWN = {GRASS: (0x1CE, 0x1CF), TALL: (0x1C6, 0x1C7)}
CROWN_PATH = ('mauville', 0x278, 0x279)                                        # over a path's bottom edge (ROUTE 117)
LEDGE = (0x0D5, 0x087, 0x0D6)                                                  # ROUTE 101's ledge: west end, middle, east end
DARK_TOP = (0x14B, 0x14C, 0x14D)                                               # ROUTE 117's dark hollow (and its lip)
DARK_LIP = (0x113, 0x114, 0x115)
PUDDLE = [[0x0C8, 0x0C9, 0x0CA], [0x0D0, 0x0D1, 0x0D2], [0x0D8, 0x0D9, 0x0DA]]


def nine(table, n, s, w, e):
    if not n:
        return table['TL'] if not w else table['TR'] if not e else table['T']
    if not s:
        return table['BL'] if not w else table['BR'] if not e else table['B']
    return table['L'] if not w else table['R'] if not e else table['C']

# ---------------------------------------------------------------------------
# Copied whole from the original maps (with their own bits)

def chunk(layout, x0, y0, w, h):
    return L.chunk(layout, x0, y0, w, h)

FLOWER_BED = chunk('Route117', 7, 3, 8, 5)      # gray fence round MAUVILLE's flowers
OUTCROP = chunk('Route117', 32, 1, 6, 4)        # two rock peaks (its top row is below)
OUTCROP_TOP = chunk('Route117', 33, 0, 3, 1)    # the left peak's tip (MAUVILLE 0x235-0x237)
# (ROUTE 117's bushes behind it kept anyone off its top; here open grass is behind it: every piece of it is solid)
solid = lambda ch: [[((c[0] & 0xF000) | 0x0400, c[1]) for c in row] for row in ch]
OUTCROP, OUTCROP_TOP = solid(OUTCROP), solid(OUTCROP_TOP)
SPRING = chunk('LavaridgeTown', 2, 2, 6, 5)     # the hot spring
BOULDER = [[SEC('fallarbor', 0x238), SEC('fallarbor', 0x23A)],
           [SEC('fallarbor', 0x248), SEC('fallarbor', 0x24A)]]
MARSH_POND = [[SEC('fortree', 0x274), SEC('fortree', 0x275)],
              [SEC('fortree', 0x27C), SEC('fortree', 0x27D)]]
BUSH = SEC('fortree', 0x276)
LONG_EDGE = SEC('fortree', 0x208)       # the long grass's south edge (ROUTE 120)
PUDDLE_STAMP = [[gen(v) for v in row] for row in PUDDLE]

def check_chunks():
    ids = lambda ch: [[c[1] if isinstance(c[1], int) else c[1][2] for c in row] for row in ch]
    assert ids(FLOWER_BED)[0][0] == 0x138 and ids(FLOWER_BED)[1][1] == 0x215, ids(FLOWER_BED)
    assert ids(OUTCROP)[0] == [0x068, 0x073, 0x071, 0x075, 0x069, 0x06A], ids(OUTCROP)
    assert ids(SPRING)[0][0] == 0x299 and ids(SPRING)[4][5] == 0x2B5, ids(SPRING)
    for row in BOULDER + MARSH_POND:
        for b, (f, p, m) in row:
            assert m - 0x200 < len(L.tileset(f, False)['metas'])
check_chunks()

# ---------------------------------------------------------------------------
# Changed to fit (CUSTOM metatiles)

LAYER_NORMAL, LAYER_COVERED = 0, 1

def grass_bottom():
    return [('keep', e) for e in L.tileset('general', True)['metas'][GRASS][:4]]

def lifted(name, ref, keep_colors=None, floor=None):
    """A crystal / rock taken off its own floor and set on grass: its floor's colours become see-through. floor:
    the colours to drop (default: the colours round the picture's edge that aren't the object's)."""
    img = L.meta_image(ref)
    if floor is None:
        ring = Counter()
        for i in range(16):
            for c in (img[0][i], img[15][i], img[i][0], img[i][15]):
                ring[c] += 1
        floor = {c for c, n in ring.items() if c is not None and n >= 3}
        if keep_colors:
            floor -= set(keep_colors)
    out = [[(None if c in floor else c) for c in row] for row in img]
    # (both layers under the sprites: a Pokemon walking north of it is never covered by it)
    return L.custom(name, grass_bottom(), L.split4(out), LAYER_COVERED << 12)

def is_blue(c):
    return c is not None and c[2] > c[0] + 40

def is_red(c):
    return c is not None and c[0] > c[2] + 70 and c[0] > c[1] + 70

def crystal_keep(ref):
    img = L.meta_image(ref)
    return {c for row in img for c in row if is_blue(c) or is_red(c) or (c is not None and min(c) > 200)}

CRYSTAL_BLUE = ('cave', 'general', 0x31E)
CRYSTAL_BLUE2 = ('cave', 'general', 0x31F)
CRYSTAL_RED = ('cave', 'general', 0x349)
CRYSTAL_RED2 = ('cave', 'general', 0x34E)

def lift_by(name, ref, drop):
    """Like lifted(), the colours to drop chosen by a test."""
    img = L.meta_image(ref)
    out = [[(None if (c is None or drop(c)) else c) for c in row] for row in img]
    return L.custom(name, grass_bottom(), L.split4(out), LAYER_COVERED << 12)

def is_green(c):
    return c[1] > c[0] + 12 and c[1] >= c[2]

STONE_IN_POND = 0x0C5      # General: a gray boulder in a pond (its water dropped: a gray stone on grass)
FOAM = (222, 230, 238)     # (the white ring round it where it meets the water)

def make_customs():
    c = {}
    for name, ref in (('crystal_blue', CRYSTAL_BLUE), ('crystal_blue2', CRYSTAL_BLUE2), ('crystal_red', CRYSTAL_RED),
                      ('crystal_red2', CRYSTAL_RED2)):
        c[name] = (0x0400, lifted(name, ref, keep_colors=crystal_keep(ref)))
    water = {px for row in L.meta_image(POND['C']) for px in row if px is not None}
    c['stone'] = (0x0400, lift_by('stone', ('general_as_ref', STONE_IN_POND), lambda px: px in water or px == FOAM))
    # ROUTE 117's outcrop: the left peak's tip stands in front of bushes there; here on grass
    rock = {px for m in (0x071, 0x073, 0x07B, 0x07C) for row in L.meta_image(m) for px in row if px is not None}
    for i, (b, ref) in enumerate(OUTCROP_TOP[0]):
        c['tip%d' % i] = (b, lift_by('tip%d' % i, ref, lambda px: px not in rock))
    # the hot spring's top rim: LAVARIDGE has its cliff behind it (the bottom layer); here grass. Its wooden steps
    # stay: from the grass above you walk down into the water, as LAVARIDGE's visitors do from the cliff
    for i, (b, ref) in enumerate(SPRING[0]):
        top = L.split4(L.meta_image(ref, 1))
        c['rim%d' % i] = (b, L.custom('rim%d' % i, grass_bottom(), top, LAYER_COVERED << 12))
    return c

CUSTOMS = make_customs()
SPRING_FIXED = [[CUSTOMS['rim%d' % i] for i in range(6)]] + SPRING[1:]
OUTCROP_TIP = [[CUSTOMS['tip%d' % i] for i in range(3)]]

# ---------------------------------------------------------------------------
# The plan: the ground (a letter per metatile), the trees (a 2x2 grid), and what's stamped on top

class Plan:
    def __init__(self):
        self.ground = [['.'] * W for _ in range(H)]
        self.trees = set()
        self.stamps = []       # (x, y, rows of cells)
        self.cells = {}        # (x, y) -> cell, after the stamps

    def copy(self):
        p = Plan()
        p.ground = [row[:] for row in self.ground]
        p.trees = set(self.trees)
        p.stamps = list(self.stamps)
        p.cells = dict(self.cells)
        return p

    def fill(self, x0, y0, x1, y1, k):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.ground[y][x] = k

    def fill_over(self, x0, y0, x1, y1, k, over='.g,'):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                if self.ground[y][x] in over:
                    self.ground[y][x] = k

    def put(self, pts, k):
        for x, y in pts:
            self.ground[y][x] = k

    def tree(self, tx, ty):
        self.trees.add((tx, ty))

    def no_tree(self, tx, ty):
        self.trees.discard((tx, ty))

    def stamp(self, x, y, rows):
        self.stamps.append((x, y, rows))

    def cell(self, x, y, c):
        self.cells[(x, y)] = c

def has_tree(plan, tx, ty):
    if tx < 0 or ty < 0 or tx >= TW or ty >= TH:
        return (tx, ty) not in OPEN_EDGE
    return (tx, ty) in plan.trees

OPEN_EDGE = set()

def render(plan):
    g = [[None] * W for _ in range(H)]
    G = plan.ground
    def k(x, y):
        return G[y][x] if 0 <= x < W and 0 <= y < H else None
    def same(x, y, cls):
        if not (0 <= x < W and 0 <= y < H):
            return True
        return G[y][x] in cls
    for y in range(H):
        for x in range(W):
            c = G[y][x]
            if c == '.':
                g[y][x] = gen(GRASS)
            elif c == ',':
                g[y][x] = gen(FLOWER)
            elif c == 'g':
                g[y][x] = gen(TALL)
            elif c == 'l':
                g[y][x] = LONG_EDGE if k(x, y + 1) != 'l' else gen(LONG)
            elif c in 'sd':
                n, s, w, e = (same(x, y - 1, 'sd'), same(x, y + 1, 'sd'), same(x - 1, y, 'sd'), same(x + 1, y, 'sd'))
                pc = nine(PATH, n, s, w, e)
                if c == 'd' and pc == PATH['C']:
                    pc = DIG
                g[y][x] = gen(pc)
            elif c == 'p':
                g[y][x] = gen(nine(PALE, same(x, y - 1, 'p'), same(x, y + 1, 'p'), same(x - 1, y, 'p'), same(x + 1, y, 'p')))
            elif c == 'm':
                m = nine(MARSH, same(x, y - 1, 'm'), same(x, y + 1, 'm'), same(x - 1, y, 'm'), same(x + 1, y, 'm'))
                g[y][x] = SEC('fortree', m, 0x3000)
            elif c == 'w':
                N, S, Wt, E = same(x, y - 1, 'w'), same(x, y + 1, 'w'), same(x - 1, y, 'w'), same(x + 1, y, 'w')
                if not N:
                    pc = POND['TL'] if not Wt else POND['TR'] if not E else POND['T']
                elif not Wt:
                    pc = POND['IN_SW'] if same(x - 1, y - 1, 'w') else POND['L']
                elif not E:
                    pc = POND['R']
                elif not same(x + 1, y - 1, 'w'):
                    pc = POND['IN_NE']
                else:
                    pc = POND['C']
                g[y][x] = gen(pc)
            else:
                raise SystemExit('ground %r at %d,%d' % (c, x, y))
    # the shore lip under a pond (ROUTE 117: 0x002 below the water's last row)
    for y in range(1, H):
        for x in range(W):
            if G[y - 1][x] == 'w' and G[y][x] != 'w':
                assert G[y][x] == '.', 'under the pond at %d,%d: %r' % (x, y, G[y][x])
                g[y][x] = gen(SHORE)
    # trees
    for (tx, ty) in plan.trees:
        L_, R_, U, D = (has_tree(plan, tx - 1, ty), has_tree(plan, tx + 1, ty), has_tree(plan, tx, ty - 1),
                        has_tree(plan, tx, ty + 1))
        x, y = tx * 2, ty * 2
        g[y][x] = gen(TREE_TOP[(1 if (L_ or U) else 0, 0)])
        g[y][x + 1] = gen(TREE_TOP[(1 if (R_ or U) else 0, 1)])
        if D:
            g[y + 1][x], g[y + 1][x + 1] = gen(TREE_BOT_ON[0]), gen(TREE_BOT_ON[1])
        else:
            g[y + 1][x] = gen(TREE_BOT[(1 if L_ else 0, 0)])
            g[y + 1][x + 1] = gen(TREE_BOT[(1 if R_ else 0, 1)])
        if not U and y > 0:
            for side in (0, 1):
                under = g[y - 1][x + side]
                if under[1] in CROWN:
                    g[y - 1][x + side] = gen(CROWN[under[1]][side])
                elif under[1] == PATH['B']:
                    g[y - 1][x + side] = SEC(CROWN_PATH[0], CROWN_PATH[1 + side])
                else:
                    raise SystemExit('a crown over %r at %d,%d' % (under, x + side, y - 1))
    for x0, y0, rows in plan.stamps:
        for dy, row in enumerate(rows):
            for dx, c in enumerate(row):
                if c is not None:
                    g[y0 + dy][x0 + dx] = c
    for (x, y), c in plan.cells.items():
        g[y][x] = c
    return g

# ---------------------------------------------------------------------------
# The Garden, stage by stage

def rect_ring(x0, y0, x1, y1):
    return [(x, y0) for x in range(x0, x1 + 1)] + [(x, y1) for x in range(x0, x1 + 1)] + \
           [(x0, y) for y in range(y0 + 1, y1)] + [(x1, y) for y in range(y0 + 1, y1)]

PAD = (0, 18, 4, 24)       # the escalator's landing: x0, y0, x1, y1 (the CENTER's floor)
BOARD = (6, 17)            # the GARDEN BOARD (a sign)
STONE = (29, 13)           # the storm stone (a sprite: mp_garden_decor.c)
ZONES = []                 # (need, x0, y0, x1, y1, badges, face x, face y)

def zone(need, x0, y0, x1, y1, badges=0, face=None):
    ZONES.append((need, x0, y0, x1, y1, badges, face))

def base():
    p = Plan()
    # the forest round it: the outer ring of trees
    for tx in range(TW):
        p.tree(tx, 0)
        p.tree(tx, TH - 1)
    for ty in range(TH):
        p.tree(0, ty)
        p.tree(TW - 1, ty)
    # the landing: the west column opens for it (rows 18-25: four tree rows, the last one's crown row free)
    for ty in range(PAD[1] // 2, PAD[1] // 2 + 4):
        p.no_tree(0, ty)
    # ---- the main sand path: from the landing east to the hot spring, a branch north and one south
    p.fill(5, 20, 24, 21, 's')          # (stepping down a row halfway, as ROUTE 117's paths do)
    p.fill(22, 21, 44, 22, 's')
    p.fill(20, 11, 21, 19, 's')         # north, to the standing stone and the pond
    p.fill(36, 4, 37, 20, 's')          # north-east, up to the plateau's way in
    p.fill(28, 23, 29, 29, 's')         # south, to the training ground and the long grass
    # ---- NORMAL: a cozy nook of pale grass by the landing, sheltered by trees
    p.fill(6, 12, 11, 16, 'p')
    for tx, ty in ((3, 5), (4, 5), (5, 5), (2, 6), (2, 7)):
        p.tree(tx, ty)
    zone('COZY', 6, 12, 11, 16)
    # ---- GRASS: ROUTE 117's flower bed (a second one at badge 7)
    p.stamp(4, 3, FLOWER_BED)
    zone('SUN', 4, 8, 11, 9, face=(7, 6))
    # ---- WATER: the pond (ROUTE 117's shape, its steps down to the east)
    p.fill(23, 3, 30, 4, 'w')
    p.fill(23, 5, 31, 6, 'w')
    p.fill(25, 7, 32, 8, 'w')
    zone('SOAK', 22, 2, 34, 10)
    # ---- PSYCHIC: a standing stone in a ring of flowers (the circle closes at badge 8)
    p.put(rect_ring(14, 11, 18, 15), ',')
    p.cell(16, 13, CUSTOMS['stone'])
    zone('STILL', 15, 12, 17, 14, face=(16, 13))
    # ---- FLYING: the open meadow (a few flowers); ELECTRIC: the storm stone in it
    p.put([(25, 14), (26, 14), (33, 12), (24, 17), (31, 17), (32, 17)], ',')
    zone('SKY', 23, 12, 33, 18)
    zone('STORM', STONE[0] - 1, STONE[1] - 1, STONE[0] + 1, STONE[1] + 1, face=STONE)
    # ---- ROCK and DRAGON: a plateau with ROUTE 117's rock outcrop, above ROUTE 101's ledge
    for x in range(38, 54):
        p.cell(x, 10, gen(LEDGE[0] if x == 38 else LEDGE[2] if x == 53 else LEDGE[1]))
    p.stamp(44, 4, OUTCROP)
    p.stamp(45, 3, OUTCROP_TIP)
    p.stamp(40, 6, BOULDER)
    zone('STONE', 39, 4, 43, 9, face=(41, 6))
    zone('HEIGHT', 40, 9, 52, 9)
    # ---- STEEL: a shiny spot (crystals by gray rocks) below the ledge
    p.cell(41, 13, CUSTOMS['crystal_blue'])
    p.cell(43, 12, CUSTOMS['stone'])
    p.cell(40, 15, CUSTOMS['stone'])
    zone('SHINE', 39, 12, 44, 16)
    # ---- FIRE: LAVARIDGE's hot spring, on a sand apron
    p.fill(42, 17, 52, 21, 's')
    p.fill(44, 12, 51, 16, 's')      # (under the spring too: the sand round it joins up)
    p.stamp(46, 12, SPRING_FIXED)
    zone('BASK', 44, 13, 52, 18, face=(48, 14))
    # ---- FIGHTING: the training ground (a sand clearing off the south path)
    p.fill(24, 23, 31, 27, 's')
    zone('SPAR', 24, 23, 31, 27)
    # ---- GROUND: a patch of soft sand to dig in (a dig field at badge 4)
    p.fill(34, 25, 36, 27, 'd')
    zone('DIG', 33, 24, 37, 28)
    # ---- BUG: a patch of tall grass (a long grass field at badge 1)
    p.fill(22, 32, 27, 35, 'g')
    zone('FOLIAGE', 21, 31, 28, 36)
    # ---- POISON: a puddle (a marsh at badge 2)
    p.stamp(44, 25, PUDDLE_STAMP)
    zone('DAMP', 43, 24, 47, 28)
    # ---- GHOST / DARK: ROUTE 117's tree hollow with its dark patch, open to the south
    for tx in range(2, 8):
        p.tree(tx, 14)
    for ty in (15, 16):
        for tx in (2, 3, 6, 7):
            p.tree(tx, ty)
    p.stamp(8, 31, [[gen(DARK_TOP[0]), gen(DARK_TOP[1]), gen(DARK_TOP[1]), gen(DARK_TOP[2])],
                    [gen(DARK_LIP[0]), gen(DARK_LIP[1]), gen(DARK_LIP[1]), gen(DARK_LIP[2])]])
    zone('SHADE', 8, 30, 11, 34, face=(9, 31))
    zone('NIGHT', 8, 30, 11, 34, face=(10, 31))
    # ---- ICE: a shaded spring in the trees (south-east), crystals of frost by it
    for tx in (22, 23, 25, 26):
        p.tree(tx, 15)
    for ty in (16, 17, 18):
        p.tree(21, ty)
    p.fill(46, 33, 51, 35, 'w')
    p.cell(45, 33, CUSTOMS['crystal_blue2'])
    zone('COOL', 44, 32, 52, 37, face=(48, 34))
    # ---- the rest of the meadow, as the original routes dress theirs: the forest reaching in here and there, loose
    # flowers in twos and threes (General's and MAUVILLE's, as ROUTE 117 mixes them), tufts of tall grass by the trees
    for tx, ty in ((1, 1), (1, 2), (17, 1), (2, 17), (2, 18), (3, 18), (7, 18)):
        p.tree(tx, ty)
    loose = {',': [(13, 9), (14, 9), (19, 9), (22, 11), (23, 10), (34, 9), (35, 10), (12, 18), (13, 19), (17, 17),
                   (8, 25), (9, 26), (20, 27), (21, 26), (15, 23), (36, 28), (37, 29), (16, 33), (50, 11), (39, 18),
                   (40, 19), (34, 15), (35, 16)],
             'g': [(2, 9), (3, 9), (2, 10), (3, 10), (2, 11), (14, 25), (15, 25), (14, 26), (15, 26), (16, 26),
                   (52, 22), (53, 22), (52, 23), (53, 23), (53, 24), (5, 34), (5, 35), (6, 35), (52, 9), (53, 9)]}
    for k, pts in loose.items():
        for x, y in pts:
            assert p.ground[y][x] == '.', 'dressing over %r at %d,%d' % (p.ground[y][x], x, y)
            p.ground[y][x] = k
    for (x, y), m in (((22, 9), 0x213), ((12, 24), 0x211), ((19, 24), 0x208), ((42, 30), 0x20A), ((13, 34), 0x208),
                      ((35, 12), 0x20C)):
        assert p.ground[y][x] == '.', (x, y, p.ground[y][x])
        p.cell(x, y, SEC('mauville', m, 0x3000))
    return p

def growth(p, badge):
    """What badge `badge` adds (on top of the stage before)."""
    if badge == 1:      # STONE BADGE (through PETALBURG WOODS): BUG's long grass field round the tall grass
        for y in range(30, 38):
            for x in range(17, 35):
                if y == 37:
                    continue   # (the forest's crowns stand on that row: grass)
                if p.ground[y][x] == '.' and not (28 <= x <= 29 and y <= 31):
                    p.ground[y][x] = 'l'
        zone('FOLIAGE', 17, 30, 34, 37, badges=1)
    elif badge == 2:    # KNUCKLE BADGE (the sea air): POISON's marsh round the puddle, ponds and mud
        p.stamps.remove((44, 25, PUDDLE_STAMP))
        for y, (xa, xb) in zip(range(23, 29), ((40, 50), (39, 52), (38, 52), (38, 52), (38, 52), (39, 51))):
            p.fill_over(xa, y, xb, y, 'm')
        p.stamp(44, 25, MARSH_POND)
        p.stamp(40, 24, MARSH_POND)
        p.stamp(49, 27, MARSH_POND)
        p.cell(48, 24, BUSH)
        p.cell(42, 26, BUSH)
        zone('DAMP', 38, 23, 52, 28, badges=2)
    elif badge == 3:    # DYNAMO BADGE: ROCK's boulder field on the plateau
        p.stamp(48, 6, BOULDER)
        p.stamp(51, 3, BOULDER)
        p.stamp(39, 2, BOULDER)
        p.cell(50, 8, CUSTOMS['stone'])
        p.cell(42, 3, CUSTOMS['stone'])
        zone('STONE', 39, 2, 53, 9, badges=3)
    elif badge == 4:    # HEAT BADGE (the desert beyond): GROUND's dig field
        p.fill(33, 24, 37, 29, 'd')
        zone('DIG', 33, 24, 37, 29, badges=4)
    elif badge == 5:    # BALANCE BADGE: STEEL's shiny spot grows
        for (x, y), c in (((39, 13), 'crystal_red'), ((42, 15), 'crystal_blue2'), ((43, 14), 'crystal_red2'),
                          ((38, 12), 'stone')):
            p.cell(x, y, CUSTOMS[c])
        zone('SHINE', 38, 12, 44, 16, badges=5)
    elif badge == 6:    # FEATHER BADGE (FORTREE): FLYING's perch grove
        for tx, ty in ((16, 6), (13, 8)):
            p.tree(tx, ty)
        zone('SKY', 30, 11, 38, 19, badges=6)
    elif badge == 7:    # MIND BADGE: GRASS's second flower bed
        p.stamp(13, 3, FLOWER_BED)
        zone('SUN', 13, 8, 20, 9, badges=7, face=(16, 6))
    elif badge == 8:    # RAIN BADGE: PSYCHIC's stone circle
        for x, y in ((14, 11), (18, 11), (14, 15), (18, 15)):
            p.cell(x, y, CUSTOMS['stone'])
        zone('STILL', 15, 12, 17, 14, badges=8, face=(16, 13))

def stages():
    p = base()
    out = [p.copy()]
    for b in range(1, 9):
        growth(p, b)
        out.append(p.copy())
    return out

# ---------------------------------------------------------------------------
# The landing (the CENTER's 2F escalator against the west edge) and the board: on every stage

def landing(g):
    pc2f, w2, h2, prim, sec = L.load_layout('PokemonCenter_2F')
    def pc(r, c):
        v = pc2f[r][c]
        return (v & 0xFC00, ('pokemon_center', 'building', v & 0x3FF))
    assert pc2f[7][2] & 0x3FF == 0x202 and pc2f[4][0] & 0x3FF == 0x220 and pc2f[9][0] & 0x3FF == 0x228
    floor = pc(7, 2)
    x0, y0, x1, y1 = PAD
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            g[y][x] = floor
    esc = y0 + 1   # the escalator: the 2F's rows 5-7 (with the floor beside it as it is there: the tile you step
    for k in range(3):   # off onto has elevation 0, between the escalator's and the floor's)
        for c in range(4):
            v = pc2f[5 + k][c]
            g[esc + k][c] = (v & 0xFC00, ('pokemon_center', 'building', v & 0x3FF))
    g[y0][0] = pc(4, 0)
    g[y0 + 4][0] = pc(8, 0)
    g[y0 + 5][0] = pc(8, 0)
    g[y0 + 6][0] = pc(9, 0)
    return (1, esc + 1)    # the warp: the escalator tile you step on (MB_DOWN_ESCALATOR), as on a CENTER's 2F

ESCALATOR_IDS = [0x298, 0x299] + list(range(0x2A0, 0x2A6)) + list(range(0x2A8, 0x2AE))

def finish(g):
    warp = landing(g)
    g[BOARD[1]][BOARD[0]] = gen(SIGN)
    return warp

# ---------------------------------------------------------------------------

def main():
    st = stages()
    grids = []
    for p in st:
        g = render(p)
        warp = finish(g)
        grids.append(g)
    # every secondary piece of every stage, the escalator's frames at their own ids
    refs = set()
    for g in grids:
        for row in g:
            for b, r in row:
                if not isinstance(r, int):
                    refs.add(r)
    fixed = {('pokemon_center', 'building', i): i for i in ESCALATOR_IDS}
    out_dir = os.path.join(ROOT, 'data/tilesets/secondary/moemon_garden')
    ids, stats = L.build_tileset(refs, fixed, out_dir)
    def raw(c):
        b, r = c
        return b | (r if isinstance(r, int) else ids[r])
    rawg = [[[raw(c) for c in row] for row in g] for g in grids]
    # the growth table: what changes from one stage to the next
    grow = []
    for b in range(1, 9):
        for y in range(H):
            for x in range(W):
                if rawg[b][y][x] != rawg[b - 1][y][x]:
                    grow.append((b, x, y, rawg[b][y][x]))
    # files
    with open(os.path.join(ROOT, 'data/layouts/MoemonGarden/map.bin'), 'wb') as f:
        for row in rawg[0]:
            f.write(struct.pack('<%dH' % W, *row))
    with open(os.path.join(ROOT, 'data/layouts/LittlerootTown/border.bin'), 'rb') as f:
        border = f.read()
    with open(os.path.join(ROOT, 'data/layouts/MoemonGarden/border.bin'), 'wb') as f:
        f.write(border)
    lj = os.path.join(ROOT, 'data/layouts/layouts.json')
    txt = open(lj).read()
    i = txt.index('"name": "MoemonGarden_Layout"')
    seg_end = txt.index('}', i)
    seg = txt[i:seg_end]
    seg2 = re.sub(r'"width": \d+', '"width": %d' % W, seg)
    seg2 = re.sub(r'"height": \d+', '"height": %d' % H, seg2)
    open(lj, 'w').write(txt[:i] + seg2 + txt[seg_end:])
    gh = os.path.join(ROOT, 'src/data/tilesets/graphics.h')
    t = open(gh).read()
    t = re.sub(r'(moemon_garden/tiles\.png", "\.4bpp\.lz", "-num_tiles )\d+', r'\g<1>%d' % stats['tiles'], t)
    open(gh, 'w').write(t)
    mj = os.path.join(ROOT, 'data/maps/MoemonGarden/map.json')
    m = json.load(open(mj))
    m['warp_events'] = [{'x': warp[0], 'y': warp[1], 'elevation': 0, 'dest_map': 'MAP_OLDALE_TOWN_POKEMON_CENTER_1F', 'dest_warp_id': '2'}]
    m['bg_events'][0]['x'], m['bg_events'][0]['y'] = BOARD
    open(mj, 'w').write(json.dumps(m, indent=2) + '\n')
    write_header(warp, grow, rawg[0])
    print('moemon_garden: %d metatiles, %d tiles, palettes %s; %d growth cells' % (stats['metatiles'], stats['tiles'], stats['slots'], len(grow)))
    if '--png' in sys.argv:
        d = sys.argv[sys.argv.index('--png') + 1]
        os.makedirs(d, exist_ok=True)
        for b, g in enumerate(rawg):
            draw(g, os.path.join(d, 'stage%d.png' % b))

NEED_NAMES = ['COZY', 'SPAR', 'SKY', 'DAMP', 'DIG', 'STONE', 'FOLIAGE', 'SHADE', 'SHINE', 'BASK', 'SOAK', 'SUN',
              'STORM', 'STILL', 'COOL', 'HEIGHT', 'NIGHT']

def write_header(warp, grow, g0):
    x0, y0, x1, y1 = PAD
    lines = ['// Generated by tools/moemon_garden/gen_garden.py (the MOeMON GARDEN map\'s layout facts). Do not edit.',
             '#ifndef GUARD_MP_GARDEN_MAP_H', '#define GUARD_MP_GARDEN_MAP_H', '',
             '#define GARDEN_W %d' % W, '#define GARDEN_H %d' % H,
             '#define GARDEN_X0 2', '#define GARDEN_X1 %d' % (W - 3), '#define GARDEN_Y0 2', '#define GARDEN_Y1 %d' % (H - 3),
             '#define GARDEN_BOARD_X %d' % BOARD[0], '#define GARDEN_BOARD_Y %d' % BOARD[1],
             '#define GARDEN_WARP_X %d   // the escalator (warp 0): in and out' % warp[0],
             '#define GARDEN_WARP_Y %d' % warp[1],
             '#define GARDEN_ENTRY_X %d   // the floor beside it, where the ride up ends' % (warp[0] + 1),
             '#define GARDEN_ENTRY_Y %d' % warp[1],
             '#define GARDEN_PAD_X0 %d   // the landing (no Garden Pokemon or decorations on it)' % x0,
             '#define GARDEN_PAD_Y0 %d' % y0, '#define GARDEN_PAD_X1 %d' % x1, '#define GARDEN_PAD_Y1 %d' % y1,
             '#define GARDEN_STONE_X %d   // the storm stone (ELECTRIC\'s place: a sprite)' % STONE[0],
             '#define GARDEN_STONE_Y %d' % STONE[1],
             '#define GARDEN_FLOWER 0x%03X' % FLOWER, '',
             '#ifdef GARDEN_WANT_GROWTH',
             '// What each badge adds: {badges, x, y, metatile with its bits}, in order',
             'static const struct GardenGrowth { u8 badges, x, y; u16 tile; } sGardenGrowth[] = {']
    for b, x, y, v in grow:
        lines.append('    {%d, %d, %d, 0x%04X},' % (b, x, y, v))
    lines += ['};', '#endif', '', '#ifdef GARDEN_WANT_ZONES',
              '// Each type\'s place: {TNEED_*, x0, y0, x1, y1, badges, face x, face y} (face: what it looks at there; -1 none)',
              'static const struct GardenZone { u8 need, x0, y0, x1, y1, badges; s8 fx, fy; } sGardenZones[] = {']
    for need, zx0, zy0, zx1, zy1, b, face in ZONES:
        assert need in NEED_NAMES, need
        fx, fy = face if face else (-1, -1)
        lines.append('    {TNEED_%s, %d, %d, %d, %d, %d, %d, %d},' % (need, zx0, zy0, zx1, zy1, b, fx, fy))
    lines += ['};', '#endif', '', '#endif // GUARD_MP_GARDEN_MAP_H', '']
    open(os.path.join(ROOT, 'src/mp/mp_garden_map.h'), 'w').write('\n'.join(lines))

def draw(raw, out):
    from PIL import Image
    gts, sts = L.tileset('general', True), None
    d = os.path.join(ROOT, 'data/tilesets/secondary/moemon_garden')
    L._ts.pop(('moemon_garden', False), None)
    sts = L.tileset('moemon_garden', False)
    tiles = gts['tiles'][:512] + [[[0] * 8] * 8] * (512 - min(512, len(gts['tiles']))) + sts['tiles']
    pals = gts['pals'][:6] + sts['pals'][6:13] + gts['pals'][13:16]
    metas = gts['metas'][:512] + sts['metas']
    im = Image.new('RGB', (W * 16, H * 16))
    px = im.load()
    for my in range(H):
        for mx in range(W):
            m = metas[raw[my][mx] & 0x3FF]
            for layer in range(2):
                for k in range(4):
                    e = m[layer * 4 + k]
                    t = tiles[e & 0x3FF] if (e & 0x3FF) < len(tiles) else [[0] * 8] * 8
                    for y in range(8):
                        for x in range(8):
                            c = t[7 - y if e & 0x800 else y][7 - x if e & 0x400 else x]
                            if c == 0 and layer:
                                continue
                            px[mx * 16 + (k % 2) * 8 + x, my * 16 + (k // 2) * 8 + y] = pals[(e >> 12) & 15][c]
    im.save(out)

if __name__ == '__main__':
    main()
