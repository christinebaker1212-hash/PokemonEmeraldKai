#!/usr/bin/env python3
# summary_wrap_shots: INFO after wrapping round (info1) and after scrolling back (info2) is the INFO page it opened on
# (info0): the page panel, right of the Moemon's picture. The JOURNAL reached by LEFT round from INFO (journal) is the
# one reached by RIGHT page after page (journal2), and its one sentence's two rows are one stripe color.
import sys
from PIL import Image
d = sys.argv[1]
def load(n):
    return Image.open('%s/%s.png' % (d, n)).convert('RGB')
def panel(n):
    im = load(n)
    w, h = im.size
    return im.crop((w * 96 // 240, h * 16 // 160, w, h))
base = panel('info0')
for n in ('info1', 'info2'):
    if panel(n).tobytes() != base.tobytes():
        print('%s differs from info0' % n)
        sys.exit(1)
if panel("journal").tobytes() != panel("journal2").tobytes():
    print('the JOURNAL reached by LEFT differs from the one reached by RIGHT')
    sys.exit(1)
im = load('journal2')
w, h = im.size
# rows of 16 pixels from y 32 (the sheet under the top bar); a spot right of the text on each of the first three rows
row = [im.getpixel((w * 228 // 240, h * (32 + 16 * i + 8) // 160)) for i in range(3)]
if row[0] != row[1]:
    print('the sentence\'s two rows are different colors: %s %s' % (row[0], row[1]))
    sys.exit(1)
if row[2] == row[1]:
    print('the row after the sentence has its color: %s' % (row[2],))
    sys.exit(1)
print('ok')
