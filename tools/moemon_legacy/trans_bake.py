#!/usr/bin/env python3
"""GB OPTIONS > TRANSITIONS: Pokémon Red's and Crystal's battle transitions, run offline.

Each transition routine of pret/pokered engine/battle/battle_transitions.asm and pret/pokecrystal
engine/battle/battle_transition.asm is transliterated here onto the Game Boy's 20x18 tile map, and every frame the
screen would show is recorded: which of the map's own cells each cell shows (a shifted row or column shows another's),
the black tile ($FF / BATTLETRANSITION_BLACK), Crystal's square ($FE, the Poke Ball drawn over a trainer battle), the
background palette (rBGP: the flashes, and the screen going black at the end), Crystal's palette 7 over the whole
field for a trainer (its trainer_battle.pal) and Crystal's wave (SCX per line, from its LY overrides).

The game (src/mp/mp_gb_trans.c) draws a frame of the field taken as the transition starts through these frames,
fitted to KAI's own transition's time, which goes on underneath.

usage: trans_bake.py POKERED_DIR POKECRYSTAL_DIR OUT.h
"""
import math
import os
import random
import re
import sys

W, H = 20, 18
N = W * H
BLACK, SQUARE = 0xFFFF, 0xFFFE
BL, SQ = -1, -2   # (inside the bake: a cell's own index is 0-359, so the black / square tiles are these)
BGP_NORMAL = 0xE4


def dc(a, b, c, d):
    return (a << 6) | (b << 4) | (c << 2) | d


FLASH = [dc(3, 3, 2, 1), dc(3, 3, 3, 2), dc(3, 3, 3, 3), dc(3, 3, 3, 2), dc(3, 3, 2, 1), dc(3, 2, 1, 0),
         dc(2, 1, 0, 0), dc(1, 0, 0, 0), dc(0, 0, 0, 0), dc(1, 0, 0, 0), dc(2, 1, 0, 0), dc(3, 2, 1, 0)]


class Screen:
    """The tile map (with room around it: the routines walk past its edges) and what the screen shows each frame"""
    PAD = 400

    def __init__(self):
        self.mem = [0] * (N + 2 * self.PAD)
        for i in range(N):
            self.mem[self.PAD + i] = i
        self.visible = list(range(N))
        self.bgp = BGP_NORMAL
        self.pal7 = False
        self.scx = None
        self.auto = True     # Red's hAutoBGTransferEnabled / Crystal's hBGMapMode: the map reaches the screen
        self.frames = []

    def __getitem__(self, a):
        return self.mem[self.PAD + a]

    def __setitem__(self, a, v):
        self.mem[self.PAD + a] = v

    def frame(self, n=1):
        for _ in range(n):
            if self.auto:
                self.visible = [self[i] for i in range(N)]
            self.frames.append((list(self.visible), self.bgp, self.pal7, list(self.scx) if self.scx else None))

    def transfer(self):
        """the whole map to the screen now (BattleStart_CopyTilemapAtOnce)"""
        self.visible = [self[i] for i in range(N)]


def co(x, y):
    return y * W + x


# ------------------------------------------------------------------------------------------------------------- Red
def red_preamble(s):
    s.auto = True
    s.frame(3)   # Delay3
    s.frame(1)   # DelayFrame
    s.frame(3)   # Delay3 (the OAM cleared: the player and the trainer stay)


def red_transfer_delay3(s):
    s.auto = True
    s.frame(3)
    s.auto = False


def red_black(s):
    s.bgp = 0xFF


def red_flash(s):
    for _ in range(3):
        for p in FLASH:
            s.bgp = p
            s.frame(2)
    s.bgp = BGP_NORMAL
    s.auto = False


