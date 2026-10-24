#!/usr/bin/env python3
"""Moemon Co-op: first guesses of the recolor maps (tools/moemon_frlg/colormaps.txt) for new sprites.

Usage: suggest_maps.py DUMP_DIR PREFIX [--score]
  DUMP_DIR  from the test harness command "mp dumpcolors DIR"
  PREFIX    only assets whose ini name starts with it (kai_)
  --score   instead: compare the guesses with the maps colormaps.txt already has (all of them)

Prints colormaps.txt lines. They are guesses (skin by color and place, hair from the top of the head,
outfit and accent the two biggest groups of the rest): check every one with colorcheck.py and fix it.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'moemon_frlg'))
from colorsheets import load  # noqa: E402


def hsv(rgb):
    r, g, b = rgb
    mx, mn = max(r, g, b), min(r, g, b)
    d = mx - mn
    s = d * 255 // mx if mx else 0
    if d == 0:
        h = -1
    elif mx == r:
        h = (60 * (g - b) // d + 360) % 360
    elif mx == g:
        h = 60 * (b - r) // d + 120
    else:
        h = 60 * (r - g) // d + 240
    return h, s, mx


def hue_dist(a, b):
    d = abs(a - b) % 360
    return min(d, 360 - d)


def skin_like(c):
    h, s, v = hsv(c)
    if h < 0:
        return False
    warm = h <= 48 or h >= 345
    return warm and ((30 <= s <= 175 and v >= 130) or (60 <= s <= 215 and 70 <= v < 130))


def counts(frame, box=None):
    out = {}
    hgt, wid = frame.shape
    y0, y1, x0, x1 = box if box else (0, hgt, 0, wid)
    for y in range(max(0, y0), min(hgt, y1)):
        for x in range(max(0, x0), min(wid, x1)):
            v = int(frame[y, x])
            if v:
                out[v] = out.get(v, 0) + 1
    return out


def opaque_rows(frame):
    rows = [y for y in range(frame.shape[0]) if frame[y].any()]
    return (rows[0], rows[-1] + 1) if rows else (0, 0)


def groups(idxs, cols):
    """Colors grouped by hue (grays together), biggest first by the given weight"""
    out = []
    for i in idxs:
        h, s, v = hsv(cols[i])
        gray = h < 0 or s < 50
        for g in out:
            h2, s2, v2 = hsv(cols[g[0]])
            gray2 = h2 < 0 or s2 < 50
            if (gray and gray2) or (not gray and not gray2 and hue_dist(h, h2) < 30):
                g.append(i)
                break
        else:
            out.append([i])
    return out


def suggest(kind, frames, cols):
    f0 = frames[0]
    hgt, wid = f0.shape
    total = {}
    for f in frames:
        for k, v in counts(f).items():
            total[k] = total.get(k, 0) + v
    used = [i for i in range(1, 16) if total.get(i)]
    m = ['.'] * 15
    # Outlines: near black, or a very dark color without much hue (dark hair keeps its hue and is mapped)
    dark = {i for i in used if hsv(cols[i])[2] < 40 or (hsv(cols[i])[2] < 72 and hsv(cols[i])[1] < 90)}
    # The outline is the most used of those (or the darkest color): never mapped, even when it has a hue
    dark.add(min(used, key=lambda i: (hsv(cols[i])[2], -total[i])))
    top, bottom = opaque_rows(f0)
    tall = bottom - top
    # Where the face is: walking sprites have it in fixed rows; pictures in the upper part
    if kind == 'ow':
        # (the sprites stand on the frame's bottom: the head starts where the first pixels are)
        face = (top + 7, top + 13, 3, wid - 3)
        head = frames[1] if len(frames) > 1 else f0
        hair_box = (top, top + 9, 0, wid)
        body_box = (top + 12, hgt, 0, wid)
        torso_box = (top + 12, top + 17, 2, wid - 2)
        legs_box = (top + 17, hgt, 0, wid)
    elif kind == 'front':
        face = (top + tall * 8 // 100, top + tall * 35 // 100, wid // 5, wid * 4 // 5)
        head = f0
        hair_box = (top, top + max(4, tall * 12 // 100), 0, wid)
        body_box = (top + tall * 35 // 100, bottom, 0, wid)
        torso_box = (top + tall * 35 // 100, top + tall * 65 // 100, 0, wid)
        legs_box = (top + tall * 65 // 100, bottom, 0, wid)
    else:   # back: seen from behind, the head is hair; skin shows on the neck and arms
        face = (top, bottom, 0, wid)
        head = f0
        hair_box = (top, top + max(4, tall * 18 // 100), 0, wid)
        body_box = (top + tall * 30 // 100, bottom, 0, wid)
        torso_box = (top + tall * 30 // 100, top + tall * 70 // 100, 0, wid)
        legs_box = (top + tall * 70 // 100, bottom, 0, wid)
    fc = counts(f0, face)
    # The skin: the face's main skin-like color (the middle of the face; light skin first, a strong red is
    # clothing) and up to 3 shades of it (same hue, a similar saturation)
    fy0, fy1, fx0, fx1 = face
    mid = counts(f0, (fy0 + (fy1 - fy0) // 8, fy1 - (fy1 - fy0) // 4, fx0 + (fx1 - fx0) // 4, fx1 - (fx1 - fx0) // 4))

    def main_score(i):
        h, s, v = hsv(cols[i])
        return mid.get(i, 0) * v

    face_skin = sorted((i for i in used if i not in dark and skin_like(cols[i]) and fc.get(i, 0) >= (2 if kind == 'ow' else 8)
                        and not (hsv(cols[i])[1] > 150 and hsv(cols[i])[2] >= 170) and mid.get(i, 0)),
                       key=lambda i: -main_score(i))
    skin = []
    if face_skin:
        h0, s0, v0 = hsv(cols[face_skin[0]])
        near = [i for i in used if i not in dark and skin_like(cols[i])
                and hue_dist(hsv(cols[i])[0], h0) <= 14 and abs(hsv(cols[i])[1] - s0) <= 75 and hsv(cols[i])[2] >= v0 * 50 // 100]
        near.sort(key=lambda i: -(fc.get(i, 0) * 4 + total[i]))
        skin = near[:4]
    for i in skin:
        m[i - 1] = 'S'
    rest = [i for i in used if i not in dark and i not in skin]
    # Hair: what fills the top of the head (a hat counts: it is what HAIR recolors then)
    hc = counts(head, hair_box)
    cand = sorted((i for i in rest if hc.get(i, 0)), key=lambda i: -hc[i])
    hair = []
    if cand and hc[cand[0]] >= (3 if kind == 'ow' else 12):
        h0, s0, v0 = hsv(cols[cand[0]])
        for i in rest:
            h, s, v = hsv(cols[i])
            same = (h0 < 0 or s0 < 50) and (h < 0 or s < 50) or (h0 >= 0 and s0 >= 50 and h >= 0 and s >= 50 and hue_dist(h, h0) < 25)
            if same and hc.get(i, 0) + counts(head, (hair_box[0], hair_box[1] + (3 if kind == 'ow' else 6), 0, wid)).get(i, 0) > 0:
                hair.append(i)
    for i in hair:
        m[i - 1] = 'H'
    rest = [i for i in rest if i not in hair]
    # Outfit: the torso's main garment; accent: the legs' (or the torso's second) garment
    tc, lc = counts(f0, torso_box), counts(f0, legs_box)
    minimum = 3 if kind == 'ow' else 30
    gs = groups([i for i in rest if tc.get(i, 0) + lc.get(i, 0)], cols)
    tw = lambda g: sum(tc.get(i, 0) for i in g)
    lw = lambda g: sum(lc.get(i, 0) for i in g)
    gs = [g for g in gs if tw(g) + lw(g) >= minimum]
    outfit = max(gs, key=tw) if gs and max(tw(g) for g in gs) >= minimum else None
    rest_g = [g for g in gs if g is not outfit]
    accent = None
    if rest_g:
        accent = max(rest_g, key=lambda g: lw(g) * 2 + tw(g))
        if lw(accent) * 2 + tw(accent) < minimum * 2:
            accent = None
    for letter, g in (('O', outfit), ('A', accent)):
        for i in (g or []):
            m[i - 1] = letter
    return ''.join(m)


def main():
    dump, prefix = sys.argv[1], sys.argv[2]
    score = '--score' in sys.argv
    maps = {}
    if score:
        for line in open(os.path.join(HERE, '..', 'moemon_frlg', 'colormaps.txt')):
            p = line.split()
            if len(p) == 3 and not line.startswith('#'):
                maps[(p[0], p[1])] = p[2]
    hit = tot = 0
    per_letter = {}
    guessed = {}
    for line in open(os.path.join(dump, 'index.txt')):
        if not line.strip():
            continue
        kind, cfg = line.split()[:2]
        if score:
            if (kind, cfg) not in maps:
                continue
        elif not cfg.startswith(prefix):
            continue
        _, _, frames, cols = load(dump, line)
        cols = [(int(r), int(g), int(b)) for r, g, b in cols]
        guess = suggest(kind, frames, cols)
        if score:
            want = maps[(kind, cfg)]
            for a, b in zip(guess, want):
                if b != '.' or a != '.':
                    tot += 1
                    hit += a == b
                    k = per_letter.setdefault(b, [0, 0])
                    k[0] += a == b
                    k[1] += 1
                    g = guessed.setdefault(a, [0, 0])
                    g[0] += a == b
                    g[1] += 1
        else:
            print('%s %s %s' % (kind, cfg, guess))
    if score:
        print('agreement %d/%d = %.0f%%' % (hit, tot, 100.0 * hit / max(1, tot)))
        for k, (a, b) in sorted(per_letter.items()):
            print('  want %s: %d/%d   guessed %s: right %d/%d' % (k, a, b, k, guessed.get(k, [0, 0])[0], guessed.get(k, [0, 0])[1]))


if __name__ == '__main__':
    main()
