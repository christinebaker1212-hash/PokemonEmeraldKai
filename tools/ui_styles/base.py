"""The screens every style shares: KAI's layout (layout.py) filled by a style's own pieces. A style supplies its
words (text / measure, in its game's font), its windows, its cursor, its HP bars and its battle boxes; the screens
put them where KAI puts its own, so nothing moves between styles, only how it looks."""
import os

from PIL import Image

import layout as Lo
from common import (ACTIONS, BAG_DESC, BAG_ITEMS, BAG_POCKET, DOUBLE_FOES, DOUBLE_MINE, FOE, MESSAGE, MINE, MOVES,
                    OPTION_ROWS, OPTION_TABS, PARTY, POCKETS, PROMPT, START_MENU, SUMMARY, TRIPLE_FOES, TRIPLE_MINE,
                    H, W, creature, item_icon, paste, shot)


class Style:
    name = 'KAI'
    small = 'n'   # (the font the triple cards and the tight places use)

    def __init__(self, kai, export):
        self.kai, self.export = kai, export
        self.icons = [self.kai_type_icon(os.path.join(kai, 'icon%d.png' % k)) for k in range(6)]
        self.inks, self.logs = [], {}

    @staticmethod
    def kai_type_icon(path):
        """KAI's type icon, cut from the game's picture: the 11 x 11 rounded square only (the capture round it holds
        the scene and KAI's own card tail's shadow, which must not come along)"""
        im = Image.open(path).convert('RGBA').crop((0, 0, 11, 11))
        for c in ((0, 0), (10, 0), (0, 10), (10, 10)):
            im.putpixel(c, (0, 0, 0, 0))
        return im

    # ---------------------------------------------------------------------------------------------- what a style gives
    def text(self, im, x, y, s, col=None, shadow=None, font='n', align='l'):
        raise NotImplementedError

    def measure(self, s, font='n'):
        raise NotImplementedError

    def line(self, font='n'):
        return 16 if font == 'n' else 12

    def window(self, im, r, kind='menu'):
        raise NotImplementedError

    def cursor(self, im, x, y):
        raise NotImplementedError

    def hud(self, im, r, m, kind):
        raise NotImplementedError

    # ---------------------------------------------------------------------------------------------- battle pieces
    def battle_msg(self, im, r=Lo.BATTLE_MSG):
        self.window(im, r, 'battle_msg')
        x0, y0, x1, y1 = r
        self.text(im, x0 + 9, y0 + 8, PROMPT[0], *self.battle_msg_ink())
        self.text(im, x0 + 9, y0 + 24, PROMPT[1], *self.battle_msg_ink())

    def battle_msg_ink(self):
        return (None, None)

    def battle_menu(self, im, r=Lo.BATTLE_MENU):
        self.window(im, r, 'battle_menu')
        x0, y0, x1, y1 = r
        for i, a in enumerate(ACTIONS):
            self.text(im, x0 + 16 + (i % 2) * 56, y0 + 8 + (i // 2) * 16, a)
        self.cursor(im, x0 + 8, y0 + 8)

    def move_panel(self, im):
        self.window(im, Lo.MOVES, 'battle_menu')
        x0, y0, _, _ = Lo.MOVES
        for i, (n, *_r) in enumerate(MOVES):
            self.text(im, x0 + 16 + (i % 2) * 72, y0 + 8 + (i // 2) * 16, n, font=self.small if
                      self.measure(n) > 70 else 'n')
        self.cursor(im, x0 + 8, y0 + 8)
        self.window(im, Lo.MOVE_INFO, 'battle_menu')
        x0, y0, x1, _ = Lo.MOVE_INFO
        n, pp, mx, kind = MOVES[0]
        self.text(im, x0 + 8, y0 + 8, 'PP')
        self.text(im, x1 - 8, y0 + 8, '%d/%d' % (pp, mx), align='r')
        self.text(im, x0 + 8, y0 + 24, 'TYPE/' + kind)

    def icons_beside(self, im):
        for k, (x, y) in enumerate(Lo.TRIPLE_FOE_ICONS + Lo.TRIPLE_MINE_ICONS):
            paste(im, self.icons[k], x, y)

    # ---------------------------------------------------------------------------------------------- the screens
    def battle_single(self):
        im = shot(self.kai, 's_single')
        self.hud(im, Lo.SINGLE_FOE, FOE, 'foe')
        self.hud(im, Lo.SINGLE_MINE, MINE, 'mine')
        self.battle_msg(im)
        self.battle_menu(im)
        return im

    def battle_moves(self):
        im = shot(self.kai, 's_single')
        self.hud(im, Lo.SINGLE_FOE, FOE, 'foe')
        self.hud(im, Lo.SINGLE_MINE, MINE, 'mine')
        self.move_panel(im)
        return im

    def battle_double(self):
        im = shot(self.kai, 's_double')
        for r, m in zip(Lo.DOUBLE_FOES, DOUBLE_FOES):
            self.hud(im, r, m, 'foe2')
        for r, m in zip(Lo.DOUBLE_MINE, DOUBLE_MINE):
            self.hud(im, r, m, 'mine2')
        self.battle_msg(im)
        self.battle_menu(im)
        return im

    def battle_triple(self):
        im = shot(self.kai, 's_triple')
        for r, m in zip(Lo.TRIPLE_FOES, TRIPLE_FOES):
            self.hud(im, r, m, 'foe3')
        for r, m in zip(Lo.TRIPLE_MINE, TRIPLE_MINE):
            self.hud(im, r, m, 'mine3')
        self.icons_beside(im)
        self.battle_msg(im)
        self.battle_menu(im)
        return im

    def message(self):
        im = shot(self.kai, 'f_mart')
        self.window(im, Lo.FIELD_MSG, 'message')
        self.text(im, 16, 120, MESSAGE)
        self.more_arrow(im, 16 + self.measure(MESSAGE) + 2, 123)
        return im

    def more_arrow(self, im, x, y):
        p = im.load()
        for k in range(4):
            for j in range(k, 7 - k):
                p[x + j, y + k] = (248, 88, 56, 255)

    def start_menu(self):
        im = shot(self.kai, 'f_town')
        self.window(im, Lo.START, 'menu')
        x0, y0, _, _ = Lo.START
        for i, s in enumerate(START_MENU):
            self.text(im, x0 + 16, y0 + 10 + 16 * i, s)
        self.cursor(im, x0 + 8, y0 + 10)
        return im

    def party(self):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        self.party_bg(im)
        for i, m in enumerate(PARTY):
            r = Lo.PARTY_MAIN if i == 0 else Lo.PARTY_SLOTS[i - 1]
            self.party_slot(im, r, m, i == 0)
        for r in Lo.PARTY_SLOTS[len(PARTY) - 1:]:
            self.party_slot(im, r, None, False)
        self.window(im, Lo.PARTY_MSG, 'message')
        self.text(im, Lo.PARTY_MSG[0] + 8, Lo.PARTY_MSG[1] + 6, 'Choose a MOéMON.')
        self.party_cancel(im, Lo.PARTY_CANCEL)
        return im

    def summary(self):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        self.summary_page(im)
        return im

    def bag(self):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        self.bag_page(im)
        return im

    def options(self):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        self.options_page(im)
        return im

    def all(self):
        out = {}
        for name in ('message', 'start_menu', 'battle_single', 'battle_moves', 'battle_double', 'battle_triple',
                     'party', 'summary', 'bag', 'options'):
            self.inks = []
            out[name] = getattr(self, name)()
            self.logs[name] = self.inks
        return out
