#!/usr/bin/env python3
"""Moemon Kai: draws the e-Reader Berries' art (Bag icons 24x24, Berry tag pictures 48x48).

Each Berry is a few shaded shapes (lit from the upper left, with a dark outline)
described in unit coordinates, so one description gives both sizes.

usage: python3 tools/moemon_berries/draw_ereader_berries.py [--preview out.png]
writes graphics/items/icons/<name>_berry.png, graphics/items/icon_palettes/<name>_berry.pal
and graphics/berries/<name>.png
"""
import math
import os
import sys

from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
LIGHT = (-0.45, -0.6, 0.66)
_l = math.sqrt(sum(c * c for c in LIGHT))
LIGHT = tuple(c / _l for c in LIGHT)


def rgb(h):
    return ((h >> 16) & 255, (h >> 8) & 255, h & 255)


def snap(c):
    # GBA colors are 5 bits per channel
    return tuple((v >> 3) << 3 for v in c)


# ---------------------------------------------------------------------------
# Shapes: each returns (inside, normal) for a point in unit coordinates

class Ellipse:
    def __init__(self, cx, cy, rx, ry, rot=0.0):
        self.cx, self.cy, self.rx, self.ry = cx, cy, rx, ry
        self.c, self.s = math.cos(math.radians(rot)), math.sin(math.radians(rot))

    def local(self, x, y):
        dx, dy = x - self.cx, y - self.cy
        return (dx * self.c + dy * self.s) / self.rx, (-dx * self.s + dy * self.c) / self.ry

    def sample(self, x, y):
        u, v = self.local(x, y)
        d = u * u + v * v
        if d > 1:
            return None
        z = math.sqrt(1 - d)
        nx, ny = u * self.c - v * self.s, u * self.s + v * self.c
        return (nx, ny, z), (u, v)


class Tube:
    """A thick curve through points, with a radius at each point."""

    def __init__(self, pts, radii):
        self.pts, self.radii = pts, radii

    def sample(self, x, y):
        best = None
        for i in range(len(self.pts) - 1):
            (ax, ay), (bx, by) = self.pts[i], self.pts[i + 1]
            vx, vy = bx - ax, by - ay
            l2 = vx * vx + vy * vy
            t = max(0.0, min(1.0, ((x - ax) * vx + (y - ay) * vy) / l2)) if l2 else 0.0
            px, py = ax + vx * t, ay + vy * t
            r = self.radii[i] + (self.radii[i + 1] - self.radii[i]) * t
            d = math.hypot(x - px, y - py)
            if d <= r and (best is None or d / r < best[0]):
                along = (i + t) / (len(self.pts) - 1)
                best = (d / r, x - px, y - py, r, along)
        if best is None:
            return None
        q, ox, oy, r, along = best
        n = math.hypot(ox, oy) or 1
        z = math.sqrt(max(0.0, 1 - q * q))
        return (ox / n * q, oy / n * q, z), (q, along)


class Union:
    """Several shapes as one (no outline between them): the one whose surface is highest wins."""

    def __init__(self, *shapes):
        self.shapes = shapes

    def sample(self, x, y):
        best = None
        for sh in self.shapes:
            r = sh.sample(x, y)
            if r is not None and (best is None or r[0][2] > best[0][2]):
                best = r
        return best


class Poly:
    """A flat shape (leaves): a little shading from its middle."""

    def __init__(self, pts):
        self.pts = pts
        self.cx = sum(p[0] for p in pts) / len(pts)
        self.cy = sum(p[1] for p in pts) / len(pts)
        self.r = max(math.hypot(p[0] - self.cx, p[1] - self.cy) for p in pts)

    def sample(self, x, y):
        inside = False
        n = len(self.pts)
        j = n - 1
        for i in range(n):
            xi, yi = self.pts[i]
            xj, yj = self.pts[j]
            if (yi > y) != (yj > y) and x < (xj - xi) * (y - yi) / (yj - yi) + xi:
                inside = not inside
            j = i
        if not inside:
            return None
        u, v = (x - self.cx) / self.r, (y - self.cy) / self.r
        return (u * 0.5, v * 0.5, 0.85), (u, v)


