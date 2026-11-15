"""PLATINUM and BLACK: the DS games' interface on one GBA screen. Their own fonts (dsfont.py), Platinum's own pictures
from the user's ROM where they are whole on their own (the player's healthbox: battle/graphic/pl_batt_obj 117 with
cell 116, palette 0; the battle's message bar: pl_batt_bg 0 / 1 / 171; the BAG: graphic/pl_bag_gra 11 / 14 / 12),
the rest drawn in each game's shapes and colours: Platinum's touch-screen buttons (FIGHT red, BAG yellow, RUN blue,
POKéMON green), Black's dark slanted bars and flat buttons. Two screens' worth on one: the touch screen's buttons go
beside the message, as on the GBA's one screen."""
import os
import sys

from PIL import Image, ImageDraw

import dsfont
from common import (ACTIONS, BAG_ITEMS, DOUBLE_FOES, DOUBLE_MINE, FOE, MESSAGE, MINE, MOVES, OPTION_ROWS, OPTION_TABS,
                    PARTY, POCKETS, PROMPT, START_MENU, SUMMARY, TRIPLE_FOES, TRIPLE_MINE, H, W, creature, hp_colour,
                    item_icon, paste, shot)

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'sinnoh'))
import nds  # noqa: E402
import build_library_tv as L  # noqa: E402
import build_scratch as B  # noqa: E402

TYPE_COLOURS = {'NORMAL': (168, 160, 128), 'FIRE': (232, 112, 56), 'FLYING': (152, 144, 232),
                'GROUND': (208, 176, 96), 'WATER': (96, 136, 232), 'GRASS': (112, 192, 80)}


