#!/usr/bin/env python3
# PIKACHU mid-hop right of the player in shot hop (see conf): whole, not cut off at the bottom. Counted in GBA pixels
# (the shot may be 2x): its picture there is 10 rows and 48 pixels; cut, it was 8 rows and 43.
import os, sys
from PIL import Image
d = sys.argv[1]
path = os.path.join(d, 'hop.png') if os.path.exists(os.path.join(d, 'hop.png')) else os.path.join(d, 'hop.bmp')
im = Image.open(path).convert('RGB')
k = im.size[0] // 240
n, rows = 0, set()
for y in range(55 * k, 95 * k):
    for x in range(110 * k, 160 * k):
        r, g, b = im.getpixel((x, y))
        if r > 200 and g > 160 and b < 70:   # PIKACHU's yellow
            n += 1
            rows.add(y // k)
n //= k * k
if not rows:
    sys.exit('no PIKACHU in the hop shot')
height = max(rows) - min(rows) + 1
print('PIKACHU: %d pixels, %d rows (%d-%d)' % (n, height, min(rows), max(rows)))
if height < 10 or n < 46:
    sys.exit('PIKACHU is cut off: %d rows, %d pixels (whole: 10 rows, 48 pixels)' % (height, n))
