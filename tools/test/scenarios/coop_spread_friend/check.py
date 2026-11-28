"""coop_spread_friend: in each game's frames (shotburst a / b), the two foes' HP bars (KAI's boxes: rows 22 and 47)
start going down within two frames of each other"""
import glob
import sys
from PIL import Image


def fill(p):
    r, g, b = p
    return (g > 200 and r < 140 and b < 200) or (r > 220 and g > 170 and b < 110) or (r > 220 and g < 120 and b < 110 and r - g > 100)


def first_drop(frames, row):
    full = None
    for i, f in enumerate(frames):
        im = Image.open(f).convert('RGB').resize((240, 160))
        n = sum(fill(im.getpixel((x, row))) for x in range(0, 130))
        if full is None and n >= 40:
            full = n
        elif full is not None and 0 < n < full:
            return i
    return None


d = sys.argv[1]
bad = 0
for who in 'ab':
    frames = sorted(glob.glob('%s/%s_*.png' % (d, who)))
    one, two = first_drop(frames, 22), first_drop(frames, 47)
    print('%s: the foes\' bars start down at frames %s and %s' % (who, one, two))
    if one is None or two is None or abs(one - two) > 2:
        bad = 1
sys.exit(bad)