def rgba(p):
    im = Image.new('RGBA', (p.w, p.h), (0, 0, 0, 0))
    px = im.load()
    for k, c in enumerate(p.px):
        if c & 0x8000:
            px[k % p.w, k // p.w] = ((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3, 255)
    return im


def shade(c, k):
    return tuple(max(0, min(255, int(v * k))) for v in c)


class DS:
    style = 'platinum'

    def __init__(self, kai, export):
        self.kai, self.export = kai, export

    # ---------------------------------------------------------------------------------------------- drawing
    def text(self, im, x, y, s, ink=None, shadow=None, align='l'):
        return dsfont.draw(im, self.font, x, y, s, ink or self.ink, shadow if shadow is not None else self.shadow,
                           align)

    def width(self, s):
        return dsfont.width(self.font, s)

    def rrect(self, im, box, fill, outline=None, r=3):
        ImageDraw.Draw(im).rounded_rectangle(box, r, fill=fill, outline=outline)

    def cursor(self, im, x, y, col=(232, 72, 56)):
        p = im.load()
        for k in range(5):
            for j in range(k, 9 - k):
                p[x + k, y + j] = col + (255,)

    def hp_bar(self, im, x, y, w, frac, back=(64, 72, 64)):
        im.paste(back + (255,), (x, y, x + w, y + 4))
        n = round((w - 2) * frac)
        col = hp_colour(frac, (88, 224, 112), (248, 208, 48), (240, 72, 64))
        im.paste(col + (255,), (x + 1, y + 1, x + 1 + n, y + 3))
        im.paste(shade(col, 0.75) + (255,), (x + 1, y + 2, x + 1 + n, y + 3))

    def sex(self, im, x, y, s):
        return self.text(im, x, y, '♂' if s == 'm' else '♀', (64, 128, 232) if s == 'm' else (232, 80, 112),
                         (200, 208, 224))

    def all(self):
        return {'message': self.message(), 'start_menu': self.start_menu(), 'battle_single': self.single(),
                'battle_moves': self.moves(), 'battle_double': self.double(), 'battle_triple': self.triple(),
                'party': self.party(), 'summary': self.summary(), 'bag': self.bag(),
                'options': self.options(self.style.upper())}


# ================================================================================================= PLATINUM
class Platinum(DS):
    style = 'platinum'
    ink, shadow = (72, 72, 80), (200, 200, 208)
    frame_out, frame_in = (88, 104, 136), (176, 192, 224)

    def __init__(self, rom, kai, export):
        super().__init__(kai, export)
        self.font = dsfont.Platinum(rom)
        r = nds.Rom(rom)
        obj = r.narc('battle/graphic/pl_batt_obj.narc')
        U = lambda i: L.unpack(obj[i])  # noqa: E731
        import cells as C
        box = C.cells(U(117), U(116), L.palette(U(0)))[0][0]
        bb = box.crop(box.getbbox())
        # (the status slot left blank, as with no status)
        white = bb.getpixel((60, 10))
        bb.paste(white, (12, 19, 31, 27))
        self.player_box = bb
        # the foe's and the doubles' / triples': Platinum's box without its HP numbers' and EXP rows
        short = bb.crop((0, 0, bb.width, 30))
        short.paste((40, 56, 104, 255), (10, 28, 120, 30))
        self.foe_box = self.short_box = short
        bgn = r.narc('battle/graphic/pl_batt_bg.narc')
        V = lambda i: L.unpack(bgn[i])  # noqa: E731
        self.msgbar = rgba(B.background(V(0), V(1), L.palette(V(171)))).crop((8, 144, 248, 192))
        bag = r.narc('graphic/pl_bag_gra.narc')
        G = lambda i: L.unpack(bag[i])  # noqa: E731
        self.bag_bg = rgba(B.background(G(11), G(14), L.palette(G(12))))
        self.bag_pic = C.cells(G(2), G(1), L.palette(G(3)))[0][0]   # (the boy's BAG)

    def window(self, im, x0, y0, x1, y1):
        """Platinum's window: white inside a two-tone rounded edge"""
        self.rrect(im, (x0, y0, x1, y1), (248, 248, 248, 255), self.frame_out + (255,), 3)
        ImageDraw.Draw(im).rounded_rectangle((x0 + 1, y0 + 1, x1 - 1, y1 - 1), 2, outline=self.frame_in + (255,))

    def message(self):
        im = shot(self.kai, 'f_mart')
        self.window(im, 2, 114, 237, 157)
        self.text(im, 12, 120, MESSAGE)
        p = im.load()
        for k in range(4):
            for j in range(k, 7 - k):
                p[224 + j, 144 + k] = (232, 72, 56, 255)
        return im

    def start_menu(self):
        im = shot(self.kai, 'f_town')
        self.window(im, 158, 2, 237, 6 + 16 * len(START_MENU) + 4)
        for i, s in enumerate(START_MENU):
            if i == 0:
                self.rrect(im, (162, 6 + 16 * i, 233, 6 + 16 * i + 15), (248, 216, 152, 255), None, 2)
            self.text(im, 176, 6 + 16 * i, s)
        self.cursor(im, 166, 10)
        return im

    # the HUD
    def foe(self, im, x, y, m):
        paste(im, self.foe_box, x, y)
        self.text(im, x + 18, y + 1, m['name'])
        self.sex(im, x + 18 + self.width(m['name']), y + 1, m['sex'])
        self.text(im, x + 112, y + 1, 'Lv%d' % m['level'], align='r')
        self.slot(im, x, y, m['hp'])

    def slot(self, im, x, y, frac):
        """the HP in the box's own bar (white, 61 x 3 at 47, 22)"""
        n = round(61 * frac)
        col = hp_colour(frac, (88, 224, 112), (248, 208, 48), (240, 72, 64))
        im.paste(col + (255,), (x + 47, y + 22, x + 47 + n, y + 25))
        im.paste(shade(col, 0.75) + (255,), (x + 47, y + 24, x + 47 + n, y + 25))

    def mine(self, im, x, y, m, compact=False):
        paste(im, self.short_box if compact else self.player_box, x, y)
        self.text(im, x + 18, y + 1, m['name'])
        self.sex(im, x + 18 + self.width(m['name']), y + 1, m['sex'])
        self.text(im, x + 112, y + 1, 'Lv%d' % m['level'], align='r')
        self.slot(im, x, y, m['hp'] / m['maxhp'])
        if not compact:
            self.text(im, x + 110, y + 22, '%d/%d' % (m['hp'], m['maxhp']), align='r')

    def buttons(self, im):
        """the touch screen's command on the GBA's screen: the message bar left, Platinum's four buttons right"""
        paste(im, self.msgbar, 0, 112)
        self.text(im, 10, 118, PROMPT[0], (248, 248, 248), (64, 72, 112))
        self.text(im, 10, 134, PROMPT[1], (248, 248, 248), (64, 72, 112))
        im.paste((40, 48, 64, 255), (132, 112, 240, 160))
        specs = [('FIGHT', (232, 64, 56), (136, 114, 236, 136)), ('BAG', (232, 184, 48), (134, 139, 162, 158)),
                 ('RUN', (72, 112, 224), (165, 139, 192, 158)), ('MOéMON', (88, 176, 72), (195, 139, 238, 158))]
        for name, col, box in specs:
            self.rrect(im, box, shade(col, 0.6) + (255,), None, 4)
            self.rrect(im, (box[0] + 1, box[1] + 1, box[2] - 1, box[3] - 2), col + (255,), None, 4)
            ImageDraw.Draw(im).line((box[0] + 4, box[1] + 2, box[2] - 4, box[1] + 2), fill=shade(col, 1.3) + (255,))
            cx = (box[0] + box[2]) // 2
            self.text(im, cx, box[1] + (5 if name == 'FIGHT' else 2), name,
                      (248, 248, 248), shade(col, 0.5), 'c')

    def single(self):
        im = shot(self.kai, 's_single')
        self.foe(im, 0, 14, FOE)
        self.mine(im, 120, 70, MINE)
        self.buttons(im)
        return im

    def moves(self):
        im = shot(self.kai, 's_single')
        self.foe(im, 0, 14, FOE)
        self.mine(im, 120, 70, MINE)
        im.paste((40, 48, 64, 255), (0, 110, 240, 160))
        for i, (n, pp, mx, kind) in enumerate(MOVES):
            col = TYPE_COLOURS.get(kind, (160, 160, 160))
            x0, y0 = 2 + (i % 2) * 119, 112 + (i // 2) * 24
            box = (x0, y0, x0 + 116, y0 + 22)
            self.rrect(im, box, shade(col, 0.55) + (255,), None, 4)
            self.rrect(im, (box[0] + 1, box[1] + 1, box[2] - 1, box[3] - 1), col + (255,), None, 4)
            self.rrect(im, (box[0] + 4, box[1] + 3, box[2] - 4, box[1] + 12), shade(col, 1.25) + (255,), None, 3)
            self.text(im, x0 + 6, y0 + 3, n, (248, 248, 248), shade(col, 0.45))
            self.text(im, x0 + 111, y0 + 3, '%d/%d' % (pp, mx), (248, 248, 248), shade(col, 0.45), 'r')
        return im

    def double(self):
        im = shot(self.kai, 's_double')
        for i, m in enumerate(DOUBLE_FOES):
            self.foe(im, 0 + i * 6, 2 + i * 28, m)
        for i, m in enumerate(DOUBLE_MINE):
            self.mine(im, 120 + i * 6, 54 + i * 28, m, compact=True)
        self.buttons(im)
        return im

    def triple(self):
        im = shot(self.kai, 's_triple')
        for i, m in enumerate(TRIPLE_FOES):
            self.foe(im, 0, 0 + i * 26, m)
        for i, m in enumerate(TRIPLE_MINE):
            self.mine(im, 120, 32 + i * 26, m, compact=True)
        self.buttons(im)
        return im

    def party(self):
        im = Image.new('RGBA', (W, H), (64, 152, 136, 255))
        d = ImageDraw.Draw(im)
        for k in range(-160, 240, 12):
            d.line((k, 0, k + 160, 160), fill=(80, 168, 152, 255), width=4)
        for i in range(6):
            x0, y0 = 4 + (i % 2) * 118, 4 + (i // 2) * 40
            sel = i == 0
            col = (248, 168, 96) if sel else (112, 168, 224) if i < len(PARTY) else (96, 120, 136)
            self.rrect(im, (x0, y0, x0 + 113, y0 + 36), shade(col, 0.55) + (255,), None, 5)
            self.rrect(im, (x0 + 1, y0 + 1, x0 + 112, y0 + 35), col + (255,), None, 5)
            if i < len(PARTY):
                m = PARTY[i]
                paste(im, creature(self.export, m['species'], 'icon'), x0, y0 - 2)
                self.text(im, x0 + 32, y0 + 1, m['name'], (248, 248, 248), shade(col, 0.5))
                self.sex(im, x0 + 32 + self.width(m['name']), y0 + 1, m['sex'])
                self.text(im, x0 + 6, y0 + 20, 'Lv%d' % m['level'], (248, 248, 248), shade(col, 0.5))
                self.hp_bar(im, x0 + 40, y0 + 18, 48, 1.0)
                self.text(im, x0 + 108, y0 + 20, '%d/%d' % (m['hp'], m['maxhp']), (248, 248, 248), shade(col, 0.5),
                          'r')
        self.window(im, 2, 126, 168, 157)
        self.text(im, 10, 133, 'Choose a MOéMON.')
        self.rrect(im, (172, 128, 237, 157), (72, 112, 224, 255), (32, 56, 120, 255), 5)
        self.text(im, 204, 135, 'CANCEL', (248, 248, 248), (32, 56, 120), 'c')
        return im

    def summary(self):
        s = SUMMARY
        im = Image.new('RGBA', (W, H), (232, 240, 248, 255))
        d = ImageDraw.Draw(im)
        # (Platinum's header: the page's name and its tabs)
        im.paste((72, 96, 160, 255), (0, 0, 240, 16))
        self.text(im, 6, 0, 'MOéMON INFO', (248, 248, 248), (40, 48, 96))
        for k, c in enumerate([(232, 96, 88), (240, 176, 64), (96, 192, 96), (88, 152, 232), (176, 112, 216)]):
            self.rrect(im, (146 + k * 18, 3, 160 + k * 18, 13), c + (255,), (248, 248, 248, 255) if k == 0 else None, 3)
        # (the picture on its lit panel)
        for y in range(16, 160):
            d.line((0, y, 104, y), fill=(200 - y // 4, 224 - y // 6, 240, 255))
        paste(im, creature(self.export, 280), 20, 44)
        self.text(im, 6, 20, s['name'])
        self.sex(im, 6 + self.width(s['name']), 20, s['sex'])
        self.text(im, 6, 34, 'Lv%d' % s['level'])
        rows = [('DEX NO.', s['no']), ('NAME', s['name']), ('TYPE', s['type']), ('OT', s['ot']), ('ID NO.', s['idno']),
                ('ITEM', s['item'])]
        for i, (k, v) in enumerate(rows):
            y = 20 + i * 18
            self.rrect(im, (108, y, 236, y + 16), (248, 248, 248, 255), (176, 192, 224, 255), 3)
            self.rrect(im, (108, y, 160, y + 16), (96, 120, 184, 255), None, 3)
            self.text(im, 112, y, k, (248, 248, 248), (48, 64, 120))
            self.text(im, 166, y, v)
        self.window(im, 106, 130, 237, 157)
        self.text(im, 112, 132, s['nature'] + ' nature.')
        self.text(im, 112, 144, s['met'][:-1] + '.')
        return im

    def bag(self):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        paste(im, self.bag_bg, -8, -16)
        pic = self.bag_pic.crop(self.bag_pic.getbbox())
        paste(im, pic, 52 - pic.width // 2, 70 - pic.height)
        for i, (name, n, _item) in enumerate(BAG_ITEMS[:5]):
            y = 6 + i * 18
            self.rrect(im, (130, y, 230, y + 16), (248, 248, 248, 255) if i else (248, 224, 160, 255),
                       (128, 136, 152, 255), 3)
            self.text(im, 136, y, name)
            self.text(im, 226, y, 'x%d' % n, align='r')
        self.text(im, 52, 92, POCKETS[0], (248, 248, 248), (88, 72, 40), 'c')
        ic = item_icon(self.export, 13)
        if ic:
            paste(im, ic, 4, 126)
        self.text(im, 36, 124, 'A spray-type medicine. It', (248, 248, 248), (64, 48, 152))
        self.text(im, 36, 140, 'restores HP by 20 points.', (248, 248, 248), (64, 48, 152))
        return im

    def options(self, chosen):
        im = Image.new('RGBA', (W, H), (120, 168, 200, 255))
        d = ImageDraw.Draw(im)
        for y in range(0, H, 8):
            d.line((0, y, W, y), fill=(128, 176, 208, 255))
        self.window(im, 2, 2, 237, 22)
        x = 10
        for t in OPTION_TABS:
            x = self.text(im, x, 4, t, (232, 72, 56) if t == 'DISPLAY' else None) + 8
        self.window(im, 2, 26, 237, 157)
        for i, (name, vals) in enumerate(OPTION_ROWS):
            y = 32 + i * 18
            if i == 0:
                self.rrect(im, (6, y, 233, y + 16), (248, 224, 160, 255), None, 3)
            self.text(im, 16, y, name)
            if vals is None:
                self.text(im, 180, y, chosen, (232, 72, 56), align='c')
            else:
                xx = 128
                for k, v in enumerate(vals):
                    xx = self.text(im, xx, y, v, (232, 72, 56) if k == len(vals) - 1 else None) + 10
        return im


# ================================================================================================= BLACK
class Black(DS):
    style = 'black'
    ink, shadow = (64, 64, 72), (192, 192, 200)
    bar, bar_light = (56, 60, 72), (104, 112, 128)

    def __init__(self, rom, kai, export):
        super().__init__(kai, export)
        self.font = dsfont.black(rom)

    def window(self, im, x0, y0, x1, y1, fill=(248, 248, 248)):
        """Black's window: white, a thin grey edge, its foot shaded"""
        self.rrect(im, (x0, y0, x1, y1), fill + (255,), (128, 136, 144, 255), 4)
        ImageDraw.Draw(im).line((x0 + 4, y1 - 1, x1 - 4, y1 - 1), fill=(208, 212, 220, 255))

    def message(self):
        im = shot(self.kai, 'f_mart')
        self.window(im, 2, 114, 237, 157)
        self.text(im, 12, 120, MESSAGE)
        p = im.load()
        for k in range(4):
            for j in range(k, 7 - k):
                p[224 + j, 144 + k] = (64, 64, 72, 255)
        return im

    def start_menu(self):
        """Black's menu: rounded tiles in a grid (its touch screen's), the chosen one lit"""
        im = shot(self.kai, 'f_town')
        im.alpha_composite(Image.new('RGBA', (W, H), (16, 24, 40, 120)))
        cols = [(232, 88, 72), (232, 72, 72), (232, 168, 56), (88, 152, 232), (96, 184, 88), (136, 120, 200),
                (128, 128, 136)]
        for i, s in enumerate(START_MENU):
            x0, y0 = 14 + (i % 3) * 72, 12 + (i // 3) * 46
            col = cols[i]
            self.rrect(im, (x0, y0, x0 + 66, y0 + 40), (40, 44, 56, 255), None, 6)
            self.rrect(im, (x0 + 2, y0 + 2, x0 + 64, y0 + 38), (248, 248, 248, 255) if i == 0 else (216, 220, 228, 255),
                       None, 5)
            ImageDraw.Draw(im).ellipse((x0 + 25, y0 + 5, x0 + 41, y0 + 21), fill=col + (255,),
                                       outline=shade(col, 0.6) + (255,))
            self.text(im, x0 + 33, y0 + 22, s, align='c')
        return im

    # the HUD: Gen 5's dark slanted bars
    def slant(self, im, x0, y0, x1, y1, right=True):
        d = ImageDraw.Draw(im)
        if right:
            d.polygon([(x0, y0), (x1, y0), (x1 - 8, y1), (x0, y1)], fill=self.bar + (255,))
            d.line((x0, y0, x1, y0), fill=self.bar_light + (255,))
        else:
            d.polygon([(x0 + 8, y0), (x1, y0), (x1, y1), (x0, y1)], fill=self.bar + (255,))
            d.line((x0 + 8, y0, x1, y0), fill=self.bar_light + (255,))

    def foe(self, im, x, y, m):
        self.slant(im, x, y, x + 116, y + 24, True)
        self.text(im, x + 4, y + 1, m['name'], (248, 248, 248), (24, 24, 32))
        self.sex(im, x + 4 + self.width(m['name']), y + 1, m['sex'])
        self.text(im, x + 102, y + 1, 'Lv%d' % m['level'], (248, 248, 248), (24, 24, 32), 'r')
        self.hp_bar(im, x + 40, y + 17, 60, m['hp'], (24, 28, 32))

    def mine(self, im, x, y, m, compact=False):
        self.slant(im, x, y, x + 120, y + (24 if compact else 34), False)
        self.text(im, x + 14, y + 1, m['name'], (248, 248, 248), (24, 24, 32))
        self.sex(im, x + 14 + self.width(m['name']), y + 1, m['sex'])
        self.text(im, x + 116, y + 1, 'Lv%d' % m['level'], (248, 248, 248), (24, 24, 32), 'r')
        self.hp_bar(im, x + 52, y + 17, 64, m['hp'] / m['maxhp'], (24, 28, 32))
        if not compact:
            self.text(im, x + 116, y + 20, '%d/%d' % (m['hp'], m['maxhp']), (248, 248, 248), (24, 24, 32), 'r')
            im.paste((72, 160, 232, 255), (x + 14, y + 33, x + 14 + int(102 * m['exp']), y + 34))

    def buttons(self, im):
        self.window(im, 2, 114, 128, 157)
        self.text(im, 10, 118, PROMPT[0])
        self.text(im, 10, 134, PROMPT[1])
        specs = [('FIGHT', (224, 64, 64), (132, 113, 236, 135)), ('BAG', (232, 176, 40), (132, 138, 160, 158)),
                 ('RUN', (64, 120, 232), (163, 142, 190, 158)), ('MOéMON', (72, 168, 88), (193, 138, 238, 158))]
        for name, col, box in specs:
            self.rrect(im, box, (32, 36, 44, 255), None, 6)
            self.rrect(im, (box[0] + 1, box[1] + 1, box[2] - 1, box[3] - 1), col + (255,), None, 6)
            self.rrect(im, (box[0] + 2, box[1] + 2, box[2] - 2, (box[1] + box[3]) // 2), shade(col, 1.18) + (255,),
                       None, 5)
            self.text(im, (box[0] + box[2]) // 2, box[1] + (4 if name == 'FIGHT' else 2),
                      name, (248, 248, 248), shade(col, 0.45), 'c')

    def single(self):
        im = shot(self.kai, 's_single')
        self.foe(im, 0, 16, FOE)
        self.mine(im, 120, 72, MINE)
        self.buttons(im)
        return im

    def moves(self):
        im = shot(self.kai, 's_single')
        self.foe(im, 0, 16, FOE)
        self.mine(im, 120, 72, MINE)
        for i, (n, pp, mx, kind) in enumerate(MOVES):
            col = TYPE_COLOURS.get(kind, (160, 160, 160))
            x0, y0 = 2 + (i % 2) * 119, 112 + (i // 2) * 24
            self.rrect(im, (x0, y0, x0 + 116, y0 + 22), (32, 36, 44, 255), None, 6)
            self.rrect(im, (x0 + 1, y0 + 1, x0 + 115, y0 + 21), (240, 240, 244, 255), None, 6)
            self.rrect(im, (x0 + 3, y0 + 3, x0 + 30, y0 + 19), col + (255,), None, 4)
            self.text(im, x0 + 16, y0 + 3, kind[:3], (248, 248, 248), shade(col, 0.45), 'c')
            self.text(im, x0 + 33, y0 + 3, n)
            self.text(im, x0 + 113, y0 + 3, '%d' % pp, align='r')
        return im

    def double(self):
        im = shot(self.kai, 's_double')
        for i, m in enumerate(DOUBLE_FOES):
            self.foe(im, 0, 2 + i * 28, m)
        for i, m in enumerate(DOUBLE_MINE):
            self.mine(im, 120, 56 + i * 28, m, compact=True)
        self.buttons(im)
        return im

    def triple(self):
        im = shot(self.kai, 's_triple')
        for i, m in enumerate(TRIPLE_FOES):
            self.foe(im, 0, 0 + i * 26, m)
        for i, m in enumerate(TRIPLE_MINE):
            self.mine(im, 120, 32 + i * 26, m, compact=True)
        self.buttons(im)
        return im

    def party(self):
        im = Image.new('RGBA', (W, H), (40, 44, 56, 255))
        d = ImageDraw.Draw(im)
        for y in range(0, H, 4):
            d.line((0, y, W, y), fill=(48, 52, 64, 255))
        for i in range(6):
            x0, y0 = 4 + (i % 2) * 118, 4 + (i // 2) * 40
            sel = i == 0
            self.rrect(im, (x0, y0, x0 + 113, y0 + 36), (24, 28, 36, 255), None, 6)
            self.rrect(im, (x0 + 1, y0 + 1, x0 + 112, y0 + 35),
                       (88, 168, 232, 255) if sel else (232, 236, 244, 255) if i < len(PARTY) else (96, 104, 120, 255),
                       None, 6)
            if i < len(PARTY):
                m = PARTY[i]
                paste(im, creature(self.export, m['species'], 'icon'), x0, y0 - 2)
                ink = (248, 248, 248) if sel else None
                shd = (40, 80, 128) if sel else None
                self.text(im, x0 + 32, y0 + 1, m['name'], ink, shd)
                self.sex(im, x0 + 32 + self.width(m['name']), y0 + 1, m['sex'])
                self.text(im, x0 + 6, y0 + 20, 'Lv%d' % m['level'], ink, shd)
                self.hp_bar(im, x0 + 40, y0 + 18, 48, 1.0, (24, 28, 32))
                self.text(im, x0 + 108, y0 + 20, '%d/%d' % (m['hp'], m['maxhp']), ink, shd, 'r')
        self.window(im, 2, 126, 168, 157)
        self.text(im, 10, 133, 'Choose a MOéMON.')
        self.rrect(im, (172, 128, 237, 157), (24, 28, 36, 255), None, 6)
        self.rrect(im, (173, 129, 236, 156), (224, 64, 64, 255), None, 6)
        self.text(im, 204, 135, 'CANCEL', (248, 248, 248), (120, 32, 32), 'c')
        return im

    def summary(self):
        s = SUMMARY
        im = Image.new('RGBA', (W, H), (40, 44, 56, 255))
        d = ImageDraw.Draw(im)
        for y in range(H):
            d.line((0, y, 104, y), fill=(56 + y // 6, 96 + y // 5, 160 + y // 4, 255))
        im.paste((24, 28, 36, 255), (0, 0, 240, 16))
        self.text(im, 6, 0, 'SUMMARY', (248, 248, 248), (0, 0, 0))
        for k in range(4):
            self.rrect(im, (160 + k * 19, 3, 175 + k * 19, 13), (88, 168, 232, 255) if k == 0 else (96, 104, 120, 255),
                       None, 3)
        paste(im, creature(self.export, 280), 20, 44)
        self.text(im, 6, 20, s['name'], (248, 248, 248), (24, 40, 80))
        self.sex(im, 6 + self.width(s['name']), 20, s['sex'])
        self.text(im, 6, 34, 'Lv%d' % s['level'], (248, 248, 248), (24, 40, 80))
        rows = [('Dex No.', s['no']), ('Name', s['name']), ('Type', s['type']), ('OT', s['ot']), ('ID No.', s['idno']),
                ('Item', s['item'])]
        for i, (k, v) in enumerate(rows):
            y = 20 + i * 18
            self.rrect(im, (108, y, 236, y + 16), (232, 236, 244, 255), None, 4)
            self.text(im, 112, y, k, (112, 120, 136), (208, 212, 220))
            self.text(im, 162, y, v)
        self.window(im, 106, 130, 237, 157)
        self.text(im, 112, 132, s['nature'] + ' nature.')
        self.text(im, 112, 144, s['met'][:-1] + '.')
        return im

    def bag(self):
        im = Image.new('RGBA', (W, H), (40, 44, 56, 255))
        d = ImageDraw.Draw(im)
        # (Black's BAG: the pockets as a ring of round buttons, the list on the right)
        d.ellipse((12, 18, 104, 110), fill=(56, 60, 76, 255), outline=(96, 104, 120, 255))
        import math
        for k in range(8):
            a = -math.pi / 2 + k * math.pi / 4
            cx, cy = 58 + int(36 * math.cos(a)), 64 + int(36 * math.sin(a))
            d.ellipse((cx - 9, cy - 9, cx + 9, cy + 9), fill=(232, 176, 40, 255) if k == 0 else (200, 204, 216, 255),
                      outline=(24, 28, 36, 255))
        self.text(im, 58, 58, POCKETS[0], (248, 248, 248), (0, 0, 0), 'c')
        for i, (name, n, _item) in enumerate(BAG_ITEMS[:5]):
            y = 6 + i * 20
            self.rrect(im, (116, y, 236, y + 17), (232, 176, 40, 255) if i == 0 else (232, 236, 244, 255), None, 5)
            self.text(im, 122, y + 1, name)
            self.text(im, 232, y + 1, 'x%d' % n, align='r')
        self.window(im, 2, 120, 237, 157)
        ic = item_icon(self.export, 13)
        if ic:
            paste(im, ic, 6, 124)
        self.text(im, 38, 122, 'A spray-type medicine. It')
        self.text(im, 38, 138, 'restores HP by 20 points.')
        return im

    def options(self, chosen):
        im = Image.new('RGBA', (W, H), (40, 44, 56, 255))
        im.paste((24, 28, 36, 255), (0, 0, 240, 20))
        x = 8
        for t in OPTION_TABS:
            sel = t == 'DISPLAY'
            w = self.width(t)
            if sel:
                self.rrect(im, (x - 3, 3, x + w + 1, 17), (88, 168, 232, 255), None, 5)
            x = self.text(im, x, 3, t, (248, 248, 248), (0, 0, 0)) + 6
        for i, (name, vals) in enumerate(OPTION_ROWS):
            y = 26 + i * 21
            sel = i == 0
            self.rrect(im, (4, y, 235, y + 18), (88, 168, 232, 255) if sel else (232, 236, 244, 255), None, 6)
            self.text(im, 12, y + 1, name, (248, 248, 248) if sel else None, (40, 80, 128) if sel else None)
            if vals is None:
                self.text(im, 180, y + 1, chosen, (248, 248, 248), (40, 80, 128), 'c')
            else:
                xx = 128
                for k, v in enumerate(vals):
                    xx = self.text(im, xx, y + 1, v, (224, 64, 64) if k == len(vals) - 1 else None) + 10
        return im
