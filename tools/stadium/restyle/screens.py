# Three of STADIUM's screens (src/mp/mp_stadium_ui.c: SELECT, CLASS, VS) drawn with STADIUM 2's materials (s2.py),
# at the screens' own 480x320. What each shows is what the game's screen shows; VS is laid out like the user's
# screenshot (the two teams as staggered plates, portraits and names in glass, a banner each).
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s2 import *

OUT = os.path.dirname(os.path.abspath(__file__))


def footer(c, hints):
    c.strip(295)
    base = sum(sum(keycap(k).shape[1] // 2 + 1 for k in keys) + measure(word, SMALL) for keys, word in hints)
    gap = max(2, min(7, (231 - base) // max(1, len(hints) - 1)))
    x = 4
    for keys, word in hints:
        for k in keys:
            cap = keycap(k)
            c.blit(cap, x * 2, 149 * 2 + 1)
            x += cap.shape[1] // 2 + 1
        x += c.text(x, 148, word, SMALL, outline=hx(0x000000), ow=1, grad=0xd0d4de, bold=True) + gap


def glass_text(c, x0, y0, x1, y1, s, font=None, T=5, color=0xffffff):
    c.glass(x0, y0, x1, y1, T=T)
    if font is None:        # the bolder font where the line fits
        font = SMALL if measure(s, SMALL) * 2 <= (x1 - x0) - 2 * (T + 4) else NARROW
    c.text((x0 + x1) / 4, (y0 + y1) / 4 - 6.5, s, font, color, outline=hx(0x000000), ow=1, align='c', grad=0xd0d4de, bold=True)


def chip(c, x, y, s, color):
    """A rule chip at units; returns its width."""
    w = measure(s, NARROW) + 7
    c.pill(x * 2, y * 2, (x + w) * 2, (y + 10) * 2, color)
    c.text(x + w / 2 + 0.5, y - 1, s, NARROW, ow=1, outline=hx(0x101420), align='c')
    return w


def marks(c, x, y, won):
    """A cup's four classes as small spheres: gold (every restriction), silver, or an empty socket."""
    for t in range(4):
        cx = x + t * 13
        if won[t] == 2:
            c.sphere(cx, y, 5, 0xe8b020)
        elif won[t] == 1:
            c.sphere(cx, y, 5, 0xe4e6ee)
        else:
            c.sphere(cx, y, 4.5, 0x5a6278, flat=True)


# ---------------------------------------------------------------------------------------------------------------
def select_screen():
    c = Canvas()
    background(c, front('tauros'), (150, 20), 300, flip=True)
    c.blit(stadium_pic(8), 8, 4)                                    # STADIUM's sign
    glass_text(c, 356, 6, 472, 38, 'ROUND 1')
    # the cups: one plate, a row each
    cups = [('PIKA CUP', (2, 2, 2, 2)), ('PETIT CUP', (1, 1, 1, 1)), ('POKé CUP', (1, 1, 0, 0)), ('PRIME CUP', (0, 0, 0, 0)),
            ('CHALLENGE', (0, 0, 0, 0)), ('GYM CASTLE', (0, 0, 0, 0)), ('VS MEWTWO', (0, 0, 0, 0))]
    focus, locked = 2, {6}
    sx0, sy0, sx1, sy1 = c.slab(6, 72, 206, 286, 'blue')
    rh = (sy1 - sy0) / 7
    for i, (name, won) in enumerate(cups):
        y0, y1 = int(sy0 + i * rh), int(sy0 + (i + 1) * rh)
        if i:
            c.engrave_h(y0 - 1, sx0 + 4, sx1 - 4)
        if i == focus:
            c.sheen(sx0, y0 + 1, sx1, y1 - 1, 'gold', (sx0, sx1))
        if i in locked:
            c.sheen(sx0, y0 + 1, sx1, y1, 'grey', (sx0, sx1))
    for i, (name, won) in enumerate(cups):
        y0, y1 = int(sy0 + i * rh), int(sy0 + (i + 1) * rh)
        c.text(sx0 / 2 + 4, (y0 + y1) / 4 - 6.5, name, SMALL, 0xc4c8d2 if i in locked else 0xffffff, ow=2, grad=None if i in locked else 0xdfe4f0)
        marks(c, sx1 - 49, (y0 + y1) / 2, won)
    y0, y1 = int(sy0 + focus * rh), int(sy0 + (focus + 1) * rh)
    c.ring(sx0 - 3, y0 - 2, sx1 + 3, y1 + 1)
    # the cup's picture in a portrait's frame
    ix0, iy0, ix1, iy1 = c.frame(218, 42, 470, 230, T=5, gap=1, T2=3)
    c.blit(scale(stadium_pic(0)[2:94], ix1 - ix0, iy1 - iy0, Image.BILINEAR), ix0, iy0)
    # a light passing over it, as the game's screen has
    sl, xx, yy = c.win(ix0, iy0, ix1, iy1)
    c.mix(sl, 0.16 * np.exp(-(((xx - ix0) + (yy - iy0) * 0.45 - 150) / 16) ** 2), hx(0xffffff))
    # its rules
    gx0, gy0, gx1, gy1 = c.slab(214, 236, 474, 286, 'green')
    x = gx0 / 2 + 1.5
    x += chip(c, x, gy0 / 2 + 1, 'Lv 50-55', 0x40a050) + 1.5
    x += chip(c, x, gy0 / 2 + 1, 'SUM 155', 0x4878c0) + 1.5
    chip(c, x, gy0 / 2 + 1, '8 FOES', 0xc06030)
    c.text(gx0 / 2 + 3, gy0 / 2 + 11.5, 'No MEWTWO or MEW.', NARROW, ow=2, outline=hx(0x1c2c20))
    footer(c, [(('A',), 'ENTER'), (('L', 'R'), 'ROUND'), (('B',), 'EXIT')])
    return c


# ---------------------------------------------------------------------------------------------------------------
def class_screen():
    c = Canvas()
    background(c, front('tauros'), (120, 30), 290)
    glass_text(c, 6, 6, 150, 38, 'POKé CUP')
    glass_text(c, 356, 6, 472, 38, 'ROUND 1')
    focus = 1
    for t in range(4):
        y = 42 + t * 61
        pic = stadium_pic(4 + t)
        is_open = t <= 2
        if not is_open:                      # a class not open yet: grey, as the game draws it
            l = (pic[..., :3] * [0.30, 0.59, 0.11]).sum(-1, keepdims=True) * 0.75 + 0.08
            pic = np.concatenate([np.repeat(l, 3, -1), pic[..., 3:] * 0.6], -1)
        x = 16 + (12 if t == focus else 0)
        b = scale(pic, 220, 62, Image.LANCZOS)
        sh = np.zeros_like(b)                                    # its own shape as a hard shadow
        sh[..., 3] = b[..., 3] * (0.9 if is_open else 0.35)
        c.blit(sh, x + 3, y + 3)
        c.blit(b, x, y)
        if t == focus:
            c.ring(x - 6, y - 3, x + 226, y + 65, r=12, T=4)
        if t == 0:
            w = measure('WON', NARROW) + 9
            c.pill(248, y + 21, 248 + w * 2, y + 41, 0xc09020)
            c.text(124 + w / 2 + 0.5, (y + 21) / 2 - 1, 'WON', NARROW, ow=1, outline=hx(0x201400), align='c')
    # the class under the cursor
    sx0, sy0, sx1, sy1 = c.slab(304, 42, 474, 286, 'blue')
    ux = sx0 / 2 + 3
    c.text(ux, sy0 / 2 + 1, 'GREAT BALL', SMALL, ow=2, grad=0xdfe4f0)
    c.text(ux, sy0 / 2 + 13, '8 TRAINERS', NARROW, 0xf4f6fa, ow=2)
    faces = ['bug_catcher', 'lass', 'camper', 'picnicker', 'hiker', 'psychic_m', 'beauty', 'cooltrainer_m']
    gx, gy = sx0 + 4, sy0 + 54
    cw, ch = (sx1 - sx0 - 8) // 4, 46
    c.engrave_h(gy - 3, sx0 + 4, sx1 - 4)
    for i, name in enumerate(faces):
        x, y = gx + (i % 4) * cw, gy + (i // 4) * ch
        if i == 7:                                               # the last one: the class's boss, on gold
            c.sheen(x + 1, y, x + cw, y + ch - 2, 'gold', (sx0, sx1))
        c.blit(scale(trainer(name)[0:56, 8:56], cw - 2, int((cw - 2) * 56 / 48), Image.BILINEAR), x + 1, y)
        if i % 4:
            c.engrave_v(x - 1, y + 2, y + ch - 4)
    c.engrave_h(gy + ch - 2, sx0 + 4, sx1 - 4)
    c.engrave_h(gy + ch * 2 - 2, sx0 + 4, sx1 - 4)
    by = (gy + ch * 2) / 2 + 2
    c.text(ux, by, 'BEST', NARROW, ow=2)
    for k in range(4):
        if k < 2:
            c.sphere(sx0 + 14 + k * 20, by * 2 + 34, 7, 0xe0a040)
        else:
            c.sphere(sx0 + 14 + k * 20, by * 2 + 34, 6.5, 0x5a6278, flat=True)
    c.text(ux, by + 23, '2 of 4', NARROW, 0xf4f6fa, ow=2)
    c.text(ux, by + 33, 'restrictions', NARROW, 0xf4f6fa, ow=2)
    footer(c, [(('A',), 'CHOOSE'), (('B',), 'BACK')])
    return c


# ---------------------------------------------------------------------------------------------------------------
def team_plate(c, box, pal, mons, levels=None, picks=None, cursor=None):
    sx0, sy0, sx1, sy1 = c.slab(*box, pal)
    cw, ch = (sx1 - sx0) / 3, (sy1 - sy0) / 2
    cells = [(int(sx0 + (i % 3) * cw), int(sy0 + (i // 3) * ch), int(sx0 + (i % 3 + 1) * cw), int(sy0 + (i // 3 + 1) * ch)) for i in range(6)]
    for i, (x0, y0, x1, y1) in enumerate(cells):
        if picks and i in picks:
            c.sheen(x0 + (2 if i % 3 else 0), y0 + (2 if i // 3 else 0), x1, y1, 'gold', (sx0, sx1))
    for k in (1, 2):
        c.engrave_v(cells[k][0], sy0 + 4, sy1 - 4, pal)
    c.engrave_h(cells[3][1], sx0 + 4, sx1 - 4, pal)
    if cursor is not None:
        x0, y0, x1, y1 = cells[cursor]
        c.ring(x0 - 1, y0 - 1, x1 + 2, y1 + 1)
    for i, (x0, y0, x1, y1) in enumerate(cells):
        cx = (x0 + x1) / 2
        if levels:
            c.blit(scale(icon(mons[i]), 64, 64), cx - 32 + 4, y0 - 12 - (2 if cursor == i else 0))
            c.text(cx / 2 + 1, y1 / 2 - 14, 'L%d' % levels[i], SMALL, ow=2, align='c', grad=0xdfe4f0)
        else:
            c.blit(scale(icon(mons[i]), 64, 64), cx - 32, y1 - 62)
        if picks and i in picks:
            c.sphere(x0 + 13, y0 + 13, 9.5, 0xe03828)
            c.text((x0 + 13) / 2 + 0.5, (y0 + 13) / 2 - 6.5, str(picks[i]), SMALL, ow=1, outline=hx(0x400808), align='c')


def vs_screen():
    c = Canvas()
    background(c, front('tyranitar'), (90, 30), 300)
    # the player: banner, team, portrait, the battle and the cup in glass
    c.tab(10, 38, -6, 162, TAB_BLUE)
    c.label_rot(24, 40, '1P', 25, 0x0c1018, 90)
    picks = {0: 1, 3: 2, 5: 3}
    for k in range(3):
        c.ball(24, 84 + k * 21, 7.5, top=0x4880e8, lit=k < len(picks))
    team_plate(c, (46, 10, 278, 158), 'blue', ['ursaring', 'politoed', 'kadabra', 'hitmonlee', 'graveler', 'dragonair'],
               [50, 50, 50, 50, 50, 50], picks, cursor=1)
    ix0, iy0, ix1, iy1 = c.frame(286, 10, 352, 76)
    c.blit(trainer('brendan')[0:iy1 - iy0, 11:11 + ix1 - ix0], ix0, iy0)
    glass_text(c, 358, 10, 472, 42, 'BATTLE 3')
    glass_text(c, 358, 46, 472, 76, 'POKé CUP')
    # the clauses, the level total
    x = 143.5
    x += chip(c, x, 41, 'SLEEP', 0x585880) + 1.5
    chip(c, x, 41, 'FREEZE', 0x585880)
    x = 143.5
    x += chip(c, x, 52.5, 'SELF-KO', 0x585880) + 1.5
    chip(c, x, 52.5, 'SPECIES', 0x585880)
    chip(c, 143.5, 64, 'Lv 150/155', 0x40a050)
    # VS
    vs = chrome_word('VS', 54)
    c.blit(vs, 102 - vs.shape[1] / 2, 193 - vs.shape[0] / 2)
    # the opponent
    team_plate(c, (208, 174, 440, 286), 'green', ['starmie', 'ursaring', 'alakazam', 'umbreon', 'muk', 'tyranitar'])
    c.tab(446, 474, 300, 176, TAB_GREEN)
    for k in range(3):
        c.ball(460, 198 + k * 18, 7)
    c.label_rot(460, 264, 'COM', 20, 0x0a1c0c, -90)
    ix0, iy0, ix1, iy1 = c.frame(138, 220, 204, 286)
    c.blit(trainer('cooltrainer_m')[0:iy1 - iy0, 11:11 + ix1 - ix0], ix0, iy0)
    glass_text(c, 8, 222, 132, 252, 'COOLTRAINER')
    glass_text(c, 8, 256, 132, 286, 'MARTY')
    footer(c, [(('A',), 'PICK'), (('R',), 'SUMMARY'), (('SELECT',), 'RULES'), (('START',), 'BATTLE!'), (('B',), 'QUIT')])
    return c


if __name__ == '__main__':
    for name, fn in (('select', select_screen), ('class', class_screen), ('vs', vs_screen)):
        im = fn().image()
        im.save(f'{OUT}/s2_{name}.png')
        im.resize((960, 640), Image.NEAREST).save(f'{OUT}/s2_{name}_2x.png')
        print(name)