# ---------------------------------------------------------------------------
# A part: a shape, its colors (light to dark, last is the outline) and an
# optional pattern that darkens / lightens pixels

class Part:
    def __init__(self, shape, colors, pattern=None, outline=True):
        self.shape = shape
        self.colors = [snap(rgb(c)) for c in colors]
        self.pattern = pattern
        self.outline = outline


def tone(normal, n_tones):
    i = sum(a * b for a, b in zip(normal, LIGHT))
    # highlight, light, mid, shadow (n_tones - 1 is the outline)
    cuts = [0.93, 0.62, 0.22]
    for k, c in enumerate(cuts[:n_tones - 2]):
        if i > c:
            return k
    return n_tones - 2


def render(parts, size, scale=1.0):
    """Returns rows of (color or None, part index)."""
    img = [[None] * size for _ in range(size)]
    owner = [[-1] * size for _ in range(size)]
    for pi, part in enumerate(parts):
        for py in range(size):
            for px in range(size):
                x = (px + 0.5) / size
                y = (py + 0.5) / size
                r = part.shape.sample(x, y)
                if r is None:
                    continue
                normal, local = r
                k = tone(normal, len(part.colors))
                if part.pattern:
                    k = part.pattern(k, local, x, y, size, len(part.colors))
                k = max(0, min(len(part.colors) - 2, k))
                img[py][px] = part.colors[k]
                owner[py][px] = pi
    # Outlines: the edge of the whole picture, and where a part sits on another
    out = [row[:] for row in img]
    for py in range(size):
        for px in range(size):
            pi = owner[py][px]
            if pi < 0 or not parts[pi].outline:
                continue
            # the edge of the picture, or where it sits on a part drawn before it
            edge = False
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                qx, qy = px + dx, py + dy
                if not (0 <= qx < size and 0 <= qy < size) or owner[qy][qx] < pi:
                    edge = True
            if edge:
                out[py][px] = parts[pi].colors[-1]
    return out


# ---------------------------------------------------------------------------
# Patterns

def ribs(count, depth=1):
    def f(k, local, x, y, size, n):
        u, v = local
        w = math.cos(u * math.pi * count / 2)
        if w < -0.75:
            return k + depth
        return k
    return f


def dots(cols, rows, color_shift, phase=0.0, rad=0.13):
    def f(k, local, x, y, size, n):
        u, v = local
        gu = (u + 1) * cols / 2 + (phase if int((v + 1) * rows / 2) % 2 else 0)
        gv = (v + 1) * rows / 2
        du, dv = gu - math.floor(gu) - 0.5, gv - math.floor(gv) - 0.5
        if du * du + dv * dv < rad and size >= 24:
            return color_shift
        return k
    return f


def stripes_v(count, extra_index):
    def f(k, local, x, y, size, n):
        u, v = local
        w = math.sin(u * math.pi * count / 2)
        if abs(w) < 0.22 and k <= n - 3:
            return extra_index
        return k
    return f


def bumps(freq, amp):
    def f(k, local, x, y, size, n):
        q, along = local if len(local) == 2 else (0, 0)
        s = math.sin(x * freq * 6.28 + 1.3) * math.sin(y * freq * 6.28)
        if s > amp:
            return k - 1
        if s < -amp:
            return k + 1
        return k
    return f


def chain(*fs):
    def f(k, local, x, y, size, n):
        for g in fs:
            k = g(k, local, x, y, size, n)
        return k
    return f


# ---------------------------------------------------------------------------
# The Berries. Colors: highlight, light, mid, shadow, outline

