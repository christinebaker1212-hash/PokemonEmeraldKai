"""RED and CRYSTAL: the Game Boy games' interface, drawn tile for tile from their own tiles (pret/pokered and
pret/pokecrystal: gfx/font/font.png, font_extra.png, font_battle_extra.png, Crystal's gfx/frames/N.png) on the GBA's
30 x 20 tiles, in each game's colours (RED: the Super Game Boy's four greys with its coloured HP bar; CRYSTAL: the Game
Boy Color's palettes). The layouts are the games' own, widened to 240 pixels; KAI's functions keep their place."""
import os

from PIL import Image

from common import (ACTIONS, BAG_ITEMS, DOUBLE_FOES, DOUBLE_MINE, FOE, MESSAGE, MINE, MOVES, OPTION_ROWS, OPTION_TABS,
                    PARTY, POCKETS, START_MENU, SUMMARY, TRIPLE_FOES, TRIPLE_MINE, H, W, creature, hp_colour, item_icon,
                    paste, shot)

# the font's codes (pokered charmap.asm): 0x80 A ... 0xFF 9
CODES = {**{chr(65 + i): 0x80 + i for i in range(26)}, **{chr(97 + i): 0xA0 + i for i in range(26)},
         **{str(i): 0xF6 + i for i in range(10)}, '(': 0x9A, ')': 0x9B, ':': 0x9C, ';': 0x9D, '[': 0x9E, ']': 0x9F,
         'é': 0xBA, "'": 0xE0, '-': 0xE3, '?': 0xE6, '!': 0xE7, '.': 0xE8, '▶': 0xED, '▼': 0xEE, '♂': 0xEF,
         '¥': 0xF0, '×': 0xF1, '/': 0xF3, ',': 0xF4, '♀': 0xF5, '&': 0xE7}


