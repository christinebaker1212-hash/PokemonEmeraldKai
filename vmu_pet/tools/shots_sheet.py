#!/usr/bin/env python3
"""Moemon VMU pet: harness shots (NAME.txt, 48x32) as one picture in a VMU screen's colours.
usage: shots_sheet.py OUT.png LABEL=SHOT.txt ..."""
import sys
from PIL import Image, ImageDraw

OFF, ON, BG = (156, 176, 146), (36, 46, 40), (232, 236, 240)
Z, PAD, COLS = 5, 16, 4
items = [a.split('=', 1) for a in sys.argv[2:]]
w, h = 48 * Z, 32 * Z
rows = (len(items) + COLS - 1) // COLS
img = Image.new('RGB', (COLS * (w + PAD) + PAD, rows * (h + PAD + 18) + PAD), BG)
d = ImageDraw.Draw(img)
for i, (label, path) in enumerate(items):
    px = [r for r in open(path).read().split('\n') if r]
    ox, oy = PAD + (i % COLS) * (w + PAD), PAD + (i // COLS) * (h + PAD + 18)
    d.rounded_rectangle([ox - 4, oy - 4, ox + w + 4, oy + h + 4], 6, fill=(60, 66, 70))
    for y in range(32):
        for x in range(48):
            d.rectangle([ox + x * Z, oy + y * Z, ox + x * Z + Z - 2, oy + y * Z + Z - 2], fill=ON if px[y][x] == '#' else OFF)
    d.text((ox, oy + h + 6), label, fill=(40, 40, 40))
img.save(sys.argv[1])
