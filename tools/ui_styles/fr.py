"""FIRERED: FireRed's own interface (pret/pokefirered: its graphics, and where its code puts each window and line of
text), with KAI's functions in it.

Battles keep KAI's layout (layout.py): FireRed's healthboxes sit exactly where KAI's do (its boxes are Emerald's
shape), its HP bar is the game's own bar tiles (healthbox_elements: the HP label, 0-8 pixel fills), its names and
levels are its small font; Double Battles take its doubles boxes two rows shorter so the pairs never touch, and the
Triple Battle cards are cut from its doubles box (corners, edges, tail and the bar's end cap kept) to KAI's 77 x 20.
The command and move panels are FireRed's battle textbox (textbox.png / .bin) with the text where its windows put it.

The menus are FireRed's: the message box (menu_message frame), the START menu (the user frame) with the help bar
(help_system/msg_window), the party screen (party_menu bg and slots), the summary (its INFO page and header, the
header's page dots widened to KAI's six pages), the BAG (item_menu bg: the pocket's name over the bag, the list, the
description beside the item's icon; KAI's eight pockets and START: SORT added in its own pieces), and OPTION (its
help bar, header window and list, KAI's tabs in the header)."""
import os
import re
import struct
import sys

from PIL import Image

import layout as Lo
from base import Style
from common import (ACTIONS, BAG_DESC, BAG_ITEMS, BAG_POCKET, MESSAGE, MOVES, OPTION_ROWS, OPTION_TABS, PARTY,
                    POCKETS, PROMPT, START_MENU, SUMMARY, H, W, creature, item_icon, paste, shot)
import firered as _assets

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'stadium', 'restyle'))
import s2  # noqa: E402  (the game's character codes)

EXTRA = {'♂': 0xB5, '♀': 0xB6, '×': 0xB9, '/': 0xBA, '▶': 0xEF, '…': 0xB0, '&': 0x2D, '+': 0x2E, '{LV}': 0x34}

# text colours, from the palettes FireRed prints them with
STD = ((98, 98, 98), (213, 213, 205))               # windows: dark grey on white
BATTLE = ((74, 74, 74), (214, 214, 206))            # the battle menus (battle_bg.c palette 5: RGB(9,9,9) / (26,26,25))
BATTLE_MSG = ((255, 255, 255), (106, 90, 115))      # the battle message (textbox palette 1 / 6)
HBOX = ((65, 65, 65), (222, 213, 180))              # healthbox (palette 1 / 3)
WHITE = ((255, 255, 255), (98, 98, 98))             # headers, the help bars
RED = ((230, 8, 8), (255, 189, 115))                # chosen values
MALE, FEMALE = ((65, 205, 255), (222, 213, 180)), ((255, 156, 148), (222, 213, 180))
MALE_W, FEMALE_W = ((164, 197, 246), (49, 82, 205)), ((255, 189, 115), (230, 8, 8))
BLUE_BAR = (0, 123, 197)                            # stdpal_2's fill: the help bars


def jasc(path):
    return _assets.jasc(path)


def png_pal(im):
    return _assets.png_pal(im)


def code(ch):
    if ch in EXTRA:
        return EXTRA[ch]
    k = s2._code(ch)
    if k == s2._code('?') and ch != '?':
        # (FireRed's font holds the same codes as KAI's charset: what the short table lacks is drawn at its own code,
        # never as a ?)
        import uipack
        k = uipack.charmap().get(ch, uipack.EXTRA_CODES.get(ch, k))
    return k


def tokens(s):
    """a string's characters, '{LV}' as one"""
    out, i = [], 0
    while i < len(s):
        if s.startswith('{LV}', i):
            out.append('{LV}')
            i += 4
        else:
            out.append(s[i])
            i += 1
    return out


def wrap_px(words, width, measure):
    out, line = [], ''
    for w in words.split():
        t = (line + ' ' + w).strip()
        if line and measure(t) > width:
            out.append(line)
            line = w
        else:
            line = t
    return out + ([line] if line else [])


