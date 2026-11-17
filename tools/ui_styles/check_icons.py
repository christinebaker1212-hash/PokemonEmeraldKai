#!/usr/bin/env python3
"""OPTIONS > DISPLAY > MENU POKéMON (RED / CRYSTAL): every party icon aligned the same way (the user, 2026-11-17): each
icon's first frame centred across its 16 pixels on the bottom row like every other, its second kept where the game
draws it against the first (Red's hops and shakes, Crystal's poses: pret's own). Checks every frame of ORIGINAL (RED, CRYSTAL) and EXTENDED (CRYSTAL) and draws them all, both frames over each
other and side by side, into OUTDIR/icons_<style>.png.

    python3 tools/ui_styles/check_icons.py POKERED POKECRYSTAL [OUTDIR]
"""
import os
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gb_build  # noqa: E402


def frames_of(pack, prefix):
    out = []
    i = 0
    while '%s.%d.0' % (prefix, i) in pack.image_ids:
        out.append(tuple(pack.images[pack.image_ids['%s.%d.%d' % (prefix, i, f)]] for f in (0, 1)))
        i += 1
    return out


def check(name, pairs):
    bad = []
    for i, (a, b) in enumerate(pairs):
        box = a.getchannel('A').getbbox()
        if not box:
            continue
        w = box[2] - box[0]
        if box[3] != 16 or box[0] != (16 - w) // 2:
            bad.append('%s %d: frame 1 at %s' % (name, i, box))
    return bad


def sheet(pairs, path):
    cols = 16
    rows = (len(pairs) + cols - 1) // cols
    out = Image.new('RGBA', (cols * 56, rows * 20), (200, 210, 200, 255))
    for i, (a, b) in enumerate(pairs):
        x, y = (i % cols) * 56, (i // cols) * 20 + 2
        out.alpha_composite(a, (x + 2, y))
        out.alpha_composite(b, (x + 20, y))
        both = a.copy()
        both.alpha_composite(b)
        out.alpha_composite(both, (x + 38, y))
    out.resize((out.width * 3, out.height * 3), Image.NEAREST).save(path)


def main():
    red, crystal = sys.argv[1:3]
    outdir = sys.argv[3] if len(sys.argv) > 3 else '.'
    bad = []
    for kind, src in (('red', red), ('crystal', crystal)):
        g = gb_build.GBPack(kind, src)
        g.make()
        for prefix in ('gbicon', 'ccicon'):
            pairs = frames_of(g.p, prefix)
            if not pairs:
                continue
            bad += check('%s %s' % (kind, prefix), pairs)
            sheet(pairs, os.path.join(outdir, 'icons_%s_%s.png' % (kind, prefix)))
            print('%s %s: %d icons' % (kind, prefix, len(pairs)))
    for b in bad:
        print('MOVES:', b)
    print('ok' if not bad else '%d icons not anchored' % len(bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
