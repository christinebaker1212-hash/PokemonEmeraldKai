"""The Gen 3 trainer kit ("pokemon-gen-3" style for Top Down Sprite Maker).

Layers are drawn with marker colors (skin, hair, eye and 4 clothing ramps) that
the kit's manifest.tds recolors by hue/saturation/value. This module cuts every
layer into Emerald's 27 overworld frames (walk, run, Mach Bike; 9 each in
Emerald's order) at 32x32, following the manifest's head placement.
"""
import colorsys
import os

import numpy as np
from PIL import Image


def hx(s):
    s = s.lstrip('#')
    return tuple(int(s[i:i + 2], 16) for i in (0, 2, 4))


BASE_SKIN = hx('b8f8b8')
SKIN = {hx('b8f8b8'): 0, hx('98e898'): 1, hx('70d870'): 2, hx('557840'): 3, hx('364030'): 3}   # -> skin shade
BASE_HAIR = hx('b0b0f8')
HAIR = {hx('b0b0f8'), hx('8080f0'), hx('4848c8'), hx('303070')}
BASE_IRIS = hx('ff0000')
IRIS = {hx('ff0000'), hx('884848')}
BASE_CLOTH = [hx('f08080'), hx('f0b880'), hx('f0f080'), hx('b8f080')]
CLOTH = [{BASE_CLOTH[0], hx('f8b0b0'), hx('c84848'), hx('703030')},
         {BASE_CLOTH[1], hx('f8d4b0'), hx('c88848'), hx('705030')},
         {BASE_CLOTH[2], hx('f8f8b0'), hx('c8c848'), hx('707030')},
         {BASE_CLOTH[3], hx('d4f8b0'), hx('88c848'), hx('507030')}]

# Recolor groups (shared with mp_creator.c)
G_HAIR, G_ACC0, G_ACC1, G_C0, G_C1, G_C2, G_C3, G_IRIS = range(8)

DIRS = ['S', 'W', 'E', 'N']
ANIM_COL = {'walk': 0, 'run': 3, 'cycle': 6}
VIEWS = (('walk', 'walk'), ('run', 'run'), ('bike', 'cycle'))
# Emerald's 9 frames: stand S, N, W, then the steps S, S, N, N, W, W (kit frame 1 = standing)
EM = [('S', 1), ('N', 1), ('W', 1), ('S', 0), ('S', 2), ('N', 0), ('N', 2), ('W', 0), ('W', 2)]
FRAMES = [(view, anim, d, f) for view, anim in VIEWS for d, f in EM]   # 27
WIN_X, WIN_Y = 4, 4   # 32x32 window in the kit's 40x40 frames (feet on row 31, like Emerald)


def hsv(c):
    return colorsys.rgb_to_hsv(*(v / 255 for v in c))


def head_offset(anim, d, f):
    """manifest.tds head_offset (walk / run / cycle only)."""
    x = 0
    if anim == 'cycle':
        x = f - 1 if f % 2 == 0 else 0
    if anim == 'walk':
        ac = 2 if d == 'S' else 4 if d == 'N' else 3
    elif anim == 'run':
        ac = 2 + (0 if d == 'S' else 1)
    else:
        ac = 1 + (1 if d == 'N' else 0)
    fc = (1 if f != 1 else 0) if anim in ('walk', 'run') else 0
    return x, ac + fc


def head_col(anim, d):
    if d == 'S' and anim == 'run':
        return 4
    return DIRS.index(d)


class Kit3:
    def __init__(self, root):
        self.root = root
        self.hairs = self.csv('hair')
        self.hats = self.csv('hat')
        self.extra = {}   # (layer, name) -> sheet: hairstyles from elsewhere (donor_hair.py)

    def add_hair(self, name, nacc, sheet):
        self.hairs.append((name, nacc))
        self.extra[('hair', name)] = sheet

    def csv(self, layer):
        out = []
        for item in open(os.path.join(self.root, 'bases', layer, layer + '.csv')).read().strip().split(','):
            name, _, n = item.partition(':')
            out.append((name, int(n) if n else 0))
        return out

    def img(self, layer, name):
        if (layer, name) in self.extra:
            return self.extra[(layer, name)]
        p = os.path.join(self.root, 'bases', layer, name + '.png')
        if not os.path.exists(p):
            return None
        a = np.array(Image.open(p).convert('RGBA'))
        return a if a[..., 3].any() else None

    def body_frames(self, layer, name):
        """A body-sheet layer (40x40 cells) -> 27 RGBA frames of 32x32, or None."""
        a = self.img(layer, name)
        if a is None:
            return None
        out = []
        for view, anim, d, f in FRAMES:
            r, c = DIRS.index(d), ANIM_COL[anim] + f
            cell = a[r * 40:(r + 1) * 40, c * 40:(c + 1) * 40]
            out.append(np.ascontiguousarray(cell[WIN_Y:WIN_Y + 32, WIN_X:WIN_X + 32]))
        return out

    def head_frames(self, layer, name, eyes=False):
        """A 100x20 head sheet (5 cells: S, W, E, N, S tilted) placed on the head in each frame.
        A taller sheet has the same number of spare rows above and below the 20x20 cells
        (donor_hair.py), for hair that reaches past the head cell."""
        a = self.img(layer, name)
        if a is None:
            return None
        if eyes:   # 4 cells; the tilted one copies S
            b = np.zeros((20, 100, 4), np.uint8)
            b[:, :80] = a[:, :80]
            b[:, 80:] = a[:, :20]
            a = b
        out = []
        margin = (a.shape[0] - 20) // 2
        for view, anim, d, f in FRAMES:
            cell = a[:, head_col(anim, d) * 20:(head_col(anim, d) + 1) * 20]
            hx_, hy = head_offset(anim, d, f)
            dst = np.zeros((32, 32, 4), np.uint8)
            paste(dst, cell, 10 + hx_ - WIN_X, 10 + hy - WIN_Y - margin)
            out.append(dst)
        return out


def paste(dst, src, x, y):
    h, w = src.shape[:2]
    for yy in range(h):
        Y = y + yy
        if not 0 <= Y < dst.shape[0]:
            continue
        for xx in range(w):
            X = x + xx
            if 0 <= X < dst.shape[1] and src[yy, xx, 3]:
                dst[Y, X] = src[yy, xx]


def ratio(inp, base):
    _, s, v = hsv(inp)
    _, bs, bv = hsv(base)
    rs = s / bs if bs > 0 else 0.0
    rv = v / bv if bv > 0 else 0.0
    return min(255, int(round(rs * 64))), min(255, int(round(rv * 64)))


def marker_role(c, kind, nacc=0):
    """Role of a kit marker color: ('skin', shade) / ('role', group, rs, rv) / ('lit', rgb)."""
    if c in SKIN:
        return ('skin', SKIN[c])
    if c in HAIR:
        return ('role', G_HAIR) + ratio(c, BASE_HAIR)
    if c in IRIS:
        return ('role', G_IRIS) + ratio(c, BASE_IRIS)
    for k in range(4):
        if c in CLOTH[k]:
            if kind == 'hair':
                g = G_ACC0 + k if k < nacc else G_HAIR
                if g == G_HAIR:
                    return ('role', G_HAIR) + ratio(c, BASE_CLOTH[k])
                return ('role', g) + ratio(c, BASE_CLOTH[k])
            return ('role', G_C0 + k) + ratio(c, BASE_CLOTH[k])
    return ('lit', c)