class GB:
    def __init__(self, kind, pret, kai, export):
        self.kind, self.kai, self.export, self.pret = kind, kai, export, pret
        g = os.path.join(pret, 'gfx', 'font')
        self.font = self._tiles(os.path.join(g, 'font.png'))                  # 0x80-0xFF
        self.extra = self._tiles(os.path.join(g, 'font_extra.png'))           # 0x60-0x7F
        self.battle = self._tiles(os.path.join(g, 'font_battle_extra.png'))
        if kind == 'crystal':
            self.frame = self._tiles(os.path.join(pret, 'gfx', 'frames', '1.png'))   # ┌ ─ ┐ │ └ ┘
            # (the Game Boy Color's palettes: text boxes white on black lines, the HUD's bars coloured)
            self.ink, self.paper, self.light = (16, 16, 24), (248, 248, 248), (160, 168, 184)
        else:
            e = self.extra
            self.frame = [e[25], e[26], e[27], e[28], e[29], e[30]]        # (0x79-0x7E: ┌ ─ ┐ │ └ ┘)
            self.ink, self.paper, self.light = (24, 24, 24), (248, 248, 240), (168, 168, 160)

    @staticmethod
    def _tiles(path):
        im = Image.open(path).convert('L')
        out = []
        for ty in range(im.height // 8):
            for tx in range(im.width // 8):
                out.append(im.crop((tx * 8, ty * 8, tx * 8 + 8, ty * 8 + 8)))
        return out

    # ---------------------------------------------------------------------------------------------- tiles
    def tile(self, im, t, tx, ty, ink=None, paper=None, px=0, py=0):
        """one 8 x 8 tile at tile (tx, ty) (+ pixels): dark pixels in INK, the rest PAPER (None: clear)"""
        ink = ink or self.ink
        x0, y0 = tx * 8 + px, ty * 8 + py
        p = im.load()
        for y in range(8):
            for x in range(8):
                v = t.getpixel((x, y))
                X, Y = x0 + x, y0 + y
                if not (0 <= X < im.width and 0 <= Y < im.height):
                    continue
                if v < 64:
                    p[X, Y] = ink + (255,)
                elif v < 160:
                    p[X, Y] = (self.light if paper is None else tuple((a + b) // 2 for a, b in zip(ink, paper))) + (255,)
                elif paper is not None:
                    p[X, Y] = paper + (255,)

    def text(self, im, tx, ty, s, ink=None, paper=None, px=0, py=0):
        for i, ch in enumerate(s):
            if ch == ' ':
                if paper is not None:
                    self.fill(im, tx + i, ty, 1, 1, paper, px, py)
                continue
            if ch == '◀':
                self.tile(im, self.font[0xED - 0x80].transpose(Image.FLIP_LEFT_RIGHT), tx + i, ty, ink, paper, px, py)
                continue
            c = 0xEA if (ch == 'é' and self.kind == 'crystal') else CODES.get(ch, 0xE6)
            self.tile(im, self.font[c - 0x80], tx + i, ty, ink, paper, px, py)

    def fill(self, im, tx, ty, w, h, colour, px=0, py=0):
        x0, y0 = tx * 8 + px, ty * 8 + py
        im.paste(colour + (255,), (x0, y0, x0 + w * 8, y0 + h * 8))

    def box(self, im, tx, ty, w, h, paper=None):
        """a text box W x H tiles with the frame round it (┌ ─ ┐ │ └ ┘), inside cleared"""
        paper = paper or self.paper
        self.fill(im, tx, ty, w, h, paper)
        f = self.frame
        for x in range(1, w - 1):
            self.tile(im, f[1], tx + x, ty, paper=paper)
            self.tile(im, f[1], tx + x, ty + h - 1, paper=paper) if self.kind == 'red' else \
                self.tile(im, f[1].transpose(Image.FLIP_TOP_BOTTOM), tx + x, ty + h - 1, paper=paper)
        for y in range(1, h - 1):
            self.tile(im, f[3], tx, ty + y, paper=paper)
            self.tile(im, f[3] if self.kind == 'red' else f[3].transpose(Image.FLIP_LEFT_RIGHT), tx + w - 1, ty + y,
                      paper=paper)
        self.tile(im, f[0], tx, ty, paper=paper)
        self.tile(im, f[2], tx + w - 1, ty, paper=paper)
        self.tile(im, f[4], tx, ty + h - 1, paper=paper)
        self.tile(im, f[5], tx + w - 1, ty + h - 1, paper=paper)

    # ---------------------------------------------------------------------------------------------- the HUD
    def hp_bar(self, im, tx, ty, frac, px=0, py=0):
        """HP: and a six-tile bar (each tile 8 steps), the bar in the HP's colour"""
        b = self.battle
        if self.kind == 'red':
            self.tile(im, b[15], tx, ty, px=px, py=py)          # (the H)
            self.tile(im, b[0], tx + 1, ty, px=px, py=py)       # (P: and the bar's left end)
            empty, steps, end = b[1], b[1:10], b[10]
        else:
            self.tile(im, b[0], tx, ty, px=px, py=py)
            self.tile(im, b[1], tx + 1, ty, px=px, py=py)
            empty, steps, end = b[2], b[2:11], b[11]
        n = round(frac * 48)
        col = hp_colour(frac)
        for k in range(6):
            part = max(0, min(8, n - k * 8))
            t = steps[part]
            self.tile(im, t, tx + 2 + k, ty, px=px, py=py)
            # (the bar's fill in colour: the tile's dark middle rows)
            x0, y0 = (tx + 2 + k) * 8 + px, ty * 8 + py
            p = im.load()
            for yy in range(3, 6):
                for xx in range(part):
                    if t.getpixel((xx, yy)) < 160 and 0 <= x0 + xx < W:
                        p[x0 + xx, y0 + yy] = col + (255,)
        self.tile(im, end, tx + 8, ty, px=px, py=py)

    def level(self, im, tx, ty, n, px=0, py=0):
        self.tile(im, self.battle[12 if self.kind == 'red' else 14], tx, ty, px=px, py=py)   # (:L)
        self.text(im, tx + 1, ty, str(n), px=px, py=py)

    def sex(self, im, tx, ty, s, px=0, py=0):
        self.text(im, tx, ty, '♂' if s == 'm' else '♀', ink=(48, 104, 216) if s == 'm' else (216, 64, 96), px=px,
                  py=py)

    def foe_hud(self, im, tx, ty, m, compact=False):
        """the foe's HUD: name, :L and the bar, the bracket on its left and foot (Gen 1 / 2's)"""
        b = self.battle
        self.text(im, tx + 1, ty, m['name'])
        if compact:
            self.level(im, tx + 1, ty + 1, m['level'])
            self.hp_bar(im, tx + 4, ty + 1, m['hp'])
            return
        self.sex(im, tx + 1 + len(m['name']), ty, m['sex'])
        self.level(im, tx + 5, ty + 1, m['level'])
        self.hp_bar(im, tx + 2, ty + 2, m['hp'])
        if self.kind == 'red':
            for y in (1, 2):
                self.tile(im, b[11], tx + 1, ty + y)
            self.tile(im, b[22], tx + 1, ty + 3)
            for x in range(2, 11):
                self.tile(im, b[20], tx + x, ty + 3)
            self.tile(im, b[13], tx + 11, ty + 3)
        else:
            for y in (1, 2):
                self.tile(im, b[13], tx + 1, ty + y)
            self.tile(im, b[24], tx + 1, ty + 3)
            for x in range(2, 11):
                self.tile(im, b[22], tx + x, ty + 3)
            self.tile(im, b[15], tx + 11, ty + 3)

    def mine_hud(self, im, tx, ty, m, compact=False):
        """the player's HUD: name, :L, the bar and HP numbers, the bracket on its right"""
        b = self.battle
        self.text(im, tx, ty, m['name'])
        if compact:
            self.level(im, tx, ty + 1, m['level'])
            self.hp_bar(im, tx + 3, ty + 1, m['hp'] / m['maxhp'])
            return
        self.sex(im, tx + len(m['name']), ty, m['sex'])
        self.level(im, tx + 4, ty + 1, m['level'])
        self.hp_bar(im, tx, ty + 2, m['hp'] / m['maxhp'])
        self.text(im, tx + 2, ty + 3, '%3d/%3d' % (m['hp'], m['maxhp']))
        if self.kind == 'red':
            for y in (1, 2, 3):
                self.tile(im, b[11], tx + 9, ty + y)
            self.tile(im, b[22].transpose(Image.FLIP_LEFT_RIGHT), tx + 9, ty + 4)
            for x in range(1, 9):
                self.tile(im, b[20], tx + x, ty + 4)
            self.tile(im, b[13].transpose(Image.FLIP_LEFT_RIGHT), tx, ty + 4)
        else:
            for y in (1, 2, 3):
                self.tile(im, b[13], tx + 9, ty + y)
            self.tile(im, b[17], tx, ty + 4)
            for x in range(1, 9):
                self.tile(im, b[22], tx + x, ty + 4)
            self.tile(im, b[24].transpose(Image.FLIP_LEFT_RIGHT), tx + 9, ty + 4)
            # (Crystal's EXP bar under the HP)
            x0, y0 = tx * 8 + 8, (ty + 4) * 8 + 2
            im.paste((24, 24, 40, 255), (x0, y0, x0 + 64, y0 + 4))
            im.paste((80, 152, 248, 255), (x0 + 64 - int(64 * m['exp']), y0 + 1, x0 + 64, y0 + 3))

    def cursor(self, im, tx, ty):
        self.text(im, tx, ty, '▶')

    # ---------------------------------------------------------------------------------------------- the screens
    def message(self):
        im = shot(self.kai, 'f_mart')
        self.box(im, 0, 14, 30, 6)
        self.text(im, 1, 16, MESSAGE)
        self.text(im, 28, 18, '▼')
        return im

    def start_menu(self):
        im = shot(self.kai, 'f_town')
        self.box(im, 19, 0, 11, 2 * len(START_MENU) + 1)
        for i, s in enumerate(START_MENU):
            self.text(im, 21, 1 + 2 * i, s)
        self.cursor(im, 20, 1)
        return im

    def battle_menu(self, im):
        self.box(im, 0, 14, 30, 6)
        if self.kind == 'crystal':
            self.text(im, 1, 16, 'What will')
            self.text(im, 1, 18, 'TORCHIC do?')
        self.box(im, 14, 14, 16, 6)
        for i, a in enumerate(ACTIONS):
            self.text(im, 16 + (i % 2) * 7, 16 + (i // 2) * 2, a)
        self.cursor(im, 15, 16)

    def single(self):
        im = shot(self.kai, 's_single')
        self.foe_hud(im, 0, 0, FOE)
        self.mine_hud(im, 19, 8, MINE)
        self.battle_menu(im)
        return im

    def moves(self):
        im = shot(self.kai, 's_single')
        self.foe_hud(im, 0, 0, FOE)
        self.mine_hud(im, 19, 8, MINE)
        self.box(im, 0, 14, 30, 6)
        # (Gen 1 / 2: the moves in their own box, the chosen one's TYPE / and PP above it)
        self.box(im, 0, 8, 12, 6)
        name, pp, mx, kind = MOVES[0]
        self.text(im, 1, 9, 'TYPE/')
        self.text(im, 2, 10, kind)
        self.text(im, 5, 12, '%d/%d' % (pp, mx))
        self.box(im, 10, 13, 20, 7)
        for i, (n, *_r) in enumerate(MOVES):
            self.text(im, 12, 14 + i, n)
        self.cursor(im, 11, 14)
        return im

    def double(self):
        im = shot(self.kai, 's_double')
        for i, m in enumerate(DOUBLE_FOES):
            self.foe_hud(im, 0, i * 3, m, compact=True)
        for i, m in enumerate(DOUBLE_MINE):
            self.mine_hud(im, 18, 7 + i * 3, m, compact=True)
        self.battle_menu(im)
        return im

    def triple(self):
        im = shot(self.kai, 's_triple')
        for i, m in enumerate(TRIPLE_FOES):
            self.foe_hud(im, 0, i * 3, m, compact=True)
        for i, m in enumerate(TRIPLE_MINE):
            self.mine_hud(im, 18, 5 + i * 3, m, compact=True)
        self.battle_menu(im)
        return im

    def party(self):
        im = Image.new('RGBA', (W, H), self.paper + (255,))
        for i, m in enumerate(PARTY):
            y = i * 20
            if i == 0:
                self.text(im, 0, 0, '▶', py=y + 2)
            paste(im, creature(self.export, m['species'], 'icon'), 6, y - 6)
            self.text(im, 5, 0, m['name'], py=y + 2)
            self.sex(im, 5 + len(m['name']), 0, m['sex'], py=y + 2)
            self.level(im, 16, 0, m['level'], py=y + 2)
            self.hp_bar(im, 6, 1, m['hp'] / m['maxhp'], py=y + 2)
            self.text(im, 16, 1, '%3d/%3d' % (m['hp'], m['maxhp']), py=y + 2)
        self.box(im, 0, 15, 30, 5)
        self.text(im, 1, 17, 'Choose a MOéMON.')
        return im

    def pack(self):
        """Crystal's PACK: the bag on blue, its pocket's name, the list in a box, the words below"""
        blue = [(248, 248, 248), (120, 120, 248), (0, 0, 248), (0, 0, 0)]
        im = Image.new('RGBA', (W, H), (120, 120, 248, 255))
        bag = self.gbc_picture(os.path.join(self.pret, 'gfx', 'pack', 'pack.png'), blue, (0, 0, 40, 48))
        paste(im, bag, 12, 12)
        self.fill(im, 0, 8, 9, 2, (248, 248, 248))
        self.text(im, 1, 8, '◀')
        self.text(im, 2, 8, 'ITEMS')
        self.text(im, 8, 8, '▶')
        self.box(im, 9, 0, 21, 13)
        for i, (name, n, _item) in enumerate(BAG_ITEMS[:4]):
            self.text(im, 11, 1 + i * 2, name)
            self.text(im, 25, 2 + i * 2, '×%2d' % n)
        self.text(im, 11, 9, 'CANCEL')
        self.cursor(im, 10, 1)
        self.box(im, 0, 13, 30, 7)
        self.text(im, 1, 15, 'Restores HP by 20.')
        return im

    def summary(self):
        if self.kind == 'crystal':
            return self.stats()
        im = Image.new('RGBA', (W, H), self.paper + (255,))
        s = SUMMARY
        paste(im, creature(self.export, 280), 4, 0)
        self.text(im, 1, 8, 'No.' + s['no'])
        self.text(im, 10, 1, s['name'])
        self.sex(im, 10 + len(s['name']), 1, s['sex'])
        self.level(im, 14, 2, s['level'])
        self.hp_bar(im, 11, 3, 1.0)
        self.text(im, 13, 4, '%3d/%3d' % (56, 56))
        self.text(im, 11, 6, 'STATUS/OK')
        self.box(im, 0, 9, 30, 11)
        rows = [('TYPE1/', s['type']), ('IDNo/', s['idno']), ('OT/', s['ot']), ('ABILITY/', s['ability'])]
        for i, (k, v) in enumerate(rows):
            self.text(im, 1, 10 + i * 2, k)
            self.text(im, 10, 10 + i * 2, v)
        self.text(im, 1, 18, s['nature'] + ' nature, ' + s['met'])
        self.text(im, 17, 10, 'MET AT')
        self.text(im, 17, 12, 'INSIDE OF')
        self.text(im, 17, 14, 'TRUCK.')
        return im

    def gbc_picture(self, path, cols, crop=None):
        """a Game Boy Color picture (2bpp greys) in four colours (lightest first)"""
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

    def stats(self):
        """Crystal's STATS, page 1 (pink): the picture over the band of its three pages, below it the details"""
        s = SUMMARY
        pink = (248, 152, 248)
        im = Image.new('RGBA', (W, H), self.paper + (255,))
        paste(im, creature(self.export, 280), 4, 0)
        self.text(im, 10, 1, 'No.' + s['no'])
        self.text(im, 10, 3, s['name'])
        self.sex(im, 10 + len(s['name']), 3, s['sex'])
        self.text(im, 10, 4, '/' + s['species'])
        self.level(im, 10, 6, s['level'])
        self.fill(im, 0, 8, 30, 1, pink)
        for k, c in enumerate([(248, 120, 248), (136, 248, 0), (120, 184, 248)]):
            self.fill(im, 24 + k * 2, 8, 1, 1, c)
            self.text(im, 24 + k * 2, 8, '▼' if k == 0 else ' ', paper=c)
        self.text(im, 1, 8, 'PAGE 1', paper=pink)
        self.text(im, 1, 10, 'HP')
        self.hp_bar(im, 4, 10, 1.0)
        self.text(im, 14, 10, '%3d/%3d' % (56, 56))
        self.text(im, 1, 12, 'STATUS/')
        self.text(im, 9, 12, 'OK')
        self.text(im, 1, 14, 'TYPE/')
        self.text(im, 9, 14, s['type'])
        self.text(im, 1, 16, 'ABILITY/')
        self.text(im, 10, 16, s['ability'])
        self.text(im, 1, 18, s['nature'] + ' nature, ' + s['met'])
        return im

    def bag(self):
        if self.kind == 'crystal':
            return self.pack()
        im = shot(self.kai, 'f_town')
        self.box(im, 4, 0, 26, 14)
        self.text(im, 6, 1, '◀')
        self.text(im, 8, 1, POCKETS[0])
        self.text(im, 27, 1, '▶')
        for i, (name, n, _item) in enumerate(BAG_ITEMS[:4]):
            self.text(im, 6, 3 + i * 2, name)
            self.text(im, 22, 4 + i * 2, '×%2d' % n)
        self.text(im, 6, 11, 'CANCEL')
        self.cursor(im, 5, 3)
        self.box(im, 0, 14, 30, 6)
        self.text(im, 1, 16, 'Restores HP by 20.')
        return im

    def options(self, chosen):
        im = Image.new('RGBA', (W, H), self.paper + (255,))
        x = 1
        for t in OPTION_TABS:
            sel = t == 'DISPLAY'
            if sel:
                self.fill(im, x - 1, 0, len(t) + 2, 1, self.ink)
            self.text(im, x, 0, t, ink=self.paper if sel else None, paper=self.ink if sel else None)
            x += len(t) + 1
        self.box(im, 0, 1, 30, 19)
        for i, (name, vals) in enumerate(OPTION_ROWS):
            y = 3 + i * 2
            self.text(im, 2, y, name)
            if vals is None:
                self.text(im, 16, y, '◀ ' + chosen + ' ▶')
            else:
                xx = 16
                for k, v in enumerate(vals):
                    if k == len(vals) - 1:
                        self.text(im, xx - 1, y, '▶')
                    self.text(im, xx, y, v)
                    xx += len(v) + 2
        self.cursor(im, 1, 3)
        self.text(im, 2, 17, 'L / R: TABS')
        return im

    def all(self):
        return {'message': self.message(), 'start_menu': self.start_menu(), 'battle_single': self.single(),
                'battle_moves': self.moves(), 'battle_double': self.double(), 'battle_triple': self.triple(),
                'party': self.party(), 'summary': self.summary(), 'bag': self.bag(),
                'options': self.options(self.kind.upper())}