LEAF = [0xA8F070, 0x70D048, 0x40A030, 0x287020, 0x183818]
STEM = [0xC8A070, 0xA07848, 0x785030, 0x503018, 0x302010]


def pumkin():
    body = [0xFFE098, 0xF8B040, 0xE08020, 0xB05010, 0x582008]
    return [
        Part(Ellipse(0.5, 0.58, 0.40, 0.32), body, ribs(5)),
        Part(Tube([(0.5, 0.30), (0.53, 0.18), (0.6, 0.12)], [0.055, 0.05, 0.045]), STEM),
        Part(Poly([(0.53, 0.26), (0.70, 0.17), (0.82, 0.22), (0.66, 0.30)]), LEAF),
    ]


def drash():
    body = [0xFFFFFF, 0xF0F0F8, 0xD0D0E0, 0xA0A0B8, 0x404058]
    top = [0xFFD0E0, 0xF8A0C0, 0xE07098, 0xB04870, 0x501830]
    return [
        Part(Tube([(0.5, 0.36), (0.5, 0.62), (0.47, 0.82), (0.42, 0.93)], [0.21, 0.17, 0.09, 0.03]), body),
        Part(Ellipse(0.5, 0.38, 0.215, 0.11), top),
        Part(Poly([(0.48, 0.32), (0.30, 0.08), (0.40, 0.06), (0.52, 0.28)]), LEAF),
        Part(Poly([(0.50, 0.31), (0.62, 0.04), (0.72, 0.10), (0.56, 0.32)]), LEAF),
        Part(Poly([(0.52, 0.33), (0.78, 0.18), (0.82, 0.26), (0.58, 0.36)]), LEAF),
    ]


def eggant():
    body = [0xFFFFF8, 0xF8F0D8, 0xE0D0B0, 0xB8A080, 0x504030]
    lilac = 0xB898D8
    shell = [0xFFFFF8, 0xF8F0D8, 0xE0D0B0, 0xB8A080, 0xC0A0E0, 0x504030]

    def spots(k, local, x, y, size, n):
        u, v = local
        if size >= 24 and (math.sin(u * 7.5 + 0.6) * math.sin(v * 6.0 + 1.1) > 0.72):
            return 4
        return k
    return [
        Part(Ellipse(0.5, 0.58, 0.30, 0.37), shell, spots),
        Part(Poly([(0.36, 0.26), (0.5, 0.17), (0.64, 0.26), (0.58, 0.33), (0.5, 0.28), (0.42, 0.33)]), LEAF),
        Part(Tube([(0.5, 0.2), (0.53, 0.09)], [0.035, 0.03]), STEM),
    ]


def strib():
    body = [0xFFB0A0, 0xF85048, 0xD82828, 0xA01018, 0x480808]

    def seeds(k, local, x, y, size, n):
        u, v = local
        if size < 24:
            return k
        gu, gv = (u + 1) * 3.2 + (0.5 if int((v + 1) * 3.4) % 2 else 0), (v + 1) * 3.4
        du, dv = gu - math.floor(gu) - 0.5, gv - math.floor(gv) - 0.5
        if du * du * 2 + dv * dv < 0.06 and v > -0.6:
            return 0 if k >= 2 else 3
        return k
    return [
        Part(Union(Ellipse(0.5, 0.52, 0.33, 0.24), Tube([(0.5, 0.56), (0.5, 0.74), (0.5, 0.9)], [0.27, 0.17, 0.04])),
             body, seeds),
        Part(Poly([(0.24, 0.34), (0.38, 0.22), (0.5, 0.27), (0.62, 0.21), (0.77, 0.33), (0.6, 0.36),
                   (0.5, 0.42), (0.4, 0.36)]), LEAF),
        Part(Tube([(0.5, 0.26), (0.52, 0.12)], [0.03, 0.025]), STEM),
    ]


