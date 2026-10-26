#!/usr/bin/env python3
"""Checks the tri_party test's screenshot (tools/test/scenarios/tri_party) against the user's mock-up of the co-op
Triple Battle party screen (party_mock.png, 240x160).
usage: check_party.py RESULT_DIR   (party.png in it, a 2x screenshot)
Every pixel must match, except the Pokemon icons and the selected one's Poke Ball (animated sprites: the mock-up
shows one frame of them, the shot may catch another) and the HP numbers (the test party's stats are random)."""
import os, sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
# x0, y0, x1, y1: the selected slot's icon and ball, the other icons, then the HP numbers
SKIP = [(0, 0, 32, 40), (0, 40, 32, 80), (0, 80, 32, 120), (80, 12, 112, 40), (80, 50, 112, 80),
        (40, 38, 88, 48), (40, 78, 88, 88), (40, 118, 88, 128), (200, 22, 240, 40), (200, 60, 240, 80)]


def shot(path):
    im = Image.open(path).convert('RGB')
    return im.resize((240, 160), Image.NEAREST) if im.size != (240, 160) else im


def main():
    d = sys.argv[1]
    mock = Image.open(os.path.join(HERE, 'party_mock.png')).convert('RGB')
    g = shot(os.path.join(d, 'party.png'))
    bad = []
    for y in range(160):
        for x in range(240):
            if any(a <= x < c and b <= y < d for a, b, c, d in SKIP):
                continue
            if mock.getpixel((x, y)) != g.getpixel((x, y)):
                bad.append((x, y))
    if bad:
        xs = [p[0] for p in bad]
        ys = [p[1] for p in bad]
        print('%d pixels differ, in x %d-%d y %d-%d, first %s' % (len(bad), min(xs), max(xs), min(ys), max(ys), bad[:8]))
        return 1
    print('party screen matches the mock-up')
    return 0


if __name__ == '__main__':
    sys.exit(main())
