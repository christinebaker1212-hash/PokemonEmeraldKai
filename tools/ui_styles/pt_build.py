"""PLATINUM as the game draws it (uipack.py): pt.py's pieces made into a pack's fonts, pictures and specs, from the
user's Platinum ROM (its fonts and battle backgrounds) and pret/pokeplatinum's res/ (POKEPLATINUM). Every spec puts
things where pt.py's mock-ups put them (approved 2026-11-16); check_specs.py draws both and compares them. The pack is
never committed: the release builds it (tools/sinnoh/rom_pack.sh), as SINNOH's own."""
import os
import re

from PIL import Image

import dsfont
import layout as Lo
import uipack as U
from uipack import grey
from gb_build import diff, map_spec
from pt import Art, Platinum, R

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
# each START entry's icon (start_menu.c MENU_ACTION_*: start_menu/icons cells 0 POKéDEX, 1 POKéMON, 2 BAG, 3 the
# trainer card, 4 SAVE, 5 OPTIONS (the DS: KAI's devices too), 6 EXIT, 7 the chat (CO-OP), 8 the flag (RETIRE))
START_ICONS = [0, 1, 2, 5, 3, 4, 5, 6, 8, 3, 4, 8, 2, 7, 1, 5, 5]
TYPES = ['NORMAL', 'FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', 'NORMAL', 'FIRE', 'WATER',
         'GRASS', 'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK', 'FAIRY']     # (KAI's TYPE_* order; MYSTERY: NORMAL)


