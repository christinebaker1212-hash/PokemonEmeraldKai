#!/usr/bin/env python3
"""Moemon Co-op trainer creator: extra field-sprite hairstyles cut from fan-made
FRLG-style overworld sprites by aveontrainer
(https://www.deviantart.com/aveontrainer/gallery/67900303/overworld, used with
credit for a non-commercial project, as the artist allows).

The sheets there are RPG Maker style (4 rows S/W/E/N x 4 walk frames) drawn at
2x; at their native size the characters are FRLG scale, like our Gen 3 kit.
For every donor listed in hair_donors.txt this takes the standing frame of
each direction, finds the eyes, cuts out the hair (and hair accessories) and
places it in the kit's 20x20 head cells so the eyes line up with the kit's.
The hair is redrawn in the kit's hair marker colors (accessories in the first
clothing ramp, so they take the top's color like the kit's own accessories).

build_ow.py calls load(IMG_DIR) and adds the results after the kit's 30 hairs.
  donor_hair.py --fetch IMG_DIR              download the gallery (NNN_Title.png, gallery order)
  donor_hair.py IMG_DIR KIT3_DIR OUT.png [NAME...]   review sheet: donor, cut, on the kit head
Only clean cuts are listed; a cut with stray pixels is left out rather than patched by hand.
"""
import colorsys
import os
import re
import sys
from collections import Counter

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from kit3 import head_offset  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
DONORS = os.path.join(HERE, 'hair_donors.txt')

# Kit head cell: eyes at rows 13-14, x 8 and 11 (S); x 7 (W); x 12 (E)
KIT_EYE_ROW = 13
KIT_EYE_X = {'S': 9.5, 'W': 7, 'E': 12}
MARGIN = 4   # spare rows above and below the 20x20 head cells in our sheets (kit3.head_frames)
HAIR_MARK = [(0xb0, 0xb0, 0xf8), (0x80, 0x80, 0xf0), (0x48, 0x48, 0xc8), (0x30, 0x30, 0x70)]   # light .. dark
ACC_MARK = [[(0xf8, 0xb0, 0xb0), (0xf0, 0x80, 0x80), (0xc8, 0x48, 0x48), (0x70, 0x30, 0x30)],
            [(0xf8, 0xd4, 0xb0), (0xf0, 0xb8, 0x80), (0xc8, 0x88, 0x48), (0x70, 0x50, 0x30)]]


# The FRLG skin ramps these sprites use (the colors next to the eyes, gallery-wide)
STD_SKIN = {tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for h in (
    'f0c098 f8d8b8 f0b890 f8d0b0 d89870 d89070 f8d0b8 f8c090 f7bd94 ffd6b5 cf966f c88868 f7ba94 d8a078 '
    'de9473 ffc594 ffd5b4 deb4a4 ffeed5 bd9473 c58b6a eeb494 784040 602020 7b4141').split()}


def hsv(c):
    return colorsys.rgb_to_hsv(*(v / 255 for v in c[:3]))


def lum(c):
    return (0.3 * c[0] + 0.59 * c[1] + 0.11 * c[2]) / 255


def skinish(c):
    h, s, v = hsv(c)
    return (h <= 0.12 or h >= 0.95) and 0.1 <= s <= 0.75 and v >= 0.35


