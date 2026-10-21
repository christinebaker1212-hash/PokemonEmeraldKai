#!/usr/bin/env python3
"""Moemon Co-op: pixel art for the ball entrance / recall effects (my own art, drawn here).

Writes src/mp/mp_ballfx_gfx.h: one 16-color palette shared by every effect, the
sheets (4bpp, one per particle set, loaded only while that set is on screen) and
the "energy" sheet, whose pixels use placeholders the game fills with the
chosen energy color (E) and white (W).

usage: gen.py [--preview OUT.png]
"""
import math
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', '..', 'src', 'mp', 'mp_ballfx_gfx.h')

# 0 transparent; 1 white; 2-5 grays; 6-15 colors
PAL = [
    (0, 0, 0), (31, 31, 31), (24, 24, 26), (16, 16, 18), (8, 8, 9), (2, 2, 3),
    (31, 29, 8), (31, 18, 3), (29, 5, 4), (31, 14, 22), (19, 9, 27), (7, 13, 30),
    (10, 27, 30), (8, 25, 9), (18, 12, 6), (31, 24, 6),
]
W, LG, G, DG, K, Y, O, R, P, V, B, C, GR, BR, GD = range(1, 16)
E, EL, ED = 16, 17, 18   # energy color, its light and dark shade (filled in by the game)