def kai_font(name):
    """KAI's own font sheet (graphics/fonts/latin_<name>.png, src/fonts.c widths): (sheet, widths)"""
    src = open(os.path.join(ROOT, 'src', 'fonts.c')).read()
    table = {'normal': 'gFontNormalLatinGlyphWidths', 'small': 'gFontSmallLatinGlyphWidths'}[name]
    body = re.search(table + r'\[\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
    widths = [int(v) for v in re.findall(r'\d+', re.sub(r'//.*', '', body))]
    return Image.open(os.path.join(ROOT, 'graphics', 'fonts', 'latin_%s.png' % name)), widths


def kai_glyph(sheet, widths, k):
    gp = sheet.load()
    gx, gy = (k % 16) * 16, (k // 16) * 16
    if gy + 16 > sheet.height or k >= len(widths):
        return None
    return [[(gp[gx + x, gy + y] if gp[gx + x, gy + y] in (1, 2) else 0) for x in range(widths[k])]
            for y in range(16)], widths[k]


class PTPack:
    def __init__(self, pret, rom):
        if pret:
            os.environ['POKEPLATINUM'] = pret
        self.st = Platinum(rom, None, None)
        self.p = U.Pack('PLATINUM')
        self.pal = self.st.pal

    # ---------------------------------------------------------------------------------------------------- fonts
    def ds_font(self, name, font):
        """a Platinum font with a glyph for each of KAI's codes (KAI's own normal font where Platinum has none)"""
        f = U.Font(16)
        sheet, widths = kai_font('normal')
        for code, ch in sorted(U.kai_codes().items()):
            if code in (U.FIELD, U.EOS, U.NEWLINE):
                continue
            if ch in dsfont.FULL:
                rows, adv = font.glyph(ch)
                pix = [[v if v in (1, 2, 3) else 0 for v in r] for r in rows]
            else:
                g = kai_glyph(sheet, widths, code)
                if g is None:
                    continue
                pix, adv = g
            f.add(code, adv, 0, 0, pix)
        return self.p.font(name, f)

    def fonts(self):
        st = self.st
        self.fn = self.ds_font('pt.sys', st.font)
        self.fm = self.ds_font('pt.msg', st.fonts()['msg'])
        self.fsub = self.ds_font('pt.sub', st.fonts()['sub'])
        sheet, widths = kai_font('small')
        f = U.Font(16)
        for code in range(256):
            g = kai_glyph(sheet, widths, code)
            if g and code not in (U.FIELD, U.EOS, U.NEWLINE):
                f.add(code, g[1], 0, 0, g[0])
        self.fsmall = self.p.font('kai.small', f)
        # the special characters' digits (font_special_chars 0-9, eight wide), a space as wide
        f = U.Font(8)
        for d in range(10):
            g = st.special_glyph(d)
            f.add(0xA1 + d, 8, 0, 0, [[v if v in (1, 2) else 0 for v in r] for r in g])
        f.add(0x00, 8, 0, 0, [])
        f.add(0xBA, 8, 0, 0, [[v if v in (1, 2) else 0 for v in r] for r in st.special_glyph(10)])       # (/)
        f.add(0x34, 16, 0, 0, [[v if v in (1, 2) else 0 for v in r] for r in st.special_glyph(11, 16)])  # (Lv.)
        # (a P drawn as the set's own D (special 15-16, 'ID'), for the summary's PP where the system font's twelve
        # rows don't fit)
        P = ['........', '####+...', '##++#+..', '##+.#+..', '####++..', '##+++...', '##+.....', '++......']
        f.add(0xCA, 7, 0, 0, [[{'#': 1, '+': 2}.get(c, 0) for c in r] for r in P])
        self.fdig = self.p.font('pt.digits', f)

    # ---------------------------------------------------------------------------------------------------- helpers
    def col(self, k):
        return self.pal[k]

    def img(self, sp, name, im, x=0, y=0, **k):
        sp.add(U.OP_IMG, x=x, y=y, a=self.p.image(name, im), **k)

    def text(self, sp, s, x, y, font, ink, shadow=None, outline=None, **k):
        sp.add(U.OP_TEXT, x=x, y=y, a=self.p.string(s), font=font, ink=ink, shadow=shadow, outline=outline, **k)

    def tile_img(self, tile, ground=None, w=8):
        """a healthbox part's index pixels as a picture (its GROUND index and 0 left clear)"""
        im = Image.new('RGBA', (w, len(tile)), (0, 0, 0, 0))
        q = im.load()
        for y, r in enumerate(tile):
            for x, v in enumerate(r[:w]):
                if v and v != ground:
                    q[x, y] = self.pal[v] + (255,)
        return im

    def icon_img(self, sex, w=16):
        """Platinum's gender / Lv icon (two tiles over two), as healthbox.c puts it on the box"""
        top, bot = {'m': (62, 74), 'f': (60, 72)}.get(sex, (64, 76))
        st = self.st
        im = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
        for k, (t0, t1) in enumerate(((top, bot), (top + 1, bot + 1))):
            im.alpha_composite(self.tile_img(st.part(t0), 15), (8 * k, 0))
            im.alpha_composite(self.tile_img(st.part(t1), 15), (8 * k, 8))
        return im.crop((0, 0, w, 16))

    def gauge_imgs(self, name, size, cells, x, y):
        """the gauge's cells empty and full in each colour (healthbox_parts 2 / 11 / 20 + pixels), rows 1-6"""
        st = self.st

        def draw(base, n):
            im = Image.new('RGBA', size, (0, 0, 0, 0))
            for i in range(cells):
                t = st.part(base + n)
                for yy in range(1, 7):
                    for xx in range(8):
                        v = t[yy][xx]
                        if v:
                            im.putpixel((x + 8 * i + xx, y - 1 + yy), self.pal[v] + (255,))
            return im
        empty = draw(2, 0)
        fills = [self.p.image('%s.fill.%s' % (name, t), diff(draw(b, 8), empty)) for t, b in zip('gyr', (2, 11, 20))]
        return empty, fills

    # ---------------------------------------------------------------------------------------------------- the boxes
    def status_imgs(self):
        """Platinum's status badges (healthbox_parts HEALTHBOX_PART_STATUS_*_0..2: PAR 41, FRZ 44, SLP 47, PSN 50,
        BRN 53), by the status's bit"""
        if getattr(self, '_status', None):
            return self._status
        st = self.st
        self._status = {}
        for bit, base in ((U.C_PAR, 41), (U.C_FRZ, 44), (U.C_SLP, 47), (U.C_PSN, 50), (U.C_BRN, 53)):
            rows = [st.part(base)[y] + st.part(base + 1)[y] + st.part(base + 2)[y] for y in range(8)]
            self._status[bit] = self.p.image('pt.status.%d' % bit, self.tile_img(rows, None, 24))
        return self._status

    def status(self, sp, x, y, cond=0, unless=0):
        for bit, im in self.status_imgs().items():
            sp.add(U.OP_IMG, x=x, y=y, a=im, cond=cond | bit, unless=unless)

    def hud(self, kind):
        st, p = self.st, self.p
        rect = {'foe1': Lo.SINGLE_FOE, 'mine1': Lo.SINGLE_MINE, 'foe2': Lo.DOUBLE_FOES[0],
                'mine2': Lo.DOUBLE_MINE[0], 'foe3': Lo.TRIPLE_FOES[0], 'mine3': Lo.TRIPLE_MINE[0]}[kind]
        w, h = rect[2] - rect[0], rect[3] - rect[1]
        sp = U.Spec('hud.' + kind, w, h)
        foe = kind.startswith('foe')
        if kind in ('foe3', 'mine3'):
            return self.card(sp, foe, w)
        k = 'foe' if foe else 'mine' if kind == 'mine1' else 'mine2'
        S = st.SPECS[k]
        ys = st.ROWS[(k, h)]
        rb = w - len(S['right'])
        fr = lambda X: rb + (X - S['rb'])  # noqa: E731
        fl = lambda X: X - S['left'][0]  # noqa: E731
        bar = ys.index(S['bar'])
        wy = ys.index(19) - 3
        nx = fl(S['name'][0])
        # the box with nothing in it: its art, arrow, HP label, empty gauge, the bar's end, the empty EXP line, the
        # HP slash (healthbox.c's own pieces; the words and the gender / Lv icon are drawn by the game)
        saved = st.name_put, st.special_put, st.put
        puts = []
        st.name_put = lambda rows, x, y, s, small=False: x
        st.special_put = lambda *a, **kw: None
        st.put = lambda rows, x, y, tile, ground=None: puts.append((rows, x, y, tile, ground))
        try:
            m0 = dict(name='', level=1, sex=None, hp=0.0 if foe else 0, maxhp=1, exp=0.0)
            pic, bx = st.box(k, w, h, m0, foe)
        finally:
            st.name_put, st.special_put, st.put = saved
        if k == 'mine':
            # (the slash, the last part put: drawn into the empty box)
            _rows, sx, sy, tile, ground = puts[-1]
            pic.alpha_composite(self.tile_img(tile, ground), (bx + sx, sy))
        self.img(sp, sp.name + '.box', pic, -bx, 0)
        empty, fills = self.gauge_imgs(sp.name, (w + 8, h), 6, fr(S['gauge']), bar)
        sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
        if k == 'mine':
            ey = ys.index(48)
            ex0, ex1 = fl(S['exp'][0]), rb - 2
            line = Image.new('RGBA', (w, h), (0, 0, 0, 0))
            line.paste(self.col(12) + (255,), (ex0, ey + 2, ex1, ey + 3))
            e = p.image(sp.name + '.exp', line)
            sp.add(U.OP_BAR, a=U.F_EXP, b=e, c=e, d=e, cond=U.C_EXP)
        ink, sh = self.col(14), self.col(2)
        # the name: Platinum's letters, or KAI's small font a pixel left and down when it and the level wouldn't fit
        lvc = ((1, U.C_LV1, 0), (2, 0, U.C_LV1 | U.C_LV3), (3, U.C_LV3, 0))
        for nd, cond, unless in lvc:
            sp.add(U.OP_LONG, a=U.F_NAME, font=self.fn, w=min(64, rb - 16 - 8 * nd - nx) - 1, cond=cond,
                   unless=unless)
        self.text(sp, '{%d}' % U.F_NAME, nx, wy, self.fn, ink, sh, unless=U.C_LONG)
        self.text(sp, '{%d}' % U.F_NAME, nx - 1, wy + 1, self.fsmall, ink, sh, cond=U.C_LONG)
        icons = {s: p.image('pt.icon.%s' % (s or 'none'), self.icon_img(s)) for s in ('m', 'f', None)}
        for nd, cond, unless in lvc:
            for small in (False, True):
                lx = min(nx - (1 if small else 0) + 64, rb - 16 - 8 * nd)
                c2 = cond | (U.C_LONG if small else 0)
                u2 = unless | (0 if small else U.C_LONG)
                for s, gc, gu in (('m', U.C_MALE, 0), ('f', U.C_FEMALE, 0), (None, 0, U.C_MALE | U.C_FEMALE)):
                    sp.add(U.OP_IMG, x=lx, y=wy, a=icons[s], cond=c2 | gc, unless=u2 | gu)
                self.text(sp, '{%d}' % U.F_LEVEL, lx + 16, wy + 4, self.fdig, ink, sh, cond=c2, unless=u2)
        if foe:
            # (healthbox.c HealthBox_DrawCaughtIcon: the parts' caught ball in the bar's row at the box's x 8, a wild
            # foe's; its top row, which the box loses, a row up)
            ball = self.tile_img([r[:7] for r in st.part(59)[:7]], 15)
            self.img(sp, 'pt.caught', ball, fl(8), ys.index(33) - 1, cond=U.C_CAUGHT)
        # (HealthBox_DrawStatusIcon: the badge's three tiles just after the caught ball's in the box's VRAM,
        # sStatusIconVRAMTransfer: a foe's at x 16, the player's single box's at 32, a double's at 24; the ball's row)
        sx = {'foe': 16, 'mine': 32, 'mine2': 24}[k]
        sy = ys.index(32) if 32 in ys else ys.index(33) - 1
        self.status(sp, fl(sx), sy)
        if k == 'mine':
            hy = ys.index(41) - 1
            gx = fr(S['gauge'])
            self.text(sp, '{%d}' % U.F_HP3, gx - 8, hy, self.fdig, ink, sh)
            self.text(sp, '{%d}' % U.F_MAXHP, gx + 24, hy, self.fdig, ink, sh)
        return sp

    def card(self, sp, foe, w):
        st, p = self.st, self.p
        saved = st.name_put, st.special_put, st.put
        got = {}
        st.name_put = lambda rows, x, y, s, small=False: got.setdefault('name', (x, y)) and x
        orig_special = saved[1]

        def special(rows, x, y, k, w=8, shadow=2):
            if k in (11, 12):          # (the bar's "Lv": the box's own)
                orig_special(rows, x, y, k, w, shadow)
            elif 'lv' not in got:
                got['lv'] = (x, y, shadow)
        st.special_put = special
        st.put = lambda rows, x, y, tile, ground=None: None
        try:
            pic = st.card(foe, w, dict(name='', level=1, sex=None, hp=0.0 if foe else 0, maxhp=1))
        finally:
            st.name_put, st.special_put, st.put = saved
        self.img(sp, sp.name + '.card', pic)
        bar = 13
        rb = w - 3
        empty, fills = self.gauge_imgs(sp.name, (w, 20), 3, rb - 8 - 24, bar)
        sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
        ink = self.col(14)
        nx, ny = got['name']
        self.text(sp, '{%d}' % U.F_NAME, nx, ny, self.fn, ink, self.col(2), reg=1, unless=U.C_CAUGHT if foe else 0)
        if foe:
            # (a wild foe's caught ball, the box's own part, before the name, which moves on past it: in KAI's small
            # letters when it wouldn't leave the gender mark room)
            ball = self.tile_img([r[:7] for r in st.part(59)[:7]], 15)
            self.img(sp, 'pt.caught', ball, nx - 1, ny + 4, cond=U.C_CAUGHT)
            cx = nx - 1 + ball.width + 2
            sp.add(U.OP_LONG, a=U.F_NAME, font=self.fn, w=w - 3 - 8 - cx, cond=U.C_CAUGHT)
            self.text(sp, '{%d}' % U.F_NAME, cx, ny, self.fn, ink, self.col(2), reg=1, cond=U.C_CAUGHT,
                      unless=U.C_LONG)
            self.text(sp, '{%d}' % U.F_NAME, cx, ny + 1, self.fsmall, ink, self.col(2), reg=1,
                      cond=U.C_CAUGHT | U.C_LONG)
        for s, gc in (('m', U.C_MALE), ('f', U.C_FEMALE)):
            sp.add(U.OP_IMG, x=1, y=-1, a=p.image('pt.cardicon.' + s, self.icon_img(s, 6)), rel=True, reg=1, cond=gc)
        lx, ly, shadow = got['lv']
        self.text(sp, '{%d}' % U.F_LEVEL, lx, ly, self.fdig, ink, self.col(shadow), unless=U.C_STATUS)
        # (a status: the badge over the Lv mark and the level, as the box puts it before its bar)
        self.status(sp, lx - 16, ly)
        return sp

    # ---------------------------------------------------------------------------------------------------- panels
    def corners_img(self, w, h):
        im = Image.new('RGBA', (w + 4, h + 4), (0, 0, 0, 0))
        self.st.corners(im, 2, 2, w + 2, h + 2)
        return im

    def panels(self):
        st, p = self.st, self.p
        bb = st._batt_bg()
        ink, sh = st.BATTLE_INK
        # the battle's message: the message bar's window across the screen, FONT_MESSAGE
        sp = U.Spec('b.msg')
        cv = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        st.battle_msg(cv, (0, 112, 240, 160))
        bar = bb['bar']
        win = Image.new('RGBA', (240, 48))
        cols = list(range(16)) + [128] * 200 + list(range(232, 256))
        sp_, op_ = bar.load(), win.load()
        for y in range(48):
            for ox, sx in enumerate(cols):
                op_[ox, y] = sp_[sx, y]
        win.paste((248, 248, 248, 255), (16, 8, 216, 40))
        self.img(sp, 'pt.msgbar', win, 0, 112)
        sp.add(U.OP_MSG, x=16, y=120, a=0, h=16, n=2, w=200, e=self.fn + 1, font=self.fm, ink=ink, shadow=sh)
        p.spec(sp)
        # the command: the prompt's window, Platinum's four touch buttons, the red corners on the chosen
        sp = U.Spec('b.cmd')
        cv = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        saved = st.text
        st.text = lambda *a, **k: 0
        try:
            st.battle_msg(cv)
            st.battle_menu(cv)
        finally:
            st.text = saved
        # (the corners battle_menu drew on FIGHT come off: they're the cursor's, drawn where it is)
        clean = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        st.text = lambda *a, **k: 0
        st_corners = st.corners
        st.corners = lambda *a, **k: None
        try:
            st.battle_msg(clean)
            st.battle_menu(clean)
        finally:
            st.text, st.corners = saved, st_corners
        self.img(sp, 'pt.cmd', clean.crop((0, 112, 240, 160)), 0, 112)
        sp.add(U.OP_MSG, x=16, y=120, a=1, h=16, n=2, w=80, e=self.fn + 1, font=self.fm, ink=ink, shadow=sh)
        x0, y0, x1, y1 = Lo.BATTLE_MENU
        w = x1 - x0
        cols = {'FIGHT': ((248, 248, 248), (232, 208, 208), (88, 32, 32)),
                'BAG': ((248, 248, 248), (232, 224, 184), (96, 64, 0)),
                'RUN': ((248, 248, 248), (200, 200, 224), (16, 32, 88)),
                'MON': ((248, 248, 248), (208, 232, 208), (32, 72, 24))}
        widths, xs = [29, 29, 56], [x0 + 2, x0 + 32, x0 + 62]
        # (KAI's FIGHT 0, BAG 1, POKéMON 2, RUN 3: the buttons BAG, RUN, POKéMON left to right under FIGHT)
        c = cols['FIGHT']
        self.text(sp, '{%d}' % U.F_ITEM0, x0 + w // 2, y0 + 8, self.fsub, c[0], c[1], c[2], align=2)
        self.img(sp, 'pt.corners.fight', self.corners_img(w - 4, 23), x0, y0, cond=U.C_SEL0)
        for k, (field, word, dy, sel) in enumerate(((1, 'BAG', 0, 1), (3, 'RUN', 2, 3), (2, 'MON', 0, 2))):
            c = cols[word]
            self.text(sp, '{%d}' % (U.F_ITEM0 + field), xs[k] + widths[k] // 2, y0 + 29 + dy, self.fsub, c[0], c[1],
                      c[2], align=2)
            self.img(sp, 'pt.corners.%s' % word, self.corners_img(widths[k], 22 - dy), xs[k] - 2, y0 + 26 + dy - 2,
                     cond=U.C_SEL0 << sel)
        p.spec(sp)
        # the moves: four buttons in their types' colours, the chosen's corners; the type icon and PP beside them
        sp = U.Spec('b.moves')
        x0, y0, x1, y1 = Lo.MOVES
        self.img(sp, 'pt.back', bb['back'].crop((128 - 120, 192 - 48, 128 + 120, 192)), 0, y0)
        bw = (x1 - x0) // 2
        base = None
        for t, name in enumerate(TYPES):
            k = p.image('pt.move.%d' % t, self.move_button(name, bw, 22))
            base = k if base is None else base
            assert k == base + t
        spots = [(x0 + (i % 2) * bw, y0 + 2 + (i // 2) * 24) for i in range(4)]
        for i, (bx, by) in enumerate(spots):
            sp.add(U.OP_IMGSEL, x=bx, y=by, a=base, b=U.F_ITEM0 + 8 + i)
        for i, (bx, by) in enumerate(spots):
            self.img(sp, 'pt.corners.move', self.corners_img(bw, 22), bx - 2, by - 2, cond=U.C_SEL0 << i)
            # (the move held to swap with SELECT: its corners grey)
            self.img(sp, 'pt.corners.held', grey(self.corners_img(bw, 22)), bx - 2, by - 2, cond=U.C_HELD0 << i,
                     unless=U.C_SEL0 << i)
        for i, (bx, by) in enumerate(spots):
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), bx + bw // 2, by + 4, self.fn, (32, 32, 24), (144, 144, 128),
                      align=2)
        mx0, my0, mx1, my1 = Lo.MOVE_INFO
        info = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        st.std_frame(info, mx0 // 8 + 1, my0 // 8 + 1, (mx1 - mx0) // 8 - 2, (my1 - my0) // 8 - 2, 'standard_system')
        info.paste((255, 255, 255, 255), (mx0 + 8, my0 + 8, mx1 - 8, my1 - 8))
        self.img(sp, 'pt.info', info.crop((mx0, my0, mx1, my1)), mx0, my0)
        base = None
        for t, name in enumerate(TYPES):
            k = p.image('pt.type.%d' % t, self.type_icon(name))
            base = k if base is None else base
        sp.add(U.OP_IMGSEL, x=(mx0 + mx1) // 2 - 16, y=my0 + 8, a=base, b=U.F_TYPE, unless=U.C_ALT)
        fi, fs = st.FIELD_INK
        self.text(sp, 'PP', mx0 + 10, my0 + 21, self.fn, fi, fs, unless=U.C_ALT)
        self.text(sp, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), mx1 - 9, my0 + 21, self.fn, fi, fs, align=1, unless=U.C_ALT)
        sp.add(U.OP_MSG, x=mx0 + 10, y=my0 + 6, a=10, h=14, n=2, font=self.fn, ink=fi, shadow=fs, cond=U.C_ALT)
        p.spec(sp)
        # the field's message box: frame TYPE 1 round a 25 x 4 window from tile (2, 15), FONT_MESSAGE, the cursor
        sp = U.Spec('f.msg')
        cv = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        st.message_frame(cv, 2, 15, 25, 4)
        self.img(sp, 'pt.msgbox', cv.crop((0, 112, 240, 160)), 0, 112)
        sp.add(U.OP_MSG, x=16, y=120, a=0, h=16, n=2, w=200, e=self.fn + 1, font=self.fm, ink=fi, shadow=fs)
        cur = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
        st.scroll_cursor(cur, 0, 0)
        self.img(sp, 'pt.scroll', cur.crop((0, 0, 12, 16)), 28 * 8 + 1, 17 * 8, cond=U.C_WAIT)
        p.spec(sp)
        self.start_menus()

    def move_button(self, kind, w, h):
        if kind == 'FAIRY':
            # (Platinum has no FAIRY: PSYCHIC's button lightened toward Gen 6's pink)
            im = self.st.move_button('PSYCHIC', w, h)
            q = im.load()
            for y in range(h):
                for x in range(w):
                    r, g, b, a = q[x, y]
                    if a and not (r == g == b):
                        q[x, y] = ((r + 255) // 2, (g * 3 + 255) // 4, (b * 3 + 255) // 4, a)
            return im
        return self.st.move_button(kind, w, h)

    def type_icon(self, kind):
        name = kind
        path = os.path.join(R, 'res', 'graphics', 'battle', 'type_icons', name.lower() + '.png')
        if not os.path.exists(path):
            path = os.path.join(R, 'res', 'graphics', 'battle', 'type_icons', kind.lower() + '.png')
        if not os.path.exists(path):
            return self.fairy_icon()
        return self.closed(Image.open(path).convert('RGBA'))

    @staticmethod
    def closed(im):
        """a type icon's outline closed at its sides: the pill's colour 0 (black) round its top and bottom carried
        down its left and right columns"""
        for y in range(1, im.height - 1):
            for x in (0, im.width - 1):
                im.putpixel((x, y), (0, 0, 0, 255))
        return im

    def fairy_icon(self):
        """(Platinum has no FAIRY icon: its PSYCHIC icon's shape in FAIRY's pink with KAI's letters)"""
        im = Image.open(os.path.join(R, 'res', 'graphics', 'battle', 'type_icons', 'psychic.png')).convert('RGBA')
        q = im.load()
        letters = []
        for y in range(im.height):
            for x in range(im.width):
                r, g, b, a = q[x, y]
                if not a:
                    continue
                if r > 240 and g > 240 and b > 240:
                    letters.append((x, y))
                    q[x, y] = (232, 136, 200, 255)
                elif r == g == b:
                    continue
                else:
                    q[x, y] = (min(255, r + 40), max(0, g - 10), min(255, b + 60), a)
        # the word FAIRY in Platinum's small system letters over it
        txt = Image.new('RGBA', im.size, (0, 0, 0, 0))
        self.st.text(txt, im.width // 2, 0, 'FAIRY', (248, 248, 248), (120, 64, 104), align='c')
        bb = txt.getbbox()
        if bb:
            letters_h = bb[3] - bb[1]
            im.alpha_composite(txt.crop(bb), ((im.width - (bb[2] - bb[0])) // 2, (im.height - letters_h) // 2))
        return self.closed(im)

    def start_menus(self):
        """Platinum's START on the right: the standard frame round an 11-tile window, each entry's icon (coloured on
        the cursor's) beside its name, the orange cursor round the row; rows 20 apart (as the mock-up) up to seven,
        closer for more"""
        st, p = self.st, self.p
        tx, ty, tw, th = 18, 1, 11, 18
        x0, y0 = tx * 8, ty * 8
        frame = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        st.std_frame(frame, tx, ty, tw, th)
        frame_id = p.image('pt.start.frame', frame.crop((x0 - 8, 0, 240, 160)))
        grey = [p.image('pt.icon.g%d' % k, st.start_icon(k, False)) for k in range(10)]
        colour = [p.image('pt.icon.c%d' % k, st.start_icon(k, True)) for k in range(10)]
        assert grey == list(range(grey[0], grey[0] + 10)) and colour == list(range(colour[0], colour[0] + 10))
        cur = Image.open(os.path.join(R, 'res', 'graphics', 'start_menu', 'cursor.png'))
        cp, cpx = cur.getpalette(), cur.load()
        cw = Image.new('RGBA', (92, 26), (0, 0, 0, 0))
        q = cw.load()
        for y in range(26):
            for x in range(92):
                v = cpx[x + 2, y + 3]
                if v:
                    q[x, y] = tuple(cp[3 * v:3 * v + 3]) + (255,)
        fi, fs = st.FIELD_INK
        for n in range(1, 11):
            sp = U.Spec('start.%d' % n)
            sp.add(U.OP_IMG, x=x0 - 8, y=0, a=frame_id)
            step = 20 if n <= 7 else (th * 8 - 4) // n
            ch = min(22, step + 2)
            cur_id = p.image('pt.start.cursor%d' % ch, st.slice9(cw, 92, ch, 6, 4, 6, 4))
            for i in range(n):
                cy = y0 + 2 + step * i + step // 2
                sp.add(U.OP_IMGSEL, x=x0 + 14 - 16, y=cy - 16, a=grey[0], b=U.F_ICON0 + i, unless=U.C_SEL0 << i)
                sp.add(U.OP_IMGSEL, x=x0 + 14 - 16, y=cy - 16, a=colour[0], b=U.F_ICON0 + i, cond=U.C_SEL0 << i)
                self.text(sp, '{%d}' % (U.F_ITEM0 + i), x0 + 28, cy - 8, self.fm, fi, fs)
            for i in range(n):
                cy = y0 + 2 + step * i + step // 2
                sp.add(U.OP_IMG, x=x0 + 44 - 46, y=cy - ch // 2, a=cur_id, cond=U.C_SEL0 << i)
            p.spec(sp)
        p.spec(map_spec('start', START_ICONS))

    # ---------------------------------------------------------------------------------------------------- party
    def party(self):
        """pt.py party(): Platinum's party screen, its striped ground and six panels two by two (the lead's round,
        the chosen in its light blue, an empty slot grey), each with its ball, Kai's icon, the name and gender, Lv.
        and the HP in the special characters (or the status there, as Platinum shows it), the bar's fill; the
        message box and CANCEL under them"""
        st, p = self.st, self.p
        d = os.path.join(R, 'res', 'graphics', 'party_menu')
        pal = st.jasc(os.path.join(d, 'menu.pal'))
        ts = st.gray_tiles(os.path.join(d, 'menu_tiles.png'))
        _, _, bents = st.nscr(os.path.join(d, 'menu.NSCR'))
        _, _, pents = st.nscr(os.path.join(d, 'menu_panels.NSCR'))
        ground = Image.new('RGBA', (240, 160), pal[0] + (255,))
        ground.alpha_composite(st.map_rows(ts, bents, 32, 0, 0, 30, 20, pal))
        st.message_frame(ground, 2, 17, 20, 2)
        btn = Image.open(os.path.join(d, 'button.png'))
        bp, bpx = btn.getpalette(), btn.load()
        b = Image.new('RGBA', (56, 32), (0, 0, 0, 0))
        for y in range(32):
            for x in range(56):
                v = bpx[x, y]
                if v:
                    b.putpixel((x, y), tuple(bp[3 * v:3 * v + 3]) + (255,))
        b = b.crop(b.getbbox())
        ground.alpha_composite(b, (238 - b.width, 157 - b.height))
        sp = U.Spec('party')
        self.img(sp, 'pt.party.ground', ground)
        fi, fs = st.FIELD_INK
        sp.add(U.OP_MSG, x=16, y=136, a=0, h=16, n=1, w=152, e=self.fn + 1, font=self.fm, ink=fi, shadow=fs)
        # (CANCEL chosen: the D-pad's red corners round the button, as the battle's)
        bx0, by0 = 238 - b.width, 157 - b.height
        self.img(sp, 'pt.party.cancel', self.corners_img(b.width, b.height), bx0 - 2, by0 - 2, cond=U.C_SEL0)
        self.text(sp, 'CANCEL', 238 - b.width // 2, 157 - b.height // 2 - 8, self.fn, (248, 248, 248), (32, 56, 120),
                  align=2)
        p.spec(sp)
        white, dark = pal[15], pal[14]
        ball = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
        ball.alpha_composite(st.indexed(os.path.join(d, 'member_ball.png')).crop((0, 0, 32, 32)))
        ball_id = p.image('pt.party.ball', ball)
        ys = {y: i for i, y in enumerate(st.PANEL_ROWS)}
        X = lambda x: x if x < 40 else x - 8  # noqa: E731

        def panel(kind, prow):
            full = st.map_rows(ts, pents, 32, 0, 6 * kind, 16, 6, pal, prow)
            fp = full.load()
            out = Image.new('RGBA', (120, 40), (0, 0, 0, 0))
            q = out.load()
            for oy, sy in enumerate(st.PANEL_ROWS):
                for ox, sx in enumerate(st.PANEL_COLS):
                    q[ox, oy] = fp[sx, sy]
            return out
        fills = []
        for t, row in zip('gyr', (3, 4, 5)):
            im = Image.new('RGBA', (48, 4), (0, 0, 0, 0))
            lite, deep = pal[row * 16 + 9], pal[row * 16 + 10]
            for yy, c in enumerate((deep, lite, lite, deep)):
                im.paste(c + (255,), (0, yy, 48, yy + 1))
            fills.append(p.image('pt.partybar.' + t, im))
        for slot in range(6):
            sp = U.Spec('party.%d' % slot)
            px0, py0 = (slot % 2) * 120, 1 + (slot // 2) * 42
            kind = 0 if slot == 0 else 1
            self.img(sp, 'pt.panel.empty', panel(2, 1), px0, py0, cond=U.P_EMPTY)
            self.img(sp, 'pt.panel.%d' % kind, panel(kind, 3), px0, py0, unless=U.P_EMPTY | U.C_SEL0 | U.P_SWITCH)
            self.img(sp, 'pt.panel.%d.sel' % kind, panel(kind, 7), px0, py0, cond=U.C_SEL0, unless=U.P_EMPTY)
            self.img(sp, 'pt.panel.%d.sel' % kind, panel(kind, 7), px0, py0, cond=U.P_SWITCH,
                     unless=U.P_EMPTY | U.C_SEL0)
            off, no = U.P_EMPTY, U.P_EMPTY | U.P_EGG
            no_hp = no | U.P_DESC
            sp.add(U.OP_BAR, x=px0 + X(64), y=py0 + ys[26], a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2],
                   unless=no_hp)
            sp.add(U.OP_IMG, x=px0, y=py0 + ys[14] - 14, a=ball_id, unless=off)
            sp.add(U.OP_ICON, x=px0 + 26, y=py0 + ys[16] + 2, a=0, unless=off)
            ny = py0 + ys[11] - 3
            self.text(sp, '{%d}' % U.F_NAME, px0 + X(48), ny, self.fn, white, dark, unless=off)
            self.text(sp, '♂', px0 + X(112), ny, self.fn, pal[3], pal[4], cond=U.C_MALE, unless=no)
            self.text(sp, '♀', px0 + X(112), ny, self.fn, pal[5], pal[6], cond=U.C_FEMALE, unless=no)
            ly = py0 + ys[35] - 1
            self.text(sp, '{LV}{%d}' % U.F_LEVEL, px0 + 5, ly, self.fdig, white, dark, unless=no | U.C_STATUS)
            sp.add(U.OP_ICON, x=px0 + 5 + 16, y=ly + 4, a=2, cond=U.C_STATUS, unless=off)
            self.text(sp, '{%d}/{%d}' % (U.F_HP3, U.F_MAXHP), px0 + X(60), ly, self.fdig, white, dark, unless=no_hp)
            self.text(sp, '{%d}' % (U.F_TEXT0 + 1), px0 + X(60), ly - 2, self.fn, white, dark, cond=U.P_DESC,
                      unless=off)
            p.spec(sp)

    # ---------------------------------------------------------------------------------------------------- OPTIONS
    def options(self):
        """pt.py options_page(): Platinum's OPTIONS (the backdrop, the entries' system-frame window, a row each 16
        pixels, the choices from x 108, the chosen red; the red cursor round the cursor's row), KAI's tabs along the
        top (or a page's title), and in the message box KAI's buttons, or what the row does (in KAI's small font
        when Platinum's is too wide); the message box's scroll cursor at the window's edges for more rows"""
        from common import OPTION_TABS
        st, p = self.st, self.p
        d = os.path.join(R, 'res', 'graphics', 'options_menu')
        src = Image.open(os.path.join(d, 'tiles.png'))
        sp_ = src.getpalette()
        tpal = [tuple(sp_[3 * v:3 * v + 3]) for v in range(16)]
        ink, red = st.FIELD_INK, ((232, 32, 16), (248, 168, 184))
        tx, ty, tw, th = 1, 3, 28, 12
        ground = Image.new('RGBA', (240, 160), tpal[1] + (255,))
        st.std_frame(ground, tx, ty, tw, th, 'standard_system')
        ground.paste((255, 255, 255, 255), (tx * 8, ty * 8, (tx + tw) * 8, (ty + th) * 8))
        st.message_frame(ground, 2, 17, 25, 2)
        sp = U.Spec('o')
        self.img(sp, 'pt.o.ground', ground)
        widths = [st.measure(t) for t in OPTION_TABS]
        gap = (232 - sum(widths)) // (len(OPTION_TABS) - 1)
        x = 4
        for i, w in enumerate(widths):
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, 0, self.fn, *red, cond=U.C_SEL0 << i, unless=U.S_PAGE)
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, 0, self.fn, *ink, unless=U.S_PAGE | (U.C_SEL0 << i))
            x += w + gap
        self.text(sp, '{%d}' % U.F_TITLE, 4, 0, self.fn, *ink, cond=U.S_PAGE)
        self.text(sp, 'L R  Tab     A  Choose     B  Back', 16, 136, self.fm, *ink, unless=U.S_HINT)
        self.text(sp, '{%d}' % U.F_HINT, 16, 136, self.fm, *ink, cond=U.S_HINT, w=200, b=self.fsmall + 1)
        self.img(sp, 'pt.arrow.down', st.list_arrow(False), 112, 116, cond=U.S_DOWN)
        self.img(sp, 'pt.arrow.up', st.list_arrow(True), 112, 12, cond=U.S_UP)
        sp.add(U.OP_LIST, x=0, y=ty * 8, h=16, n=th // 2)
        p.spec(sp)
        # a row: the cursor's red outline (tilemap.bin's rows: tile 3 the corners, 4 the edge), the name, the choices
        px = src.load()

        def tile(k, hf=False, vf=False):
            t = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
            for y in range(8):
                for xx in range(8):
                    v = px[k * 8 + xx, y]
                    if v:
                        t.putpixel((7 - xx if hf else xx, 7 - y if vf else y), tpal[v] + (255,))
            return t
        cur = Image.new('RGBA', (tw * 8, 16), (0, 0, 0, 0))
        for c in range(tw):
            k = 3 if c in (0, tw - 1) else 4
            hf = c == tw - 1
            cur.alpha_composite(tile(k, hf, False), (c * 8, 0))
            cur.alpha_composite(tile(k, hf, True), (c * 8, 8))
        row = U.Spec('o.row')
        self.img(row, 'pt.o.cursor', cur, tx * 8, 0, cond=U.C_SEL0)
        self.text(row, '{%d}' % U.F_NAME, tx * 8 + 6, 0, self.fn, *ink, w=92, b=self.fsmall + 1)
        self.text(row, '{%d}' % U.F_CURSTR, tx * 8 + 100, 0, self.fn, *red, cond=U.O_VALUE, unless=U.O_ALL, w=120,
                  b=self.fsmall + 1)
        row.add(U.OP_CHOICES, x=tx * 8 + 100, y=0, w=120, n=10, a=U.F_CHOICE0, font=self.fn, ink=red[0],
                shadow=red[1], c=U.col(ink[0]), d=U.col(ink[1]), cond=U.O_ALL)
        p.spec(row)

    # ---------------------------------------------------------------------------------------------------- BAG
    def bag(self):
        """pt.py bag_page(): Platinum's BAG ground, KAI's eight pockets on the plate's bar (the open one red), the
        pocket's name, four rows (the highlight on the cursor's, the count after its x, a registered item's button
        in the count's place), START SORT, the scroll cursor when more rows follow (turned over for rows above), the
        item's icon in the band's white box and its words beside it (closer when they take a fourth line)"""
        st, p = self.st, self.p
        ground, Ly = st.bag_ground()
        dark, lite = (16, 24, 32), (172, 189, 189)
        sp = U.Spec('bag')
        self.img(sp, 'pt.bag.ground', ground)
        for sex in (0, 1):
            for i, (bag, bx, by) in enumerate(st.bag_pictures(Ly, sex)):
                g = dict(cond=U.C_SEL0 << i | U.C_FEMALE) if sex else dict(cond=U.C_SEL0 << i, unless=U.C_FEMALE)
                self.img(sp, 'pt.bag.pic%d.%d' % (sex, i), bag, bx, by, **g)
        for i, (grey, red) in enumerate(st.pocket_icons()):
            self.img(sp, 'pt.bag.pocket%d' % i, grey, 4 + 12 * i, Ly[90] - 2, unless=U.C_SEL0 << i)
            self.img(sp, 'pt.bag.pocket%d.on' % i, red, 4 + 12 * i, Ly[90] - 2, cond=U.C_SEL0 << i)
        self.text(sp, '{%d}' % U.F_TITLE, 52, Ly[106], self.fn, dark, lite, align=2)
        # START SORT on the sky under the BAG label, as KAI keeps it beside its bag (the list keeps KAI's five rows)
        self.text(sp, 'START', 22, 20, self.fn, dark, lite, align=2)
        self.text(sp, 'SORT', 22, 34, self.fn, dark, lite, align=2)
        # the scroll cursor on the list panel's foot / head, under / over the middle of the rows
        self.img(sp, 'pt.arrow.down', st.list_arrow(False), 158, 13 + 16 * 5 - 3, cond=U.S_DOWN)
        self.img(sp, 'pt.arrow.up', st.list_arrow(True), 158, -4, cond=U.S_UP)
        sp.add(U.OP_ICON, x=22, y=138, a=1)
        sp.add(U.OP_MSG, x=40, y=112, a=0, h=16, n=3, w=196, b=12, c=4, d=110, font=self.fn, ink=(255, 255, 255),
               shadow=(0, 0, 0))
        sp.add(U.OP_LIST, x=0, y=13, h=16, n=5)
        p.spec(sp)
        row = U.Spec('bag.row')
        self.img(row, 'pt.bag.highlight', st.item_highlight(), 110, 0, cond=U.C_SEL0)
        self.img(row, 'pt.bag.highlight', st.item_highlight(), 110, 0, cond=U.B_SWAP, unless=U.C_SEL0)
        self.text(row, '{%d}' % U.F_NAME, 116, 0, self.fn, dark, lite, w=78, b=self.fsmall + 1)
        self.text(row, 'x', 196, 0, self.fn, dark, lite, cond=U.B_QTY)
        self.text(row, '{%d}' % U.F_QTY, 226, 0, self.fn, dark, lite, align=1, cond=U.B_QTY)
        self.text(row, '{%d}' % U.F_TAG, 226, 0, self.fsmall, dark, lite, align=1, cond=U.B_TAG)
        p.spec(row)

    # ---------------------------------------------------------------------------------------------------- pop-ups
    def popups(self):
        """KAI's own pop-ups (menus, YES / NO, the save's box) in Platinum's field window frame and message font
        (mp_uistyle.c PopUps): the red ones red, the pale ones grey"""
        st, p = self.st, self.p
        frame = Image.new('RGBA', (24, 24), (0, 0, 0, 0))
        st.std_frame(frame, 1, 1, 1, 1)
        sp = U.Spec('win')
        self.img(sp, 'pt.win', frame)
        sp.add(U.OP_FILL, w=0, h=0, ink=(255, 255, 255), a=255)
        p.spec(sp)
        fi, fs = st.FIELD_INK
        t = U.Spec('win.text')
        self.text(t, '{%d}' % U.F_NAME, 0, -1, self.fm, fi, fs, unless=U.C_SEL0 | U.C_ALT)
        self.text(t, '{%d}' % U.F_NAME, 0, -1, self.fm, (232, 32, 16), (248, 168, 184), cond=U.C_SEL0)
        self.text(t, '{%d}' % U.F_NAME, 0, -1, self.fm, (168, 168, 176), (216, 216, 216), cond=U.C_ALT)
        p.spec(t)

    # ---------------------------------------------------------------------------------------------------- summary
    def summary(self):
        """KAI's six summary pages in Platinum's (pt.py summary_ground): INFO as approved; SKILLS on page_skills (HP
        and its bar, the five stats, nature's red / blue, IV stars or EVs, EXP. POINTS, NEXT LV., the Ability and its
        words); the moves on page_battle_moves / page_contest_moves (type or contest icon, name, PP), the picked one in
        Platinum's move_info panel (POWER, ACCURACY or APPEAL / JAM hearts, its words), the frame on its row; the
        POKé BALL and the JOURNAL on page_memo's stripes; the page's title and what A does in the header"""
        st, p = self.st, self.p
        W_, BK, BL, RD = st.SUM_INK['white'], st.SUM_INK['black'], st.SUM_INK['blue'], st.SUM_INK['red']
        tfirst = None
        for i, t in enumerate(U.TYPE_NAMES):
            ic = self.fairy_icon() if t == 'FAIRY' else st.type_icon(t if t != '???' else 'NORMAL')
            k = p.image('pt.type%d' % i, ic)
            tfirst = k if tfirst is None else tfirst
            assert k == tfirst + i
        cfirst = None
        # (Platinum's own contest icons, beside its type icons)
        for i, word in enumerate(('COOL', 'BEAUTY', 'CUTE', 'SMART', 'TOUGH')):
            k = p.image('pt.contest%d' % i, st.type_icon(word))
            cfirst = k if cfirst is None else cfirst
            assert k == cfirst + i
        labels = ['', 'A  IV RATING', 'A  EVs', 'A  STATS', 'A  INFO', 'A  SWITCH', 'A  MORE', '']
        pfirst = None
        for i, lab in enumerate(labels):
            im = Image.new('RGBA', (240, 16), (0, 0, 0, 0))
            if lab:
                st.text(im, 236, 0, lab, *W_, align='r')
            k = p.image('pt.sum.prompt%d' % i, im)
            pfirst = k if pfirst is None else pfirst
            assert k == pfirst + i
        star = (0x10, 0x10, 0x38, 0xFE, 0x7C, 0x38, 0x6C, 0x44)
        heart = (0x00, 0xD8, 0xF8, 0xF8, 0x70, 0x20, 0x00, 0x00)

        def shapes(tag, shape, counts, total, on, off, perfect=None, pitch=8):
            first = None
            for i, n in enumerate(counts):
                im = Image.new('RGBA', (pitch * total, 8), (0, 0, 0, 0))
                for j in range(total):
                    col = (perfect if perfect and n > total else on) if j < min(n, total) else off
                    for yy in range(8):
                        for xx in range(8):
                            if shape[yy] & (0x80 >> xx):
                                im.putpixel((j * pitch + xx, yy), col + (255,))
                k = p.image('pt.%s%d' % (tag, i), im)
                first = k if first is None else first
                assert k == first + i
            return first
        stars = shapes('stars', star, [0, 1, 2, 3, 4], 3, BK[0], (190, 196, 196), perfect=RD[0])
        hearts = shapes('hearts', heart, range(9), 8, RD[0], (190, 196, 196), pitch=6)
        # HP and EXP bars: a plain red / yellow / green fill over Platinum's bar art
        fills = []
        for t, c in zip('gyr', ((88, 208, 128), (248, 224, 56), (248, 88, 56))):
            im = Image.new('RGBA', (48, 3), c + (255,))
            fills.append(p.image('pt.sum.hp.%s' % t, im))
        expfill = p.image('pt.sum.exp', Image.new('RGBA', (64, 2), (64, 200, 248, 255)))

        def common(sp, Ly, k):
            self.text(sp, '{%d}' % U.F_PAGETITLE, 8, 0, self.fn, *W_)
            if k == 0:
                self.text(sp, 'SELECT RENAME', 236, 0, self.fn, *W_, align=1, cond=U.SM_RENAME)
            else:
                sp.add(U.OP_IMGSEL, x=0, y=0, a=pfirst, b=U.F_PROMPT)

        def left(sp, Ly, ability=False):
            ny = Ly[24]
            self.text(sp, '{%d}' % U.F_NAME, 24, ny, self.fn, *W_)
            self.text(sp, '♂', 24 + 64, ny, self.fn, *BL, cond=U.C_MALE)
            self.text(sp, '♀', 24 + 64, ny, self.fn, *RD, cond=U.C_FEMALE)
            self.text(sp, '{LV}{%d}' % U.F_LEVEL, 8, Ly[42], self.fdig, *BK, unless=U.SM_EGG)
            sp.add(U.OP_ICON, x=52, y=Ly[102], a=0)
            sp.add(U.OP_ICON, x=90, y=Ly[42] + 6, a=2)
            if ability:
                self.text(sp, 'Ability', 8, Ly[160], self.fn, *W_)
                self.text(sp, '{%d}' % U.F_ABILITY, 8, Ly[176], self.fn, *BK, w=92, b=self.fsmall + 1)
                return
            self.text(sp, 'Item', 8, Ly[160], self.fn, *W_)
            self.text(sp, '{%d}' % U.F_HELD, 8, Ly[176], self.fn, *BK, w=92, b=self.fsmall + 1)

        # INFO (as approved)
        g, Ly, Y, X = st.summary_ground('page_info', 0)
        sp = U.Spec('sum.0')
        self.img(sp, 'pt.sum0', g)
        common(sp, Ly, 0)
        left(sp, Ly)
        top = 160 - 90 - 34
        rows = [('Moédex No.', U.F_DEXNO, 192, 48), ('Name', U.F_SPECIES, 184, 64), ('Type', None, 0, 0),
                ('OT', U.F_OT, 184, 64), ('ID No.', U.F_IDNO, 200, 32)]
        for i, (label, fld, wx, ww) in enumerate(rows):
            y = Y(40 + 16 * i)
            self.text(sp, label.replace('Moédex', 'Moédex'), X(112), y, self.fn, *W_)
            if fld is not None:
                cx = X(wx) + ww // 2 if X(wx) else X(184) + 32
                self.text(sp, '{%d}' % fld, cx, y, self.fn, *(BL if label == 'OT' else BK), align=2)
        # (one type in the middle of the value's field; two side by side in it, both on the screen)
        sp.add(U.OP_IMGSEL, x=X(200), y=Y(72), a=tfirst, b=U.F_TYPE1, unless=U.SM_EGG | U.SM_TWOTYPES)
        sp.add(U.OP_IMGSEL, x=160, y=Y(72), a=tfirst, b=U.F_TYPE1, cond=U.SM_TWOTYPES, unless=U.SM_EGG)
        sp.add(U.OP_IMGSEL, x=160 + 34, y=Y(72), a=tfirst, b=U.F_TYPE2, cond=U.SM_TWOTYPES, unless=U.SM_EGG)
        my = top + 90 + 1
        self.text(sp, '{%d} nature.' % U.F_NATURE, X(112), my, self.fn, *BK, cond=U.SM_MEMOPLAIN)
        self.text(sp, 'Lv{%d} {%d}' % (U.F_METLV, U.F_METPLACE), X(112) - 2, my + 16, self.fn, *BK,
                  cond=U.SM_MEMOPLAIN, w=130, b=self.fsmall + 1)
        sp.add(U.OP_MSG, x=X(112), y=my, a=2, h=16, n=2, w=130, e=self.fsmall + 1, font=self.fn, ink=BK[0],
               shadow=BK[1], unless=U.SM_MEMOPLAIN)
        p.spec(sp)
        # SKILLS
        # (its rows chosen: the HP box and bar whole, each stat's stripe two rows shorter, EXP's box, the foot)
        pick = list(range(28, 56))
        for i in range(5):
            pick += [y for y in range(56 + 16 * i, 72 + 16 * i) if y - (56 + 16 * i) not in (6, 7, 8, 9)]
        pick += list(range(136, 160))
        pick += list(range(160, 192))[:124 - len(pick)]
        g, Ly, Y, X = st.summary_ground('page_skills', 1, pick=pick)
        sp = U.Spec('sum.1')
        self.img(sp, 'pt.sum1', g)
        common(sp, Ly, 1)
        left(sp, Ly, ability=True)
        self.text(sp, 'HP', X(112), Y(32), self.fn, *W_)
        self.text(sp, '{%d}/{%d}' % (U.F_HP, U.F_MAXHP), X(248), Y(32), self.fn, *BK, align=1,
                  unless=U.SM_IV | U.SM_EV)
        sp.add(U.OP_IMGSEL, x=X(224), y=Y(36), a=stars, b=U.F_IV0, cond=U.SM_IV)
        self.text(sp, '{%d}' % U.F_EV0, X(248), Y(32), self.fn, *BK, align=1, cond=U.SM_EV)
        sp.add(U.OP_BAR, x=X(194), y=Y(51), a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
        names = ['Attack', 'Defense', 'Sp. Atk', 'Sp. Def', 'Speed']
        for i, nm in enumerate(names):
            y = Y(56 + 16 * i)
            self.text(sp, nm, X(112), y, self.fn, *W_)
            self.text(sp, '{%d}' % (U.F_STAT0 + i), X(240), y, self.fn, *RD, align=1, cond=U.C_SEL0 << i,
                      unless=U.SM_IV | U.SM_EV)
            self.text(sp, '{%d}' % (U.F_STAT0 + i), X(240), y, self.fn, *BL, align=1, cond=U.C_SEL0 << (5 + i),
                      unless=U.SM_IV | U.SM_EV)
            self.text(sp, '{%d}' % (U.F_STAT0 + i), X(240), y, self.fn, *BK, align=1,
                      unless=U.SM_IV | U.SM_EV | (U.C_SEL0 << i) | (U.C_SEL0 << (5 + i)))
            sp.add(U.OP_IMGSEL, x=X(216), y=y + 4, a=stars, b=U.F_IV0 + 1 + i, cond=U.SM_IV)
            self.text(sp, '{%d}' % (U.F_EV0 + 1 + i), X(240), y, self.fn, *BK, align=1, cond=U.SM_EV)
        self.text(sp, 'Exp. Points', X(112), Y(136), self.fn, *W_)
        self.text(sp, '{%d}' % U.F_EXPPTS, X(248), Y(136), self.fn, *W_, align=1)
        self.text(sp, 'Next Lv.', X(160), Y(148), self.fn, *BK)
        self.text(sp, '{%d}' % U.F_NEXTLV, X(248), Y(148), self.fn, *BK, align=1)
        self.text(sp, 'START  CLEAR', X(248), Y(160), self.fn, *BL, align=1, cond=U.SM_CLEAREV)
        p.spec(sp)
        # the moves
        for k, contest in ((2, False), (3, True)):
            nscr = 'page_contest_moves' if contest else 'page_battle_moves'
            sp = U.Spec('sum.%d' % k)
            # (each move's 32 rows, its light stripe (icon and name) whole and ten of the dark one's eighteen (PP),
            # five moves under three rows of the panel's top)
            pick = [29, 30, 31]
            for i in range(5):
                pick += list(range(50 + 32 * i, 64 + 32 * i)) + list(range(64 + 32 * min(i, 3), 74 + 32 * min(i, 3)))
            g, Ly, Y, X = st.summary_ground(nscr, k, pick=pick)
            self.img(sp, 'pt.sum%d' % k, g, unless=U.SM_CHOOSING)
            g2, _, Y2, X2 = st.summary_ground(nscr + '_select_mode', k, choosing=True, pick=pick)
            if contest:
                # (the contest page's own cream bubble on the stripes, where Platinum's condition chart sits: the
                # stripes carried over it, so the hearts sit on clean bands)
                q = g2.load()
                for yy in range(16, 52):
                    for xx in range(50, 102):
                        q[xx, yy] = q[30, yy]
            self.img(sp, 'pt.sum%dc' % k, g2, cond=U.SM_CHOOSING)
            common(sp, Ly, k)
            # (the normal page's left column; picked, the move's panel)
            ny = Ly[24]
            self.text(sp, '{%d}' % U.F_NAME, 24, ny, self.fn, *W_, unless=U.SM_CHOOSING)
            self.text(sp, '♂', 88, ny, self.fn, *BL, cond=U.C_MALE, unless=U.SM_CHOOSING)
            self.text(sp, '♀', 88, ny, self.fn, *RD, cond=U.C_FEMALE, unless=U.SM_CHOOSING)
            self.text(sp, '{LV}{%d}' % U.F_LEVEL, 8, Ly[42], self.fdig, *BK, unless=U.SM_CHOOSING | U.SM_EGG)
            sp.add(U.OP_ICON, x=52, y=Ly[102], a=0, unless=U.SM_CHOOSING)
            self.text(sp, 'Item', 8, Ly[160], self.fn, *W_, unless=U.SM_CHOOSING)
            self.text(sp, '{%d}' % U.F_HELD, 8, Ly[176], self.fn, *BK, w=92, b=self.fsmall + 1, unless=U.SM_CHOOSING)
            if contest:
                self.text(sp, 'Appeal', 4, 18, self.fn, *W_, cond=U.SM_CHOOSING)
                sp.add(U.OP_IMGSEL, x=46, y=23, a=hearts, b=U.F_APPEAL, cond=U.SM_CHOOSING)
                self.text(sp, 'Jam', 4, 34, self.fn, *W_, cond=U.SM_CHOOSING)
                sp.add(U.OP_IMGSEL, x=46, y=39, a=hearts, b=U.F_JAM, cond=U.SM_CHOOSING)
            else:
                self.text(sp, 'Power', 4, 18, self.fn, *W_, cond=U.SM_CHOOSING)
                self.text(sp, '{%d}' % U.F_POWER, 98, 18, self.fn, *W_, align=1, cond=U.SM_CHOOSING)
                self.text(sp, 'Accuracy', 4, 34, self.fn, *W_, cond=U.SM_CHOOSING)
                self.text(sp, '{%d}' % U.F_ACC, 93, 34, self.fn, *BK, align=1, cond=U.SM_CHOOSING)
            sp.add(U.OP_MSG, x=4, y=50, a=1, h=14, n=7, w=96, e=self.fsmall + 1, font=self.fn, ink=BK[0],
                   shadow=BK[1], cond=U.SM_CHOOSING)
            y0 = Y2(50)
            sp.add(U.OP_LIST, x=0, y=y0, h=24, n=5)
            p.spec(sp)
            row = U.Spec('sum.%d.row' % k)
            frame = Image.new('RGBA', (X(251) - X(184) + 4, 18), (0, 0, 0, 0))
            frame.paste((232, 32, 16, 255), (0, 0, frame.width, 2))
            frame.paste((232, 32, 16, 255), (0, 16, frame.width, 18))
            frame.paste((232, 32, 16, 255), (0, 0, 2, 18))
            frame.paste((232, 32, 16, 255), (frame.width - 2, 0, frame.width, 18))
            blue = frame.copy()
            px = blue.load()
            for yy in range(16):
                for xx in range(blue.width):
                    if px[xx, yy][3]:
                        px[xx, yy] = (0, 115, 255, 255)
            self.img(row, 'pt.sum.frame', frame, X(184) - 2, -2, cond=U.C_SEL0)
            self.img(row, 'pt.sum.frame2', blue, X(184) - 2, -2, cond=U.B_SWAP, unless=U.C_SEL0)
            if contest:
                row.add(U.OP_IMGSEL, x=X(136), y=-1, a=cfirst, b=U.F_CTYPE, unless=U.R_EMPTY | U.R_CANCEL)
            else:
                row.add(U.OP_IMGSEL, x=X(136), y=-1, a=tfirst, b=U.F_TYPE, unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '{%d}' % U.F_NAME, X(188), -2, self.fn, *BK, w=X(250) - X(188), b=self.fsmall + 1,
                      unless=U.R_EMPTY | U.R_CANCEL)
            # (PP in the special characters' digits: the dark stripe's ten rows hold their eight)
            self.text(row, 'PP', X(188), 15, self.fdig, *W_, unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), X(249), 15, self.fdig, *W_, align=1,
                      unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '-', X(188), -2, self.fn, *BK, cond=U.R_EMPTY)
            self.text(row, 'CANCEL', X(188), -2, self.fn, *BK, cond=U.R_CANCEL)
            p.spec(row)
        # the POKé BALL and the JOURNAL on page_memo's stripes
        for k in (4, 5):
            g, Ly, Y, X = st.summary_ground('page_memo', k)
            sp = U.Spec('sum.%d' % k)
            self.img(sp, 'pt.sum%d' % k, g)
            common(sp, Ly, k)
            left(sp, Ly)
            if k == 4:
                sp.add(U.OP_LIST, x=104, y=36, h=14, n=1)
            else:
                sp.add(U.OP_MSG, x=110, y=38, a=3, h=16, n=7, w=126, e=self.fsmall + 1, font=self.fn, ink=BK[0],
                       shadow=BK[1])
                self.text(sp, '{%d}' % U.F_JPAGE, 236, 144, self.fn, *BK, align=1, cond=U.S_DOWN)
            p.spec(sp)
            row = U.Spec('sum.%d.row' % k)
            self.text(row, '{%d}' % U.F_NAME, 0, 0, self.fn, *BK, w=132, b=self.fsmall + 1)
            p.spec(row)

    def make(self):
        self.fonts()
        for kind in ('foe1', 'mine1', 'foe2', 'mine2', 'foe3', 'mine3'):
            self.p.spec(self.hud(kind))
        self.panels()
        self.party()
        self.options()
        self.bag()
        self.popups()
        self.summary()
        return self.p
