"""FIRERED as the game draws it (uipack.py): fr.py's pieces made into a pack's fonts, pictures and specs. Every spec
puts things where fr.py's mock-ups put them (approved 2026-11-16); check_specs.py draws both and compares them."""
from PIL import Image

import layout as Lo
import uipack as U
import fr as F
from fr import FireRed
from gb_build import diff, words_spec

# FireRed's START help bar words, by start_menu.c's MENU_ACTION_* (its own where it has them: pokefirered's
# gStartMenuDesc_*; KAI's extras in the same voice)
HELP = [('A device that records MOéMON secrets', 'upon meeting or catching them.'),
        ('Check and organize MOéMON that are', 'traveling with you in your party.'),
        ('Equipped with pockets for storing items', 'you bought, received, or found.'),
        ('Your MOéNAV: the map, MATCH CALL', 'and more on the road.'),
        ('Check your money and other game data.', ''),
        ('Save your game with a complete record', 'of your progress to take a break.'),
        ('Adjust various game settings such as text', 'speed, game rules, etc.'),
        ('Close this MENU window.', ''),
        ('Retire from the SAFARI GAME and return', 'to the registration counter.'),
        ('Check your money and other game data.', ''),
        ('Save your game and take a rest.', ''),
        ('Retire from the challenge and return', 'to the counter.'),
        ('Equipped with pockets for storing items', 'you bought, received, or found.'),
        ('Play with friends: rooms, trades,', 'battles and more.'),
        ('Go to the MOéMON GARDEN to see', 'your MOéMON at play.'),
        ('Your POKéGEAR: the map, the phone', 'and the radio.'),
        ('Your C-GEAR: wireless play with', 'trainers near and far.')]


