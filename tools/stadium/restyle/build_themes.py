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

import s2 as _s2


def _drop_text_rgba(s, font, color=0xffffff, outline=None, shadow=None, ow=2, grad=None, bold=False):
    """The game's words as the themes write them: a hard shadow SHADOW_PX real pixels down and right in the outline's
    colour (the user: no soft outline)."""
    face, sh = _s2.text_masks(s, font)
    face = np.kron(face, np.ones((2, 2), bool))
    pad = 3
    face = np.pad(face, pad)
    if bold:
        face = face | np.roll(face, 1, 1)
    rgba = np.zeros(face.shape + (4,), np.float32)
    ink = outline if outline is not None else (_s2.hx(shadow) if shadow is not None else None)
    if ink is not None:
        k = SHADOW_PX
        drop = np.roll(np.roll(face, k, 0), k, 1)
        rgba[drop] = np.append(np.asarray(ink, np.float32), 1)
    col = np.broadcast_to(_s2.hx(color), face.shape + (3,))
    if grad is not None:
        t = np.linspace(0, 1, face.shape[0])[:, None, None]
        col = np.broadcast_to(_s2.hx(color) * (1 - t) + _s2.hx(grad) * t, face.shape + (3,))
    rgba[face, :3] = col[face]
    rgba[face, 3] = 1
    return rgba, _s2.measure(s, font), pad


SHADOW_PX = int(os.environ.get('TH_SHADOW', '2'))
_s2.text_rgba = _drop_text_rgba

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
    c.strip(295)   # (the hints are the game's: they follow the controller's buttons)
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
    c.strip(295)   # (the hints are the game's: they follow the controller's buttons)
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


def s2_options():
    """The cup's menu (reconcile.py options_s2): the cup's picture in a portrait frame, the restrictions' plate, the help
    in glass, the team's plate, three glass buttons. The game writes the names, lights the row and the button under
    the cursor, puts the switches, the spheres and the team."""
    c = C4()
    background(c, front('tauros'), (140, 30), 290)
    c.glass(6, 6, 300, 38)
    defines('SX_S2_OHEAD', (6, 6, 300, 38))
    c.glass(356, 6, 472, 38)
    defines('SX_S2_OPIC', c.frame(8, 44, 200, 196, T=5, gap=1, T2=3))
    sx0, sy0, sx1, sy1 = c.slab(208, 44, 474, 196, 'blue')
    defines('SX_S2_OPTS', (sx0, sy0, sx1, sy1))
    c.engrave_h(sy0 + 20, sx0 + 4, sx1 - 4)
    c.glass(208, 202, 474, 254, T=5)
    defines('SX_S2_OHELP', (208, 202, 474, 254))
    gx0, gy0, gx1, gy1 = c.slab(8, 202, 200, 254, 'green')
    defines('SX_S2_OTEAM', (gx0, gy0, gx1, gy1))
    for i in range(3):
        x0 = 8 + i * 156
        c.glass(x0, 259, x0 + 152, 287, T=5)
    define('SX_S2_OBTN_X', 8); define('SX_S2_OBTN_STEP', 156); define('SX_S2_OBTN_W', 152)
    define('SY_S2_OBTN_Y0', 259); define('SY_S2_OBTN_Y1', 287)
    c.strip(295)   # (the hints are the game's: they follow the controller's buttons)
    opaque('S2_OPTIONS', c)
    # the four rows (25 px apart under the title's line), lit and ringed under the cursor
    ry = sy0 + 24
    define('SY_S2_OROW0', ry); define('SY_S2_OROW_H', 25)
    for i in range(4):
        y0 = ry + i * 25
        cut(f'S2_OROW_GOLD{i}', lambda k, y0=y0: k.sheen(sx0, y0, sx1, y0 + 24, 'gold', (sx0, sx1)), (sx0, y0, sx1, y0 + 24))
        cut(f'S2_OROWRING{i}', lambda k, y0=y0: k.ring(sx0 - 2, y0 - 2, sx1 + 2, y0 + 26), (sx0 - 6, y0 - 6, sx1 + 6, y0 + 30))
    for i in range(3):
        x0 = 8 + i * 156
        cut(f'S2_OBTNRING{i}', lambda k, x0=x0: k.ring(x0 - 3, 256, x0 + 155, 290, r=8), (x0 - 7, 252, x0 + 159, 294))
    # a switch's body: ON (green, the knob right) and OFF (red, the knob left), drawn at (10, 10)
    for on in (True, False):
        def sw(k, on=on):
            k.pill(10, 11, 78, 31, 0x40a050 if on else 0xa04040)
            k.sphere(10 + (56 if on else 12), 21, 7, 0xf0f0f4)
        cut('S2_SWITCH_' + ('ON' if on else 'OFF'), sw, (6, 6, 86, 38))
    cut('S2_STAR_LIT', lambda k: k.sphere(20, 20, 6, 0xe8b020), (11, 11, 30, 30))
    cut('S2_STAR_EMPTY', lambda k: k.sphere(20, 20, 5.5, 0x5a6278, flat=True), (11, 11, 30, 30))
    cut('S2_SOCKET_GREEN', lambda k: k.sphere(20, 20, 6, 0x5a7860, flat=True), (11, 11, 30, 30))


BADGE_RGB = [0xa8a8b0, 0x58a0f0, 0xf0a030, 0x70c868, 0xe878b8, 0xe0c040, 0xe85038, 0x48a870]   # (mp_stadium_ui.c sBadgeRgb)


