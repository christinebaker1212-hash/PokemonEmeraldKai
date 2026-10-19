#!/usr/bin/env python3
"""Moemon Co-op trainer creator: the Gen 3 field sprites (walk, run, Mach Bike).

Usage: build_ow.py KIT3_DIR FRLG_DIR [--preview OUT.png]
  KIT3_DIR  the Gen 3 trainer kit (Top Down Sprite Maker "pokemon-gen-3": bases/, manifest.tds)
  FRLG_DIR  a pokefirered checkout (official overworld sprites to borrow clothes and bags from)
Reads the CREATE parts made by build.py (graphics/moemon/creator/parts.bin, src/mp/mp_creator_data.h)
for the colors of every part and the head accessories the Gen 3 kit doesn't have.
Writes graphics/moemon/creator/ow3.bin and src/mp/mp_creator_ow.h.

Where each piece of the field sprite comes from:
  body, head, eyes, hair (30 styles), T-shirt, pants, shoes, 5 hats   the Gen 3 kit
  skirts, shorts, long coat, overalls, swimwear, backpacks            official FRLG sprites
  other head accessories (crown, glasses, cat ears, fedora...)       the CREATE parts (front/back art)
Skin tones are exact Gen 3 colors (overworld ramp and trainer pictures).

Every piece is baked into Emerald's 27 frames (32x32) with a color role per palette entry:
  skin shade (exact table), recolor group (hue/sat/value like the kit's manifest) or a fixed color.
mp_creator.c stacks them for a recipe (the same steps as render() below).
"""
import colorsys
import os
import re
import sys
from collections import Counter

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
from kit3 import (Kit3, FRAMES, hx, hsv, marker_role, paste,  # noqa: E402
                  G_HAIR, G_ACC0, G_ACC1, G_C0, G_C1, G_C2, G_C3, G_IRIS, BASE_HAIR, HAIR)

CATS = ('hair', 'tops', 'bottoms', 'hats', 'bags')
OLD_VIEWS = ('front', 'back', 'walk', 'run', 'bike')

# Exact Gen 3 skin ramps: highlight, base, shade, outline (CREATE's PALE, LIGHT, MEDIUM, DARK)
SKINS = [
    ['ffeed5', 'deb4a4', 'bd9473', '7b5a52'],   # Wally's trainer picture
    ['ffd5b4', 'ffc594', 'de9473', '7b4141'],   # the overworld ramp (Brendan, May, every NPC)
    ['eeb494', 'c58b6a', '735241', '4a3941'],   # Phoebe's trainer picture
    ['bd9473', '8b6a52', '735241', '392939'],   # Cycling Triathlete / Swimmer trainer pictures
]
EYE_COLOR = '7b4141'

# CREATE hair (boy: spiky, short, medium, slicked back; girl: long, short, medium, ponytail) -> Gen 3 kit hair
HAIR_AUTO = {'m': ['anime', 'cowlick', 'sweep', 'magnate'], 'f': ['psychic', 'tomboy', 'waves', 'ponytail']}
# CREATE hats with a Gen 3 kit equivalent
HAT_KIT = {'beanie': 'knit-cap', 'youngster hat': 'yakyu-cap', 'beret': 'brimmed-beret',
           'straw hat': 'hiking-hat', 'sun hat': 'adventure', 'fedora': 'top-hat', 'feather hat': 'adventure',
           'bonnet': 'bandana', 'mining': 'bike-helmet'}
# Borrowed head accessories (small enough for a Gen 3 head): anchor ('top' of the head or 'face'),
# the Gen 3 kit hat whose mask hides the hair, and a nudge up/down
HAT_BORROW = {'crown': ('top', None, 0), 'flower': ('top', None, 0), 'ribbon': ('top', None, 0),
              'hair band': ('top', None, 0), 'headband': ('top', None, -3), 'glasses': ('face', None, -2),
              'cat ears': ('top', None, 0), 'devil': ('top', None, 0), 'poop': ('top', None, 0)}

