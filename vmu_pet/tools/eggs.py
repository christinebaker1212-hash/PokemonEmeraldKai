#!/usr/bin/env python3
"""Moemon VMU pet: the Eggs, a look for each MOeMON inside (the user's style guide, docs/user_images/96.png: about
18x20, outlined in a dark shade of their own colour, a pattern on each: flames, bands, roses, stars...).

An Egg is drawn from three things:
  its shape     the guide's own silhouette (19x20, leaning a little), and the same scaled up for the colour file icon
  its pattern   one of 8 kinds by the MOeMON's first type, in one of 2 phases and maybe mirrored (by its second type
                and its number), so a line of the same type doesn't all look the same
  its colours   from the MOeMON's PMD sprite: the body's colour for the shell, the next one for the pattern, the
                outline a dark shade of the pattern's
The LCD (1 bit) draws the outline and the pattern; the colour icon draws 4 colours (clear, outline, shell, pattern).

usage: eggs.py PREVIEW.png     a sheet of every KANTO Egg, colour and LCD
"""
import colorsys
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, '..', '..')

# The guide's first Egg (docs/user_images/96.png, x 0-20): '#' inside
GUIDE = """
.....######........
...##########......
..############.....
..#############....
.###############...
.################..
#################..
##################.
##################.
###################
###################
###################
###################
##################.
##################.
.#################.
..###############..
..##############...
...############....
.....#########.....
""".split()

KINDS = ['FLAMES', 'WAVES', 'ROSES', 'STARS', 'BANDS', 'SPOTS', 'ZIGZAG', 'BLOTCHES']
KIND_OF_TYPE = {'FIRE': 0, 'DRAGON': 0, 'WATER': 1, 'ICE': 1, 'GRASS': 2, 'BUG': 2, 'PSYCHIC': 3, 'FAIRY': 3,
                'GROUND': 4, 'ROCK': 4, 'FIGHTING': 4, 'STEEL': 4, 'NORMAL': 5, 'ELECTRIC': 6, 'FLYING': 6,
                'POISON': 7, 'GHOST': 7, 'DARK': 7}
TYPES = ['NORMAL', 'FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', 'MYSTERY', 'FIRE',
         'WATER', 'GRASS', 'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK', 'FAIRY']

LCD_W, LCD_H = 19, 20           # in 24 columns: 2 left, 3 right
ICON_W, ICON_H = 27, 28         # in 32 columns: 2 left, 3 right; rows 2-29


def shape(w, h):
    """The guide's silhouette at w x h (the guide itself at 19x20; larger by sampling it smoothly)."""
    gw, gh = len(GUIDE[0]), len(GUIDE)
    g = [[1.0 if c == '#' else 0.0 for c in row] for row in GUIDE]

    def at(x, y):
        x = min(max(x, 0), gw - 1)
        y = min(max(y, 0), gh - 1)
        return g[y][x]

    out = []
    for y in range(h):
        row = []
        for x in range(w):
            if (w, h) == (gw, gh):
                row.append(g[y][x] > 0.5)
                continue
            fx = (x + 0.5) * gw / w - 0.5
            fy = (y + 0.5) * gh / h - 0.5
            x0, y0 = math.floor(fx), math.floor(fy)
            ax, ay = fx - x0, fy - y0
            v = (at(x0, y0) * (1 - ax) * (1 - ay) + at(x0 + 1, y0) * ax * (1 - ay) +
                 at(x0, y0 + 1) * (1 - ax) * ay + at(x0 + 1, y0 + 1) * ax * ay)
            row.append(v >= 0.5)
        out.append(row)
    return out


def outline_of(sh):
    h, w = len(sh), len(sh[0])
    out = []
    for y in range(h):
        row = []
        for x in range(w):
            e = False
            if sh[y][x]:
                for ax, ay in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                    if not (0 <= ax < w and 0 <= ay < h) or not sh[ay][ax]:
                        e = True
            row.append(e)
        out.append(row)
    return out


def star(u, v, cx, cy, r):
    """A five-pointed star of radius r at (cx, cy) (u, v in the Egg's box, 0-1, v scaled to look round)."""
    dx, dy = u - cx, (v - cy) * 1.05
    d = math.hypot(dx, dy)
    if d > r:
        return False
    a = math.atan2(dy, dx) + math.pi / 2
    k = (a % (2 * math.pi / 5)) / (2 * math.pi / 5)
    edge = r * (0.45 + 0.55 * abs(1 - 2 * k) ** 1.6)
    return d <= edge


def rose(u, v, cx, cy, r):
    """A rose: a spiral line round (cx, cy)."""
    dx, dy = u - cx, v - cy
    d = math.hypot(dx, dy)
    if d > r or d < r * 0.12:
        return False
    a = math.atan2(dy, dx)
    turns = 1.25
    s = (d / r) * turns - (a / (2 * math.pi))
    return abs(s - round(s)) < 0.24


