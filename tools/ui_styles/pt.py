"""PLATINUM: the game's interface in Pokémon Platinum's own pieces, laid out where KAI lays its own.

Everything is Platinum's: the art from pret/pokeplatinum's res/ (healthboxes, their parts, the special characters, the
windows, the party / summary / BAG / OPTIONS screens' tiles and maps) and the system font from the user's ROM
(graphic/pl_font.narc, dsfont.py). Where Platinum's touch screen holds something (the battle's command, the moves), its
buttons stand on the GBA's one screen where KAI puts the same thing.

The battle boxes (src/battle/healthbox.c): each is Platinum's healthbox sliced to KAI's rectangle: its left border,
its plain inside repeated, its right border; the rows Platinum leaves empty above and below its words taken out until
the box is KAI's height. The words and gauges go where healthbox.c's VRAM transfers put them on Platinum's box (the name
window, the gender / Lv icon after it, the level's digits four rows lower, the HP gauge's six cells after the HP label,
the bar's end after them, the HP numbers and the slash, the EXP gauge in the frame's foot). The bar's arrow points at
the box's POKéMON from beside the box, as on the DS (in a Triple Battle the type icon stands there instead)."""
import os
import sys

from PIL import Image

import dsfont
import layout as Lo
from base import Style
from common import H, W, paste

R = os.environ.get('POKEPLATINUM', '/tmp/claude-0/ppl_sparse')


def png_indices(path):
    im = Image.open(path)
    pal = im.getpalette()[:48]
    cols = [tuple(pal[3 * k:3 * k + 3]) for k in range(16)]
    w, h = im.size
    px = im.load()
    return [[px[x, y] for x in range(w)] for y in range(h)], cols


class Art:
    """a picture as palette indices (0 clear) with its palette"""
    def __init__(self, rows, cols):
        self.rows, self.cols = rows, cols
        self.h, self.w = len(rows), len(rows[0]) if rows else 0

    @classmethod
    def load(cls, path):
        return cls(*png_indices(path))

    def crop(self, x0, y0, x1, y1):
        return Art([r[x0:x1] for r in self.rows[y0:y1]], self.cols)

    def pick_rows(self, ys):
        return Art([list(self.rows[y]) for y in ys], self.cols)

    def pick_cols(self, xs):
        return Art([[r[x] for x in xs] for r in self.rows], self.cols)

    def image(self):
        im = Image.new('RGBA', (self.w, self.h), (0, 0, 0, 0))
        p = im.load()
        for y, r in enumerate(self.rows):
            for x, v in enumerate(r):
                if v:
                    p[x, y] = self.cols[v] + (255,)
        return im


def hcat(*parts):
    rows = [sum((p.rows[y] for p in parts), []) for y in range(parts[0].h)]
    return Art(rows, parts[0].cols)


