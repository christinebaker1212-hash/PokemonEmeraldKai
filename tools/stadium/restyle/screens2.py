# STADIUM's REGISTER, VS and RESULT screens (src/mp/mp_stadium_ui.c) redone after the user's three STADIUM 2
# screenshots: the entry screen (team plate, tab keys, the blue list), the pre-battle plates with VS, the result
# plates with WIN / LOSE. 480x320, the game's fonts and pictures.
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s3 import *
from screens import footer, chip

OUT = os.path.dirname(os.path.abspath(__file__))
ITALIC = '/usr/share/fonts/truetype/crosextra/Carlito-BoldItalic.ttf'
HEAVY = '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'
LINES = ((27, -40, 70), (45, 400, 520), (158, -40, 520), (300, 180, 520))


def sprite_in(c, box, rgba, dim=1.0, dy=0):
    """A front picture at its own pixels in a window: what's drawn of it centred, or from its top when it's taller
    than the window (the head stays in)."""
    x0, y0, x1, y1 = box
    rows = np.nonzero(rgba[..., 3].max(1) > 0.5)[0]
    cols = np.nonzero(rgba[..., 3].max(0) > 0.5)[0]
    top, bot, left, right = rows[0], rows[-1] + 1, cols[0], cols[-1] + 1
    x = (x0 + x1) / 2 - (left + right) / 2
    if bot - top <= (y1 - y0) - 2:
        y = (y0 + y1) / 2 - (top + bot) / 2 + dy
    else:
        y = y0 + 1 - top
    c.clip_blit(rgba, x, y, box, dim=dim)


def label(c, x, y, s, align='l', color=0xffffff):
    """Writing on a silver plate: white in a dark line, as STADIUM 2's is."""
    return c.text(x, y, s, SMALL, color, ow=2, outline=hx(0x202020), align=align, bold=True)


