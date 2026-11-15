"""FIRERED: FireRed's interface from its own graphics (pret/pokefirered graphics/: text_window/std.png and its palette,
battle_interface/textbox.png + .bin, the healthboxes, party_menu/bg + its slots and box palettes, item_menu/bg and the
BAG's picture, summary_screen/bg + its pages; the font graphics/fonts/latin_normal.png with its widths from src/text.c)
laid out as FireRed lays them, KAI's functions in their places."""
import os
import re
import struct

from PIL import Image

from common import (ACTIONS, BAG_ITEMS, DOUBLE_FOES, DOUBLE_MINE, FOE, MESSAGE, MINE, MOVES, OPTION_ROWS, OPTION_TABS,
                    PARTY, POCKETS, PROMPT, START_MENU, SUMMARY, TRIPLE_FOES, TRIPLE_MINE, H, W, creature, hp_colour,
                    item_icon, paste, shot)
import sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'stadium', 'restyle'))
import s2  # noqa: E402  (the game's character codes)

DARK, SHADOW = (80, 80, 88), (208, 208, 200)
WHITE_INK, WHITE_SHADOW = (248, 248, 248), (104, 88, 112)


def jasc(path):
    v = open(path).read().split()
    return [(int(v[3 + i * 3]), int(v[4 + i * 3]), int(v[5 + i * 3])) for i in range(int(v[2]))]


