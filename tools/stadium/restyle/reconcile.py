# The game's STADIUM screens the recordings don't show, in both themes: the cup's menu with its restrictions
# (DrawOptions: REGISTER / SAVED TEAMS / START!), the GYM LEADER CASTLE's stages (DrawCastle), SAVED TEAMS (DrawTeams),
# THE RULES (DrawVs, SELECT), the cup won with its prizes (DrawResult RES_CUP + DrawPrizes). 480x320.
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s3 import *
from screens import footer, chip, glass_text, marks
from screens2 import sprite_in, label, LINES, ITALIC, HEAVY
from screens3 import C4, NAVY
import screens2, stadium1
from stadium1 import S1, SHADOW

OUT = os.path.dirname(os.path.abspath(__file__))
RESTR = [('LEVEL CAP', 0, 1), ('SPECIES', 1, 1), ('HELD ITEMS', 0, 1), ('RENTALS ONLY', 0, 0)]      # name, on, counts as hard
HELP = 'Your own levels. Bigger rewards!'
TEAM = ['ursaring', 'politoed', 'kadabra', 'hitmonlee']
GYMS = [('PEWTER GYM', 'BROCK', 0xa8a8b0), ('CERULEAN GYM', 'MISTY', 0x58a0f0), ('VERMILION GYM', 'LT. SURGE', 0xf0a030),
        ('CELADON GYM', 'ERIKA', 0x70c868), ('FUCHSIA GYM', 'KOGA', 0xe878b8), ('SAFFRON GYM', 'SABRINA', 0xe0c040),
        ('CINNABAR GYM', 'BLAINE', 0xe85038), ('VIRIDIAN GYM', 'GIOVANNI', 0x48a870), ('ELITE FOUR', '', 0)]
FACES = ['leader_roxanne', 'leader_brawly', 'leader_wattson', 'leader_flannery', 'leader_norman', 'leader_winona', 'leader_tate_and_liza',
         'leader_juan', 'champion_wallace']
CLAUSES = [('SLEEP CLAUSE', 'Your moves can put only one of their POKéMON to sleep.'), ('FREEZE CLAUSE', 'Only one of their POKéMON can be frozen at a time.'),
           ('SELF-KO CLAUSE', 'SELFDESTRUCT or EXPLOSION on your last POKéMON loses.'), ('SPECIES CLAUSE', 'One of each POKéMON.'),
           ('NO ITEMS', 'No BAG items in battle.')]
SAVED = [('BRENDAN', 'POKé CUP'), ('BRENDAN', 'PIKA CUP'), ('MAY', 'PRIME CUP'), None, ('WALLY', 'PETIT CUP'), None, None, None]
PRIZES = ['MONEY 20000', 'RARE CANDY x3', 'PP UP', 'GOLD RIBBON']


def wrap(s, width, font=SMALL):
    out, line = [], ''
    for w in s.split(' '):
        if line and measure(line + ' ' + w, font) > width:
            out.append(line); line = w
        else:
            line = (line + ' ' + w).strip()
    return out + [line]


def switch(c, x, y, on, s1=False):
    """A restriction's switch (units): a glossy slider, green ON / red OFF."""
    c.pill(x * 2, y * 2 + 1, (x + 34) * 2, (y + 10) * 2 + 1, 0x40a050 if on else 0xa04040)
    c.sphere((x + (28 if on else 6)) * 2, y * 2 + 11, 7, 0xf0f0f4)
    c.text(x + (13 if on else 21), y - 1, 'ON' if on else 'OFF', NARROW, ow=1, outline=hx(0x181818), align='c')