def s2_castle():
    """The GYM LEADER CASTLE (reconcile.py castle_s2): the stages as rows of one plate, the stage under the cursor on a
    green plate (its trainers, the badges won)."""
    c = C4()
    background(c, None)
    c.glass(6, 6, 300, 38)
    c.glass(356, 6, 472, 38)
    sx0, sy0, sx1, sy1 = c.slab(6, 44, 300, 286, 'blue')
    defines('SX_S2_CASTLE', (sx0, sy0, sx1, sy1))
    rh = (sy1 - sy0) / 9
    rows = []
    for i in range(9):
        y0, y1 = int(sy0 + i * rh), int(sy0 + (i + 1) * rh)
        rows.append((y0, y1))
        if i:
            c.engrave_h(y0 - 1, sx0 + 4, sx1 - 4)
        define(f'SY_S2_CROW{i}_Y0', y0); define(f'SY_S2_CROW{i}_Y1', y1)
    gx0, gy0, gx1, gy1 = c.slab(306, 44, 474, 286, 'green')
    defines('SX_S2_CSIDE', (gx0, gy0, gx1, gy1))
    c.engrave_h(gy0 + 112, gx0 + 4, gx1 - 4, 'green')
    define('SY_S2_CCELLS', gy0 + 54); define('SY_S2_CCELL_H', 52); define('SY_S2_CBADGES', gy0 + 116)
    c.strip(295)   # (the hints are the game's: they follow the controller's buttons)
    opaque('S2_CASTLE', c)
    for i, (y0, y1) in enumerate(rows):
        yb = y1 if i == 8 else y1 - 1
        cut(f'S2_CROW_GOLD{i}', lambda k, y0=y0, yb=yb: k.sheen(sx0, y0 + 1, sx1, yb, 'gold', (sx0, sx1)), (sx0, y0 + 1, sx1, yb))
        cut(f'S2_CROW_GREY{i}', lambda k, y0=y0, yb=yb: k.sheen(sx0, y0 + 1, sx1, yb, 'grey', (sx0, sx1)), (sx0, y0 + 1, sx1, yb))
        cut(f'S2_CROWRING{i}', lambda k, y0=y0, y1=y1: k.ring(sx0 - 3, y0 - 2, sx1 + 3, y1 + 1), (sx0 - 7, y0 - 6, sx1 + 7, y1 + 5))
    # the last trainer's cell on gold: four cells (a gym) or five (the ELITE FOUR and the RIVAL)
    for n in (4, 5):
        w = (gx1 - gx0) // n
        x = gx0 + (n - 1) * w
        y = gy0 + 54
        cut(f'S2_CCELL_GOLD{n}', lambda k, x=x, w=w, y=y: k.sheen(x, y, x + w, y + 52, 'gold', (gx0, gx1)), (x, y, x + w, y + 52))
    for g, rgb in enumerate(BADGE_RGB):
        cut(f'S2_BADGE{g}', lambda k, rgb=rgb: k.sphere(20, 20, 7.5, rgb), (10, 10, 31, 31))



def s2_boxes():
    """Boxes the game draws at any size (cut into nine): the blue panel (menus, questions), its pale one (THE RULES),
    black glass; the menus' white pointer."""
    cut('S2_PANEL', lambda k: k.bluepanel(10, 10, 130, 130), (6, 6, 136, 136))
    cut('S2_PANEL_PALE', lambda k: k.bluepanel(10, 10, 130, 130, color=(0x7c98f4, 0x6c88ec)), (6, 6, 136, 136))
    cut('S2_GLASSBOX', lambda k: k.glass(10, 10, 130, 130, T=5), (6, 6, 136, 136))
    cut('S2_POINTER', lambda k: k.pointer(10, 20), (6, 10, 26, 30))


TEAM_ROWS = 8


def ribbon(c, y, left, right):
    """The titles' ribbon (s3.py) as long as its words, without them: the game writes them."""
    t = c.text
    c.text = lambda *a, **k: 0
    c.ribbon(y, left, right)
    c.text = t


def s2_teams():
    """SAVED TEAMS (reconcile.py teams_s2): the slots on the blue list, the slot's team in the plate's windows."""
    c = C4()
    c.sky()
    ribbon(c, 8, 'SAVED TEAMS', 'THIS SAVE 99 / 99')
    bx0, by0, bx1, by1 = c.listpanel(8, 30, 250, 288)
    defines('SX_S2_TLIST', (bx0, by0, bx1, by1))
    c.plate(258, 30, 470, 288)
    defines('SX_S2_TPLATE', (258, 30, 470, 288))
    for i in range(6):
        x0, y0 = 266 + (i % 3) * 67, 86 + (i // 3) * 98
        ix = c.window(x0, y0, x0 + 62, y0 + 72)
        if i == 0:
            define('SX_S2_TWIN_X', ix[0]); define('SX_S2_TWIN_Y', ix[1])
            define('SX_S2_TWIN_W', ix[2] - ix[0]); define('SX_S2_TWIN_H', ix[3] - ix[1])
    define('SX_S2_TWIN_DX', 67); define('SX_S2_TWIN_DY', 98)
    c.strip(295)   # (the hints are the game's: they follow the controller's buttons)
    opaque('S2_TEAMS', c)
    rh = (by1 - by0) / TEAM_ROWS
    define('SY_S2_TROW_H', rh)
    for i in range(TEAM_ROWS):
        y0 = by0 + i * rh
        cut(f'S2_TBRACKETS{i}', lambda k, y0=y0: k.brackets(bx0 + 1, y0 + 1, bx1 - 1, y0 + rh - 1, arm=8),
            (bx0 - 2, int(y0) - 2, bx1 + 2, int(y0 + rh) + 2))
    cut('S2_THUMB', lambda k: k.thumb(21, 56), (10, 42, 32, 70))


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
    s2_options()
    s2_castle()
    s2_boxes()
    s2_teams()
    write()
