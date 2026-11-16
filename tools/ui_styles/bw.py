"""BLACK: the game's interface in Pokémon Black's own pieces, laid out where KAI lays its own.

Everything is Black's, read from the user's ROM (never committed): the battle archive a/0/1/1 (the gauges, the
message window, the touch screen's command and move buttons), the field's talk balloon (a/0/0/5), the X menu
(a/0/7/1), the party (a/0/7/6), the summary (a/0/7/8), the BAG (a/0/8/8), OPTIONS (a/0/2/7) and the fonts (a/0/2/3).
Where the decomp (Project-Holon/aletheia, code only) says how Black puts a piece together, it is put together the same
way: the battle gauges are composed exactly as src/game/ov094/btlv_gauge.c composes them (its VRAM writes of the name,
gender, level, bar and number tiles into the frame's characters, its actors' offsets read from overlay 94's tables).

Where a piece is bigger than KAI's room for it, it is cut down the way pixel art is cut down: whole runs of identical
rows or columns taken out (carve()), never scaled."""
import os
import re
import struct
import sys

from PIL import Image

import dsfont
import layout as Lo
from base import Style
from common import H, MOVES, PROMPT, W, paste, shot

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'sinnoh'))
import build_library_tv as L  # noqa: E402
import nds  # noqa: E402

OUTLINE = {(32, 32, 32), (40, 40, 40), (48, 48, 48)}   # (a button word's outline)
OAM = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
       (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}


