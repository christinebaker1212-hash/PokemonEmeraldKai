#!/usr/bin/env python3
"""STADIUM's two themes for the game (OPTIONS > BATTLE > STADIUM SETTINGS > STYLE: POKESTADIUM 2, POKESTADIUM), drawn
with the mock-ups' own materials (s2.py, s3.py, screens*.py, stadium1.py) into pieces the game puts together:

  graphics/stadium/themes.bin     every piece (a screen's still layer, a cursor, a plate's lit row...) as zlib'd BGRA
  src/data/stadium_themes.h       their numbers (TH_*) and where the game draws its own things (names, POKéMON, the
                                  cursor's places: SX_* / SY_* in real pixels of the 480x320 canvas)

A piece with see-through parts is drawn twice, over black and over white: its alpha is what the two differ by, so the
soft edges, shadows and glows come out as the mock-ups have them. Run after changing a theme's look:
    tools/stadium/restyle/prep.sh   (once)
    python3 tools/stadium/restyle/build_themes.py
The pack and the header are committed (the release builds nothing of the themes: no fonts or numpy needed there).
"""
import os, sys, struct, zlib
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import numpy as np
from screens3 import *          # noqa: F401,F403  (C4 and every material, the fonts, the pictures)
import screens as S1x
from screens import footer, chip, glass_text, marks

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
PACK = f'{REPO}/graphics/stadium/themes.bin'
HEADER = f'{REPO}/src/data/stadium_themes.h'

PIECES = []      # (name, rgba float HxWx4, x, y: where it sits on the canvas)
DEFS = []        # (name, value)


def define(name, value):
    DEFS.append((name, int(round(value))))


def defines(prefix, box):
    for k, v in zip(('X0', 'Y0', 'X1', 'Y1'), box):
        define(f'{prefix}_{k}', v)


def opaque(name, c):
    """A whole screen's still layer."""
    a = np.clip(c.a, 0, 1)
    PIECES.append((name, np.concatenate([a, np.ones(a.shape[:2] + (1,), np.float32)], -1), 0, 0))


def cut(name, draw, box):
    """A piece with see-through parts: DRAW(canvas) drawn over black and over white, cut to BOX (real pixels)."""
    x0, y0, x1, y1 = [int(v) for v in box]
    k = C4(); k.a[:] = 0; draw(k); blk = np.clip(k.a[y0:y1, x0:x1], 0, 1)
    k = C4(); k.a[:] = 1; draw(k); wht = np.clip(k.a[y0:y1, x0:x1], 0, 1)
    al = np.clip(1 - (wht - blk).mean(-1), 0, 1)
    col = np.where(al[..., None] > 1e-3, blk / np.maximum(al[..., None], 1e-3), 0)
    PIECES.append((name, np.concatenate([np.clip(col, 0, 1), al[..., None]], -1), x0, y0))


# ---------------------------------------------------------------------------------------------------------------
# POKESTADIUM 2 THEME
def s2_strip(c):
    c.strip(295)


