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
    """The game's words as the themes write them: a hard outline OUTLINE_PX real pixels round them in the outline's
    colour (the user: hard, never soft)."""
    face, sh = _s2.text_masks(s, font)
    face = np.kron(face, np.ones((2, 2), bool))
    pad = 4
    face = np.pad(face, pad)
    if bold:
        face = face | np.roll(face, 1, 1)
    rgba = np.zeros(face.shape + (4,), np.float32)
    ink = outline if outline is not None else (_s2.hx(shadow) if shadow is not None else None)
    if ink is not None:
        k = OUTLINE_PX
        ring = np.zeros_like(face)
        for dy in range(-k, k + 1):
            for dx in range(-k, k + 1):
                ring |= np.roll(np.roll(face, dy, 0), dx, 1)
        rgba[ring] = np.append(np.asarray(ink, np.float32), 1)
    col = np.broadcast_to(_s2.hx(color), face.shape + (3,))
    if grad is not None:
        t = np.linspace(0, 1, face.shape[0])[:, None, None]
        col = np.broadcast_to(_s2.hx(color) * (1 - t) + _s2.hx(grad) * t, face.shape + (3,))
    rgba[face, :3] = col[face]
    rgba[face, 3] = 1
    return rgba, _s2.measure(s, font), pad


OUTLINE_PX = 2
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



REG_TABS = ['PARTY', 'BOXES', 'SHARED', 'GLOBAL', 'RENTALS']


