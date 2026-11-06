#!/bin/bash
# the stat columns (the STATS page's right half) differ between the two POKéMON, in both views
python3 - "$1" <<'PY'
import sys, os, glob
from PIL import Image
d = sys.argv[1]
def shot(name):
    for f in (os.path.join(d, name + '.png'), os.path.join(d, 'a_' + name + '.png'), os.path.join(d, name + '.bmp')):
        if os.path.exists(f):
            return Image.open(f).convert('RGB')
    sys.exit('no shot ' + name)
def cols(im):
    w, h = im.size
    return im.crop((w // 2, h * 40 // 160, w, h * 100 // 160)).tobytes()
bad = 0
for a, b in (('first', 'second'), ('second_evs', 'first_evs')):
    if cols(shot(a)) == cols(shot(b)):
        print('the stat columns of %s and %s are the same' % (a, b)); bad = 1
    else:
        print('ok: %s and %s differ' % (a, b))
sys.exit(bad)
PY