def pattern(kind, phase, u, v):
    """True where the pattern is (u across, v down, 0-1 in the Egg's box). phase 0 / 1 moves it."""
    p = phase
    if kind == 0:   # flames rising from below
        base = 0.74 - 0.08 * p
        tips = base - 0.18 * (0.5 + 0.5 * math.cos((u * 3.2 + 0.35 * p) * 2 * math.pi))
        lick = 0.06 * math.sin((u * 7.0 + p) * 2 * math.pi)
        return v > tips + lick
    if kind == 1:   # waves: two wavy lines
        out = False
        for c in ((0.40, 0.70) if p == 0 else (0.33, 0.60, 0.86)):
            y = c + 0.05 * math.sin((u * 2.2 + c) * 2 * math.pi)
            out = out or abs(v - y) < 0.045
        return out
    if kind == 2:   # roses
        spots = ((0.66, 0.30, 0.21), (0.38, 0.70, 0.23)) if p == 0 else ((0.32, 0.34, 0.21), (0.64, 0.70, 0.23))
        return any(rose(u, v, cx, cy, r) for cx, cy, r in spots)
    if kind == 3:   # stars
        spots = ((0.66, 0.36, 0.20), (0.24, 0.70, 0.17)) if p == 0 else ((0.34, 0.32, 0.18), (0.70, 0.70, 0.20))
        return any(star(u, v, cx, cy, r) for cx, cy, r in spots)
    if kind == 4:   # bands across, rough-edged
        out = False
        for c in ((0.33, 0.66) if p == 0 else (0.48, 0.80)):
            y = c + 0.07 * math.sin((u * 1.3 + c * 2) * 2 * math.pi) + 0.03 * math.sin(u * 23)
            out = out or abs(v - y) < 0.075
        return out
    if kind == 5:   # spots
        spots = ((0.30, 0.30, 0.10), (0.68, 0.42, 0.13), (0.36, 0.72, 0.12), (0.76, 0.78, 0.07)) if p == 0 else \
                ((0.62, 0.24, 0.09), (0.30, 0.50, 0.13), (0.70, 0.66, 0.12), (0.40, 0.86, 0.07))
        return any((u - cx) ** 2 + ((v - cy) * 1.05) ** 2 < r * r for cx, cy, r in spots)
    if kind == 6:   # a zigzag band
        out = False
        for c in ((0.52,) if p == 0 else (0.36, 0.74)):
            t = (u * (3.0 if p == 0 else 3.5)) % 1.0
            y = c + 0.11 * (abs(2 * t - 1) * 2 - 1)
            out = out or abs(v - y) < (0.07 if p == 0 else 0.055)
        return out
    if kind == 7:   # blotches: soft blobs with a lobe
        blobs = ((0.30, 0.34, 0.15), (0.40, 0.44, 0.10), (0.70, 0.72, 0.17)) if p == 0 else \
                ((0.70, 0.30, 0.14), (0.60, 0.40, 0.09), (0.32, 0.72, 0.16))
        f = sum(r * r / ((u - cx) ** 2 + (v - cy) ** 2 + 1e-6) for cx, cy, r in blobs)
        return f > 1.0
    return False


def masks(w, h, kind, phase, mirror=False):
    """(outline, inside, pattern) as lists of rows of booleans, w x h."""
    sh = shape(w, h)
    ol = outline_of(sh)
    pat = []
    for y in range(h):
        row = []
        for x in range(w):
            u, v = (x + 0.5) / w, (y + 0.5) / h
            row.append(sh[y][x] and not ol[y][x] and pattern(kind, phase, u, v))
        pat.append(row)
    if mirror:
        sh = [r[::-1] for r in sh]
        ol = [r[::-1] for r in ol]
        pat = [r[::-1] for r in pat]
    return ol, sh, pat