# Official FRLG sprites: band ('legs', 'torso', 'both' or 'bag'), the palette ramp that takes the part's color,
# the row where the legs start when standing (S, N, W; bags: the first row of the bag), and palette entries to
# leave out (the NPC's own hair over the body)
DONORS = {
    'skirt':        dict(src='lass', band='legs', ramp=[9, 10], waist=(27, 27, 26), drop=[5, 6, 7]),
    'ribbon skirt': dict(src='woman_1', band='legs', ramp=[11, 12, 13], waist=26, drop=[5, 6, 7]),
    'long skirt':   dict(src='woman_2', band='legs', ramp=[8, 9, 10], waist=26, drop=[5, 6, 7]),
    'shorts m':     dict(src='youngster', band='legs', ramp=[7, 13], waist=27, drop=[5, 6]),
    'shorts f':     dict(src='crush_girl', band='legs', ramp=[8, 9, 10], waist=26, drop=[5, 6, 7]),
    'coat':         dict(src='gentleman', band='both', ramp=[5, 6, 7], waist=26, drop=[]),
    'overall top':  dict(src='worker_m', band='torso', ramp=[5, 6, 7], waist=26, drop=[]),
    'overall legs': dict(src='worker_m', band='legs', ramp=[5, 6, 7], waist=27, drop=[]),
    'swim top m':   dict(src='swimmer_m_land', band='torso', ramp=[], waist=26, drop=[5, 6, 7, 11, 12, 13, 14]),
    'swim legs m':  dict(src='swimmer_m_land', band='legs', ramp=[11, 12, 13], waist=26, drop=[5, 6, 7]),
    'swim top f':   dict(src='swimmer_f_land', band='torso', ramp=[8, 9, 10], waist=26, drop=[11, 12, 13]),
    'swim legs f':  dict(src='swimmer_f_land', band='legs', ramp=[8, 9, 10], waist=26, drop=[11, 12, 13]),
    'bag':          dict(src='green_normal', band='bag', ramp=[13, 14], waist=24, drop=[]),     # Leaf's bag
    'rucksack':     dict(src='red_normal', band='bag', ramp=[13, 14], waist=23, drop=[]),       # Red's backpack
    'sporty':       dict(src='hiker', band='bag', ramp=[8, 9, 10], waist=21, drop=[]),          # Hiker's pack
}
TOP_MAP = {'trench coat': 'coat', 'jumpsuit top': 'overall top', 'alt jumpsuit top': 'overall top',
           'beach top': ('swim top m', 'swim top f')}
BOTTOM_MAP = {'shorts': ('shorts m', 'shorts f'), 'long skirt': 'long skirt', 'pencil skirt': 'skirt',
              'school uniform bottom': 'skirt', 'ribbon skirt': 'ribbon skirt',
              'jumpsuit pants': 'overall legs', 'jumpsuit pants alt': 'overall legs',
              'beach bottom': ('swim legs m', 'swim legs f')}
BAG_MAP = {'bag': 'bag', 'rucksack': 'rucksack', 'sporty backpack': 'sporty'}
SHOES = '383838'
NONE = 0xFFFF


def rgb555(c):
    return (c[0] >> 3) | ((c[1] >> 3) << 5) | ((c[2] >> 3) << 10)


def from555(v):
    return ((v & 31) << 3, ((v >> 5) & 31) << 3, ((v >> 10) & 31) << 3)


# ---------------------------------------------------------------------------
# The CREATE parts (build.py output)

