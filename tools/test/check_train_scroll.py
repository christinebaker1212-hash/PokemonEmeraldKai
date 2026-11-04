#!/usr/bin/env python3
"""region_train_ride's CHECK: the MAGNET TRAIN's scenery moves over the whole height of the screen.

    python3 tools/test/check_train_scroll.py RESULT_DIR

Two frames in a row (shotburst ride_s 2: ride_s_0000 / _0001, taken before the train comes on) must differ in every
8-pixel row of the screen (the scenery scrolls 4 pixels a frame; only the train's band, rows 7-12, stands still while
it waits off screen), and the top and bottom rows must be the trees (green), as Crystal's first and last rows are.
"""
import os
import sys
from PIL import Image

TRAIN_ROWS = range(7, 13)


def load(d, n):
    for ext in ('png', 'bmp'):
        p = os.path.join(d, 'ride_s_%04d.%s' % (n, ext))
        if os.path.exists(p):
            im = Image.open(p).convert('RGB')
            return im.resize((240, 160), Image.NEAREST) if im.size != (240, 160) else im
    sys.exit('FAIL: no ride_s_%04d picture in %s' % (n, d))


def main():
    d = sys.argv[1]
    a, b = load(d, 0), load(d, 1)
    bad = []
    for row in range(20):
        if row in TRAIN_ROWS:
            continue
        box = (0, row * 8, 240, row * 8 + 8)
        if a.crop(box).tobytes() == b.crop(box).tobytes():
            bad.append('row %d stands still' % row)
    for row in (0, 19):
        px = a.crop((0, row * 8, 240, row * 8 + 8)).getdata()
        green = sum(1 for r, g, bl in px if g > r + 16 and g > bl + 16)
        if green < len(px) // 4:
            bad.append('row %d is not the trees (%d green pixels of %d)' % (row, green, len(px)))
    if bad:
        sys.exit('FAIL: ' + '; '.join(bad))
    print('train scroll: every row moves, trees at the top and bottom')


if __name__ == '__main__':
    main()