def s2_select():
    """The cups (screens.py select_screen): the plate's rows, the picture's frame, the rules' plate. The game writes the
    cups' names, lights the row under the cursor, puts the marks, the picture and the chips."""
    c = C4()
    background(c, front('tauros'), (150, 20), 300, flip=True)
    c.blit(stadium_pic(8), 8, 4)
    c.glass(356, 6, 472, 38)
    defines('SX_S2_ROUND', (356, 6, 472, 38))
    sx0, sy0, sx1, sy1 = c.slab(6, 72, 206, 286, 'blue')
    defines('SX_S2_CUPS', (sx0, sy0, sx1, sy1))
    rh = (sy1 - sy0) / 7
    rows = []
    for i in range(7):
        y0, y1 = int(sy0 + i * rh), int(sy0 + (i + 1) * rh)
        rows.append((y0, y1))
        if i:
            c.engrave_h(y0 - 1, sx0 + 4, sx1 - 4)
        define(f'SY_S2_CUP{i}_Y0', y0)
        define(f'SY_S2_CUP{i}_Y1', y1)
    ix0, iy0, ix1, iy1 = c.frame(218, 42, 470, 222, T=5, gap=1, T2=3)
    defines('SX_S2_PIC', (ix0, iy0, ix1, iy1))
    gx0, gy0, gx1, gy1 = c.slab(214, 228, 474, 286, 'green')   # (taller than the mock-up's: the line under the chips clears its edge)
    defines('SX_S2_RULES', (gx0, gy0, gx1, gy1))
    footer(c, [(('A',), 'ENTER'), (('L', 'R'), 'ROUND'), (('B',), 'EXIT')])
    opaque('S2_SELECT', c)
    # the row under the cursor (gold) and a cup not open (grey): each row's own, the sheen spans the plate
    for i, (y0, y1) in enumerate(rows):
        cut(f'S2_CUPROW_GOLD{i}', lambda k, y0=y0, y1=y1: k.sheen(sx0, y0 + 1, sx1, y1 - 1, 'gold', (sx0, sx1)),
            (sx0, y0 + 1, sx1, y1 - 1))
        cut(f'S2_CUPROW_GREY{i}', lambda k, y0=y0, y1=y1, i=i: k.sheen(sx0, y0 + 1, sx1, y1 if i == 6 else y1 - 1, 'grey', (sx0, sx1)),
            (sx0, y0 + 1, sx1, y1 if i == 6 else y1 - 1))
        cut(f'S2_CUPRING{i}', lambda k, y0=y0, y1=y1: k.ring(sx0 - 3, y0 - 2, sx1 + 3, y1 + 1),
            (sx0 - 7, y0 - 6, sx1 + 7, y1 + 5))
    # the classes won: gold (every restriction), silver, an empty socket (at real pixels, centred)
    cut('S2_MARK_GOLD', lambda k: k.sphere(20, 20, 5, 0xe8b020), (12, 12, 29, 29))
    cut('S2_MARK_SILVER', lambda k: k.sphere(20, 20, 5, 0xe4e6ee), (12, 12, 29, 29))
    cut('S2_MARK_EMPTY', lambda k: k.sphere(20, 20, 4.5, 0x5a6278, flat=True), (12, 12, 29, 29))
    s2_chips()
    cut('S2_STRIP', s2_strip, (0, 280, 480, 320))   # (the foot, for a note in the hints' place)


