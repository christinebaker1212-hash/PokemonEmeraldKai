"""RED and CRYSTAL in KAI's layout: the Game Boy games' own tiles (pret/pokered, pret/pokecrystal gfx/font: the font,
the box borders, the HUD's HP: label, bar, :L and brackets; Crystal's window frame, PACK picture and STATS colours),
drawn tile by tile where KAI puts its own pieces. The battle boxes keep the Game Boy's bracket HUD on white, so the
words stay readable over any scene; RED in the Super Game Boy's greys with a coloured HP bar, CRYSTAL in the Game Boy
Color's colours (its EXP bar, gender marks, page colours)."""
import os

from PIL import Image

import layout as Lo
from base import Style
from common import (BAG_DESC, BAG_ITEMS, BAG_POCKET, OPTION_ROWS, OPTION_TABS, SUMMARY, H, W, creature, hp_colour,
                    item_icon, paste, shot)

CODES = {**{chr(65 + i): 0x80 + i for i in range(26)}, **{chr(97 + i): 0xA0 + i for i in range(26)},
         **{str(i): 0xF6 + i for i in range(10)}, '(': 0x9A, ')': 0x9B, ':': 0x9C, ';': 0x9D, '[': 0x9E, ']': 0x9F,
         'é': 0xBA, "'": 0xE0, '-': 0xE3, '?': 0xE6, '!': 0xE7, '.': 0xE8, '▶': 0xED, '▼': 0xEE, '♂': 0xEF,
         '¥': 0xF0, '×': 0xF1, '/': 0xF3, ',': 0xF4, '♀': 0xF5,
         '’': 0xE0, '‘': 0xE0}   # (the game's ' is its ’)


def wrap(s, n):
    out, line = [], ''
    for w in s.split():
        if line and len(line) + 1 + len(w) > n:
            out.append(line)
            line = w
        else:
            line = (line + ' ' + w).strip()
    return out + ([line] if line else [])


