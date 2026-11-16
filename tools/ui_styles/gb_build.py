"""RED and CRYSTAL as the game draws them (uipack.py): gbstyle.py's pieces made into a pack's fonts, pictures and
specs. Every spec puts things where gbstyle.py's mock-ups put them (approved 2026-11-16); check_specs.py draws both
and compares them."""
from PIL import Image

import layout as Lo
import uipack as U
from gbstyle import GB

MALE_INK, FEMALE_INK = (56, 112, 232), (232, 64, 104)
GREEN, YELLOW, RED = (48, 184, 72), (232, 184, 0), (232, 64, 48)
EXP_BLUE = (32, 136, 248)

# Crystal's MENU ACCOUNT words for each START entry, by start_menu.c's MENU_ACTION_* (its own where it has one: POKéDEX,
# PARTY, PACK, POKéGEAR, status, SAVE, OPTION, EXIT; KAI's extras in the same voice, ten letters a line at most)
ACCOUNT = [('MOéMON', 'database'), ('Party', 'status'), ('Contains', 'items'), ('Trainer\'s', 'key device'),
           ('Your own', 'status'), ('Save your', 'progress'), ('Change', 'settings'), ('Close this', 'menu'),
           ('Quit and', 'go home'), ('Your own', 'status'), ('Save and', 'rest'), ('Quit and', 'go home'),
           ('Contains', 'items'), ('Play with', 'friends'), ('Visit the', 'GARDEN'), ('Trainer\'s', 'key device'),
           ('Trainer\'s', 'key device')]