def s2_class():
    """The classes (screens.py class_screen): STADIUM's ball banners (lit, or grey for one not open) the game moves out
    for the one under the cursor; the class's plate on the right with its trainers' cells."""
    c = C4()
    background(c, front('tauros'), (120, 30), 290)
    c.glass(6, 6, 150, 38)
    defines('SX_S2_CUPNAME', (6, 6, 150, 38))
    c.glass(356, 6, 472, 38)
    sx0, sy0, sx1, sy1 = c.slab(304, 42, 474, 286, 'blue')
    defines('SX_S2_CLASS', (sx0, sy0, sx1, sy1))
    gx, gy = sx0 + 4, sy0 + 54
    cw, ch = (sx1 - sx0 - 8) // 4, 46
    define('SX_S2_FACES_X', gx); define('SX_S2_FACES_Y', gy); define('SX_S2_FACE_W', cw); define('SX_S2_FACE_H', ch)
    c.engrave_h(gy - 3, sx0 + 4, sx1 - 4)
    for i in range(8):
        x, y = gx + (i % 4) * cw, gy + (i // 4) * ch
        if i % 4:
            c.engrave_v(x - 1, y + 2, y + ch - 4)
    c.engrave_h(gy + ch - 2, sx0 + 4, sx1 - 4)
    c.engrave_h(gy + ch * 2 - 2, sx0 + 4, sx1 - 4)
    footer(c, [(('A',), 'CHOOSE'), (('B',), 'BACK')])
    opaque('S2_CLASS', c)
    # the class's boss: the last cell on gold
    x, y = gx + 3 * cw, gy + ch
    cut('S2_FACE_GOLD', lambda k: k.sheen(x + 1, y, x + cw, y + ch - 2, 'gold', (sx0, sx1)), (x + 1, y, x + cw, y + ch - 2))
    # the banners: lit and grey, each with its own shape as a hard shadow; the cursor's ring round one moved out
    for t in range(4):
        y = 42 + t * 61
        define(f'SY_S2_BALL{t}', y)
        for grey in (False, True):
            pic = stadium_pic(4 + t)
            if grey:
                l = (pic[..., :3] * [0.30, 0.59, 0.11]).sum(-1, keepdims=True) * 0.75 + 0.08
                pic = np.concatenate([np.repeat(l, 3, -1), pic[..., 3:] * 0.6], -1)
            b = scale(pic, 220, 62, Image.LANCZOS)

            def draw(k, b=b, y=y, grey=grey):
                sh = np.zeros_like(b)
                sh[..., 3] = b[..., 3] * (0.35 if grey else 0.9)
                k.blit(sh, 16 + 3, y + 3)
                k.blit(b, 16, y)
            cut(f'S2_BALL{t}_{"GREY" if grey else "LIT"}', draw, (14, y - 2, 16 + 226, y + 68))
        cut(f'S2_BALLRING{t}', lambda k, y=y: k.ring(28 - 6, y - 3, 28 + 226, y + 65, r=12, T=4), (28 - 10, y - 7, 28 + 230, y + 69))
    cut('S2_WON', lambda k: k.pill(10, 10, 10 + (measure('WON', NARROW) + 9) * 2, 30, 0xc09020), (6, 6, 16 + (measure('WON', NARROW) + 9) * 2, 34))
    # the BEST spheres: lit (gold for all four), and an empty socket
    cut('S2_BEST_LIT', lambda k: k.sphere(20, 20, 7, 0xe0a040), (10, 10, 31, 31))
    cut('S2_BEST_GOLD', lambda k: k.sphere(20, 20, 7, 0xe8b020), (10, 10, 31, 31))
    cut('S2_BEST_EMPTY', lambda k: k.sphere(20, 20, 6.5, 0x5a6278, flat=True), (10, 10, 31, 31))


def s2_chips():
    """The rule chips as long capsules a colour each (the game cuts them to the word: its ends and its middle)."""
    for name, rgb in (('GREEN', 0x40a050), ('BLUE', 0x4878c0), ('ORANGE', 0xc06030), ('GREY', 0x585880), ('GOLD', 0xc09020),
                      ('RED', 0xc04040), ('VIOLET', 0x8058c0)):
        cut(f'S2_CHIP_{name}', lambda k, rgb=rgb: k.pill(10, 10, 210, 30, rgb), (6, 6, 216, 34))


# ---------------------------------------------------------------------------------------------------------------
def write():
    out = bytearray(b'STTH' + struct.pack('<H', len(PIECES)))
    for name, rgba, x, y in PIECES:
        h, w = rgba.shape[:2]
        b = (np.clip(rgba, 0, 1) * 255 + 0.5).astype(np.uint8)
        bgra = b[..., [2, 1, 0, 3]].tobytes()
        z = zlib.compress(bgra, 9)
        out += struct.pack('<hhHHI', x, y, w, h, len(z)) + z
    open(PACK, 'wb').write(out)
    lines = ['// Generated by tools/stadium/restyle/build_themes.py: STADIUM\'s themes\' pieces (graphics/stadium/themes.bin) and',
             '// where the game draws its own things over them (real pixels of the 480x320 canvas). Not to be edited.',
             '#ifndef GUARD_STADIUM_THEMES_H', '#define GUARD_STADIUM_THEMES_H', 'enum {']
    lines += [f'    TH_{p[0]},' for p in PIECES]
    lines += ['    TH_COUNT', '};']
    lines += [f'#define {n} {v}' for n, v in DEFS]
    lines += ['#endif', '']
    open(HEADER, 'w').write('\n'.join(lines))
    print(f'{len(PIECES)} pieces, {len(out) // 1024} KB')


if __name__ == '__main__':
    s2_select()
    s2_class()
    write()
