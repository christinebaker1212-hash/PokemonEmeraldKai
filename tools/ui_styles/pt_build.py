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
        self.text(sp, '{%d}' % U.F_NAME, nx, ny, self.fn, ink, self.col(2), reg=1)
        for s, gc in (('m', U.C_MALE), ('f', U.C_FEMALE)):
            sp.add(U.OP_IMG, x=1, y=-1, a=p.image('pt.cardicon.' + s, self.icon_img(s, 6)), rel=True, reg=1, cond=gc)
        lx, ly, shadow = got['lv']
        self.text(sp, '{%d}' % U.F_LEVEL, lx, ly, self.fdig, ink, self.col(shadow))
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
        return Image.open(path).convert('RGBA')

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
        return im

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

    def make(self):
        self.fonts()
        for kind in ('foe1', 'mine1', 'foe2', 'mine2', 'foe3', 'mine3'):
            self.p.spec(self.hud(kind))
        self.panels()
        return self.p