class GBPack:
    def __init__(self, kind, pret):
        self.st = GB(kind, pret, None, None)
        self.kind, self.red = kind, kind == 'red'
        self.p = U.Pack(kind.upper())

    # ---------------------------------------------------------------------------------------------------- fonts
    def fonts(self):
        st = self

        def draw(pitch):
            def d(im, x, y, s, ink, sh, out):
                g = st.st
                mid = g.mid
                g.mid = sh
                try:
                    g.tile(im, g.glyph(s), x, y, ink)
                finally:
                    g.mid = mid
            return d
        self.font = self.p.font('gb', U.font_from(draw(8), lambda s: 8, 8))
        f7 = U.font_from(draw(7), lambda s: 7, 8)
        self.font7 = self.p.font('gb7', f7)

    # ---------------------------------------------------------------------------------------------------- pieces
    def tile_img(self, k):
        c = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
        self.st.put(c, k, 0, 0)
        return c

    def gb_bar(self, c, col, row, frac, end, label, tiles, fill):
        """gbstyle.hp_tiles with its fill's colour given"""
        st, red = self.st, self.red
        if label == 'edge':
            GB.tile(st, c, st.bt(0x6C).transpose(Image.FLIP_LEFT_RIGHT), col * 8, row * 8)
            col += 1
        elif label:
            st.put(c, 0x71 if red else 0x60, col, row)
            st.put(c, 0x62 if red else 0x61, col + 1, row)
            col += 2
        n = round(frac * 8 * tiles)
        for k in range(tiles):
            part = max(0, min(8, n - 8 * k))
            st.put(c, (0x63 if red else 0x62) + part, col + k, row, fill)
        st.put(c, end, col + tiles, row)

    def bar_specs(self, name, size, draw_bar):
        """the bar's frame (empty) and its fill pictures (each colour's full bar less the empty one)"""
        empty = Image.new('RGBA', size, (0, 0, 0, 0))
        draw_bar(empty, 0.0, GREEN)
        fills = []
        for tier, col in (('g', GREEN), ('y', YELLOW), ('r', RED)):
            full = Image.new('RGBA', size, (0, 0, 0, 0))
            draw_bar(full, 1.0, col)
            fills.append(self.p.image('%s.fill.%s' % (name, tier), diff(full, empty)))
        return empty, fills

    @staticmethod
    def kept_rows(c, h):
        """GB.fit_rows' choice of rows, as indices (the rows a full box keeps)"""
        rows = [[c.getpixel((x, y)) for x in range(c.width)] for y in range(c.height)]
        idx = list(range(len(rows)))
        blank = lambda r: all(p[3] == 0 for p in r)  # noqa: E731
        while len(rows) > h and blank(rows[-1]):
            rows.pop()
            idx.pop()
        while len(rows) > h:
            cand = [y for y in range(1, len(rows) - 1)
                    if blank(rows[y]) or rows[y] == rows[y - 1] or rows[y] == rows[y + 1]]
            runs, run = [], []
            for y in cand:
                if run and y == run[-1] + 1:
                    run.append(y)
                else:
                    if run:
                        runs.append(run)
                    run = [y]
            if run:
                runs.append(run)
            runs = [r for r in runs if len(r) >= 2]
            best = max(runs, key=len)
            k = best[len(best) // 2]
            rows.pop(k)
            idx.pop(k)
        return idx

    # ---------------------------------------------------------------------------------------------------- HUDs
    def text(self, sp, s, x, y, ink=None, cond=0, unless=0, rel=False, reg=0, align=0, w=0, alt=0):
        sp.add(U.OP_TEXT, x=x, y=y, a=self.p.string(s), b=alt, font=self.font, ink=ink or self.st.ink,
               shadow=self.st.mid, cond=cond, unless=unless, rel=rel, reg=reg, align=align, w=w)

    def level(self, sp, x, y, reg=0):
        """PrintLevel: :L and the digits; three digits over the :L"""
        lv = self.p.image('lv', self.tile_img(0x6E))
        sp.add(U.OP_IMG, x=x, y=y, a=lv, unless=U.C_LV3)
        self.text(sp, '{%d}' % U.F_LEVEL, x + 8, y, unless=U.C_LV3, reg=reg)
        self.text(sp, '{%d}' % U.F_LEVEL, x, y, cond=U.C_LV3, reg=reg)

    def gender(self, sp, x, y, rel=False, reg=0):
        if self.red:
            return
        self.text(sp, '♂', x, y, MALE_INK, cond=U.C_MALE, rel=rel, reg=reg)
        self.text(sp, '♀', x, y, FEMALE_INK, cond=U.C_FEMALE, rel=rel, reg=reg)

    def hud(self, kind):
        st, red, p = self.st, self.red, self.p
        rect = {'foe1': Lo.SINGLE_FOE, 'mine1': Lo.SINGLE_MINE, 'foe2': Lo.DOUBLE_FOES[0],
                'mine2': Lo.DOUBLE_MINE[0], 'foe3': Lo.TRIPLE_FOES[0], 'mine3': Lo.TRIPLE_MINE[0]}[kind]
        rw, rh = rect[2] - rect[0], rect[3] - rect[1]
        foe = kind.startswith('foe')
        if kind == 'foe1':
            cw, ch = 80, 32
            frame = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
            st.put(frame, 0x73 if red else 0x6D, 0, 2)
            st.put(frame, 0x74, 0, 3)
            for k in range(1, 9):
                st.put(frame, 0x76, k, 3)
            st.put(frame, 0x78, 9, 3)
            bar = (lambda c, f, col: self.gb_bar(c, 1, 2, f, 0x6C if red else 0x6B, True, 6, col))
            ops = [('name', 0, 0), ('level', 24 if red else 40, 8), ('gender', 64, 8, False)]
        elif kind == 'mine1':
            cw, ch = 88, 40
            frame = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
            st.put(frame, 0x73, 9, 3)
            st.put(frame, 0x6F, 0, 4)
            if red:
                for k in range(1, 9):
                    st.put(frame, 0x76, k, 4)
            st.put(frame, 0x77, 9, 4)
            bar = (lambda c, f, col: self.gb_bar(c, 1, 2, f, 0x6D if red else 0x6B, True, 6, col))
            ops = [('name', 8, 0), ('level', 40, 8), ('gender', 64, 8, False), ('hpnum', 16, 24)]
        elif kind in ('foe2', 'mine2'):
            cw, ch = 96, 24
            frame = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
            nc = 0 if foe else 1
            if foe:
                st.put(frame, 0x73 if red else 0x6D, 0, 1)
            st.put(frame, 0x74 if foe else 0x6F, 0, 2)
            for k in range(1, 11):
                st.put(frame, 0x76, k, 2)
            st.put(frame, 0x78 if foe else 0x77, 11, 2)
            if red:
                bar = (lambda c, f, col: self.gb_bar(c, 4, 1, f, 0x6C if foe else 0x6D, 'edge', 6, col))
            else:
                bar = (lambda c, f, col: self.gb_bar(c, 4, 1, f, 0x6B, True, 5, col))
            ops = [('name', nc * 8, 0), ('gender', 0, 0, True), ('level', 8, 8)]
        else:
            return self.card(kind, rect)
        sp = U.Spec('hud.' + kind, rw, rh, (cw, ch))
        empty, fills = self.bar_specs('hud.' + kind, (cw, ch), bar)
        frame.alpha_composite(empty)
        exp = None
        if kind == 'mine1' and not red:
            exp_empty = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
            exp_full = Image.new('RGBA', (cw, ch), (0, 0, 0, 0))
            for k in range(1, 9):
                st.put(exp_empty, 0x62, k, 4)
                st.put(exp_full, 0x6A, k, 4, EXP_BLUE)
            frame.alpha_composite(exp_empty)
            exp = p.image('hud.mine1.exp', diff(exp_full, exp_empty))
        sp.add(U.OP_IMG, a=p.image('hud.%s.frame' % kind, frame))
        sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
        if exp is not None:
            sp.add(U.OP_BAR, a=U.F_EXP, b=exp, c=exp, d=exp, e=1, cond=U.C_EXP)
        for o in ops:
            if o[0] == 'name':
                self.text(sp, '{%d}' % U.F_NAME, o[1], o[2], reg=1)
            elif o[0] == 'level':
                self.level(sp, o[1], o[2])
            elif o[0] == 'gender':
                self.gender(sp, o[1], o[2], rel=o[3], reg=1)
            elif o[0] == 'hpnum':
                self.text(sp, '{%d}/{%d}' % (U.F_HP3, U.F_MAXHP3), o[1], o[2])
        # (the rows a full box keeps, and where it sits: KAI's box's side, a pixel in; on the Game Boy's white)
        worst = U.render(p, sp_canvas_only(sp), {U.F_NAME: 'MASQUERAIN', U.F_LEVEL: 100, U.F_HP3: '999',
                                                 U.F_MAXHP3: '999', U.F_HPFRAC: 1.0, U.F_EXP: 1.0},
                         U.C_LV3 | U.C_MALE | U.C_EXP)
        keep = self.kept_rows(worst, rh) if ch > rh else list(range(ch))
        x = 1 if foe else rw - 1 - cw
        y = (rh - len(keep)) // 2
        sp.add(U.OP_ROWMAP, x=x, y=y, a=p.string(bytes(keep)))
        sp.add(U.OP_BACKING, a=255, b=1, ink=st.paper)
        return sp

    def card(self, kind, rect):
        """gbstyle.card_grid: a Triple Battle card, 72 x 20; its bar where the level's digits leave it"""
        st, red, p = self.st, self.red, self.p
        foe = kind == 'foe3'
        rw, rh = rect[2] - rect[0], rect[3] - rect[1]
        sp = U.Spec('hud.' + kind, rw, rh, (72, 20))
        border = Image.new('RGBA', (72, 8), (0, 0, 0, 0))
        if foe:
            for k in range(0, 8):
                st.put(border, 0x76, k, 0)
            st.put(border, 0x78, 8, 0)
        else:
            st.put(border, 0x6F, 0, 0)
            for k in range(1, 8):
                st.put(border, 0x76, k, 0)
            st.put(border, 0x77, 8, 0)
        sp.add(U.OP_IMG, y=20 - (5 if red else 7), a=p.image('hud.%s.border' % kind, border))
        # (a level of one digit leaves the bar a tile more)
        for digits, cond, unless in ((1, U.C_LV1, 0), (2, 0, U.C_LV1)):
            col = 1 + digits + (0 if red else 1)
            if red:
                bar = (lambda c, f, colr, col=col: self.gb_bar(c, col, 1, f, 0x6C if foe else 0x6D, 'edge',
                                                               8 - col - 1, colr))
            else:
                bar = (lambda c, f, colr, col=col: self.gb_bar(c, col, 1, f, 0x6B, True, 8 - col - 2, colr))
            empty, fills = self.bar_specs('hud.%s.%d' % (kind, digits), (72, 20), bar)
            sp.add(U.OP_IMG, a=p.image('hud.%s.bar%d' % (kind, digits), empty), cond=cond, unless=unless)
            sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2], cond=cond, unless=unless)
        # (nine tiles hold nine letters; a tenth sets them a pixel closer)
        self.text(sp, '{%d}' % U.F_NAME, 0, 0, w=72, alt=self.font7 + 1)
        self.level(sp, 0, 8, reg=1)
        self.gender(sp, 0, 8, rel=True, reg=1)
        sp.add(U.OP_ROWMAP, x=2 if foe else rw - 2 - 72, y=(rh - 20) // 2, a=p.string(bytes(range(20))))
        sp.add(U.OP_BACKING, a=255, b=1, ink=st.paper)
        return sp

    # ---------------------------------------------------------------------------------------------------- panels
    def window_img(self, name, r):
        c = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        self.st.window(c, r)
        return self.p.image(name, c.crop(r))

    def panels(self):
        st, p = self.st, self.p
        # the battle's message: the whole panel, two lines (hlcoord 1, 14 / 1, 16)
        sp = U.Spec('b.msg')
        sp.add(U.OP_IMG, x=0, y=112, a=self.window_img('w.240x48', (0, 112, 240, 160)))
        sp.add(U.OP_MSG, x=8, y=128, a=0, h=16, n=2, w=216, b=8, c=4, d=120, font=self.font, ink=st.ink,
               shadow=st.mid)
        p.spec(sp)
        # the command: the prompt's box, the menu's box, ▶ on the chosen one
        sp = U.Spec('b.cmd')
        sp.add(U.OP_IMG, x=0, y=112, a=self.window_img('w.120x48', Lo.BATTLE_MSG))
        sp.add(U.OP_IMG, x=120, y=112, a=self.window_img('w.120x48', Lo.BATTLE_MENU))
        sp.add(U.OP_MSG, x=8, y=128, a=1, h=16, n=2, w=104, b=8, c=4, d=120, font=self.font, ink=st.ink,
               shadow=st.mid)
        for i in range(4):
            x, y = 136 + (i % 2) * 56, 128 + (i // 2) * 16
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, y)
            self.text(sp, '▶', x - 8, y, cond=U.C_SEL0 << i)
        p.spec(sp)
        # the moves: four rows, ▶ on the chosen; TYPE/ and the type, its PP (or what R / START show there)
        sp = U.Spec('b.moves')
        sp.add(U.OP_IMG, x=0, y=112, a=self.window_img('w.160x48', Lo.MOVES))
        sp.add(U.OP_IMG, x=160, y=112, a=self.window_img('w.80x48', Lo.MOVE_INFO))
        for i in range(4):
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), 16, 120 + 8 * i)
            self.text(sp, '▶', 8, 120 + 8 * i, cond=U.C_SEL0 << i)
        self.text(sp, 'TYPE/', 168, 120, unless=U.C_ALT)
        self.text(sp, '{%d}' % U.F_TYPE, 176, 128, unless=U.C_ALT)
        self.text(sp, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), 232, 140, align=1)
        sp.add(U.OP_MSG, x=168, y=120, a=10, h=8, n=2, font=self.font7, ink=st.ink, shadow=st.mid, cond=U.C_ALT)
        p.spec(sp)
        # the field's message box: two lines, ▼ waiting
        sp = U.Spec('f.msg')
        sp.add(U.OP_IMG, x=0, y=112, a=self.window_img('w.240x48', Lo.FIELD_MSG))
        sp.add(U.OP_MSG, x=8, y=128, a=0, h=16, n=2, w=216, b=8, c=4, d=120, font=self.font, ink=st.ink,
               shadow=st.mid)
        arrow = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
        GB.text(st, arrow, 0, 0, '▼')
        sp.add(U.OP_IMG, x=224, y=144, a=p.image('arrow', arrow), cond=U.C_WAIT)
        p.spec(sp)
        # START: DrawStartMenu's box from (10, 0), an entry every two rows, ▶ in column 11; Crystal's MENU ACCOUNT
        for n in range(1, 11):
            sp = U.Spec('start.%d' % n)
            sp.add(U.OP_IMG, x=160, y=0, a=self.window_img('w.start%d' % n, (160, 0, 240, min(160, 16 + 16 * n))))
            for i in range(n):
                self.text(sp, '{%d}' % (U.F_ITEM0 + i), 176, 16 + 16 * i)
                self.text(sp, '▶', 168, 16 + 16 * i, cond=U.C_SEL0 << i)
            if not self.red:
                # (ten tiles of words, as Crystal's own: "Close this", "key device")
                sp.add(U.OP_IMG, x=0, y=120, a=self.window_img('w.account', (0, 120, 96, 160)))
                self.text(sp, '{%d}' % (U.F_TEXT0 + 30), 8, 128)
                self.text(sp, '{%d}' % (U.F_TEXT0 + 31), 8, 144)
            p.spec(sp)

    def make(self):
        self.fonts()
        for kind in ('foe1', 'mine1', 'foe2', 'mine2', 'foe3', 'mine3'):
            self.p.spec(self.hud(kind))
        self.panels()
        if not self.red:
            self.p.spec(words_spec(self.p, 'start', ACCOUNT))
        return self.p