def red_spiral_out(s):
    hl = co(10, 10)
    direction = 3
    for _ in range(120):
        for _ in range(3):
            if direction == 0:     # up
                hl -= 1
                if s[hl] != BL:
                    s[hl] = BL
                    direction = (direction + 1) % 4
                    continue
                hl += 1
                hl -= W
            elif direction == 1:   # left
                hl += W
                if s[hl] != BL:
                    s[hl] = BL
                    direction = (direction + 1) % 4
                    continue
                hl -= W
                hl -= 1
            elif direction == 2:   # down
                hl += 1
                if s[hl] != BL:
                    s[hl] = BL
                    direction = (direction + 1) % 4
                    continue
                hl -= 1
                hl += W
            else:                  # right
                hl -= W
                if s[hl] != BL:
                    s[hl] = BL
                    direction = (direction + 1) % 4
                    continue
                hl += W
                hl += 1
            s[hl] = BL
        s.frame(1)
    red_black(s)


def red_spiral_in(s):
    counter = [7]
    hl = [0]

    def run(c, de):
        for _ in range(c):
            s[hl[0]] = BL
            hl[0] += de
            counter[0] -= 1
            if counter[0] == 0:
                red_transfer_delay3(s)
                counter[0] = 7

    c = H - 1
    run(c, W)
    c += 1
    first = True
    while True:
        if not first:
            run(c, W)
        first = False
        c += 1
        run(c, 1)
        c -= 2
        run(c, -W)
        c += 1
        run(c, -1)
        c -= 2
        if c & 0xFF == 0:
            break
    red_black(s)


def red_copy_tiles1(s, hl, de, bc):
    for _ in range(8):
        for k in range(W):
            s[de + k] = s[hl + k]
        hl, de = de, hl
        hl += bc
    for k in range(W):
        s[de + k] = BL