def png_pal(im):
    p = im.getpalette()
    return [tuple(p[i * 3:i * 3 + 3]) for i in range(len(p) // 3)]


class FireRed:
    def __init__(self, root, kai, export):
        self.g = os.path.join(root, 'graphics')
        self.kai, self.export = kai, export
        # the font: 16 x 16 cells, 16 a row, by the game's character codes
        self.glyphs = Image.open(os.path.join(self.g, 'fonts', 'latin_normal.png'))
        src = open(os.path.join(root, 'src', 'text.c')).read()
        body = re.search(r'sFontNormalLatinGlyphWidths\[\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
        self.widths = [int(v) for v in re.findall(r'\d+', re.sub(r'//.*', '', body))]
        self.std = self._sheet('text_window/std.png', jasc(os.path.join(self.g, 'text_window', 'stdpal_0.pal')))
        self.hbpal = jasc(os.path.join(self.g, 'battle_interface', 'healthbox.pal'))
        self.textbox = self._tilemap('battle_interface/textbox.png', 'battle_interface/textbox.bin',
                                     jasc(os.path.join(self.g, 'battle_interface', 'textbox1.pal')), 32, 64)

    # ---------------------------------------------------------------------------------------------- pictures
    def _sheet(self, rel, pal):
        im = Image.open(os.path.join(self.g, rel))
        out = Image.new('RGBA', im.size, (0, 0, 0, 0))
        p, q = im.load(), out.load()
        for y in range(im.height):
            for x in range(im.width):
                if p[x, y] % 16:
                    q[x, y] = pal[p[x, y] % len(pal)] + (255,)
        return out

    def _tiles(self, im, pal):
        """an indexed picture's 8 x 8 tiles in order, coloured by PAL (the index's low four bits)"""
        p, out = im.load(), []
        for ty in range(im.height // 8):
            for tx in range(im.width // 8):
                t = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
                q = t.load()
                for y in range(8):
                    for x in range(8):
                        v = p[tx * 8 + x, ty * 8 + y] % 16
                        if v:
                            q[x, y] = pal[v] + (255,)
                out.append(t)
        return out

    def _tilemap(self, png, binpath, pal, w, h, entries=None):
        im = Image.open(os.path.join(self.g, png))
        pal = pal or png_pal(im)
        px, tw = im.load(), im.width // 8
        if entries is None:
            d = open(os.path.join(self.g, binpath), 'rb').read()
            entries = struct.unpack('<%dH' % (len(d) // 2), d)
        out = Image.new('RGBA', (w * 8, h * 8), (0, 0, 0, 0))
        q = out.load()
        for k, e in enumerate(entries):
            tx, ty = k % w, k // w
            if ty >= h:
                break
            t, hf, vf, pn = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1, e >> 12
            sx, sy = (t % tw) * 8, (t // tw) * 8
            if sy >= im.height:
                continue
            for y in range(8):
                for x in range(8):
                    v = px[sx + x, sy + y] & 15
                    if v:
                        q[tx * 8 + (7 - x if hf else x), ty * 8 + (7 - y if vf else y)] = \
                            pal[(pn * 16 + v) % len(pal)] + (255,)
        return out

    def healthbox(self, kind):
        """the healthbox sprites put together as the game's OAMs are (64 x 64 + 64 x 64, or 64 x 32 + 64 x 32)"""
        name = {'player': 'healthbox_singles_player', 'foe': 'healthbox_singles_opponent',
                'pdouble': 'healthbox_doubles_player', 'fdouble': 'healthbox_doubles_opponent'}[kind]
        ts = self._tiles(Image.open(os.path.join(self.g, 'battle_interface', name + '.png')), self.hbpal)
        side = 8 if kind == 'player' else 4
        out = Image.new('RGBA', (128, side * 8), (0, 0, 0, 0))
        for half in range(2):
            for i in range(8 * side):
                out.alpha_composite(ts[half * 8 * side + i], (half * 64 + (i % 8) * 8, (i // 8) * 8))
        return out

    # ---------------------------------------------------------------------------------------------- words
    def measure(self, s):
        return sum(self.widths[s2._code(c)] for c in s)

    def text(self, im, x, y, s, ink=DARK, shadow=SHADOW, align='l'):
        if align == 'r':
            x -= self.measure(s)
        elif align == 'c':
            x -= self.measure(s) // 2
        p, gp = im.load(), self.glyphs.load()
        for c in s:
            k = s2._code(c)
            gx, gy = (k % 16) * 16, (k // 16) * 16
            for yy in range(16):
                for xx in range(self.widths[k]):
                    v = gp[gx + xx, gy + yy]
                    X, Y = x + xx, y + yy
                    if 0 <= X < W and 0 <= Y < H and v in (1, 2):
                        col = ink if v == 1 else shadow
                        if col:
                            p[X, Y] = col + (255,)
            x += self.widths[k]
        return x

    def window(self, im, x, y, w, h):
        """the standard window: FireRed's frame (3 x 3 tiles) round W x H tiles of white"""
        im.paste((255, 255, 255, 255), (x + 8, y + 8, x + 8 + w * 8, y + 8 + h * 8))
        for ty in range(h + 2):
            for tx in range(w + 2):
                sx = 0 if tx == 0 else 16 if tx == w + 1 else 8
                sy = 0 if ty == 0 else 16 if ty == h + 1 else 8
                if (sx, sy) == (8, 8):
                    continue
                im.alpha_composite(self.std.crop((sx, sy, sx + 8, sy + 8)), (x + tx * 8, y + ty * 8))

    def arrow(self, im, x, y, col=DARK):
        p = im.load()
        for k in range(5):
            for j in range(k, 9 - k):
                p[x + k, y + j] = col + (255,)

    def hp_bar(self, im, x, y, frac, w=48):
        im.paste((72, 72, 72, 255), (x, y, x + w + 2, y + 4))
        im.paste((104, 104, 104, 255), (x + 1, y + 1, x + w + 1, y + 3))
        n = round(w * frac)
        col = hp_colour(frac, (112, 248, 168), (248, 224, 0), (248, 88, 56))
        im.paste(col + (255,), (x + 1, y + 1, x + 1 + n, y + 3))
        im.paste(tuple(max(0, c - 64) for c in col) + (255,), (x + 1, y + 2, x + 1 + n, y + 3))

    def hp_tag(self, im, x, y):
        im.paste((72, 72, 72, 255), (x, y, x + 14, y + 6))
        im.paste((248, 176, 64, 255), (x + 1, y + 1, x + 13, y + 5))

    def sex(self, im, x, y, s):
        col = (64, 136, 248) if s == 'm' else (248, 96, 120)
        p = im.load()
        for (dx, dy) in ([(2, 0), (3, 0), (4, 0), (4, 1), (3, 2), (1, 2), (0, 3), (0, 4), (1, 5), (2, 5), (3, 4),
                          (3, 3), (2, 2)] if s == 'm' else
                         [(1, 0), (2, 0), (0, 1), (3, 1), (0, 2), (3, 2), (1, 3), (2, 3), (1, 4), (2, 4), (0, 5),
                          (3, 5), (1, 6), (2, 6)]):
            p[x + dx, y + dy] = col + (255,)

    # ---------------------------------------------------------------------------------------------- the HUD
    def foe_box(self, im, x, y, m, doubles=False):
        hb = self.healthbox('fdouble' if doubles else 'foe')
        paste(im, hb, x, y)
        self.text(im, x + 8, y + 2, m['name'], (64, 64, 64), (216, 208, 176))
        self.sex(im, x + 8 + self.measure(m['name']), y + 5, m['sex'])
        self.text(im, x + 74, y + 2, str(m['level']), (64, 64, 64), (216, 208, 176))
        self.hp_tag(im, x + 24, y + 17)
        self.hp_bar(im, x + 39, y + 17, m['hp'])

    def mine_box(self, im, x, y, m, doubles=False):
        hb = self.healthbox('pdouble' if doubles else 'player')
        paste(im, hb, x, y)
        self.text(im, x + 16, y + 2, m['name'], (64, 64, 64), (216, 208, 176))
        self.sex(im, x + 16 + self.measure(m['name']), y + 5, m['sex'])
        self.text(im, x + 82 if not doubles else x + 82, y + 2, str(m['level']), (64, 64, 64), (216, 208, 176))
        self.hp_tag(im, x + 32, y + 17)
        self.hp_bar(im, x + 47, y + 17, m['hp'] / m['maxhp'])
        if not doubles:
            self.text(im, x + 60, y + 21, '%d/ %d' % (m['hp'], m['maxhp']), (64, 64, 64), (216, 208, 176))
            im.paste((80, 152, 248, 255), (x + 34 + int(64 * (1 - m['exp'])) - 4, y + 34,
                                          x + 98, y + 36))

    def command(self, im):
        paste(im, self.textbox.crop((0, 272, 240, 320)), 0, 112)
        self.text(im, 8, 120, PROMPT[0], WHITE_INK, WHITE_SHADOW)
        self.text(im, 8, 136, PROMPT[1], WHITE_INK, WHITE_SHADOW)
        for i, a in enumerate(ACTIONS):
            self.text(im, 136 + (i % 2) * 56, 120 + (i // 2) * 16, a)
        self.arrow(im, 128, 123)

    # ---------------------------------------------------------------------------------------------- the screens
    def message(self):
        im = shot(self.kai, 'f_mart')
        self.window(im, 0, 112, 28, 4)
        self.text(im, 16, 120, MESSAGE)
        p = im.load()
        for k in range(4):
            for j in range(k, 7 - k):
                p[16 + self.measure(MESSAGE) + 2 + j, 124 + k] = (248, 88, 56, 255)
        return im

    def start_menu(self):
        im = shot(self.kai, 'f_town')
        self.window(im, 160, 0, 8, 2 * len(START_MENU))
        for i, s in enumerate(START_MENU):
            self.text(im, 176, 8 + 16 * i, s)
        self.arrow(im, 168, 11)
        # (FireRed's help line at the foot)
        im.paste((40, 72, 104, 255), (0, 144, 240, 160))
        self.text(im, 4, 145, 'Check and organize MOéMON.', WHITE_INK, (40, 40, 56))
        return im

    def single(self):
        im = shot(self.kai, 's_single')
        self.foe_box(im, 13, 16, FOE)
        self.mine_box(im, 126, 74, MINE)
        self.command(im)
        return im

    def moves(self):
        im = shot(self.kai, 's_single')
        self.foe_box(im, 13, 16, FOE)
        self.mine_box(im, 126, 74, MINE)
        paste(im, self.textbox.crop((0, 432, 240, 480)), 0, 112)
        for i, (n, *_r) in enumerate(MOVES):
            self.text(im, 16 + (i % 2) * 76, 120 + (i // 2) * 16, n)
        self.arrow(im, 8, 123)
        n, pp, mx, kind = MOVES[0]
        self.text(im, 168, 120, 'PP')
        self.text(im, 232, 120, '%d/%d' % (pp, mx), align='r')
        self.text(im, 168, 136, 'TYPE/' + kind)
        return im

    def double(self):
        im = shot(self.kai, 's_double')
        for i, m in enumerate(DOUBLE_FOES):
            self.foe_box(im, i * 8, 4 + i * 26, m, True)
        for i, m in enumerate(DOUBLE_MINE):
            self.mine_box(im, 132 + i * 8, 58 + i * 26, m, True)
        self.command(im)
        return im

    def triple(self):
        im = shot(self.kai, 's_triple')
        for i, m in enumerate(TRIPLE_FOES):
            self.foe_box(im, 0, 2 + i * 23, m, True)
        for i, m in enumerate(TRIPLE_MINE):
            self.mine_box(im, 136, 42 + i * 23, m, True)
        self.command(im)
        return im

    def party(self):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        bg = Image.open(os.path.join(self.g, 'party_menu', 'bg.png'))
        pal = png_pal(bg)
        paste(im, self._tilemap('party_menu/bg.png', 'party_menu/bg.bin', pal, 32, 20), 0, 0)
        tiles = self._tiles(bg, pal[48:64])
        sel = list(pal[48:64])
        for off, idx in zip((4, 5, 6, 1, 7, 8), (116, 117, 118, 97, 103, 104)):
            sel[off] = pal[idx]
        sel_tiles = self._tiles(bg, sel)

        def slot(name, w, h, x, y, current):
            d = open(os.path.join(self.g, 'party_menu', name), 'rb').read()
            ts = sel_tiles if current else tiles
            for k, t in enumerate(d[:w * h]):
                im.alpha_composite(ts[t], (x + (k % w) * 8, y + (k // w) * 8))

        slot('slot_main.bin', 10, 7, 8, 24, True)
        for i in range(5):
            slot('slot_wide.bin' if i < 3 else 'slot_wide_empty.bin', 18, 3, 96, 8 + i * 24, False)
        for i, m in enumerate(PARTY):
            if i == 0:
                x, y = 8, 24
                paste(im, creature(self.export, m['species'], 'icon'), x + 2, y + 2)
                self.text(im, x + 32, y + 10, m['name'], WHITE_INK, (96, 96, 96))
                self.text(im, x + 40, y + 22, 'Lv%d' % m['level'], WHITE_INK, (96, 96, 96))
                self.hp_bar(im, x + 24, y + 38, 1.0)
                self.text(im, x + 72, y + 42, '%d/ %d' % (m['hp'], m['maxhp']), WHITE_INK, (96, 96, 96),
                          align='r')
            else:
                x, y = 96, 8 + (i - 1) * 24
                paste(im, creature(self.export, m['species'], 'icon'), x - 4, y - 6)
                self.text(im, x + 24, y + 1, m['name'], WHITE_INK, (96, 96, 96))
                self.text(im, x + 32, y + 11, 'Lv%d' % m['level'], WHITE_INK, (96, 96, 96))
                self.hp_bar(im, x + 88, y + 4, 1.0)
                self.text(im, x + 138, y + 9, '%d/ %d' % (m['hp'], m['maxhp']), WHITE_INK, (96, 96, 96),
                          align='r')
        self.window(im, 0, 128, 18, 2)
        self.text(im, 8, 136, 'Choose a MOéMON.')
        self.text(im, 220, 138, 'CANCEL', WHITE_INK, (96, 96, 96), align='c')
        return im

    def summary(self):
        im = Image.new('RGBA', (W, H), (248, 240, 216, 255))
        bg = Image.open(os.path.join(self.g, 'summary_screen', 'bg.png'))
        page = self._tilemap('summary_screen/bg.png', 'summary_screen/page_info.bin', png_pal(bg), 32, 20)
        # (FireRed's header and the picture's panel)
        im.paste((48, 56, 88, 255), (0, 0, 240, 16))
        self.text(im, 4, 0, 'MOéMON INFO', WHITE_INK, (40, 40, 56))
        for k, c in enumerate([(248, 128, 88), (104, 200, 104), (88, 136, 248)]):
            im.paste(c + (255,), (176 + k * 20, 4, 192 + k * 20, 12))
        im.paste((216, 232, 248, 255), (0, 16, 112, 160))
        paste(im, page, 0, 0)
        paste(im, creature(self.export, 280), 24, 36)
        s = SUMMARY
        self.text(im, 4, 18, 'Lv%d' % s['level'])
        self.text(im, 40, 18, s['name'])
        rows = [s['no'], s['name'], s['type'], s['ot'], s['idno'], s['item']]
        for i, v in enumerate(rows):
            self.text(im, 172, 16 + i * 15, v)
        self.text(im, 108, 116, s['nature'] + ' nature,')
        self.text(im, 108, 130, s['met'] + ' ' + s['place'][:6])
        return im

    def bag(self):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        bg = Image.open(os.path.join(self.g, 'item_menu', 'bg.png'))
        paste(im, self._tilemap('item_menu/bg.png', 'item_menu/bg.bin', png_pal(bg), 32, 20), 0, 0)
        bagpic = self._sheet('interface/bag_male.png', png_pal(Image.open(os.path.join(self.g, 'interface',
                                                                                        'bag_male.png'))))
        paste(im, bagpic.crop((0, 0, 64, 64)), 12, 26)
        self.text(im, 44, 4, POCKETS[0], WHITE_INK, (96, 96, 96), align='c')
        for i, (name, n, _item) in enumerate(BAG_ITEMS[:4]):
            self.text(im, 104, 10 + i * 16, name)
            self.text(im, 224, 10 + i * 16, 'x%d' % n, align='r')
        self.text(im, 104, 10 + 64, 'CLOSE BAG')
        self.arrow(im, 96, 13, (248, 88, 56))
        ic = item_icon(self.export, 13)
        if ic:
            paste(im, ic, 4, 116)
        self.text(im, 40, 116, 'A spray-type medicine', WHITE_INK, (40, 40, 56))
        self.text(im, 40, 132, 'that restores HP by 20.', WHITE_INK, (40, 40, 56))
        return im

    def options(self, chosen):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        for y in range(H):
            for x in range(W):
                im.putpixel((x, y), (168, 200, 248, 255) if (x // 8 + y // 8) % 2 else (152, 184, 240, 255))
        self.window(im, 0, 0, 28, 2)
        x = 12
        for t in OPTION_TABS:
            sel = t == 'DISPLAY'
            x = self.text(im, x, 8, t, (248, 88, 56) if sel else DARK) + 8
        self.window(im, 0, 32, 28, 14)
        for i, (name, vals) in enumerate(OPTION_ROWS):
            y = 40 + i * 16
            self.text(im, 16, y, name)
            if vals is None:
                self.text(im, 172, y, chosen, (248, 88, 56), align='c')
            else:
                xx = 124
                for k, v in enumerate(vals):
                    xx = self.text(im, xx, y, v, (248, 88, 56) if k == len(vals) - 1 else DARK) + 8
        self.arrow(im, 8, 43)
        return im

    def all(self):
        return {'message': self.message(), 'start_menu': self.start_menu(), 'battle_single': self.single(),
                'battle_moves': self.moves(), 'battle_double': self.double(), 'battle_triple': self.triple(),
                'party': self.party(), 'summary': self.summary(), 'bag': self.bag(),
                'options': self.options('FIRERED')}