def design(dex, types):
    """(kind, phase, mirror) for a MOeMON: the kind by its first type, the rest by its second type and number."""
    kind = KIND_OF_TYPE.get(types[0], 5)
    second = TYPES.index(types[1]) if types[1] in TYPES else 0
    phase = (dex + second) % 2
    mirror = ((dex * 7 + second * 3) // 2) % 2 == 1
    return kind, phase, mirror


# ---- colours

def to4444(rgb):
    """An RGB colour to the VMS icon's ARGB4444 (opaque), and back to the RGB it shows."""
    r, g, b = (min(15, (c + 8) // 17) for c in rgb)
    return 0xF000 | (r << 8) | (g << 4) | b, (r * 17, g * 17, b * 17)


def body_colours(dex):
    """The body's main colour and the next distinct one, from the MOeMON's PMD sprite (only its own pixels: the
    sprite has no background), weighting the bright, coloured pixels over the outlines and shadows."""
    sys.path.insert(0, HERE)
    from pmd_read import frame
    got = frame(dex, 0, 0, 0)
    if got is None:
        return (200, 200, 200), None
    m, rgb = got
    px = [tuple(int(v) for v in rgb[y, x]) for y in range(m.shape[0]) for x in range(m.shape[1]) if m[y, x]]
    buckets = {}
    for c in px:
        h, l, s = colorsys.rgb_to_hls(*(x / 255 for x in c))
        if l < 0.24 or (l < 0.34 and s < 0.25):      # outlines and shadows
            continue
        key = ('h', round(h * 14) % 14) if s > 0.16 else ('g', round(l * 3))
        buckets.setdefault(key, []).append(c)
    ranked = sorted(buckets.values(), key=len, reverse=True)
    mean = lambda cs: tuple(sum(c[i] for c in cs) // len(cs) for i in range(3))
    first = mean(ranked[0]) if ranked else (200, 200, 200)
    second = None
    for cs in ranked[1:]:
        mc = mean(cs)
        h1, l1, s1 = colorsys.rgb_to_hls(*(x / 255 for x in first))
        h2, l2, s2 = colorsys.rgb_to_hls(*(x / 255 for x in mc))
        dh = min(abs(h1 - h2), 1 - abs(h1 - h2))
        if len(cs) >= max(6, len(ranked[0]) // 7) and (dh > 0.08 and max(s1, s2) > 0.2 or abs(l1 - l2) > 0.25):
            second = mc
            break
    return first, second


def shade(rgb, light=None, sat=None, mul_l=None):
    h, l, s = colorsys.rgb_to_hls(*(x / 255 for x in rgb))
    if mul_l is not None:
        l *= mul_l
    if light is not None:
        l = light
    if sat is not None:
        s = sat
    return tuple(int(round(c * 255)) for c in colorsys.hls_to_rgb(h, max(0, min(1, l)), max(0, min(1, s))))


def palette(first, second):
    """(outline, shell, pattern) as RGB, after the guide: a light shell, a mid pattern, the outline a dark shade of the
    pattern's colour."""
    h1, l1, s1 = colorsys.rgb_to_hls(*(x / 255 for x in first))
    shell = shade(first, light=min(0.80, max(0.62, l1 + 0.12)), sat=min(s1, 0.55))
    if second is None:
        pat = shade(first, light=max(0.36, min(0.52, l1 - 0.12)), sat=min(1.0, s1 * 1.1 + 0.05))
    else:
        h2, l2, s2 = colorsys.rgb_to_hls(*(x / 255 for x in second))
        pat = shade(second, light=max(0.30, min(0.55, l2)), sat=min(0.85, s2 + 0.05))
        # keep the shell lighter than the pattern
        if colorsys.rgb_to_hls(*(x / 255 for x in pat))[1] > colorsys.rgb_to_hls(*(x / 255 for x in shell))[1] - 0.15:
            shell, pat = shade(second, light=0.74, sat=min(s2, 0.55)), shade(first, light=0.42, sat=min(0.85, s1 + .05))
    outline = shade(pat, light=0.22, sat=min(0.75, colorsys.rgb_to_hls(*(x / 255 for x in pat))[2] + 0.1))
    return outline, shell, pat


def egg_colours(dex):
    first, second = body_colours(dex)
    return [to4444(c)[1] for c in palette(first, second)]


# ---- the preview

def preview(out):
    sys.path.insert(0, HERE)
    import gen_edition as ge
    from PIL import Image
    nums = ge.species_numbers()
    info, evo = ge.infos(nums), ge.evolutions(nums)
    dex = list(range(1, 152))
    targets = {t for d in dex for k, a, t in evo.get(d, []) if t in dex}
    noegg = lambda g: 'UNDISCOVERED' in g or 'NO_EGGS' in g or g == 'DITTO'
    hatch = [d for d in dex if d not in targets and not any(noegg(g) for g in info[d]['eggs'])]
    Z, cols = 3, 10
    cw, ch = 32 + 24 + 6, 32
    img = Image.new('RGB', (cols * cw * Z, ((len(hatch) + cols - 1) // cols) * (ch + 4) * Z), (226, 222, 210))
    for i, d in enumerate(hatch):
        kind, phase, mirror = design(d, info[d]['types'])
        outline, shell, pat = egg_colours(d)
        ox, oy = (i % cols) * cw * Z, (i // cols) * (ch + 4) * Z
        ol, sh, pt = masks(ICON_W, ICON_H, kind, phase, mirror)
        for y in range(ICON_H):
            for x in range(ICON_W):
                c = outline if ol[y][x] else pat if pt[y][x] else shell if sh[y][x] else None
                if c:
                    img.paste(c, (ox + (x + 2) * Z, oy + (y + 2) * Z, ox + (x + 3) * Z, oy + (y + 3) * Z))
        ol, sh, pt = masks(LCD_W, LCD_H, kind, phase, mirror)
        lx = ox + 34 * Z
        img.paste((140, 156, 120), (lx, oy + 6 * Z, lx + 24 * Z, oy + 30 * Z))
        for y in range(LCD_H):
            for x in range(LCD_W):
                if ol[y][x] or pt[y][x]:
                    img.paste((30, 40, 50), (lx + (x + 2) * Z, oy + (y + 8) * Z, lx + (x + 3) * Z, oy + (y + 9) * Z))
    img.save(out)
    print('%d Eggs' % len(hatch))


if __name__ == '__main__':
    preview(sys.argv[1])