def native(path):
    """The 2x sheet at its native size (picking the pixel phase that agrees best)."""
    a = np.array(Image.open(path).convert('RGBA'))
    h, w = a.shape[:2]
    best = None
    for ox in (0, 1):
        for oy in (0, 1):
            b = a[oy:oy + (h - oy) // 2 * 2, ox:ox + (w - ox) // 2 * 2]
            same = (b[0::2, 0::2] == b[1::2, 1::2]).all(-1)
            op = b[0::2, 0::2, 3] > 0
            r = same[op].mean() if op.any() else 0
            if best is None or r > best[0]:
                best = (r, ox, oy)
    _, ox, oy = best
    n = a[oy::2, ox::2][:(h - oy) // 2, :(w - ox) // 2].copy()
    n[..., 3] = np.where(n[..., 3] >= 128, 255, 0)
    return n


def standing(n):
    """Standing frame of each direction (rows S, W, E, N; column 0)."""
    H, W = n.shape[0] // 4, n.shape[1] // 4
    return {d: n[r * H:(r + 1) * H, 0:W] for r, d in enumerate('SWEN')}


def px(f, y, x):
    if 0 <= y < f.shape[0] and 0 <= x < f.shape[1] and f[y, x, 3]:
        return tuple(int(v) for v in f[y, x, :3])
    return None


def is_eye(f, y, x):
    c, l, r = px(f, y, x), px(f, y, x - 1), px(f, y, x + 1)
    return c is not None and l is not None and r is not None and lum(c) < 0.33 and skinish(l) and skinish(r)


def find_eyes(fr):
    """(row, [x...]) of the eyes in each standing frame, or None."""
    out = {}
    f = fr['S']
    w = f.shape[1]
    best = None
    for y in range(7, 17):
        for x1 in range(1, w - 4):
            for dx in range(3, 7):
                x2 = x1 + dx
                if is_eye(f, y, x1) and is_eye(f, y, x2) and abs((x1 + x2) / 2 - (w - 1) / 2) <= 1.5:
                    score = (abs(y - 12), abs((x1 + x2) / 2 - (w - 1) / 2))
                    if best is None or score < best[0]:
                        best = (score, y, [x1, x2])
    out['S'] = (best[1], best[2]) if best else None
    for d, rng in (('W', range(1, w // 2 + 1)), ('E', range(w // 2 - 1, w - 1))):
        f = fr[d]
        best = None
        for y in range(7, 17):
            for x in rng:
                if is_eye(f, y, x):
                    ref = out['S'][0] if out['S'] else 12
                    score = abs(y - ref)
                    if best is None or score < best[0]:
                        best = (score, y, [x])
        out[d] = (best[1], best[2]) if best else None
    return out


class Donor:
    def __init__(self, img_dir, file, name, opts):
        self.name = name
        self.opts = opts
        self.fr = standing(native(os.path.join(img_dir, file)))
        self.eyes = find_eyes(self.fr)
        for d in 'SWE':
            if d in opts.get('eyes', {}):
                self.eyes[d] = opts['eyes'][d]
        self.ok = self.eyes['S'] is not None
        if not self.ok:
            return
        self.ey = self.eyes['S'][0]
        self.classify_colors()

    def classify_colors(self):
        fS = self.fr['S']
        ey, ex = self.eyes['S']
        # What the top rows of the head are made of is hair, whatever it looks like
        seed = Counter()
        for d in 'SN':
            f = self.fr[d]
            rows = [y for y in range(f.shape[0]) if f[y, :, 3].any()][:3]
            for y in rows:
                for x in range(f.shape[1]):
                    c = px(f, y, x)
                    if c and lum(c) >= 0.06:
                        seed[c] += 1
        # Skin: colors beside and under the eyes
        skin = set()
        for x in ex:
            for y, xx in ((ey, x - 1), (ey, x + 1), (ey + 2, x), (ey + 2, x - 1), (ey + 2, x + 1)):
                c = px(fS, y, xx)
                if c and skinish(c):
                    skin.add(c)
        # where each color sits: hair colors live above the eyes, the face's shade below
        above, total = Counter(), Counter()
        for d in 'SN':
            f = self.fr[d]
            for y, x in zip(*np.nonzero(f[..., 3])):
                c = px(f, y, x)
                total[c] += 1
                if y < ey:
                    above[c] += 1
        self.mostly_above = {c for c in total if above[c] * 2 > total[c]}
        # the face's shade/outline: dark warm colors right under the face
        for d in 'SWE':
            f = self.fr[d]
            for y in range(ey, ey + 4):
                for x in range(f.shape[1]):
                    c = px(f, y, x)
                    if c and c not in skin:
                        h, s, v = hsv(c)
                        nb = [px(f, y + dy, x + dx) for dy, dx in ((0, 1), (0, -1), (-1, 0), (1, 0))]
                        if (h <= 0.06 or h >= 0.97) and 0.25 <= s <= 0.7 and 0.3 <= v <= 0.6 and any(n in skin for n in nb) \
                                and c not in seed and c not in self.mostly_above:
                            skin.add(c)
        for c in self.opts.get('skin', []):
            skin.add(c)
        # the standard ramps anywhere (ears, neck, hands) unless the top of the head is made of them
        top = set(seed)
        for d in 'SWEN':
            f = self.fr[d]
            for y, x in zip(*np.nonzero(f[..., 3])):
                c = px(f, y, x)
                if c in STD_SKIN and c not in top:
                    skin.add(c)
        self.skin = skin
        # Hair: the top rows (less any skin: a bald top, a forehead)
        for c in list(seed):
            if c in skin:
                del seed[c]
        for c in self.opts.get('hair', []):
            seed[c] += 100
        hair = set(seed)
        if seed:
            top = seed.most_common(1)[0][0]
            th, ts, tv = hsv(top)
        # Grow: colors in the head that sit next to hair and share its hue
        changed = True
        while changed:
            changed = False
            adj = Counter()
            for d in 'SWEN':
                f = self.fr[d]
                for y in range(0, min(f.shape[0], ey + 3)):
                    for x in range(f.shape[1]):
                        c = px(f, y, x)
                        if not c or c in hair or c in skin or lum(c) < 0.06:
                            continue
                        if any(px(f, y + dy, x + dx) in hair for dy, dx in ((0, 1), (0, -1), (-1, 0), (1, 0))):
                            adj[c] += 1
            for c, n in adj.items():
                h, s, v = hsv(c)
                if ts < 0.22:   # black, grey or white hair: only other greys
                    same_hue = s < 0.3
                else:
                    same_hue = min(abs(h - th), 1 - abs(h - th)) < 0.09
                light_skin = skinish(c) and v > 0.75 and s < 0.5
                if same_hue and not light_skin and n >= 2 and c not in self.opts.get('nothair', []):
                    hair.add(c)
                    changed = True
        for c in self.opts.get('nothair', []):
            hair.discard(c)
        self.hair = hair
        # Accessories: other colors in the head above the eyes, touching the hair
        acc = set(self.opts.get('acc', []))
        if not self.opts.get('noacc'):
            for d in 'SWEN':
                f = self.fr[d]
                for y in range(0, ey):
                    for x in range(f.shape[1]):
                        c = px(f, y, x)
                        if c and c not in hair and c not in skin and lum(c) >= 0.06:
                            if any(px(f, y + dy, x + dx) in hair for dy, dx in ((0, 1), (0, -1), (-1, 0), (1, 0))):
                                acc.add(c)
        self.acc = acc - hair - skin

    def cut(self, d):
        """The hair of one direction: RGBA in the donor's frame (original colors)."""
        if d in self.opts.get('copy', {}):
            return self.cut(self.opts['copy'][d])
        f = self.fr[d]
        h, w = f.shape[:2]
        head = self.ey + 3
        maxrow = self.opts.get('long', {}).get(d, self.opts.get('long', {}).get('*', h - 1))
        keep = np.zeros((h, w), bool)
        for y in range(min(h, head + 1)):
            for x in range(w):
                c = px(f, y, x)
                if c in self.hair or (c in self.acc and y < self.ey):
                    keep[y, x] = True
        # Long hair: follow it down from the head, column by column
        for y in range(head + 1, min(h, maxrow + 1)):
            for x in range(w):
                if px(f, y, x) in self.hair and any(0 <= x + dx < w and keep[y - 1, x + dx] for dx in (-1, 0, 1)):
                    keep[y, x] = True
        # drop thin strays below the head (a collar or strap in the hair's color)
        for _ in range(2):
            thin = []
            for y in range(head + 1, h):
                for x in range(w):
                    if keep[y, x]:
                        n = sum(keep[y + dy, x + dx] for dy in (-1, 0, 1) for dx in (-1, 0, 1)
                                if (dy or dx) and 0 <= y + dy < h and 0 <= x + dx < w)
                        if n < 2:
                            thin.append((y, x))
            for y, x in thin:
                keep[y, x] = False
        # the eyes stay the kit's
        if d in self.eyes and self.eyes[d]:
            ey, exs = self.eyes[d]
            for x in exs:
                keep[ey, x] = keep[ey + 1, x] = False
        # outline around the hair (not the face's)
        out = keep.copy()
        for y in range(min(h, maxrow + 2)):
            for x in range(w):
                c = px(f, y, x)
                if c is None or keep[y, x] or lum(c) >= 0.06:
                    continue
                nb = [(y + dy, x + dx) for dy, dx in ((0, 1), (0, -1), (-1, 0), (1, 0))]
                near_hair = any(0 <= yy < h and 0 <= xx < w and keep[yy, xx] for yy, xx in nb)
                near_skin = any(px(f, yy, xx) in self.skin for yy, xx in nb)
                if near_hair and not near_skin:
                    out[y, x] = True
        # only what hangs together with the top of the head
        lab = np.zeros((h, w), int)
        comp = 0
        sizes = {}
        tops = {}
        for y in range(h):
            for x in range(w):
                if out[y, x] and not lab[y, x]:
                    comp += 1
                    stack = [(y, x)]
                    lab[y, x] = comp
                    n = 0
                    top = y
                    while stack:
                        yy, xx = stack.pop()
                        n += 1
                        top = min(top, yy)
                        for dy in (-1, 0, 1):
                            for dx in (-1, 0, 1):
                                Y, X = yy + dy, xx + dx
                                if 0 <= Y < h and 0 <= X < w and out[Y, X] and not lab[Y, X]:
                                    lab[Y, X] = comp
                                    stack.append((Y, X))
                    sizes[comp] = n
                    tops[comp] = top
        if not sizes:
            return np.zeros((h, w, 4), np.uint8)
        main = min(sizes, key=lambda k: (tops[k], -sizes[k]))
        res = np.zeros((h, w, 4), np.uint8)
        for c in sizes:
            if c == main or sizes[c] >= 3 and tops[c] < self.ey:
                res[lab == c] = f[lab == c]
        # loose pixels at the edges (a hair tip cut off from the rest, a collar corner)
        for _ in range(2):
            a = res[..., 3] > 0
            lone = [(y, x) for y, x in zip(*np.nonzero(a))
                    if sum(a[y + dy, x + dx] for dy in (-1, 0, 1) for dx in (-1, 0, 1)
                           if (dy or dx) and 0 <= y + dy < h and 0 <= x + dx < w) <= 1]
            for y, x in lone:
                res[y, x] = 0
        return res

    def offset(self, d):
        """dx, dy that put the donor frame on the kit head cell.

        Every direction moves by the same amount (from the front view's eyes), so the
        hair keeps the donor sprite's own layout when turning; side-view eyes are too
        easy to mistake for a nose or an outline to line up on. The kit draws its head
        cell lower in some views (kit3.head_offset), which is taken back out here so the
        hair doesn't sink when the player turns sideways or away. The side and back
        views' tops are pinned to the front view's top, so the tip never moves."""
        ey, ex = self.eyes['S']
        dx = KIT_EYE_X['S'] - (ex[0] + ex[-1]) / 2
        dy = KIT_EYE_ROW - ey - (head_offset('walk', d, 1)[1] - head_offset('walk', 'S', 1)[1])
        dx += self.opts.get('dx', {}).get(d, 0)
        dy += self.opts.get('dy', {}).get(d, 0)
        if d != 'S':
            yd = np.nonzero(self.cut(d)[..., 3])[0]
            ys = np.nonzero(self.cut('S')[..., 3])[0]
            if len(yd) and len(ys):
                place = head_offset('walk', d, 1)[1] - head_offset('walk', 'S', 1)[1]
                dy = int(ys.min()) + self.offset('S')[1] - place - int(yd.min()) + self.opts.get('dy', {}).get(d, 0)
        return int(round(dx)), int(dy)

    def marker_map(self):
        """Donor color -> kit marker color."""
        m = {}
        hair = sorted(self.hair, key=lum, reverse=True)
        # the most used shade becomes the kit's base (#8080f0)
        cnt = Counter()
        for d in 'SWEN':
            cut = self.cut(d)
            for c in map(tuple, cut[cut[..., 3] > 0][:, :3].tolist()):
                cnt[c] += 1
        used = [c for c in hair if cnt[c]]
        if used:
            # the most used shade is the kit's base (#8080f0); the others by how much lighter or
            # darker they are, keeping at least one step apart from the base in each direction
            base = max(used, key=lambda c: cnt[c])
            bl = max(lum(base), 0.02)
            for c in used:
                r = lum(c) / bl
                if c == base:
                    m[c] = HAIR_MARK[1]
                elif lum(c) > lum(base):
                    m[c] = HAIR_MARK[0] if r > 1.12 else HAIR_MARK[1]
                else:
                    m[c] = HAIR_MARK[3] if r < 0.5 else HAIR_MARK[2] if r < 0.9 else HAIR_MARK[1]
            # a base that is the darkest shade: spread the lighter ones over base / light
            lighter = sorted((c for c in used if lum(c) > lum(base)), key=lum)
            if lighter and all(m[c] == HAIR_MARK[0] for c in lighter) and len(lighter) >= 2:
                for c in lighter[:len(lighter) // 2]:
                    m[c] = HAIR_MARK[1]
                m[base] = HAIR_MARK[2]
        # Accessories: one or two color families, light .. dark
        acc = sorted((c for c in self.acc if cnt[c]), key=lambda c: hsv(c)[0])
        fams = []
        for c in acc:
            for fam in fams:
                h0 = hsv(fam[0])[0]
                if min(abs(hsv(c)[0] - h0), 1 - abs(hsv(c)[0] - h0)) < 0.08 or (hsv(c)[1] < 0.2 and hsv(fam[0])[1] < 0.2):
                    fam.append(c)
                    break
            else:
                fams.append([c])
        fams.sort(key=lambda fam: -sum(cnt[c] for c in fam))
        self.nacc = 0
        for k, fam in enumerate(fams):
            if k >= 2 or self.opts.get('acc_fixed'):
                for c in fam:
                    m[c] = c   # a fixed color
                continue
            self.nacc = k + 1
            fam.sort(key=lum, reverse=True)
            for i, c in enumerate(fam):
                m[c] = ACC_MARK[k][min(3, i * 4 // max(1, len(fam)) if len(fam) > 1 else 1)]
        return m

    def sheet(self, recolor=True):
        """100x20 head sheet (S, W, E, N, S again for the run's tilted head), with MARGIN
        spare rows above and below."""
        m = self.marker_map() if recolor else {}
        out = np.zeros((20 + 2 * MARGIN, 100, 4), np.uint8)
        for i, d in enumerate('SWENS'):
            cut = self.cut(d)
            dx, dy = self.offset(d)
            for y, x in zip(*np.nonzero(cut[..., 3])):
                Y, X = y + dy + MARGIN, x + dx
                if 0 <= Y < 20 + 2 * MARGIN and 0 <= X < 20:
                    c = tuple(int(v) for v in cut[y, x, :3])
                    if lum(c) < 0.06:
                        c = (0, 0, 0)
                    out[Y, i * 20 + X, :3] = m.get(c, c)
                    out[Y, i * 20 + X, 3] = 255
        return out


def read_donors():
    """hair_donors.txt: FILE NAME [key=value ...]  (colors as hex, lists with ',')."""
    out = []
    for line in open(DONORS):
        line = line.split('#', 1)[0].strip()
        if not line:
            continue
        parts = line.split()
        file, name, opts = parts[0], parts[1], {}
        for kv in parts[2:]:
            k, _, v = kv.partition('=')
            if k in ('skin', 'hair', 'nothair', 'acc'):
                opts[k] = [tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for h in v.split(',')]
            elif k in ('long', 'dx', 'dy'):
                # long=S:16,N:20 or long=18 (every direction)
                dd = {}
                for item in v.split(','):
                    if ':' in item:
                        a, b = item.split(':')
                        dd[a] = int(b)
                    else:
                        dd['*'] = int(item)
                if k != 'long' and '*' in dd:
                    dd = {d: dd['*'] for d in 'SWEN'}
                opts[k] = dd
            elif k == 'eyes':
                # eyes=S:12:6:9,W:12:5  (direction:row:x...)
                opts[k] = {}
                for item in v.split(','):
                    t = item.split(':')
                    opts[k][t[0]] = (int(t[1]), [int(x) for x in t[2:]])
            elif k == 'copy':
                # copy=N:S  (a view with no usable hair borrows another view's cut)
                opts[k] = dict(item.split(':') for item in v.split(','))
            elif k in ('noacc', 'acc_fixed'):
                opts[k] = True
        out.append((file, name, opts))
    return out


def find_file(img_dir, stem):
    for f in sorted(os.listdir(img_dir)):
        if f == stem or f.startswith(stem + '.') or f.startswith(stem + '_') or re.sub(r'^\d+_', '', f).rsplit('.', 1)[0] == stem:
            return f
    return None


def load(img_dir):
    """[(name, nacc, sheet100x20)] for build_ow.py."""
    res = []
    for file, name, opts in read_donors():
        f = find_file(img_dir, file)
        if not f:
            print('donor_hair: no file for', file, file=sys.stderr)
            continue
        d = Donor(img_dir, f, name, opts)
        if not d.ok:
            print('donor_hair: no eyes found in', file, file=sys.stderr)
            continue
        sh = d.sheet()
        res.append((name, d.nacc, sh))
    return res


def review(img_dir, kit_dir, out, names):
    """Rows: the donor's 4 standing frames, the cut hair, the hair on the kit head (+ body)."""
    def L(p):
        return np.array(Image.open(p).convert('RGBA'))
    head = L(os.path.join(kit_dir, 'bases/head/standard-head.png'))
    eyes = L(os.path.join(kit_dir, 'bases/eyes/default.png'))
    body = L(os.path.join(kit_dir, 'bases/body/default-female.png'))

    def comp(dst, src, x=0, y=0):
        h, w = src.shape[:2]
        m = src[..., 3] > 0
        sub = dst[y:y + h, x:x + w]
        sub[m] = src[m]
    rows = []
    donors = read_donors()
    if names:
        donors = [t for t in donors if t[1] in names or t[0] in names]
    for file, name, opts in donors:
        f = find_file(img_dir, file)
        if not f:
            continue
        d = Donor(img_dir, f, name, opts)
        row = np.zeros((40, 4 * 24 + 4 * 24 + 4 * 34, 4), np.uint8)
        for i, dd in enumerate('SWEN'):
            comp(row, d.fr[dd], i * 24 + 4, 8)
        if d.ok:
            sh = d.sheet(recolor=False)
            for i, dd in enumerate('SWEN'):
                comp(row, d.cut(dd), 96 + i * 24 + 4, 8)
                # kit: body frame (standing), head, eyes, then the hair
                cell = np.zeros((40, 34, 4), np.uint8)
                r = 'SWEN'.index(dd)
                comp(cell, body[r * 40:(r + 1) * 40, 40:80][0:40, 3:37])
                hc = head[:, i * 20:(i + 1) * 20]
                hy = 4 + (2 if dd == 'S' else 4 if dd == 'N' else 3) - 4
                comp(cell, hc, 10 - 3, 10 + hy)
                if i < 4:
                    comp(cell, eyes[:, i * 20:(i + 1) * 20], 10 - 3, 10 + hy)
                comp(cell, sh[:, i * 20:(i + 1) * 20], 10 - 3, 10 + hy - MARGIN)
                comp(row, cell, 192 + i * 34, 0)
        rows.append(row)
    a = np.concatenate(rows, 0)
    bg = Image.new('RGBA', (a.shape[1], a.shape[0]), (120, 180, 120, 255))
    bg.alpha_composite(Image.fromarray(a))
    bg = bg.resize((a.shape[1] * 6, a.shape[0] * 6), Image.NEAREST)
    bg.save(out)


def fetch(out_dir):
    """The gallery through DeviantArt's RSS feed, saved as NNN_Title.png in gallery order."""
    import html
    import urllib.request
    os.makedirs(out_dir, exist_ok=True)
    feed = 'https://backend.deviantart.com/rss.xml?type=deviation&q=gallery%3Aaveontrainer%2F67900303&offset='
    n, off = 0, 0
    while True:
        req = urllib.request.Request(feed + str(off), headers={'User-Agent': 'Mozilla/5.0'})
        xml = urllib.request.urlopen(req).read().decode('utf-8', 'replace')
        items = re.findall(r'<item>(.*?)</item>', xml, re.S)
        for it in items:
            title = html.unescape(re.search(r'<title>(.*?)</title>', it, re.S).group(1))
            img = re.search(r'<media:content url="([^"]+)"', it)
            if img:
                name = '%03d_%s.png' % (n, re.sub(r'[^A-Za-z0-9]+', '_', title).strip('_'))
                req = urllib.request.Request(html.unescape(img.group(1)), headers={'User-Agent': 'Mozilla/5.0'})
                open(os.path.join(out_dir, name), 'wb').write(urllib.request.urlopen(req).read())
            n += 1
        if len(items) < 60:
            break
        off += 60
    print('fetched', n)


if __name__ == '__main__':
    if sys.argv[1] == '--fetch':
        fetch(sys.argv[2])
    else:
        review(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:])
