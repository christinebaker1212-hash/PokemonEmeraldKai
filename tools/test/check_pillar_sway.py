#!/usr/bin/env python3
"""johto_sprout_pillar's CHECK: the pillar (the screen's top middle, 2x pictures p0-p4 taken 16 frames apart) changes
between every two pictures: Crystal's sway steps every 16 frames.

    python3 tools/test/check_pillar_sway.py RESULT_DIR
"""
import os
import sys
from PIL import Image, ImageChops

BOX = (200, 20, 280, 120)   # (the pillar above the player, in the 480x320 picture)


def main():
    d = sys.argv[1]
    ims = [Image.open(os.path.join(d, 'p%d.png' % i)).convert('RGB').crop(BOX) for i in range(5)]
    same = [i for i in range(4) if ImageChops.difference(ims[i], ims[i + 1]).getbbox() is None]
    if len(same) > 1:   # (one pair can land on either side of the same step)
        sys.exit('FAIL: the pillar stood still between pictures %s' % same)
    print('pillar: sways (%d of 4 steps seen)' % (4 - len(same)))


if __name__ == '__main__':
    main()