class Img:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.p = [[0] * w for _ in range(h)]

    def set(self, x, y, c):
        x, y = int(round(x)), int(round(y))
        if 0 <= x < self.w and 0 <= y < self.h:
            self.p[y][x] = c

    def get(self, x, y):
        if 0 <= x < self.w and 0 <= y < self.h:
            return self.p[y][x]
        return 0

    def disc(self, cx, cy, r, c, only_empty=False):
        for y in range(self.h):
            for x in range(self.w):
                if (x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2 <= r * r:
                    if not only_empty or self.p[y][x] == 0:
                        self.p[y][x] = c

    def ellipse(self, cx, cy, rx, ry, c):
        for y in range(self.h):
            for x in range(self.w):
                if ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2 <= 1:
                    self.p[y][x] = c

    def ring(self, cx, cy, r, c, width=1.0):
        for y in range(self.h):
            for x in range(self.w):
                d = math.hypot(x + 0.5 - cx, y + 0.5 - cy)
                if r - width < d <= r:
                    self.p[y][x] = c

    def line(self, x0, y0, x1, y1, c):
        n = int(max(abs(x1 - x0), abs(y1 - y0))) + 1
        for i in range(n + 1):
            t = i / n
            self.set(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t, c)

    def poly(self, pts, c):
        # even-odd fill of a polygon
        for y in range(self.h):
            yy = y + 0.5
            xs = []
            for i in range(len(pts)):
                (x0, y0), (x1, y1) = pts[i], pts[(i + 1) % len(pts)]
                if (y0 <= yy < y1) or (y1 <= yy < y0):
                    xs.append(x0 + (yy - y0) * (x1 - x0) / (y1 - y0))
            xs.sort()
            for a, b in zip(xs[::2], xs[1::2]):
                for x in range(self.w):
                    if a <= x + 0.5 <= b:
                        self.p[y][x] = c

    def outline(self, c, inside_of=None):
        # a 1px outline around the filled shape (on empty pixels)
        src = [row[:] for row in self.p]
        for y in range(self.h):
            for x in range(self.w):
                if src[y][x] != 0:
                    continue
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    v = src[y + dy][x + dx] if 0 <= x + dx < self.w and 0 <= y + dy < self.h else 0
                    if v != 0 and (inside_of is None or v in inside_of):
                        self.p[y][x] = c
                        break


# ---------------------------------------------------------------------------
# The sets. Each frame: (name, Img). Sizes: 8x8, 16x8, 8x16, 16x16, 32x16, 32x32, 32x64, 64x64

def confetti():
    frames = []
    for col in (R, B, GR, Y, P, C):
        a = Img(8, 8); a.poly([(2, 3), (6, 2), (6.5, 4.5), (2.5, 5.5)], col); a.set(5, 3, W if col in (B, R, V) else col)
        b = Img(8, 8); b.poly([(3, 2), (5, 2.5), (5, 6), (3, 5.5)], col)
        c = Img(8, 8); c.line(2, 4, 6, 3, col)
        frames += [('confetti%d_%d' % (col, i), f) for i, f in enumerate((a, b, c))]
    return frames


def explosion():
    random.seed(7)
    flash = Img(32, 32)
    for k in range(8):
        ang = k * math.pi / 4 + math.pi / 8
        L = 15 if k % 2 == 0 else 11
        pts = [(16 + math.cos(ang) * L, 16 + math.sin(ang) * L),
               (16 + math.cos(ang + 0.35) * 5, 16 + math.sin(ang + 0.35) * 5),
               (16 + math.cos(ang - 0.35) * 5, 16 + math.sin(ang - 0.35) * 5)]
        flash.poly(pts, Y)
    flash.disc(16, 16, 7, Y)
    flash.disc(16, 16, 5.5, W)
    blasts = []
    for i, (r, holes) in enumerate(((8, 0), (13, 0), (14.5, 6))):
        b = Img(32, 32)
        for k in range(9):
            ang = k * 2 * math.pi / 9
            b.disc(16 + math.cos(ang) * r * 0.55, 16 + math.sin(ang) * r * 0.55, r * 0.5, R)
        b.disc(16, 16, r * 0.8, R)
        for k in range(9):
            ang = k * 2 * math.pi / 9 + 0.3
            b.disc(16 + math.cos(ang) * r * 0.45, 16 + math.sin(ang) * r * 0.45, r * 0.4, O)
        b.disc(16, 15, r * 0.55, O)
        b.disc(15, 14, r * 0.42, Y)
        if i == 0:
            b.disc(15, 14, r * 0.25, W)
        if holes:
            for h in range(holes):
                ang = random.random() * 2 * math.pi
                d = r * (0.3 + random.random() * 0.5)
                b.disc(16 + math.cos(ang) * d, 16 + math.sin(ang) * d, 2.2, BR)
            b.disc(16, 16, 3, DG)
        b.outline(BR, inside_of=(R,))
        blasts.append(b)
    soot = []
    for i, s in enumerate((1.0, 0.75)):
        a = Img(16, 16)
        for (x, y, r) in ((6, 9, 4.2), (10, 8, 4.5), (8, 5.5, 3.8)):
            a.disc(x, y, r * s + (1 - s) * 1.5, DG)
        for (x, y, r) in ((7, 5, 1.6), (10.5, 6.5, 1.4)):
            a.disc(x, y, r * s, G)
        a.outline(K)
        soot.append(a)
    return [('flash', flash)] + [('blast%d' % i, b) for i, b in enumerate(blasts)] + [('soot%d' % i, s) for i, s in enumerate(soot)]


def smoke():
    frames = []
    for i, s in enumerate((1.0, 0.72, 0.45)):
        a = Img(16, 16)
        for (x, y, r) in ((5.5, 9.5, 4.3), (10.5, 9.5, 4.3), (8, 6, 4.6), (8, 11, 4)):
            a.disc(8 + (x - 8) * s, 8 + (y - 8) * s, r * s, G)
        for (x, y, r) in ((6.5, 5.5, 2.4), (10, 6.5, 2.0), (5, 8.5, 1.6)):
            a.disc(8 + (x - 8) * s, 8 + (y - 8) * s, r * s, LG)
        a.disc(8 + (7 - 8) * s, 8 + (4.8 - 8) * s, max(0.8, 1.1 * s), W)
        a.outline(DG)
        frames.append(('smoke%d' % i, a))
    return frames


def fireworks():
    rocket = Img(8, 8)
    rocket.line(4, 0, 4, 3, W); rocket.set(4, 4, Y); rocket.set(4, 5, O); rocket.set(3, 6, O); rocket.set(5, 7, R)
    frames = [('rocket', rocket)]
    for col in (R, Y, C, GR, P, GD):
        a = Img(8, 8); a.line(3, 1, 3, 5, col); a.line(1, 3, 5, 3, col); a.set(3, 3, W)
        b = Img(8, 8); b.set(3, 3, col); b.set(4, 4, col); b.set(3, 4, W)
        frames += [('spark%d_0' % col, a), ('spark%d_1' % col, b)]
    return frames


def flower():
    frames = []
    for i, op in enumerate((0.35, 0.7, 1.0)):
        a = Img(32, 16)
        n = 6
        for k in range(n):
            ang = k * 2 * math.pi / n + 0.4
            px = 16 + math.cos(ang) * 9 * op
            py = 9 + math.sin(ang) * 4.5 * op
            a.ellipse(px, py, 4.2 * op + 1.2, 2.4 * op + 1.0, P)
        for k in range(n):
            ang = k * 2 * math.pi / n + 0.4
            a.ellipse(16 + math.cos(ang) * 6 * op, 9 + math.sin(ang) * 3 * op, 1.8 * op + 0.6, 1.1 * op + 0.5, W if op > 0.5 else P)
        a.ellipse(16, 9, 2.6 * op + 0.8, 1.6 * op + 0.6, Y)
        a.set(16, 9, GD)
        a.outline(R, inside_of=(P,))
        frames.append(('bloom%d' % i, a))
    p0 = Img(8, 8); p0.ellipse(4, 4, 2.6, 1.5, P); p0.set(3, 3, W)
    p1 = Img(8, 8); p1.ellipse(4, 4, 1.3, 2.4, P); p1.set(4, 3, W)
    return frames + [('petal0', p0), ('petal1', p1)]


def bubbles():
    big = Img(16, 16); big.ring(8, 8, 7, C, 1.2); big.set(5, 4, W); big.set(4, 5, W); big.set(5, 5, W); big.set(11, 11, W)
    small = Img(8, 8); small.ring(4, 4, 3.4, C, 1.1); small.set(3, 2, W)
    pop = Img(8, 8)
    for k in range(8):
        ang = k * math.pi / 4
        pop.set(4 + math.cos(ang) * 3.4, 4 + math.sin(ang) * 3.4, C if k % 2 else W)
    huge = Img(64, 64)
    huge.ring(32, 32, 30, C, 1.6)
    huge.ring(32, 32, 28.5, W, 0.7) if False else None
    for (x, y) in ((18, 14), (17, 16), (16, 18), (19, 13), (21, 12), (16, 20)):
        huge.disc(x, y, 1.6, W)
    huge.disc(45, 47, 1.4, W)
    return [('bubble16', big), ('bubble8', small), ('pop', pop), ('bubble64', huge)]


def lightning():
    bolt = Img(32, 64)
    pts = [(19, 0), (10, 22), (17, 23), (8, 44), (15, 45), (6, 64), (24, 36), (17, 35), (26, 18), (18, 17), (25, 0)]
    bolt.poly(pts, Y)
    inner = [(21, 2), (13, 21), (19.5, 22), (11, 43), (17, 44), (10, 59), (21, 38), (14.5, 37), (23, 19), (15.5, 18.5), (22.5, 2)]
    bolt.poly(inner, W)
    bolt.outline(O, inside_of=(Y,))
    s0 = Img(8, 8); s0.line(3, 0, 3, 6, Y); s0.line(0, 3, 6, 3, Y); s0.set(3, 3, W)
    s1 = Img(8, 8); s1.set(3, 3, Y); s1.set(2, 2, Y); s1.set(4, 4, Y); s1.set(3, 2, W)
    return [('bolt', bolt), ('zap0', s0), ('zap1', s1)]


def wisps():
    frames = []
    for i in range(3):
        a = Img(8, 16)
        sway = (-0.8, 0, 0.8)[i]
        a.ellipse(4, 11, 3.3, 3.6, V)
        a.poly([(1.2, 10), (4 + sway * 1.5, 1 + i), (6.8, 10)], V)
        a.ellipse(4, 11.5, 2.0, 2.4, B)
        a.ellipse(4, 12, 1.0, 1.4, W)
        frames.append(('wisp%d' % i, a))
    eyes = Img(16, 8)
    for cx in (4, 11):
        eyes.ellipse(cx, 4, 2.6, 2.2, Y)
        eyes.ellipse(cx, 4.2, 0.9, 1.5, K)
    blink = Img(16, 8)
    for cx in (4, 11):
        blink.line(cx - 2, 4, cx + 2, 4, Y)
    return frames + [('eyes', eyes), ('blink', blink)]


def crystal():
    case = Img(64, 64)
    outer = [(32, 1), (52, 14), (55, 46), (32, 62), (9, 46), (12, 14)]
    case.poly(outer, C)
    inner = [(32, 4), (50, 16), (52, 45), (32, 59), (12, 45), (14, 16)]
    case.poly(inner, 0)
    # facets: lines from the top and bottom points
    for (x0, y0, x1, y1) in ((32, 4, 26, 13), (32, 4, 38, 13), (14, 16, 20, 21), (50, 16, 44, 21),
                             (12, 45, 19, 41), (52, 45, 45, 41), (32, 59, 27, 52), (32, 59, 37, 52)):
        case.line(x0, y0, x1, y1, LG)
    for (x, y) in ((22, 9), (21, 10), (20, 11), (44, 50)):
        case.set(x, y, W)
    shards = []
    for i, ang in enumerate((0.0, 0.8, 1.6)):
        s = Img(8, 8)
        pts = [(4 + math.cos(ang + t) * r, 4 + math.sin(ang + t) * r)
               for t, r in ((0, 3.6), (math.pi / 2, 1.5), (math.pi, 3.6), (3 * math.pi / 2, 1.5))]
        s.poly(pts, C)
        s.set(4, 4, W)
        shards.append(('shard%d' % i, s))
    return [('case', case)] + shards


def spotlight():
    top = Img(32, 64)
    top.poly([(13, 0), (19, 0), (25, 64), (7, 64)], W)
    top.poly([(14.5, 0), (17.5, 0), (21, 64), (11, 64)], W)
    bottom = Img(64, 64)
    bottom.poly([(23, 0), (41, 0), (62, 58), (2, 58)], W)
    bottom.ellipse(32, 57, 30, 6, W)
    # a soft yellow edge
    bottom.outline(Y)
    top.outline(Y)
    return [('cone_top', top), ('cone', bottom)]


def dust():
    frames = []
    for i, s in enumerate((1.0, 0.7)):
        a = Img(16, 8)
        for (x, y, r) in ((5, 5, 2.8), (9, 4.5, 3.2), (12, 5.5, 2.4)):
            a.disc(8 + (x - 8) * s, 4 + (y - 4) * s, r * s + 0.3, BR)
        a.disc(8 + (8 - 8) * s, 3.5, 1.5 * s, LG)
        frames.append(('dust%d' % i, a))
    impact = Img(32, 16)
    impact.ellipse(16, 11, 14, 4, Y)
    impact.ellipse(16, 11, 9, 2.5, W)
    for k in range(5):
        x = 6 + k * 5
        impact.line(x, 9, 16 + (x - 16) * 1.3, 2, Y)
    return frames + [('impact', impact)]


def royal():
    frames = []
    for i, (L, core) in enumerate(((2, GD), (3.6, Y), (2, W))):
        s = Img(8, 8)
        s.line(3.5 - L, 3.5, 3.5 + L, 3.5, core); s.line(3.5, 3.5 - L, 3.5, 3.5 + L, core)
        s.set(3.5, 3.5, W)
        if i == 1:
            s.set(2.5, 2.5, GD); s.set(4.5, 4.5, GD); s.set(2.5, 4.5, GD); s.set(4.5, 2.5, GD)
        frames.append(('sparkle%d' % i, s))
    crown = Img(16, 16)
    crown.poly([(2, 13), (2, 5), (5, 9), (8, 3), (11, 9), (14, 5), (14, 13)], GD)
    crown.poly([(2, 11), (14, 11), (14, 13.5), (2, 13.5)], Y)
    for (x, y, c) in ((8, 2.6, W), (2, 4.6, W), (14, 4.6, W)):
        crown.disc(x, y, 1.3, c)
    crown.set(5, 12, R); crown.set(8, 12, B); crown.set(11, 12, R)
    crown.set(7, 6, Y); crown.set(8, 5, W)
    crown.outline(BR)
    return frames + [('crown', crown)]


def energy():
    mote = Img(8, 8); mote.disc(3.5, 3.5, 3.2, ED); mote.disc(3.5, 3.5, 2.5, E); mote.disc(3.5, 3.5, 1.6, EL); mote.disc(3.5, 3.5, 0.8, W)
    sq4 = Img(8, 8); sq4.poly([(2, 2), (6, 2), (6, 6), (2, 6)], E); sq4.set(2, 2, W)
    sq2 = Img(8, 8); sq2.poly([(3, 3), (5, 3), (5, 5), (3, 5)], E)
    streak = Img(8, 16); streak.ellipse(4, 8, 2.4, 7.8, ED); streak.ellipse(4, 8, 1.8, 7.2, E); streak.ellipse(4, 8, 1.1, 6, EL); streak.ellipse(4, 8, 0.5, 4.5, W)
    scan = Img(32, 8)
    for x in range(32):
        scan.set(x, 3, W); scan.set(x, 2, E); scan.set(x, 4, E)
        if x % 2 == 0:
            scan.set(x, 1, E)
    trail = Img(16, 32); trail.poly([(8, 0), (12.5, 20), (8, 31.5), (3.5, 20)], ED); trail.poly([(8, 2), (11.5, 20), (8, 31), (4.5, 20)], E)
    trail.poly([(8, 5), (10.4, 21), (8, 30), (5.6, 21)], EL); trail.poly([(8, 12), (9.2, 22), (8, 29), (6.8, 22)], W)
    splat = Img(32, 32)
    random.seed(3)
    splat.disc(16, 16, 9, E)
    for k in range(9):
        ang = k * 2 * math.pi / 9 + random.random() * 0.4
        d = 9 + random.random() * 4
        splat.disc(16 + math.cos(ang) * d, 16 + math.sin(ang) * d * 0.8, 2.2 + random.random() * 1.6, E)
        splat.line(16, 16, 16 + math.cos(ang) * d, 16 + math.sin(ang) * d * 0.8, E)
    for k in range(5):
        ang = random.random() * 2 * math.pi
        d = 14 + random.random() * 2
        splat.disc(16 + math.cos(ang) * d, 16 + math.sin(ang) * d * 0.8, 1.1, E)
    # a darker rim and a wet highlight
    src = [row[:] for row in splat.p]
    for y in range(32):
        for x in range(32):
            if src[y][x] == E and any(not (0 <= x + dx < 32 and 0 <= y + dy < 32) or src[y + dy][x + dx] == 0
                                      for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                splat.p[y][x] = ED
    splat.set(12, 12, EL); splat.set(13, 11, W); splat.set(12, 11, EL)
    drop = Img(8, 8); drop.ellipse(3.5, 4, 1.8, 2.4, E); drop.set(3, 3, W)
    ball = Img(8, 8)
    ball.disc(3.5, 3.5, 3.4, W)
    ball.poly([(0, 0), (8, 0), (8, 3), (0, 3)], 0)
    ball.disc(3.5, 3.5, 3.4, E, only_empty=True)
    ball.line(0, 3.5, 7, 3.5, K)
    ball.set(3.5, 3.5, W)
    ball.outline(K)
    return [('mote', mote), ('sq4', sq4), ('sq2', sq2), ('streak', streak), ('scan', scan), ('trail', trail),
            ('splat', splat), ('drop', drop), ('miniball', ball)]


def extras():
    frames = []
    for size in (16, 32):
        g = Img(size, size)
        c = size / 2
        # a dithered soft glow: dark rim -> energy -> light -> white core
        for y in range(size):
            for x in range(size):
                d = math.hypot(x + 0.5 - c, y + 0.5 - c) / c
                bayer = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]][y % 4][x % 4] / 16.0
                v = d + (bayer - 0.5) * 0.18
                if v < 0.18:
                    g.p[y][x] = W
                elif v < 0.42:
                    g.p[y][x] = EL
                elif v < 0.7:
                    g.p[y][x] = E
                elif v < 0.98:
                    g.p[y][x] = ED
        frames.append(('glow%d' % size, g))
    ring = Img(32, 32); ring.ring(16, 16, 15, E, 2.2); ring.ring(16, 16, 13.6, EL, 0.9)
    wring = Img(32, 32); wring.ring(16, 16, 15, W, 1.6); wring.ring(16, 16, 13.6, LG, 0.8)
    ember = Img(8, 8); ember.disc(3.5, 3.5, 2.2, O); ember.disc(3.5, 3.5, 1.1, Y); ember.set(3, 3, W)
    soft = Img(16, 16)
    for y in range(16):
        for x in range(16):
            d = math.hypot(x + 0.5 - 8, y + 0.5 - 8) / 8
            bayer = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]][y % 4][x % 4] / 16.0
            if d + (bayer - 0.5) * 0.3 < 0.9:
                soft.p[y][x] = W if d < 0.35 else LG
    return frames + [('ring', ring), ('wring', wring), ('ember', ember), ('soft', soft)]