class Platinum(Style):
    name = 'PLATINUM'
    small = 'n'
    # (what check.py finds that is Platinum's own: the HP bar's arrow pointing from beside its box at its POKéMON,
    # into the open space between box and POKéMON (never over a POKéMON or another box; in Triple Battles the type
    # icons stand there and the cards have none); the touch buttons' words over their lit / shaded faces)
    ORIGINAL = [('battle_single', 'edge', None, "Platinum's HP bar arrow beside its box, pointing at its POKéMON"),
                ('battle_moves', 'edge', None, "Platinum's HP bar arrow beside its box, pointing at its POKéMON"),
                ('battle_double', 'edge', None, "Platinum's HP bar arrow beside its box, pointing at its POKéMON"),
                ('battle_single', 'collide', ['BAG', 'RUN', 'MOéMON'], "Platinum's words on its touch buttons"),
                ('battle_double', 'collide', ['BAG', 'RUN', 'MOéMON'], "Platinum's words on its touch buttons"),
                ('battle_triple', 'collide', ['BAG', 'RUN', 'MOéMON'], "Platinum's words on its touch buttons")]

    def __init__(self, rom, kai, export):
        super().__init__(kai, export)
        self.rom = rom
        self.font = dsfont.Platinum(rom)
        hb = os.path.join(R, 'res', 'graphics', 'battle', 'healthbox')
        self.enemy = Art.load(os.path.join(hb, 'enemy.png'))
        self.player = Art.load(os.path.join(hb, 'player_singles.png'))
        self.player2 = Art.load(os.path.join(hb, 'player_doubles.png'))
        self.parts = Art.load(os.path.join(hb, 'healthbox_parts.png'))
        sc = Art.load(os.path.join(R, 'res', 'fonts', 'font_special_chars.png'))
        self.special = sc
        self.pal = self.enemy.cols

    # ---------------------------------------------------------------------------------------------- the boxes' pieces
    def part(self, k):
        """healthbox_parts tile K (HEALTHBOX_PART_*), its index pixels"""
        return [r[k * 8:k * 8 + 8] for r in self.parts.rows]

    def special_glyph(self, k, w=8):
        return [r[k * 8:k * 8 + w] for r in self.special.rows]

    @staticmethod
    def put(rows, x, y, tile, ground=None):
        """a tile's index pixels into ROWS at (X, Y); with GROUND, its pixels of that index (the box's inside,
        which the tile's own window is filled with) are left as they were, so a window whose empty rows were taken
        out of the box never paints over its frame"""
        for yy, r in enumerate(tile):
            for xx, v in enumerate(r):
                Y, X = y + yy, x + xx
                if 0 <= Y < len(rows) and 0 <= X < len(rows[0]) and v != ground and v:
                    rows[Y][X] = v

    def special_put(self, rows, x, y, k, w=8, shadow=2):
        """a special character (font_special_chars: 1 the ink -> 14, 2 its shadow -> 2, 0 the window -> 15) as
        FontSpecialChars_Init(14, 2, 15) colours it (on the bar: its shadow black, as the HP label's)"""
        g = self.special_glyph(k, w)
        for yy, r in enumerate(g):
            for xx, v in enumerate(r):
                if v in (1, 2) and 0 <= y + yy < len(rows) and 0 <= x + xx < len(rows[0]):
                    rows[y + yy][x + xx] = 14 if v == 1 else shadow

    def small_font(self):
        """KAI's own small font (graphics/fonts/latin_small.png, src/fonts.c gFontSmallLatinGlyphWidths): a box's
        name in it only when Platinum's six-pixel letters and its level would not fit KAI's box together"""
        if not getattr(self, '_small', None):
            import re
            import fr
            root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
            src = open(os.path.join(root, 'src', 'fonts.c')).read()
            body = re.search(r'gFontSmallLatinGlyphWidths\[\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
            widths = [int(v) for v in re.findall(r'\d+', re.sub(r'//.*', '', body))]
            self._small = (Image.open(os.path.join(root, 'graphics', 'fonts', 'latin_small.png')), widths, fr.code)
        return self._small

    def small_width(self, s):
        sheet, widths, code = self.small_font()
        return sum(widths[code(c)] for c in s)

    def name_put(self, rows, x, y, s, small=False):
        """the name as healthbox.c prints it: FONT_SYSTEM, TEXT_COLOR(14, 2, 15) into the box's own palette (or,
        SMALL, KAI's small font, its letters' foot on the same row)"""
        if small:
            sheet, widths, code = self.small_font()
            sp = sheet.load()
            for c in s:
                k = code(c)
                gx, gy = (k % 16) * 16, (k // 16) * 16
                for yy in range(16):
                    for xx in range(widths[k]):
                        v = sp[gx + xx, gy + yy]
                        Y, X = y + yy + 1, x + xx
                        if v in (1, 2) and 0 <= Y < len(rows) and 0 <= X < len(rows[0]):
                            rows[Y][X] = 14 if v == 1 else 2
                x += widths[k]
            return x
        for c in s:
            g, adv = self.font.glyph(c)
            for yy, r in enumerate(g):
                for xx, v in enumerate(r):
                    if v in (1, 2) and 0 <= y + yy < len(rows) and 0 <= x + xx < len(rows[0]):
                        rows[y + yy][x + xx] = 14 if v == 1 else 2
            x += adv
        return x

    # ---------------------------------------------------------------------------------------------- the battle boxes
    # Each art's slices (pret's healthbox PNGs as the cells draw them): its left border, a plain inside column, its
    # right border (the player's singles box runs off the DS's screen: its right border is its left one turned), the
    # arrow beside it, the bar's rows, and where healthbox.c's transfers put each thing, measured from the box's
    # right border (RB) or its name.
    SPECS = {
        'foe': dict(art='enemy', left=[3, 4, 5], mid=30, right=list(range(112, 117)), arrow=range(117, 128),
                    arrow_side='r', close='l', bar=33, name=(8, 16), rb=112, gauge=56, label=40, barend=104),
        'mine': dict(art='player', left=list(range(17, 23)), mid=50, right=list(range(22, 16, -1)), arrow=range(0, 17),
                     arrow_side='l', close='r', bar=33, name=(24, 16), rb=128, gauge=72, label=56, barend=120,
                     hp=(64, 40), slash=88, maxhp=(96, 40), exp=(24, 48)),
        'mine2': dict(art='player2', left=list(range(10, 15)), mid=50, right=[122, 123, 124], arrow=range(0, 10),
                      arrow_side='l', close='r', bar=33, name=(16, 16), rb=122, gauge=64, label=48, barend=112),
    }
    # (the rows kept for each height: the words' window loses the empty rows above and below the letters, keeping one
    # each side; the doubles' boxes also lose a frame row top and bottom and the two rows under the bar)
    ROWS = {
        ('foe', 28): [12, 13, 14, 15, 18] + list(range(19, 29)) + [29] + list(range(33, 39)) + [39, 40, 41, 42, 43, 44],
        ('foe', 24): [12, 14, 15, 18] + list(range(19, 29)) + [29] + list(range(33, 39)) + [41, 43, 44],
        ('mine', 36): [11, 12, 13, 14, 15, 18] + list(range(19, 29)) + [29] + list(range(33, 39)) + [39] +
        list(range(41, 48)) + [48, 49, 50, 51, 52],
        ('mine2', 24): [12, 14, 15, 18] + list(range(19, 29)) + [29] + list(range(33, 39)) + [41, 43, 44],
    }

    def box(self, kind, w, h, m, foe):
        """KIND's healthbox W x H with M's name, level, HP (and EXP), and the arrow beside it: (picture, the box's
        x within it)"""
        sp = self.SPECS[kind]
        art = getattr(self, sp['art'])
        ys = self.ROWS[(kind, h)]
        nmid = w - len(sp['left']) - len(sp['right'])
        cols = sp['left'] + [sp['mid']] * nmid + sp['right']
        rows = [[art.rows[y][x] for x in cols] for y in ys]
        bar = ys.index(sp['bar'])
        # (the bar crosses the box's far side to the screen's edge on the DS; here it ends at the box's outline)
        oc = 0 if sp['close'] == 'l' else w - 1
        for y in range(bar, bar + 6):
            rows[y][oc] = 4
        # (positions: art X measured from the right border RB lands at w - len(right) + (X - RB); from the left
        # border at X - left[0])
        rb = w - len(sp['right'])
        fr = lambda X: rb + (X - sp['rb'])  # noqa: E731
        fl = lambda X: X - sp['left'][0]  # noqa: E731
        # the HP label, the gauge's six cells, the bar's end (their rows 1-6: the bar's)
        frac = m['hp'] if foe else m['hp'] / m['maxhp']
        pixels = max(1, round(48 * frac)) if frac > 0 else 0
        base = 2 if pixels > 24 else 11 if pixels > 9 else 20
        tiles = [(fr(sp['label']), 0), (fr(sp['label']) + 8, 1)] + \
            [(fr(sp['gauge']) + 8 * i, base + max(0, min(8, pixels - 8 * i))) for i in range(6)] + \
            [(fr(sp['barend']), 68)]
        for x, k in tiles:
            t = self.part(k)
            for yy in range(1, 7):
                for xx in range(8):
                    if 0 <= x + xx < w:
                        rows[bar - 1 + yy][x + xx] = t[yy][xx]
        # the name (its window's top row: the art's, less the rows taken out above the letters)
        wy = ys.index(19) - 3
        nx = fl(sp['name'][0])
        # (Platinum's letters; KAI's small font, a pixel further left, when the name and the level wouldn't fit)
        nd = len(str(m['level']))
        small = nx + self.measure(m['name']) + 1 > min(nx + 64, rb - 16 - 8 * nd)
        if small:
            nx -= 1
        name_end = self.name_put(rows, nx, wy, m['name'], small)
        self._box_inks = [((nx, wy + 3, name_end, wy + 13), m['name'])]
        # the gender / Lv icon after the name's 64-pixel window, the level's digits after it (left-aligned, four
        # rows down): moved left as far as the right border needs, never onto the name
        lx = min(nx + 64, rb - 16 - 8 * nd)
        if lx < name_end + 1:
            raise ValueError('%s: %s Lv%d does not fit a %d-wide box' % (kind, m['name'], m['level'], w))
        top, bot = {'m': (62, 74), 'f': (60, 72)}.get(m.get('sex'), (64, 76))
        for k, (t0, t1) in enumerate(((top, bot), (top + 1, bot + 1))):
            self.put(rows, lx + 8 * k, wy, self.part(t0), 15)
            self.put(rows, lx + 8 * k, wy + 8, self.part(t1), 15)
        for i, d in enumerate(str(m['level'])):
            self.special_put(rows, lx + 16 + 8 * i, wy + 4, int(d))
        self._box_inks.append(((lx, wy + 3, lx + 16 + 8 * len(str(m['level'])), wy + 12), 'Lv%d' % m['level']))
        if kind == 'mine':
            # the HP numbers under the gauge (current right-aligned, the slash, the maximum left-aligned) and the
            # EXP gauge's cells along the frame's foot
            hy = ys.index(41) - 1
            gx = fr(sp['gauge'])
            cur = '%3d' % m['hp']
            for i, d in enumerate(cur):
                if d != ' ':
                    self.special_put(rows, gx - 8 + 8 * i, hy, int(d))
            self.put(rows, gx + 16, hy, self.part(69), 15)
            for i, d in enumerate(str(m['maxhp'])):
                self.special_put(rows, gx + 24 + 8 * i, hy, int(d))
            self._box_inks.append(((gx - 8, hy + 1, gx + 24 + 8 * len(str(m['maxhp'])), hy + 8),
                                   '%d/%d' % (m['hp'], m['maxhp'])))
            ey = ys.index(48)
            ex0, ex1 = fl(sp['exp'][0]), rb - 2
            fill = round((ex1 - ex0) * m['exp'])
            for x in range(ex0, ex1):
                rows[ey + 2][x] = 12 if x - ex0 < fill else 14
        pic = Art(rows, self.pal).image()
        # the arrow beside the box, its rows on the bar's (three above it, one below)
        ar = Art([[art.rows[y][x] for x in sp['arrow']] for y in range(30, 40)], self.pal).image()
        out = Image.new('RGBA', (w + ar.width, h), (0, 0, 0, 0))
        bx = ar.width if sp['arrow_side'] == 'l' else 0
        out.alpha_composite(pic, (bx, 0))
        out.alpha_composite(ar, (0 if sp['arrow_side'] == 'l' else w, bar - 3))
        return out, bx

    def hud(self, im, r, m, kind):
        x0, y0, x1, y1 = r
        foe = kind.startswith('foe')
        if kind in ('foe3', 'mine3'):
            paste(im, self.card(foe, x1 - x0, m), x0, y0)
            for (a, b, c, d), w in self._box_inks:
                self.inks.append(((x0 + a, y0 + b, x0 + c, y0 + d), w))
            return
        k = 'foe' if foe else 'mine' if kind == 'mine' else 'mine2'
        pic, bx = self.box(k, x1 - x0, y1 - y0, m, foe)
        paste(im, pic, x0 - bx, y0)
        for (a, b, c, d), w in self._box_inks:
            self.inks.append(((x0 + a, y0 + b, x0 + c, y0 + d), w))

    def card(self, foe, w, m):
        """a Triple Battle's card, 20 rows: Platinum's box (the foe's art or the player's doubles art) with its frame's
        outline and one dark row on top, the name's ten rows of ink, then the bar across the card carrying the level
        as the bar carries its HP label (Platinum's small Lv and digits, white over the bar), three gauge cells and
        the bar's end; the border is the art's thin one on both sides (the highlighted side belongs to the arrow)"""
        art = self.enemy if foe else self.player2
        left = [3, 4, 5] if foe else [10, 11, 12]
        mid = 30 if foe else 50
        right = left[::-1] if foe else [122, 123, 124]
        if foe:
            right = [114, 115, 116]
        ys = [12, 14] + list(range(19, 29)) + [32] + list(range(33, 39)) + [44]
        cols = left + [mid] * (w - 6) + right
        rows = [[art.rows[y][x] for x in cols] for y in ys]
        bar = 13
        for y in range(bar, bar + 6):
            rows[y][0] = rows[y][w - 1] = 4
        # the name and its gender (the ♂ / ♀ of Platinum's gender / Lv icon) on the dark frame row's foot
        nx = 4
        end = self.name_put(rows, nx, -1, m['name'])
        self._box_inks = [((nx, 2, end, 12), m['name'])]
        top, bot = {'m': (62, 74), 'f': (60, 72)}.get(m.get('sex'), (None, None))
        if top:
            g = [r[:6] for r in self.part(top)] + [r[:6] for r in self.part(bot)]
            self.put(rows, end + 1, -1, g, 15)
        # the level on the bar
        lx = nx - 5
        for k in (11, 12):
            self.special_put(rows, lx + 8 * (k - 11), bar - 2, k, shadow=5)
        for i, d in enumerate(str(m['level'])):
            self.special_put(rows, lx + 16 + 8 * i, bar - 2, int(d), shadow=5)
        level_end = lx + 16 + 8 * len(str(m['level']))
        self._box_inks.append(((lx + 5, bar - 1, level_end, bar + 6), 'Lv%d' % m['level']))
        # three gauge cells and the bar's end against the right border
        rb = w - 3
        frac = m['hp'] if foe else m['hp'] / m['maxhp']
        pixels = round(24 * frac)
        base = 2 if pixels > 12 else 11 if pixels > 4 else 20
        gx = rb - 8 - 24
        if gx < level_end + 1:
            raise ValueError('card: %s Lv%d does not fit %d' % (m['name'], m['level'], w))
        for i in range(3):
            t = self.part(base + max(0, min(8, pixels - 8 * i)))
            for yy in range(1, 7):
                rows[bar - 1 + yy][gx + 8 * i:gx + 8 * i + 8] = t[yy]
        t = self.part(68)
        for yy in range(1, 7):
            rows[bar - 1 + yy][rb - 8:rb] = t[yy]
        return Art(rows, self.pal).image()

    # ---------------------------------------------------------------------------------------------- the battle's panels
    def _batt_bg(self):
        """Platinum's battle backgrounds (battle/graphic/pl_batt_bg.narc): the message bar (chars 0, map 1, palette
        171) and the touch screen's (chars 28; maps 0x31 the backdrop, 0x2A the command, 0x2B the moves; palette 242
        with row 0 from the field's 0xF3, BattleSubscreen_New)"""
        if getattr(self, '_bb', None):
            return self._bb
        sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'sinnoh'))
        import nds
        import build_library_tv as L
        import build_scratch as B
        bgn = nds.Rom(self.rom).narc('battle/graphic/pl_batt_bg.narc')
        U = lambda i: L.unpack(bgn[i])  # noqa: E731

        def rgba(p):
            im = Image.new('RGBA', (p.w, p.h), (0, 0, 0, 0))
            px = im.load()
            for k, c in enumerate(p.px):
                if c & 0x8000:
                    px[k % p.w, k // p.w] = ((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3, 255)
            return im
        sub = L.palette(U(0xF3))[:16] + L.palette(U(242))[16:]
        self._bb = dict(bar=rgba(B.background(U(0), U(1), L.palette(U(171)))).crop((0, 144, 256, 192)),
                        back=rgba(B.background(U(28), U(0x31), sub)),
                        command=rgba(B.background(U(28), U(0x2A), sub)),
                        moves_raw=(U(28), U(0x2B), sub), B=B, L=L, rgba=rgba)
        return self._bb

    def font_draw(self, im, font, x, y, s, cols, align='l'):
        """S in FONT (dsfont.Platinum) with its values 1, 2, 3 in COLS (None: left out)"""
        w = sum(font.glyph(c)[1] for c in s)
        x -= w // 2 if align == 'c' else w if align == 'r' else 0
        p = im.load()
        for c in s:
            g, adv = font.glyph(c)
            for yy, r in enumerate(g):
                for xx, v in enumerate(r):
                    if v and cols[v - 1] and 0 <= x + xx < im.width and 0 <= y + yy < im.height:
                        p[x + xx, y + yy] = cols[v - 1] + (255,)
            x += adv
        return x

    def fonts(self):
        if not getattr(self, '_fonts', None):
            self._fonts = {'msg': dsfont.Platinum(self.rom, 1), 'sub': dsfont.Platinum(self.rom, 2)}
        return self._fonts

    @staticmethod
    def slice9(src, w, h, l, t, r, b):
        """SRC stretched to W x H by repeating its middle row and column (corners L / T / R / B kept)"""
        out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        sw, sh = src.size
        xs = list(range(l)) + [l + (sw - l - r) // 2] * (w - l - r) + list(range(sw - r, sw))
        ys = list(range(t)) + [t + (sh - t - b) // 2] * (h - t - b) + list(range(sh - b, sh))
        sp, op = src.load(), out.load()
        for oy, sy in enumerate(ys):
            for ox, sx in enumerate(xs):
                op[ox, oy] = sp[sx, sy]
        return out

    BATTLE_INK = ((80, 80, 88), (160, 160, 168))      # (pl_font palette 7: the battle window's text)

    def battle_msg(self, im, r=Lo.BATTLE_MSG):
        """the battle's text window (healthbox-era BattleSystem window 0: (2, 19) 27 x 4 tiles, palette 11, filled
        white) inside the message bar: the bar's left 16 pixels, the window, its right 24"""
        bb = self._batt_bg()
        x0, y0, x1, y1 = r
        w = x1 - x0
        bar = bb['bar']
        cols = list(range(16)) + [128] * (w - 40) + list(range(232, 256))
        out = Image.new('RGBA', (w, 48))
        sp, op = bar.load(), out.load()
        for y in range(48):
            for ox, sx in enumerate(cols):
                op[ox, y] = sp[sx, y]
        out.paste((248, 248, 248, 255), (16, 8, w - 24, 40))
        paste(im, out, x0, y0)
        ink, sh = self.BATTLE_INK
        from common import PROMPT
        for i, line in enumerate(PROMPT):
            self.text(im, x0 + 16, y0 + 8 + 16 * i, line, ink, sh, 'msg')

    def text(self, im, x, y, s, col=None, shadow=None, font='n', align='l', outline=None):
        """S in one of Platinum's fonts ('n' FONT_SYSTEM, 'msg' FONT_MESSAGE, 'sub' FONT_SUBSCREEN): its pixels 1 in
        COL, 2 in SHADOW, 3 (the subscreen font's outline) in OUTLINE; by default the system's dark grey on white"""
        f = {'n': self.font}.get(font) or self.fonts()[font]
        if col is None:
            col, shadow = (80, 80, 88), (160, 160, 168)
        return self.font_draw(im, f, x, y, s, (col, shadow, outline), align)

    def measure(self, s, font='n'):
        f = {'n': self.font}.get(font) or self.fonts()[font]
        return dsfont.width(f, s)

    def move_palettes(self):
        """src/overlay011/move_palettes.c: each type's sixteen colours for the move buttons (LoadMoveSelectPltt)"""
        if getattr(self, '_mp', None):
            return self._mp
        import re
        src = open(os.path.join(R, 'src', 'overlay011', 'move_palettes.c')).read() \
            if os.path.exists(os.path.join(R, 'src', 'overlay011', 'move_palettes.c')) else \
            open('/root/.cache/moemon/pokeplatinum/src/overlay011/move_palettes.c').read()
        out = {}
        for name, body in re.findall(r'sMovePalette(\w+)\[\] = \{(.*?)\};', src, re.S):
            cols = [(int(r) * 8, int(g) * 8, int(b) * 8) for r, g, b in re.findall(r'RGB\((\d+), (\d+), (\d+)\)', body)]
            out[name.upper()] = cols
        self._mp = out
        return out

    def battle_menu(self, im, r=Lo.BATTLE_MENU):
        """the touch screen's command on the GBA's: the backdrop (0x31) behind Platinum's four buttons (0x2A), FIGHT
        across the top, BAG / RUN / POKéMON under it as on the DS (RUN a little lower), each nine-sliced to fit; the
        words in FONT_SUBSCREEN with each button's colours (pl_batt_obj 72, row 2: TEXT_COLOR(1, 2, 3) ...); the
        D-pad's cursor, Platinum's red corners, on FIGHT"""
        bb = self._batt_bg()
        x0, y0, x1, y1 = r
        back = bb['back'].crop((128 - (x1 - x0) // 2, 192 - (y1 - y0), 128 + (x1 - x0) // 2, 192))
        paste(im, back, x0, y0)
        c = bb['command']
        fight = c.crop((20, 34, 236, 127))
        bag, run, mon = c.crop((1, 144, 79, 191)), c.crop((89, 152, 167, 192)), c.crop((177, 144, 255, 191))
        w = x1 - x0
        f = self.slice9(fight, w - 4, 23, 12, 9, 12, 4)
        paste(im, f, x0 + 2, y0 + 2)
        # (BAG and RUN as wide as their words need, POKéMON the rest: FONT_SUBSCREEN's "MOéMON" is 42 pixels)
        widths, xs = [29, 29, 56], [x0 + 2, x0 + 32, x0 + 62]
        for k, (pic, dy) in enumerate(((bag, 0), (run, 2), (mon, 0))):
            s = self.slice2(pic, widths[k], 22 - dy, 10, 9, 4)
            paste(im, s, xs[k], y0 + 26 + dy)
        cols = {'FIGHT': ((248, 248, 248), (232, 208, 208), (88, 32, 32)),
                'BAG': ((248, 248, 248), (232, 224, 184), (96, 64, 0)),
                'RUN': ((248, 248, 248), (200, 200, 224), (16, 32, 88)),
                'MOéMON': ((248, 248, 248), (208, 232, 208), (32, 72, 24))}
        self.text(im, x0 + w // 2, y0 + 8, 'FIGHT', *cols['FIGHT'][:2], font='sub', align='c', outline=cols['FIGHT'][2])
        for k, (word, dy) in enumerate((('BAG', 0), ('RUN', 2), ('MOéMON', 0))):
            cx = xs[k] + widths[k] // 2
            self.text(im, cx, y0 + 29 + dy, word, *cols[word][:2], font='sub', align='c', outline=cols[word][2])
        self.corners(im, x0 + 2, y0 + 2, x0 + w - 2, y0 + 25)

    def slice2(self, src, w, h, cap, top, bottom):
        """a small button: its caps and the tab in its middle kept, the plain columns either side of the tab
        repeated; its top and foot kept, a middle row repeated"""
        sw, sh = src.size
        mid0, mid1 = sw // 2 - 8, sw // 2 + 8
        cap = min(cap, (w - (mid1 - mid0)) // 2)
        extra = w - (cap * 2 + (mid1 - mid0))
        la, ra = extra // 2, extra - extra // 2
        xs = list(range(cap)) + [cap + 2] * la + list(range(mid0, mid1)) + [sw - cap - 3] * ra + \
            list(range(sw - cap, sw))
        ys = list(range(top)) + [top + (sh - top - bottom) // 2] * (h - top - bottom) + list(range(sh - bottom, sh))
        out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        sp, op = src.load(), out.load()
        for oy, sy in enumerate(ys):
            for ox, sx in enumerate(xs):
                op[ox, oy] = sp[sx, sy]
        return out

    def corners(self, im, x0, y0, x1, y1):
        """battle/interface/cursor: the L of the D-pad's cursor at each corner (white edge, red inside)"""
        src = Image.open(os.path.join(R, 'res', 'graphics', 'battle', 'interface', 'cursor.png'))
        pal = src.getpalette()
        px = src.load()
        L = Image.new('RGBA', (12, 12), (0, 0, 0, 0))
        q = L.load()
        for y in range(12):
            for x in range(12):
                v = px[x, y]
                if v:
                    q[x, y] = tuple(pal[3 * v:3 * v + 3]) + (255,)
        paste(im, L, x0 - 2, y0 - 2)
        paste(im, L.transpose(Image.FLIP_LEFT_RIGHT), x1 - 10, y0 - 2)
        paste(im, L.transpose(Image.FLIP_TOP_BOTTOM), x0 - 2, y1 - 10)
        paste(im, L.transpose(Image.ROTATE_180), x1 - 10, y1 - 10)

    TYPE_OF = {'FOCUS ENERGY': 'NORMAL', 'EMBER': 'FIRE', 'PECK': 'FLYING', 'SAND-ATTACK': 'GROUND'}

    def move_button(self, kind, w, h=23):
        """a move button of 0x2B in KIND's palette (LoadMoveSelectPltt into rows 8-11), cut to W x H: its outline,
        top and bevel, the cream panel's top edge, ten rows of the panel for the name, its foot and the button's"""
        bb = self._batt_bg()
        mp = self.move_palettes()
        ncgr, nscr, sub = bb['moves_raw']
        pal = list(sub)
        pal[8 * 16:9 * 16] = [(r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) for r, g, b in mp[kind]]
        full = bb['rgba'](bb['B'].background(ncgr, nscr, pal)).crop((2, 24, 126, 79))
        ys = [2, 3, 4, 5, 8, 12] + [20] * (h - 11) + [49, 51, 52, 53, 54]
        # (the sides thinned to the outline, one column of the type's band and the panel's edge, so a twelve-letter
        # name has KAI's 72 pixels)
        xs = [0, 3, 13] + [62] * (w - 6) + [110, 116, 123]
        out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        sp, op = full.load(), out.load()
        for oy, sy in enumerate(ys):
            for ox, sx in enumerate(xs):
                op[ox, oy] = sp[sx, sy]
        return out

    def type_icon(self, kind):
        im = Image.open(os.path.join(R, 'res', 'graphics', 'battle', 'type_icons', kind.lower() + '.png'))
        return im.convert('RGBA')

    def move_panel(self, im):
        """the moves: Platinum's four buttons, each its type's colours, two by two where KAI has its four; beside
        them, where KAI shows the cursor's move's PP and TYPE, the backdrop with that move's type icon and its PP
        (FONT_SYSTEM, GetPPTextColor's TEXT_COLOR(1, 2, 0) of pl_batt_obj 72's row 4)"""
        from common import MOVES
        bb = self._batt_bg()
        x0, y0, x1, y1 = Lo.MOVES
        back = bb['back'].crop((128 - 120, 192 - 48, 128 + 120, 192))
        paste(im, back, 0, y0)
        bw = (x1 - x0) // 2
        # (each button 80 x 22, two rows from y0 + 2 so the cursor's corners, two pixels out, stay in the panel)
        for i, (n, pp, mx, kind) in enumerate(MOVES):
            bx, by = x0 + (i % 2) * bw, y0 + 2 + (i // 2) * 24
            paste(im, self.move_button(self.TYPE_OF.get(n, 'NORMAL'), bw, 22), bx, by)
        self.corners(im, x0, y0 + 2, x0 + bw, y0 + 24)
        for i, (n, pp, mx, kind) in enumerate(MOVES):
            bx, by = x0 + (i % 2) * bw, y0 + 2 + (i // 2) * 24
            # (FONT_SYSTEM for every name: FONT_SUBSCREEN's 84 pixels for twelve letters don't fit a button half
            # Platinum's width; the darkest and lightest of the name's own colours)
            self.text(im, bx + bw // 2, by + 4, n, (32, 32, 24), (144, 144, 128), 'n', 'c')
        # (the cursor's move: its type icon and PP, in a system window as Platinum's own small windows)
        mx0, my0, mx1, my1 = Lo.MOVE_INFO
        n, pp, mx, kind = MOVES[0]
        self.std_frame(im, mx0 // 8 + 1, my0 // 8 + 1, (mx1 - mx0) // 8 - 2, (my1 - my0) // 8 - 2, 'standard_system')
        im.paste((255, 255, 255, 255), (mx0 + 8, my0 + 8, mx1 - 8, my1 - 8))
        icon = self.type_icon(kind)
        paste(im, icon, (mx0 + mx1) // 2 - 16, my0 + 8)
        self.text(im, mx0 + 10, my0 + 24 - 3, 'PP', *self.FIELD_INK)
        self.text(im, mx1 - 9, my0 + 24 - 3, '%d/%d' % (pp, mx), *self.FIELD_INK, align='r')

    # ---------------------------------------------------------------------------------------------- the field
    WIN = os.path.join(R, 'res', 'graphics', 'windows')
    FIELD_INK = ((88, 88, 80), (168, 184, 184))      # (pl_font palette 6: FONT_MESSAGE's colours on white)

    def sheet_tiles(self, png):
        im = Image.open(png)
        pal = im.getpalette()[:48]
        cols = [tuple(pal[3 * k:3 * k + 3]) for k in range(16)]
        px = im.load()
        out = []
        for ty in range(im.height // 8):
            for tx in range(im.width // 8):
                t = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
                q = t.load()
                for y in range(8):
                    for x in range(8):
                        v = px[tx * 8 + x, ty * 8 + y]
                        if v:
                            q[x, y] = cols[v] + (255,)
                out.append(t)
        return out

    def message_frame(self, im, tx, ty, tw, th, frame='message_box_00'):
        """DrawMessageBoxFrame: a window of TW x TH tiles at tile (TX, TY), its frame two tiles left, three right,
        one above and below (tiles 0-17 of the frame's sheet), the window filled white"""
        t = self.sheet_tiles(os.path.join(self.WIN, frame + '.png'))
        im.paste((255, 255, 255, 255), (tx * 8, ty * 8, (tx + tw) * 8, (ty + th) * 8))

        def put(k, cx, cy):
            paste(im, t[k], cx * 8, cy * 8)
        for row, cy, ks in ((0, ty - 1, (0, 1, 2, 3, 4, 5)), (1, None, (6, 7, 8, 9, 10, 11)),
                            (2, ty + th, (12, 13, 14, 15, 16, 17))):
            for yy in ([cy] if cy is not None else range(ty, ty + th)):
                put(ks[0], tx - 2, yy)
                put(ks[1], tx - 1, yy)
                if row != 1:
                    for xx in range(tx, tx + tw):
                        put(ks[2], xx, yy)
                put(ks[3], tx + tw, yy)
                put(ks[4], tx + tw + 1, yy)
                put(ks[5], tx + tw + 2, yy)

    def scroll_cursor(self, im, x, y):
        """scroll_cursor's first frame (tiles 0-3 as a 16 x 16 bitmap, its x 4-15)"""
        t = self.sheet_tiles(os.path.join(self.WIN, 'scroll_cursor.png'))
        f = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
        for k, (ox, oy) in enumerate(((0, 0), (8, 0), (0, 8), (8, 8))):
            f.alpha_composite(t[k], (ox, oy))
        paste(im, f.crop((4, 0, 16, 16)), x, y)

    def message(self):
        """the field's message box: frame TYPE 1 (message_box_00) round a white window of 25 x 4 tiles from tile
        (2, 15), the words in FONT_MESSAGE at the window's corner, the scroll cursor in the frame's right side"""
        from common import shot, MESSAGE
        im = shot(self.kai, 'f_mart')
        self.message_frame(im, 2, 15, 25, 4)
        self.text(im, 16, 120, MESSAGE, *self.FIELD_INK, font='msg')
        self.scroll_cursor(im, 28 * 8 + 1, 17 * 8)
        return im

    def std_frame(self, im, tx, ty, tw, th, kind='standard_field'):
        """DrawStandardWindowFrame: tiles 0-8 of the frame (4 its inside) round a window of TW x TH tiles"""
        t = self.sheet_tiles(os.path.join(self.WIN, kind + '.png'))
        for yy in range(ty - 1, ty + th + 1):
            for xx in range(tx - 1, tx + tw + 1):
                r = 0 if yy == ty - 1 else 2 if yy == ty + th else 1
                c = 0 if xx == tx - 1 else 2 if xx == tx + tw else 1
                paste(im, t[r * 3 + c], xx * 8, yy * 8)

    def start_icon(self, k, coloured):
        """start_menu/icons cell K (33 pixels apart in the sheet), in menu.pal's row 1 (coloured) or row 0 (grey)"""
        d = os.path.join(R, 'res', 'graphics', 'start_menu')
        src = Image.open(os.path.join(d, 'icons.png'))
        lines = open(os.path.join(d, 'menu.pal')).read().split('\n')[3:]
        pal = [tuple(map(int, line.split())) for line in lines if line.strip()]
        px = src.load()
        out = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
        q = out.load()
        for y in range(32):
            for x in range(32):
                v = px[x, 33 * k + y]
                if v:
                    q[x, y] = pal[(16 if coloured else 0) + v] + (255,)
        return out

    def start_menu(self):
        """Platinum's START menu on the right: the field's standard frame round an 11-tile window, each option's icon
        (coloured on the cursor's, grey on the rest) beside its name, the orange cursor round the row. Seven rows
        24 pixels apart are taller than the GBA's screen: they are 20 apart here (the icons are 20 tall at most)"""
        from common import START_MENU, shot
        im = shot(self.kai, 'f_town')
        tx, ty, tw, th = 18, 1, 11, 18
        self.std_frame(im, tx, ty, tw, th)
        x0, y0 = tx * 8, ty * 8
        icons = [0, 1, 2, 3, 4, 5, 6]     # (POKéDEX, POKéMON, BAG, the trainer's card, SAVE, OPTIONS, EXIT)
        cur = Image.open(os.path.join(R, 'res', 'graphics', 'start_menu', 'cursor.png'))
        cp = cur.getpalette()
        cpx = cur.load()
        cw = Image.new('RGBA', (92, 26), (0, 0, 0, 0))
        q = cw.load()
        for y in range(26):
            for x in range(92):
                v = cpx[x + 2, y + 3]
                if v:
                    q[x, y] = tuple(cp[3 * v:3 * v + 3]) + (255,)
        cw = self.slice9(cw, 92, 22, 6, 4, 6, 4)
        for i, s in enumerate(START_MENU):
            cy = y0 + 2 + 20 * i + 10
            ic = self.start_icon(icons[i], i == 0)
            paste(im, ic, x0 + 14 - 16, cy - 16)
            self.text(im, x0 + 28, cy - 8, s, *self.FIELD_INK, font='msg')
        paste(im, cw, x0 + 44 - 46, y0 + 2 + 10 - 11)
        return im

    # ---------------------------------------------------------------------------------------------- the party
    @staticmethod
    def gray_tiles(png):
        """an 'L' sheet of pret's (4-bit grey): its levels are palette indices counted down from white"""
        im = Image.open(png)
        px = im.load()
        w, h = im.size
        return [[(255 - px[tx * 8 + x, ty * 8 + y]) // 17 for y in range(8) for x in range(8)]
                for ty in range(h // 8) for tx in range(w // 8)]

    @staticmethod
    def jasc(path):
        lines = open(path).read().split('\n')
        return [tuple(map(int, line.split())) for line in lines[3:3 + int(lines[2])] if line.strip()]

    @staticmethod
    def nscr(path):
        import struct
        d = open(path, 'rb').read()
        o = d.find(b'NRCS')
        w, h = struct.unpack_from('<HH', d, o + 8)
        n = struct.unpack_from('<I', d, o + 16)[0]
        return w, h, struct.unpack_from('<%dH' % (n // 2), d, o + 20)

    @staticmethod
    def map_rows(ts, ents, mapw, c0, r0, cols, rows, pal, prow=None):
        """tile map rows R0.. / columns C0.. of a map MAPW tiles wide: an index picture (and its RGBA)"""
        idx = [[0] * (cols * 8) for _ in range(rows * 8)]
        for r in range(rows):
            for c in range(cols):
                e = ents[(r0 + r) * mapw + c0 + c]
                t, hf, vf = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1
                if t >= len(ts):
                    continue
                for i, v in enumerate(ts[t]):
                    x, y = i % 8, i // 8
                    if hf:
                        x = 7 - x
                    if vf:
                        y = 7 - y
                    idx[r * 8 + y][c * 8 + x] = (v, prow if prow is not None else e >> 12)
        im = Image.new('RGBA', (cols * 8, rows * 8), (0, 0, 0, 0))
        p = im.load()
        for y, row in enumerate(idx):
            for x, vp in enumerate(row):
                if vp and vp[0]:
                    p[x, y] = pal[vp[1] * 16 + vp[0]] + (255,)
        return im

    PANEL_ROWS = [y for y in range(4, 48) if y not in (7, 32, 43, 44)]     # (44 rows of art less four: 40)
    PANEL_COLS = [x for x in range(128) if not 40 <= x < 48]              # (128 less eight plain columns: 120)

    def party(self):
        """Platinum's party (src/applications/party_menu): its striped background, the six panels two by two
        (the lead's round, the rest cut; blue, the cursor's light blue, an empty slot's grey: colors 0 / 4 / none in
        row 1), each 128 x 48 panel 120 x 40 here (eight plain columns and four empty rows out); in each the ball,
        the icon, the name and gender (FONT_SYSTEM, TEXT_COLOR(15, 14, 0) / (3, 4, 0) / (5, 6, 0)), "Lv." and the
        level, the HP bar's fill and the HP in the special characters (FontSpecialChars_Init(15, 14, 0)); under them
        the message box and the CANCEL button"""
        from common import PARTY, creature
        d = os.path.join(R, 'res', 'graphics', 'party_menu')
        pal = self.jasc(os.path.join(d, 'menu.pal'))
        ts = self.gray_tiles(os.path.join(d, 'menu_tiles.png'))
        _, _, bents = self.nscr(os.path.join(d, 'menu.NSCR'))
        _, _, pents = self.nscr(os.path.join(d, 'menu_panels.NSCR'))
        im = Image.new('RGBA', (W, H), pal[0] + (255,))
        bg = self.map_rows(ts, bents, 32, 0, 0, 30, 20, pal)
        im.alpha_composite(bg)
        white, dark = pal[15], pal[14]
        for slot in range(6):
            m = PARTY[slot] if slot < len(PARTY) else None
            kind = 0 if slot == 0 else 1 if m else 2
            prow = 3 + (4 if slot == 0 else 0) if m else 1
            full = self.map_rows(ts, pents, 32, 0, 6 * kind, 16, 6, pal, prow)
            fp = full.load()
            panel = Image.new('RGBA', (120, 40), (0, 0, 0, 0))
            q = panel.load()
            for oy, sy in enumerate(self.PANEL_ROWS):
                for ox, sx in enumerate(self.PANEL_COLS):
                    q[ox, oy] = fp[sx, sy]
            px0, py0 = (slot % 2) * 120, 1 + (slot // 2) * 42
            ys = {y: i for i, y in enumerate(self.PANEL_ROWS)}
            X = lambda x: x if x < 40 else x - 8  # noqa: E731
            if m:
                # (the HP bar's fill: rows 2, 3-4, 5 of the window at (8, 3); colors 9 / 10 of the bar's colour)
                frac = m['hp'] / m['maxhp']
                n = round(48 * frac)
                lite, deep = (pal[3 * 16 + 9], pal[3 * 16 + 10]) if frac > 0.5 else \
                    (pal[4 * 16 + 9], pal[4 * 16 + 10]) if frac > 0.2 else (pal[5 * 16 + 9], pal[5 * 16 + 10])
                for x in range(n):
                    for yy, c in ((26, deep), (27, lite), (28, lite), (29, deep)):
                        q[X(64 + x), ys[yy]] = c + (255,)
            paste(im, panel, px0, py0)
            if not m:
                continue
            ball = self.indexed(os.path.join(d, 'member_ball.png')).crop((0, 0, 32, 32))
            paste(im, ball, px0 + 16 - 16, py0 + ys[14] - 16 + 2)
            icon = creature(self.export, m['species'], 'icon')
            paste(im, icon, px0 + 26 - 16, py0 + ys[16] - 16 + 2)
            ny = py0 + ys[11] - 3
            self.text(im, px0 + X(48), ny, m['name'], white, dark)
            sex = ('♂', pal[3], pal[4]) if m['sex'] == 'm' else ('♀', pal[5], pal[6])
            self.text(im, px0 + X(112), ny, sex[0], sex[1], sex[2])
            ly = py0 + ys[35] - 1
            self.special_text(im, px0 + 5, ly, 'L', white, dark)
            self.special_text(im, px0 + 5 + 16, ly, str(m['level']), white, dark)
            self.special_text(im, px0 + X(60), ly, '%3d' % m['hp'], white, dark)
            self.special_text(im, px0 + X(84), ly, '/', white, dark)
            self.special_text(im, px0 + X(92), ly, str(m['maxhp']), white, dark)
        # the message box and the CANCEL button
        self.message_frame(im, 2, 17, 20, 2)
        self.text(im, 16, 136, 'Choose a MOéMON.', *self.FIELD_INK, font='msg')
        btn = Image.open(os.path.join(d, 'button.png'))
        bp = btn.getpalette()
        b = Image.new('RGBA', (56, 32), (0, 0, 0, 0))
        bq, bpx = b.load(), btn.load()
        for y in range(32):
            for x in range(56):
                v = bpx[x, y]
                if v:
                    bq[x, y] = tuple(bp[3 * v:3 * v + 3]) + (255,)
        b = b.crop(b.getbbox())
        paste(im, b, 238 - b.width, 157 - b.height)
        self.text(im, 238 - b.width // 2, 157 - b.height // 2 - 8, 'CANCEL', (248, 248, 248), (32, 56, 120),
                  font='n', align='c')
        return im

    @staticmethod
    def indexed(path, pal=None):
        """a paletted PNG with its index 0 clear"""
        src = Image.open(path)
        pal = pal or [tuple(src.getpalette()[3 * k:3 * k + 3]) for k in range(16)]
        out = Image.new('RGBA', src.size, (0, 0, 0, 0))
        sp, op = src.load(), out.load()
        for y in range(src.height):
            for x in range(src.width):
                v = sp[x, y] % 16
                if v:
                    op[x, y] = pal[v] + (255,)
        return out

    def special_text(self, im, x, y, s, ink, shadow):
        """font_special_chars: digits, '/' (10) and 'L' meaning the "Lv." pair (11-12), 8 pixels a character"""
        for c in s:
            if c == ' ':
                x += 8
                continue
            k = 10 if c == '/' else 11 if c == 'L' else int(c)
            w = 16 if c == 'L' else 8
            g = self.special_glyph(k, w)
            p = im.load()
            for yy, r in enumerate(g):
                for xx, v in enumerate(r):
                    if v in (1, 2) and 0 <= x + xx < im.width and 0 <= y + yy < im.height:
                        p[x + xx, y + yy] = (ink if v == 1 else shadow) + (255,)
            x += w
        return x

    # ---------------------------------------------------------------------------------------------- the summary
    SUM_INK = {'black': ((16, 24, 32), (172, 189, 189)), 'white': ((255, 255, 255), (82, 82, 82)),
               'blue': ((0, 115, 255), (123, 189, 238)), 'red': ((238, 32, 16), (255, 172, 189))}

    def cell_sprite(self, png, cell_json, k, pal=None):
        """cell K of a pret cell JSON over its PNG (1D tiles, the PNG's tiles in rows of its width)"""
        import json
        j = json.load(open(cell_json))
        OAM = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
               (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}
        src = Image.open(png)
        sp = src.getpalette()
        pal = pal or [tuple(sp[3 * v:3 * v + 3]) for v in range(16)]
        px = src.load()
        tw = src.width // 8
        oams = []
        for o in j['cells'][k]['OAM']:
            a0, a1, a2 = o['Attr0'], o['Attr1'], o['Attr2']
            w, h = OAM[(a0['Shape'], a1['Size'])]
            x, y = a1['XCoordinate'], a0['YCoordinate']
            x -= 512 if x > 255 else 0
            y -= 256 if y > 127 else 0
            oams.append((x, y, w, h, a2['CharName']))
        x0 = min(o[0] for o in oams)
        y0 = min(o[1] for o in oams)
        out = Image.new('RGBA', (max(o[0] + o[2] for o in oams) - x0, max(o[1] + o[3] for o in oams) - y0),
                        (0, 0, 0, 0))
        for x, y, w, h, c in oams:
            for ty in range(h // 8):
                for tx in range(w // 8):
                    t = c + ty * (w // 8) + tx
                    for yy in range(8):
                        for xx in range(8):
                            v = px[(t % tw) * 8 + xx, (t // tw) * 8 + yy]
                            if v:
                                out.putpixel((x - x0 + tx * 8 + xx, y - y0 + ty * 8 + yy), pal[v] + (255,))
        return out

    def summary_page(self, im):
        """Platinum's INFO page (pokemon_summary_screen/page_info) made 240 x 160: the header; KAI's six pages as
        Platinum's tabs under it (INFO, SKILLS, BATTLE MOVES, CONTEST MOVES, the BALL page as CONDITION's, the
        JOURNAL as MEMO's) and EXIT, centred at 188 less the 16 the screen is narrower; on the left the name plate,
        the picture's panel and the ITEM plate (the empty rows between them fewer); on the right the green panel's
        rows (POKéDEX No., NAME, TYPE, OT, ID No.) with sixteen of its columns out before the values, and under them
        KAI's TRAINER MEMO on the stripes; RENAME in the header's prompt"""
        from common import SUMMARY, creature
        d = os.path.join(R, 'res', 'graphics', 'pokemon_summary_screen')
        pal = self.jasc(os.path.join(d, 'tiles_main.pal'))
        page = self.page_art(os.path.join(d, 'tiles_main.png'), os.path.join(d, 'page_info.NSCR'), pal)
        pp = page.load()
        s = SUMMARY
        im.paste(pp[2, 100][:3] + (255,), (0, 0, W, H))
        # header (rows 0-15; its 240 left columns)
        paste(im, page.crop((0, 0, 240, 16)), 0, 0)
        # left column: rows kept (header 0-15, then 16-19, the plate 21-54, 55-58, the picture 70-134, 135, the
        # ITEM plate 156-191)
        left_rows = list(range(16, 20)) + list(range(21, 55)) + list(range(55, 59)) + list(range(70, 135)) + [135] + \
            list(range(156, 192))
        for oy, sy in enumerate(left_rows):
            for x in range(104):
                im.putpixel((x, 16 + oy), pp[x, sy])
        Ly = {sy: 16 + i for i, sy in enumerate(left_rows)}
        # right panel: columns 104-167 and 184-255 less 252-255 (136); rows 30-117, then the stripes 118-149 drawn
        # without the EXP boxes, then the foot
        rcols = list(range(104, 168)) + list(range(184, 252)) + [252, 253, 254, 255][:0]
        rcols = rcols[:136]
        right_rows = list(range(28, 118))
        top = 160 - len(right_rows) - 34
        for oy, sy in enumerate(right_rows):
            for ox, sx in enumerate(rcols):
                im.putpixel((104 + ox, top + oy), pp[sx, sy])
        Ry = {sy: top + i for i, sy in enumerate(right_rows)}
        stripe = [pp[120, y] for y in range(118, 150)]
        for i in range(34):
            y = top + len(right_rows) + i
            for ox, sx in enumerate(rcols):
                c = pp[sx, 118 + i % 32]
                im.putpixel((104 + ox, y), c if sx < 106 else stripe[i % 32])
        X = lambda x: 104 + rcols.index(x) if x in rcols else None  # noqa: E731
        # the tabs (16 x 16, the current 24 x 16) under the header, centred
        tabs = [0, 2, 4, 5, 3, 1, 7]
        base = 172 - (24 + (len(tabs) - 1) * 16) // 2
        x = base
        for i, k in enumerate(tabs):
            cell = self.cell_sprite(os.path.join(d, 'tabs.png'), os.path.join(d, 'tabs_cell.json'), k + (8 if i == 0 else 0))
            paste(im, cell, x, 16)
            x += cell.width
        # the header's words: the page, and KAI's RENAME where Platinum prompts
        self.text(im, 8, 0, 'MOéMON INFO', *self.SUM_INK['white'])
        self.text(im, 236, 0, 'SELECT RENAME', *self.SUM_INK['white'], align='r')
        # the name plate: the name (white) and gender, "Lv." and the level
        ny = Ly[24] - 0
        nx = self.text(im, 24, ny, s['name'], *self.SUM_INK['white'])
        self.text(im, 24 + 64, ny, '♂' if s['sex'] == 'm' else '♀', *self.SUM_INK['blue' if s['sex'] == 'm' else 'red'])
        self.special_text(im, 8, Ly[42] - 0, 'L', *self.SUM_INK['black'])
        self.special_text(im, 8 + 16, Ly[42] - 0, str(s['level']), *self.SUM_INK['black'])
        pic = creature(self.export, 280)
        paste(im, pic, 52 - pic.width // 2, Ly[102] - pic.height // 2)
        # the ITEM plate
        self.text(im, 8, Ly[160], 'Item', *self.SUM_INK['white'])
        self.text(im, 8, Ly[176], s['item'], *self.SUM_INK['black'])
        # the right panel's labels (white, at x 112) and values (black, centred in their windows)
        rows = [('Moédex No.', s['no'], 192, 48), ('Name', s['species'], 184, 64), ('Type', None, 0, 0),
                ('OT', s['ot'], 184, 64), ('ID No.', s['idno'], 200, 32)]
        for i, (label, val, wx, ww) in enumerate(rows):
            y = Ry[40 + 16 * i]
            self.text(im, X(112), y, label, *self.SUM_INK['white'])
            if val is not None:
                cx = X(wx) + ww // 2 if X(wx) else X(184) + 32
                self.text(im, cx, y, val, *self.SUM_INK['blue' if label == 'OT' else 'black'], align='c')
        icon = self.type_icon(s['type'])
        paste(im, icon, X(200) + 0, Ry[72] + 0)
        # KAI's TRAINER MEMO on the stripes
        my = top + len(right_rows) + 1
        self.text(im, X(112), my, s['nature'] + ' nature.', *self.SUM_INK['black'])
        # (as Platinum's memo writes a place: no stop after it; so KAI's longest, Lv100 EVER GRANDE CITY, fits)
        self.text(im, X(112) - 2, my + 16, 'Lv%d %s' % (s['level'], s['place'].rstrip('.')), *self.SUM_INK['black'])

    def page_art(self, png, scr, pal):
        src = Image.open(png)
        px = src.load()
        w, h = src.size
        ts = [[px[tx * 8 + x, ty * 8 + y] % 16 for y in range(8) for x in range(8)]
              for ty in range(h // 8) for tx in range(w // 8)]
        mw, mh, ents = self.nscr(scr)
        return self.map_rows(ts, ents, mw // 8, 0, 0, 32, 24, pal)

    # ---------------------------------------------------------------------------------------------- the BAG
    def bag_page(self, im):
        """Platinum's BAG (src/applications/bag): its sky, the BAG label, the bag on its pedestal (here beside the
        label, as KAI sets its bag beside its title: the bag is 58 tall and the GBA's screen 32 shorter), the
        pocket plate (its icon bar holding KAI's eight pockets, Platinum's icons, the current red; the pocket's
        name in its dark field; the plate's POCKET tab left out for the room), the list in item_list_border's white
        panel (FONT_SYSTEM, TEXT_COLOR(1, 2, 0), the count at ITEM_COUNT_START_POS) with the item highlight on the
        cursor's row and KAI's START SORT under the items, and the purple description band with the item's icon in
        its white box and the words in TEXT_COLOR(15, 14, 0) from x 40"""
        from common import BAG_ITEMS, BAG_DESC, BAG_POCKET, POCKETS, item_icon
        d = os.path.join(R, 'res', 'graphics', 'bag')
        pal = self.jasc(os.path.join(d, 'bag_ui_main.pal'))
        ts = self.gray_tiles(os.path.join(d, 'bag_ui_main_tileset.png'))
        mw, _, ents = self.nscr(os.path.join(d, 'bag_ui_main.NSCR'))
        page = self.map_rows(ts, ents, mw // 8, 0, 0, 32, 24, pal)
        pp = page.load()
        dark, lite = (16, 24, 32), (172, 189, 189)
        # left column: the label and sky rows 0-17 and 52-83 (the pedestal), the plate 84-123 and one orange row,
        # rows 140-191 the description band
        rows = list(range(0, 18)) + list(range(35, 84)) + list(range(84, 124)) + [124] + list(range(140, 192))
        # BG3 (item_list_border: white, the list's frame) under BG1 (bag_ui_main, clear where the list and the
        # pocket's name show through); both cut the same way: the left column by ROWS, the right by RROWS / RCOLS
        lw, _, lents = self.nscr(os.path.join(d, 'item_list_border.NSCR'))
        lb = self.map_rows(ts, lents, lw // 8, 0, 0, 32, 24, pal)
        lp = lb.load()
        rrows = list(range(0, 96)) + list(range(124, 136)) + list(range(140, 192))
        rcols = list(range(104, 220)) + list(range(236, 256))     # (136: sixteen of the list's plain columns out)
        for layer in (lp, pp):
            for oy, sy in enumerate(rows):
                for x in range(104):
                    c = layer[x, sy]
                    if c[3]:
                        im.putpixel((x, oy), c)
            for oy, sy in enumerate(rrows):
                for ox, sx in enumerate(rcols):
                    c = layer[sx, sy]
                    if c[3]:
                        im.putpixel((104 + ox, oy), c)
        # the bag beside the label, on the pedestal
        bag = self.indexed(os.path.join(d, 'bag_sprite_male.png')).crop((0, 64, 64, 128))
        bag = bag.crop(bag.getbbox())
        Ly = {sy: i for i, sy in enumerate(rows)}
        paste(im, bag, 72 - bag.width // 2, Ly[76] - bag.height)
        # the pocket icons on the plate's bar (KAI's eight: FAVORITES has MAIL's place and envelope)
        icons = self.indexed(os.path.join(d, 'pocket_selector_icons.png'))
        order = [0, 1, 2, 3, 4, 6, 7, 5]
        cur = POCKETS.index(BAG_POCKET)
        for i, k in enumerate(order):
            ic = icons.crop((32 * k + (16 if i == cur else 0), 0, 32 * k + 16 + (16 if i == cur else 0), 16))
            paste(im, ic, 4 + 12 * i, Ly[90] - 2)
        name = BAG_POCKET
        self.text(im, 52, Ly[106], name, dark, lite, align='c')
        # the list: FONT_SYSTEM rows 16 apart from the panel's top
        ly0 = 13
        hl = self.indexed(os.path.join(d, 'item_highlight.png'))
        hl = hl.crop(hl.getbbox())
        for i, (n, q, _item) in enumerate(BAG_ITEMS):
            y = ly0 + 16 * i
            if i == 0:
                paste(im, self.slice9(hl, 124, 16, 4, 4, 4, 4), 110, y)
            self.text(im, 116, y, n, dark, lite)
            self.text(im, 196, y, 'x', dark, lite)
            self.text(im, 226, y, str(q), dark, lite, align='r')
        self.text(im, 226, ly0 + 16 * 4 + 2, 'START  SORT', (106, 139, 180), (164, 180, 205), align='r')
        # the description band
        ic = item_icon(self.export, 13)
        if ic:
            paste(im, ic, 18 - ic.width // 2, 134 - ic.height // 2)
        for i, line in enumerate(BAG_DESC):
            self.text(im, 40, 112 + 16 * i, line, (255, 255, 255), (0, 0, 0))

    # ---------------------------------------------------------------------------------------------- OPTIONS
    def options_page(self, im):
        """Platinum's OPTIONS (src/applications/options_menu.c): the backdrop (config_gra tile 1), the entries in a
        system-frame window 16 pixels a row (labels, then the choices from x 100, the chosen red: TEXT_COLOR(3, 4,
        15), the rest TEXT_COLOR(1, 2, 15)), the red cursor round the cursor's row (tilemap.bin's two rows), the
        message box under it; KAI's five tabs where Platinum titles the screen (the open one red), and in the
        message box KAI's buttons (Platinum describes the row there; KAI's rows have no descriptions)"""
        from common import OPTION_ROWS, OPTION_TABS
        d = os.path.join(R, 'res', 'graphics', 'options_menu')
        src = Image.open(os.path.join(d, 'tiles.png'))
        sp = src.getpalette()
        tpal = [tuple(sp[3 * v:3 * v + 3]) for v in range(16)]
        im.paste(tpal[1] + (255,), (0, 0, W, H))
        ink, red = self.FIELD_INK, ((232, 32, 16), (248, 168, 184))
        # the tabs along the top
        widths = [self.measure(t) for t in OPTION_TABS]
        gap = (232 - sum(widths)) // (len(OPTION_TABS) - 1)
        x = 4
        for t, w in zip(OPTION_TABS, widths):
            self.text(im, x, 0, t, *(red if t == 'DISPLAY' else ink))
            x += w + gap
        # the entries' window
        tx, ty, tw, th = 1, 3, 28, 12
        self.std_frame(im, tx, ty, tw, th, 'standard_system')
        im.paste((255, 255, 255, 255), (tx * 8, ty * 8, (tx + tw) * 8, (ty + th) * 8))
        # the cursor's red outline (tilemap.bin rows 0-1: tile 3 the corner, 4 the edge; flipped for the rest)
        px = src.load()

        def tile(k, hf=False, vf=False):
            t = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
            for y in range(8):
                for xx in range(8):
                    v = px[k * 8 + xx, y]
                    if v:
                        t.putpixel((7 - xx if hf else xx, 7 - y if vf else y), tpal[v] + (255,))
            return t
        cy = ty * 8
        for c in range(tw):
            k = 3 if c in (0, tw - 1) else 4
            hf = c == tw - 1
            paste(im, tile(k, hf, False), (tx + c) * 8, cy)
            paste(im, tile(k, hf, True), (tx + c) * 8, cy + 8)
        for i, (name, vals) in enumerate(OPTION_ROWS):
            y = ty * 8 + 16 * i
            self.text(im, tx * 8 + 6, y, name, *ink)
            if vals is None:
                self.text(im, tx * 8 + 100, y, self.name, *red)
            else:
                xx = tx * 8 + 100
                for k, v in enumerate(vals):
                    xx = self.text(im, xx, y, v, *(red if k == len(vals) - 1 else ink)) + 10
        # the message box: KAI's buttons
        self.message_frame(im, 2, 17, 25, 2)
        self.text(im, 16, 136, 'L R  Tab     A  Choose     B  Back', *self.FIELD_INK, font='msg')
