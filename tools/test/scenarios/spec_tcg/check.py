#!/usr/bin/env python3
# spec_tcg: the panel's card of BOB (below the game, display info's rectangle) changed between ALICE's two shots,
# taken while her card screens were open and BOB walked
import re, sys
from PIL import Image, ImageChops
d = sys.argv[1]
log = open(d + '/a.log').read()
m = re.search(r'card 1 below 1x at (\d+),(\d+) (\d+)x(\d+)', log)
x, y, w, h = map(int, m.groups())
out = int(re.search(r'output (\d+)x\d+', log).group(1))
shots = [Image.open('%s/a_cards%d.png' % (d, i)).convert('RGB') for i in (1, 2)]
k = shots[0].width // out   # (the shots are scaled up)
a, b = (s.crop((x * k, y * k, (x + w) * k, (y + h) * k)) for s in shots)
diff = ImageChops.difference(a, b).getbbox()
print('panel card %dx%d at %d,%d: %s' % (w, h, x, y, 'changed' if diff else 'FROZEN'))
sys.exit(0 if diff else 1)