class FireRed(Style):
    name = 'FIRERED'
    small = 's'
    # (what check.py finds that is FireRed's own layout: party_menu.h sPartyBoxInfoRects puts a wide slot's level,
    # gender and HP numbers on its lower band, their foot on the slot's edge and the "/" reaching up to the bar,
    # as the game itself and KAI's Emerald party screen show)
    # (party_menu.h sPartyBoxInfoRects: a slot's second row, the level, gender and HP, ends on the slot's foot)
    ORIGINAL = [('party', 'collide',
                 lambda f: re.search(r'\)-\(\d+,(31|55|79|103|127)\)$', f) is not None,
                 "FireRed's own party slot layout")]

    def __init__(self, root, kai, export):
        super().__init__(kai, export)
        self.root = root
        self.a = _assets.FireRed(root, kai, export)        # (FireRed's pictures, tiles and tilemaps)
        self.g = self.a.g
        src = open(os.path.join(root, 'src', 'text.c')).read()

        def widths(name):
            body = re.search(name + r'\[\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
            return [int(v) for v in re.findall(r'\d+', re.sub(r'//.*', '', body))]
        self.fonts = {'n': (Image.open(os.path.join(self.g, 'fonts', 'latin_normal.png')),
                            widths('sFontNormalLatinGlyphWidths'), 16, 16),
                      's': (Image.open(os.path.join(self.g, 'fonts', 'latin_small.png')),
                            widths('sFontSmallLatinGlyphWidths'), 8, 16)}
        self.keypad = Image.open(os.path.join(self.g, 'fonts', 'keypad_icons.png'))
        self.hbpal = jasc(os.path.join(self.g, 'battle_interface', 'healthbox.pal'))
        self.barpal = jasc(os.path.join(self.g, 'battle_interface', 'healthbar.pal'))
        el = Image.open(os.path.join(self.g, 'battle_interface', 'healthbox_elements.png'))
        self.elements = self._tiles(el, self.barpal)
        self.frames = {n: self._sheet_tiles(os.path.join(self.g, 'text_window', n + '.png'))
                       for n in ('type1', 'menu_message', 'std')}
        self.frames['std'] = self._sheet_tiles(os.path.join(self.g, 'text_window', 'std.png'),
                                               jasc(os.path.join(self.g, 'text_window', 'stdpal_3.pal')))
        self.help = self._sheet_tiles(os.path.join(self.g, 'help_system', 'msg_window.png'))
        # (the battle textbox: palette 0 the message side, palette 1 the menus: textbox1 + textbox2)
        tpal = jasc(os.path.join(self.g, 'battle_interface', 'textbox1.pal')) + \
            jasc(os.path.join(self.g, 'battle_interface', 'textbox2.pal'))
        self.textbox = self.a._tilemap('battle_interface/textbox.png', 'battle_interface/textbox.bin', tpal, 32, 64)
        self.exp_tiles = self._tiles(el, self.hbpal)     # (the EXP bar's tiles take the healthbox's blues)

    # ---------------------------------------------------------------------------------------------- pictures
    @staticmethod
    def _tiles(im, pal, base=0):
        p, out = im.load(), []
        for ty in range(im.height // 8):
            for tx in range(im.width // 8):
                t = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
                q = t.load()
                for y in range(8):
                    for x in range(8):
                        v = p[tx * 8 + x, ty * 8 + y] % 16
                        if v:
                            q[x, y] = tuple(pal[base + v][:3]) + (255,)
                out.append(t)
        return out

    def _sheet_tiles(self, path, pal=None):
        im = Image.open(path)
        return self._tiles(im, pal or png_pal(im))

    # ---------------------------------------------------------------------------------------------- words
    def measure(self, s, font='n'):
        _, wd, _, _ = self.fonts[font]
        return sum(wd[code(c)] for c in tokens(s))

    def text(self, im, x, y, s, col=None, shadow=None, font='n', align='l'):
        ink, shade = (col, shadow) if col else STD
        if col and shadow is None:
            shade = None
        if align == 'r':
            x -= self.measure(s, font)
        elif align == 'c':
            x -= self.measure(s, font) // 2
        sheet, wd, cw, ch = self.fonts[font]
        per = sheet.width // cw
        p, gp = im.load(), sheet.load()
        for c in tokens(s):
            k = code(c)
            gx, gy = (k % per) * cw, (k // per) * ch
            for yy in range(ch):
                for xx in range(wd[k]):
                    v = gp[gx + xx, gy + yy]
                    X, Y = x + xx, y + yy
                    if 0 <= X < im.width and 0 <= Y < im.height and v in (1, 2):
                        c2 = ink if v == 1 else shade
                        if c2:
                            p[X, Y] = c2 + (255,)
            x += wd[k]
        return x

    def key(self, im, x, y, which, ink=None):
        """one of FireRed's keypad icons (fonts/keypad_icons.png: A, B, L, R, START, SELECT, the d-pad's)"""
        idx, w = {'A': (0x0, 8), 'B': (0x1, 8), 'L': (0x2, 16), 'R': (0x4, 16), 'START': (0x6, 24),
                  'SELECT': (0x9, 24), 'UP': (0xC, 8), 'DOWN': (0xD, 8), 'LEFT': (0xE, 8), 'RIGHT': (0xF, 8),
                  'UPDOWN': (0x20, 8), 'LEFTRIGHT': (0x21, 8)}[which]
        pal = png_pal(self.keypad)
        kp, p = self.keypad.load(), im.load()
        sx, sy = (idx % 16) * 8, (idx // 16) * 8      # (a glyph's lower rows are the tile row below)
        for yy in range(12):
            for xx in range(w):
                v = kp[sx + xx, sy + yy]
                if v == 0:
                    continue
                col = (255, 255, 255) if v == 1 else (98, 98, 98) if v == 2 else pal[v][:3]
                if 0 <= x + xx < im.width and 0 <= y + yy < im.height:
                    p[x + xx, y + yy] = col + (255,)
        return x + w

    # ---------------------------------------------------------------------------------------------- windows
    def frame(self, im, r, kind='type1'):
        """a window frame drawn as FireRed draws it round a window: r is the frame's outside, in whole tiles"""
        x0, y0, x1, y1 = r
        ts = self.frames[kind]
        im.paste((255, 255, 255, 255), (x0 + 8, y0 + 8, x1 - 8, y1 - 8))
        for ty in range(y0, y1, 8):
            for tx in range(x0, x1, 8):
                row = 0 if ty == y0 else 2 if ty == y1 - 8 else 1
                col = 0 if tx == x0 else 2 if tx == x1 - 8 else 1
                if (row, col) != (1, 1):
                    im.alpha_composite(ts[row * 3 + col], (tx, ty))

    def dialogue(self, im, r):
        """the message box (text_window/menu_message, DrawDialogueFrame's tiles: two columns each side, the bottom
        rows the top's flipped)"""
        x0, y0, x1, y1 = r
        ts = self.frames['menu_message']
        im.paste((255, 255, 255, 255), (x0 + 16, y0 + 8, x1 - 16, y1 - 8))
        rows = (y1 - y0) // 8
        flip = lambda t: t.transpose(Image.FLIP_TOP_BOTTOM)  # noqa: E731
        for j in range(rows):
            y = y0 + j * 8
            if j == 0 or j == rows - 1:
                ids = (0, 1, 2, 3, 4)
            elif j in (1, rows - 2):
                ids = (5, 6, None, 8, 9)
            else:
                ids = (10, 11, None, 12, 13)
            fl = j >= rows / 2
            put = lambda t, x: im.alpha_composite(flip(ts[t]) if fl else ts[t], (x, y))  # noqa: E731
            put(ids[0], x0)
            put(ids[1], x0 + 8)
            if ids[2] is not None:
                for x in range(x0 + 16, x1 - 16, 8):
                    put(ids[2], x)
            put(ids[3], x1 - 16)
            put(ids[4], x1 - 8)

    def help_bar(self, im, r):
        """the blue help bar (help_system/msg_window: a top row, middle rows, a bottom row)"""
        x0, y0, x1, y1 = r
        for y in range(y0, y1, 8):
            t = self.help[0 if y == y0 else 14 if y == y1 - 8 else 5]
            for x in range(x0, x1, 8):
                im.alpha_composite(t, (x, y))

    def window(self, im, r, kind='menu'):
        if kind == 'message':
            self.dialogue(im, r)
        else:
            self.frame(im, r)

    def cursor(self, im, x, y, ink=None):
        self.text(im, x, y, '▶', *(ink or STD))

    def more_arrow(self, im, x, y):
        sheet = Image.open(os.path.join(self.g, 'fonts', 'down_arrows.png'))
        pal = png_pal(sheet)
        sp, p = sheet.load(), im.load()
        for yy in range(16):
            for xx in range(16):
                v = sp[xx, yy]
                if v > 1:
                    p[x + xx, y + yy] = pal[v][:3] + (255,)

    # ---------------------------------------------------------------------------------------------- HUD pieces
    def bar(self, im, x, y, frac, tiles=6, label=True, cut=None):
        """FireRed's HP bar: the HP label (elements 1-2) and TILES bar tiles (3 + pixels filled); CUT trims the bar
        to a width that isn't whole tiles (the triple cards)"""
        if label:
            im.alpha_composite(self.elements[1], (x, y))
            im.alpha_composite(self.elements[2], (x + 8, y))
            x += 16
        width = cut or tiles * 8
        filled = round(width * frac)
        strip = Image.new('RGBA', (tiles * 8, 8), (0, 0, 0, 0))
        for k in range(tiles):
            n = max(0, min(8, filled - k * 8))
            strip.alpha_composite(self.elements[3 + n], (k * 8, 0))
        im.alpha_composite(strip.crop((0, 0, width, 8)), (x, y))

    def exp_bar(self, im, x, y, frac, tiles=8, bottom=None):
        """FireRed's EXP bar tiles (12 + pixels filled), cut at BOTTOM (the box's own last row)"""
        filled = round(tiles * 8 * frac)
        rows = 8 if bottom is None else max(0, min(8, bottom - y))
        for k in range(tiles):
            n = max(0, min(8, filled - k * 8))
            im.alpha_composite(self.exp_tiles[12 + n].crop((0, 0, 8, rows)), (x + k * 8, y))

    def text_clipped(self, im, x, y, s, clip, *a, **k):
        layer = Image.new('RGBA', im.size, (0, 0, 0, 0))
        self.text(layer, x, y, s, *a, **k)
        im.alpha_composite(layer.crop(clip), clip[:2])

    def sex(self, im, x, y, s, font='s', white=False):
        cols = (MALE_W if s == 'm' else FEMALE_W) if white else (MALE if s == 'm' else FEMALE)
        return self.text(im, x, y, '♂' if s == 'm' else '♀', *cols, font=font)

    def art(self, kind, lv=False):
        """a healthbox's art cropped to its own pixels; its printed Lv mark lifted off (LV keeps it), the level
        then printed by the font where it fits every name"""
        b = self.a.healthbox(kind)
        b = b.crop(b.getbbox())
        if not lv:
            p = b.load()
            for y in range(6, 16):
                for x in range(48, 80):
                    if p[x, y][:3] in ((65, 65, 65), (222, 213, 180)):
                        p[x, y] = (255, 255, 222, 255)
        return b

    @staticmethod
    def drop_rows(img, rows):
        keep = [y for y in range(img.height) if y not in rows]
        out = Image.new('RGBA', (img.width, len(keep)), (0, 0, 0, 0))
        for k, y in enumerate(keep):
            out.paste(img.crop((0, y, img.width, y + 1)), (0, k))
        return out

    @staticmethod
    def drop_col(img, x):
        out = Image.new('RGBA', (img.width - 1, img.height), (0, 0, 0, 0))
        out.paste(img.crop((0, 0, x, img.height)), (0, 0))
        out.paste(img.crop((x + 1, 0, img.width, img.height)), (x, 0))
        return out

    @staticmethod
    def bar_slot(img):
        """where FireRed's HP bar goes in a box: found from the bar's end cap printed in the art (its darkest green,
        (57,82,65), just inside the right frame): the bar's tile row starts two rows above the cap and the bar stops
        at the cap's first column, as the game's sprite does (battle_interface.c: the bar at +24..+88 on the foe's
        box, +32..+96 on the player's; the cap's ")" printed just past it)"""
        p = img.load()
        mid = img.height // 3
        frame = max(x for x in range(img.width) if p[x, mid][3] and p[x, mid][:3] == (32, 57, 0))
        pts = [(x, y) for y in range(4, img.height - 4) for x in range(frame - 8, frame - 1)
               if p[x, y][3] and p[x, y][:3] == (57, 82, 65)]
        top = min(y for x, y in pts)
        cap = min(x for x, y in pts if y <= top + 6)
        return cap, top - 2               # (the bar's end, exclusive: the cap's first column; its tile row)

    def card(self, mine, w, h=20):
        """a Triple Battle card cut from FireRed's doubles box to KAI's size (W x 20): its dark top line and the
        corner's next step, the plain middle for the name, the seven bar rows with the bar's end cap, the two
        bottom rows. Every column takes the same rows, so the tail keeps its place against the box as in the
        game's art (it ends above the box's bottom corner, where FireRed's ends)"""
        b = self.art('pdouble' if mine else 'fdouble')        # 100 x 26, the tail left (mine) or right (foe)
        rows = [1, 3] + [6] * 9 + list(range(15, 22)) + [24, 25]
        assert len(rows) == h
        tall = Image.new('RGBA', (b.width, h), (0, 0, 0, 0))
        for k, y in enumerate(rows):
            tall.paste(b.crop((0, y, b.width, y + 1)), (0, k))
        bb = tall.getbbox()
        tall = tall.crop((bb[0], 0, bb[2], h))                 # (the card's own extent, tail included)
        # columns: the left 12 and the right 20 (frame, end cap, tail) kept, a plain middle column repeated
        left, right = 12, 20
        out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        out.paste(tall.crop((0, 0, left, h)), (0, 0))
        mid = tall.crop((30, 0, 31, h))
        for x in range(left, w - right):
            out.paste(mid, (x, 0))
        out.paste(tall.crop((tall.width - right, 0, tall.width, h)), (w - right, 0))
        return out

    # ---------------------------------------------------------------------------------------------- the HUD
    def hud(self, im, r, m, kind):
        x0, y0, x1, y1 = r
        frac = m['hp'] / m['maxhp'] if 'maxhp' in m else m['hp']
        mine = kind.startswith('mine')
        dx = 8 if mine else 0                  # (the player's boxes start after their tail)
        right = x0 + (95 if mine else 87)      # (the box's inside edge, where the level ends)
        if kind in ('foe', 'mine'):
            # (FireRed's single boxes are a row taller than KAI's, the player's a column wider: the drop shadow's
            # outermost row goes, and a column inside the EXP strip's shadow, so the box itself is FireRed's own)
            art = self.art('player' if mine else 'foe')
            art = art.crop((0, 0, art.width, art.height - 1))
            if mine:
                art = self.drop_col(art, 102)
            ty = y0 + 1
            nx = x0 + (16 if mine else 7)                       # (the name's window: tile 2 / tile 1)
            right = x0 + (95 if mine else 86)                    # (the level's window ends there)
        elif kind in ('foe2', 'mine2'):
            art = self.drop_rows(self.art('pdouble' if mine else 'fdouble'), {6, 7})
            ty = y0 - 1
            nx = x0 + (15 if mine else 7)
            right = x0 + (94 if mine else 86)
        else:
            return self.hud_card(im, r, m, mine, frac)
        if kind == 'mine':
            # (the HP numbers' window, x 56-95, rows 21-31 of the box: the game prints over the art's own "/")
            p = art.load()
            for y in range(19, 30):
                for x in range(56, 96):
                    p[x, y] = (255, 255, 222, 255)
        paste(im, art, x0, y0)
        end, by = self.bar_slot(art)
        self.text(im, nx, ty, m['name'], *HBOX, font='s')
        self.sex(im, nx + self.measure(m['name'], 's'), ty, m['sex'])
        self.text(im, right, ty, '{LV}%d' % m['level'], *HBOX, font='s', align='r')
        self.bar(im, x0 + end - 64, y0 + by, frac)
        if kind == 'mine':
            # " 56/" at 60 and " 56" at 80 (UpdateHpTextInHealthbox), cut at the window's last row as the game's is
            self.text_clipped(im, x0 + 60, y0 + 19, '%3d/' % m['hp'], (0, 0, W, y0 + 30), *HBOX, font='s')
            self.text_clipped(im, x0 + 80, y0 + 19, '%3d' % m['maxhp'], (0, 0, W, y0 + 30), *HBOX, font='s')
            self.exp_bar(im, x0 + 32, y0 + 30, m['exp'], bottom=y0 + art.height)

    @staticmethod
    def card_text_x(card):
        """where a card's words start: two pixels inside its left frame (the frame's dark line found on the name
        rows)"""
        p = card.load()
        frame = min(x for x in range(card.width) if p[x, 5][3] and p[x, 5][:3] == (32, 57, 0))
        return frame + 3

    def hud_card(self, im, r, m, mine, frac):
        x0, y0, x1, y1 = r
        card = self.card(mine, x1 - x0, y1 - y0)
        paste(im, card, x0, y0)
        end, by = self.bar_slot(card)
        tx = x0 + self.card_text_x(card)
        self.text(im, tx, y0 - 2, m['name'], *HBOX, font='s')
        self.sex(im, tx + self.measure(m['name'], 's'), y0 - 2, m['sex'])
        lx = self.text(im, tx, y0 + 6, '{LV}%d' % m['level'], *HBOX, font='s')
        bx = max(lx + 1, tx + 19)
        self.bar(im, bx, y0 + by, frac, tiles=6, cut=x0 + end - (bx + 16))

    # ---------------------------------------------------------------------------------------------- battle panels
    def battle_msg(self, im, r=Lo.BATTLE_MSG):
        paste(im, self.textbox.crop((0, 272, 240, 320)), 0, 112)
        self.text(im, 10, 122, PROMPT[0], *BATTLE_MSG)
        self.text(im, 10, 138, PROMPT[1], *BATTLE_MSG)

    def battle_menu(self, im, r=Lo.BATTLE_MENU):
        for i, a in enumerate(ACTIONS):
            self.text(im, 136 + (i % 2) * 56, 122 + (i // 2) * 16, a, *BATTLE)
        self.cursor(im, 128, 122, BATTLE)

    def move_panel(self, im):
        paste(im, self.textbox.crop((0, 432, 240, 480)), 0, 112)
        for i, (n, *_r) in enumerate(MOVES):
            self.text(im, 16 + (i % 2) * 72, 121 + (i // 2) * 16, n, *BATTLE, font='s')
        self.cursor(im, 8, 120, BATTLE)
        n, pp, mx, kind = MOVES[0]
        self.text(im, 168, 122, 'PP', *BATTLE, font='s')
        self.text(im, 230, 122, '%d/%d' % (pp, mx), *BATTLE, align='r')
        self.text(im, 168, 138, 'TYPE/' + kind, *BATTLE, font='s')

    # ---------------------------------------------------------------------------------------------- field
    def message(self):
        im = shot(self.kai, 'f_mart')
        self.dialogue(im, (0, 112, 240, 160))
        x = self.text(im, 16, 121, MESSAGE)
        self.more_arrow(im, x + 1, 121)
        return im

    def start_menu(self):
        """FireRed's START menu (the user frame at tiles 22-28, a row each 15 pixels) and its help bar"""
        im = shot(self.kai, 'f_town')
        self.frame(im, (168, 0, 240, 120))
        for i, s in enumerate(START_MENU):
            self.text(im, 184, 9 + 15 * i, s)
        self.cursor(im, 176, 9)
        self.help_bar(im, (0, 120, 240, 160))
        self.text(im, 2, 125, 'The MOéDEX keeps your record of', *WHITE)
        self.text(im, 2, 141, 'every MOéMON seen and caught.', *WHITE)
        return im

    # ---------------------------------------------------------------------------------------------- party
    def party(self):
        a = self.a
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        bg = Image.open(os.path.join(self.g, 'party_menu', 'bg.png'))
        pal = png_pal(bg)
        paste(im, a._tilemap('party_menu/bg.png', 'party_menu/bg.bin', pal, 32, 20), 0, 0)
        tiles = self._tiles(bg, pal, 48)
        sel = list(pal)
        for off, idx in zip((4, 5, 6, 1, 7, 8), (116, 117, 118, 97, 103, 104)):
            sel[48 + off] = pal[idx]
        sel_tiles = self._tiles(bg, sel, 48)

        def slot(name, w, h, x, y, current):
            d = open(os.path.join(self.g, 'party_menu', name), 'rb').read()
            ts = sel_tiles if current else tiles
            for k, t in enumerate(d[:w * h]):
                im.alpha_composite(ts[t], (x + (k % w) * 8, y + (k // w) * 8))

        ink = (pal[48 + 3][:3], pal[48 + 2][:3])                  # (sFontColorTable[0]: white, grey shadow)
        male, female = (pal[48 + 11][:3], pal[48 + 12][:3]), (pal[64 + 11][:3], pal[64 + 12][:3])
        slot('slot_main.bin', 10, 7, 8, 24, True)
        for i in range(5):
            slot('slot_wide.bin' if i < len(PARTY) - 1 else 'slot_wide_empty.bin', 18, 3, 96, 8 + i * 24, False)
        # (party_menu.h sPartyBoxInfoRects: nickname, level, gender, HP, max HP, the bar, all in the small font)
        for i, m in enumerate(PARTY):
            if i == 0:
                x, y, rect = 8, 24, (24, 11, 32, 20, 64, 20, 38, 36, 53, 36, 24, 35)
                paste(im, creature(self.export, m['species'], 'icon'), 16 - 16, 40 - 16)
            else:
                x, y, rect = 96, 8 + (i - 1) * 24, (22, 3, 32, 12, 64, 12, 102, 12, 117, 12, 88, 10)
                paste(im, creature(self.export, m['species'], 'icon'), 104 - 16, 18 + 24 * (i - 1) - 16)
            nx, ny, lx, ly, gx, gy, hx, hy, mx, my, bx, by = rect
            self.text(im, x + nx, y + ny, m['name'], *ink, font='s')
            self.text(im, x + lx, y + ly, '{LV}%d' % m['level'], *ink, font='s')
            self.text(im, x + gx, y + gy, '♂' if m['sex'] == 'm' else '♀', *(male if m['sex'] == 'm' else female),
                      font='s')
            self.text(im, x + hx + 20, y + hy, '%d/' % m['hp'], *ink, font='s', align='r')
            self.text(im, x + mx + 20, y + my, '%d' % m['maxhp'], *ink, font='s', align='r')
            self.hp_fill(im, x + bx, y + by, 48, m['hp'] / m['maxhp'])
        self.frame(im, (0, 128, 184, 160))
        self.text(im, 8, 137, 'Choose a MOéMON.')
        self.text(im, 216, 135, 'CANCEL', *ink, font='s', align='c')
        return im

    def hp_fill(self, im, x, y, w, frac):
        n = round(w * frac)
        rows = [(90, 213, 131), (115, 255, 172), (115, 255, 172)]
        for k, c in enumerate(rows):
            im.paste(c + (255,), (x, y + k, x + n, y + k + 1))

    # ---------------------------------------------------------------------------------------------- summary
    def summary_page(self, im):
        a = self.a
        bg = Image.open(os.path.join(self.g, 'summary_screen', 'bg.png'))
        pal = png_pal(bg)
        im.paste(pal[0][:3] + (255,), (0, 0, W, H))
        back = a._tilemap('summary_screen/bg.png', 'summary_screen/moves_info_page.bin', pal, 32, 20)
        # (the header's page dots: the cream band up to tile 11, its white dot for this page, then KAI's five others)
        d = open(os.path.join(self.g, 'summary_screen', 'moves_info_page.bin'), 'rb').read()
        ent = list(struct.unpack('<%dH' % (len(d) // 2), d))
        for row in (0, 1):
            r = ent[row * 32:row * 32 + 32]
            band, cap, edge, dot, end, blue = r[0], r[13], r[14], r[15], r[18], r[19]
            new = [band] * 10 + [cap, edge] + [dot] * 5 + [end] + [blue] * 14
            ent[row * 32:row * 32 + 32] = new
        back = a._tilemap('summary_screen/bg.png', None, pal, 32, 20, entries=ent)
        paste(im, back, 0, 0)
        paste(im, a._tilemap('summary_screen/bg.png', 'summary_screen/page_info.bin', pal, 32, 20), 0, 0)
        s = SUMMARY
        self.text(im, 4, 1, 'MOéMON INFO', *WHITE)
        x = 236 - self.measure('PAGE', 's') - self.measure('CANCEL', 's') - 8 - 8 - 4
        x = self.key(im, x, 2, 'LEFTRIGHT')
        x = self.text(im, x, 0, 'PAGE', *WHITE, font='s') + 4
        x = self.key(im, x, 2, 'A')
        self.text(im, x, 0, 'CANCEL', *WHITE, font='s')
        # the level / name row, the picture
        lx = self.text(im, 4, 18, '{LV}', *WHITE)
        self.text(im, lx, 18, str(s['level']), *WHITE)
        self.text(im, 40, 18, s['name'], *WHITE)
        self.sex(im, 105, 18, s['sex'], font='n', white=True)
        paste(im, creature(self.export, 280), 28, 32)
        # the right pane: No., NAME, TYPE, OT, IDNo, ITEM
        ink = (pal[6 * 16 + 14][:3], pal[6 * 16 + 10][:3])
        if sum(ink[0]) > 500:
            ink = ((65, 65, 65), (222, 213, 180))
        self.text(im, 167, 21, s['no'], *ink)
        self.text(im, 167, 35, s['species'], *ink)
        self.type_icon(im, 167, 52, s['type'])
        self.text(im, 167, 65, s['ot'], *ink)
        self.text(im, 167, 80, s['idno'], *ink)
        # (FireRed's names are 12 letters at most, KAI's 13: one that runs past the pane is in FireRed's small font)
        self.text(im, 167, 95, s['item'], *ink, **({} if self.measure(s['item']) <= 238 - 167 else {'font': 's'}))
        self.text(im, 8, 115, s['nature'] + ' nature.', *ink)
        self.text(im, 8, 129, 'Met in %s at {LV}%d.' % (s['place'].rstrip('.'), s['level']), *ink)
        # KAI's RENAME, in FireRed's keypad style, on the memo's empty third line
        w = 24 + 2 + self.measure('RENAME', 's')
        kx = self.key(im, 228 - w, 146, 'SELECT')
        self.text(im, kx + 2, 144, 'RENAME', *ink, font='s')

    def type_icon(self, im, x, y, kind):
        """FireRed's type icon (interface/menu_info.png, list_menu.c sMenuInfoIcons: FIRE at tile 0x24)"""
        sheet = Image.open(os.path.join(self.g, 'interface', 'menu_info.png'))
        pal = jasc(os.path.join(self.g, 'interface', 'pokemon_types.pal'))
        off = {'FIRE': 0x24, 'NORMAL': 0x20}.get(kind, 0x24)
        sx, sy = (off % 16) * 8, (off // 16) * 8      # (BlitBitmapRectToWindow over the tiles from OFF)
        sp, p = sheet.load(), im.load()
        for yy in range(12):
            for xx in range(32):
                v = sp[sx + xx, sy + yy] % 16
                if v:
                    p[x + xx, y + yy] = pal[v][:3] + (255,)

    # ---------------------------------------------------------------------------------------------- BAG
    def bag_page(self, im):
        a = self.a
        bg = Image.open(os.path.join(self.g, 'item_menu', 'bg.png'))
        paste(im, a._tilemap('item_menu/bg.png', 'item_menu/bg.bin', png_pal(bg), 32, 20), 0, 0)
        bagpic = a._sheet('interface/bag_male.png', png_pal(Image.open(os.path.join(self.g, 'interface',
                                                                                       'bag_male.png'))))
        # (the BAG open on the ITEMS side for KAI's MEDICINE: item_menu_icons.c sAnims_Bag's frame 2)
        paste(im, bagpic.crop((0, 128, 64, 192)), 40 - 32, 68 - 32)
        # the pocket's name on the band, KAI's eight pockets as the band's arrows and a row of pips
        self.text(im, 44, 9, BAG_POCKET, *WHITE, align='c')
        self.arrow(im, 8 - 8, 72 - 8, left=True)       # (item_menu.c sPocketSwitchArrowPairTemplate)
        self.arrow(im, 72 - 8, 72 - 8, left=False)
        k = POCKETS.index(BAG_POCKET)
        for i in range(len(POCKETS)):
            x = 8 + i * 9
            im.paste((98, 98, 98, 255), (x, 97, x + 6, 103))
            im.paste(((255, 255, 255) if i == k else (148, 205, 205)) + (255,), (x + 1, 98, x + 5, 102))
        # the list: item_X 9, the quantity in the small font at 110, rows 16 apart from 10
        for i, (name, n, _item) in enumerate(BAG_ITEMS):
            self.text(im, 97, 10 + i * 16, name)
            self.text(im, 198, 10 + i * 16, '×%2d' % n, font='s')
        self.text(im, 97, 10 + len(BAG_ITEMS) * 16, 'CLOSE BAG')
        self.cursor(im, 89, 10, RED)
        # START: SORT, in FireRed's keypad style, on the list's foot
        kx = self.key(im, 160, 91, 'START')
        self.text(im, kx + 2, 89, 'SORT', font='s')
        ic = item_icon(self.export, BAG_ITEMS[0][2])
        if ic:
            paste(im, ic, 24 - 16, 140 - 16)     # (item_menu_icons.c: the icon's sprite at 24, 140)
        for i, ln in enumerate(wrap_px(' '.join(BAG_DESC), 196, lambda t: self.measure(t))[:3]):
            self.text(im, 40, 115 + 14 * i, ln, *WHITE)

    def arrow(self, im, x, y, left):
        """a pocket arrow: FireRed's scroll arrow (interface/red_arrow_other.png frame 0, the left chevron; turned
        round for the right), 16 x 16"""
        sheet = Image.open(os.path.join(self.g, 'interface', 'red_arrow_other.png'))
        pal = jasc(os.path.join(self.g, 'interface', 'red_arrow.pal'))
        t = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
        sp, q = sheet.load(), t.load()
        for yy in range(16):
            for xx in range(16):
                v = sp[xx, yy] % 16
                if v:
                    q[xx, yy] = pal[v][:3] + (255,)
        if not left:
            t = t.transpose(Image.FLIP_LEFT_RIGHT)
        im.alpha_composite(t, (x, y))

    # ---------------------------------------------------------------------------------------------- OPTION
    def options_page(self, im):
        im.paste((0, 0, 0, 255), (0, 0, W, H))
        # the help bar along the top
        im.paste(BLUE_BAR + (255,), (0, 0, W, 16))
        x = 228 - (8 + self.measure('PICK', 's') + 4 + 8 + self.measure('SWITCH', 's') + 4 + 32 +
                   self.measure('TAB', 's') + 4 + 16 + self.measure('CANCEL', 's'))
        x = self.key(im, x, 2, 'UPDOWN')
        x = self.text(im, x, 0, 'PICK', *WHITE, font='s') + 4
        x = self.key(im, x, 2, 'LEFTRIGHT')
        x = self.text(im, x, 0, 'SWITCH', *WHITE, font='s') + 4
        x = self.key(im, x, 2, 'L')
        x = self.key(im, x, 2, 'R')
        x = self.text(im, x, 0, 'TAB', *WHITE, font='s') + 4
        x = self.key(im, x, 2, 'A')
        x = self.key(im, x, 2, 'B')
        self.text(im, x, 0, 'CANCEL', *WHITE, font='s')
        # the header window (std frame): KAI's tabs where FireRed writes OPTION
        self.frame(im, (8, 16, 232, 48), 'std')
        gap = (208 - sum(self.measure(t) for t in OPTION_TABS)) // (len(OPTION_TABS) - 1)
        x = 16
        for t in OPTION_TABS:
            x = self.text(im, x, 25, t, *(RED if t == 'DISPLAY' else STD)) + gap
        # the list window (user frame): the names at 8, the chosen value at 130, rows 13 apart
        self.frame(im, (8, 48, 232, 160))
        vals = ['FIRERED', 'LIGHT', 'TYPE 1', 'ON', 'MODERN', 'ON']
        for i, ((name, _v), val) in enumerate(zip(OPTION_ROWS, vals)):
            y = 58 + i * 13
            self.text(im, 24, y, name)
            self.text(im, 146, y, val, *RED)
        # (the picked row stands out as FireRed's does: the others lightened, white over them at 48 / 255)
        for i in range(1, len(OPTION_ROWS)):
            paste(im, Image.new('RGBA', (208, 13), (255, 255, 255, 48)), 16, 58 + 13 * i)
