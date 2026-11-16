"""The DS games' own fonts for the mock-ups: Platinum's (graphic/pl_font.narc: a 16-byte header, the glyphs 2bpp in
2 x 2 tiles, a width per glyph; a glyph is its character code less one, Gen 4's codes) and Black's (a/0/2/3: Nitro
NFTR fonts, Unicode)."""
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'sinnoh'))
import nds  # noqa: E402

# Gen 4's Latin codes in order (pret/pokeplatinum tools/msgenc/charmap.txt, 0x121-0x1E2 and 0x1E8-0x1E9)
LATIN = ('0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
         'ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿŒœŞşªº¹²³$¡¿!?,.…·/‘’“”„《》()♂♀+-*#=&~:;'
         '♠♣♥♦★◉●■▲◆@♪%☀☁☂☃☺♚♛☹↗↘☽ ⁴₧₦')
# Gen 4's codes for what the mock-ups write (pokeplatinum's charmap): digits, letters, the punctuation used
GEN4 = {**{str(i): 0x121 + i for i in range(10)}, **{chr(65 + i): 0x12B + i for i in range(26)},
        **{chr(97 + i): 0x145 + i for i in range(26)}, '!': 0x1AB, '?': 0x1AC, ',': 0x1AD, '.': 0x1AE,
        '…': 0x1AF, '/': 0x1B1, "'": 0x1B3, '(': 0x1B9, ')': 0x1BA, '♂': 0x1BB, '♀': 0x1BC, '+': 0x1BD,
        '-': 0x1BE, ':': 0x1C4, ';': 0x1C5, 'é': 0x188, ' ': 0x1DE, '&': 0x1C2, '%': 0x1D2}
# every Latin character Platinum's fonts have (KAI's words in them: the game's own text), "'" as its ’
FULL = dict({c: 0x121 + i for i, c in enumerate(LATIN)}, **{'°': 0x1E8, '_': 0x1E9}, **GEN4)


class Platinum:
    def __init__(self, rom, which=0):
        """WHICH: 0 FONT_SYSTEM, 1 FONT_MESSAGE, 2 FONT_SUBSCREEN (its 3 the letters' outline)"""
        d = nds.Rom(rom).narc('graphic/pl_font.narc')[which]
        hs, wo, n = struct.unpack_from('<III', d, 0)
        self.d, self.hs, self.widths, self.n = d, hs, d[wo:wo + n], n
        self.height = 16

    def glyph(self, ch):
        i = FULL.get(ch, 0x1AC) - 1
        g = self.d[self.hs + i * 64:self.hs + i * 64 + 64]
        rows = [[0] * 16 for _ in range(16)]
        for t in range(4):
            for r in range(8):
                v = g[t * 16 + r * 2] | g[t * 16 + r * 2 + 1] << 8
                for x in range(8):
                    rows[(t // 2) * 8 + r][(t % 2) * 8 + x] = (v >> (14 - 2 * x)) & 3
        return rows, self.widths[i]


class Nftr:
    """a Nitro font (Black's): glyphs by Unicode"""
    def __init__(self, data):
        d = data
        o = struct.unpack_from('<H', d, 0x0C)[0]   # (the FINF block)
        self.lf = d[o + 9]
        cglp, cwdh, cmap = struct.unpack_from('<III', d, o + 0x10)
        self.cw, self.ch, self.size, _base, _mw, self.bpp, _f = struct.unpack_from('<BBHbBBB', d, cglp)
        self.glyphs = cglp + 8
        self.wd = {}   # (widths: worked out from each glyph's pixels)
        self.map = {}
        while cmap and cmap + 12 <= len(d):
            begin, end, kind, _pad, nxt = struct.unpack_from('<HHHHI', d, cmap)
            p = cmap + 12
            if kind == 0:
                off = struct.unpack_from('<H', d, p)[0]
                for c in range(begin, end + 1):
                    self.map[c] = off + c - begin
            elif kind == 1:
                for c in range(begin, end + 1):
                    v = struct.unpack_from('<H', d, p + 2 * (c - begin))[0]
                    if v != 0xFFFF:
                        self.map[c] = v
            else:
                cnt = struct.unpack_from('<H', d, p)[0]
                for k in range(cnt):
                    c, v = struct.unpack_from('<HH', d, p + 2 + 4 * k)
                    self.map[c] = v
            cmap = nxt
        self.d = d
        self.height = self.ch

    def glyph(self, ch):
        i = self.map.get(ord(ch), self.map.get(ord('?'), 0))
        cell = self.d[self.glyphs + i * self.size:self.glyphs + (i + 1) * self.size]
        left, _gw, adv = cell[0], cell[1], cell[2]   # (Black's cells start with the glyph's left, width, advance)
        bits = cell[3:]
        rows = [[0] * (self.cw + left) for _ in range(self.ch)]
        for y in range(self.ch):
            for x in range(self.cw):
                k = (y * self.cw + x) * self.bpp
                if k // 8 >= len(bits):
                    continue
                v = (bits[k // 8] >> (8 - self.bpp - k % 8)) & ((1 << self.bpp) - 1)
                rows[y][x + left] = v if v in (1, 2) else 0   # (3: the glyph's background)
        return rows, adv


def black(rom, which=0):
    """Black's fonts: 0 the system font (12 x 15 cells), 1 the small one (10 x 10)"""
    return Nftr(nds.Rom(rom).narc('a/0/2/3')[which])


def draw(im, font, x, y, s, ink, shadow=None, align='l'):
    """S at (X, Y) in FONT: the glyphs' colour 1 in INK, 2 in SHADOW (none: left out)"""
    w = sum(font.glyph(c)[1] for c in s)
    if align == 'c':
        x -= w // 2
    elif align == 'r':
        x -= w
    p = im.load()
    for c in s:
        rows, adv = font.glyph(c)
        for yy, row in enumerate(rows):
            for xx, v in enumerate(row):
                X, Y = x + xx, y + yy
                if v and 0 <= X < im.width and 0 <= Y < im.height:
                    col = ink if v == 1 else shadow if v == 2 else None
                    if col:
                        p[X, Y] = col + (255,)
        x += adv
    return x


def width(font, s):
    return sum(font.glyph(c)[1] for c in s)
