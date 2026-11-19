"""RED and CRYSTAL as the game draws them (uipack.py): gbstyle.py's pieces made into a pack's fonts, pictures and
specs. Every spec puts things where gbstyle.py's mock-ups put them (approved 2026-11-16); check_specs.py draws both
and compares them."""
import os

from PIL import Image

import layout as Lo
import uipack as U
from gbstyle import GB

MALE_INK, FEMALE_INK = (56, 112, 232), (232, 64, 104)
GREEN, YELLOW, RED = (48, 184, 72), (232, 184, 0), (232, 64, 48)
EXP_BLUE = (32, 136, 248)

# Crystal's MENU ACCOUNT words for each START entry, by start_menu.c's MENU_ACTION_* (its own where it has one: POKéDEX,
# PARTY, PACK, POKéGEAR, status, SAVE, OPTION, EXIT; KAI's extras in the same voice, ten letters a line at most)
STATUS_WORDS = (('PSN', U.C_PSN), ('PAR', U.C_PAR), ('SLP', U.C_SLP), ('FRZ', U.C_FRZ), ('BRN', U.C_BRN))
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
                    for k, ch in enumerate(cells(s)):
                        g.tile(im, g.glyph(ch), x + pitch * k, y, ink)
                finally:
                    g.mid = mid
            return d

        def cells(s):
            return [s] if s in TOKENS else list(s)
        # (the codes KAI's charset has no single letter for, as the Game Boy fonts write them: the arrows the
        # d-pad's icons are written with, <PK><MN> and the level mark from the games' own tiles, POKéBLOCK's five
        # codes as its letters, the French ordinals' small letters)
        TOKENS = ('PK', 'MN', 'LV')
        extra = {0x79: '▲', 0x7C: '▶', 0x53: 'PK', 0x54: 'MN', 0x34: 'LV', 0x55: 'PO', 0x56: 'Ké', 0x57: 'BL',
                 0x58: 'OC', 0x59: 'K', 0x2C: 'er', 0x84: 'e', 0xA0: 're'}
        self.font = self.p.font('gb', U.font_from(draw(8), lambda s: 8 * len(cells(s)), 8, extra))
        f7 = U.font_from(draw(7), lambda s: 7 * len(cells(s)), 8, extra)
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

    def level(self, sp, x, y, reg=0, cond=0, unless=0):
        """PrintLevel: :L and the digits; three digits over the :L"""
        lv = self.p.image('lv', self.tile_img(0x6E))
        sp.add(U.OP_IMG, x=x, y=y, a=lv, cond=cond, unless=unless | U.C_LV3)
        self.text(sp, '{%d}' % U.F_LEVEL, x + 8, y, cond=cond, unless=unless | U.C_LV3, reg=reg)
        self.text(sp, '{%d}' % U.F_LEVEL, x, y, cond=cond | U.C_LV3, unless=unless, reg=reg)

    def level_or_status(self, sp, x, y, reg=0, cond=0, unless=0):
        """a battle box's level, or its status in the level's place (PrintStatusAilment: RED's and CRYSTAL's
        boxes print PSN / PAR / SLP / FRZ / BRN where :L and the digits go)"""
        self.level(sp, x, y, reg=reg, cond=cond, unless=unless | U.C_STATUS)
        for word, bit in STATUS_WORDS:
            self.text(sp, word, x, y, cond=cond | bit, unless=unless, reg=reg)

    def ball(self, sp, x, y, cond=U.C_CAUGHT):
        """the caught ball: CRYSTAL's own (DrawEnemyHUDBorder puts tile $5D, the expbar's last, at hlcoord 1, 1).
        RED had none (the first games never marked a caught kind): with GB OPTIONS > CAUGHT MARK on (the user,
        2026-11-17) it takes Red's own POKé BALL, the party's ball from its battles (gfx/battle/balls.png's first),
        in the same place"""
        if self.red:
            im = self.red_ball()
        else:
            im = self.tile_img(0x5D)
        sp.add(U.OP_IMG, x=x, y=y, a=self.p.image('ball', im), cond=cond)

    def red_ball(self):
        """Red's party POKé BALL tile in the style's ink, its grey and a lighter grey; white inside its outline"""
        t = Image.open(os.path.join(self.st.pret, 'gfx', 'battle', 'balls.png')).convert('L').crop((0, 0, 8, 8))
        st = self.st
        light = tuple((a + b) // 2 for a, b in zip(st.mid, st.paper))
        c = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
        for y in range(8):
            row = [t.getpixel((x, y)) for x in range(8)]
            inner = [x for x in range(8) if row[x] < 220]
            for x in range(8):
                v = row[x]
                if v < 64:
                    c.putpixel((x, y), st.ink + (255,))
                elif v < 128:
                    c.putpixel((x, y), st.mid + (255,))
                elif v < 220:
                    c.putpixel((x, y), light + (255,))
                elif inner and inner[0] < x < inner[-1]:
                    c.putpixel((x, y), st.paper + (255,))
        return c

    def gender(self, sp, x, y, rel=False, reg=0, cond=0, unless=0):
        if self.red:
            return
        self.text(sp, '♂', x, y, MALE_INK, cond=cond | U.C_MALE, unless=unless, rel=rel, reg=reg)
        self.text(sp, '♀', x, y, FEMALE_INK, cond=cond | U.C_FEMALE, unless=unless, rel=rel, reg=reg)

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
        if kind == 'foe2':
            # (the caught ball takes the bracket's top tile, sitting on its corner as CRYSTAL's sits over its side)
            caught = frame.copy()
            caught.paste((0, 0, 0, 0), (0, 8, 8, 16))
            sp.add(U.OP_IMG, a=p.image('hud.%s.frame' % kind, frame), unless=U.C_CAUGHT)
            sp.add(U.OP_IMG, a=p.image('hud.%s.framec' % kind, caught), cond=U.C_CAUGHT)
        else:
            sp.add(U.OP_IMG, a=p.image('hud.%s.frame' % kind, frame))
        sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
        if exp is not None:
            sp.add(U.OP_BAR, a=U.F_EXP, b=exp, c=exp, d=exp, e=1, cond=U.C_EXP)
        for o in ops:
            if o[0] == 'name':
                self.text(sp, '{%d}' % U.F_NAME, o[1], o[2], reg=1)
            elif o[0] == 'level':
                self.level_or_status(sp, o[1], o[2])
            elif o[0] == 'gender':
                self.gender(sp, o[1], o[2], rel=o[3], reg=1)
            elif o[0] == 'hpnum':
                self.text(sp, '{%d}/{%d}' % (U.F_HP3, U.F_MAXHP3), o[1], o[2])
        if foe:
            # (left of the level, where DrawEnemyHUDBorder puts it: hlcoord 1, 1)
            self.ball(sp, 0, 8)
        # (the rows a full box keeps, and where it sits: KAI's box's side, a pixel in; on the Game Boy's white)
        worst = U.render(p, sp_canvas_only(sp), {U.F_NAME: 'MASQUERAIN', U.F_LEVEL: 100, U.F_HP3: '999',
                                                 U.F_MAXHP3: '999', U.F_HPFRAC: 1.0, U.F_EXP: 1.0},
                         U.C_LV3 | U.C_MALE | U.C_EXP)
        keep = self.kept_rows(worst, rh) if ch > rh else list(range(ch))
        x = 1 if foe else rw - 1 - cw
        y = (rh - len(keep)) // 2
        sp.add(U.OP_ROWMAP, x=x, y=y, a=p.string(bytes(keep)))
        sp.add(U.OP_BACKING, a=255, b=1, ink=st.paper, unless=U.C_NOBG)
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
        if foe and not red:
            # (a caught kind's ball sits on the bracket's corner, as in CRYSTAL's own box: DrawEnemyHUDBorder)
            pedestal = border.copy()
            pedestal.paste((0, 0, 0, 0), (0, 0, 8, 8))
            st.put(pedestal, 0x74, 0, 0)
            pedestal.paste((0, 0, 0, 0), (0, 0, 8, 3))   # (the border starts in the level's row: the ball's over that)
            sp.add(U.OP_IMG, y=20 - 7, a=p.image('hud.%s.border' % kind, border), unless=U.C_CAUGHT)
            sp.add(U.OP_IMG, y=20 - 7, a=p.image('hud.%s.borderc' % kind, pedestal), cond=U.C_CAUGHT)
        else:
            sp.add(U.OP_IMG, y=20 - (5 if red else 7), a=p.image('hud.%s.border' % kind, border))
        # (a level of one digit leaves the bar a tile more; a wild foe's caught ball goes first on the level's row,
        # the level and the bar a tile on, CRYSTAL's bar then without its HP: to keep two tiles of gauge)
        self.text(sp, '{%d}' % U.F_NAME, 0, 0, w=72, alt=self.font7 + 1)
        for caught in ((False, True) if foe else (False,)):
            k = 1 if caught else 0
            cc = U.C_CAUGHT if caught else 0
            cu = U.C_CAUGHT if foe and not caught else 0
            # (a status takes the level's three tiles, as two digits and :L do: the two-digit bar)
            for digits, cond, unless, sfx in ((1, U.C_LV1, U.C_STATUS, ''), (2, 0, U.C_LV1, ''),
                                              (2, U.C_LV1 | U.C_STATUS, 0, 's')):
                col = 1 + digits + (0 if red else 1) + k
                if red:
                    bar = (lambda c, f, colr, col=col: self.gb_bar(c, col, 1, f, 0x6C if foe else 0x6D, 'edge',
                                                                   8 - col - 1, colr))
                elif caught:
                    bar = (lambda c, f, colr, col=col: self.gb_bar(c, col, 1, f, 0x6B, False, 8 - col, colr))
                else:
                    bar = (lambda c, f, colr, col=col: self.gb_bar(c, col, 1, f, 0x6B, True, 8 - col - 2, colr))
                tag = 'hud.%s.%d%s%s' % (kind, digits, 'c' if caught else '', sfx)
                empty, fills = self.bar_specs(tag, (72, 20), bar)
                sp.add(U.OP_IMG, a=p.image(tag.replace('.%d' % digits, '.bar%d' % digits), empty), cond=cond | cc,
                       unless=unless | cu)
                sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2], cond=cond | cc,
                       unless=unless | cu)
            if caught:
                self.ball(sp, 0, 8)
            self.level_or_status(sp, 8 * k, 8, reg=1, cond=cc, unless=cu)
            self.gender(sp, 0, 8, rel=True, reg=1, cond=cc, unless=cu)
        sp.add(U.OP_ROWMAP, x=2 if foe else rw - 2 - 72, y=(rh - 20) // 2, a=p.string(bytes(range(20))))
        sp.add(U.OP_BACKING, a=255, b=1, ink=st.paper, unless=U.C_NOBG)
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
        sp.add(U.OP_MSG, x=8, y=128, a=0, h=16, n=2, w=216, font=self.font, ink=st.ink,
               shadow=st.mid)
        p.spec(sp)
        # the command: the prompt's box, the menu's box, ▶ on the chosen one
        sp = U.Spec('b.cmd')
        sp.add(U.OP_IMG, x=0, y=112, a=self.window_img('w.120x48', Lo.BATTLE_MSG))
        sp.add(U.OP_IMG, x=120, y=112, a=self.window_img('w.120x48', Lo.BATTLE_MENU))
        sp.add(U.OP_MSG, x=8, y=128, a=1, h=16, n=2, w=104, font=self.font, ink=st.ink,
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
            # (the move held to swap with SELECT: its arrow in the light ink, as the PACK's held item's)
            self.text(sp, '▶', 8, 120 + 8 * i, st.mid, cond=U.C_HELD0 << i, unless=U.C_SEL0 << i)
        self.text(sp, 'TYPE/', 168, 120, unless=U.C_ALT)
        self.text(sp, '{%d}' % U.F_TYPE, 176, 128, unless=U.C_ALT)
        self.text(sp, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), 232, 140, align=1)
        sp.add(U.OP_MSG, x=168, y=120, a=10, h=8, n=2, font=self.font7, ink=st.ink, shadow=st.mid, cond=U.C_ALT)
        p.spec(sp)
        # the field's message box: two lines, ▼ waiting
        sp = U.Spec('f.msg')
        sp.add(U.OP_IMG, x=0, y=112, a=self.window_img('w.240x48', Lo.FIELD_MSG))
        sp.add(U.OP_MSG, x=8, y=128, a=0, h=16, n=2, w=216, font=self.font, ink=st.ink,
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

    # ---------------------------------------------------------------------------------------------------- party
    def party(self):
        """gbstyle.party(): RED / CRYSTAL's list, a row of name and :L and a row of HP: and its numbers each, ▶ on
        the chosen; the message along the foot (Kai's icons drawn where the list puts them)"""
        import gbstyle
        st, p = self.st, self.p
        sp = U.Spec('party')
        ground = Image.new('RGBA', (240, 160), st.paper + (255,))
        st.window(ground, (0, 136, 240, 160))
        sp.add(U.OP_IMG, a=p.image('party.ground', ground))
        sp.add(U.OP_MSG, x=8, y=144, a=0, h=16, n=1, w=224, font=self.font, ink=st.ink, shadow=st.mid)
        p.spec(sp)
        full = Image.new('RGBA', (240, 24), (0, 0, 0, 0))
        empty = Image.new('RGBA', (240, 24), (0, 0, 0, 0))
        st.hp_bar(empty, 48, 0, 0.0)
        fills = []
        keep = gbstyle.hp_colour
        for tier, col in (('g', GREEN), ('y', YELLOW), ('r', RED)):
            gbstyle.hp_colour = lambda f, g, y, r, col=col: col
            full = Image.new('RGBA', (240, 24), (0, 0, 0, 0))
            st.hp_bar(full, 48, 0, 1.0)
            fills.append(p.image('party.fill.' + tier, diff(full, empty)))
        gbstyle.hp_colour = keep
        bar = p.image('party.bar', empty)
        lv = p.image('lv.party', self.tile_img(0x6E))
        for i in range(6):
            y = 2 + 22 * i
            sp = U.Spec('party.%d' % i)
            sp.add(U.OP_ICON, x=22, y=y + 9, a=0, unless=U.P_EMPTY)
            sp.add(U.OP_ICON, x=152, y=y + 6, a=2, unless=U.P_EMPTY)
            self.text(sp, '▶', 0, y + 2, cond=U.C_SEL0)
            self.text(sp, '▶', 0, y + 2, cond=U.P_SWITCH)
            self.text(sp, '{%d}' % U.F_NAME, 40, y + 2, reg=1, unless=U.P_EMPTY)
            if not self.red:
                self.text(sp, '♂', 0, y + 2, MALE_INK, cond=U.C_MALE, rel=True, reg=1, unless=U.P_EMPTY | U.P_EGG)
                self.text(sp, '♀', 0, y + 2, FEMALE_INK, cond=U.C_FEMALE, rel=True, reg=1, unless=U.P_EMPTY | U.P_EGG)
            # (gbstyle.level: :L and the digits; three digits over the :L, as PrintLevel)
            sp.add(U.OP_IMG, x=176, y=y + 2, a=lv, unless=U.C_LV3 | U.P_EMPTY | U.P_EGG)
            self.text(sp, '{%d}' % U.F_LEVEL, 184, y + 2, unless=U.C_LV3 | U.P_EMPTY | U.P_EGG)
            self.text(sp, '{%d}' % U.F_LEVEL, 176, y + 2, cond=U.C_LV3, unless=U.P_EMPTY | U.P_EGG)
            no_hp = U.P_EMPTY | U.P_EGG | U.P_DESC
            sp.add(U.OP_IMG, x=0, y=y + 11, a=bar, unless=no_hp)
            sp.add(U.OP_BAR, x=0, y=y + 11, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2], unless=no_hp)
            self.text(sp, '{%d}/{%d}' % (U.F_HP3, U.F_MAXHP3), 232, y + 11, align=1, unless=no_hp)
            self.text(sp, '{%d}' % (U.F_TEXT0 + 1), 48, y + 11, cond=U.P_DESC, unless=U.P_EMPTY)
            p.spec(sp)
        # the Double / Triple Battle party (as approved, 2026-11-18): the POKéMON in battle in a framed box down the
        # left, each as the status screens draw one (name, :L, HP: and its bar, the numbers under it), the rest down
        # the right (name, :L and the numbers, the bar); the cursor goes between the columns as KAI's does
        for n, nact in (('party2', 2), ('party3', 3)):
            g = ground.copy()
            st.window(g, (0, 0, 120, 136))
            gsp = U.Spec(n)
            gsp.add(U.OP_IMG, a=p.image('%s.ground' % n, g))
            gsp.add(U.OP_MSG, x=8, y=144, a=0, h=16, n=1, w=224, font=self.font, ink=st.ink, shadow=st.mid)
            p.spec(gsp)
            for i in range(6):
                if i < nact:
                    x, y, act = 16, (8 + 40 * i) if nact == 3 else (12 + 56 * i), True
                else:
                    x, y, act = 128, 8 + 32 * (i - nact), False
                sp = U.Spec('%s.%d' % (n, i))
                if i not in (0, nact):
                    # (a co-op battle: a line where another player's POKéMON begin, across the column in the gap
                    # above the slot, the ink's light shade, as the status screens' rules)
                    gap = (12 if nact == 2 else 4) if act else 4
                    left, right = (8, 112) if act else (120, 232)
                    rule = Image.new('RGBA', (right - left, 1), st.mid + (255,))
                    sp.add(U.OP_IMG, x=left, y=y - gap, a=p.image('%s.teamrule.%d' % (n, right - left), rule),
                           cond=U.P_TEAMTOP)
                off, no = U.P_EMPTY, U.P_EMPTY | U.P_EGG
                no_hp = no | U.P_DESC
                sp.add(U.OP_ICON, x=x + 8, y=y + 4, a=0, unless=off)
                # (a status in the level's place, as the Game Boy's boxes print one)
                sp.add(U.OP_ICON, x=x + 36, y=y + 12, a=2, cond=U.C_STATUS, unless=off)
                self.text(sp, '▶', x - 8, y, cond=U.C_SEL0)
                self.text(sp, '▶', x - 8, y, cond=U.P_SWITCH)
                self.text(sp, '{%d}' % U.F_NAME, x + 20, y, unless=off)
                sp.add(U.OP_IMG, x=x + 20, y=y + 8, a=lv, unless=U.C_LV3 | no | U.C_STATUS)
                self.text(sp, '{%d}' % U.F_LEVEL, x + 28, y + 8, reg=2, unless=U.C_LV3 | no | U.C_STATUS)
                self.text(sp, '{%d}' % U.F_LEVEL, x + 20, y + 8, reg=2, cond=U.C_LV3, unless=no | U.C_STATUS)
                if not self.red:
                    self.text(sp, '♂', 0, y + 8, MALE_INK, cond=U.C_MALE, rel=True, reg=2, unless=no | U.C_STATUS)
                    self.text(sp, '♀', 0, y + 8, FEMALE_INK, cond=U.C_FEMALE, rel=True, reg=2,
                              unless=no | U.C_STATUS)
                # (the bar's picture is drawn from x 48 of its own: placed so its HP: sits at x + 12)
                sp.add(U.OP_IMG, x=x + 12 - 48, y=y + 16, a=bar, unless=no_hp)
                sp.add(U.OP_BAR, x=x + 12 - 48, y=y + 16, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2],
                       unless=no_hp)
                if act:
                    self.text(sp, '{%d}/{%d}' % (U.F_HP3, U.F_MAXHP3), x + 84, y + 24, align=1, unless=no_hp)
                else:
                    self.text(sp, '{%d}/{%d}' % (U.F_HP3, U.F_MAXHP3), x + 104, y + 8, align=1, unless=no_hp)
                self.text(sp, '{%d}' % (U.F_TEXT0 + 1), x + 20, y + 16, cond=U.P_DESC, unless=off)
                p.spec(sp)

    # ---------------------------------------------------------------------------------------------------- OPTION
    def options(self):
        """gbstyle.py options_page(): the tab (or a page's title) in the ink bar, L / R; six rows two tiles apart, the
        name, then on the row under it CRYSTAL's ":" and the value at column 10, RED's choices with ▶ at the one on
        (or ◀ the one on ▶ when there are more than three); ▶ at the picked row; a line about the row on the last two
        rows, ▼ when more rows follow"""
        from common import OPTION_TABS
        st, p = self.st, self.p
        ground = Image.new('RGBA', (240, 160), st.paper + (255,))
        st.fill(ground, (0, 0, 240, 24), st.ink)
        st.at(ground, 1, 1, 'L', st.paper)
        st.at(ground, 28, 1, 'R', st.paper)
        st.window(ground, (0, 24, 240, 160))
        sp = U.Spec('o')
        sp.add(U.OP_IMG, a=p.image('%s.o.ground' % self.kind, ground))
        for i in range(len(OPTION_TABS)):
            self.text(sp, '◀ {%d} ▶' % (U.F_ITEM0 + i), 120, 8, st.paper, cond=U.C_SEL0 << i, unless=U.S_PAGE,
                      align=3)
        self.text(sp, '{%d}' % U.F_TITLE, 120, 8, st.paper, cond=U.S_PAGE, align=3)
        sp.add(U.OP_MSG, x=8, y=128, a=0, h=8, n=2, w=216, font=self.font, ink=st.ink, shadow=st.mid, cond=U.S_HINT)
        self.text(sp, '▼', 224, 136, cond=U.S_DOWN)
        sp.add(U.OP_LIST, x=0, y=32, h=16, n=6)
        p.spec(sp)
        row = U.Spec('o.row')
        self.text(row, '▶', 8, 0, cond=U.C_SEL0)
        self.text(row, '{%d}' % U.F_NAME, 16, 0)
        if self.red:
            self.text(row, '◀ {%d} ▶' % U.F_CURSTR, 24, 8, cond=U.O_VALUE, unless=U.O_ALL)
            row.add(U.OP_CHOICES, x=24, y=8, w=208, n=16, a=U.F_CHOICE0, b=U.encode('▶')[0], font=self.font,
                    ink=st.ink, shadow=st.mid, c=U.col(st.ink), d=U.col(st.mid), cond=U.O_ALL)
        else:
            self.text(row, ':{%d}' % U.F_CURSTR, 80, 8, cond=U.O_VALUE)
        p.spec(row)

    # ---------------------------------------------------------------------------------------------------- BAG
    def bag(self):
        """gbstyle.py bag_page(): RED's ITEM box over the field (four rows, two rows each, the count on the second,
        ◀ the pocket ▶ across the top, ▼ for more) and its words below; CRYSTAL's PACK (the header with START:SORT,
        the PACK and the pocket's name under it, five rows, CANCEL for CLOSE BAG) and its words below; more words
        than fit close up to a row apart"""
        st, p = self.st, self.p
        from gbstyle import GB
        sp = U.Spec('bag')
        if self.red:
            frame = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
            st.window(frame, (32, 0, 240, 88))
            st.window(frame, (0, 88, 240, 160))
            sp.add(U.OP_SNAP)
            sp.add(U.OP_IMG, a=p.image('red.bag.frame', frame))
            self.text(sp, '◀{%d}▶' % U.F_TITLE, 148, 8, align=3)
            self.text(sp, '▼', 224, 72, cond=U.S_DOWN)
            sp.add(U.OP_MSG, x=8, y=96, a=0, h=16, n=4, w=224, font=self.font, ink=st.ink,
                   shadow=st.mid)
            sp.add(U.OP_LIST, x=0, y=16, h=16, n=4)
            col_name, col_qty, col_cur = 6, 24, 5
        else:
            import os
            # (CHRIS's PACK on blue, KRIS's on pink: cgb_layouts.asm _CGB_PackPals by wPlayerGender, gfx/pack/pack.pal
            # and pack_f.pal; the picture for the pocket as pack.asm DrawPackGFX picks it: PackGFX's frame 1 the
            # ITEM pocket's, 3 the BALL pocket's, 0 the KEY ITEM pocket's, 2 the TM/HM pocket's)
            menu = st._tiles(os.path.join(st.pret, 'gfx', 'pack', 'pack_menu.png'))
            frames = [1, 1, 3, 2, 1, 1, 0, 1]     # (KAI's ITEMS, MEDICINE, POKé BALLS, TMs & HMs, BERRIES, ...)
            for sex, png, cols in ((0, 'pack.png', [(248, 248, 248), (120, 152, 248), (24, 56, 200), (16, 16, 24)]),
                                   (1, 'pack_f.png', [(248, 248, 248), (240, 136, 224), (208, 64, 192),
                                                      (16, 16, 24)])):
                ground = Image.new('RGBA', (240, 160), cols[1] + (255,))
                ground.paste((16, 16, 24, 255), (0, 0, 240, 8))
                for col in range(20):
                    GB.tile(st, ground, menu[0x28 + col], col * 8, 0, st.paper, (16, 16, 24))
                st.at(ground, 20, 0, 'START:SORT', st.paper)
                st.fill(ground, (0, 56, 80, 64), st.paper)
                st.fill(ground, (80, 8, 240, 96), st.paper)   # (to the box: the fifth row's count on row 11)
                st.window(ground, (0, 96, 240, 160))
                g = dict(cond=U.C_FEMALE) if sex else dict(unless=U.C_FEMALE)
                sp.add(U.OP_IMG, a=p.image('crystal.bag.ground%d' % sex, ground), **g)
                for i, k in enumerate(frames):
                    pic = st.gbc_picture(os.path.join(st.pret, 'gfx', 'pack', png), cols, (0, 24 * k, 40, 24 * k + 24))
                    c = dict(g)
                    c['cond'] = c.get('cond', 0) | (U.C_SEL0 << i)
                    sp.add(U.OP_IMG, x=24, y=24, a=p.image('crystal.bag.pack%d.%d' % (sex, k), pic), **c)
            # (the pocket's name on its strip under the PACK; one too long for its ten columns on two lines)
            sp.add(U.OP_LONG, a=U.F_TITLE, font=self.font, w=80)
            self.text(sp, '{%d}' % U.F_TITLE, 40, 56, align=3, unless=U.C_LONG)
            strip = Image.new('RGBA', (80, 8), st.paper + (255,))
            sp.add(U.OP_IMG, x=0, y=64, a=p.image('crystal.bag.strip', strip), cond=U.C_LONG)
            sp.add(U.OP_MSG, x=0, y=56, a=1, h=8, n=2, w=80, align=3, font=self.font, ink=st.ink, shadow=st.mid,
                   cond=U.C_LONG)
            self.text(sp, '▼', 232, 80, cond=U.S_DOWN)
            sp.add(U.OP_MSG, x=8, y=104, a=0, h=16, n=3, w=224, font=self.font, ink=st.ink,
                   shadow=st.mid)
            sp.add(U.OP_LIST, x=0, y=16, h=16, n=5)
            col_name, col_qty, col_cur = 12, 26, 11
        p.spec(sp)
        row = U.Spec('bag.row')
        self.text(row, '▶', col_cur * 8, 0, cond=U.C_SEL0)
        if self.red:
            self.text(row, '{%d}' % U.F_NAME, col_name * 8, 0)
        else:
            self.text(row, '{%d}' % U.F_NAME, col_name * 8, 0, unless=U.B_CLOSE)
            self.text(row, 'CANCEL', col_name * 8, 0, cond=U.B_CLOSE)
        self.text(row, '×{%d}' % U.F_QTY2, col_qty * 8, 8, cond=U.B_QTY)
        self.text(row, '{%d}' % U.F_TAG, 232, 8, cond=U.B_TAG, align=1)
        p.spec(row)

    # ---------------------------------------------------------------------------------------------------- pop-ups
    def popups(self):
        """KAI's own pop-ups (menus, YES / NO, the save's box) in the Game Boy's box and font (mp_uistyle.c PopUps)"""
        st, p = self.st, self.p
        frame = Image.new('RGBA', (24, 24), (0, 0, 0, 0))
        st.window(frame, (0, 0, 24, 24))
        sp = U.Spec('win')
        sp.add(U.OP_IMG, a=p.image('%s.win' % self.kind, frame))
        sp.add(U.OP_FILL, w=0, h=0, ink=st.paper, a=255)
        p.spec(sp)
        t = U.Spec('win.text')
        self.text(t, '{%d}' % U.F_NAME, 0, 3, unless=U.C_ALT)
        self.text(t, '{%d}' % U.F_NAME, 0, 3, st.mid, cond=U.C_ALT)
        p.spec(t)

    # ---------------------------------------------------------------------------------------------------- summary
    TYPE_WORDS = ['NORMAL', 'FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', '???', 'FIRE',
                  'WATER', 'GRASS', 'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK', 'FAIRY']
    CONTEST_WORDS = ['COOL', 'BEAUTY', 'CUTE', 'SMART', 'TOUGH']

    def words_imgs(self, tag, words, ink=None):
        """a picture of each word in the Game Boy font (a value shown by its number: IMGSEL)"""
        st, p = self.st, self.p
        first = None
        for i, wd in enumerate(words):
            im = Image.new('RGBA', (8 * max(1, len(wd)), 8), (0, 0, 0, 0))
            st.text(im, 0, 0, wd, ink or st.ink)
            k = p.image('%s.%s%d' % (self.kind, tag, i), im)
            first = k if first is None else first
            assert k == first + i
        return first

    def shape_imgs(self, tag, shape, counts, on, off, perfect=None):
        """N of a little 8 x 8 shape (stars, hearts) in a row, the rest pale: one picture for each count"""
        p = self.p
        first = None
        for i, (n, total) in enumerate(counts):
            im = Image.new('RGBA', (8 * total, 8), (0, 0, 0, 0))
            for k in range(total):
                col = (perfect if (perfect and n > total) else on) if k < min(n, total) else off
                for yy in range(8):
                    for xx in range(8):
                        if shape[yy] & (0x80 >> xx):
                            im.putpixel((k * 8 + xx, yy), col + (255,))
            j = p.image('%s.%s%d' % (self.kind, tag, i), im)
            first = j if first is None else first
            assert j == first + i
        return first

    def summary(self):
        """KAI's six summary pages as RED's status screens and CRYSTAL's STATS pages lay theirs out: the picture,
        name and :L above (CRYSTAL's page squares, six), the page's own below: INFO as approved; SKILLS the stats
        (KAI's IV stars and EVs), EXP POINTS and LEVEL UP; the moves with PP (the picked one's TYPE / POWER / ACC.
        and its words; for contests its kind, APPEAL and JAM); the POKé BALL and the JOURNAL on the whole
        screen"""
        import os
        import gbstyle
        from gbstyle import GB
        st, p, red = self.st, self.p, self.red
        ink, mid, paper = st.ink, st.mid, st.paper
        types = self.words_imgs('type', self.TYPE_WORDS)
        contests = self.words_imgs('contest', self.CONTEST_WORDS)
        star = (0x10, 0x10, 0x38, 0xFE, 0x7C, 0x38, 0x6C, 0x44)
        heart = (0x00, 0x6C, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00)
        pale = tuple((c + 2 * pc) // 3 for c, pc in zip(mid, paper))
        stars = self.shape_imgs('stars', star, [(0, 3), (1, 3), (2, 3), (3, 3), (4, 3)], ink, pale,
                                perfect=ink if red else (232, 64, 48))
        hearts = self.shape_imgs('hearts', heart, [(n, 8) for n in range(9)], ink if red else (232, 64, 48), pale)
        # the HP bar (as the party's): empty, and each colour's fill
        empty = Image.new('RGBA', (240, 8), (0, 0, 0, 0))
        hp_at = (11, 3) if red else (0, 9)
        st.hp_tiles(empty, hp_at[0], 0, 0.0, 0x6D if red else 0x6B)
        fills = []
        keep = gbstyle.hp_colour
        for tier, col in (('g', GREEN), ('y', YELLOW), ('r', RED)):
            gbstyle.hp_colour = lambda f, g, y, r, col=col: col
            full = Image.new('RGBA', (240, 8), (0, 0, 0, 0))
            st.hp_tiles(full, hp_at[0], 0, 1.0, 0x6D if red else 0x6B)
            fills.append(p.image('%s.sum.fill.%s' % (self.kind, tier), diff(full, empty)))
        gbstyle.hp_colour = keep
        bar = p.image('%s.sum.bar' % self.kind, empty)
        page_cols = [(232, 96, 184), (96, 200, 72), (72, 136, 232), (232, 160, 72), (160, 120, 232), (88, 184, 200)]
        light = [(248, 176, 232)] + [tuple((c + 2 * 248) // 3 for c in pc) for pc in page_cols[1:]]
        stiles = None
        if not red:
            tiles = st._tiles_2bpp(os.path.join(st.pret, 'gfx', 'stats', 'stats_tiles.2bpp'))
            stiles = {0x31 + k: t for k, t in enumerate(tiles)}

        def at(sp, col, row, s_, ink_=None, **k):
            self.text(sp, s_, col * 8, row * 8, ink_, **k)

        def status(sp, col, row):
            for word, bit in (('PSN', U.C_PSN), ('PAR', U.C_PAR), ('SLP', U.C_SLP), ('FRZ', U.C_FRZ), ('BRN', U.C_BRN)):
                at(sp, col, row, word, cond=bit)
            at(sp, col, row, 'FNT', cond=U.C_STATUS, unless=U.C_PSN | U.C_PAR | U.C_SLP | U.C_FRZ | U.C_BRN)
            at(sp, col, row, 'OK', unless=U.C_STATUS)

        def top(k, ground, sp, info=True):
            """the picture and the words above (RED's status screen's top, CRYSTAL's STATS top)"""
            sp.add(U.OP_ICON, x=32, y=32, a=0)
            if red:
                at(sp, 9, 1, '{%d}' % U.F_NAME)
                self.level(sp, 14 * 8, 2 * 8)
                if info:
                    sp.add(U.OP_IMG, x=0, y=24, a=bar)
                    sp.add(U.OP_BAR, x=0, y=24, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
                    at(sp, 12, 4, '{%d}/{%d}' % (U.F_HP3, U.F_MAXHP3))
                    st.at(ground, 9, 6, 'STATUS/')
                    status(sp, 16, 6)
                    for r in (1, 2, 4, 5, 6):
                        st.tile_at(ground, st.bt(0x73), 19, r)
                    st.tile_at(ground, st.bt(0x77), 19, 7)
                    for col in range(10, 19):
                        st.tile_at(ground, st.bt(0x76), col, 7)
                    st.tile_at(ground, st.bt(0x6F), 9, 7)
                st.tile_at(ground, st.batt[0x74 - 0x62], 1, 8)
                at(sp, 2, 8, '.{%d}' % U.F_DEXNO)
                st.at(ground, 27, 1, '%d/6' % (k + 1))
            else:
                ground.paste(paper + (255,), (0, 0, 240, 64))
                st.tile_at(ground, st.batt[0x74 - 0x60], 8, 0)
                at(sp, 9, 0, '.{%d}' % U.F_DEXNO)
                self.level(sp, 14 * 8, 0)
                at(sp, 18, 0, '♂', MALE_INK, cond=U.C_MALE)
                at(sp, 18, 0, '♀', FEMALE_INK, cond=U.C_FEMALE)
                at(sp, 8, 2, '{%d}' % U.F_NAME, unless=U.SM_CHOOSING)
                at(sp, 9, 4, '/{%d}' % U.F_SPECIES, unless=U.SM_CHOOSING)
                for j in range(6):
                    base = 0x3A if j == k else 0x36
                    for q in range(4):
                        t, col, row = stiles[base + q], 9 + 2 * j + q % 2, 5 + q // 2
                        GB.tile(st, ground, t, col * 8, row * 8)
                        px = ground.load()
                        for yy in range(8):
                            for xx in range(8):
                                if 64 <= t.getpixel((xx, yy)) < 160:
                                    px[col * 8 + xx, row * 8 + yy] = page_cols[j] + (255,)
                for col in range(30):
                    st.tile_at(ground, st.bt(0x62), col, 7)

        def new(k):
            g = Image.new('RGBA', (240, 160), (paper if red else light[k]) + (255,))
            return g, U.Spec('sum.%d' % k)

        def done(sp, g, k):
            sp.ops.insert(0, U.Op(U.OP_IMG, a=p.image('%s.sum%d' % (self.kind, k), g)))
            p.spec(sp)

        # INFO (as approved)
        g, sp = new(0)
        top(0, g, sp)
        if red:
            st.at(g, 21, 5, 'SELECT')
            st.at(g, 22, 6, ':RENAME')
            st.window(g, (0, 9 * 8, 19 * 8, 160))
            for j, (lab, fld) in enumerate([('ABILITY/', U.F_ABILITY), ('NATURE/', U.F_NATURE),
                                           ('MET/', U.F_METPLACE)]):
                st.at(g, 1, 10 + 3 * j, lab)
                at(sp, 2, 11 + 3 * j, '{%d}' % fld)
            # (as RED's status screen: TYPE1/, TYPE2/ when there's a second, IDNo/, OT/, two rows each)
            for j, (lab, fld) in enumerate([('TYPE1/', None), (None, None), ('IDNo/', U.F_IDNO), ('OT/', U.F_OT)]):
                if lab is None:
                    at(sp, 20, 10 + 2 * j, 'TYPE2/', cond=U.SM_TWOTYPES)
                    sp.add(U.OP_IMGSEL, x=21 * 8, y=(11 + 2 * j) * 8, a=types, b=U.F_TYPE2, cond=U.SM_TWOTYPES)
                    continue
                st.at(g, 20, 10 + 2 * j, lab)
                if fld is None:
                    sp.add(U.OP_IMGSEL, x=21 * 8, y=11 * 8, a=types, b=U.F_TYPE1)
                else:
                    at(sp, 22, 11 + 2 * j, '{%d}' % fld)
            st.line_box(g, 29, 10, 19, 19)
        else:
            for row in range(8, 20):
                st.tile_at(g, stiles[0x31], 9, row)
            sp.add(U.OP_IMG, x=0, y=72, a=bar)
            sp.add(U.OP_BAR, x=0, y=72, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
            at(sp, 1, 10, '{%d}/{%d}' % (U.F_HP3, U.F_MAXHP3))
            st.at(g, 0, 12, 'STATUS/')
            status(sp, 1, 13)
            st.at(g, 0, 15, 'TYPE/')
            sp.add(U.OP_IMGSEL, x=8, y=16 * 8, a=types, b=U.F_TYPE1)
            sp.add(U.OP_IMGSEL, x=8, y=17 * 8, a=types, b=U.F_TYPE2, cond=U.SM_TWOTYPES)   # (the second under it)
            for j, (lab, fld) in enumerate([('ABILITY/', U.F_ABILITY), ('NATURE/', U.F_NATURE),
                                           ('MET/', U.F_METPLACE)]):
                st.at(g, 10, 9 + 3 * j, lab)
                at(sp, 11, 10 + 3 * j, '{%d}' % fld)
            at(sp, 16, 18, 'SELECT:RENAME', cond=U.SM_RENAME)
        done(sp, g, 0)
        # SKILLS
        g, sp = new(1)
        top(1, g, sp)
        labels = ['HP', 'ATTACK', 'DEFENSE', 'SPCL.ATK', 'SPCL.DEF', 'SPEED']
        if red:
            st.window(g, (0, 9 * 8, 19 * 8, 160))
            rows = [(1, 10 + j, 17, 10 + j) for j in range(6)]
            st.at(g, 20, 10, 'EXP POINTS')
            at(sp, 29, 11, '{%d}' % U.F_EXPPTS, align=1)
            st.at(g, 20, 13, 'LEVEL UP')
            at(sp, 29, 14, '{%d}' % U.F_NEXTLV, align=1)
            at(sp, 1, 17, 'START:CLEAR', cond=U.SM_CLEAREV)
        else:
            for row in range(8, 20):
                st.tile_at(g, stiles[0x31], 9, row)
            st.at(g, 0, 9, 'ID')
            st.tile_at(g, st.batt[0x74 - 0x60], 2, 9)
            st.at(g, 3, 9, '/')
            at(sp, 1, 10, '{%d}' % U.F_IDNO)
            st.at(g, 0, 11, 'OT/')
            at(sp, 1, 12, '{%d}' % U.F_OT)
            st.at(g, 0, 14, 'EXP')
            at(sp, 9, 15, '{%d}' % U.F_EXPPTS, align=1)
            st.at(g, 0, 16, 'NEXT LV')
            at(sp, 9, 17, '{%d}' % U.F_NEXTLV, align=1)
            at(sp, 0, 18, 'START:CLEAR', cond=U.SM_CLEAREV)
            rows = [(11, 8 + 2 * j, 29, 9 + 2 * j) for j in range(6)]
        for j, (lc, lr, vc, vr) in enumerate(rows):
            st.at(g, lc, lr, labels[j])
            fld = U.F_STAT0 + j - 1
            if j == 0:
                at(sp, vc, vr, '{%d}' % U.F_MAXHP, align=1, unless=U.SM_IV | U.SM_EV)
            else:
                at(sp, vc, vr, '{%d}' % fld, align=1, unless=U.SM_IV | U.SM_EV)
                # (the stat its nature raises or lowers: + or - after it, the Game Boy has no colours for it)
                at(sp, vc + 1 if not red else vc - 7, vr, '+', cond=U.C_SEL0 << (j - 1), unless=U.SM_IV | U.SM_EV)
                at(sp, vc + 1 if not red else vc - 7, vr, '-', cond=U.C_SEL0 << (4 + j), unless=U.SM_IV | U.SM_EV)
            sp.add(U.OP_IMGSEL, x=vc * 8 - 24, y=vr * 8, a=stars, b=U.F_IV0 + j, cond=U.SM_IV)
            at(sp, vc, vr, '{%d}' % (U.F_EV0 + j), align=1, cond=U.SM_EV)
        done(sp, g, 1)
        # the moves
        for k, contest in ((2, False), (3, True)):
            g, sp = new(k)
            top(k, g, sp, info=False)
            if red:
                st.window(g, (0, 9 * 8, 240, 160))
                mcol, ppcol = 2, 11
                sp.add(U.OP_MSG, x=9 * 8, y=3 * 8, a=1, h=8, n=4, w=160, font=self.font, ink=ink, shadow=mid,
                       cond=U.SM_CHOOSING)
                dcol, drow = 20, 10
            else:
                for row in range(8, 20):
                    st.tile_at(g, stiles[0x31], 9, row)
                mcol, ppcol = 11, 20
                sp.add(U.OP_MSG, x=8 * 8, y=8, a=1, h=8, n=4, w=176, font=self.font, ink=ink, shadow=mid,
                       cond=U.SM_CHOOSING)
                dcol, drow = 0, 9
                st.at(g, 0, 15, 'ITEM')
                at(sp, 0, 16, '{%d}' % U.F_HELD, unless=U.SM_CHOOSING, w=72, alt=self.font7 + 1)
            if contest:
                at(sp, dcol, drow, 'APPEAL', cond=U.SM_CHOOSING)
                sp.add(U.OP_IMGSEL, x=dcol * 8 + 8, y=(drow + 1) * 8, a=hearts, b=U.F_APPEAL, cond=U.SM_CHOOSING)
                at(sp, dcol, drow + 2, 'JAM', cond=U.SM_CHOOSING)
                sp.add(U.OP_IMGSEL, x=dcol * 8 + 8, y=(drow + 3) * 8, a=hearts, b=U.F_JAM, cond=U.SM_CHOOSING)
            else:
                at(sp, dcol, drow, 'POWER/', cond=U.SM_CHOOSING)
                at(sp, dcol + 8, drow + 1, '{%d}' % U.F_POWER, align=1, cond=U.SM_CHOOSING)
                at(sp, dcol, drow + 2, 'ACC./', cond=U.SM_CHOOSING)
                at(sp, dcol + 8, drow + 3, '{%d}' % U.F_ACC, align=1, cond=U.SM_CHOOSING)
            sp.add(U.OP_LIST, x=0, y=(10 if red else 9) * 8, h=16, n=5)
            done(sp, g, k)
            row = U.Spec('sum.%d.row' % k)
            self.text(row, '▶', (mcol - 1) * 8, 0, cond=U.C_SEL0)
            self.text(row, '▶', (mcol - 1) * 8, 0, mid, cond=U.B_SWAP, unless=U.C_SEL0)
            self.text(row, '{%d}' % U.F_NAME, mcol * 8, 0, unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '-', mcol * 8, 0, cond=U.R_EMPTY)
            self.text(row, 'CANCEL', mcol * 8, 0, cond=U.R_CANCEL)
            if contest:
                row.add(U.OP_IMGSEL, x=ppcol * 8, y=8, a=contests, b=U.F_CTYPE, unless=U.R_EMPTY | U.R_CANCEL)
            else:
                self.text(row, 'PP', ppcol * 8, 8, unless=U.R_EMPTY | U.R_CANCEL)
                self.text(row, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), (ppcol + 8) * 8, 8, align=1,
                          unless=U.R_EMPTY | U.R_CANCEL)
            p.spec(row)
        # the POKé BALL (as approved, 2026-11-18): the INFO page's top, the ball's words in the page's own boxes
        # (RED: BALL / COLOR / DECOR in the window, EFFECT and the buttons in the line box, the ball where RENAME
        # was; CRYSTAL: the ball and the buttons in the left column, the words in the right), a message box for
        # KAI's question (use a ball?) and its notes
        g, sp = new(4)
        top(4, g, sp)
        narrow = self.font7 + 1
        ED, ASK, IDLE = U.SB_EDIT, U.SB_ASK, U.SB_IDLE
        if red:
            sp.add(U.OP_ICON, x=184, y=32, a=4)
            at(sp, 21, 5, 'A', cond=IDLE)
            at(sp, 22, 6, ':BALL', cond=IDLE)
            st.window(g, (0, 9 * 8, 19 * 8, 160))
            st.at(g, 1, 10, 'BALL/')
            at(sp, 2, 11, '{%d}' % U.F_BALL, w=16 * 8, alt=narrow)
            at(sp, 2, 12, '{%d}' % U.F_BALLSUB, w=16 * 8, alt=narrow)
            st.at(g, 1, 14, 'COLOR/')
            at(sp, 2, 15, '{%d}' % U.F_BALLCOLOR, w=16 * 8, alt=narrow)
            st.at(g, 1, 17, 'DECOR/')
            at(sp, 2, 18, '{%d}' % U.F_BALLDECOR, w=16 * 8, alt=narrow)
            at(sp, 20, 10, 'EFFECT/', unless=ED)
            at(sp, 21, 11, '{%d}' % U.F_BALLFX, w=8 * 8, alt=narrow, unless=ED)
            at(sp, 20, 14, 'SELECT', cond=IDLE)
            at(sp, 21, 15, ':EFFECT', cond=IDLE)
            at(sp, 20, 10, '▲▼:PICK', cond=ED)
            at(sp, 20, 12, '◀▶:', cond=ED)
            at(sp, 21, 13, 'CHANGE', cond=ED)
            at(sp, 20, 15, 'A:OK', cond=ED)
            at(sp, 20, 17, 'B:CANCEL', cond=ED)
            st.line_box(g, 29, 10, 19, 19)
        else:
            for row in range(8, 20):
                st.tile_at(g, stiles[0x31], 9, row)
            sp.add(U.OP_ICON, x=36, y=88, a=4)
            # (the left column the ball and its EFFECT; the right the ball's words, the buttons at its foot where
            # INFO's SELECT:RENAME is)
            st.at(g, 0, 14, 'EFFECT/')
            at(sp, 1, 15, '{%d}' % U.F_BALLFX, w=8 * 8, alt=narrow)
            st.at(g, 10, 9, 'BALL/')
            at(sp, 11, 10, '{%d}' % U.F_BALL, w=19 * 8, alt=narrow)
            at(sp, 11, 11, '{%d}' % U.F_BALLSUB, w=19 * 8, alt=narrow)
            st.at(g, 10, 12, 'COLOR/')
            at(sp, 11, 13, '{%d}' % U.F_BALLCOLOR, w=19 * 8, alt=narrow)
            st.at(g, 10, 14, 'DECOR/')
            at(sp, 11, 15, '{%d}' % U.F_BALLDECOR, w=19 * 8, alt=narrow)
            at(sp, 10, 17, 'A:BALL', cond=IDLE)
            at(sp, 30, 17, 'SELECT:EFFECT', cond=IDLE, align=1)
            at(sp, 10, 17, '▲▼:PICK', cond=ED)
            at(sp, 30, 17, '◀▶:CHANGE', cond=ED, align=1)
            at(sp, 10, 18, 'A:OK', cond=ED)
            at(sp, 30, 18, 'B:CANCEL', cond=ED, align=1)
        # (KAI's question and notes: the games' message box over the page's foot)
        box = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        st.window(box, (0, 14 * 8, 240, 160), 'message')
        box = box.crop((0, 14 * 8, 240, 160))
        boxk = p.image('%s.sum4.box' % self.kind, box)
        for cond, unless in ((ASK, 0), (0, IDLE | ED | ASK)):
            sp.add(U.OP_IMG, x=0, y=14 * 8, a=boxk, cond=cond, unless=unless)
            at(sp, 1, 16, '{%d}' % U.F_BALLHELP0, w=28 * 8, alt=narrow, cond=cond, unless=unless)
            at(sp, 1, 18, '{%d}' % U.F_BALLHELP1, w=28 * 8, alt=narrow, cond=cond, unless=unless)
        done(sp, g, 4)
        # the JOURNAL: the whole screen
        for k in (5,):
            g = Image.new('RGBA', (240, 160), paper + (255,))
            sp = U.Spec('sum.%d' % k)
            st.window(g, (0, 8, 240, 160))
            at(sp, 0, 0, '{%d}' % U.F_PAGETITLE)
            st.at(g, 27, 0, '%d/6' % (k + 1))
            if True:
                at(sp, 1, 2, '{%d}' % U.F_NAME)
                # (a blank row between lines as the games' own text has them; a page that needs more rows than that
                # gives them all, eight pixels apart)
                sp.add(U.OP_MSG, x=8, y=32, a=3, h=16, n=7, w=224, font=self.font, ink=ink,
                       shadow=mid)
                at(sp, 28, 18, '{%d}' % U.F_JPAGE, align=1, cond=U.S_DOWN)
            done(sp, g, k)
            row = U.Spec('sum.%d.row' % k)
            self.text(row, '{%d}' % U.F_NAME, 0, 4, w=216, alt=self.font7 + 1)
            p.spec(row)

    def menu_icons(self):
        """the party's POKéMON as the Game Boy game drew them (gb_icons.py: its own icons for its own POKéMON, the same
        decisions carried to every other), in the style's shades: OPTIONS > DISPLAY > MENU POKéMON ORIGINAL"""
        import gb_icons
        st, p = self.st, self.p
        frames, table, names, _ = gb_icons.icons(self.kind, st.pret)
        light = tuple((a + b) // 2 for a, b in zip(st.mid, st.paper))
        cols = [st.ink, st.mid, light]
        base = None
        for i, pair in enumerate(frames):
            ims = []
            for f, rows in enumerate(pair):
                im = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
                for y, r in enumerate(rows):
                    for x, v in enumerate(r):
                        if v < 3:
                            im.putpixel((x, y), cols[v] + (255,))
                ims.append(im)
            for f, im in enumerate(anchor_icon(*ims)):
                k = p.image('gbicon.%d.%d' % (i, f), im)
                base = k if base is None else base
                assert k == base + 2 * i + f
        # a held item: Crystal's own little tile (gfx/stats/item.png, mail.png), in place of the icon's bottom-left
        # quarter as Crystal lays it (SPRITE_ANIM_OAMSET_PARTY_MON_WITH_ITEM); RED's too (the user, 2026-11-17)
        held = U.Spec('helditem')
        for n in ('item', 'mail'):
            src = Image.open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data',
                                          'crystal_%s.png' % n)).convert('L')   # (pokecrystal gfx/stats)
            im = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
            for y in range(8):
                for x in range(8):
                    v = gb_icons._shade(src.getpixel((x, y)))
                    if v < 3:
                        im.putpixel((x, y), cols[v] + (255,))
            held.add(U.OP_IMG, a=p.image('gbheld.' + n, im))
        p.spec(held)
        default = names.index('ICON_MON' if self.red else 'ICON_MONSTER')
        sp = U.Spec('icons')
        sp.add(U.OP_ICONSET, a=p.string(table + bytes([default])), b=base, c=len(frames), d=0)
        # EXTENDED (CRYSTAL): Crystal Clear's own icon for each of the 251 and the EGG, in its own colours
        # (cc_icons.py, data/crystal_clear_icons.png); every later POKéMON takes one as gb_icons does ORIGINAL's
        cc = None if self.red else __import__('cc_icons').frames()
        if cc:
            ext = None
            for i, pair in enumerate(cc):
                for f, im in enumerate(anchor_icon(*pair)):
                    k = p.image('ccicon.%d.%d' % (i, f), im)
                    ext = k if ext is None else ext
                    assert k == ext + 2 * i + f
            species = gb_icons._pokeapi()
            own = ['CC%d' % n for n in range(1, 252)]
            mapping = gb_icons._map(own, species, 'CC1')
            egg = len(cc) - 1
            ctable = bytes([egg] + [int(mapping[n][2:]) - 1 for n in range(1, gb_icons.NATIONAL_MAX + 1)])
            sp.add(U.OP_ICONSET, a=p.string(ctable + bytes([0])), b=ext, c=len(cc), d=1)
        p.spec(sp)

    def make(self):
        self.fonts()
        self.menu_icons()
        for kind in ('foe1', 'mine1', 'foe2', 'mine2', 'foe3', 'mine3'):
            self.p.spec(self.hud(kind))
        self.panels()
        self.party()
        self.options()
        self.bag()
        self.popups()
        self.summary()
        if not self.red:
            self.p.spec(words_spec(self.p, 'start', ACCOUNT))
        return self.p


def _shift(im, dx, dy):
    out = Image.new('RGBA', im.size, (0, 0, 0, 0))
    out.paste(im, (dx, dy))
    return out


def anchor_icon(a, b):
    """a party icon's two frames, both moved by the same amount: the first frame centred across its 16 pixels and
    standing on the bottom row, like every other icon, and the second kept where the game draws it against the first.
    So the frames move exactly as pret draws them (the user, 2026-11-17: "some Pokemon in Red and Crystal SHOULD bounce
    1px up"): Red's MONSTER hops, the BALL and the HELIX shake a pixel (pokered AnimatePartyMon), Crystal's poses stand
    where Crystal puts them. Each frame centred by itself made icons drift; lining the second up on the first took the
    games' own hops away."""
    box = a.getchannel('A').getbbox()
    if not box:
        return a, b
    dx = (a.width - (box[2] - box[0])) // 2 - box[0]
    dy = a.height - box[3]
    return _shift(a, dx, dy), _shift(b, dx, dy)


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
