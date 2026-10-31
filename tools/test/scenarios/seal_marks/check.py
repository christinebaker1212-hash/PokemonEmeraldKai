#!/usr/bin/env python3
# SEALED marks: a past-GENERATION Pokemon's icon is gray in the party and the PC (TORCHIC's isn't), and its summary
# says SEALED in red where the number goes (not for TORCHIC).
import sys
from PIL import Image
d = sys.argv[1]


def count(name, box, test):
    im = Image.open('%s/%s.png' % (d, name)).convert('RGB').crop(box)
    return sum(1 for p in im.getdata() if test(*p))


def gray(r, g, b):
    return max(r, g, b) - min(r, g, b) < 16 and 40 < r < 235


def red(r, g, b):
    return r > 180 and g < 90 and b < 90


checks = [
    ('party: DIALGA gray', count('party', (170, 16, 214, 64), gray) > 900),
    ('party: TORCHIC colored', count('party', (10, 60, 60, 110), gray) < 500),
    ('PC: DIALGA gray', count('pc', (168, 70, 212, 112), gray) > 900),
    ('PC: TORCHIC colored', count('pc', (220, 70, 264, 112), gray) < 500),
    ('summary: SEALED in red', count('sumsealed', (10, 36, 60, 60), red) > 100),
    ('summary: TORCHIC not sealed', count('sumok', (10, 36, 60, 60), red) == 0),
]
bad = [n for n, ok in checks if not ok]
for n in bad:
    print('FAIL', n)
print('%d checks: %s' % (len(checks), 'ok' if not bad else '%d failed' % len(bad)))
sys.exit(1 if bad else 0)
