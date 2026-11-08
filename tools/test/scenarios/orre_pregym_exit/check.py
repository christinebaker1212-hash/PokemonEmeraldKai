#!/usr/bin/env python3
"""orre_pregym_exit: the player is drawn where they come out of the PRE GYM (their cap's white near the middle)"""
import sys
from PIL import Image
im = Image.open(sys.argv[1] + '/out0.png').convert('RGB')
w, h = im.size
n = sum(1 for y in range(int(h * 0.3), int(h * 0.7)) for x in range(int(w * 0.35), int(w * 0.65))
        if min(im.getpixel((x, y))) > 235)
print(('ok  ' if n > 20 else 'FAIL') + ' %d white pixels of the player near the middle (want > 20)' % n)
sys.exit(0 if n > 20 else 1)