def red_copy_tiles2(s, hl, de, bc):
    for _ in range(H // 2):
        for r in range(H):
            s[de + r * W] = s[hl + r * W]
        hl, de = de, hl
        hl += bc
    for r in range(H):
        s[de + r * W] = BL


def red_shrink(s):
    for _ in range(H // 2):
        s.auto = False
        red_copy_tiles1(s, co(0, 7), co(0, 8), -W * 2)
        red_copy_tiles1(s, co(0, 10), co(0, 9), W * 2)
        red_copy_tiles2(s, co(8, 0), co(9, 0), -2)
        red_copy_tiles2(s, co(11, 0), co(10, 0), 2)
        s.auto = True
        s.frame(6)
    red_black(s)
    s.frame(10)


def red_split(s):
    s.auto = False
    for _ in range(H // 2):
        red_copy_tiles1(s, co(0, 16), co(0, 17), -W * 2)
        red_copy_tiles1(s, co(0, 1), co(0, 0), W * 2)
        red_copy_tiles2(s, co(18, 0), co(19, 0), -2)
        red_copy_tiles2(s, co(1, 0), co(0, 0), 2)
        red_transfer_delay3(s)
        s.frame(3)
    red_black(s)
    s.frame(10)


def red_vstripes(s):
    hl, de = co(0, 0), co(1, 17)
    s.auto = False
    for _ in range(H):
        for k in range(W // 2):
            s[hl + 2 * k] = BL
        for k in range(W // 2):
            s[de + 2 * k] = BL
        red_transfer_delay3(s)
        de -= W
        hl += W
    red_black(s)


def red_hstripes(s):
    hl, de = co(0, 0), co(19, 1)
    s.auto = False
    for _ in range(W):
        for k in range(H // 2):
            s[hl + 2 * W * k] = BL
        for k in range(H // 2):
            s[de + 2 * W * k] = BL
        red_transfer_delay3(s)
        hl += 1
        de -= 1
    red_black(s)


CIRCLE_DATA = [
    [2, 3, 5, 4, 9, -1],
    [1, 1, 2, 2, 4, 2, 4, 2, 3, -1],
    [2, 1, 3, 1, 4, 1, 4, 1, 4, 1, 3, 1, 2, 1, 1, 1, 1, -1],
    [4, 1, 4, 0, 3, 1, 3, 0, 2, 1, 2, 0, 1, -1],
    [4, 0, 3, 0, 3, 0, 2, 0, 2, 0, 1, 0, 1, 0, 1, -1],
]
LEFT, RIGHT = 0, 1
HALF1 = [(RIGHT, 0, 18, 6), (RIGHT, 1, 19, 3), (RIGHT, 2, 18, 0), (RIGHT, 3, 14, 0), (RIGHT, 4, 10, 0),
         (LEFT, 4, 9, 0), (LEFT, 3, 5, 0), (LEFT, 2, 1, 0), (LEFT, 1, 0, 3), (LEFT, 0, 1, 6)]
HALF2 = [(LEFT, 0, 1, 11), (LEFT, 1, 0, 14), (LEFT, 2, 1, 17), (LEFT, 3, 5, 17), (LEFT, 4, 9, 17),
         (RIGHT, 4, 10, 17), (RIGHT, 3, 14, 17), (RIGHT, 2, 18, 17), (RIGHT, 1, 19, 14), (RIGHT, 0, 18, 11)]


def wedge(s, quad_x_right, quad_y_lower, data, hl, value=BL):
    """BattleTransition_Circle_Sub3 / Crystal's SpinToBlack .load: black runs of a wedge of the circle"""
    di = 0
    while True:
        start = hl
        c = data[di]
        di += 1
        for _ in range(c):
            s[hl] = value
            hl += 1 if quad_x_right else -1
        hl = start + (-W if quad_y_lower else W)
        a = data[di]
        di += 1
        if a == -1:
            return
        if a == 0:
            continue
        hl += -a if quad_x_right else a


def red_circle(s):
    red_flash(s)
    for i in range(10):
        q, d, x, y = HALF1[i]
        wedge(s, q == RIGHT, False, CIRCLE_DATA[d], co(x, y))
        red_transfer_delay3(s)
    for i in range(10):
        q, d, x, y = HALF2[i]
        wedge(s, q == RIGHT, True, CIRCLE_DATA[d], co(x, y))
        red_transfer_delay3(s)
    red_black(s)


def red_double_circle(s):
    red_flash(s)
    for i in range(10):
        q, d, x, y = HALF1[i]
        wedge(s, q == RIGHT, False, CIRCLE_DATA[d], co(x, y))
        q, d, x, y = HALF2[i]
        wedge(s, q == RIGHT, True, CIRCLE_DATA[d], co(x, y))
        red_transfer_delay3(s)
    red_black(s)


def red(routine):
    s = Screen()
    red_preamble(s)
    routine(s)
    s.frame(1)   # (black: the battle begins)
    return s


# --------------------------------------------------------------------------------------------------------- Crystal
POKEBALL = ['......XXXX......', '....XXXXXXXX....', '..XXXX....XXXX..', '..XX........XX..', '.XX..........XX.',
            '.XX...XXXX...XX.', 'XX...XX..XX...XX', 'XXXXXX....XXXXXX', 'XXXXXX....XXXXXX', 'XX...XX..XX...XX',
            '.XX...XXXX...XX.', '.XX..........XX.', '..XX........XX..', '..XXXX....XXXX..', '....XXXXXXXX....',
            '......XXXX......']
CFLASH = FLASH + [dc(0, 0, 0, 1)]
SPIN = [(0, 0, 1, 6), (0, 1, 0, 3), (0, 2, 1, 0), (0, 3, 5, 0), (0, 4, 9, 0), (1, 4, 10, 0), (1, 3, 14, 0),
        (1, 2, 18, 0), (1, 1, 19, 3), (1, 0, 18, 6), (3, 0, 18, 11), (3, 1, 19, 14), (3, 2, 18, 17), (3, 3, 14, 17),
        (3, 4, 10, 17), (2, 4, 9, 17), (2, 3, 5, 17), (2, 2, 1, 17), (2, 1, 0, 14), (2, 0, 1, 11)]
ZOOM = [(4, 2, 8, 8), (6, 4, 7, 7), (8, 6, 6, 6), (10, 8, 5, 5), (12, 10, 4, 4), (14, 12, 3, 3), (16, 14, 2, 2),
        (18, 16, 1, 1), (20, 18, 0, 0)]


def sine(a, d):
    """calc_sine_wave: d * sin(a * pi / 32), a a signed 6-bit value (sine_table 32 in Q8)"""
    a &= 0x3F
    neg = a >= 0x20
    a &= 0x1F
    t = int(math.sin(a * 0.5 / 32 * 2 * math.pi) * 256)
    h = ((d * t) >> 8) & 0xFF
    return (-h) & 0xFF if neg else h


def crystal(kind, trainer):
    """kind: 0 cave, 1 cave stronger, 2 no cave, 3 no cave stronger"""
    s = Screen()
    s.auto = True
    s.frame(3)   # .InitGFX (DelayFrame, the tiles, the map at once, DelayFrame)
    s.auto = False
    s.frame(1)   # DoBattleTransition: DelayFrame
    s.frame(1)   # 00 DetermineWhichAnimation
    if trainer:
        for y in range(16):
            row = POKEBALL[y]
            for x in range(16):
                if row[x] == 'X':
                    s[co(2 + x, 1 + y)] = SQ
        s.pal7 = True
        s.frame(1)   # DelayFrame
        s.transfer()
    s.frame(1)   # LoadPokeBallGraphics
    s.frame(1)   # SetUpBGMap
    for _ in range(3):
        counter = 0
        while True:
            p = CFLASH[counter >> 1]
            counter += 1
            if p == dc(0, 0, 0, 1):
                s.frame(1)
                break
            s.bgp = p
            s.frame(1)
    s.bgp = BGP_NORMAL
    s.frame(1)   # NextScene
    if kind == 0:
        s.frame(1)   # SetUpForWavyOutro
        counter = offset = 0
        while counter < 0x60:
            old_off = offset
            offset += 1
            d = counter
            counter = (old_off + counter) & 0xFF
            s.scx = [sine(2 * ly, d) for ly in range(144)]
            s.frame(1)
        s.frame(1)   # FINISH
    elif kind == 1:
        for (w, h, y, x) in ZOOM:
            for yy in range(h):
                for xx in range(w):
                    s[co(x + xx, y + yy)] = BL
            s.auto = True
            s.frame(4)   # WaitBGMap
            s.auto = False
        s.frame(1)   # FINISH
    elif kind == 2:
        s.frame(1)   # SetUpForSpinOutro
        for (q, d, x, y) in SPIN:
            s.auto = False
            wedge(s, q & 1, q & 2, CIRCLE_DATA[d], co(x, y))
            s.auto = True
            s.frame(2)
            s.frame(1)   # (the loop's own DelayFrame)
        s.auto = True
        s.frame(3)
        s.auto = False
        s.frame(1)   # FINISH
    else:
        s.frame(1)   # SetUpForRandomScatterOutro (hBGMapMode 1)
        s.auto = True
        rng = random.Random(1997)
        for _ in range(16):
            for _ in range(12):
                while True:
                    y = rng.randrange(H)
                    x = rng.randrange(W)
                    if s[co(x, y)] != BL:
                        s[co(x, y)] = BL
                        break
            s.frame(1)
        s.frame(3)
        s.auto = False
        s.frame(1)   # FINISH
    s.bgp = 0xFF     # .done: every BG palette black
    s.frame(1)
    return s


# ------------------------------------------------------------------------------------------------------------ out
RED_LIST = [('RED_DOUBLE_CIRCLE', red_double_circle), ('RED_SPIRAL_IN', red_spiral_in),
            ('RED_CIRCLE', red_circle), ('RED_SPIRAL_OUT', red_spiral_out),
            ('RED_HSTRIPES', red_hstripes), ('RED_SHRINK', red_shrink),
            ('RED_VSTRIPES', red_vstripes), ('RED_SPLIT', red_split)]
GSC_LIST = [('GSC_CAVE', 0), ('GSC_CAVE_STRONGER', 1), ('GSC_NO_CAVE', 2), ('GSC_NO_CAVE_STRONGER', 3)]


def value(v):
    return BLACK if v == BL else SQUARE if v == SQ else v


def emit(out, name, s):
    prev = list(range(N))
    changes = []
    starts = []
    bgp = []
    flags = []
    scx = []
    for (vis, b, p7, sx) in s.frames:
        starts.append(len(changes))
        for i in range(N):
            v = value(vis[i])
            if v != prev[i]:
                changes.append((i, v))
        prev = [value(v) for v in vis]
        bgp.append(b)
        flags.append(int(p7) | (2 if sx else 0))
        if sx:
            scx.append(sx)
    starts.append(len(changes))
    n = len(s.frames)
    out.append('static const u16 s%sStart[%d] = {%s};' % (name, n + 1, ','.join(map(str, starts))))
    out.append('static const struct GbTransChange s%sChanges[%d] = {%s};' % (
        name, max(1, len(changes)), ','.join('{%d,%d}' % c for c in changes) or '{0,0}'))
    out.append('static const u8 s%sBgp[%d] = {%s};' % (name, n, ','.join('0x%02X' % b for b in bgp)))
    out.append('static const u8 s%sFlags[%d] = {%s};' % (name, n, ','.join(map(str, flags))))
    if scx:
        out.append('static const s8 s%sScx[%d][144] = {%s};' % (
            name, len(scx), ','.join('{%s}' % ','.join(str(v - 256 if v > 127 else v) for v in row) for row in scx)))
    return n, bool(scx)


def main():
    red_dir, gsc_dir, out_path = sys.argv[1:4]
    out = ['// Generated by tools/moemon_legacy/trans_bake.py from pret/pokered and pret/pokecrystal: Red\'s and',
           '// Crystal\'s battle transitions, every frame. Don\'t edit. (GB OPTIONS > TRANSITIONS: src/mp/mp_gb_trans.c)',
           '']
    table = []
    for name, routine in RED_LIST:
        n, has_scx = emit(out, name, red(routine))
        table.append((name, n, has_scx))
    for trainer in (0, 1):
        for name, kind in GSC_LIST:
            nm = name + ('_TRAINER' if trainer else '')
            n, has_scx = emit(out, nm, crystal(kind, trainer))
            table.append((nm, n, has_scx))
    out.append('enum { %s, GB_TRANS_COUNT };' % ', '.join('GB_TRANS_' + t[0] for t in table))
    out.append('static const struct GbTransTimeline sGbTransTimelines[GB_TRANS_COUNT] = {')
    for name, n, has_scx in table:
        out.append('    {%d, s%sStart, s%sChanges, s%sBgp, s%sFlags, %s},' % (
            n, name, name, name, name, ('s%sScx' % name) if has_scx else 'NULL'))
    out.append('};')
    # Crystal's square and its palette 7 (gfx/overworld/battle_transition_tiles, trainer_battle.pal)
    from PIL import Image
    im = Image.open(os.path.join(gsc_dir, 'gfx', 'overworld', 'battle_transition_tiles.png')).convert('L')
    sq = [[3 - im.getpixel((x, y)) // 85 for x in range(8)] for y in range(8)]
    out.append('static const u8 sGbTransSquare[8][8] = {%s};' % ','.join('{%s}' % ','.join(map(str, r)) for r in sq))
    pal = []
    for l in open(os.path.join(gsc_dir, 'gfx', 'overworld', 'trainer_battle.pal')):
        m = re.match(r'\s*RGB\s+(\d+),\s*(\d+),\s*(\d+)', l)
        if m:
            pal.append('RGB(%s, %s, %s)' % m.groups())
    out.append('static const u16 sGbTransPal7[4] = {%s};' % ', '.join(pal[:4]))
    open(out_path, 'w').write('\n'.join(out) + '\n')
    for name, n, _ in table:
        print('%-30s %d frames' % (name, n))


if __name__ == '__main__':
    main()
