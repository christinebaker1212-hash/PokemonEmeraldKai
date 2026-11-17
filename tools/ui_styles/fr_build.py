"""FIRERED as the game draws it (uipack.py): fr.py's pieces made into a pack's fonts, pictures and specs. Every spec
puts things where fr.py's mock-ups put them (approved 2026-11-16); check_specs.py draws both and compares them."""
import os

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

    def status_imgs(self):
        """FireRed's status badges (healthbox_elements: three tiles each from B_INTERFACE_GFX_STATUS_*_BATTLER0, their
        body colour 12 filled with sStatusIconColors as UpdateStatusIconInHealthbox does), by the status's bit"""
        if getattr(self, '_status', None):
            return self._status
        el = Image.open(os.path.join(self.st.g, 'battle_interface', 'healthbox_elements.png'))
        self._status = {}
        for bit, base, c in ((U.C_PSN, 21, (24, 12, 24)), (U.C_PAR, 24, (23, 23, 3)), (U.C_SLP, 27, (20, 20, 17)),
                             (U.C_FRZ, 30, (17, 22, 28)), (U.C_BRN, 33, (28, 14, 10))):
            pal = list(self.st.barpal)
            pal[12] = tuple((v << 3) | (v >> 2) for v in c)
            tiles = self.st._tiles(el, pal)
            im = Image.new('RGBA', (24, 8), (0, 0, 0, 0))
            for k in range(3):
                im.alpha_composite(tiles[base + k], (k * 8, 0))
            self._status[bit] = self.p.image('fr.status.%d' % bit, im)
        return self._status

    def status(self, sp, x, y, cond=0, unless=0):
        for bit, im in self.status_imgs().items():
            sp.add(U.OP_IMG, x=x, y=y, a=im, cond=cond | bit, unless=unless)

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
        # (a status: UpdateStatusIconInHealthbox's badge, three tiles that end where the HP label began, the label
        # given up for the bar's left edge, B_INTERFACE_GFX_HP_BAR_LEFT_BORDER, the box's own ground behind)
        hx, hy = x0 + end - 64, y0 + by
        patch = art.crop((hx, hy, hx + 16, hy + 8)) if hx + 16 <= art.width and hy + 8 <= art.height else None
        if patch is not None:
            patch = patch.copy()
            patch.alpha_composite(st.elements[65], (8, 0))
            self.img(sp, name + '.nolabel', patch, hx, hy, cond=U.C_STATUS)
        self.status(sp, hx - 16, hy)
        if exp is not None:
            sp.add(U.OP_BAR, a=U.F_EXP, b=exp, c=exp, d=exp, cond=U.C_EXP)
        self.text(sp, '{%d}' % U.F_NAME, nx, ty, F.HBOX, 's', reg=1)
        self.text(sp, '♂', 0, ty, F.MALE, 's', rel=True, reg=1, cond=U.C_MALE)
        self.text(sp, '♀', 0, ty, F.FEMALE, 's', rel=True, reg=1, cond=U.C_FEMALE)
        self.text(sp, '{LV}{%d}' % U.F_LEVEL, right, ty, F.HBOX, 's', align=1)
        if not mine:
            # (TryAddPokeballIconToHealthbox: B_INTERFACE_GFX_BALL_CAUGHT two tiles before the HP label, a wild foe's)
            self.img(sp, 'fr.caught', st.elements[70], x0 + end - 64 - 16, y0 + by, cond=U.C_CAUGHT,
                     unless=U.C_STATUS)   # (TryAddPokeballIconToHealthbox(noStatus FALSE): a status takes its place)
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
        # (a status: the badge in the level's place, the bar after it as after a three-digit level)
        for n, cond, unless in ((1, U.C_LV1, U.C_STATUS), (2, 0, U.C_LV1 | U.C_LV3 | U.C_STATUS),
                                (3, U.C_LV3, U.C_STATUS), (4, U.C_STATUS, 0)):
            lx = tx + lvw + n * digit if n < 4 else tx + 24
            bx = max(lx + 1, tx + 19)
            cut = end - (bx + 16)

            def bar(im, frac, bx=bx, cut=cut):
                st.bar(im, bx, by, frac, tiles=6, cut=cut)
            empty, fills = self.bar_imgs('%s.%d' % (sp.name, n), (w + 8, h + 8), bar)
            self.img(sp, '%s.bar%d' % (sp.name, n), empty, cond=cond, unless=unless)
            sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2], cond=cond, unless=unless)
        if mine:
            self.text(sp, '{%d}' % U.F_NAME, tx, -2, F.HBOX, 's', reg=1)
        else:
            # (a wild foe's caught ball before the name, which moves on past it)
            ball = st.elements[70]
            self.img(sp, 'fr.caught', ball, tx - 1, 0, cond=U.C_CAUGHT)
            self.text(sp, '{%d}' % U.F_NAME, tx, -2, F.HBOX, 's', reg=1, unless=U.C_CAUGHT)
            self.text(sp, '{%d}' % U.F_NAME, tx + 7, -2, F.HBOX, 's', reg=1, cond=U.C_CAUGHT)
        self.text(sp, '♂', 0, -2, F.MALE, 's', rel=True, reg=1, cond=U.C_MALE)
        self.text(sp, '♀', 0, -2, F.FEMALE, 's', rel=True, reg=1, cond=U.C_FEMALE)
        self.text(sp, '{LV}{%d}' % U.F_LEVEL, tx, 6, F.HBOX, 's', unless=U.C_STATUS)
        self.status(sp, tx, 9)
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
            # (the move held to swap with SELECT: its arrow pale, as the BAG's held item's)
            self.text(sp, '▶', x - 8, y - 1, (F.BATTLE[1], F.BATTLE[1]), cond=U.C_HELD0 << i, unless=U.C_SEL0 << i)
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

    # ---------------------------------------------------------------------------------------------------- party
    def party(self):
        """fr.py party(): FireRed's party screen (its bg and slots, the chosen slot in its own colours), the words in
        its small font where party_menu.h's sPartyBoxInfoRects put them, the HP bar's fill, the message, CANCEL"""
        import os
        from PIL import Image as I
        st, p = self.st, self.p
        a = st.a
        bg = I.open(os.path.join(st.g, 'party_menu', 'bg.png'))
        pal = F.png_pal(bg)
        tiles = st._tiles(bg, pal, 48)
        sel = list(pal)
        for off, idx in zip((4, 5, 6, 1, 7, 8), (116, 117, 118, 97, 103, 104)):
            sel[48 + off] = pal[idx]
        sel_tiles = st._tiles(bg, sel, 48)

        def slot(name, w, h, current):
            d = open(os.path.join(st.g, 'party_menu', name), 'rb').read()
            ts = sel_tiles if current else tiles
            im = I.new('RGBA', (w * 8, h * 8), (0, 0, 0, 0))
            for k, t in enumerate(d[:w * h]):
                im.alpha_composite(ts[t], ((k % w) * 8, (k // w) * 8))
            return im
        ground = I.new('RGBA', (240, 160), (0, 0, 0, 255))
        ground.paste(a._tilemap('party_menu/bg.png', 'party_menu/bg.bin', pal, 32, 20), (0, 0))
        st.frame(ground, (0, 128, 184, 160))
        ink = (pal[48 + 3][:3], pal[48 + 2][:3])
        male, female = (pal[48 + 11][:3], pal[48 + 12][:3]), (pal[64 + 11][:3], pal[64 + 12][:3])
        sp = U.Spec('party')
        self.img(sp, 'fr.party.ground', ground)
        sp.add(U.OP_MSG, x=8, y=137, a=0, h=16, n=1, w=168, e=self.fs + 1, font=self.fn, ink=F.STD[0],
               shadow=F.STD[1])
        self.text(sp, 'CANCEL', 216, 135, ink, 's', align=2)
        p.spec(sp)
        # the HP bar's fill colours: green as fr.py's, yellow and red from the battle bar's own tiles (healthbar.pal)
        def bar_rows(base):
            t = st.elements[base + 8]
            return t.getpixel((2, 3))[:3], t.getpixel((2, 4))[:3]
        tiers = [((90, 213, 131), (115, 255, 172))] + [bar_rows(b) for b in (47, 56)]
        for i in range(6):
            sp = U.Spec('party.%d' % i)
            if i == 0:
                x, y, rect = 8, 24, (24, 11, 32, 20, 64, 20, 38, 36, 53, 36, 24, 35)
                icon, status, desc = (16, 40), (50, 52), (12, 34)
                art = [('main', slot('slot_main.bin', 10, 7, False), slot('slot_main.bin', 10, 7, True))]
            else:
                x, y, rect = 96, 8 + (i - 1) * 24, (22, 3, 32, 12, 64, 12, 102, 12, 117, 12, 88, 10)
                icon, status, desc = (104, 18 + 24 * (i - 1)), (136, 27 + 24 * (i - 1)), (77, 4)
                art = [('wide', slot('slot_wide.bin', 18, 3, False), slot('slot_wide.bin', 18, 3, True))]
                self.img(sp, 'fr.slot.empty', slot('slot_wide_empty.bin', 18, 3, False), x, y, cond=U.P_EMPTY)
            for name, plain, chosen in art:
                self.img(sp, 'fr.slot.%s' % name, plain, x, y, unless=U.P_EMPTY | U.C_SEL0 | U.P_SWITCH)
                self.img(sp, 'fr.slot.%s.sel' % name, chosen, x, y, cond=U.C_SEL0, unless=U.P_EMPTY)
                self.img(sp, 'fr.slot.%s.sel' % name, chosen, x, y, cond=U.P_SWITCH, unless=U.P_EMPTY | U.C_SEL0)
            nx, ny, lx, ly, gx, gy, hx, hy, mx, my, bx, by = rect
            off = U.P_EMPTY
            sp.add(U.OP_ICON, x=icon[0], y=icon[1], a=0, unless=off)
            sp.add(U.OP_ICON, x=status[0], y=status[1], a=2, unless=off)
            self.text(sp, '{%d}' % U.F_NAME, x + nx, y + ny, ink, 's', unless=off)
            no = U.P_EMPTY | U.P_EGG
            # (Emerald's and FireRed's party: a status in the level's place)
            self.text(sp, '{LV}{%d}' % U.F_LEVEL, x + lx, y + ly, ink, 's', unless=no | U.C_STATUS)
            self.text(sp, '♂', x + gx, y + gy, male, 's', cond=U.C_MALE, unless=no)
            self.text(sp, '♀', x + gx, y + gy, female, 's', cond=U.C_FEMALE, unless=no)
            no_hp = no | U.P_DESC
            self.text(sp, '{%d}/' % U.F_HP, x + hx + 20, y + hy, ink, 's', align=1, unless=no_hp)
            self.text(sp, '{%d}' % U.F_MAXHP, x + mx + 20, y + my, ink, 's', align=1, unless=no_hp)
            fills = []
            for t, (deep, lite) in zip('gyr', tiers):
                im = I.new('RGBA', (48, 3), (0, 0, 0, 0))
                for k, c in enumerate((deep, lite, lite)):
                    im.paste(c + (255,), (0, k, 48, k + 1))
                fills.append(p.image('fr.partybar.%s' % t, im))
            sp.add(U.OP_BAR, x=x + bx, y=y + by, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2], unless=no_hp)
            self.text(sp, '{%d}' % (U.F_TEXT0 + 1), x + desc[0], y + desc[1], ink, 's', cond=U.P_DESC, unless=off)
            p.spec(sp)

    # ---------------------------------------------------------------------------------------------------- OPTION
    def options(self):
        """fr.py options_page(): FireRed's OPTION (the help bar, the header window with KAI's tabs, the list window),
        seven rows 13 apart, each its name and the chosen value in red, the rows not picked lightened; a page's title
        in the header; a line about the row in the help bar's place; FireRed's scroll arrows for more rows"""
        from common import OPTION_TABS
        st, p = self.st, self.p
        ground = Image.new('RGBA', (240, 160), (0, 0, 0, 255))
        st.frame(ground, (8, 16, 232, 48), 'std')
        st.frame(ground, (8, 48, 232, 160))
        sp = U.Spec('o')
        self.img(sp, 'fr.o.ground', ground)
        bar = Image.new('RGBA', (240, 16), F.BLUE_BAR + (255,))
        keys = bar.copy()
        x = 228 - (8 + st.measure('PICK', 's') + 4 + 8 + st.measure('SWITCH', 's') + 4 + 32 +
                   st.measure('TAB', 's') + 4 + 16 + st.measure('CANCEL', 's'))
        x = st.key(keys, x, 2, 'UPDOWN')
        x = st.text(keys, x, 0, 'PICK', *F.WHITE, font='s') + 4
        x = st.key(keys, x, 2, 'LEFTRIGHT')
        x = st.text(keys, x, 0, 'SWITCH', *F.WHITE, font='s') + 4
        x = st.key(keys, x, 2, 'L')
        x = st.key(keys, x, 2, 'R')
        x = st.text(keys, x, 0, 'TAB', *F.WHITE, font='s') + 4
        x = st.key(keys, x, 2, 'A')
        x = st.key(keys, x, 2, 'B')
        st.text(keys, x, 0, 'CANCEL', *F.WHITE, font='s')
        self.img(sp, 'fr.o.keys', keys, 0, 0, unless=U.S_HINT)
        self.img(sp, 'fr.o.bar', bar, 0, 0, cond=U.S_HINT)
        self.text(sp, '{%d}' % U.F_HINT, 4, 0, F.WHITE, 's', cond=U.S_HINT)
        # KAI's tabs where FireRed writes OPTION (the gaps as the mock-up's), the open one red; a page's title
        gap = (208 - sum(st.measure(t) for t in OPTION_TABS)) // (len(OPTION_TABS) - 1)
        x = 16
        for i, t in enumerate(OPTION_TABS):
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, 25, F.RED, cond=U.C_SEL0 << i, unless=U.S_PAGE)
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, 25, F.STD, unless=U.S_PAGE | (U.C_SEL0 << i))
            x += st.measure(t) + gap
        self.text(sp, '{%d}' % U.F_TITLE, 16, 25, F.STD, cond=U.S_PAGE, w=208, b=self.fs + 1)
        # (list_menu.c's scroll arrows: red_arrow's chevron turned up and down, on the list window's edges)
        chev = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
        st.arrow(chev, 0, 0, left=True)
        self.img(sp, 'fr.o.up', chev.transpose(Image.ROTATE_270), 112, 42, cond=U.S_UP)
        self.img(sp, 'fr.o.down', chev.transpose(Image.ROTATE_90), 112, 150, cond=U.S_DOWN)
        sp.add(U.OP_LIST, x=0, y=58, h=13, n=7)
        p.spec(sp)
        row = U.Spec('o.row')
        self.text(row, '{%d}' % U.F_NAME, 24, 0, F.STD, w=118, b=self.fs + 1)
        self.text(row, '{%d}' % U.F_CURSTR, 146, 0, F.RED, w=78, b=self.fs + 1)
        row.add(U.OP_FILL, x=16, y=0, w=208, h=13, ink=(255, 255, 255), a=48, unless=U.C_SEL0)
        p.spec(row)

    # ---------------------------------------------------------------------------------------------------- BAG
    def bag(self):
        """fr.py bag_page(): FireRed's BAG (item_menu bg, the bag, the pocket arrows), the pocket's name on the
        band, KAI's eight pockets as pips, five rows 16 apart (the count in the small font, a registered item's
        button where its count would be), the red ▶, START: SORT, the item's icon and its words beside it"""
        import os
        st, p = self.st, self.p
        a = st.a
        ground = Image.new('RGBA', (240, 160), (0, 0, 0, 255))
        bg = Image.open(os.path.join(st.g, 'item_menu', 'bg.png'))
        ground.paste(a._tilemap('item_menu/bg.png', 'item_menu/bg.bin', F.png_pal(bg), 32, 20), (0, 0))
        bagpic = a._sheet('interface/bag_male.png', F.png_pal(Image.open(os.path.join(st.g, 'interface',
                                                                                         'bag_male.png'))))
        st.arrow(ground, 8 - 8, 72 - 8, left=True)
        st.arrow(ground, 72 - 8, 72 - 8, left=False)
        kx = st.key(ground, 160, 91, 'START')
        st.text(ground, kx + 2, 89, 'SORT', font='s')
        sp = U.Spec('bag')
        self.img(sp, 'fr.bag.ground', ground)
        self.text(sp, '{%d}' % U.F_TITLE, 44, 9, F.WHITE, align=2)
        # the picture for the pocket: the BAG open where item_menu_icons.c sAnims_Bag opens it (frame 1 the POKé
        # BALLS', 2 the ITEMS', 3 the KEY ITEMS'), the TM CASE's disc (tm_case/disc.png, NORMAL's colours) for TMs &
        # HMs, the BERRY POUCH (berry_pouch.png) for BERRIES; KAI's other pockets the ITEMS side
        disc = Image.open(os.path.join(st.g, 'tm_case', 'disc.png')).convert('L')
        dpal = F.jasc(os.path.join(st.g, 'tm_case', 'disc_types_1.pal'))
        discpic = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
        dp, dq = disc.load(), discpic.load()
        for yy in range(32):
            for xx in range(32):
                v = dp[xx, yy] // 17
                if v:
                    dq[xx, yy] = dpal[v][:3] + (255,)
        pouch = Image.open(os.path.join(st.g, 'berry_pouch', 'berry_pouch.png'))
        pouch = a._sheet('berry_pouch/berry_pouch.png', F.png_pal(pouch)).crop((0, 0, 64, 64))
        # (the BAG is the player's own: item_menu_icons.c gSpriteSheet_BagMale / BagFemale by gSaveBlock2 gender)
        frames = [2, 2, 1, None, None, 2, 3, 2]
        for sex, png in ((0, 'bag_male.png'), (1, 'bag_female.png')):
            sheet = a._sheet('interface/' + png, F.png_pal(Image.open(os.path.join(st.g, 'interface', png))))
            for i, fr_ in enumerate(frames):
                if fr_ is not None:
                    g = dict(cond=U.C_SEL0 << i | U.C_FEMALE) if sex else dict(cond=U.C_SEL0 << i,
                                                                               unless=U.C_FEMALE)
                    self.img(sp, 'fr.bag.pic%d.%d' % (sex, fr_), sheet.crop((0, fr_ * 64, 64, fr_ * 64 + 64)),
                             40 - 32, 68 - 32, **g)
        self.img(sp, 'fr.bag.disc', discpic, 40 - 16, 68 - 16, cond=U.C_SEL0 << 3)
        self.img(sp, 'fr.bag.pouch', pouch, 40 - 32, 68 - 32, cond=U.C_SEL0 << 4)
        for i in range(8):
            x = 8 + i * 9
            pip = Image.new('RGBA', (6, 6), (98, 98, 98, 255))
            on = pip.copy()
            pip.paste((148, 205, 205, 255), (1, 1, 5, 5))
            on.paste((255, 255, 255, 255), (1, 1, 5, 5))
            self.img(sp, 'fr.bag.pip', pip, x, 97, unless=U.C_SEL0 << i)
            self.img(sp, 'fr.bag.pip.on', on, x, 97, cond=U.C_SEL0 << i)
        chev = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
        st.arrow(chev, 0, 0, left=True)
        self.img(sp, 'fr.o.up', chev.transpose(Image.ROTATE_270), 152, 0, cond=U.S_UP)
        self.img(sp, 'fr.o.down', chev.transpose(Image.ROTATE_90), 136, 92, cond=U.S_DOWN)
        sp.add(U.OP_ICON, x=24, y=140, a=1)     # (item_menu_icons.c: the item's icon at 24, 140)
        sp.add(U.OP_MSG, x=40, y=115, a=0, h=14, n=3, w=196, font=self.fn, ink=F.WHITE[0], shadow=F.WHITE[1])
        sp.add(U.OP_LIST, x=0, y=10, h=16, n=5)
        p.spec(sp)
        row = U.Spec('bag.row')
        self.text(row, '▶', 89, 0, F.RED, cond=U.C_SEL0)
        self.text(row, '▶', 89, 0, F.STD, cond=U.B_SWAP, unless=U.C_SEL0)
        self.text(row, '{%d}' % U.F_NAME, 97, 0, F.STD, w=100, b=self.fs + 1)
        self.text(row, '×{%d}' % U.F_QTY2, 198, 0, F.STD, 's', cond=U.B_QTY)
        self.text(row, '{%d}' % U.F_TAG, 230, 0, F.STD, 's', cond=U.B_TAG, align=1)
        p.spec(row)

    # ---------------------------------------------------------------------------------------------------- pop-ups
    def popups(self):
        """KAI's own pop-ups (menus, YES / NO, the save's box) in FireRed's window frame and font (mp_uistyle.c
        PopUps): the red ones red, the pale ones grey"""
        st, p = self.st, self.p
        frame = Image.new('RGBA', (24, 24), (0, 0, 0, 0))
        st.frame(frame, (0, 0, 24, 24))
        sp = U.Spec('win')
        self.img(sp, 'fr.win', frame)
        sp.add(U.OP_FILL, w=0, h=0, ink=(255, 255, 255), a=255)
        p.spec(sp)
        t = U.Spec('win.text')
        self.text(t, '{%d}' % U.F_NAME, 0, 0, F.STD, unless=U.C_SEL0 | U.C_ALT)
        self.text(t, '{%d}' % U.F_NAME, 0, 0, F.RED, cond=U.C_SEL0)
        self.text(t, '{%d}' % U.F_NAME, 0, 0, ((172, 172, 172), (222, 222, 213)), cond=U.C_ALT)
        p.spec(t)

    # ---------------------------------------------------------------------------------------------------- summary
    TYPE_OFFSETS = [0x20, 0x64, 0x60, 0x80, 0x48, 0x44, 0x6C, 0x68, 0x88, 0xA4, 0x24, 0x28, 0x2C, 0x40, 0x84, 0x4C,
                    0xA0, 0x8C]   # (list_menu.c sMenuInfoIcons, KAI's TYPE_* order; FAIRY made below)
    CONTEST = [('COOL', (232, 64, 48)), ('BEAUTY', (64, 120, 232)), ('CUTE', (240, 120, 184)),
               ('SMART', (72, 176, 72)), ('TOUGH', (216, 176, 32))]

    def fr_icon(self, off, w=32):
        import os
        st = self.st
        sheet = Image.open(os.path.join(st.g, 'interface', 'menu_info.png'))
        pal = F.jasc(os.path.join(st.g, 'interface', 'pokemon_types.pal'))
        sx, sy = (off % 16) * 8, (off // 16) * 8
        im = Image.new('RGBA', (w, 12), (0, 0, 0, 0))
        sp, q = sheet.load(), im.load()
        for yy in range(12):
            for xx in range(w):
                v = sp[sx + xx, sy + yy] % 16
                if v:
                    q[xx, yy] = pal[v][:3] + (255,)
        return im

    def plate_icon(self, word, col, w=32):
        """a type icon's plate (NORMAL's, its letters cleared) in COL with WORD in white in FireRed's small font"""
        base = self.fr_icon(0x20, w)
        px = base.load()
        edge = px[0, 5]
        out = Image.new('RGBA', base.size, (0, 0, 0, 0))
        q = out.load()
        dark = tuple(max(0, c - 72) for c in col)
        for y in range(base.height):
            for x in range(base.width):
                if px[x, y][3] == 0:
                    continue
                inner = 1 <= x < base.width - 1 and 1 <= y < base.height - 1
                q[x, y] = (col if inner else dark) + (255,)
        tw = self.st.measure(word, 's')
        self.st.text(out, (w - tw) // 2, -2, word, (255, 255, 255), dark, font='s')
        return out

    def summary(self):
        """KAI's six summary pages in FireRed's: the header (its page dots widened to six, the page's title, ◀▶ PAGE
        and what A does), the picture panel; INFO as approved (fr.py summary_page); SKILLS on page_skills (HP, the
        five stats, nature's in red / blue, IV stars or EVs, EXP., NEXT LV., ABILITY); the moves on page_moves (type,
        name, PP), the move picked in the moves_info panel (POWER, ACCURACY, EFFECT; for contests APPEAL and JAM
        hearts); the POKé BALL and the JOURNAL in FireRed's window"""
        import os
        import struct
        st, p = self.st, self.p
        a = st.a
        bg = Image.open(os.path.join(st.g, 'summary_screen', 'bg.png'))
        pal = F.png_pal(bg)
        d = open(os.path.join(st.g, 'summary_screen', 'moves_info_page.bin'), 'rb').read()
        ent0 = list(struct.unpack('<%dH' % (len(d) // 2), d))
        base = 0x159

        def header_ents(ent, k):
            ent = list(ent)
            for row in (0, 1):
                band, blue = ent[row * 32], ent[row * 32 + 19]
                t = lambda n: base + n + 16 * row  # noqa: E731
                if k < 5:
                    new = [band] * 10 + [t(49)] * k + [t(17), t(16)] + [t(18)] * (5 - k) + [t(21)] + [blue] * 14
                else:
                    new = [band] * 10 + [t(49)] * 5 + [t(17), t(48)] + [blue] * 15
                ent[row * 32:row * 32 + 32] = new[:32]
            return ent

        def ground(k, moves_base=False, overlay=None, oh=20):
            im = Image.new('RGBA', (240, 160), pal[0][:3] + (255,))
            if moves_base:
                d2 = open(os.path.join(st.g, 'summary_screen', 'moves_page.bin'), 'rb').read()
                e2 = list(struct.unpack('<%dH' % (len(d2) // 2), d2))[:32 * 20]
                F.paste(im, a._tilemap('summary_screen/bg.png', None, pal, 32, 20, entries=header_ents(e2, k)), 0, 0)
            else:
                F.paste(im, a._tilemap('summary_screen/bg.png', None, pal, 32, 20, entries=header_ents(ent0, k)), 0, 0)
            for ov in overlay or []:
                F.paste(im, a._tilemap('summary_screen/bg.png', 'summary_screen/' + ov, pal, 32, oh), 0, 0)
            return im
        # the header's right: ◀▶ PAGE and what A does (one picture each), its words in white
        labels = ['CANCEL', 'IV RATING', 'EVs', 'STATS', 'INFO', 'SWITCH', 'MORE', '']
        first = None
        for i, lab in enumerate(labels):
            im = Image.new('RGBA', (240, 16), (0, 0, 0, 0))
            x = 236 - st.measure('PAGE', 's') - (st.measure(lab, 's') + 8 if lab else 0) - 8 - 4
            x = st.key(im, x, 2, 'LEFTRIGHT')
            x = st.text(im, x, 0, 'PAGE', *F.WHITE, font='s') + 4
            if lab:
                x = st.key(im, x, 2, 'A')
                st.text(im, x, 0, lab, *F.WHITE, font='s')
            k = p.image('fr.sum.prompt%d' % i, im)
            first = k if first is None else first
            assert k == first + i
        prompt = first
        # type icons (FAIRY a plate of its own) and the contest kinds
        tfirst = None
        for i, off in enumerate(self.TYPE_OFFSETS + [None]):
            ic = self.fr_icon(off) if off is not None else self.plate_icon('FAIRY', (232, 120, 200))
            k = p.image('fr.type%d' % i, ic)
            tfirst = k if tfirst is None else tfirst
            assert k == tfirst + i
        cfirst = None
        for i, (word, col) in enumerate(self.CONTEST):
            k = p.image('fr.contest%d' % i, self.plate_icon(word, col))
            cfirst = k if cfirst is None else cfirst
            assert k == cfirst + i
        ink = (pal[6 * 16 + 14][:3], pal[6 * 16 + 10][:3])
        if sum(ink[0]) > 500:
            ink = ((65, 65, 65), (222, 213, 180))
        red, blue = ((230, 8, 8), (255, 189, 115)), ((40, 96, 216), (164, 197, 246))
        rename = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        w = 24 + 2 + st.measure('RENAME', 's')
        kx = st.key(rename, 228 - w, 146, 'SELECT')
        st.text(rename, kx + 2, 144, 'RENAME', *ink, font='s')
        stars = []
        for n, col in ((0, None), (1, (65, 65, 65)), (2, (65, 65, 65)), (3, (65, 65, 65)), (3, (230, 8, 8))):
            im = Image.new('RGBA', (24, 8), (0, 0, 0, 0))
            shape = (0x10, 0x10, 0x38, 0xFE, 0x7C, 0x38, 0x6C, 0x44)
            for i in range(3):
                c = col if i < n else (190, 190, 180)
                for yy in range(8):
                    for xx in range(8):
                        if shape[yy] & (0x80 >> xx):
                            im.putpixel((i * 8 + xx, yy), c + (255,))
            stars.append(p.image('fr.stars%d%s' % (n, '' if col != (230, 8, 8) else 'r'), im))
        assert stars == list(range(stars[0], stars[0] + 5))

        def common(sp, k, pic=True, row=(4, 18, F.WHITE, F.MALE_W, F.FEMALE_W)):
            # (a title wider than the band before the page dots in the small font)
            self.text(sp, '{%d}' % U.F_PAGETITLE, 4, 1, F.WHITE, w=76, b=self.fs + 1)
            sp.add(U.OP_IMGSEL, x=0, y=0, a=prompt, b=U.F_PROMPT)
            if row is None:
                return
            x, y, inkr, male, female = row
            self.text(sp, '{LV}{%d}' % U.F_LEVEL, x, y, inkr, unless=U.SM_EGG)
            self.text(sp, '{%d}' % U.F_NAME, x + 36, y, inkr)
            self.text(sp, '♂', x + 101, y, male, cond=U.C_MALE)
            self.text(sp, '♀', x + 101, y, female, cond=U.C_FEMALE)
            if pic:
                sp.add(U.OP_ICON, x=60, y=64, a=0)
                sp.add(U.OP_ICON, x=104, y=88, a=2)

        # INFO
        sp = U.Spec('sum.0')
        self.img(sp, 'fr.sum.info', ground(0, overlay=['page_info.bin']))
        common(sp, 0)
        self.text(sp, '{%d}' % U.F_DEXNO, 167, 21, ink, unless=U.SM_EGG)
        self.text(sp, '{%d}' % U.F_SPECIES, 167, 35, ink, unless=U.SM_EGG)
        sp.add(U.OP_IMGSEL, x=167, y=52, a=tfirst, b=U.F_TYPE1, unless=U.SM_EGG)
        sp.add(U.OP_IMGSEL, x=201, y=52, a=tfirst, b=U.F_TYPE2, cond=U.SM_TWOTYPES, unless=U.SM_EGG)
        self.text(sp, '{%d}' % U.F_OT, 167, 65, ink, unless=U.SM_EGG)
        self.text(sp, '{%d}' % U.F_IDNO, 167, 80, ink, unless=U.SM_EGG)
        self.text(sp, '{%d}' % U.F_HELD, 167, 95, ink, w=238 - 167, b=self.fs + 1, unless=U.SM_EGG)
        self.text(sp, '{%d} nature.' % U.F_NATURE, 8, 115, ink, cond=U.SM_MEMOPLAIN)
        self.text(sp, 'Met in {%d} at {LV}{%d}.' % (U.F_METPLACE, U.F_METLV), 8, 129, ink, cond=U.SM_MEMOPLAIN,
                  w=224, b=self.fs + 1)
        sp.add(U.OP_MSG, x=8, y=115, a=2, h=14, n=2, w=224, b=10, c=3, d=114, font=self.fn, ink=ink[0],
               shadow=ink[1], unless=U.SM_MEMOPLAIN)
        sp.add(U.OP_MSG, x=167, y=21, a=0, h=14, n=5, w=70, e=self.fs + 1, font=self.fn, ink=ink[0], shadow=ink[1],
               cond=U.SM_EGG)
        self.img(sp, 'fr.sum.rename', rename, cond=U.SM_RENAME)
        p.spec(sp)
        # SKILLS
        sp = U.Spec('sum.1')
        self.img(sp, 'fr.sum.skills', ground(1, overlay=['page_skills.bin']))
        common(sp, 1)
        # (each number in the middle of its pale box: HP's x 165-237, the stats' 201-237, the boxes 11 rows tall
        # from y 21 / 39, 13 apart; the digits' 10 rows with their shadow one row in)
        self.text(sp, '{%d}/{%d}' % (U.F_HP, U.F_MAXHP), 201, 19, ink, align=2, unless=U.SM_IV | U.SM_EV)
        sp.add(U.OP_IMGSEL, x=212, y=24, a=stars[0], b=U.F_IV0, cond=U.SM_IV)
        self.text(sp, '{%d}' % U.F_EV0, 201, 19, ink, align=2, cond=U.SM_EV)
        for i in range(5):
            y = 38 + 13 * i
            self.text(sp, '{%d}' % (U.F_STAT0 + i), 219, y, red, align=2, cond=U.C_SEL0 << i,
                      unless=U.SM_IV | U.SM_EV)
            self.text(sp, '{%d}' % (U.F_STAT0 + i), 219, y, blue, align=2, cond=U.C_SEL0 << (5 + i),
                      unless=U.SM_IV | U.SM_EV)
            self.text(sp, '{%d}' % (U.F_STAT0 + i), 219, y, ink, align=2,
                      unless=U.SM_IV | U.SM_EV | (U.C_SEL0 << i) | (U.C_SEL0 << (5 + i)))
            sp.add(U.OP_IMGSEL, x=210, y=y + 5, a=stars[0], b=U.F_IV0 + 1 + i, cond=U.SM_IV)
            self.text(sp, '{%d}' % (U.F_EV0 + 1 + i), 219, y, ink, align=2, cond=U.SM_EV)
        # (EXP. POINTS on its band above the line at y 116, NEXT LV. under it: the label on the number's line)
        self.text(sp, '{%d}' % U.F_EXPPTS, 233, 103, ink, align=1)
        self.text(sp, 'NEXT LV.', 170, 117, ink, 's')
        self.text(sp, '{%d}' % U.F_NEXTLV, 233, 117, ink, align=1)
        self.text(sp, '{%d}' % U.F_ABILITY, 72, 130, ink, w=78, b=self.fs + 1)
        sp.add(U.OP_MSG, x=8, y=144, a=0, h=14, n=1, w=228, e=self.fs + 1, font=self.fn, ink=ink[0], shadow=ink[1])
        clear = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        kx = st.key(clear, 168, 132, 'START')
        st.text(clear, kx + 2, 130, 'CLEAR', *ink, font='s')
        self.img(sp, 'fr.sum.clear', clear, cond=U.SM_CLEAREV)
        p.spec(sp)
        # the moves pages (normal: the picture; choosing: the move's panel)
        for k, contest in ((2, False), (3, True)):
            sp = U.Spec('sum.%d' % k)
            self.img(sp, 'fr.sum.moves%d' % k, ground(k, overlay=['page_moves.bin'], oh=32), unless=U.SM_CHOOSING)
            info = ground(k, moves_base=True, overlay=['page_moves_info.bin'])
            mv = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
            F.paste(mv, a._tilemap('summary_screen/bg.png', 'summary_screen/page_moves.bin', pal, 32, 32), 0, 0)
            F.paste(info, mv.crop((120, 0, 240, 160)), 120, 0)
            if contest:
                # (APPEAL and JAM where POWER and ACCURACY are: their pills' words written again)
                # (the label pills drawn afresh as FireRed's are made: the word's letters, a dark outline round
                # them and the dark bar through their middle; the old words' pills covered first)
                pill = self.fr_icon(0xC0, 40)
                pp = pill.load()
                dark = next(pp[xx, yy] for yy in range(12) for xx in range(40) if pp[xx, yy][3]
                            and sum(pp[xx, yy][:3]) < 200)
                light = next(pp[xx, yy] for yy in range(12) for xx in range(40) if pp[xx, yy][3]
                             and sum(pp[xx, yy][:3]) >= 450)
                under = info.crop((6, 58, 46, 58 + 26))
                for word, y in (('APPEAL', 58), ('JAM', 72)):
                    # (what was under the old pill: the panel's own colour beside it)
                    bg = info.getpixel((5, y + 6))
                    info.paste(bg, (6, y, 46, y + 12))
                    letters = Image.new('RGBA', (40, 12), (0, 0, 0, 0))
                    st.text(letters, (40 - st.measure(word, 's')) // 2, -1, word, light[:3], None, font='s')
                    lp = letters.load()
                    pl = Image.new('RGBA', (40, 12), (0, 0, 0, 0))
                    px = pl.load()
                    for yy in range(4, 9):
                        for xx in range(0 if 5 <= yy <= 7 else 1, 40 if 5 <= yy <= 7 else 39):
                            px[xx, yy] = dark
                    for yy in range(12):
                        for xx in range(40):
                            if lp[xx, yy][3]:
                                for dy in (-1, 0, 1):
                                    for dx in (-1, 0, 1):
                                        if 0 <= xx + dx < 40 and 0 <= yy + dy < 12 and not lp[xx + dx, yy + dy][3]:
                                            px[xx + dx, yy + dy] = dark
                    for yy in range(12):
                        for xx in range(40):
                            if lp[xx, yy][3]:
                                px[xx, yy] = light
                    F.paste(info, pl, 6, y)
                    # (the value's white box made long enough for eight hearts: its own rows carried on to x 114)
                    ip = info.load()
                    for yy in range(y, y + 11):
                        ends = [xx for xx in range(46, 90) if ip[xx, yy][:3] == (246, 255, 255)]
                        if ends:
                            for xx in range(ends[-1] + 1, ends[-1] + 1 + (114 - 82)):
                                ip[xx, yy] = ip[ends[-1], yy]
            self.img(sp, 'fr.sum.moves%di' % k, info, cond=U.SM_CHOOSING)
            common(sp, k, pic=False)
            sp.add(U.OP_ICON, x=60, y=64, a=0, unless=U.SM_CHOOSING)
            sp.add(U.OP_ICON, x=104, y=88, a=2, unless=U.SM_CHOOSING)
            sp.add(U.OP_IMGSEL, x=8, y=34, a=tfirst, b=U.F_TYPE1, cond=U.SM_CHOOSING, unless=U.SM_EGG)
            sp.add(U.OP_IMGSEL, x=42, y=34, a=tfirst, b=U.F_TYPE2, cond=U.SM_CHOOSING | U.SM_TWOTYPES)
            if contest:
                for i, (fld, y) in enumerate(((U.F_APPEAL, 60), (U.F_JAM, 74))):
                    hearts = []
                    for n in range(9):
                        im = Image.new('RGBA', (64, 8), (0, 0, 0, 0))
                        for h in range(8):
                            col = (230, 8, 8) if h < n else (190, 190, 180)
                            for yy, row in enumerate((0x6C, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00)):
                                for xx in range(7):
                                    if row & (0x80 >> xx):
                                        im.putpixel((h * 8 + xx, yy + 1), col + (255,))
                        hearts.append(p.image('fr.hearts%d' % n, im))
                    sp.add(U.OP_IMGSEL, x=50, y=y, a=hearts[0], b=fld, cond=U.SM_CHOOSING)
            else:
                self.text(sp, '{%d}' % U.F_POWER, 80, 57, ink, align=1, cond=U.SM_CHOOSING)
                self.text(sp, '{%d}' % U.F_ACC, 80, 71, ink, align=1, cond=U.SM_CHOOSING)
            sp.add(U.OP_MSG, x=8, y=97, a=1, h=14, n=4, w=106, e=self.fs + 1, font=self.fn, ink=ink[0],
                   shadow=ink[1], cond=U.SM_CHOOSING)
            sp.add(U.OP_LIST, x=0, y=22, h=28, n=5)
            p.spec(sp)
            row = U.Spec('sum.%d.row' % k)
            cur = Image.open(os.path.join(st.g, 'summary_screen', 'move_selection_cursor_left.png')).convert('RGBA')
            cur2 = Image.open(os.path.join(st.g, 'summary_screen', 'move_selection_cursor_right.png')).convert('RGBA')
            frames = []
            for half in (0, 32):
                fr_ = Image.new('RGBA', (128, 32), (0, 0, 0, 0))
                fr_.alpha_composite(cur.crop((0, half, 64, half + 32)), (0, 0))
                fr_.alpha_composite(cur2.crop((0, half, 64, half + 32)), (64, 0))
                bb = fr_.getbbox()
                fr_ = fr_.crop(bb)
                # (cut to the box: 114 x 26 round it)
                w_, h_ = fr_.size
                outf = Image.new('RGBA', (114, h_), (0, 0, 0, 0))
                outf.alpha_composite(fr_.crop((0, 0, 57, h_)), (0, 0))
                outf.alpha_composite(fr_.crop((w_ - 57, 0, w_, h_)), (57, 0))
                frames.append(outf)
            self.img(row, 'fr.sum.cursor', frames[0], 126, -2, cond=U.C_SEL0)
            self.img(row, 'fr.sum.cursor2', frames[1], 126, -2, cond=U.B_SWAP, unless=U.C_SEL0)
            if contest:
                row.add(U.OP_IMGSEL, x=130, y=2, a=cfirst, b=U.F_CTYPE, unless=U.R_EMPTY | U.R_CANCEL)
            else:
                row.add(U.OP_IMGSEL, x=130, y=2, a=tfirst, b=U.F_TYPE, unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '{%d}' % U.F_NAME, 164, 0, ink, w=72, b=self.fs + 1, unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, 'PP', 164, 10, ink, 's', unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), 236, 10, ink, 's', align=1,
                      unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '-', 164, 0, ink, cond=U.R_EMPTY)
            self.text(row, 'CANCEL', 164, 4, ink, cond=U.R_CANCEL)
            p.spec(row)
        # the POKé BALL and the JOURNAL: FireRed's window under the header (the page's lines where KAI prints them
        # in its box; the POKéMON named in the window's free corner, or over the JOURNAL's rows)
        for k in (4, 5):
            sp = U.Spec('sum.%d' % k)
            g = ground(k)
            st.frame(g, (0, 16, 240, 160))
            self.img(sp, 'fr.sum.win%d' % k, g)
            if k == 4:
                common(sp, k, pic=False, row=None)
                self.text(sp, '{%d}' % U.F_NAME, 216, 26, F.STD, align=1)
                self.text(sp, '♂', 219, 26, F.MALE, cond=U.C_MALE)
                self.text(sp, '♀', 219, 26, F.FEMALE, cond=U.C_FEMALE)
                sp.add(U.OP_LIST, x=12, y=24, h=0, n=1)
            else:
                common(sp, k, pic=False, row=(12, 24, F.STD, F.MALE, F.FEMALE))
                sp.add(U.OP_MSG, x=12, y=40, a=3, h=14, n=7, w=216, e=self.fs + 1, font=self.fn, ink=F.STD[0],
                       shadow=F.STD[1])
                self.text(sp, '{%d}' % U.F_JPAGE, 228, 139, F.STD, 's', align=1, cond=U.S_DOWN)
            p.spec(sp)
            row = U.Spec('sum.%d.row' % k)
            if k == 5:
                row.add(U.OP_FILL, x=0, y=0, w=220, h=14, ink=(232, 240, 255), a=255, cond=U.R_NEW)
            self.text(row, '{%d}' % U.F_NAME, 2 if k == 5 else 0, -1 if k == 5 else 0, F.STD, w=216, b=self.fs + 1)
            p.spec(row)

    def make(self):
        self.fonts()
        for kind in ('foe', 'mine', 'foe2', 'mine2', 'foe3', 'mine3'):
            self.p.spec(self.hud(kind))
        self.panels()
        self.party()
        self.options()
        self.bag()
        self.popups()
        self.summary()
        return self.p