SETS = [
    ('CONFETTI', confetti), ('EXPLOSION', explosion), ('SMOKE', smoke), ('FIREWORKS', fireworks),
    ('FLOWER', flower), ('BUBBLES', bubbles), ('LIGHTNING', lightning), ('WISPS', wisps),
    ('CRYSTAL', crystal), ('SPOTLIGHT', spotlight), ('DUST', dust), ('ROYAL', royal), ('ENERGY', energy),
    ('EXTRA', extras),
]

def main():
    preview = None
    if '--preview' in sys.argv:
        preview = sys.argv[sys.argv.index('--preview') + 1]
    lines = ['// Generated by tools/moemon_ballfx/gen.py (Moemon Co-op ball effects). Do not edit.',
             '// Indexed pixels, drawn by the effects layer over the finished frame (mp_ballfx.c):',
             '// 0 transparent, 1 white, 2-5 grays, 6 yellow, 7 orange, 8 red, 9 pink, 10 purple, 11 blue,',
             '// 12 cyan, 13 green, 14 brown, 15 gold; 16/17/18 = the energy color / its light / dark shade.', '']
    lines.append('static const u16 sFxPalette[16] = {')
    lines.append('    ' + ', '.join('RGB(%d, %d, %d)' % c for c in PAL) + ',')
    lines.append('};')
    lines.append('')
    enums, table, pixels, all_frames = [], [], [], []
    for set_name, fn in SETS:
        for name, img in fn():
            enum = 'FXA_%s_%s' % (set_name, name.upper())
            enums.append(enum)
            table.append('    [%s] = {%d, %d, %d},' % (enum, img.w, img.h, len(pixels)))
            for row in img.p:
                pixels += row
            all_frames.append((set_name, name, img))
    lines.append('enum {\n' + ''.join('    %s,\n' % e for e in enums) + '    FXA_COUNT,\n};\n')
    lines.append('static const u8 sFxPixels[] = {')
    for k in range(0, len(pixels), 32):
        lines.append('    ' + ','.join(str(v) for v in pixels[k:k + 32]) + ',')
    lines.append('};')
    lines.append('')
    lines.append('struct FxArt { u8 w, h; u32 offset; };')
    lines.append('static const struct FxArt sFxArt[FXA_COUNT] = {')
    lines += table
    lines.append('};')
    with open(OUT, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    print('wrote %s (%d frames, %d bytes of pixels)' % (OUT, len(enums), len(pixels)))

    if preview:
        from PIL import Image
        scale = 4
        W_ = 900
        x = y = 4
        rowh = 0
        placed = []
        for set_name, name, img in all_frames:
            if x + img.w + 4 > W_ // scale:
                x = 4
                y += rowh + 6
                rowh = 0
            placed.append((x, y, img))
            x += img.w + 6
            rowh = max(rowh, img.h)
        H_ = (y + rowh + 6) * scale
        out = Image.new('RGB', (W_, H_), (40, 70, 60))
        px = out.load()
        demo = {E: (31, 6, 5), EL: (31, 19, 18), ED: (16, 3, 2)}   # energy shown in red
        for (ox, oy, img) in placed:
            for yy in range(img.h):
                for xx in range(img.w):
                    v = img.p[yy][xx]
                    if v == 0:
                        continue
                    c = demo[v] if v in demo else PAL[v]
                    rgb = tuple(int(ch * 255 / 31) for ch in c)
                    for sy in range(scale):
                        for sx in range(scale):
                            px[(ox + xx) * scale + sx, (oy + yy) * scale + sy] = rgb
        out.save(preview)
        print('preview', preview)


if __name__ == '__main__':
    main()