class OldParts:
    def __init__(self):
        h = open(os.path.join(ROOT, 'src/mp/mp_creator_data.h')).read()
        self.blob = open(os.path.join(ROOT, 'graphics/moemon/creator/parts.bin'), 'rb').read()
        body = re.search(r'sCrImgOffset\[CR_IMG_COUNT\] = \{(.*?)\};', h, re.S).group(1)
        self.offsets = [int(x) for x in re.findall(r'\d+', body)]
        body = re.search(r'sCrBase\[2\]\[\d+\]\[CR_VIEW_COUNT\] = \{(.*?)\};', h, re.S).group(1)
        nums = [int(x) for x in re.findall(r'\d+', body)]
        self.base = [[nums[(g * 4 + s) * 5:(g * 4 + s) * 5 + 5] for s in range(4)] for g in range(2)]
        body = re.search(r'sCrStyles\[\] = \{(.*?)\};', h, re.S).group(1)
        self.styles = [(n.lower(), int(a), int(b)) for n, a, b in re.findall(r'\{"([^"]*)", (\d+), (\d+)\}', body)]
        body = re.search(r'sCrColors\[\] = \{(.*?)\};\n', h, re.S).group(1)
        self.colors = [(n.lower(), [int(x) for x in imgs.split(',')])
                       for n, imgs in re.findall(r'\{"([^"]*)", \{([^}]*)\}\}', body)]
        body = re.search(r'sCrCatStyles\[2\]\[CR_CAT_COUNT\]\[2\] = \{(.*?)\};', h, re.S).group(1)
        nums = [int(x) for x in re.findall(r'\d+', body)]
        self.cat = {(g, c): (nums[(gi * 5 + ci) * 2], nums[(gi * 5 + ci) * 2 + 1])
                    for gi, g in enumerate('mf') for ci, c in enumerate(CATS)}
        self._cache = {}

    def image(self, idx):
        if idx == NONE:
            return None
        if idx in self._cache:
            return self._cache[idx]
        b = self.blob
        o = self.offsets[idx]
        w, h, n = b[o] | b[o + 1] << 8, b[o + 2] | b[o + 3] << 8, b[o + 4]
        pal = [b[o + 6 + i * 2] | b[o + 7 + i * 2] << 8 for i in range(n)]
        p = o + 6 + n * 2
        flat = []
        while len(flat) < w * h:
            flat += [b[p + 1]] * b[p]
            p += 2
        a = np.zeros((h, w, 4), np.uint8)
        for i, v in enumerate(flat):
            if v:
                a[i // w, i % w] = from555(pal[v - 1]) + (255,)
        self._cache[idx] = a
        return a

    def style_list(self, g, cat):
        first, n = self.cat[(g, cat)]
        return [(i, self.styles[first + i]) for i in range(n)]

    def frames27(self, color_idx_or_imgs):
        """walk/run/bike strips -> 27 frames of 32x32."""
        imgs = color_idx_or_imgs
        out = []
        for v in (2, 3, 4):
            a = self.image(imgs[v])
            for f in range(9):
                out.append(None if a is None else a[:, f * 32:(f + 1) * 32])
        return out


def main_tones(a):
    """The main color of a part (most pixels, weighted by brightness) and the next different one."""
    if a is None:
        return None, None
    px = a[a[..., 3] > 0][:, :3].astype(int)
    cnt = Counter(map(tuple, px.tolist()))
    # Weighted by brightness: the dark outline around long hair can outnumber the hair itself
    cand = [(n * hsv(c)[2], c) for c, n in cnt.items() if hsv(c)[2] > 0.15]
    if not cand:
        cand = [(n, c) for c, n in cnt.items()]
    cand.sort(reverse=True)
    main = cand[0][1]
    second = None
    for n, c in cand[1:]:
        if sum(abs(x - y) for x, y in zip(c, main)) > 90:
            second = c
            break
    return main, second


def hair_target(main):
    """Target for the kit's hair ramp so its most used shade (#8080f0) comes out as `main`."""
    h, s, v = hsv(main)
    _, bs, bv = hsv(BASE_HAIR)
    _, is_, iv = hsv(hx('8080f0'))
    t = colorsys.hsv_to_rgb(h, min(1.0, s * bs / is_), min(1.0, v * bv / iv))
    return tuple(int(round(x * 255)) for x in t)


# ---------------------------------------------------------------------------
# Encoded images: 32x32, palette of roles

class Store:
    def __init__(self):
        self.blob = bytearray()
        self.offsets = []
        self.layers = []   # each: 27 image ids

    def image(self, entries, idx):
        """entries: list of role tuples (index 1..n); idx: 32x32 uint8."""
        if not idx.any():
            return NONE
        out = bytearray([len(entries)])
        for e in entries:
            if e[0] == 'lit':
                v = rgb555(e[1])
                out += bytes((1, v & 255, v >> 8, 0))
            elif e[0] == 'role':
                out += bytes((2, e[1], e[2], e[3]))
            elif e[0] == 'skin':
                out += bytes((3, e[1], 0, 0))
            else:
                out += bytes((4, 0, 0, 0))
        flat = idx.reshape(-1)
        i = 0
        while i < len(flat):
            j = i
            while j < len(flat) and flat[j] == flat[i] and j - i < 255:
                j += 1
            out += bytes((j - i, int(flat[i])))
            i = j
        self.offsets.append(len(self.blob))
        self.blob += out
        return len(self.offsets) - 1

    def layer(self, frames27, classify):
        """frames27: 27 RGBA arrays (or None); classify(rgb) -> role tuple. Returns a layer id or NONE."""
        if frames27 is None or all(f is None or not f[..., 3].any() for f in frames27):
            return NONE
        ids = []
        for fr in frames27:
            if fr is None:
                ids.append(NONE)
                continue
            entries, lut = [], {}
            idx = np.zeros((32, 32), np.uint8)
            for y, x in zip(*np.nonzero(fr[..., 3])):
                c = tuple(int(v) for v in fr[y, x, :3])
                if c not in lut:
                    entries.append(classify(c))
                    lut[c] = len(entries)
                idx[y, x] = lut[c]
            ids.append(self.image(entries, idx))
        self.layers.append(ids)
        return len(self.layers) - 1

    def layer_indexed(self, frames27):
        """frames27: list of (entries, idx) or None."""
        if all(f is None or not f[1].any() for f in frames27):
            return NONE
        self.layers.append([NONE if f is None else self.image(*f) for f in frames27])
        return len(self.layers) - 1


def bbox(a):
    ys, xs = np.nonzero(a[..., 3] if a.ndim == 3 else a)
    if not len(xs):
        return None
    return xs.min(), xs.max(), ys.min(), ys.max()


def shift(a, dx, dy):
    out = np.zeros_like(a)
    h, w = a.shape[:2]
    ys, xs = np.nonzero(a[..., 3] if a.ndim == 3 else a)
    for y, x in zip(ys, xs):
        if 0 <= y + dy < h and 0 <= x + dx < w:
            out[y + dy, x + dx] = a[y, x]
    return out


# ---------------------------------------------------------------------------

class Builder:
    def __init__(self, kit_dir, frlg_dir):
        self.kit = Kit3(kit_dir)
        self.frlg = os.path.join(frlg_dir, 'graphics/object_events/pics/people')
        self.old = OldParts()
        self.st = Store()

    def kit_layer(self, frames, kind, nacc=0):
        return self.st.layer(frames, lambda c: marker_role(c, kind, nacc))

    def build_kit(self):
        k = self.kit
        self.body = [self.kit_layer(k.body_frames('body', 'default-' + g), 'body') for g in ('male', 'female')]
        self.head = self.kit_layer(k.head_frames('head', 'standard-head'), 'head')
        self.eyes = self.kit_layer(k.head_frames('eyes', 'default', eyes=True), 'eyes')
        self.hair = []
        for name, nacc in k.hairs:
            self.hair.append([self.kit_layer(k.head_frames(layer, name), 'hair', nacc)
                              for layer in ('hair', 'hair-back', 'hair-front')])
        self.hat = []
        for name, n in k.hats:
            self.hat.append([self.kit_layer(k.head_frames('hat', name), 'clothes'),
                             self.kit_layer(k.head_frames('hat-back', name), 'clothes'),
                             self.st.layer(k.head_frames('hat-mask', name), lambda c: ('mask',)),
                             self.st.layer(k.head_frames('hat-back-mask', name), lambda c: ('mask',))])
        # Which CREATE color each hat color slot takes: the biggest area the hat's main color, the next its
        # second color, then white and dark grey
        self.hat_slots = []
        for name, n in k.hats:
            fr = k.head_frames('hat', name)[0]
            cnt = Counter()
            for c in map(tuple, fr[fr[..., 3] > 0][:, :3].tolist()):
                r = marker_role(c, 'clothes')
                if r[0] == 'role' and G_C0 <= r[1] <= G_C3:
                    cnt[r[1] - G_C0] += 1
            order = [g for g, _ in cnt.most_common()] + [g for g in range(4) if g not in cnt]
            slots = [0] * 4
            for rank, g in enumerate(order):
                slots[g] = rank
            self.hat_slots.append(slots)
        self.top, self.bottom, self.shoes, self.band = [], [], [], []
        for g in 'mf':
            tf = k.body_frames('d%s-top' % g, 't-shirt')
            bf = k.body_frames('d%s-bottom' % g, 'pants')
            self.top.append(self.kit_layer(tf, 'clothes'))
            self.bottom.append(self.kit_layer(bf, 'clothes'))
            self.shoes.append(self.kit_layer(k.body_frames('d%s-shoes' % g, 'simple'), 'clothes'))
            band = []   # neck, waist rows
            for t, b in zip(tf, bf):
                neck = bbox(t)[2] if bbox(t) else 22
                band.append((neck, bbox(b)[2] if bbox(b) else neck + 4))
            self.band.append(band)
        self.tshirt_frames = {g: k.body_frames('d%s-top' % g, 't-shirt') for g in 'mf'}
        self.head_frames = k.head_frames('head', 'standard-head')

    # ---- official donors
    def donor_frames(self, name):
        im = Image.open(os.path.join(self.frlg, name + '.png'))
        a = np.array(im)
        pal = im.getpalette()
        return [a[:32, i * 16:(i + 1) * 16] for i in range(9)], [tuple(pal[i * 3:i * 3 + 3]) for i in range(16)]

    def build_donors(self):
        self.donors = {}
        self.donor_list = []
        for key, spec in DONORS.items():
            frames, pal = self.donor_frames(spec['src'])
            ramp = spec['ramp']
            cnt = Counter()
            for fr in frames:
                for i in ramp:
                    cnt[i] += int((fr[16:] == i).sum())
            ref = pal[cnt.most_common(1)[0][0]] if cnt else None
            per_g = []
            for g in 'mf':
                out27, waist27 = [], []
                for fi, (view, anim, d, f) in enumerate(FRAMES):
                    wi = fi % 9
                    src = np.zeros((32, 32), np.uint8)
                    src[:, 8:24] = frames[wi]
                    # Line up with the kit's torso in this frame (running and biking lean)
                    a = bbox(self.tshirt_frames[g][fi])
                    b = bbox(self.tshirt_frames[g][wi])
                    dx = int(round((a[0] + a[1]) / 2 - (b[0] + b[1]) / 2))
                    dy = int(a[2] - b[2])
                    if view == 'bike' and spec['band'] in ('legs', 'both'):
                        out27.append(None)
                        waist27.append(32)
                        continue
                    rows = np.arange(32)[:, None]
                    # The step frames bob down like the kit's
                    stand = frames[[0, 1, 2][[0, 1, 2, 0, 0, 1, 1, 2, 2][wi]]]
                    bob = int(np.nonzero(frames[wi])[0].min() - np.nonzero(stand)[0].min())
                    w = spec['waist']
                    waist = (w if isinstance(w, int) else w[[0, 1, 2, 0, 0, 1, 1, 2, 2][wi]]) + bob
                    if spec['band'] == 'legs':
                        m = (src > 0) & (rows >= waist)
                    elif spec['band'] in ('torso', 'both'):
                        m = (src > 0) & (rows >= 16)
                    else:   # bag: its ramp plus the outline touching it
                        r = np.isin(src, ramp) & (rows >= waist)
                        near = np.zeros_like(r)
                        near[1:] |= r[:-1]
                        near[:-1] |= r[1:]
                        near[:, 1:] |= r[:, :-1]
                        near[:, :-1] |= r[:, 1:]
                        m = r | (near & (src == 15))
                    m &= ~np.isin(src, spec['drop'])
                    idx = np.where(m, src, 0)
                    idx = shift(idx, dx, dy)
                    entries, lut = [], {}
                    out = np.zeros((32, 32), np.uint8)
                    for y, x in zip(*np.nonzero(idx)):
                        i = int(idx[y, x])
                        if i not in lut:
                            if 1 <= i <= 4:
                                entries.append(('skin', i - 1))
                            elif i in ramp:
                                rs, rv = (64, 64)
                                _, s, v = hsv(pal[i])
                                _, bs, bv = hsv(ref)
                                rs = min(255, int(round(s / bs * 64))) if bs else 0
                                rv = min(255, int(round(v / bv * 64))) if bv else 0
                                entries.append(('role', G_C0, rs, rv))
                            else:
                                entries.append(('lit', pal[i]))
                            lut[i] = len(entries)
                        out[y, x] = lut[i]
                    out27.append((entries, out))
                    waist27.append(min(32, waist + dy) if spec['band'] == 'legs' else 32)
                per_g.append((self.st.layer_indexed(out27), waist27))
            self.donors[key] = len(self.donor_list)
            self.donor_list.append((key, spec['band'], per_g))

    # ---- borrowed head accessories (CREATE parts)
    def old_head_boxes(self, g):
        gi = 'mf'.index(g)
        base = self.old.frames27(self.old.base[gi][1])
        _, (name, fc, n) = self.old.style_list(g, 'tops')[0]
        shirt = self.old.frames27(self.old.colors[fc][1])
        out = []
        for b, s in zip(base, shirt):
            top = bbox(s)[2]
            h = b.copy()
            h[top:] = 0
            out.append(bbox(h))
        return out

    def build_accessories(self):
        self.acc = []        # (gender, color index, layer, mask kit hat)
        kit_hats = [n for n, _ in self.kit.hats]
        for g in 'mf':
            gi = 'mf'.index(g)
            ob = self.old_head_boxes(g)
            kb = [bbox(f) for f in self.head_frames]
            for si, (name, fc, n) in self.old.style_list(g, 'hats'):
                if name not in HAT_BORROW:
                    continue
                anchor, maskhat, nudge = HAT_BORROW[name]
                for ci in range(fc, fc + n):
                    frames = self.old.frames27(self.old.colors[ci][1])
                    out = []
                    for fi, fr in enumerate(frames):
                        if fr is None:
                            out.append(None)
                            continue
                        o, k = ob[fi], kb[fi]
                        dx = int(round((k[0] + k[1]) / 2 - (o[0] + o[1]) / 2))
                        if anchor == 'face':
                            dy = int(round((k[2] + 0.55 * (k[3] - k[2])) - (o[2] + 0.55 * (o[3] - o[2]))))
                        else:
                            dy = int(k[2] - o[2])
                        out.append(np.ascontiguousarray(shift(fr, dx, dy + nudge)))
                    lay = self.st.layer(out, lambda c: ('lit', c))
                    self.acc.append((gi, ci, lay, kit_hats.index(maskhat) if maskhat else 0xFF))

    # ---- tables
    def build_maps(self):
        kit_hats = [n for n, _ in self.kit.hats]
        kit_hairs = [n for n, _ in self.kit.hairs]
        self.hair_auto = [[kit_hairs.index(n) for n in HAIR_AUTO[g]] for g in 'mf']
        self.maps = {}
        for g in 'mf':
            gi = 'mf'.index(g)

            def donor(v):
                if v is None:
                    return 0xFF
                if isinstance(v, tuple):
                    v = v[gi]
                return self.donors[v]
            self.maps[(g, 'tops')] = [donor(TOP_MAP.get(name)) for _, (name, _, _) in self.old.style_list(g, 'tops')]
            self.maps[(g, 'bottoms')] = [donor(BOTTOM_MAP.get(name)) for _, (name, _, _) in self.old.style_list(g, 'bottoms')]
            self.maps[(g, 'bags')] = [donor(BAG_MAP.get(name)) for _, (name, _, _) in self.old.style_list(g, 'bags')]
            hats = []
            for _, (name, _, _) in self.old.style_list(g, 'hats'):
                if name in HAT_KIT:
                    hats.append(kit_hats.index(HAT_KIT[name]))
                elif name in HAT_BORROW:
                    hats.append(0x80)
                else:
                    print('  hat without a field sprite:', g, name)
                    hats.append(0xFF)
            self.maps[(g, 'hats')] = hats
        # Colors of every CREATE part, from its ID picture (what the field sprite matches; bags: the walking sprite)
        self.tones = []
        hair_ranges = [range(self.old.styles[self.old.cat[(g, 'hair')][0]][1],
                             self.old.styles[self.old.cat[(g, 'hair')][0] + self.old.cat[(g, 'hair')][1] - 1][1]
                             + self.old.styles[self.old.cat[(g, 'hair')][0] + self.old.cat[(g, 'hair')][1] - 1][2])
                       for g in 'mf']
        for ci, (name, imgs) in enumerate(self.old.colors):
            a = self.old.image(imgs[0]) if imgs[0] != NONE else self.old.image(imgs[2])
            main, second = main_tones(a)
            if main is None:
                main = (128, 128, 128)
            if any(ci in r for r in hair_ranges):
                main = hair_target(main)
            self.tones.append((main, second or (248, 248, 248)))

    def run(self, preview=None):
        self.build_kit()
        self.build_donors()
        self.build_accessories()
        self.build_maps()
        self.write()
        if preview:
            self.preview(preview)

    def write(self):
        st = self.st
        with open(os.path.join(ROOT, 'graphics/moemon/creator/ow3.bin'), 'wb') as f:
            f.write(st.blob)
        L = ['// Moemon Co-op: CREATE field sprites, generated by tools/moemon_creator/build_ow.py. Do not edit.',
             '// Art: the Gen 3 trainer kit (Top Down Sprite Maker), official FireRed/LeafGreen sprites,',
             '// and the CREATE parts for head accessories.', '']
        L.append('#define OW3_IMG_COUNT %d' % len(st.offsets))
        L.append('static const u32 sOw3ImgOffset[OW3_IMG_COUNT] = {')
        for i in range(0, len(st.offsets), 12):
            L.append('    ' + ', '.join(map(str, st.offsets[i:i + 12])) + ',')
        L.append('};')
        L.append('#define OW3_LAYER_COUNT %d' % len(st.layers))
        L.append('static const u16 sOw3Layers[OW3_LAYER_COUNT][27] = {')
        for ids in st.layers:
            L.append('    {' + ', '.join(map(str, ids)) + '},')
        L.append('};')
        L.append('static const u16 sOw3Body[2] = {%d, %d};' % tuple(self.body))
        L.append('static const u16 sOw3Head = %d, sOw3Eyes = %d;' % (self.head, self.eyes))
        L.append('#define OW3_HAIR_COUNT %d' % len(self.hair))
        L.append('// hair, hair behind the head, hair in front of a hat; number of accessory colors')
        L.append('static const struct { const char *name; u16 layer[3]; u8 acc; } sOw3Hair[OW3_HAIR_COUNT] = {')
        for (name, nacc), lay in zip(self.kit.hairs, self.hair):
            L.append('    {"%s", {%s}, %d},' % (name.replace('-', ' ').upper(), ', '.join(map(str, lay)), nacc))
        L.append('};')
        L.append('#define OW3_HAT_COUNT %d' % len(self.hat))
        L.append('// hat, hat behind the head, hair mask, back hair mask')
        L.append('static const u16 sOw3Hat[OW3_HAT_COUNT][4] = {')
        for lay in self.hat:
            L.append('    {' + ', '.join(map(str, lay)) + '},')
        L.append('};')
        L.append('// Hat color slot -> 0 main color, 1 second color, 2 white, 3 dark grey')
        L.append('static const u8 sOw3HatSlots[OW3_HAT_COUNT][4] = {%s};' % ', '.join(
            '{%s}' % ', '.join(map(str, r)) for r in self.hat_slots))
        L.append('static const u16 sOw3Top[2] = {%d, %d}, sOw3Bottom[2] = {%d, %d}, sOw3Shoes[2] = {%d, %d};'
                 % tuple(self.top + self.bottom + self.shoes))
        L.append('// Rows where the kit\'s shirt and pants start in each frame')
        L.append('static const u8 sOw3Band[2][27][2] = {')
        for g in range(2):
            L.append('    {' + ', '.join('{%d, %d}' % b for b in self.band[g]) + '},')
        L.append('};')
        L.append('enum { OW3_LEGS, OW3_TORSO, OW3_BOTH, OW3_BAG };')
        L.append('#define OW3_DONOR_COUNT %d' % len(self.donor_list))
        L.append('// Official sprites: band, layer per body, first row of the legs per frame')
        L.append('static const struct { u8 band; u16 layer[2]; u8 waist[2][27]; } sOw3Donor[OW3_DONOR_COUNT] = {')
        bands = {'legs': 'OW3_LEGS', 'torso': 'OW3_TORSO', 'both': 'OW3_BOTH', 'bag': 'OW3_BAG'}
        for key, band, per_g in self.donor_list:
            L.append('    {%s, {%d, %d}, {{%s}, {%s}}},   // %s' % (
                bands[band], per_g[0][0], per_g[1][0], ', '.join(map(str, per_g[0][1])),
                ', '.join(map(str, per_g[1][1])), key))
        L.append('};')
        L.append('// Head accessories borrowed from the CREATE parts: body, color, layer, kit hat whose mask hides hair')
        L.append('#define OW3_ACC_COUNT %d' % len(self.acc))
        L.append('static const struct { u8 girl; u16 color; u16 layer; u8 maskHat; } sOw3Acc[OW3_ACC_COUNT] = {')
        for gi, ci, lay, mh in self.acc:
            L.append('    {%d, %d, %d, %d},' % (gi, ci, lay, mh))
        L.append('};')
        L.append('// CREATE style -> field sprite: 0xFF = the kit\'s own (T-shirt / pants), else a donor;')
        L.append('// hats: kit hat, 0x80 = borrowed accessory, 0xFF = none')
        for cat in ('tops', 'bottoms', 'hats', 'bags'):
            n = max(len(self.maps[(g, cat)]) for g in 'mf')
            L.append('static const u8 sOw3Map_%s[2][%d] = {' % (cat, n))
            for g in 'mf':
                v = self.maps[(g, cat)] + [0xFF] * (n - len(self.maps[(g, cat)]))
                L.append('    {' + ', '.join(map(str, v)) + '},')
            L.append('};')
        L.append('static const u8 sOw3HairAuto[2][4] = {%s};' % ', '.join(
            '{%s}' % ', '.join(map(str, r)) for r in self.hair_auto))
        L.append('// Main and second color of every CREATE part (hair: the kit hair ramp\'s target)')
        L.append('static const u16 sOw3Tone[%d][2] = {' % len(self.tones))
        for i in range(0, len(self.tones), 8):
            L.append('    ' + ' '.join('{0x%04x, 0x%04x},' % (rgb555(a), rgb555(b)) for a, b in self.tones[i:i + 8]))
        L.append('};')
        L.append('static const u16 sOw3Skin[4][4] = {%s};' % ', '.join(
            '{%s}' % ', '.join('0x%04x' % rgb555(hx(c)) for c in r) for r in SKINS))
        L.append('#define OW3_EYE_COLOR 0x%04x' % rgb555(hx(EYE_COLOR)))
        L.append('#define OW3_SHOES_COLOR 0x%04x' % rgb555(hx(SHOES)))
        with open(os.path.join(ROOT, 'src/mp/mp_creator_ow.h'), 'w') as f:
            f.write('\n'.join(L) + '\n')
        print('%d images in %d layers, %.1f KB' % (len(st.offsets), len(st.layers), len(st.blob) / 1024))

    # ---------------------------------------------------------------------
    # Reference renderer (mp_creator.c ComposeOverworld does the same)

    def decode(self, img_id):
        b = self.st.blob
        o = self.st.offsets[img_id]
        n = b[o]
        ents = [tuple(b[o + 1 + i * 4:o + 5 + i * 4]) for i in range(n)]
        p = o + 1 + n * 4
        flat = []
        while len(flat) < 1024:
            flat += [b[p + 1]] * b[p]
            p += 2
        return ents, np.array(flat, np.uint8).reshape(32, 32)

    def draw(self, canvas, layer, fi, targets, skin, rows=(0, 32), mask=None):
        if layer == NONE:
            return
        img = self.st.layers[layer][fi]
        if img == NONE:
            return
        ents, idx = self.decode(img)
        cols = [None]
        for e in ents:
            if e[0] == 1:
                cols.append(from555(e[1] | e[2] << 8))
            elif e[0] == 2:
                t = targets.get(e[1], (255, 0, 255))
                h, s, v = hsv(t)
                c = colorsys.hsv_to_rgb(h, min(1, s * e[2] / 64), min(1, v * e[3] / 64))
                cols.append(from555(rgb555(tuple(int(round(x * 255)) for x in c))))
            elif e[0] == 3:
                cols.append(from555(skin[e[1]]))
            else:
                cols.append((255, 0, 0))
        for y in range(rows[0], rows[1]):
            for x in range(32):
                i = idx[y, x]
                if i and not (mask is not None and mask[y, x]):
                    canvas[y, x] = cols[i] + (255,)

    def mask_of(self, layer, fi):
        m = np.zeros((32, 32), bool)
        if layer == NONE or self.st.layers[layer][fi] == NONE:
            return m
        _, idx = self.decode(self.st.layers[layer][fi])
        return idx > 0

    def render(self, r):
        """r: dict(girl, skin, hair, hairColor, top, topColor, bottom, bottomColor, hat, hatColor, bag, bagColor,
        fieldHair) with style indexes like struct MpCreated. Returns 27 RGBA frames."""
        g = r['girl']
        gs = 'mf'[g]
        skin = [rgb555(hx(c)) for c in SKINS[r['skin']]]

        def color_index(cat, style, color):
            first, _ = self.old.cat[(gs, cat)]
            return self.old.styles[first + style][1] + color
        hair = r['fieldHair'] - 1 if r.get('fieldHair') else self.hair_auto[g][r['hair']]
        hairT = self.tones[color_index('hair', r['hair'], r['hairColor'])][0]
        topT = self.tones[color_index('tops', r['top'], r['topColor'])]
        botT = self.tones[color_index('bottoms', r['bottom'], r['bottomColor'])]
        hatT = self.tones[color_index('hats', r['hat'] - 1, r['hatColor'])] if r['hat'] else None
        bagT = self.tones[color_index('bags', r['bag'] - 1, r['bagColor'])] if r['bag'] else None
        top_src = self.maps[(gs, 'tops')][r['top']]
        bot_src = self.maps[(gs, 'bottoms')][r['bottom']]
        bag_src = self.maps[(gs, 'bags')][r['bag'] - 1] if r['bag'] else 0xFF
        kit_hat, acc = 0xFF, None
        if r['hat']:
            hm = self.maps[(gs, 'hats')][r['hat'] - 1]
            if hm == 0x80:
                ci = color_index('hats', r['hat'] - 1, r['hatColor'])
                acc = next(a for a in self.acc if a[0] == g and a[1] == ci)
            else:
                kit_hat = hm
        mask_hat = kit_hat if kit_hat != 0xFF else (acc[3] if acc else 0xFF)
        hair_t = {G_HAIR: hairT, G_ACC0: topT[0], G_ACC1: botT[0], G_IRIS: hx(EYE_COLOR)}
        hat_t = {}
        if hatT and kit_hat != 0xFF:
            choice = [hatT[0], hatT[1], (248, 248, 248), hx(SHOES)]
            hat_t = {G_C0 + i: choice[self.hat_slots[kit_hat][i]] for i in range(4)}
        out = []
        for fi in range(27):
            c = np.zeros((32, 32, 4), np.uint8)
            neck, waist = self.band[g][fi]
            top_d = self.donor_list[top_src] if top_src != 0xFF else None
            bot_d = self.donor_list[bot_src] if bot_src != 0xFF else None
            if top_d and self.st.layers[top_d[2][g][0]][fi] == NONE:
                top_d = None
            if bot_d and self.st.layers[bot_d[2][g][0]][fi] == NONE:
                bot_d = None
            W = bot_d[2][g][1][fi] if bot_d else waist
            if kit_hat != 0xFF:
                self.draw(c, self.hat[kit_hat][1], fi, hat_t, skin)
            hb = self.hair[hair][1]
            self.draw(c, hb, fi, hair_t, skin, mask=self.mask_of(self.hat[mask_hat][3], fi) if mask_hat != 0xFF else None)
            self.draw(c, self.body[g], fi, {}, skin)
            # legs, then the torso over them
            if top_d and top_d[1] == 'both':
                self.draw(c, top_d[2][g][0], fi, {G_C0: topT[0]}, skin, rows=(neck, 32))
            else:
                if bot_d:
                    self.draw(c, bot_d[2][g][0], fi, {G_C0: botT[0]}, skin, rows=(W, 32))
                else:
                    self.draw(c, self.bottom[g], fi, {G_C0: botT[0]}, skin)
                    self.draw(c, self.shoes[g], fi, {G_C0: hx(SHOES)}, skin)
                if top_d:
                    self.draw(c, top_d[2][g][0], fi, {G_C0: topT[0]}, skin, rows=(neck, W))
                else:
                    self.draw(c, self.top[g], fi, {G_C0: topT[0]}, skin, rows=(0, W) if bot_d else (0, 32))
            if bag_src != 0xFF:
                self.draw(c, self.donor_list[bag_src][2][g][0], fi, {G_C0: bagT[0]}, skin)
            self.draw(c, self.head, fi, {}, skin)
            self.draw(c, self.eyes, fi, hair_t, skin)
            self.draw(c, self.hair[hair][0], fi, hair_t, skin,
                      mask=self.mask_of(self.hat[mask_hat][2], fi) if mask_hat != 0xFF else None)
            if kit_hat != 0xFF:
                self.draw(c, self.hat[kit_hat][0], fi, hat_t, skin)
            self.draw(c, self.hair[hair][2], fi, hair_t, skin)
            if acc:
                self.draw(c, acc[2], fi, {}, skin)
            out.append(c)
        return out

    def preview(self, path, looks=None):
        looks = looks or PREVIEW_LOOKS
        rows = []
        for r in looks:
            fr = self.render(r)
            rows.append(np.concatenate([fr[i] for i in (0, 1, 2, 3, 4, 9, 18, 19, 20)], 1))
        a = np.concatenate(rows, 0)
        im = Image.fromarray(a)
        bg = Image.new('RGBA', im.size, (120, 180, 120, 255))
        bg.alpha_composite(im)
        bg.resize((im.width * 4, im.height * 4), Image.NEAREST).save(path)


def look(girl, skin, hair, hc, top, tc, bottom, bc, hat=0, hatc=0, bag=0, bagc=0, fieldHair=0):
    return dict(girl=girl, skin=skin, hair=hair, hairColor=hc, top=top, topColor=tc, bottom=bottom, bottomColor=bc,
                hat=hat, hatColor=hatc, bag=bag, bagColor=bagc, fieldHair=fieldHair)


PREVIEW_LOOKS = [look(0, 1, 1, 1, 0, 3, 0, 3), look(1, 1, 3, 1, 0, 3, 0, 3)]


if __name__ == '__main__':
    args = sys.argv[1:]
    prev = None
    if '--preview' in args:
        i = args.index('--preview')
        prev = args[i + 1]
        del args[i:i + 2]
    Builder(args[0], args[1]).run(prev)