# ---------------------------------------------------------------------------------------------------------------
def register_screen():
    c = C3()
    c.sky()
    c.ribbon(8, 'REGISTER A TEAM', 'POKé CUP')
    # the team so far: the 1P plate, three in, the fourth asked for
    c.tag(8, 30, 150)
    sx0, sy0, sx1, sy1 = c.slab(12, 40, 206, 150, 'blue')
    cw, ch = (sx1 - sx0) / 3, (sy1 - sy0) / 2
    cells = [(int(sx0 + (i % 3) * cw), int(sy0 + (i // 3) * ch), int(sx0 + (i % 3 + 1) * cw), int(sy0 + (i // 3 + 1) * ch)) for i in range(6)]
    for k in (1, 2):
        c.engrave_v(cells[k][0], sy0 + 4, sy1 - 4)
    c.engrave_h(cells[3][1], sx0 + 4, sx1 - 4)
    for i, sp in enumerate(['ursaring', 'politoed', 'kadabra']):
        x0, y0, x1, y1 = cells[i]
        c.clip_blit(scale(icon(sp), 64, 64), (x0 + x1) / 2 - 32, y1 - 60, (x0 + 2, y0 - 8, x1, y1))
    x0, y0, x1, y1 = cells[3]
    c.brackets(x0 + 5, y0 + 6, x1 - 3, y1 - 4, arm=8)
    c.text((x0 + x1) / 4 + 1, (y0 + y1) / 4 - 6, '4th', SMALL, YELLOW, ow=2, outline=hx(YELLOW_INK), align='c')
    c.text(102, 13.5, '3 / 6', SMALL, ow=2, outline=hx(0x101830), align='r')
    # the one under the cursor: its picture in a window, what it is, its moves
    c.plate(214, 28, 470, 150)
    w = c.window(222, 36, 290, 104)
    sprite_in(c, w, front('espeon'))
    label(c, 148, 17, 'ESPEON')
    label(c, 231, 17, 'L50', align='r')
    x = 148 + c.typepill(148, 30, 'PSYCHIC') + 2
    chip(c, 231 - measure('RENTAL', NARROW) - 7, 29.5, 'RENTAL', 0x4878c0)
    for i, (mv, ty) in enumerate((('PSYCHIC', 'PSYCHIC'), ('SHADOW BALL', 'GHOST'), ('MORNING SUN', 'NORMAL'), ('REFLECT', 'PSYCHIC'))):
        x, y = 111.5 + (i % 2) * 61, 53 + (i // 2) * 9.5
        c.sphere(x * 2 + 6, y * 2 + 13, 4.5, TYPE_RGB[ty])
        c.text(x + 6.5, y, mv, SMALL, ow=1, outline=hx(0x202020))
    # where from: the tab keys
    c.key(10, 156, 34, 176, 'L')
    x = 40
    for name in ('PARTY', 'BOXES', 'SHARED', 'GLOBAL', 'RENTALS'):
        wpx = (measure(name, SMALL) + 9) * 2
        c.key(x, 156, x + wpx, 176, name, selected=name == 'RENTALS')
        x += wpx + (8 if name in ('GLOBAL', 'RENTALS') else 4)
    c.key(446, 156, 470, 176, 'R')
    # the list
    bx0, by0, bx1, by1 = c.listpanel(8, 184, 472, 288)
    c.thumb(21, 210)
    names = ['EKANS', 'ELECTABUZZ', 'ELECTRODE', 'ELEKID', 'ENTEI', 'ESPEON', 'EXEGGCUTE', 'EXEGGUTOR', "FARFETCH'D", 'FEAROW',
             'FERALIGATR', 'FLAAFFY']
    folders = ['ekans', 'electabuzz', 'electrode', 'elekid', 'entei', 'espeon', 'exeggcute', 'exeggutor', 'farfetchd', 'fearow',
               'feraligatr', 'flaaffy']
    cols, rows = 3, 4
    cw, ch = (bx1 - bx0) / cols, (by1 - by0) / rows
    cursor = 5
    for i, (nm, fd) in enumerate(zip(names, folders)):
        x0, y0 = bx0 + (i % cols) * cw, by0 + (i // cols) * ch
        c.cell(x0 + 1, y0 + 1, x0 + cw - 2, y0 + ch - 1)
    for i, (nm, fd) in enumerate(zip(names, folders)):
        x0, y0 = bx0 + (i % cols) * cw, by0 + (i // cols) * ch
        c.blit(icon(fd), x0 + 5, y0 + ch / 2 - 19)
        c.text((x0 + 38) / 2, (y0 + ch / 2) / 2 - 6.5, nm, SMALL, ow=2, outline=hx(0x0c0c40))
    x0, y0 = bx0 + (cursor % cols) * cw, by0 + (cursor // cols) * ch
    c.brackets(x0 + 1, y0 + 1, x0 + cw - 2, y0 + ch - 1, arm=8)
    footer(c, [(('A',), 'MENU'), (('SELECT',), 'SUMMARY'), (('L', 'R'), 'FROM'), (('START',), 'TEAM'), (('B',), 'DONE')])
    return c


# ---------------------------------------------------------------------------------------------------------------
def vs_screen():
    c = C3()
    c.night(LINES)
    c.ribbon(8, 'BATTLE 3', 'POKé CUP')
    # the player's plate: portrait and name, the six to pick three from
    c.plate(14, 24, 466, 146)
    w = c.window(22, 32, 96, 104, glow=False)
    sprite_in(c, w, trainer('brendan'))
    label(c, 11, 51.5, 'BRENDAN')
    wd = measure('Lv 150/155', NARROW) + 7
    label(c, 11, 61.5, '1P')
    mine = ['ursaring', 'politoed', 'kadabra', 'hitmonlee', 'graveler', 'dragonair']
    picks = {}
    wins = []
    for i in range(6):
        x0 = 132 + (i % 3) * 111
        y0 = 32 + (i // 3) * 56
        box = (x0, y0, x0 + 106, y0 + 52)
        wins.append(box)
        w = c.window(*box, band=0xf0c838 if i in picks else 0xa0a2fa)
        sprite_in(c, w, front(mine[i]), dy=2)
        if i in picks:
            c.sphere(box[0] + 15, box[1] + 15, 9.5, 0xe03828)
            c.text((box[0] + 15) / 2 + 0.5, (box[1] + 15) / 2 - 6.5, str(picks[i]), SMALL, ow=1, outline=hx(0x400808), align='c')
    # the opponent's plate: their six, who they are, their picture
    c.plate(14, 170, 466, 288)
    foes = ['starmie', 'ursaring', 'alakazam', 'umbreon', 'muk', 'tyranitar']
    for i in range(6):
        x0 = 22 + (i % 3) * 110
        y0 = 178 + (i // 3) * 54
        w = c.window(x0, y0, x0 + 105, y0 + 50)
        sprite_in(c, w, front(foes[i]), dy=2)
    label(c, 176, 87, 'COOLTRAINER')
    label(c, 176, 97.5, 'MARTY')
    w = c.window(356, 220, 458, 282, glow=False)
    sprite_in(c, w, trainer('cooltrainer_m'), dy=3)
    # between them: the clauses, and VS
    x = 8
    x += chip(c, x, 74.5, 'SLEEP', 0x585880) + 2
    chip(c, x, 74.5, 'FREEZE', 0x585880)
    w2 = measure('SPECIES', NARROW) + 7
    w1 = measure('SELF-KO', NARROW) + 7
    chip(c, 232 - w2, 74.5, 'SPECIES', 0x585880)
    chip(c, 232 - w2 - 2 - w1, 74.5, 'SELF-KO', 0x585880)
    c.neon(240, 157, 'VS', 56, 0x40e8f0, HEAVY, hollow=False, glow=6)
    footer(c, [(('A',), 'CHOOSE POKéMON'), (('SELECT',), 'RULES'), (('B',), 'QUIT')])
    return c


# ---------------------------------------------------------------------------------------------------------------
def result_screen():
    c = C3()
    c.night(LINES)
    c.ribbon(8, 'POKé CUP', 'BATTLE 3')
    # the winner: the player and the three that fought, what HP they have left
    c.plate(14, 24, 466, 146)
    w = c.window(22, 32, 104, 114, glow=False)
    sprite_in(c, w, trainer('brendan'))
    label(c, 11, 58, 'BRENDAN')
    for i, (sp, hp) in enumerate((('ursaring', 0.86), ('hitmonlee', 1.0), ('dragonair', 1.0))):
        x0 = 112 + i * 118
        w = c.window(x0, 32, x0 + 112, 124)
        sprite_in(c, w, front(sp))
        c.hpbar(x0, 129, x0 + 112, 139, hp)
    c.neon(292, 78, 'WIN', 96, 0xf8a010, ITALIC)
    # the road so far, between the plates
    road_x = 150
    for i in range(8):
        c.ball(road_x + i * 22, 158, 7, top=0xe03828, lit=i < 3)
    label(c, 233, 73, 'CONTINUES 2', align='r')
    label(c, 7, 73, 'GREAT BALL')
    # the loser
    c.plate(14, 170, 466, 288)
    for i, sp in enumerate(('starmie', 'umbreon', 'tyranitar')):
        x0 = 22 + i * 110
        w = c.window(x0, 178, x0 + 105, 268)
        sprite_in(c, w, front(sp), dim=0.55)
        c.hpbar(x0, 272, x0 + 105, 282, 0.0)
    label(c, 176, 87, 'COOLTRAINER')
    label(c, 176, 97.5, 'MARTY')
    w = c.window(356, 220, 458, 282, glow=False)
    sprite_in(c, w, trainer('cooltrainer_m'), dy=3)
    c.neon(184, 222, 'LOSE', 92, 0x30e0e8, ITALIC)
    footer(c, [(('A',), 'OK')])
    return c


if __name__ == '__main__':
    for name, fn in (('register', register_screen), ('vs2', vs_screen), ('result', result_screen)):
        im = fn().image()
        im.save(f'{OUT}/s3_{name}.png')
        im.resize((960, 640), Image.NEAREST).save(f'{OUT}/s3_{name}_2x.png')
        print(name)