# ---------------------------------------------------------------------------------------------------------------
# POKESTADIUM 2 THEME
def options_s2():
    c = C4()
    background(c, front('tauros'), (140, 30), 290)
    glass_text(c, 6, 6, 250, 38, 'POKé CUP  GREAT BALL'); glass_text(c, 356, 6, 472, 38, 'ROUND 1')
    ix0, iy0, ix1, iy1 = c.frame(8, 44, 200, 196, T=5, gap=1, T2=3)
    c.blit(scale(stadium_pic(0), ix1 - ix0, iy1 - iy0, Image.BILINEAR), ix0, iy0)
    b = scale(stadium_pic(5), 170, 48, Image.LANCZOS); c.blit(b, 19, 140)
    sx0, sy0, sx1, sy1 = c.slab(208, 44, 474, 196, 'blue')
    c.text(108, 23.5, 'RESTRICTIONS', SMALL, ow=2, grad=0xdfe4f0)
    for k in range(4):
        c.sphere(412 + k * 16, 60, 6, 0xe8b020) if k < 3 else c.sphere(412 + k * 16, 60, 5.5, 0x5a6278, flat=True)
    for i, (nm, on, hard) in enumerate(RESTR):
        y = 36 + i * 12.5
        if i == 0:
            c.sheen(sx0, int(y * 2) - 1, sx1, int(y * 2) + 23, 'gold', (sx0, sx1)); c.ring(sx0 - 2, y * 2 - 3, sx1 + 2, y * 2 + 25)
        c.text(108, y, nm, SMALL, ow=2, grad=0xdfe4f0)
        switch(c, 186, y + 1, on)
        if hard:
            c.sphere(457, y * 2 + 12, 5, 0xe8b020)
    c.glass(208, 202, 474, 254, T=5)
    for k, ln in enumerate(wrap(HELP, 118)):
        c.text(170.5, 102.5 + k * 10, ln, SMALL, outline=hx(0x000000), ow=1, align='c', grad=0xd0d4de, bold=True)
    gx0, gy0, gx1, gy1 = c.slab(8, 202, 200, 254, 'green')
    for i in range(6):
        x = gx0 + 2 + i * 30
        if i < len(TEAM):
            c.clip_blit(scale(icon(TEAM[i]), 64, 64), x - 16, gy0 - 20, (gx0, gy0 - 6, gx1, gy1))
        else:
            c.sphere(x + 16, (gy0 + gy1) / 2, 6, 0x5a7860, flat=True)
    c.a[236:252, 208:474] = c.a[236:252, 208:474]
    for i, word in enumerate(('REGISTER 4 / 6', 'SAVED TEAMS', 'START!')):
        x0 = 8 + i * 156
        glass_text(c, x0, 259, x0 + 152, 287, word)
        if i == 0:
            c.ring(x0 - 3, 256, x0 + 155, 290, r=8)
    footer(c, [(('A',), 'CHANGE / CHOOSE'), (('B',), 'BACK')])
    return c


