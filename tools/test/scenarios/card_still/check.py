#!/usr/bin/env python3
# card_still: the TRAINER CARD holds still with nothing pressed (still1 = still2), turns with the right stick
# (tilted), and settles flat again when the stick is let go (settled = still1)
import sys
from PIL import Image, ImageChops
d = sys.argv[1]
s = {n: Image.open('%s/%s.png' % (d, n)).convert('RGB') for n in ('still1', 'still2', 'tilted', 'settled')}
# (the card, not the hints under it: their button pictures change once the controller is plugged in)
s = {n: im.crop((0, 0, im.width, im.height * 88 // 100)) for n, im in s.items()}
same = lambda a, b: ImageChops.difference(s[a], s[b]).getbbox() is None
ok = same('still1', 'still2') and not same('still2', 'tilted') and same('still1', 'settled')
print('still: %s, turned by the stick: %s, settled flat: %s' % (same('still1', 'still2'), not same('still2', 'tilted'),
                                                               same('still1', 'settled')))
sys.exit(0 if ok else 1)
