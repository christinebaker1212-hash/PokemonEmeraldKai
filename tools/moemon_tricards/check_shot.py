#!/usr/bin/env python3
"""Checks the tri_cards test's screenshots (tools/test/scenarios/tri_cards) against the mock-up.
usage: check_shot.py RESULT_DIR   (tri.png and tri_st.png in it, 2x screenshots)
- tri.png: the POOCHYENA Lv8 (female, full HP) card at the bottom of the foes: every solid pixel (text, gender,
  HP bar) as in cards_mockup.png's foe card; its Dark type icon in front at the mock-up's place
- tri_st.png: ZIGZAGOON (top foe card) poisoned: the PSN tag over the level, as status.png draws it"""
import os, sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
SOLID = {(74, 74, 82), (255, 156, 148), (82, 106, 90), (255, 255, 255), (90, 213, 131), (115, 255, 172),
         (255, 180, 123), (246, 148, 115), (222, 106, 90), (213, 205, 189)}


def shot(path):
    im = Image.open(path).convert('RGB')
    return im.resize((240, 160), Image.NEAREST) if im.size != (240, 160) else im


def main():
    d = sys.argv[1]
    art = Image.open(os.path.join(HERE, 'cards_mockup.png')).convert('RGB')
    g = shot(os.path.join(d, 'tri.png'))
    bad = 0
    for y in range(22):
        for x in range(80):
            a = art.getpixel((x, y))
            if a in SOLID and g.getpixel((1 + x, 43 + y)) != a:
                bad += 1
    print('POOCHYENA card: %d solid pixels differ' % bad)
    # the Dark icon's outline (49,41,41) in front: 11x11 at body + (79, 6), the mock-up's place
    pts = [(x, y) for y in range(43, 64) for x in range(74, 100) if g.getpixel((x, y)) == (49, 41, 41)]
    box = (min(p[0] for p in pts), min(p[1] for p in pts), max(p[0] for p in pts), max(p[1] for p in pts)) if pts else None
    print('Dark icon outline at', box)
    ok = bad == 0 and box == (80, 49, 90, 59)
    # PSN: the tag's purple RGB(24,12,24) where status.png has its color, over the level of the top card
    st = Image.open(os.path.join(HERE, '..', '..', 'graphics', 'battle_interface', 'status.png'))
    g2 = shot(os.path.join(d, 'tri_st.png'))
    miss = 0
    for y in range(8):
        for x in range(20):
            want = (st.getpixel((x, y)) & 15) == 12
            got = g2.getpixel((1 + 4 + x, 1 + 11 + y))
            if want and abs(got[0] - 197) > 8:
                miss += 1
    print('PSN tag: %d pixels off' % miss)
    ok = ok and miss == 0
    print('OK' if ok else 'MISMATCH')
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