def rgb(c):
    return ((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3)


# ------------------------------------------------------------------------------------------------- cutting pixel art
def _lines(im, axis):
    p = im.load()
    if axis == 'x':
        return [tuple(p[x, y] for y in range(im.height)) for x in range(im.width)]
    return [tuple(p[x, y] for x in range(im.width)) for y in range(im.height)]


def _rebuild(im, axis, keep):
    if axis == 'x':
        out = Image.new('RGBA', (len(keep), im.height))
        for nx, x in enumerate(keep):
            out.paste(im.crop((x, 0, x + 1, im.height)), (nx, 0))
    else:
        out = Image.new('RGBA', (im.width, len(keep)))
        for ny, y in enumerate(keep):
            out.paste(im.crop((0, y, im.width, y + 1)), (0, ny))
    return out


def carve(im, axis, n, lo=0, hi=None, keep_out=()):
    """IM with N columns (axis 'x') or rows ('y') taken out between LO and HI (never inside a KEEP_OUT range): each
    cut is a run [a, b) whose line b is the same as line a, so the picture goes on exactly as it did (a stripe, a
    mesh, a plain bar shortened); when no such run is left, the cut whose two sides differ least"""
    lines = _lines(im, axis)
    keep = list(range(len(lines)))
    hi = len(lines) if hi is None else hi
    blocked = {i for a, b in keep_out for i in range(a, b)}
    while n > 0:
        idx = [i for i in keep if lo <= i < hi]
        best = None
        for ia in range(len(idx)):
            for ib in range(ia + 1, min(len(idx), ia + n + 1)):
                if any(idx[j] in blocked for j in range(ia, ib)):
                    break
                a, b = idx[ia], idx[ib]
                if lines[a] == lines[b]:
                    key = (0, 0, -(ib - ia), abs((a + b) / 2 - (lo + hi) / 2))
                    if best is None or key < best[0]:
                        best = (key, ia, ib)
        if best is None:      # (no run repeats: the least different cut, as long as the room allows)
            for length in range(n, 0, -1):
                for ia in range(len(idx)):
                    ib = ia + length
                    if ib >= len(idx) or any(idx[j] in blocked for j in range(ia, ib)):
                        continue
                    a, b = idx[ia], idx[ib]
                    d = sum(1 for u, v in zip(lines[a], lines[b]) if u != v)
                    key = (1, d, 0, abs((a + b) / 2 - (lo + hi) / 2))
                    if best is None or key < best[0]:
                        best = (key, ia, ib)
                if best is not None:
                    break
        if best is None:
            raise ValueError('nothing left to carve')
        _, ia, ib = best
        cut = set(idx[ia:ib])
        keep = [i for i in keep if i not in cut]
        n -= len(cut)
    return _rebuild(im, axis, keep)


def widen(im, axis, n, at):
    """IM with line AT repeated N more times (a plain run made longer)"""
    keep = list(range(im.width if axis == 'x' else im.height))
    keep = keep[:at] + [at] * n + keep[at:]
    return _rebuild(im, axis, keep)


class Black(Style):
    name = 'BLACK'
    small = 'n'
    FIT = 'inside'
    ORIGINAL = [('battle_single', 'edge', None, "Black's gauges run in from the screen's edge"),
                ('battle_moves', 'edge', None, "Black's gauges run in from the screen's edge"),
                ('battle_double', 'edge', None, "Black's gauges run in from the screen's edge")]

    def __init__(self, rom, kai, export):
        super().__init__(kai, export)
        self.rom = rom
        self.r = nds.Rom(rom)
        self._narcs = {}
        fonts = self.r.narc('a/0/2/3')
        self.fmain, self.fsmall, self.fsub = (dsfont.Nftr(fonts[k]) for k in (0, 1, 2))
        self.font = self.fmain
        self.seam_ok, self._rect, self._shift = {}, None, 0
        floating = (self.floating, "Black's names float over the scene with no box behind them (and its arrows' "
                                   "points step out pixel by pixel)")
        self.ORIGINAL = self.ORIGINAL + [(n, 'seam') + floating for n in ('battle_single', 'battle_moves',
                                                                          'battle_double', 'battle_triple')] + [
            ('battle_moves', 'collide', None, "Black's move names and PP on its buttons' mesh"),
            ('start_menu', 'collide', None, "Black's X menu: its words across its buttons' shading and the grey bar"),
            ('party', 'collide', None, "Black's plates: their words over the plates' diagonal shading"),
            ('bag', 'collide', ['MEDICINE', 'START: SORT'], "Black's BAG: words on its patterned ground"),
            ('summary', 'collide', ['Lv24'], "Black's summary: the level on its grid"),
            ('options', 'collide', None, "Black's OPTIONS title bar is a gradient")]

    def floating(self, finding):
        """a seam check.py found where Black leaves the scene showing: between a floating word's letters, inside
        its gender mark or Lv icon, between the steps of an arrow's point"""
        m = re.search(r'at \((\d+),(\d+)\) inside (\(.*\))', finding)
        if not m:
            return False
        x, y, r = int(m.group(1)), int(m.group(2)), m.group(3)
        return any(a <= x < c and b <= y < d for a, b, c, d in self.seam_ok.get(tuple(eval(r)), []))

    # --------------------------------------------------------------------------------------------- the ROM's pieces
    def U(self, arc, i):
        if arc not in self._narcs:
            self._narcs[arc] = self.r.narc(arc)
        return L.unpack(self._narcs[arc][i])

    def pal(self, arc, i):
        return L.palette(self.U(arc, i))

    def tiles(self, arc, i):
        return [list(t) for t in L.tiles(self.U(arc, i))[0]]

    def screen(self, arc, ch, scr, pal, ts=None, palette=None):
        """a BG map whole (every 256 x 256 block where the DS keeps it), colour 0 of each tile clear"""
        ts = ts or L.tiles(self.U(arc, ch))
        tl, depth = ts
        w, h, ents = L.screen(self.U(arc, scr))
        P = palette or self.pal(arc, pal)
        im = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        p = im.load()
        for k, e in enumerate(ents):
            if w > 256:
                blk, rr = divmod(k, 1024)
                bx, by = blk % (w // 256), blk // (w // 256)
                tx, ty = bx * 256 + (rr % 32) * 8, by * 256 + (rr // 32) * 8
            else:
                tx, ty = (k % (w // 8)) * 8, (k // (w // 8)) * 8
            if tx >= w or ty >= h:
                continue
            t, hf, vf, pn = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1, e >> 12
            if t >= len(tl):
                continue
            for i, v in enumerate(tl[t]):
                if not v:
                    continue
                x, y = i % 8, i // 8
                if hf:
                    x = 7 - x
                if vf:
                    y = 7 - y
                k2 = (pn * 16 + v) if depth == 3 else v
                p[tx + x, ty + y] = rgb(P[k2] if k2 < len(P) else 0) + (255,)
        return im

    def cell(self, arc, cell_id, ts, P, k=0):
        """cell K of an NCER drawn from tiles TS: (picture, x of its left, y of its top) about the sprite's point"""
        c = L.section(self.U(arc, cell_id), b'CEBK')
        n, attrs, off, mapping = struct.unpack_from('<HHII', c, 8)
        shift, base, step = mapping & 3, 8 + off, 16 if attrs & 1 else 8
        cnt, _, ooff = struct.unpack_from('<HHI', c, base + k * step)
        oams = []
        for j in range(cnt):
            a0, a1, a2 = struct.unpack_from('<HHH', c, base + n * step + ooff + 6 * j)
            y = (a0 & 0xFF) - (256 if a0 & 0x80 else 0)
            x = (a1 & 0x1FF) - (512 if a1 & 0x100 else 0)
            oams.append((x, y, OAM[(a0 >> 14, a1 >> 14)], (a2 & 0x3FF) << shift, a2 >> 12, a1 & 0x1000, a1 & 0x2000))
        x0, y0 = min(o[0] for o in oams), min(o[1] for o in oams)
        x1, y1 = max(o[0] + o[2][0] for o in oams), max(o[1] + o[2][1] for o in oams)
        im = Image.new('RGBA', (x1 - x0, y1 - y0), (0, 0, 0, 0))
        p = im.load()
        for x, y, (ow, oh), t0, pn, hf, vf in reversed(oams):
            for ty in range(oh // 8):
                for tx in range(ow // 8):
                    t = t0 + ty * (ow // 8) + tx
                    if t >= len(ts):
                        continue
                    for i, v in enumerate(ts[t]):
                        if not v:
                            continue
                        X, Y = tx * 8 + i % 8, ty * 8 + i // 8
                        if hf:
                            X = ow - 1 - X
                        if vf:
                            Y = oh - 1 - Y
                        kk = pn * 16 + v
                        p[x - x0 + X, y - y0 + Y] = rgb(P[kk] if kk < len(P) else P[v]) + (255,)
        return im, x0, y0

    # --------------------------------------------------------------------------------------------- words
    def draw(self, im, font, x, y, s, ink, shadow=None, align='l'):
        """S in FONT: its pixels 1 in INK, 2 in SHADOW"""
        w = dsfont.width(font, s)
        x -= w // 2 if align == 'c' else w if align == 'r' else 0
        p = im.load()
        for ch in s:
            rows, adv = font.glyph(ch)
            for yy, row in enumerate(rows):
                for xx, v in enumerate(row):
                    if v and 0 <= x + xx < im.width and 0 <= y + yy < im.height:
                        col = ink if v == 1 else shadow
                        if col:
                            p[x + xx, y + yy] = col + (255,)
            x += adv
        return x

    def text(self, im, x, y, s, col=None, shadow=None, font='n', align='l'):
        """S in Black's main font ('n') or its small one ('s'); by default the field's dark grey on white"""
        f = self.fsub if font == 's' else self.fmain
        if col is None:
            col, shadow = (88, 80, 80), (160, 160, 168)   # (a/0/2/3 palette 5: letters 1, shadow 2)
        return self.draw(im, f, x, y, s, col, shadow, align)

    def measure(self, s, font='n'):
        return dsfont.width(self.fsub if font == 's' else self.fmain, s)

    def log(self, im_or_none, x, y, font, s, ink, shadow, ox=0, oy=0):
        """draw S on IM and keep its inked rectangle for check.py (words drawn outside text())"""
        probe = Image.new('RGBA', (dsfont.width(font, s) + 16, 32), (0, 0, 0, 0))
        self.draw(probe, font, 8, 8, s, ink, shadow)
        bb = probe.getbbox()
        if bb and s.strip():
            self.inks.append(((ox + x + bb[0] - 8, oy + y + bb[1] - 8, ox + x + bb[2] - 8, oy + y + bb[3] - 8), s))

    # --------------------------------------------------------------------------------------------- battle gauges
    GAUGE = 'a/0/1/1'

    def gauge_parts(self):
        if not getattr(self, '_gp', None):
            a = self.GAUGE
            ov = OverlayTables(self.r)
            self._gp = dict(pal=self.pal(a, 0xA2), parts=self.tiles(a, 0xA4), ov=ov)
        return self._gp

    def _bmp(self, s, wt, ht, x, y, font):
        """S printed into a WT x HT tile bitmap at (X, Y) as btlv_gauge.c prints it: letters 1, shadow 4, ground 0"""
        Wd, Hd = wt * 8, ht * 8
        px = [[0] * Wd for _ in range(Hd)]
        for ch in s:
            rows, adv = font.glyph(ch)
            for yy, row in enumerate(rows):
                for xx, v in enumerate(row):
                    X, Y = x + xx, y + yy
                    if v and 0 <= X < Wd and 0 <= Y < Hd:
                        px[Y][X] = 1 if v == 1 else 4
            x += adv
        return [[px[ty * 8 + j // 8][tx * 8 + j % 8] for j in range(64)] for ty in range(ht) for tx in range(wt)]

    def gauge(self, side, layout, m, numbers=True, exp=True, name=True):
        """one of Black's battle gauges as btlv_gauge.c builds it, on a canvas whose (64, 16) is the frame's point:
        SIDE 1 the foe's, 0 the player's; LAYOUT 0 single / double frames, 2 triple; M the POKéMON (common.py)"""
        g = self.gauge_parts()
        a, parts, P = self.GAUGE, g['parts'], g['pal']
        ch = (0xA5 if side else 0xA8) if layout < 2 else (0xAB if side else 0xAE)
        ts = self.tiles(a, ch)

        def put(dst, src):
            for k, t in enumerate(src):
                ts[dst // 32 + k] = list(t)
        gender = {'m': 0, 'f': 1}.get(m.get('sex'), 2)
        if name:
            w = dsfont.width(self.fsub, m['name'])
            b = self._bmp(m['name'], 8, 2, 2 if w > 48 else 8, 5, self.fsub)
            if side:
                put(0x40, b[0:6]), put(0x140, b[8:14]), put(0x400, b[6:8]), put(0x500, b[14:16])
            else:
                put(0x20, b[0:7]), put(0x120, b[8:15]), put(0x400, b[7:8]), put(0x500, b[15:16])
            gt, gb = (parts[28 + 2 * gender], parts[29 + 2 * gender]) if gender < 2 else (parts[51], parts[51])
            o = 0x440 if side else 0x420
            put(o, [gt]), put(o + 0x100, [gb])
            lb = self._bmp(str(m['level']), 3, 2, 0, 5, self.fsub)
            o = 0x480 if side else 0x460
            put(o, lb[0:3]), put(o + 0x100, lb[3:6])
        canvas = Image.new('RGBA', (160, 48), (0, 0, 0, 0))

        def at(img_x0_y0, cx, cy):
            img, x0, y0 = img_x0_y0
            canvas.alpha_composite(img, (64 + cx + x0, 16 + cy + y0))
        at(self.cell(a, ch + 1, ts, P), 0, 0)
        # the HP bar (6 cells of 8 pixels after its cap), green / yellow / red by BGWINFRM_GetScaleRank
        frac = m['hp'] / m['maxhp'] if 'maxhp' in m else m['hp']
        bt = self.tiles(a, 0xB1)
        px = round(48 * frac)
        colour = 0 if frac > 0.5 else 1 if frac > 0.2 else 2
        for i in range(6):
            bt[i + 1] = list(parts[(colour * 0x120) // 32 + max(0, min(8, px - 8 * i))])
        bo = g['ov'].bar[side]
        at(self.cell(a, 0xB2, bt, P), bo[0], bo[1])
        if side == 0 and numbers and 'maxhp' in m:
            nt = self.tiles(a, 0xBA)
            hp, mx = str(m['hp']), str(m['maxhp'])
            cur = [None] * (3 - len(hp)) + [int(c) for c in hp]
            top = [int(c) for c in mx] + [None] * (3 - len(mx))
            for i in range(3):
                nt[i] = list(parts[(0x520 + 32 * cur[i]) // 32] if cur[i] is not None else parts[0x660 // 32])
                nt[i + 4] = list(parts[(0x520 + 32 * top[i]) // 32] if top[i] is not None else parts[0x660 // 32])
            at(self.cell(a, 0xBB, nt, P), 16, 13 if layout == 0 else 7)
        if side == 0 and exp and layout < 2 and 'exp' in m:
            et = self.tiles(a, 0xB7)
            px = 0 if m['level'] >= 100 else round(80 * m['exp'])
            for i in range(10):
                et[i + 1] = list(parts[0x400 // 32 + max(0, min(8, px - 8 * i))])
            at(self.cell(a, 0xB8, et, P), 8, 21)
        return canvas

    def hud(self, im, r, m, kind):
        x0, y0, x1, y1 = r
        self._rect = tuple(r)
        foe = kind.startswith('foe')
        if kind in ('foe3', 'mine3'):
            pic, inks = self.triple_card(foe, x1 - x0, y1 - y0, m)
            paste(im, pic, x0, y0)
            self.seam_ok.setdefault(self._rect, []).extend([(x0, y0, x1, y0 + 10),
                                                            (x1 - 12, y0, x1 + 2, y1) if foe else (x0 - 2, y0, x0 + 8, y1)])
            for (a, b, c, d), s in inks:
                self.inks.append(((x0 + a, y0 + b, x0 + c, y0 + d), s))
            return
        g = self.gauge(1 if foe else 0, 0, m, numbers=(kind == 'mine'))
        bb = g.getbbox()
        if kind == 'mine2' and bb[3] - bb[1] > y1 - y0:
            # (a double's box has no HP numbers: the dark band under the bar that holds them is shortened)
            g = carve(g, "y", bb[3] - bb[1] - (y1 - y0), 22, 31)
            bb = g.getbbox()
        # the frame's own edge where KAI's box starts (the arrow's point); the bar runs on to the screen's edge.
        # Where that would put the foe's name, or the player's level, off the screen, the plain bar is shortened
        # (the foe's between its level and its point, the player's between its point and its HP label) and what
        # is still missing moves the frame in
        w = dsfont.width(self.fsub, m['name'])
        if foe:
            fx = x1 - 128
            short = 2 - (fx + 16 + (2 if w > 48 else 8))
            if short > 0:
                n = min(short, 7)
                g = carve(g, 'x', n, 112, 120)
                fx = x1 - (128 - n) + max(0, short - n)
        else:
            fx = x0
            over = fx + 88 + dsfont.width(self.fsub, str(m['level'])) - (W - 2)
            if over > 0:
                free = 10 if w > 48 else 16
                n = min(over, free - 6)
                g = carve(g, 'x', n, 6, free)
                fx -= max(0, over - n)
                self._shift = n
        fy = y0 + ((y1 - y0) - (bb[3] - bb[1])) // 2 - bb[1]
        paste(im, g, fx, fy)
        self._gap = [y for y in range(18, 44) if not g.crop((0, y, g.width, y + 1)).getbbox()
                     and g.crop((0, y - 1, g.width, y)).getbbox() and g.crop((0, y + 1, g.width, y + 2)).getbbox()]
        self._gauge_inks(m, foe, fx, fy, getattr(self, '_shift', 0) if not foe else 0)
        self._shift = 0

    def _gauge_inks(self, m, foe, fx, fy, cut=0):
        """the gauge's words for check.py (the name, the level's digits), and where the scene may show through
        Black's floating words, its gender mark and Lv icon, and between its arrow's point's steps"""
        P = self.gauge_parts()['pal']
        ink, sh = rgb(P[1]), rgb(P[4])
        w = dsfont.width(self.fsub, m['name'])
        nx = (16 if foe else 8) + (2 if w > 48 else 8) - (cut if not foe else 0)
        self.log(None, nx, 5, self.fsub, m['name'], ink, sh, fx, fy)
        lx = (96 if foe else 88) - (cut if not foe else 0)
        self.log(None, lx, 5, self.fsub, str(m['level']), ink, sh, fx, fy)
        rows = (fx + nx - 1, fy + 5, fx + lx + 20, fy + 18)
        point = (fx + 112, fy + 14, fx + 130, fy + 26) if foe else (fx - 2, fy + 14, fx + 8, fy + 34)
        # (the player's EXP bar floats a row under its box, as Black's)
        gaps = [(fx, fy + y, fx + 132, fy + y + 1) for y in getattr(self, '_gap', [])]
        self.seam_ok.setdefault(self._rect, []).extend([rows, point] + ([] if foe else gaps))

    def triple_card(self, foe, w, h, m):
        """a Triple Battle's card: Black's triple gauge (0xAB / 0xAE) with the plain bar on either side of the HP
        gauge shortened to the card's width; above it the name, the gender, the Lv icon (lifted from the frame) and
        the level, one after another as Black's own row puts them, from the card's left. Where a ten-letter name at
        level 100 is wider than the card, that name is written in KAI's small font (as PLATINUM's cards do)"""
        g = self.gauge(1 if foe else 0, 2, m, numbers=False, name=False)
        P = self.gauge_parts()['pal']
        # the frame's Lv icon (the orange pixels in the name row) lifted off
        lv_box = (86, 6, 98, 16) if foe else (79, 6, 91, 16)
        lv = g.crop(lv_box)
        lv = lv.crop(lv.getbbox())
        g.paste((0, 0, 0, 0), lv_box)
        if foe:
            # (the bar: plain 0-32, the HP label 33-46, the gauge 47-96, plain 97-117, the arrow's point 118-127: the
            # plain on the right shortened to two columns, then the plain on the left to what the card has room for)
            right = min(19, g.getbbox()[2] - w)
            g = carve(g, 'x', right, 97, 118) if right > 0 else g
            left = g.getbbox()[2] - w
            g = carve(g, 'x', left, 0, 33) if left > 0 else g
        else:
            # (the point 0-4, plain 5-39, the HP label 40-53, the gauge 54-104, plain on: the plain before the label
            # shortened so the gauge ends two columns inside the card)
            g = carve(g, 'x', max(0, 107 - w), 5, 40)
            g = g.crop((0, 0, w, g.height))
        bb = g.getbbox()
        card = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        dy = (h - (bb[3] - 6)) // 2 - 6     # (the words' ink starts on the canvas's row 6)
        paste(card, g, 0, dy)
        ink, sh = rgb(P[1]), rgb(P[4])
        parts = self.gauge_parts()['parts']
        gender = {'m': 0, 'f': 1}.get(m.get('sex'), 2)
        x0 = 2
        lvw = dsfont.width(self.fsub, str(m['level']))
        nw = dsfont.width(self.fsub, m['name'])
        # (Black's own row, name / gender / Lv / level, is wider than KAI's card only for a ten-letter name at a
        # level of three digits: then the Lv mark gives way, the level's digits stay)
        mark = x0 + nw + (8 if gender < 2 else 0) + 1 + lv.width + lvw <= w
        inks = []
        y = 5 + dy
        x = self.draw(card, self.fsub, x0, y, m['name'], ink, sh)
        self.log(None, x0, y, self.fsub, m['name'], ink, sh)
        inks.append(self.inks.pop())
        if gender < 2:
            gt = Image.new('RGBA', (8, 16), (0, 0, 0, 0))
            gp = gt.load()
            for k, t in enumerate((parts[28 + 2 * gender], parts[29 + 2 * gender])):
                for j, v in enumerate(t):
                    if v:
                        gp[j % 8, k * 8 + j // 8] = rgb(P[v]) + (255,)
            paste(card, gt, x, y - 5)
            x += 8
        if mark:
            paste(card, lv, x + 1, y + 2 + (lv_box[1] + 2 - 6) - 1)
            x += 1 + lv.width
        self.draw(card, self.fsub, x, y, str(m['level']), ink, sh)
        self.log(None, x, y, self.fsub, str(m['level']), ink, sh)
        inks.append(self.inks.pop())
        return card, inks

    def kai_small(self, im, x, y, s, ink, shadow):
        """S in KAI's small font (graphics/fonts/latin_small.png, widths from src/fonts.c), Black's colours"""
        if not getattr(self, '_ks', None):
            import re
            import fr
            root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
            src = open(os.path.join(root, 'src', 'fonts.c')).read()
            body = re.search(r'gFontSmallLatinGlyphWidths\[\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
            widths = [int(v) for v in re.findall(r'\d+', re.sub(r'//.*', '', body))]
            self._ks = (Image.open(os.path.join(root, 'graphics', 'fonts', 'latin_small.png')), widths, fr.code)
        sheet, widths, code = self._ks
        sp, p = sheet.load(), im.load()
        for ch in s:
            k = code(ch)
            gx, gy = (k % 16) * 16, (k // 16) * 16
            for yy in range(16):
                for xx in range(widths[k]):
                    v = sp[gx + xx, gy + yy]
                    if v in (1, 2) and 0 <= x + xx < im.width and 0 <= y + yy < im.height:
                        p[x + xx, y + yy] = (ink if v == 1 else shadow) + (255,)
            x += widths[k]
        return x

    # --------------------------------------------------------------------------------------------- the battle's panels
    def blend(self, im, layer, x, y, evb=7):
        """LAYER over IM as Black's message window is blended (btlv_scu.c: BG1 first target at EVA 16, the rest
        second at EVB 7): each drawn pixel is its colour plus 7/16 of what lies under it"""
        p, q = im.load(), layer.load()
        for yy in range(layer.height):
            for xx in range(layer.width):
                c = q[xx, yy]
                if c[3] and 0 <= x + xx < im.width and 0 <= y + yy < im.height:
                    u = p[x + xx, y + yy]
                    p[x + xx, y + yy] = tuple(min(248, c[k] + (u[k] * evb >> 4) & 0xF8) for k in range(3)) + (255,)

    def msg_window(self, w, h):
        """Black's battle message window (btlv_scu.c: BMP_WIN_DrawFrame with a/0/1/1's frame 0x59 in palette 0x5A, the
        window cleared to colour 12) W x H pixels, frame included"""
        ts = self.tiles(self.GAUGE, 0x59)
        P = self.pal(self.GAUGE, 0x5A)
        tw, th = w // 8, h // 8
        out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        p = out.load()

        def tile(t, tx, ty):
            for i, v in enumerate(ts[t]):
                if v:
                    p[tx * 8 + i % 8, ty * 8 + i // 8] = rgb(P[v]) + (255,)
        for tx in range(tw):
            tile(0 if tx == 0 else 2 if tx == tw - 1 else 1, tx, 0)
            tile(5 if tx == 0 else 7 if tx == tw - 1 else 6, tx, th - 1)
        for ty in range(1, th - 1):
            tile(3, 0, ty)
            tile(4, tw - 1, ty)
            for tx in range(1, tw - 1):
                for i in range(64):
                    p[tx * 8 + i % 8, ty * 8 + i // 8] = rgb(P[12]) + (255,)
        return out

    MSG_INK = ((248, 248, 248), (104, 104, 104))     # (GF_FONT_SetTextColor(1, 9, 12) in palette 0x5A)

    def battle_msg(self, im, r=Lo.BATTLE_MSG):
        x0, y0, x1, y1 = r
        layer = Image.new('RGBA', im.size, (0, 0, 0, 0))
        layer.paste(self.msg_window(x1 - x0, y1 - y0), (x0, y0))
        ink, sh = self.MSG_INK
        for i, line in enumerate(PROMPT):
            self.text(layer, x0 + 8, y0 + 8 + 16 * i, line, ink, sh)
        self.blend(im, layer, 0, 0)

    def touch(self):
        """Black's battle touch screen as the command screen shows it: the background (8 bits, 0x71 / 0x72), the
        POKé BALL (0x68) and the BAG / RUN / POKéMON buttons (0x5D), all in palette 0x5C"""
        if not getattr(self, '_touch', None):
            a = self.GAUGE
            P = self.pal(a, 0x5C)
            out = Image.new('RGBA', (256, 192), (0, 0, 0, 255))
            for ch, sc in ((0x71, 0x72), (0x5B, 0x68), (0x5B, 0x5D)):
                out.alpha_composite(self.screen(a, ch, sc, None, palette=P).crop((0, 0, 256, 192)))
            self._touch = out
        return self._touch.copy()

    @staticmethod
    def letters(im, box, ground):
        """the word in BOX of a button's art: [(x0, x1)] each letter's columns with its outline (letters share the
        outline column between them); GROUND the button's own colours behind the word"""
        x0, y0, x1, y1 = box
        p = im.load()
        fill = [x for x in range(x0, x1) if any(p[x, y][:3] not in ground and p[x, y][:3] not in OUTLINE
                                                 for y in range(y0, y1))]
        runs = []
        for x in fill:
            if runs and x == runs[-1][1]:
                runs[-1][1] = x + 1
            else:
                runs.append([x, x + 1])
        return [(a - 1, b + 1) for a, b in runs]

    def relabel(self, im, box, ground, order):
        """the word in BOX spelt again from its own letters in ORDER (indices of its letters), centred where it was,
        the button's mesh laid back where the old word stood"""
        x0, y0, x1, y1 = box
        lets = self.letters(im, box, ground)
        word = im.crop((x0, y0, x1, y1))
        # (the ground behind the word: the mesh repeats every five columns, so each column takes the one five away
        # that the word doesn't cover)
        p = im.load()
        cols = [x for x in range(x0, x1) if any(p[x, y][:3] not in ground for y in range(y0, y1))]
        span0, span1 = min(cols), max(cols) + 1
        for x in range(span0, span1):
            sx = x
            while span0 <= sx < span1:
                sx -= 5
            for y in range(y0, y1):
                p[x, y] = p[sx, y]
        pieces = [word.crop((a - x0, 0, b - x0, y1 - y0)) for a, b in (lets[i] for i in order)]
        total = sum(pc.width for pc in pieces) - (len(pieces) - 1)
        x = (span0 + span1) // 2 - total // 2
        for pc in pieces:
            q = pc.load()
            for yy in range(pc.height):
                for xx in range(pc.width):
                    c = q[xx, yy]
                    if c[:3] not in ground:
                        p[x + xx, y0 + yy] = c
            x += pc.width - 1
        return im

    @staticmethod
    def squash_band(band, h, text=(41, 54), k=0.1, red=2, white=2):
        """the POKé BALL's band (rows from the red to the white, the oval button in the middle) H rows tall: each
        column keeps its outline runs and the TEXT rows whole; the red above, the oval's inside and the white below
        are shortened (the oval's inside to K of its length, so the button still bulges), each column on its own"""
        p = band.load()
        W, Hs = band.size

        def kind(c):
            r, g, b = c[:3]
            if r >= 40 and g < 20 and b <= 40:         # (the red and its dark shading, not FIGHT!'s letters)
                return 'red'
            if min(r, g, b) >= 150:
                return 'white'
            if (r, g, b) in ((40, 32, 32), (64, 48, 48), (72, 56, 56)):
                return 'in'
            return 'line'
        out = Image.new('RGBA', (W, h))
        q = out.load()
        t0, t1 = text
        for x in range(W):
            col = [p[x, y] for y in range(Hs)]
            ks = [kind(c) for c in col]
            top = [y for y in range(t0) if ks[y] != 'red']
            a0 = top[0] if top else t0                 # (where the red ends)
            a1 = a0
            while a1 < t0 and ks[a1] == 'line':
                a1 += 1                                # (the outline above the button's inside)
            bot = [y for y in range(t1, Hs) if ks[y] != 'white']
            b1 = bot[-1] + 1 if bot else t1            # (where the white starts)
            b0 = b1
            while b0 > t1 and ks[b0 - 1] == 'line':
                b0 -= 1                                # (the outline under it)
            i_top = round((t0 - a1) * k)
            i_bot = round((b0 - t1) * k)
            up, down = col[a0:a1], col[b0:b1]
            up = up[:1] + up[-1:] if len(up) > 2 else up            # (each outline its outer and inner rows)
            down = down[:1] + down[-1:] if len(down) > 2 else down
            seq = up + col[t0 - i_top:t0] + col[t0:t1] + col[t1:t1 + i_bot] + down
            spare = h - len(seq)
            red_n = max(red, spare // 2)
            white_n = max(white, spare - red_n)
            seq = [col[14]] * red_n + seq + [col[78]] * white_n     # (plain red, plain white)
            if len(seq) > h:                            # (too tall: the outer rows give way)
                over = len(seq) - h
                seq = seq[over // 2:len(seq) - (over - over // 2)]
            for y, c in enumerate(seq[:h]):
                q[x, y] = c
        return out

    def battle_menu(self, im, r=Lo.BATTLE_MENU):
        """Black's command screen in KAI's room for it: the POKé BALL's band with FIGHT! over the three buttons,
        each cut down by whole runs of rows and columns (the ball's red and white, the button's roofs and mesh), the
        words kept; POKéMON spelt MOéMON from its own letters"""
        x0, y0, x1, y1 = r
        w, h = x1 - x0, y1 - y0
        t = self.touch()
        mesh = {(56, 56, 56), (64, 64, 64), (72, 72, 72)}
        self.relabel(t, (193, 159, 248, 175), mesh, [4, 1, 3, 4, 1, 6])
        bh = 24                          # (the band's rows; the buttons get the rest)
        band = t.crop((0, 40, 256, 120))
        # (the band's ends beyond the oval, and the oval's flat top either side of FIGHT!, shortened alike)
        for n_, lo_, hi_ in ((43, 206, 256), (25, 155, 182), (25, 74, 101), (43, 0, 50)):
            band = carve(band, 'x', n_, lo_, hi_)
        band = self.squash_band(band, bh)
        # each button on its own: (its columns, its rows, where its word is)
        row = Image.new('RGBA', (w, h - bh))
        row.paste(t.crop((0, 120, w, 120 + h - bh)).resize((1, 1)).resize((w, h - bh)), (0, 0))
        # each button: its roof's top rows and its word's rows (Black's buttons run on under the screen's foot)
        specs = [((0, 80), list(range(132, 144)) + list(range(162, 174))),
                 ((92, 170), list(range(150, 162)) + list(range(172, 184))),
                 ((176, 256), list(range(132, 144)) + list(range(162, 174)))]
        widths = [34, 28, 54]
        xs = [0, 36, 66]
        for ((bx0, bx1), rows), bw_, bx in zip(specs, widths, xs):
            b_ = _rebuild(t.crop((bx0, 0, bx1, 192)), 'y', rows)
            q = b_.load()
            lab = [x for x in range(b_.width) for y in range(b_.height) if max(q[x, y][:3]) - min(q[x, y][:3]) >= 48]
            b_ = carve(b_, 'x', b_.width - bw_, 0, b_.width, keep_out=[(min(lab) - 2, max(lab) + 3)])
            row.alpha_composite(b_, (bx, h - bh - b_.height))
        out = Image.new('RGBA', (w, h))
        out.paste(band, (0, 0))
        out.paste(row, (0, bh))
        paste(im, out, x0, y0)

    # the move buttons (btlv_input.c BTLV_INPUT_DrawWazaButtons): a type's palette (overlay 94's d_0220A5F0) into
    # the button's palette row; the type icons (a/0/8/3: 0x22 + type, cell 0x3B, palette 0x21 at the row
    # _PokeTypePltOffset gives)
    TYPES = ['NORMAL', 'FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', 'FIRE', 'WATER',
             'GRASS', 'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK']
    TYPE_PAL = [0xC2, 0xC8, 0xCB, 0xC9, 0xCA, 0xCE, 0xCD, 0xCF, 0xD1, 0xC3, 0xC4, 0xC6, 0xC5, 0xC7, 0xD0, 0xCC, 0xD2]
    TYPE_ICON_ROW = [0, 0, 1, 1, 0, 0, 2, 1, 0, 0, 1, 2, 0, 1, 1, 2, 0]
    MOVE_INK = ((248, 248, 248), (112, 112, 112))     # (GF_FONT_SetTextColor(1, 2, 0) in palette 0x5C's row 13)

    def type_icon(self, kind):
        t = self.TYPES.index(kind)
        P = self.pal('a/0/8/3', 0x21)
        row = self.TYPE_ICON_ROW[t]
        im, _, _ = self.cell('a/0/8/3', 0x3B, self.tiles('a/0/8/3', 0x22 + t), P[row * 16:] + P)
        return im

    def move_button(self, kind, w, h):
        """the first move button of Black's move screen (0x68's block at (256, 256)) in the type's colours, cut down
        to W x H: its arched top and bottom kept, the mesh between shortened, its ends kept"""
        a = self.GAUGE
        P = list(self.pal(a, 0x5C))
        tp = self.pal(a, self.TYPE_PAL[self.TYPES.index(kind)])
        P[9 * 16:10 * 16] = tp[:16]
        b = self.screen(a, 0x5B, 0x68, None, palette=P).crop((258, 225, 383, 272))
        b = carve(b, 'x', b.width - w, 12, b.width - 12)
        b = carve(b, 'y', b.height - h, 8, b.height - 8)
        return b

    def move_panel(self, im):
        x0, y0, x1, y1 = Lo.MOVES
        a = self.GAUGE
        P = self.pal(a, 0x5C)
        ground = Image.new('RGBA', (x1 - x0, y1 - y0))
        gp = ground.load()
        for y in range(ground.height):         # (the touch screen's dark ground, its stripes two rows apart)
            for x in range(ground.width):
                gp[x, y] = rgb(P[16 + (0 if (y // 2) % 2 else 11)]) + (255,)
        paste(im, ground, x0, y0)
        bw_, bh_ = 80, 24
        ink, sh = self.MOVE_INK
        for i, (n, pp, mx, kind) in enumerate(MOVES):
            bx, by = x0 + (i % 2) * 80, y0 + (i // 2) * 24
            paste(im, self.move_button(kind, bw_, bh_), bx, by)
            # (Black's small letters: the cut-down button's inside is ten rows, its main font's eleven)
            self.text(im, bx + bw_ // 2, by + 5, n, ink, sh, 's', 'c')
        # (the chosen move's own row from its button: the type's icon and its PP, on the same button)
        x0, y0, x1, y1 = Lo.MOVE_INFO
        n, pp, mx, kind = MOVES[0]
        paste(im, ground.crop((0, 0, x1 - x0, y1 - y0)), x0, y0)
        paste(im, self.move_button(kind, x1 - x0, y1 - y0), x0, y0)
        icon = self.type_icon(kind)
        paste(im, icon, x0 + (x1 - x0 - icon.width) // 2, y0 + 7)
        self.text(im, x0 + 14, y0 + 25, 'PP', ink, sh)
        self.text(im, x1 - 14, y0 + 25, '%d/%d' % (pp, mx), ink, sh, 'n', 'r')

    # --------------------------------------------------------------------------------------------- the field
    TALK = 'a/0/0/5'
    INK = ((88, 80, 80), (160, 160, 168))           # (a/0/2/3 palette 5: letters 1, shadow 2, on 15 white)

    def balloon(self, w, h):
        """Black's talk balloon (talkmsgwin.c: a/0/0/5 char 5, palette 0): the rounded corner (tile 1, turned for
        each corner), the edges (tiles 3 and 4, turned), the inside cleared to white (15)"""
        ts = self.tiles(self.TALK, 5)
        P = self.pal(self.TALK, 0)
        out = Image.new('RGBA', (w, h), (0, 0, 0, 0))
        p = out.load()
        tw, th = w // 8, h // 8

        def tile(t, tx, ty, hf=False, vf=False):
            for i, v in enumerate(ts[t]):
                x, y = i % 8, i // 8
                if hf:
                    x = 7 - x
                if vf:
                    y = 7 - y
                if v:
                    p[tx * 8 + x, ty * 8 + y] = rgb(P[v]) + (255,)
        for ty in range(1, th - 1):
            for tx in range(1, tw - 1):
                for i in range(64):
                    p[tx * 8 + i % 8, ty * 8 + i // 8] = rgb(P[15]) + (255,)
        tile(1, 0, 0), tile(1, tw - 1, 0, True), tile(1, 0, th - 1, False, True), tile(1, tw - 1, th - 1, True, True)
        for tx in range(1, tw - 1):
            tile(3, tx, 0), tile(3, tx, th - 1, False, True)
        for ty in range(1, th - 1):
            tile(4, 0, ty), tile(4, tw - 1, ty, True)
        return out

    def next_arrow(self, im, x, y):
        """Black's waiting cursor (field_msgbg.c: a/0/0/5 char 12), its first frame"""
        ts = self.tiles(self.TALK, 12)
        P = self.pal(self.TALK, 0)
        p = im.load()
        for i, v in enumerate(ts[0]):
            if v:
                p[x + i % 8, y + i // 8] = rgb(P[v]) + (255,)

    def message(self):
        from common import MESSAGE
        im = shot(self.kai, 'f_mart')
        x0, y0, x1, y1 = Lo.FIELD_MSG
        paste(im, self.balloon(x1 - x0, y1 - y0), x0, y0)
        ink, sh = self.INK
        end = self.text(im, x0 + 10, y0 + 10, MESSAGE, ink, sh)
        self.next_arrow(im, end + 2, y0 + 12)
        return im

    # --------------------------------------------------------------------------------------------- the X menu
    XMENU = 'a/0/7/1'
    # (field_menu.c gFieldMenuIconRes in overlay 21: (character, cell) of each button's icon; KAI's START rows
    # to Black's buttons: MOéDEX the POKéDEX, MOéMON the POKé BALL, BAG, the TRAINER CARD, SAVE's journal, OPTIONS' DS)
    XICONS = {'MOéDEX': (11, 26), 'MOéMON': (8, 23), 'BAG': (6, 21), 'TEST': (10, 25), 'SAVE': (9, 24),
              'OPTIONS': (5, 20)}
    XINK = ((248, 248, 248), (152, 152, 152))       # (0x440 in palette 0's row 13)

    def x_icon(self, key):
        c, ce = self.XICONS[key]
        im, _, _ = self.cell(self.XMENU, ce, self.tiles(self.XMENU, c), self.pal(self.XMENU, 1))
        return im.crop(im.getbbox())

    def start_menu(self):
        """Black's X menu (field_menu.c) at the screen's right: its ground (0x0E: the grid between two grey bars),
        its six buttons two by two (0x10) with their icons and words, the chosen one Black's lit button (cell 28);
        KAI's seventh row, EXIT, on the bottom bar as Black's return button (a/0/8/3's touch bar)"""
        from common import START_MENU
        im = shot(self.kai, 'f_town')
        A = self.XMENU
        W_, H_ = 152, 160
        x0, y0 = W - W_, 0
        bg = self.screen(A, 2, 0x0E, 0)
        bg = carve(bg, 'x', bg.width - W_, 8, 248)
        bg = carve(bg, 'y', bg.height - H_, 0, bg.height, keep_out=[(10, 24), (164, 182)])
        paste(im, bg, x0, y0)
        tabs = self.screen(A, 2, 0x10, 0)
        tab = tabs.crop((3, 24, 126, 67))
        tw, th = 72, 36
        tab = carve(carve(tab, 'x', tab.width - tw, 40, tab.width - 6), 'y', tab.height - th, 12, tab.height - 8)
        lit, _, _ = self.cell(A, 28, self.tiles(A, 13), self.pal(A, 1))
        lit = lit.crop(lit.getbbox())
        lit = carve(carve(lit, 'x', lit.width - tw, 40, lit.width - 6), 'y', lit.height - th, 12, lit.height - 8)
        ink, sh = self.XINK
        items = [k for k in START_MENU if k != 'EXIT']
        for i, key in enumerate(items):
            bx, by = x0 + 3 + (i % 2) * (tw + 2), y0 + 14 + (i // 2) * (th + 6)
            paste(im, lit if i == 0 else tab, bx, by)
            ic = self.x_icon(key)
            paste(im, ic, bx + 4 + (22 - ic.width) // 2, by + (th - ic.height) // 2)
            self.text(im, bx + 26, by + (th - 16) // 2 + 1, key, ink, sh, 'n' if self.measure(key) <= tw - 29 else 's')
        # (EXIT: the bottom bar's return button)
        T = 'a/0/8/3'
        ret, _, _ = self.cell(T, 21, self.tiles(T, 20), self.pal(T, 19), 1)
        ret = ret.crop(ret.getbbox())
        ex = x0 + W_ - 8 - ret.width
        paste(im, ret, ex, y0 + H_ - 4 - ret.height - 4)
        self.text(im, ex - 4, y0 + H_ - 18, 'EXIT', ink, sh, 'n', 'r')
        return im

    # --------------------------------------------------------------------------------------------- the party
    PARTY = 'a/0/7/6'
    PLATE_INK = ((248, 248, 248), (128, 128, 128))    # (palette 0 into row 13: letters 1, shadow 2)
    MALE_INK, FEMALE_INK = ((0, 232, 248), (0, 104, 96)), ((224, 56, 56), (112, 32, 48))

    def plate(self, kind, w, h):
        """one of Black's party plates (pokelist.c / plist_plate.c: screen 0x12, the lead's from tile (0, 1), the
        others' from (16, 2), an empty slot's from (0, 19)) cut to W x H"""
        src = {'lead': (0, 8), 'other': (128, 16), 'empty': (0, 152)}[kind]
        pl = self.screen(self.PARTY, 12, 18, 6).crop((src[0], src[1], src[0] + 128, src[1] + 48))
        pl = carve(pl, 'x', 128 - w, 8, 120)
        return carve(pl, 'y', 48 - h, 8, 40)

    def party_hp(self, frac):
        """the party's HP bar (a/0/8/3: character 2, cell 5, palette 0) filled as plist_plate.c fills it"""
        A = 'a/0/8/3'
        P = self.pal(A, 0)
        im, _, _ = self.cell(A, 5, self.tiles(A, 2), P)
        p = im.load()
        n = round(48 * frac)
        colour = (5, 6) if frac > 0.5 else (9, 10) if frac > 0.2 else (7, 8)
        for i in range(n):
            p[16 + i, 3] = rgb(P[colour[0]]) + (255,)
            p[16 + i, 4] = rgb(P[colour[1]]) + (255,)
        return im

    def party(self):
        from common import PARTY, creature
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        bg = self.screen(self.PARTY, 15, 20, 6).crop((0, 0, 256, 192))
        bg = carve(carve(bg, 'x', 16, 0, 256), 'y', 32, 0, 192)
        paste(im, bg, 0, 0)
        pw, ph = 120, 44
        ink, sh = self.PLATE_INK
        for i in range(6):
            x, y = (i % 2) * pw, (i // 2) * (ph + 1)
            if i >= len(PARTY):
                paste(im, self.plate('empty', pw, ph), x, y)
                continue
            m = PARTY[i]
            paste(im, self.plate('lead' if i == 0 else 'other', pw, ph), x, y)
            ic = creature(self.export, m['species'], 'icon')
            paste(im, ic, x + 4, y + 4)
            self.text(im, x + 38, y + 3, m['name'], ink, sh)
            gi = self.MALE_INK if m['sex'] == 'm' else self.FEMALE_INK
            self.text(im, x + 104, y + 3, '♂' if m['sex'] == 'm' else '♀', gi[0], gi[1])
            paste(im, self.party_hp(m['hp'] / m['maxhp']), x + 42, y + 17)
            self.text(im, x + 8, y + 26, 'Lv%d' % m['level'], ink, sh, 's')
            self.text(im, x + 74, y + 26, '%d' % m['hp'], ink, sh, 's', 'r')
            self.text(im, x + 76, y + 26, '/', ink, sh, 's')
            self.text(im, x + 110, y + 26, '%d' % m['maxhp'], ink, sh, 's', 'r')
        # (the foot: Black's balloon with the prompt, the return button for CANCEL)
        paste(im, self.balloon(168, 24), 0, 136)
        self.text(im, 10, 140, 'Choose a MOéMON.', *self.INK)
        T = 'a/0/8/3'
        ret, _, _ = self.cell(T, 21, self.tiles(T, 20), self.pal(T, 19), 1)
        ret = ret.crop(ret.getbbox())
        paste(im, ret, W - 6 - ret.width, 140)
        self.text(im, W - 10 - ret.width, 141, 'CANCEL', *self.PLATE_INK, 'n', 'r')
        return im

    # --------------------------------------------------------------------------------------------- the summary
    SUMMARY_ARC = 'a/0/7/8'
    TEAL = ((64, 232, 248), (0, 104, 96))

    def respell(self, im, box, order, ground):
        """the word in BOX (letters apart, on a plain GROUND) spelt again from its own letters in ORDER (indices
        into its letters), from the word's own left"""
        x0, y0, x1, y1 = box
        p = im.load()
        cols = [x for x in range(x0, x1) if any(p[x, y][:3] != ground for y in range(y0, y1))]
        runs = []
        for x in cols:
            if runs and x == runs[-1][1]:
                runs[-1][1] = x + 1
            else:
                runs.append([x, x + 1])
        lets = [im.crop((a, y0, b, y1)) for a, b in runs]
        gaps = [runs[i + 1][0] - runs[i][1] for i in range(len(runs) - 1)]
        gap = min(gaps) if gaps else 1
        start = runs[0][0]
        for x in range(start, x1):
            for y in range(y0, y1):
                p[x, y] = ground + (255,)
        x = start
        for i in order:
            if i is None:            # (a space: the word's own space)
                x += max(gaps)
                continue
            im.paste(lets[i], (x, y0))
            x += lets[i].width + gap
        return im

    def summary_page(self, im):
        from common import SUMMARY as S_, MINE, creature
        A = self.SUMMARY_ARC
        top = Image.new('RGBA', (256, 192), (0, 0, 0, 255))
        top.alpha_composite(self.screen(A, 8, 70, 0).crop((0, 0, 256, 192)))
        top.alpha_composite(self.screen(A, 8, 63, 0).crop((0, 0, 256, 192)))
        paste(im, top.crop((0, 8, 240, 152)), 0, 16)
        # (the header: the touch screen's title bar, its POKéMON INFO spelt MOéMON INFO from its own letters)
        bar = self.screen(A, 9, 65, 1).crop((0, 8, 256, 24))
        q = bar.load()
        ground = q[100, 8][:3]
        self.respell(bar, (4, 4, 100, 12), [4, 5, 3, 4, 5, 6, None, 7, 8, 9, 10], ground)
        bar = bar.crop((0, 0, 240, 16))
        bp = bar.load()
        for x in range(150, 240):              # (its «STATUS» taken off: KAI's page name and six pages there)
            for y in range(1, 15):
                bp[x, y] = q[100, y]
        paste(im, bar, 0, 0)
        ti, ts_ = self.TEAL
        self.text(im, 236, 2, '«INFO»', ti, ts_, 's', 'r')
        for k in range(6):
            cx = 152 + k * 7
            for dy in range(3):
                for dx in range(3):
                    if k == 0 or dx in (0, 2) or dy in (0, 2):
                        im.putpixel((cx + dx, 7 + dy), ti + (255,))
        lab, lsh = (248, 248, 248), (128, 128, 128)
        ink, sh = self.INK
        rows = [('Dex No.', S_['no']), ('Name', S_['name']), ('Type', None), ('OT', S_['ot']), ('ID No.', S_['idno'])]
        for i, (k, v) in enumerate(rows):
            y = 16 + 16 * i + 1
            self.text(im, 16, y, k, lab, lsh)
            if v is not None:
                self.text(im, 86, y, v, ink, sh)
        paste(im, self.type_icon(S_['type']), 86, 16 + 32 + 1)
        self.text(im, 16, 97, 'Item', lab, lsh)
        self.text(im, 86, 97, S_['item'], lab, lsh)
        self.text(im, 58, 113, '%s nature.' % S_['nature'], ink, sh)
        self.text(im, 16, 130, '%s %s' % (S_['met'], S_['place']), lab, lsh, 's')
        self.text(im, 30, 145, 'Ability', ink, sh, 's')
        self.text(im, 72, 145, S_['ability'], ink, sh, 's')
        # (the POKéMON on the grid, its name and level above it)
        gi = self.MALE_INK if S_['sex'] == 'm' else self.FEMALE_INK
        nx = self.text(im, 159, 19, S_['name'], ink, sh)          # (on the white panel at the top right)
        self.text(im, nx + 1, 19, '♂' if S_['sex'] == 'm' else '♀', gi[0], gi[1])
        self.text(im, 166, 42, 'Lv%d' % S_['level'], lab, lsh, 's')
        pic = creature(self.export, MINE['species'])
        paste(im, pic, 196 - pic.width // 2, 46)

    # --------------------------------------------------------------------------------------------- the BAG
    BAG = 'a/0/8/8'
    # (itemmenu.c: the pocket tabs (character 0x0F, cells 0x0E, palette 0x10, a boy's), sTabAnimSeqs and the
    # animations' first cells: each pocket's cell when not chosen and when chosen; Black has no POKé BALLS, BATTLE
    # ITEMS or FAVORITES pocket: those take the X menu's POKé BALL, the battle BAG's BATTLE ITEMS mark and the
    # registered item's check)
    POCKET_CELLS = {'ITEMS': (4, 9), 'MEDICINE': (2, 7), 'TMs & HMs': (3, 8), 'BERRIES': (1, 6), 'KEY ITEMS': (0, 5)}

    def pocket_icon(self, name, chosen):
        A = self.BAG
        if name in self.POCKET_CELLS:
            im, _, _ = self.cell(A, 0x0E, self.tiles(A, 0x0F), self.pal(A, 0x10), self.POCKET_CELLS[name][chosen])
        elif name == 'POKé BALLS':
            im, _, _ = self.cell(self.XMENU, 23, self.tiles(self.XMENU, 8), self.pal(self.XMENU, 1))
        elif name == 'BATTLE ITEMS':
            im = self.screen('a/0/9/9', 0, 3, 1).crop((182, 0, 200, 24))
        else:
            T = 'a/0/8/3'
            im, _, _ = self.cell(T, 21, self.tiles(T, 20), self.pal(T, 19), 7)
        return im.crop(im.getbbox())

    def bag_page(self, im):
        from common import BAG_DESC, BAG_ITEMS, BAG_POCKET, POCKETS, item_icon
        A = self.BAG
        bg = self.screen(A, 3, 5, 4).crop((0, 0, 256, 168))
        bg = carve(carve(bg, 'x', 16, 0, 240), 'y', 8, 20, 150)
        paste(im, bg, 0, 0)
        lab, lsh = (248, 248, 248), (64, 64, 96)
        # (the chosen pocket's own tab, as big as Black draws it, its name above; the eight pockets as a row of
        # marks under it between the touch bar's << and >>)
        self.text(im, 52, 4, BAG_POCKET, lab, lsh, 'n', 'c')
        ic = self.pocket_icon(BAG_POCKET, True)
        paste(im, ic, 52 - ic.width // 2, 50 - ic.height // 2)
        T = 'a/0/8/3'
        for k, frame in ((0, 4), (1, 5)):
            ar, _, _ = self.cell(T, 21, self.tiles(T, 20), self.pal(T, 19), frame)
            ar = ar.crop(ar.getbbox())
            paste(im, ar, 4 if k == 0 else 100 - ar.width, 84 - ar.height // 2)
        cur = POCKETS.index(BAG_POCKET)
        for i in range(len(POCKETS)):
            x = 52 - (len(POCKETS) * 7) // 2 + i * 7
            for dy in range(4):
                for dx in range(4):
                    if i == cur or dx in (0, 3) or dy in (0, 3):
                        im.putpixel((x + dx, 82 + dy), lab + (255,))
        # (the list: each item on Black's row plate, its count at the right; the chosen row marked by the touch
        # bar's arrow)
        plate = self.screen(A, 0x21, 0x23, 0x22)
        plate = plate.crop(plate.getbbox())
        plate = carve(carve(plate, 'y', plate.height - 20, 6, plate.height - 6), 'x', plate.width - 128, 20, 120)
        T = 'a/0/8/3'
        arrow, _, _ = self.cell(T, 21, self.tiles(T, 20), self.pal(T, 19), 5)
        arrow = arrow.crop(arrow.getbbox())
        for i, (n, count, _icon) in enumerate(BAG_ITEMS):
            y = 6 + i * 21
            paste(im, plate, 108, y)
            self.text(im, 118, y + 2, n, lab, (64, 64, 72))
            self.text(im, 232, y + 2, 'x%d' % count, lab, (64, 64, 72), 'n', 'r')
            if i == 0:
                paste(im, arrow, 108 - arrow.width + 2, y + (18 - arrow.height) // 2)
        self.text(im, 232, 92, 'START: SORT', lab, lsh, 's', 'r')
        paste(im, self.balloon(240, 56), 0, 104)
        ic = item_icon(self.export, BAG_ITEMS[0][2])
        if ic is not None:
            paste(im, ic, 6, 116)
        for k, line in enumerate(BAG_DESC):
            self.text(im, 38, 109 + 15 * k, line, *self.INK)

    # --------------------------------------------------------------------------------------------- OPTIONS
    def options_page(self, im):
        """Black's OPTIONS (option_menu.c, a/0/2/7: the rows 0x02 over character 0x00, the title bar 0x0E over 0x0D,
        palette 0x01): KAI's five tabs on the title bar, its six rows on Black's six bars (the sixth, Black's grey
        one, as the others), each row's name and its choices on the bar"""
        from common import OPTION_ROWS, OPTION_TABS
        A = 'a/0/2/7'
        m = Image.new('RGBA', (256, 192), (0, 0, 0, 255))
        m.alpha_composite(self.screen(A, 0, 2, 1).crop((0, 0, 256, 192)))
        m.alpha_composite(self.screen(A, 13, 14, 1).crop((0, 0, 256, 192)))
        m.paste(m.crop((0, 116, 256, 140)), (0, 140))            # (the sixth bar like the five above it)
        m = carve(m.crop((0, 0, 256, 160)), 'x', 16, 20, 236)
        paste(im, m, 0, 0)
        green, gsh = (96, 248, 144), (8, 64, 48)
        white, wsh = (248, 248, 248), (48, 48, 64)
        dim = (152, 136, 216)
        x = 4
        for t in OPTION_TABS:
            on = t == 'DISPLAY'
            x = self.text(im, x, 11, t, green if on else (160, 160, 160), gsh if on else (40, 40, 40), 's') + 7
        chosen = {'UI STYLE': self.name, 'THEME': 'LIGHT', 'FRAME': 'TYPE 1', 'SHADOWS': 'ON',
                  'BATTLE ANIMS': 'MODERN', 'TYPE ICONS': 'ON'}
        for i, (label, vals) in enumerate(OPTION_ROWS):
            ty = [24, 46, 70, 94, 118, 142][i]          # (each of Black's bars' first row)
            self.text(im, 8, ty, label, white, wsh, 's')
            if vals is None:
                self.text(im, 232, ty, chosen[label], green, gsh, 's', 'r')
                continue
            xx = 232
            for v in reversed(vals):
                on = v == chosen[label]
                xx = xx - self.measure(v, 's')
                self.text(im, xx, ty, v, green if on else dim, gsh if on else wsh, 's')
                xx -= 8

    # --------------------------------------------------------------------------------------------- stand-ins
    def window(self, im, r, kind='menu'):
        pass

    def cursor(self, im, x, y):
        pass


class OverlayTables:
    """btlv_gauge.c's tables in overlay 94 (read from the ROM's own overlay; addresses from the decomp)"""
    def __init__(self, rom):
        sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'unova'))
        import gimmicks
        img = gimmicks.Image(94)

        def pts(a, n):
            return [struct.unpack_from('<hh', img.d, a - img.ram + 4 * i) for i in range(n)]
        self.bar = pts(0x0220AAD0, 2)          # sGaugeBarOffsets: the player's (8, 7), the foe's (0, 7)
        self.single = pts(0x0220AAD8, 6)       # sGaugeDefaultPositions