def chilan():
    body = [0xF8FFC0, 0xE0F070, 0xB8D040, 0x80A028, 0x304010]
    return [
        Part(Tube([(0.30, 0.22), (0.42, 0.42), (0.56, 0.62), (0.70, 0.80), (0.80, 0.90)],
                  [0.13, 0.15, 0.14, 0.10, 0.04]), body, ribs(3)),
        Part(Poly([(0.18, 0.20), (0.28, 0.12), (0.40, 0.18), (0.34, 0.27), (0.24, 0.27)]), LEAF),
        Part(Tube([(0.29, 0.15), (0.22, 0.06)], [0.03, 0.025]), STEM),
    ]


def nutpea():
    shell = [0xF8E8C0, 0xE0C890, 0xC8A060, 0x987040, 0x483018]

    def net(k, local, x, y, size, n):
        if size < 24:
            return k
        a = math.sin((x + y) * 38) * math.sin((x - y) * 38)
        if a > 0.8:
            return k + 1
        return k
    return [
        Part(Tube([(0.48, 0.24), (0.49, 0.36), (0.5, 0.5), (0.51, 0.64), (0.52, 0.76)],
                  [0.17, 0.19, 0.13, 0.20, 0.18]), shell, net),
    ]


def ginema():
    body = [0xFFFFE0, 0xF8F0A0, 0xE0D060, 0xB0A030, 0x504010]
    return [
        Part(Ellipse(0.40, 0.56, 0.22, 0.27, -12), body, ribs(1, 1)),
        Part(Ellipse(0.63, 0.50, 0.20, 0.25, 15), body, ribs(1, 1)),
        Part(Tube([(0.52, 0.30), (0.56, 0.12)], [0.03, 0.025]), STEM),
    ]


def kuo():
    body = [0xC8F8A0, 0x80D858, 0x50B038, 0x307820, 0x183010]
    cap = [0xB8E080, 0x88C050, 0x60A030, 0x407020, 0x203010]
    return [
        Part(Tube([(0.40, 0.24), (0.47, 0.45), (0.56, 0.66), (0.66, 0.86)], [0.14, 0.14, 0.10, 0.025]),
             body, ribs(4)),
        Part(Ellipse(0.39, 0.22, 0.13, 0.07, 18), cap),
        Part(Tube([(0.37, 0.18), (0.32, 0.07)], [0.035, 0.03]), STEM),
    ]


def yago():
    body = [0xD8FFB0, 0x98E070, 0x60B848, 0x387830, 0x183018]

    def warts(k, local, x, y, size, n):
        if size < 24:
            return k
        s = math.sin(x * 70 + 0.5) * math.sin(y * 52 + 1.0)
        if s > 0.7:
            return max(0, k - 1)
        if s < -0.85:
            return k + 1
        return k
    return [
        Part(Tube([(0.34, 0.16), (0.44, 0.36), (0.52, 0.58), (0.58, 0.78), (0.6, 0.9)],
                  [0.07, 0.17, 0.19, 0.14, 0.04]), body, warts),
        Part(Tube([(0.33, 0.13), (0.28, 0.05)], [0.03, 0.025]), STEM),
    ]


def touga():
    body = [0xFFC0A0, 0xF85030, 0xD02818, 0x981010, 0x400808]
    return [
        Part(Tube([(0.30, 0.26), (0.40, 0.42), (0.52, 0.58), (0.66, 0.74), (0.80, 0.86)],
                  [0.10, 0.12, 0.11, 0.08, 0.025]), body),
        Part(Poly([(0.19, 0.26), (0.30, 0.17), (0.42, 0.24), (0.36, 0.32), (0.25, 0.32)]), LEAF),
        Part(Tube([(0.30, 0.19), (0.27, 0.07), (0.33, 0.03)], [0.03, 0.028, 0.025]), STEM),
    ]


