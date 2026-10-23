#!/usr/bin/env python3
"""Moemon Kai: the type CANDIES (CANDY JAR).

Usage: draw_candies.py SRC_DIR [--preview out.png]
SRC_DIR holds the user's candy<type>.png for ELECTRIC, FIRE, GRASS, GROUND, POISON and WATER (items.zip). Those are
kept as they are; the other 11 types are drawn from the same candy: every pixel position has a role (outline, body,
shadow, stripes...), each role's color is fitted as a linear function of the body color over the six candies, and a new
type's candy is that function of its own body color.
Writes graphics/moemon/candies/<type>.png (the 16x16 ball, 16-color indexed, index 0 see-through) and jar.png (the
CANDY JAR screen's jar, 96x128, drawn here).
"""
import os
import sys

import numpy as np
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
OUT = os.path.join(ROOT, 'graphics', 'moemon', 'candies')

GIVEN = ['electric', 'fire', 'grass', 'ground', 'poison', 'water']
# body colors for the drawn ones (the classic type colors, tuned to sit next to the given candies)
NEW = {
    'normal': (184, 176, 148), 'fighting': (204, 74, 46), 'flying': (160, 178, 242), 'rock': (190, 164, 80),
    'bug': (164, 192, 40), 'ghost': (112, 88, 164), 'steel': (178, 182, 204), 'psychic': (246, 98, 164),
    'ice': (118, 214, 224), 'dragon': (108, 74, 232), 'dark': (106, 86, 74),
}


def main():
    src = sys.argv[1]
    preview = sys.argv[sys.argv.index('--preview') + 1] if '--preview' in sys.argv else None
    os.makedirs(OUT, exist_ok=True)
    ims = {n: np.array(Image.open(os.path.join(src, 'candy%s.png' % n)).convert('RGBA')).astype(float) for n in GIVEN}
    h, w = ims[GIVEN[0]].shape[:2]
    # roles: positions sharing the same colors in every given candy
    roles = {}
    for y in range(h):
        for x in range(w):
            cols = tuple(tuple(ims[n][y, x]) for n in GIVEN)
            if all(c[3] < 128 for c in cols):
                continue
            roles.setdefault(cols, []).append((y, x))
    # the body: the most common role that changes between candies
    body = max((c for c in roles if len({k[:3] for k in c}) > 1), key=lambda c: len(roles[c]))
    bodies = np.array([c[:3] for c in body])
    design = np.hstack([bodies, np.ones((len(GIVEN), 1))])
    fits = {}
    for cols in roles:
        target = np.array([c[:3] for c in cols])
        coef, *_ = np.linalg.lstsq(design, target, rcond=None)   # rgb = M @ body + b
        fits[cols] = coef
    results = {}
    for n in GIVEN:
        results[n] = ims[n].astype(np.uint8)
    for n, rgb in NEW.items():
        img = np.zeros((h, w, 4), np.uint8)
        v = np.array(list(rgb) + [1.0])
        for cols, ps in roles.items():
            c = np.clip(np.rint(v @ fits[cols]), 0, 255)
            for (y, x) in ps:
                img[y, x, :3] = c
                img[y, x, 3] = 255
        results[n] = img
    for n, img in results.items():
        write_indexed(os.path.join(OUT, n), img[8:24, 4:20])
    write_indexed(os.path.join(OUT, 'jar'), draw_jar())
    if preview:
        order = ['normal', 'fighting', 'flying', 'poison', 'ground', 'rock', 'bug', 'ghost', 'steel', 'fire', 'water',
                 'grass', 'electric', 'psychic', 'ice', 'dragon', 'dark']
        sheet = Image.new('RGBA', (28 * len(order), 28), (232, 232, 232, 255))
        for i, n in enumerate(order):
            sheet.alpha_composite(Image.fromarray(results[n], 'RGBA'), (28 * i + 2, 2))
        sheet.resize((sheet.width * 6, sheet.height * 6), Image.NEAREST).save(preview)


def gba(c):
    return tuple(int(v) >> 3 << 3 for v in c[:3])


def write_indexed(base, rgba):
    """rgba (h, w, 4) -> base.png (16 colors indexed, 0 = see-through)"""
    h, w = rgba.shape[:2]
    colors = []
    idx = np.zeros((h, w), np.uint8)
    for y in range(h):
        for x in range(w):
            if rgba[y, x, 3] < 128:
                continue
            c = gba(rgba[y, x])
            if c not in colors:
                colors.append(c)
            idx[y, x] = 1 + colors.index(c)
    if len(colors) > 15:
        raise SystemExit('%s: %d colors' % (base, len(colors)))
    pal = [(0x70, 0xC0, 0xA0)] + colors + [(0, 0, 0)] * (15 - len(colors))
    im = Image.fromarray(idx, 'P')
    im.putpalette([v for c in pal for v in c] + [0] * (768 - 48))
    im.save(base + '.png', transparency=0)   # the game takes both the tiles and the palette from it


def draw_jar():
    """A glass jar with a pink lid, 96x128: rim outline, pale glass, a shine on the left, a shadow on the right."""
    W, H = 96, 128
    img = np.zeros((H, W, 4), np.uint8)
    OUTLINE, GLASS, GLASS2, SHINE, LID, LID2, LIDHI, BAND = (
        (40, 48, 72), (216, 240, 248), (184, 216, 232), (248, 248, 248), (232, 96, 136), (184, 64, 104),
        (248, 168, 192), (248, 216, 112))

    def put(x, y, c):
        if 0 <= x < W and 0 <= y < H:
            img[y, x, :3] = c
            img[y, x, 3] = 255

    def inside_body(x, y):
        # body: x 6..89, y 26..123, corners rounded (radius 14 at the bottom, 8 at the shoulders)
        if not (6 <= x <= 89 and 26 <= y <= 123):
            return False
        for cx, cy, r, top in ((20, 109, 14, False), (75, 109, 14, False), (14, 34, 8, True), (81, 34, 8, True)):
            if (top and y < cy) or (not top and y > cy):
                if (x < cx if cx < 48 else x > cx) and (x - cx) ** 2 + (y - cy) ** 2 > r * r:
                    return False
        return True

    for y in range(H):
        for x in range(W):
            if inside_body(x, y):
                edge = not all(inside_body(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
                if edge:
                    put(x, y, OUTLINE)
                elif x > 78:
                    put(x, y, GLASS2)
                elif 12 <= x <= 16 and 36 <= y <= 100:
                    put(x, y, SHINE)
                else:
                    put(x, y, GLASS)
    # neck (y 18..27) and lid (y 2..17)
    for y in range(18, 28):
        for x in range(18, 78):
            put(x, y, OUTLINE if x in (18, 77) else (GLASS2 if x > 70 else GLASS))
    for y in range(2, 18):
        for x in range(12, 84):
            edge = y in (2, 17) or x in (12, 83)
            put(x, y, OUTLINE if edge else (LIDHI if y < 6 else (BAND if 11 <= y <= 13 else (LID2 if x > 74 else LID))))
    return img


if __name__ == '__main__':
    main()