def words_spec(p, menu, lines):
    """a style's words for each entry of a menu (uipack.py: words.<menu>)"""
    sp = U.Spec('words.' + menu)
    for entry, ls in enumerate(lines):
        for k, t in enumerate(ls):
            if t:
                sp.add(U.OP_TEXT, a=p.string(t), b=entry, c=k)
    return sp


def map_spec(menu, values):
    """a style's value for each entry of a menu (uipack.py: map.<menu>)"""
    sp = U.Spec('map.' + menu)
    for entry, v in enumerate(values):
        sp.add(U.OP_IMG, b=entry, c=v)
    return sp


def diff(a, b):
    """A's pixels where they differ from B's (clear elsewhere)"""
    out = Image.new('RGBA', a.size, (0, 0, 0, 0))
    pa, pb, po = a.load(), b.load(), out.load()
    for y in range(a.height):
        for x in range(a.width):
            if pa[x, y] != pb[x, y] and pa[x, y][3]:
                po[x, y] = pa[x, y]
    return out


def sp_canvas_only(sp):
    """SP without its placing (its canvas as it's drawn)"""
    c = U.Spec(sp.name, sp.canvas[0], sp.canvas[1])
    c.ops = [o for o in sp.ops if o.kind not in (U.OP_ROWMAP, U.OP_BACKING)]
    return c