def niniku():
    shell = [0xFFFFFF, 0xF0ECF0, 0xD8C8D8, 0xA890B0, 0xA070B8, 0x483848]

    def cloves(k, local, x, y, size, n):
        u, v = local
        w = math.cos(u * math.pi * 2.5)
        if w < -0.8 and k < 3:
            return 3
        if abs(w) < 0.12 and v > 0.0 and size >= 24:
            return 4
        return k
    return [
        Part(Ellipse(0.5, 0.60, 0.36, 0.30), shell, cloves),
        Part(Tube([(0.5, 0.34), (0.5, 0.22), (0.52, 0.10)], [0.09, 0.05, 0.03]), shell),
        Part(Tube([(0.42, 0.88), (0.38, 0.95)], [0.02, 0.015]), STEM, outline=False),
        Part(Tube([(0.5, 0.89), (0.5, 0.96)], [0.02, 0.015]), STEM, outline=False),
        Part(Tube([(0.58, 0.88), (0.62, 0.95)], [0.02, 0.015]), STEM, outline=False),
    ]


def topo():
    body = [0xF8E0B0, 0xE0B878, 0xC09050, 0x906838, 0x403018]

    def eyes(k, local, x, y, size, n):
        if size < 24:
            return k
        for ex, ey in ((0.38, 0.45), (0.62, 0.62), (0.46, 0.72), (0.66, 0.40)):
            if (x - ex) ** 2 + (y - ey) ** 2 < 0.0012:
                return 3
        return k
    return [
        Part(Ellipse(0.5, 0.57, 0.38, 0.29, -20), body, eyes),
        Part(Poly([(0.50, 0.30), (0.62, 0.16), (0.72, 0.20), (0.58, 0.32)]), LEAF),
    ]


BERRIES = [
    ('pumkin', pumkin), ('drash', drash), ('eggant', eggant), ('strib', strib), ('chilan', chilan),
    ('nutpea', nutpea), ('ginema', ginema), ('kuo', kuo), ('yago', yago), ('touga', touga),
    ('niniku', niniku), ('topo', topo),
]


def to_indexed(rows, size):
    colors = []
    for row in rows:
        for c in row:
            if c is not None and c not in colors:
                colors.append(c)
    if len(colors) > 15:
        raise SystemExit('too many colors: %d' % len(colors))
    pal = [(0x70, 0xC0, 0xA0)] + colors
    pal += [(0, 0, 0)] * (16 - len(pal))
    im = Image.new('P', (size, size), 0)
    flat = []
    for p in pal:
        flat += list(p)
    im.putpalette(flat + [0] * (768 - len(flat)))
    px = im.load()
    for y in range(size):
        for x in range(size):
            c = rows[y][x]
            px[x, y] = 0 if c is None else 1 + colors.index(c)
    return im, pal


def write_pal(path, pal):
    with open(path, 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n16\n')
        for c in pal:
            f.write('%d %d %d\n' % c)


def main():
    preview = sys.argv[sys.argv.index('--preview') + 1] if '--preview' in sys.argv else None
    sheet = Image.new('RGBA', (48 * len(BERRIES), 48 + 24), (240, 240, 232, 255))
    for i, (name, fn) in enumerate(BERRIES):
        big, bpal = to_indexed(render(fn(), 48), 48)
        big.save(os.path.join(ROOT, 'graphics', 'berries', name + '.png'), transparency=0)
        small, spal = to_indexed(render(fn(), 24), 24)
        small.save(os.path.join(ROOT, 'graphics', 'items', 'icons', name + '_berry.png'), transparency=0)
        write_pal(os.path.join(ROOT, 'graphics', 'items', 'icon_palettes', name + '_berry.pal'), spal)
        sheet.alpha_composite(big.convert('RGBA'), (48 * i, 0))
        sheet.alpha_composite(small.convert('RGBA'), (48 * i + 12, 48))
    if preview:
        sheet.resize((sheet.width * 4, sheet.height * 4), Image.NEAREST).save(preview)


if __name__ == '__main__':
    main()