def castle_s2():
    c = C4()
    background(c, None)
    glass_text(c, 6, 6, 220, 38, 'GYM LEADER CASTLE'); glass_text(c, 356, 6, 472, 38, 'ROUND 1')
    sx0, sy0, sx1, sy1 = c.slab(6, 44, 300, 286, 'blue')
    rh = (sy1 - sy0) / 9
    for i, (nm, who, col) in enumerate(GYMS):
        y0, y1 = int(sy0 + i * rh), int(sy0 + (i + 1) * rh)
        if i:
            c.engrave_h(y0 - 1, sx0 + 4, sx1 - 4)
        if i == 3:
            c.sheen(sx0, y0 + 1, sx1, y1 - 1, 'gold', (sx0, sx1))
        if i == 8:
            c.sheen(sx0, y0 + 1, sx1, y1, 'grey', (sx0, sx1))
    for i, (nm, who, col) in enumerate(GYMS):
        y0, y1 = int(sy0 + i * rh), int(sy0 + (i + 1) * rh)
        c.clip_blit(trainer(FACES[i]), sx0 - 12, y0 - 4, (sx0 + 2, y0 + 1, sx0 + 36, y1 - 1))
        c.text(sx0 / 2 + 20, (y0 + y1) / 4 - 6.5, nm, SMALL, 0xc4c8d2 if i == 8 else 0xffffff, ow=2, grad=None if i == 8 else 0xdfe4f0)
        c.text(sx0 / 2 + 84, (y0 + y1) / 4 - 6.5, who if i < 8 else 'LOCKED', SMALL, 0xe8ecf8 if i < 8 else 0xc4c8d2, ow=2)
        if i < 8:
            c.sphere(sx1 - 14, (y0 + y1) / 2, 7, col) if i < 3 else c.sphere(sx1 - 14, (y0 + y1) / 2, 6, 0x5a6278, flat=True)
    y0, y1 = int(sy0 + 3 * rh), int(sy0 + 4 * rh)
    c.ring(sx0 - 3, y0 - 2, sx1 + 3, y1 + 1)
    gx0, gy0, gx1, gy1 = c.slab(306, 44, 474, 286, 'green')
    ux = gx0 / 2 + 3
    ink = hx(0x1c2c20)
    c.text(ux, gy0 / 2 + 1, 'CELADON GYM', SMALL, ow=2, outline=ink, grad=0xdfe4f0)
    c.text(ux, gy0 / 2 + 12, '4 BATTLES', SMALL, ow=2, outline=ink)
    for i, f in enumerate(('lass', 'beauty', 'picnicker', 'leader_flannery')):
        x, y = gx0 + 2 + i * 40, gy0 + 52
        if i == 3:
            c.sheen(x, y, x + 38, y + 50, 'gold', (gx0, gx1))
        c.clip_blit(trainer(f), x - 12, y - 4, (x, y, x + 38, y + 50))
    c.engrave_h(gy0 + 106, gx0 + 4, gx1 - 4, 'green')
    c.text(ux, gy0 / 2 + 56, 'BADGES 3 / 8', SMALL, ow=2, outline=ink)
    for i in range(8):
        cx, cy = gx0 + 16 + (i % 4) * 26, gy0 + 148 + (i // 4) * 24
        c.sphere(cx, cy, 8, GYMS[i][2]) if i < 3 else c.sphere(cx, cy, 7, 0x5a7860, flat=True)
    c.text(ux, gy0 / 2 + 97, 'RAINBOWBADGE', SMALL, ow=2, outline=ink, grad=0xdfe4f0)
    footer(c, [(('A',), 'CHOOSE'), (('B',), 'BACK')])
    return c


def teams_s2():
    c = C4()
    c.sky()
    c.ribbon(8, 'SAVED TEAMS', 'SERVER 3 / 99')
    bx0, by0, bx1, by1 = c.listpanel(8, 30, 250, 288)
    c.thumb(21, 56)
    rh = (by1 - by0) / 8
    for i, row in enumerate(SAVED):
        y0 = by0 + i * rh
        c.cell(bx0 + 1, y0 + 1, bx1 - 1, y0 + rh - 1)
        c.text(bx0 / 2 + 3, (y0 + rh / 2) / 2 - 6.5, '%02d' % (i + 1), SMALL, 0xc8d0ff, ow=2, outline=NAVY)
        if row:
            c.text(bx0 / 2 + 19, (y0 + rh / 2) / 2 - 6.5, row[0], SMALL, ow=2, outline=NAVY)
            c.text(bx1 / 2 - 3, (y0 + rh / 2) / 2 - 6.5, row[1], SMALL, 0xffe060, ow=2, outline=NAVY, align='r')
        else:
            c.text(bx0 / 2 + 19, (y0 + rh / 2) / 2 - 6.5, '- - - - -', SMALL, 0x8890e0, ow=2, outline=NAVY)
    c.brackets(bx0 + 1, by0 + 2 * rh + 1, bx1 - 1, by0 + 3 * rh - 1, arm=8)
    c.plate(258, 30, 470, 288)
    label(c, 133, 17, 'SLOT 03'); label(c, 231, 17, 'PRIME CUP', align='r'); label(c, 133, 28, 'by MAY')
    for i, sp in enumerate(['starmie', 'alakazam', 'tauros', 'snorlax', 'jolteon', 'exeggutor']):
        x0, y0 = 266 + (i % 3) * 67, 86 + (i // 3) * 98
        w = c.window(x0, y0, x0 + 62, y0 + 72)
        sprite_in(c, w, front(sp))
        label(c, (x0 + 31) / 2, (y0 + 72) / 2, 'L50', align='c')
    footer(c, [(('A',), 'MENU'), (('B',), 'BACK')])
    return c


def rules_s2():
    c = screens2.vs_screen()
    c.a *= 0.35
    c.__class__ = C4
    c.bluepanel(30, 4, 450, 316, color=(0x7c98f4, 0x6c88ec))
    c.text(120, 4, 'THE RULES', SMALL, YELLOW, ow=2, outline=NAVY, align='c', bold=True)
    y = 15
    for nm, text in CLAUSES:
        c.text(22, y, nm, SMALL, 0xffe060, ow=2, outline=NAVY)
        for ln in wrap(text, 196):
            y += 10
            c.text(22, y, ln, SMALL, ow=2, outline=NAVY)
        y += 11.5
    return c


def prizes_s2():
    c = C4()
    c.night(LINES)
    c.ribbon(8, 'POKé CUP', 'GREAT BALL')
    c.neon(240, 62, 'CUP WON!', 60, 0xf8a010, ITALIC)
    for i in range(8):
        c.ball(163 + i * 22, 112, 7, top=0xe03828)
    c.plate(40, 130, 440, 286)
    w = c.window(50, 140, 150, 276, band=0xf0c838)
    sprite_in(c, w, scale(front('eevee'), 128, 128))
    label(c, 82, 71, 'PRIZES', color=0xffe060)
    label(c, 82, 82.5, 'EEVEE  L25  3 perfect IVs')
    for i, p in enumerate(PRIZES):
        label(c, 82 + (i % 2) * 70, 98 + (i // 2) * 11, p)
    footer(c, [(('A',), 'OK')])
    return c


# ---------------------------------------------------------------------------------------------------------------
# POKESTADIUM THEME
def options_s1():
    c = S1()
    c.cup_backdrop(0, dim=0.15)
    c.titlebar('POKé CUP  GREAT BALL'); c.t(214, 3.5, 'ROUND 1', align='r')
    c.box(160, 40, 468, 188)
    c.t(86, 22, 'RESTRICTIONS', 0xf8f040, bold=True)
    for k in range(4):
        c.sphere(402 + k * 16, 58, 6, 0xe8b020) if k < 3 else c.sphere(402 + k * 16, 58, 5.5, 0x5060a0, flat=True)
    for i, (nm, on, hard) in enumerate(RESTR):
        y = 35 + i * 12.5
        if i == 0:
            c.wbrackets(166, int(y * 2) - 2, 462, int(y * 2) + 24)
        c.t(88, y, nm); switch(c, 178, y + 1, on)
        if hard:
            c.sphere(448, y * 2 + 12, 5, 0xe8b020)
    c.veil(14, 196, 466, 222); c.t(120, 98.5, HELP, align='c')
    c.box(14, 40, 152, 188)
    c.t(41.5, 22, 'TEAM 4 / 6', align='c')
    for i in range(6):
        x, y = 22 + (i % 2) * 64, 72 + (i // 2) * 38
        c.a[y:y + 34, x:x + 58] = hx(0x5858d8) if (i + i // 2) % 2 == 0 else hx(0x3c3cb8)
        if i < len(TEAM):
            c.clip_blit(scale(icon(TEAM[i]), 64, 64), x - 3, y - 22, (x, y - 8, x + 58, y + 34))
    for i, word in enumerate(('START!', 'SAVED TEAMS', 'REGISTER')):
        x0 = 14 + i * 152
        c.box(x0, 232, x0 + 148, 262)
        c.t((x0 + 74) / 2, 118, word, align='c')
        if i == 0:
            c.wbrackets(x0 - 4, 228, x0 + 152, 266)
    c.veil(0, 292, 480, 320, 0x101060, 0.75, 0)
    c.hints([('A', 'CHANGE / CHOOSE'), ('B', 'BACK')])
    return c


def castle_s1():
    c = S1()
    c.cup_backdrop(1, dim=0.3, zoom=1.4)
    c.titlebar('GYM LEADER CASTLE'); c.t(214, 3.5, 'ROUND 1', align='r')
    x0, y0, x1, y1 = 14, 36, 300, 288
    c.a[y0 - 2:y1 + 2, x0 - 2:x1 + 2] = hx(0x101030); c.a[y0 - 1:y1 + 1, x0 - 1:x1 + 1] = hx(0xe8e8ff)
    rh = (y1 - y0) / 9
    for i, (nm, who, col) in enumerate(GYMS):
        ya, yb = int(y0 + i * rh), int(y0 + (i + 1) * rh)
        c.a[ya:yb, x0:x1] = hx(0x30306c) if i == 8 else hx(0x5858d8) if i % 2 == 0 else hx(0x4444c4)
        c.clip_blit(trainer(FACES[i]), x0 - 12, ya - 4, (x0 + 2, ya + 1, x0 + 36, yb - 1))
        c.t(x0 / 2 + 20, (ya + yb) / 4 - 6.5, nm, 0x9898c8 if i == 8 else 0xffffff)
        c.t(x0 / 2 + 82, (ya + yb) / 4 - 6.5, who if i < 8 else 'LOCKED', 0xf8f040 if i < 8 else 0x9898c8)
        if i < 8:
            c.sphere(x1 - 14, (ya + yb) / 2, 7, col) if i < 3 else c.sphere(x1 - 14, (ya + yb) / 2, 6, 0x30308c, flat=True)
        if i == 3:
            c.wbrackets(x0 + 2, ya + 2, x1 - 2, yb - 2)
    c.box(308, 36, 468, 288)
    c.t(159, 21, 'CELADON GYM', 0xf8f040, bold=True); c.t(159, 32, '4 BATTLES')
    for i, f in enumerate(('lass', 'beauty', 'picnicker', 'leader_flannery')):
        x, y = 314 + i * 38, 92
        c.a[y:y + 50, x:x + 36] = hx(0x5050d0) if i < 3 else hx(0xb030a0)
        c.clip_blit(trainer(f), x - 14, y - 4, (x, y, x + 36, y + 50))
    c.t(159, 76, 'BADGES 3 / 8')
    for i in range(8):
        cx, cy = 326 + (i % 4) * 26, 186 + (i // 4) * 24
        c.sphere(cx, cy, 8, GYMS[i][2]) if i < 3 else c.sphere(cx, cy, 7, 0x5060a0, flat=True)
    c.t(159, 118, 'RAINBOWBADGE', 0xf8f040)
    c.veil(0, 292, 480, 320, 0x101060, 0.75, 0)
    c.hints([('A', 'CHOOSE'), ('B', 'BACK')])
    return c


def teams_s1():
    """After STADIUM's Registered Pokémon list: a row a team, its six beside who saved it."""
    c = S1()
    c.cup_backdrop(3, dim=0.3, zoom=1.4)
    c.titlebar('Saved teams'); c.t(214, 3.5, 'SERVER 3 / 99', align='r')
    x0, y0, x1, y1 = 14, 36, 466, 288
    c.a[y0 - 2:y1 + 2, x0 - 2:x1 + 2] = hx(0x101030); c.a[y0 - 1:y1 + 1, x0 - 1:x1 + 1] = hx(0xe8e8ff)
    c.a[y0:y1, x0:x1] = hx(0x2c2ca0)
    c.thumb(x0 + 7, y0 + 60)
    teams = [['raichu', 'poliwrath', 'arcanine', 'exeggutor', 'drowzee', 'dratini'], ['voltorb', 'cloyster', 'diglett', 'starmie', 'abra', 'dratini'],
             ['starmie', 'alakazam', 'tauros', 'snorlax', 'jolteon', 'exeggutor'], None, ['clefairy', 'jigglypuff', 'pikachu', 'vulpix', 'oddish', 'meowth']]
    rh = (y1 - y0) / 5
    for i in range(5):
        ya, yb = int(y0 + i * rh), int(y0 + (i + 1) * rh)
        c.a[ya + 1:yb, x0 + 14:x1] = hx(0x5858d8) if i % 2 == 0 else hx(0x4444c4)
        c.t(x0 / 2 + 10, ya / 2 + 2, '%02d' % (i + 1), 0xc8d0ff)
        row = SAVED[i]
        if row:
            c.t(x0 / 2 + 26, ya / 2 + 2, row[0]); c.t(x0 / 2 + 26, ya / 2 + 12, row[1], 0xf8f040)
            for k, sp in enumerate(teams[i]):
                c.clip_blit(scale(icon(sp), 64, 64), x0 + 150 + k * 50, ya - 12, (x0 + 14, ya + 1, x1, yb))
        else:
            c.t(x0 / 2 + 26, ya / 2 + 7, '- - - - -', 0x9898e0)
        if i == 2:
            c.wbrackets(x0 + 16, ya + 3, x1 - 2, yb - 2)
    c.veil(0, 292, 480, 320, 0x101060, 0.75, 0)
    c.hints([('A', 'MENU'), ('B', 'BACK')])
    return c


def rules_s1():
    c = stadium1.vs_screen()
    c.a *= 0.35
    c.box(30, 4, 450, 316)
    c.t(120, 4, 'THE RULES', 0xf8f040, align='c', bold=True)
    y = 15
    for nm, text in CLAUSES:
        c.t(22, y, nm, 0xf8f040)
        for ln in wrap(text, 196):
            y += 10
            c.t(22, y, ln)
        y += 11.5
    return c


def prizes_s1():
    c = S1()
    c.stars()
    c.fat(240, 58, 'CUP WON!', 58, 0xfff8a0, 0xe8a010, 0x503008)
    for i in range(8):
        c.ball(163 + i * 22, 112, 7, top=0xe82020)
    cells = c.gridplate(40, 130, 180, 286, 1, 1)
    c.clip_blit(scale(front('eevee'), 128, 128), 46, 140, cells[0])
    c.box(186, 130, 440, 286)
    c.t(98, 68, 'PRIZES', 0xf8f040, bold=True)
    c.t(98, 80, 'EEVEE  L25'); c.t(98, 90.5, '3 perfect IVs')
    for i, p in enumerate(PRIZES):
        c.t(98, 101 + i * 9.6, p, 0xf8e8a0)
    c.hints([('A', 'OK')], y=147)
    return c


if __name__ == '__main__':
    for theme, fns in (('s2', (options_s2, castle_s2, teams_s2, rules_s2, prizes_s2)), ('p1', (options_s1, castle_s1, teams_s1, rules_s1, prizes_s1))):
        ims = []
        for fn in fns:
            im = fn().image(); im.save(f'{OUT}/r_{fn.__name__}.png'); ims.append(im); print(fn.__name__)
        s = Image.new('RGB', (968, 3 * 328 - 8), (40, 40, 48))
        for i, im in enumerate(ims):
            s.paste(im, ((i % 2) * 488, (i // 2) * 328))
        s.resize((s.width * 2, s.height * 2), Image.NEAREST).save(f'{OUT}/r_all_{theme}.png')