def s2_register():
    """REGISTER A TEAM (screens3.py entry_screen / entry_info_screen / entry_menu_screen): the team on the 1P plate,
    the tabs of where from, the list three by three; A's panel (the POKéMON, ENTER Yes / No) over the list."""
    c = C4()
    c.sky()
    ribbon(c, 8, 'REGISTER A TEAM', 'CHALLENGE CUP')
    sx0, sy0, sx1, sy1 = c.slab(12, 42, 468, 176, 'blue')
    defines('SX_S2_RTEAM', (sx0, sy0, sx1, sy1))
    c.tag(8, 31, 250)
    cw, ch = (sx1 - sx0) / 3, (sy1 - sy0) / 2
    cells = [(int(sx0 + (i % 3) * cw), int(sy0 + (i // 3) * ch), int(sx0 + (i % 3 + 1) * cw), int(sy0 + (i // 3 + 1) * ch)) for i in range(6)]
    for k in (1, 2):
        c.engrave_v(cells[k][0], sy0 + 4, sy1 - 4)
    c.engrave_h(cells[3][1], sx0 + 4, sx1 - 4)
    # the tabs: L, the five (the game puts the chosen one on top), R
    y = 181
    c.key(10, y, 34, y + 20, 'L')
    x = 40
    tabx = []
    for name in REG_TABS:
        wpx = (measure(name, SMALL) + 9) * 2
        c.key(x, y, x + wpx, y + 20, name)
        tabx.append((x, x + wpx))
        x += wpx + 6
    c.key(446, y, 470, y + 20, 'R')
    bx0, by0, bx1, by1 = c.listpanel(8, 206, 472, 288)
    defines('SX_S2_RLIST', (bx0, by0, bx1, by1))
    lw, lh = (bx1 - bx0) / 3, (by1 - by0) / 3
    for i in range(9):
        x0, y0 = bx0 + (i % 3) * lw, by0 + (i // 3) * lh
        c.cell(x0 + 1, y0 + 1, x0 + lw - 2, y0 + lh - 1)
    c.strip(295)
    opaque('S2_REGISTER', c)
    for k, (x0, x1) in enumerate(tabx):
        cut(f'S2_RTAB{k}', lambda q, x0=x0, x1=x1, k=k: q.key(x0, y, x1, y + 20, REG_TABS[k], selected=True), (x0 - 6, y - 8, x1 + 7, y + 31))
    for i, (x0, y0, x1, y1) in enumerate(cells):
        cut(f'S2_RTEAMBR{i}', lambda q, x0=x0, y0=y0, x1=x1, y1=y1: q.brackets(x0 + 8, y0 + 7, x1 - 6, y1 - 6, arm=10, th=4),
            (x0 + 4, y0 + 3, x1 - 2, y1 - 2))
    for i in range(9):
        x0, y0 = bx0 + (i % 3) * lw, by0 + (i // 3) * lh
        cut(f'S2_RLISTBR{i}', lambda q, x0=x0, y0=y0: q.brackets(x0 + 1, y0 + 1, x0 + lw - 2, y0 + lh - 1, arm=8),
            (int(x0) - 2, int(y0) - 2, int(x0 + lw) + 2, int(y0 + lh) + 2))
    cut('S2_THUMB_R', lambda q: q.thumb(21, 228), (10, 214, 32, 242))
    cut('S2_ITEMBOX', lambda q: q.itembox(5, 5), (8, 10, 30, 30))
    # A's panel over the list: the POKéMON's, ENTER's
    def info(q):
        q.bluepanel(8, 180, 386, 288)
        q.window(18, 208, 80, 250, band=0x9ab0ff)
        q.bluepanel(392, 180, 472, 250)
    cut('S2_RINFO', info, (4, 176, 478, 294))
    define('SX_S2_RWIN_X', 22); define('SX_S2_RWIN_Y', 212); define('SX_S2_RWIN_W', 54); define('SX_S2_RWIN_H', 34)
    cut('S2_DOT', lambda q: q.sphere(20, 20, 4.5, 0xffffff), (13, 13, 28, 28))
    cut('S2_CHIP_WHITE', lambda q: q.pill(10, 10, 210, 30, 0xffffff), (6, 6, 216, 34))



VS_LINES = ((27, -40, 70), (45, 400, 520), (158, -40, 520), (300, 180, 520))


def s2_ladder():
    """The cup's ladder (screens3.py ladder_screen): eight tiles round two bars (the cup, the battle); the castle's
    gyms four, with the bars under them. The game puts the trainers met in the windows, a ? tile on the ones to come."""
    for n, y0 in ((8, 34), (4, 80)):
        c = C4()
        c.night(((20, -40, 60), (22, 440, 520), (300, 430, 520), (302, -40, 40)))
        y = y0
        c.plate(30, y, 450, y + 82)
        for i in range(4):
            c.window(38 + i * 102, y + 8, 38 + i * 102 + 98, y + 74, glow=False)
        define(f'SY_S2_LAD{n}_TOP', y + 8)
        y += 84
        define(f'SY_S2_LAD{n}_BARS', y)
        for k in range(2):
            c.plate(30, y, 450, y + 26)
            y += 28
        if n == 8:
            c.plate(30, y, 450, y + 82)
            for i in range(4):
                c.window(38 + i * 102, y + 8, 38 + i * 102 + 98, y + 74, glow=False)
            define('SY_S2_LAD8_BOTTOM', y + 8)
        c.strip(295)
        opaque(f'S2_LADDER{n}', c)
    define('SX_S2_LAD_X', 38); define('SX_S2_LAD_STEP', 102)
    # a battle to come: ribbed metal and a ? (in the first tile's place), a slot not used (the plate's own metal)
    def q(k):
        k.plate(30, 34, 450, 116)
        k.qtile_bare = True
        ix0, iy0, ix1, iy1 = k.window(38, 42, 136, 108, glow=False)
        h = iy1 - iy0
        ribs = 0.78 - 0.30 * (((np.arange(h) % 8) >= 5).astype(np.float32)) + 0.10 * ((np.arange(h) % 8) == 0)
        u = np.linspace(0, 1, ix1 - ix0)
        sh = 1 + 0.10 * np.cos((u - 0.3) * 5)
        k.a[iy0:iy1, ix0:ix1] = (ribs[:, None] * sh[None])[..., None]
        from PIL import ImageFont, ImageDraw
        S = 3
        f = ImageFont.truetype(HEAVY, 46 * S)
        l, t, r, b = f.getbbox('?')
        m = Image.new('L', ((ix1 - ix0) * S, h * S), 0)
        ImageDraw.Draw(m).text((((ix1 - ix0) * S - (r - l)) / 2 - l, (h * S - (b - t)) / 2 - t - 6 * S), '?', font=f, fill=255)
        qq = np.array(m.resize((ix1 - ix0, h), Image.LANCZOS)).astype(np.float32) / 255
        reg = k.a[iy0:iy1, ix0:ix1]
        reg[:] = reg * (1 - 0.8 * qq[..., None]) + 0.97 * 0.8 * qq[..., None]
    cut('S2_QTILE', q, (42, 46, 132, 104))
    def blank(k):
        k.plate(30, 34, 450, 116)
    cut('S2_LADBLANK', blank, (38, 42, 136, 108))
    def frame(k):
        k.a[45:105, 41:43] = hx(0x30e0e0); k.a[45:105, 131:133] = hx(0x30e0e0)
        k.a[45:47, 41:133] = hx(0x30e0e0); k.a[103:105, 41:133] = hx(0x30e0e0)
    cut('S2_LADFRAME', frame, (40, 44, 134, 106))


def s2_vs():
    """VS (screens2.py vs_screen): the player's plate (portrait, the six), the opponent's (the six, who, portrait),
    the clauses' chips and a glowing VS between them. Only shows: A goes on to the choice."""
    c = C4()
    c.night(VS_LINES)
    ribbon(c, 8, 'SEMI-FINAL', 'GYM LEADER CASTLE')
    c.plate(14, 24, 466, 146)
    c.window(22, 32, 96, 104, glow=False)
    for i in range(6):
        x0, y0 = 132 + (i % 3) * 111, 32 + (i // 3) * 56
        c.window(x0, y0, x0 + 106, y0 + 52)
    c.plate(14, 170, 466, 288)
    for i in range(6):
        x0, y0 = 22 + (i % 3) * 110, 178 + (i // 3) * 54
        c.window(x0, y0, x0 + 105, y0 + 50)
    c.window(356, 220, 458, 282, glow=False)
    c.neon(240, 157, 'VS', 56, 0x40e8f0, HEAVY, hollow=False, glow=6)
    c.strip(295)
    opaque('S2_VS', c)


def s2_choice():
    """The choice (screens3.py choice_screen): both teams as plates, a pick button on each of the player's, the
    message and the opponent's name in black glass, the portraits in frames, the 1P / COM banners."""
    c = C4()
    background(c, front('tyranitar'), (120, 20), 290)
    c.tab(10, 34, -6, 150, TAB_BLUE)
    c.label_rot(22, 36, '1P', 22, 0x0c1018, 90)
    for team, box, pal in ((0, (40, 8, 360, 140), 'blue'), (1, (126, 154, 446, 286), 'green')):
        sx0, sy0, sx1, sy1 = c.slab(*box, pal)
        defines(f'SX_S2_CPLATE{team}', (sx0, sy0, sx1, sy1))
        cw, ch = (sx1 - sx0) / 3, (sy1 - sy0) / 2
        for k in (1, 2):
            c.engrave_v(int(sx0 + k * cw), sy0 + 4, sy1 - 4, pal)
        c.engrave_h(int(sy0 + ch), sx0 + 4, sx1 - 4, pal)
        if team == 0:
            for i in range(6):
                x0, y0, x1, y1 = int(sx0 + (i % 3) * cw), int(sy0 + (i // 3) * ch), int(sx0 + (i % 3 + 1) * cw), int(sy0 + (i // 3 + 1) * ch)
                cut(f'S2_CPICK{i}', lambda k, x0=x0, y0=y0, x1=x1, y1=y1, i=i: k.sheen(x0 + (2 if i % 3 else 0), y0 + (2 if i // 3 else 0), x1, y1, 'gold', (sx0, sx1)),
                    (x0 + (2 if i % 3 else 0), y0 + (2 if i // 3 else 0), x1, y1))
                cut(f'S2_CRING{i}', lambda k, x0=x0, y0=y0, x1=x1, y1=y1: k.ring(x0 - 1, y0 - 1, x1 + 2, y1 + 1), (x0 - 5, y0 - 5, x1 + 6, y1 + 5))
    defines('SX_S2_CPORT0', c.frame(388, 8, 454, 74))
    c.glass(366, 80, 474, 112)
    c.tab(452, 476, 300, 158, TAB_GREEN)
    for k in range(3):
        c.ball(464, 182 + k * 18, 7)
    c.label_rot(464, 254, 'COM', 19, 0x0a1c0c, -90)
    c.glass(6, 146, 120, 182)
    c.glass(6, 188, 120, 218)
    defines('SX_S2_CPORT1', c.frame(54, 222, 118, 286))
    c.strip(295)
    opaque('S2_CHOICE', c)
    for lit in (True, False):
        cut('S2_PICKBALL_' + ('LIT' if lit else 'OFF'), lambda k, lit=lit: k.ball(20, 20, 7, top=0x4880e8, lit=lit), (10, 10, 31, 31))
    cut('S2_PICKRED', lambda k: k.sphere(20, 20, 9.5, 0xe03828), (8, 8, 33, 33))
    cut('S2_PICKSOCKET', lambda k: k.sphere(20, 20, 8, 0x5a6278, flat=True), (9, 9, 32, 32))
    cut('S2_DIM', lambda k: k.a.__setitem__(slice(None), 0), (0, 0, 8, 8))



def s2_result():
    """The result (screens2.py result_screen): the player's plate (portrait, the three that fought, their HP), the
    opponent's under it, WIN / LOSE glowing over them, the road of balls between."""
    c = C4()
    c.night(VS_LINES)
    ribbon(c, 8, 'GYM LEADER CASTLE', 'FINAL BATTLE')
    c.plate(14, 24, 466, 146)
    c.window(22, 32, 104, 114, glow=False)
    for i in range(3):
        x0 = 112 + i * 118
        c.window(x0, 32, x0 + 112, 124)
        c.hpbar(x0, 129, x0 + 112, 139, 0)
    c.plate(14, 170, 466, 288)
    for i in range(3):
        x0 = 22 + i * 110
        c.window(x0, 178, x0 + 105, 268)
        c.hpbar(x0, 272, x0 + 105, 282, 0)
    c.window(356, 220, 458, 282, glow=False)
    c.strip(295)
    opaque('S2_RESULT', c)
    for name, word, cx, cy, px, col in (('WIN_TOP', 'WIN', 292, 78, 96, 0xf8a010), ('LOSE_TOP', 'LOSE', 292, 78, 92, 0x30e0e8),
                                        ('WIN_BOTTOM', 'WIN', 184, 222, 96, 0xf8a010), ('LOSE_BOTTOM', 'LOSE', 184, 222, 92, 0x30e0e8)):
        cut('S2_NEON_' + name, lambda k, word=word, cx=cx, cy=cy, px=px, col=col: k.neon(cx, cy, word, px, col, ITALIC),
            (max(0, cx - 200), max(0, cy - 90), min(480, cx + 200), min(320, cy + 90)))
    for lit in (True, False):
        cut('S2_ROADBALL_' + ('LIT' if lit else 'OFF'), lambda k, lit=lit: k.ball(20, 20, 7, top=0xe03828, lit=lit), (10, 10, 31, 31))
    cut('S2_CROSS', lambda k: k.cross((10, 10, 112, 100)), (10, 10, 112, 100))


def s2_prizes():
    """The cup won (reconcile.py prizes_s2): the words glowing, the road all lit, the prizes on a plate."""
    c = C4()
    c.night(VS_LINES)
    ribbon(c, 8, 'GYM LEADER CASTLE', 'MASTER BALL')
    c.plate(40, 130, 440, 286)
    c.window(50, 140, 150, 276, band=0xf0c838)
    c.strip(295)
    opaque('S2_PRIZES', c)
    for name, word in (('CUPWON', 'CUP WON!'), ('CASTLEWON', 'CASTLE WON!'), ('BADGEWON', 'BADGE WON!'), ('GAMEOVER', 'GAME OVER')):
        cut('S2_NEON_' + name, lambda k, word=word: k.neon(240, 62, word, 60 if len(word) < 10 else 52, 0x30e0e8 if word == 'GAME OVER' else 0xf8a010, ITALIC),
            (0, 0, 480, 130))


def s2_badges():
    """The GYM LEADER CASTLE's badge board (screens3.py badge_screen): eight sockets, a name plate each; the badges
    won set into theirs."""
    c = C4()
    c.night(((3, 150, 520), (150, -40, 30), (312, -40, 60)))
    for r in range(2):
        y = 22 + r * 138
        c.plate(30, y, 450, y + 134)
        for k in range(4):
            i = r * 4 + k
            col = BADGE_RGB[i]
            cx, cy = 86 + k * 104, y + 50
            sl, xx, yy = c.win(cx - 36, cy - 36, cx + 36, cy + 36)
            e = np.hypot((xx - cx) / 30.0, (yy - cy) / 30.0)
            gy, gx = np.gradient(e)
            lit = smooth(-0.01, 0.01, -gx - gy)
            ring = aa((e - 1) * 30) * (1 - aa((e - 0.84) * 30))
            c.mix(sl, aa((e - 1) * 30 - 1), hx(0x505050))
            c.mix(sl, ring, np.repeat((0.55 + 0.42 * (1 - lit))[..., None], 3, -1))
            c.mix(sl, aa((e - 0.84) * 30), np.repeat((0.42 + 0.20 * lit)[..., None], 3, -1))
            for sx, sy in ((-1, -1), (1, -1), (-1, 1), (1, 1)):
                bx, by = int(cx + sx * 36), int(cy + sy * 36)
                c.a[min(by, by - sy * 8):max(by, by - sy * 8) + 1, bx - 1:bx + 1] = 0.94
                c.a[by - 1:by + 1, min(bx, bx - sx * 8):max(bx, bx - sx * 8) + 1] = 0.94
            x0, x1 = 38 + k * 104, 38 + k * 104 + 96
            c.a[y + 100:y + 124, x0:x1] = 0.02
            u = np.linspace(0, 1, x1 - x0 - 4)
            g = hx(col)[None] * (0.75 + 0.5 * np.exp(-((u - 0.5) / 0.3) ** 2))[:, None] * 0.6 + 0.42
            c.a[y + 102:y + 122, x0 + 2:x1 - 2] = np.clip(g, 0, 1)[None]
    c.strip(295)
    opaque('S2_BADGES', c)
    for g, col in enumerate(BADGE_RGB):
        def bb(k, col=col):
            k.sphere(40, 40, 22, col)
            k.sphere(40, 40, 12, 0xe8e0d0)
        cut(f'S2_BBADGE{g}', bb, (14, 14, 67, 67))


def s2_continue():
    """Keep battling? (screens3.py continue_screen): the cup and the battle in a pale cyan box, the question under."""
    c = C4()
    c.night(((60, -40, 520), (166, -40, 130), (250, -40, 520)))
    c.cyanbox(176, 74, 304, 130)
    c.cyanbox(120, 140, 360, 244)
    c.strip(295)
    opaque('S2_CONTINUE', c)


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
    s2_register()
    s2_ladder()
    s2_vs()
    s2_choice()
    s2_result()
    s2_prizes()
    s2_badges()
    s2_continue()
    write()