class FRPack:
    def __init__(self, root):
        self.st = FireRed(root, None, None)
        self.p = U.Pack('FIRERED')

    # ---------------------------------------------------------------------------------------------------- fonts
    def font(self, which):
        """FireRed's font sheet, a glyph for each of the game's codes (FireRed's charset is Emerald's)"""
        sheet, wd, cw, ch = self.st.fonts[which]
        per = sheet.width // cw
        gp = sheet.load()
        f = U.Font(ch)
        for k in range(256):
            if k in (U.FIELD, U.EOS, U.NEWLINE) or k >= len(wd):
                continue
            gx, gy = (k % per) * cw, (k // per) * ch
            if gy + ch > sheet.height:
                continue
            pix = [[(1 if gp[gx + x, gy + y] == 1 else 2 if gp[gx + x, gy + y] == 2 else 0) for x in range(wd[k])]
                   for y in range(ch)]
            f.add(k, wd[k], 0, 0, pix)
        return self.p.font('fr.' + which, f)

    def fonts(self):
        self.fn, self.fs = self.font('n'), self.font('s')

    # ---------------------------------------------------------------------------------------------------- helpers
    def text(self, sp, s, x, y, cols, font=None, **k):
        ink, sh = cols
        sp.add(U.OP_TEXT, x=x, y=y, a=self.p.string(s), font=self.fs if font == 's' else self.fn, ink=ink,
               shadow=sh, **k)

    def img(self, sp, name, im, x=0, y=0, **k):
        sp.add(U.OP_IMG, x=x, y=y, a=self.p.image(name, im), **k)

    def bar_imgs(self, name, size, draw):
        """the HP bar empty (a picture) and each colour's fill: FireRed's green / yellow / red tiles
        (healthbox_elements 3 / 47 / 56 + pixels)"""
        st = self.st
        green = st.elements

        def tiers():
            for base in (3, 47, 56):
                yield base

        empty = Image.new('RGBA', size, (0, 0, 0, 0))
        draw(empty, 0.0)
        fills = []
        for tier, base in zip('gyr', tiers()):
            st.elements = list(green)
            for n in range(9):
                st.elements[3 + n] = green[base + n]
            full = Image.new('RGBA', size, (0, 0, 0, 0))
            draw(full, 1.0)
            fills.append(self.p.image('%s.fill.%s' % (name, tier), diff(full, empty)))
        st.elements = green
        return empty, fills

    # ---------------------------------------------------------------------------------------------------- the HUDs
    def hud(self, kind):
        """fr.py hud(): the box's art, its words, its bars, each a part the game fills"""
        st, p = self.st, self.p
        rect = {'foe': Lo.SINGLE_FOE, 'mine': Lo.SINGLE_MINE, 'foe2': Lo.DOUBLE_FOES[0],
                'mine2': Lo.DOUBLE_MINE[0], 'foe3': Lo.TRIPLE_FOES[0], 'mine3': Lo.TRIPLE_MINE[0]}[kind]
        name = 'hud.' + {'foe': 'foe1', 'mine': 'mine1'}.get(kind, kind)
        sp = U.Spec(name, rect[2] - rect[0], rect[3] - rect[1])
        mine = kind.startswith('mine')
        if kind in ('foe3', 'mine3'):
            return self.card(sp, rect, mine)
        x0, y0 = 0, 0
        if kind in ('foe', 'mine'):
            art = st.art('player' if mine else 'foe')
            art = art.crop((0, 0, art.width, art.height - 1))
            if mine:
                art = st.drop_col(art, 102)
            ty, nx, right = 1, (16 if mine else 7), (95 if mine else 86)
        else:
            art = st.drop_rows(st.art('pdouble' if mine else 'fdouble'), {6, 7})
            ty, nx, right = -1, (15 if mine else 7), (94 if mine else 86)
        if kind == 'mine':
            q = art.load()
            for y in range(19, 30):
                for x in range(56, 96):
                    q[x, y] = (255, 255, 222, 255)
        end, by = st.bar_slot(art)
        size = (art.width + 8, art.height + 8)

        def bar(im, frac):
            st.bar(im, x0 + end - 64, y0 + by, frac)
        empty, fills = self.bar_imgs(name, size, bar)
        frame = Image.new('RGBA', size, (0, 0, 0, 0))
        frame.alpha_composite(art, (0, 0))
        frame.alpha_composite(empty)
        exp = None
        if kind == 'mine':
            e0 = Image.new('RGBA', size, (0, 0, 0, 0))
            e1 = Image.new('RGBA', size, (0, 0, 0, 0))
            st.exp_bar(e0, 32, 30, 0.0, bottom=art.height)
            st.exp_bar(e1, 32, 30, 1.0, bottom=art.height)
            frame.alpha_composite(e0)
            exp = p.image(name + '.exp', diff(e1, e0))
        self.img(sp, name + '.frame', frame)
        sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
        if exp is not None:
            sp.add(U.OP_BAR, a=U.F_EXP, b=exp, c=exp, d=exp, cond=U.C_EXP)
        self.text(sp, '{%d}' % U.F_NAME, nx, ty, F.HBOX, 's', reg=1)
        self.text(sp, '♂', 0, ty, F.MALE, 's', rel=True, reg=1, cond=U.C_MALE)
        self.text(sp, '♀', 0, ty, F.FEMALE, 's', rel=True, reg=1, cond=U.C_FEMALE)
        self.text(sp, '{LV}{%d}' % U.F_LEVEL, right, ty, F.HBOX, 's', align=1)
        if kind == 'mine':
            # " 56/" at 60 and " 56" at 80, cut at the window's last row (row 30) as the game's are
            self.text(sp, '{%d}/' % U.F_HP3, 60, 19, F.HBOX, 's', h=11)
            self.text(sp, '{%d}' % U.F_MAXHP3, 80, 19, F.HBOX, 's', h=11)
        return sp

    def card(self, sp, rect, mine):
        """fr.py hud_card(): the Triple Battle card, its bar after the level (as wide as the level leaves it)"""
        st, p = self.st, self.p
        w, h = rect[2] - rect[0], rect[3] - rect[1]
        card = st.card(mine, w, h)
        end, by = st.bar_slot(card)
        tx = st.card_text_x(card)
        self.img(sp, sp.name + '.card', card)
        lvw = st.measure('{LV}', 's')
        digit = st.measure('0', 's')
        assert all(st.measure(str(d), 's') == digit for d in range(10)), 'FireRed small digits not all one width'
        for n, cond, unless in ((1, U.C_LV1, 0), (2, 0, U.C_LV1 | U.C_LV3), (3, U.C_LV3, 0)):
            lx = tx + lvw + n * digit
            bx = max(lx + 1, tx + 19)
            cut = end - (bx + 16)

            def bar(im, frac, bx=bx, cut=cut):
                st.bar(im, bx, by, frac, tiles=6, cut=cut)
            empty, fills = self.bar_imgs('%s.%d' % (sp.name, n), (w + 8, h + 8), bar)
            self.img(sp, '%s.bar%d' % (sp.name, n), empty, cond=cond, unless=unless)
            sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2], cond=cond, unless=unless)
        self.text(sp, '{%d}' % U.F_NAME, tx, -2, F.HBOX, 's', reg=1)
        self.text(sp, '♂', 0, -2, F.MALE, 's', rel=True, reg=1, cond=U.C_MALE)
        self.text(sp, '♀', 0, -2, F.FEMALE, 's', rel=True, reg=1, cond=U.C_FEMALE)
        self.text(sp, '{LV}{%d}' % U.F_LEVEL, tx, 6, F.HBOX, 's')
        return sp

    # ---------------------------------------------------------------------------------------------------- panels
    def panels(self):
        st, p = self.st, self.p
        tb = st.textbox
        # the battle's message: the textbox's message screen, two lines from (10, 122)
        sp = U.Spec('b.msg')
        self.img(sp, 'tb.msg', tb.crop((0, 112, 240, 160)), 0, 112)
        sp.add(U.OP_MSG, x=10, y=122, a=0, h=16, n=2, w=220, font=self.fn, ink=F.BATTLE_MSG[0],
               shadow=F.BATTLE_MSG[1])
        p.spec(sp)
        # the command: the prompt and FIGHT BAG / MOéMON RUN, ▶ on the chosen
        sp = U.Spec('b.cmd')
        self.img(sp, 'tb.cmd', tb.crop((0, 272, 240, 320)), 0, 112)
        sp.add(U.OP_MSG, x=10, y=122, a=1, h=16, n=2, w=106, font=self.fn, ink=F.BATTLE_MSG[0],
               shadow=F.BATTLE_MSG[1])
        for i in range(4):
            x, y = 136 + (i % 2) * 56, 122 + (i // 2) * 16
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, y, F.BATTLE)
            self.text(sp, '▶', x - 8, y, F.BATTLE, cond=U.C_SEL0 << i)
        p.spec(sp)
        # the moves: FireRed's two by two in the small font, PP and TYPE/ beside them (or what R / START show)
        sp = U.Spec('b.moves')
        self.img(sp, 'tb.moves', tb.crop((0, 432, 240, 480)), 0, 112)
        for i in range(4):
            x, y = 16 + (i % 2) * 72, 121 + (i // 2) * 16
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, y, F.BATTLE, 's')
            self.text(sp, '▶', x - 8, y - 1, F.BATTLE, cond=U.C_SEL0 << i)
        self.text(sp, 'PP', 168, 122, F.BATTLE, 's', unless=U.C_ALT)
        self.text(sp, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), 230, 122, F.BATTLE, align=1, unless=U.C_ALT)
        self.text(sp, 'TYPE/{%d}' % U.F_TYPE, 168, 138, F.BATTLE, 's', unless=U.C_ALT)
        sp.add(U.OP_MSG, x=168, y=122, a=10, h=16, n=2, font=self.fs, ink=F.BATTLE[0], shadow=F.BATTLE[1],
               cond=U.C_ALT)
        p.spec(sp)
        # the field's message box: menu_message's frame, two lines from (16, 121), the ▼ after the words
        sp = U.Spec('f.msg')
        box = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        st.dialogue(box, (0, 112, 240, 160))
        self.img(sp, 'f.msg.box', box.crop((0, 112, 240, 160)), 0, 112)
        sp.add(U.OP_MSG, x=16, y=121, a=0, h=16, n=2, w=208, font=self.fn, ink=F.STD[0], shadow=F.STD[1], reg=2)
        arrow = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
        st.more_arrow(arrow, 0, 0)
        self.img(sp, 'arrow', arrow, 1, 0, rel=True, rely=True, reg=2, cond=U.C_WAIT)
        p.spec(sp)
        # START: the user frame at tiles 21-29, a row each 15 pixels, ▶ at 176; the blue help bar under it (up to
        # seven entries, as FireRed has: more take the bar's place)
        for n in range(1, 11):
            sp = U.Spec('start.%d' % n)
            h = min(160, 16 * n + 8)        # (CreateStartMenuWindow: 2n - 1 tiles inside the frame)
            box = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
            st.frame(box, (168, 0, 240, h))
            self.img(sp, 'start.box%d' % n, box.crop((168, 0, 240, h)), 168, 0)
            for i in range(n):
                self.text(sp, '{%d}' % (U.F_ITEM0 + i), 184, 9 + 15 * i, F.STD)
                self.text(sp, '▶', 176, 9 + 15 * i, F.STD, cond=U.C_SEL0 << i)
            if n <= 7:
                bar = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
                st.help_bar(bar, (0, 120, 240, 160))
                self.img(sp, 'start.help', bar.crop((0, 120, 240, 160)), 0, 120)
                self.text(sp, '{%d}' % (U.F_TEXT0 + 30), 2, 125, F.WHITE, w=236, b=self.fs + 1)
                self.text(sp, '{%d}' % (U.F_TEXT0 + 31), 2, 141, F.WHITE, w=236, b=self.fs + 1)
            p.spec(sp)
        for a, b in HELP:
            for t in (a, b):
                assert st.measure(t) <= 236, ('FireRed help too wide', t)
        p.spec(words_spec(p, 'start', HELP))

    def make(self):
        self.fonts()
        for kind in ('foe', 'mine', 'foe2', 'mine2', 'foe3', 'mine3'):
            self.p.spec(self.hud(kind))
        self.panels()
        return self.p