class GB(Style):
    FIT = 'inside'   # (the Game Boy's HUDs keep their own size: inside KAI's boxes, anchored to their side)
    def __init__(self, kind, pret, kai, export, pack=None):
        super().__init__(kai, export)
        self.kind, self.pret = kind, pret
        # (the BAG's picture: Crystal's PACK; RED, which has none, borrows it in its own greys)
        self.pack_png = pack or os.path.join(pret, 'gfx', 'pack', 'pack.png')
        self.name = kind.upper()
        g = os.path.join(pret, 'gfx', 'font')
        self.font = self._tiles(os.path.join(g, 'font.png'))
        self.extra = self._tiles(os.path.join(g, 'font_extra.png'))
        self.batt = self._tiles(os.path.join(g, 'font_battle_extra.png'))
        # (the battle HUD's own tiles, by the numbers the games load them at: RED gfx/battle/battle_hud_1-3 at $6D-$6F
        # and $73-$78; CRYSTAL gfx/battle/enemy_hp_bar_border at $6C-$6F, hp_exp_bar_border at $73-$78, expbar at
        # $55-$5D)
        b = os.path.join(pret, 'gfx', 'battle')
        self.hud_tiles = {}
        if kind == 'crystal':
            for base, name in ((0x6C, 'enemy_hp_bar_border'), (0x73, 'hp_exp_bar_border'), (0x55, 'expbar')):
                for k, t in enumerate(self._tiles(os.path.join(b, name + '.png'))):
                    self.hud_tiles[base + k] = t
        else:
            for base, name in ((0x6D, 'battle_hud_1'), (0x73, 'battle_hud_2'), (0x76, 'battle_hud_3')):
                for k, t in enumerate(self._tiles(os.path.join(b, name + '.png'))):
                    self.hud_tiles[base + k] = t
        if kind == 'crystal':
            self.frame = self._tiles(os.path.join(pret, 'gfx', 'frames', '1.png'))
            self.ink, self.paper, self.mid = (24, 24, 32), (248, 248, 248), (136, 144, 168)
        else:
            e = self.extra
            self.frame = [e[25], e[26], e[27], e[28], e[29], e[30]]
            self.ink, self.paper, self.mid = (32, 32, 32), (248, 248, 240), (160, 160, 152)

    @staticmethod
    def _tiles(path):
        im = Image.open(path).convert('L')
        return [im.crop((tx * 8, ty * 8, tx * 8 + 8, ty * 8 + 8)) for ty in range(im.height // 8)
                for tx in range(im.width // 8)]

    # ---------------------------------------------------------------------------------------------- tiles
    def tile(self, im, t, x, y, ink=None, paper=None):
        ink = ink or self.ink
        p = im.load()
        for yy in range(8):
            for xx in range(8):
                X, Y = x + xx, y + yy
                if not (0 <= X < im.width and 0 <= Y < im.height):
                    continue
                v = t.getpixel((xx, yy))
                if v < 64:
                    p[X, Y] = ink + (255,)
                elif v < 160:
                    p[X, Y] = self.mid + (255,)
                elif paper is not None:
                    p[X, Y] = paper + (255,)

    # letters KAI's words use that the Game Boy fonts don't have, drawn in their thin strokes
    SYNTH = {'+': ('........', '........', '...#....', '...#....', '.#####..', '...#....', '...#....', '........'),
             '=': ('........', '........', '........', '.######.', '........', '.######.', '........', '........'),
             '%': ('........', '.##...#.', '.##..#..', '....#...', '...#....', '..#..##.', '.#...##.', '........'),
             '&': ('..##....', '.#..#...', '.#..#...', '..##....', '.#..#.#.', '.#...#..', '..###.#.', '........')}

    def glyph(self, ch):
        if ch == '◀':
            return self.font[0xED - 0x80].transpose(Image.FLIP_LEFT_RIGHT)
        if ch == '…':
            return self.extra[0x75 - 0x60]   # (charmap.asm's "…", $75: font_extra)
        if ch == '&' and self.kind == 'crystal':
            return self.font[0xE9 - 0x80]   # (Crystal's own &: charmap.asm $E9)
        if ch in self.SYNTH:
            t = Image.new('L', (8, 8), 255)
            for y, row in enumerate(self.SYNTH[ch]):
                for x, c in enumerate(row):
                    if c == '#':
                        t.putpixel((x, y), 0)
            return t
        c = 0xEA if (ch == 'é' and self.kind == 'crystal') else CODES.get(ch, 0xE6)
        return self.font[c - 0x80]

    def text(self, im, x, y, s, col=None, shadow=None, font='n', align='l'):
        if align == 'r':
            x -= self.measure(s)
        elif align == 'c':
            x -= self.measure(s) // 2
        for i, ch in enumerate(s):
            if ch != ' ':
                if ch in '♂♀' and col is None:
                    self.tile(im, self.glyph(ch), x + i * 8, y,
                              (56, 112, 232) if ch == '♂' else (232, 64, 104))
                else:
                    self.tile(im, self.glyph(ch), x + i * 8, y, col)
        return x + 8 * len(s)

    def measure(self, s, font='n'):
        return 8 * len(s)

    def fill(self, im, r, col):
        im.paste(col + (255,), r)

    def window(self, im, r, kind='menu'):
        x0, y0, x1, y1 = r
        self.fill(im, r, self.paper)
        # (the frame's six tiles as both games lay them, TextboxBorder: ┌ ─ ┐ along the top, │ down both sides,
        # └ ─ ┘ along the foot: the same ─ and │ tiles, never turned; the corners meet them)
        f = self.frame
        for x in list(range(x0 + 8, x1 - 8, 8)) + [x1 - 16]:
            self.tile(im, f[1], x, y0)
            self.tile(im, f[1], x, y1 - 8)
        for y in list(range(y0 + 8, y1 - 8, 8)) + [y1 - 16]:
            self.tile(im, f[3], x0, y)
            self.tile(im, f[3], x1 - 8, y)
        self.tile(im, f[0], x0, y0)
        self.tile(im, f[2], x1 - 8, y0)
        self.tile(im, f[4], x0, y1 - 8)
        self.tile(im, f[5], x1 - 8, y1 - 8)

    def cursor(self, im, x, y):
        self.text(im, x, y, '▶')

    # (text boxes as both games print them: the lines on the box's 2nd and 4th rows inside, from its 2nd column:
    # hlcoord 1, 14 and 1, 16 in a box from row 12)
    def message(self):
        from common import MESSAGE
        im = shot(self.kai, 'f_mart')
        self.window(im, Lo.FIELD_MSG, 'message')
        self.text(im, 8, 128, MESSAGE)
        self.more_arrow(im, 0, 0)
        return im

    def battle_msg(self, im, r=Lo.BATTLE_MSG):
        from common import PROMPT
        self.window(im, r, 'battle_msg')
        self.text(im, r[0] + 8, r[1] + 16, PROMPT[0])
        self.text(im, r[0] + 8, r[1] + 32, PROMPT[1])

    def battle_menu(self, im, r=Lo.BATTLE_MENU):
        from common import ACTIONS
        self.window(im, r, 'battle_menu')
        x0, y0 = r[0], r[1]
        for i, a in enumerate(ACTIONS):
            self.text(im, x0 + 16 + (i % 2) * 56, y0 + 16 + (i // 2) * 16, a)
        self.cursor(im, x0 + 8, y0 + 16)

    def more_arrow(self, im, x, y):
        # (the ▼ where both games put it: the frame's last column inside, on the last line, tile (18, 16) of their
        # 20-tile box; here the box is 30 tiles: column 28)
        self.text(im, 224, 144, '▼')

    # ---------------------------------------------------------------------------------------------- the HUD
    def hp_bar(self, im, x, y, frac, tiles=6, label=True):
        """HP: and a bar of TILES tiles (8 steps each, its fill in the HP's colour) and its end"""
        b = self.batt
        if self.kind == 'red':
            hp, steps, end = [b[15], b[0]], b[1:10], b[10]
        else:
            hp, steps, end = [b[0], b[1]], b[2:11], b[11]
        if label:
            for k, t in enumerate(hp):
                self.tile(im, t, x + 8 * k, y)
            x += 16
        n = round(frac * 8 * tiles)
        col = hp_colour(frac, (48, 184, 72), (232, 184, 0), (232, 64, 48))
        p = im.load()
        for k in range(tiles):
            part = max(0, min(8, n - 8 * k))
            t = steps[part]
            self.tile(im, t, x + 8 * k, y)
            for yy in range(3, 5):   # (the fill's own rows, the grey ones: the outline above and below stays)
                for xx in range(part):
                    if 64 <= t.getpixel((xx, yy)) < 160:
                        p[x + 8 * k + xx, y + yy] = col + (255,)
        self.tile(im, end, x + 8 * tiles, y)
        return x + 8 * tiles + 8

    def level(self, im, x, y, n):
        self.tile(im, self.batt[12 if self.kind == 'red' else 14], x, y)
        self.text(im, x + 8, y, str(n))
        return x + 8 + 8 * len(str(n))

    def sex(self, im, x, y, s):
        if self.kind == 'crystal':
            self.text(im, x, y, '♂' if s == 'm' else '♀')

    # (the battle boxes: each the Game Boy game's own HUD, tile for tile where its code places them, on a grid of
    # tiles; only empty pixel rows come out where KAI's box is shorter, never a line of a tile)
    def bt(self, k):
        """a battle tile by its number: the HUD's own, else the battle font's"""
        if k in self.hud_tiles:
            return self.hud_tiles[k]
        return self.batt[k - (0x62 if self.kind == 'red' else 0x60)]

    def put(self, c, k, col, row, fill=None, frac=None):
        """tile K at grid (COL, ROW); FILL colours its bar pixels (rows 3-4, the grey ones)"""
        t = self.bt(k)
        GB.tile(self, c, t, col * 8, row * 8)
        if fill:
            p = c.load()
            for yy in range(3, 5):
                for xx in range(8):
                    if 64 <= t.getpixel((xx, yy)) < 160:
                        p[col * 8 + xx, row * 8 + yy] = fill + (255,)

    def hp_tiles(self, c, col, row, frac, end, label=True, tiles=6):
        """the HP bar as DrawHPBar lays it: HP: (RED $71 $62, CRYSTAL $60 $61), TILES bar tiles (empty + pixels
        filled), and END. LABEL 'edge' (RED, where there is no room for HP:) opens the bar with its own end turned
        round, the bar's other side"""
        red = self.kind == 'red'
        if label == 'edge':
            GB.tile(self, c, self.bt(0x6C).transpose(Image.FLIP_LEFT_RIGHT), col * 8, row * 8)
            col += 1
        elif label:
            self.put(c, 0x71 if red else 0x60, col, row)
            self.put(c, 0x62 if red else 0x61, col + 1, row)
            col += 2
        n = round(frac * 8 * tiles)
        fill = hp_colour(frac, (48, 184, 72), (232, 184, 0), (232, 64, 48))
        for k in range(tiles):
            part = max(0, min(8, n - 8 * k))
            self.put(c, (0x63 if red else 0x62) + part, col + k, row, fill)
        self.put(c, end, col + tiles, row)

    def grid_text(self, c, col, row, s):
        GB.text(self, c, col * 8, row * 8, s)

    def grid_level(self, c, col, row, n):
        """PrintLevel: ":L" and two digits; three digits write over the ":L" (both games)"""
        if n >= 100:
            self.grid_text(c, col, row, str(n))
            return
        self.put(c, 0x6E, col, row)
        self.grid_text(c, col + 1, row, str(n))

    def hud_grid(self, kind, m, frac):
        red = self.kind == 'red'
        if kind == 'foe':
            # PlaceEnemyHUDTiles / DrawEnemyHUDBorder from (1, 2); the name at (1, 0), the level at (4, 1) (RED) or
            # (6, 1) with the gender at (9, 1) (CRYSTAL); the bar from (2, 2)
            c = Image.new('RGBA', (80, 32), (0, 0, 0, 0))
            self.grid_text(c, 0, 0, m['name'])
            if red:
                self.grid_level(c, 3, 1, m['level'])
            else:
                self.grid_level(c, 5, 1, m['level'])
                GB.text(self, c, 64, 8, '♂' if m['sex'] == 'm' else '♀')
            self.put(c, 0x73 if red else 0x6D, 0, 2)
            self.hp_tiles(c, 1, 2, frac, 0x6C if red else 0x6B)
            self.put(c, 0x74, 0, 3)
            for k in range(1, 9):
                self.put(c, 0x76, k, 3)
            self.put(c, 0x78, 9, 3)
            return c
        if kind == 'mine':
            # PlacePlayerHUDTiles / DrawPlayerHUDBorder from (18, 10) leftwards and $73 at (18, 9); the name at
            # (10, 7), the level at (14, 8) (CRYSTAL: the gender at (17, 8)), the bar from (10, 9), its numbers at
            # (11, 10); CRYSTAL's EXP bar along the border's foot from (17, 11) leftwards (FillInExpBar). The
            # name's ten letters run a tile past the border's end, to the screen's last column, as in both games
            c = Image.new('RGBA', (88, 40), (0, 0, 0, 0))
            self.grid_text(c, 1, 0, m['name'])
            self.grid_level(c, 5, 1, m['level'])
            if not red:
                GB.text(self, c, 64, 8, '♂' if m['sex'] == 'm' else '♀')
            self.hp_tiles(c, 1, 2, frac, 0x6D if red else 0x6B)
            self.grid_text(c, 2, 3, '%3d/%3d' % (m['hp'], m['maxhp']))
            self.put(c, 0x73, 9, 3)
            self.put(c, 0x6F, 0, 4)
            if red:
                for k in range(1, 9):
                    self.put(c, 0x76, k, 4)
            else:
                b = round(64 * m['exp'])
                blue = (32, 136, 248)
                for k in range(8, 0, -1):               # (from the right, as PlaceExpBar fills it)
                    if b >= 8:
                        self.put(c, 0x6A, k, 4, blue)
                        b -= 8
                    elif b > 0:
                        self.put(c, 0x54 + b, k, 4, blue)
                        b = 0
                    else:
                        self.put(c, 0x62, k, 4)
            self.put(c, 0x77, 9, 4)
            return c
        # (Double Battles, which the Game Boy games never had: the same pieces on three rows, the level before the
        # bar (RED: the bar opened by its own edge; CRYSTAL: by its HP:), the border's line under them)
        c = Image.new('RGBA', (96, 24), (0, 0, 0, 0))
        foe = kind == 'foe2'
        nc = 0 if foe else 1
        self.grid_text(c, nc, 0, m['name'])
        if not red:
            GB.text(self, c, (nc + len(m['name'])) * 8, 0, '♂' if m['sex'] == 'm' else '♀')
        if foe:
            self.put(c, 0x73 if red else 0x6D, 0, 1)
        self.grid_level(c, 1, 1, m['level'])
        if red:
            self.hp_tiles(c, 4, 1, frac, 0x6C if foe else 0x6D, label='edge', tiles=6)
        else:
            self.hp_tiles(c, 4, 1, frac, 0x6B, label=True, tiles=5)
        self.put(c, 0x74 if foe else 0x6F, 0, 2)
        for k in range(1, 11):
            self.put(c, 0x76, k, 2)
        self.put(c, 0x78 if foe else 0x77, 11, 2)
        return c

    @staticmethod
    def fit_rows(c, h):
        """take empty pixel rows out of C until it is H tall: the empty foot first, then rows from the widest gap
        (a row is empty when it is blank or holds just what the row beside it holds: a running line); a gap of
        one row between two lines of ink is never closed (it raises instead, so a box that can't fit is seen)"""
        rows = [[c.getpixel((x, y)) for x in range(c.width)] for y in range(c.height)]
        blank = lambda r: all(p[3] == 0 for p in r)  # noqa: E731
        while len(rows) > h and blank(rows[-1]):
            rows.pop()
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
            if not runs:
                raise ValueError('a HUD can not be fitted to %d rows without closing a gap' % h)
            best = max(runs, key=len)
            rows.pop(best[len(best) // 2])
        out = Image.new('RGBA', (c.width, len(rows)), (0, 0, 0, 0))
        p = out.load()
        for y, r in enumerate(rows):
            for x, v in enumerate(r):
                p[x, y] = v
        return out

    def hud(self, im, r, m, kind):
        x0, y0, x1, y1 = r
        foe = kind.startswith('foe')
        frac = m['hp'] if foe else m['hp'] / m['maxhp']
        if kind in ('foe', 'mine', 'foe2', 'mine2'):
            c = self.fit_rows(self.hud_grid(kind, m, frac), y1 - y0)
            self.on_white(im, r, c, x0 + 1 if foe else x1 - 1 - c.width)
            return
        # (a Triple Battle card: the same pieces on a nine-tile grid, the bar after the level with a tile between,
        # the border's line and wedge underneath; empty rows out to KAI's 20)
        c = self.card_grid(foe, m, frac)
        self.on_white(im, r, c, x0 + 2 if foe else x1 - 2 - c.width)

    def on_white(self, im, r, c, x):
        """a HUD on the Game Boy's white (so it reads over any scene), the white hugging the HUD a pixel round,
        inside KAI's box and anchored to its side"""
        x0, y0, x1, y1 = r
        y = y0 + (y1 - y0 - c.height) // 2
        bx0, by0, bx1, by1 = c.getbbox()
        im.paste(self.paper + (230,), (max(x0, x + bx0 - 1), max(y0, y + by0 - 1),
                                       min(x1, x + bx1 + 1), min(y1, y + by1 + 1)))
        paste(im, c, x, y)

    def card_grid(self, foe, m, frac):
        """a card 20 rows tall: the name, a blank row, the level and bar, and the border's tile row raised so its
        line lies on the last row (RED's line is on its tile's rows 3-4, CRYSTAL's on row 6): only the border
        tiles' empty top rows and the corner's line down meet the row above"""
        red = self.kind == 'red'
        c = Image.new('RGBA', (72, 24), (0, 0, 0, 0))
        border = Image.new('RGBA', (72, 8), (0, 0, 0, 0))
        # (nine tiles hold nine letters; a tenth sets the name's letters a pixel closer, the only way ten fit)
        if len(m['name']) > 9:
            for i, ch in enumerate(m['name']):
                if ch != ' ':
                    self.tile(c, self.glyph(ch), i * 7, 0)
        else:
            self.grid_text(c, 0, 0, m['name'])
        self.grid_level(c, 0, 1, m['level'])
        col = (1 if m['level'] < 100 else 0) + len(str(m['level']))
        if not red:
            GB.text(self, c, col * 8, 8, '♂' if m['sex'] == 'm' else '♀')
            col += 1
        # (RED: the bar opened by its own edge; CRYSTAL: by its HP:. The player's card closes its border as the
        # player's HUD does: the bar's end (RED's with the line down) over the corner $77)
        if red:
            self.hp_tiles(c, col, 1, frac, 0x6C if foe else 0x6D, label='edge', tiles=8 - col - 1)
        else:
            self.hp_tiles(c, col, 1, frac, 0x6B, label=True, tiles=8 - col - 2)
        if foe:
            for k in range(0, 8):
                self.put(border, 0x76, k, 0)
            self.put(border, 0x78, 8, 0)
        else:
            self.put(border, 0x6F, 0, 0)
            for k in range(1, 8):
                self.put(border, 0x76, k, 0)
            self.put(border, 0x77, 8, 0)
        out = c.crop((0, 0, 72, 20))
        out.alpha_composite(border, (0, 20 - (5 if red else 7)))
        return out

    def move_panel(self, im):
        from common import MOVES
        self.window(im, Lo.MOVES)
        x0, y0, _, _ = Lo.MOVES
        for i, (n, *_r) in enumerate(MOVES):
            self.text(im, x0 + 16, y0 + 8 + 8 * i, n)
        self.cursor(im, x0 + 8, y0 + 8)
        self.window(im, Lo.MOVE_INFO)
        x0, y0, x1, _ = Lo.MOVE_INFO
        n, pp, mx, kind = MOVES[0]
        self.text(im, x0 + 8, y0 + 8, 'TYPE/')
        self.text(im, x0 + 16, y0 + 16, kind)
        self.text(im, x1 - 8, y0 + 28, '%d/%d' % (pp, mx), align='r')

    # ---------------------------------------------------------------------------------------------- the menus
    # (each the Game Boy game's own screen, filled with KAI's things: six POKéMON, eight pockets, five OPTIONS tabs,
    # the summary's pages)
    def start_menu(self):
        from common import START_MENU
        im = shot(self.kai, 'f_town')
        # (DrawStartMenu: the box from (10, 0), the first item at (12, 2), one every two rows, the ▶ in column 11)
        self.window(im, (160, 0, 240, 128))
        for i, n in enumerate(START_MENU):
            self.text(im, 176, 16 + 16 * i, n)
        self.cursor(im, 168, 16)
        if self.kind == 'crystal':   # (Crystal's MENU ACCOUNT, _DrawMenuAccount: a box 10 tiles wide, three rows
            # inside, the words on its 1st and 3rd)
            self.window(im, (0, 120, 96, 160))
            self.at(im, 1, 16, 'MOéMON')
            self.at(im, 1, 18, 'database')
        return im

    def party(self):
        """RED / CRYSTAL's party list: each POKéMON a line of name and :L, a line of HP: and its numbers"""
        from common import PARTY
        im = Image.new('RGBA', (W, H), self.paper + (255,))
        for i, m in enumerate(PARTY):
            y = 2 + 22 * i
            paste(im, creature(self.export, m['species'], 'icon'), 6, y - 7)
            self.text(im, 40, y + 2, m['name'])
            self.sex(im, 40 + self.measure(m['name']), y + 2, m['sex'])
            self.level(im, 176, y + 2, m['level'])
            self.hp_bar(im, 48, y + 11, m['hp'] / m['maxhp'])
            self.text(im, 232, y + 11, '%3d/%3d' % (m['hp'], m['maxhp']), align='r')
        self.cursor(im, 0, 4)
        self.window(im, (0, 136, 240, 160))
        self.text(im, 8, 144, 'Choose a MOéMON.')
        return im

    # ---------------------------------------------------------------------------------------------- tile grids
    def at(self, im, col, row, s, ink=None):
        """words on the tile grid, as the Game Boy prints them"""
        self.text(im, col * 8, row * 8, s, ink)

    def tile_at(self, im, t, col, row, ink=None):
        GB.tile(self, im, t, col * 8, row * 8, ink)

    def numero(self, im, col, row, n):
        """№. and a number, as both games print a POKéDEX number: the № tile ($74 of the battle font, which the
        status and STATS screens load there; in battle the HUD's corner takes $74), '.'"""
        self.tile_at(im, self.batt[0x74 - (0x62 if self.kind == 'red' else 0x60)], col, row)
        self.at(im, col + 1, row, '.' + n)

    def line_box(self, im, right, top, bottom, left):
        """RED's DrawLineBox (status screen): a line down column RIGHT from row TOP, the corner at BOTTOM, the
        line back along BOTTOM to column LEFT, its wedge there"""
        for r in range(top, bottom):
            self.tile_at(im, self.bt(0x73), right, r)
        self.tile_at(im, self.bt(0x77), right, bottom)
        for c in range(left + 1, right):
            self.tile_at(im, self.bt(0x76), c, bottom)
        self.tile_at(im, self.bt(0x6F), left, bottom)

    @staticmethod
    def _tiles_2bpp(path):
        d = open(path, 'rb').read()
        out = []
        for t in range(len(d) // 16):
            im = Image.new('L', (8, 8))
            for y in range(8):
                lo, hi = d[t * 16 + y * 2], d[t * 16 + y * 2 + 1]
                for x in range(8):
                    v = ((lo >> (7 - x)) & 1) | (((hi >> (7 - x)) & 1) << 1)
                    im.putpixel((x, y), (255, 192, 96, 0)[v])
            out.append(im)
        return out

    def summary_page(self, im):
        if self.kind == 'crystal':
            return self.stats_crystal(im)
        # RED's status screen (engine/pokemon/status_screen.asm), on its own tile grid from the top left: the
        # name at (9, 1), :L at (14, 2), the HP bar at (11, 3) (its end joined to the line box), the numbers
        # below, STATUS/ at (9, 6), OK at (16, 6), №. under the picture; the line boxes round TYPE1 / IDNo / OT
        # (from (10, 9) in RED). KAI's own: the picture (8 x 8 tiles), ABILITY / NATURE / MET in the box where
        # RED has its stats, the page and RENAME in the columns RED doesn't have.
        s = SUMMARY
        im.paste(self.paper + (255,), (0, 0, W, H))
        paste(im, creature(self.export, 280), 0, 0)
        self.at(im, 9, 1, s['name'])
        self.level(im, 14 * 8, 2 * 8, s['level'])
        c = Image.new('RGBA', (W, 8), (0, 0, 0, 0))
        self.hp_tiles(c, 11, 0, 1.0, 0x6D)
        paste(im, c, 0, 3 * 8)
        self.at(im, 12, 4, '%3d/%3d' % (56, 56))
        self.at(im, 9, 6, 'STATUS/')
        self.at(im, 16, 6, 'OK')
        for r in (1, 2, 4, 5, 6):
            self.tile_at(im, self.bt(0x73), 19, r)
        self.tile_at(im, self.bt(0x77), 19, 7)
        for col in range(10, 19):
            self.tile_at(im, self.bt(0x76), col, 7)
        self.tile_at(im, self.bt(0x6F), 9, 7)
        self.numero(im, 1, 8, s['no'])
        self.at(im, 29 - 2, 1, '1/6')
        self.at(im, 21, 5, 'SELECT')
        self.at(im, 22, 6, ':RENAME')
        self.window(im, (0, 9 * 8, 19 * 8, H))
        for k, (a, v) in enumerate([('ABILITY/', s['ability']), ('NATURE/', s['nature']),
                                    ('MET/', 'INSIDE OF TRUCK')]):
            self.at(im, 1, 10 + 3 * k, a)
            self.at(im, 2, 11 + 3 * k, v)
        # (TYPE1/, then TYPE2/ for a second type (TORCHIC has none), IDNo/, OT/: two rows each, as RED's)
        for k, row in enumerate([('TYPE1/', s['type']), None, ('IDNo/', s['idno']), ('OT/', s['ot'])]):
            if row is None:
                continue
            a, v = row
            self.at(im, 20, 10 + 2 * k, a)
            self.at(im, 21 if k == 0 else 22, 11 + 2 * k, v)
        self.line_box(im, 29, 10, 19, 19)

    def stats_crystal(self, im):
        """Crystal's STATS, its pink first page (engine/pokemon/stats_screen.asm) on its own tile grid: №. at
        (8, 0), :L at (14, 0), the gender at (18, 0), the name at (8, 2), /species at (9, 4), the page squares
        on row 5, the divider along the row under the picture; below it HP / STATUS / TYPE on the left of the
        upright divider; on its right KAI's ABILITY / NATURE / MET (where Crystal counts EXP), RENAME at the foot"""
        s = SUMMARY
        tiles = self._tiles_2bpp(os.path.join(self.pret, 'gfx', 'stats', 'stats_tiles.2bpp'))
        st = {0x31 + k: t for k, t in enumerate(tiles)}
        pink = (248, 176, 232)
        page_cols = [(232, 96, 184), (96, 200, 72), (72, 136, 232), (232, 160, 72), (160, 120, 232), (88, 184, 200)]
        im.paste(pink + (255,), (0, 0, W, H))
        im.paste(self.paper + (255,), (0, 0, W, 8 * 8))
        paste(im, creature(self.export, 280), 0, 0)
        self.numero(im, 8, 0, s['no'])
        self.level(im, 14 * 8, 0, s['level'])
        self.text(im, 18 * 8, 0, '♂' if s['sex'] == 'm' else '♀')
        self.at(im, 8, 2, s['name'])
        self.at(im, 9, 4, '/' + s['species'])
        for k in range(6):                         # (KAI's six pages; the first large, as Crystal marks its page)
            base = 0x3A if k == 0 else 0x36
            for j in range(4):
                t, col, row = st[base + j], 9 + 2 * k + j % 2, 5 + j // 2
                GB.tile(self, im, t, col * 8, row * 8)
                p = im.load()
                for yy in range(8):
                    for xx in range(8):
                        if 64 <= t.getpixel((xx, yy)) < 160:
                            p[col * 8 + xx, row * 8 + yy] = page_cols[k] + (255,)
        for col in range(30):
            self.tile_at(im, self.bt(0x62), col, 7)
        for row in range(8, 20):
            self.tile_at(im, st[0x31], 9, row)
        c = Image.new('RGBA', (W, 8), (0, 0, 0, 0))
        self.hp_tiles(c, 0, 0, 1.0, 0x6B)
        paste(im, c, 0, 9 * 8)
        self.at(im, 1, 10, '%3d/%3d' % (56, 56))
        self.at(im, 0, 12, 'STATUS/')
        self.at(im, 1, 13, 'OK')
        self.at(im, 0, 15, 'TYPE/')
        self.at(im, 1, 16, s['type'])
        for k, (a, v) in enumerate([('ABILITY/', s['ability']), ('NATURE/', s['nature']),
                                    ('MET/', 'INSIDE OF TRUCK')]):
            self.at(im, 10, 9 + 3 * k, a)
            self.at(im, 11, 10 + 3 * k, v)
        self.at(im, 16, 18, 'SELECT:RENAME')

    def bag_page(self, im):
        if self.kind == 'crystal':
            return self.pack(im)
        # RED's ITEM list over the field: its box (as engine/menus/list_menu: four rows shown, two rows each, the
        # quantity on the second, ▼ when more follow), KAI's pocket across its top; KAI's words below, two rows
        # apart as RED's text box prints
        im.paste(shot(self.kai, 'f_town'))
        self.window(im, (32, 0, 240, 88))
        self.at(im, 17 - (len(BAG_POCKET) + 2) // 2 + 1, 1, '◀' + BAG_POCKET + '▶')
        for i, (n, q, _it) in enumerate(BAG_ITEMS[:4]):
            self.at(im, 6, 2 + 2 * i, n)
            self.at(im, 24, 3 + 2 * i, '×%2d' % q)
        self.text(im, 28 * 8, 9 * 8, '▼')
        self.cursor(im, 5 * 8, 2 * 8)
        self.window(im, (0, 88, 240, 160))
        lines = wrap(' '.join(BAG_DESC), 28)
        for k, ln in enumerate(lines[:7]):     # (more than four lines close up to a row apart)
            self.at(im, 1, 12 + (2 * k if len(lines) <= 4 else k), ln)

    def pack(self, im):
        """Crystal's PACK (engine/items/pack.asm): the header strip along row 0 (its own tiles, from
        gfx/pack/pack_menu), the blue ground, the PACK at (0, 3), the pocket's name under it, the list on white from
        column 5 (here 10, for KAI's longer pockets), two rows an item; KAI's START: SORT in the header's spare
        columns; the words in the box below, two rows apart"""
        blue = [(248, 248, 248), (120, 152, 248), (24, 56, 200), (16, 16, 24)]
        menu = self._tiles(os.path.join(self.pret, 'gfx', 'pack', 'pack_menu.png'))
        im.paste((120, 152, 248, 255), (0, 0, W, H))
        im.paste((16, 16, 24, 255), (0, 0, W, 8))
        for col in range(20):
            GB.tile(self, im, menu[0x28 + col], col * 8, 0, self.paper, (16, 16, 24))
        self.at(im, 20, 0, 'START:SORT', self.paper)
        pic = self.gbc_picture(self.pack_png, blue, (0, 24, 40, 48))     # (PackGFX frame 1: the ITEM pocket's)
        paste(im, pic, 3 * 8 - 4 + 4, 3 * 8)
        self.fill(im, (0, 7 * 8, 10 * 8, 8 * 8), self.paper)
        self.at(im, (10 - len(BAG_POCKET)) // 2, 7, BAG_POCKET)
        self.fill(im, (10 * 8, 8, W, 12 * 8), self.paper)
        for i, (n, q, _it) in enumerate(BAG_ITEMS):
            self.at(im, 12, 2 + 2 * i, n)
            self.at(im, 26, 3 + 2 * i, '×%2d' % q)
        self.at(im, 12, 10, 'CANCEL')
        self.cursor(im, 11 * 8, 2 * 8)
        # (the words' box from row 12, as Crystal's text box: the list's last count, on row 11, stays clear of it)
        self.window(im, (0, 12 * 8, W, H))
        lines = wrap(' '.join(BAG_DESC), 28)
        for k, ln in enumerate(lines[:6]):     # (more than three lines close up to a row apart)
            self.at(im, 1, 13 + (2 * k if len(lines) <= 3 else k), ln)

    def options_page(self, im):
        """the OPTION screen: KAI's tabs in a bar of three rows (L / R), then the rows as each game prints them:
        CRYSTAL (engine/menus/options_menu.asm) the name, then ":" and the value at column 10 on the row under it,
        the ▶ at column 1; RED (engine/menus/options.asm) the name, then every choice on the row under it, ▶ at the
        one chosen"""
        im.paste(self.paper + (255,), (0, 0, W, H))
        cur = OPTION_TABS.index('DISPLAY')
        self.fill(im, (0, 0, W, 24), self.ink)
        title = '◀ ' + OPTION_TABS[cur] + ' ▶'
        self.at(im, (30 - len(title)) // 2, 1, title, self.paper)
        self.at(im, 1, 1, 'L', self.paper)
        self.at(im, 28, 1, 'R', self.paper)
        self.window(im, (0, 24, W, H))
        for i, (name, vals) in enumerate(OPTION_ROWS):
            r = 4 + 2 * i
            self.at(im, 2, r, name)
            if self.kind == 'crystal':
                self.at(im, 10, r + 1, ':' + (self.name if vals is None else vals[-1]))
            elif vals is None or len(vals) == 1:   # (UI STYLE's six, FRAME's twenty: the one on between arrows)
                self.at(im, 3, r + 1, '◀ ' + (self.name if vals is None else vals[0]) + ' ▶')
            else:
                col = 3
                for k, v in enumerate(vals):
                    if k == len(vals) - 1:
                        self.at(im, col - 1, r + 1, '▶')
                    self.at(im, col, r + 1, v)
                    col += len(v) + 2
        self.cursor(im, 8, 4 * 8)

    def gbc_picture(self, path, cols, crop=None):
        """a Game Boy picture (2bpp greys) in four colours, lightest first"""
        im = Image.open(path).convert('L')
        if crop:
            im = im.crop(crop)
        out = Image.new('RGBA', im.size)
        p, q = im.load(), out.load()
        for y in range(im.height):
            for x in range(im.width):
                v = p[x, y]
                q[x, y] = cols[0 if v > 200 else 1 if v > 120 else 2 if v > 50 else 3] + (255,)
        return out
