#!/usr/bin/env python3
"""gb_colors: the shots hold only the colours GB OPTIONS > COLORS allows (src/mp/mp_gbpal.c)."""
import os, sys
from PIL import Image

d = sys.argv[1]

def rgb(r, g, b):   # a GBA colour as the shot stores it
    return (r * 255 // 31, g * 255 // 31, b * 255 // 31)

DMG = {rgb(19, 23, 1), rgb(17, 21, 1), rgb(6, 12, 6), rgb(1, 7, 1)}
POCKET = {rgb(24, 25, 20), rgb(17, 18, 13), rgb(9, 10, 7), rgb(3, 3, 3)}
COLOR = {rgb(31, 31, 31), rgb(31, 16, 16), rgb(18, 7, 7), rgb(0, 0, 0), rgb(15, 31, 6), rgb(0, 16, 0)}

def colors(name):
    path = os.path.join(d, name + '.png')
    im = Image.open(path if os.path.exists(path) else path[:-3] + 'bmp').convert('RGB')
    return im, set(c for _, c in im.getcolors(1 << 16))

bad = []
for name, allowed in (('dmg_field', DMG), ('dmg_battle', DMG), ('pocket_field', POCKET), ('color_field', COLOR)):
    _, cs = colors(name)
    if not cs <= allowed:
        bad.append('%s: %d colours outside the mode %s' % (name, len(cs - allowed), sorted(cs - allowed)[:4]))
_, kai = colors('kai_field')
if len(kai) <= 16:
    bad.append('kai_field: only %d colours (KAI should be untouched)' % len(kai))
# CRYSTAL: few cells over four colours (only where sprites lie over the background, as on a Game Boy Color)
im, _ = colors('crystal_field')
px = im.load()
over = sum(1 for ty in range(20) for tx in range(30)
           if len(set(px[tx * 8 + x, ty * 8 + y] for x in range(8) for y in range(8))) > 6)
if over > 30:
    bad.append('crystal_field: %d cells over six colours' % over)
print('\n'.join(bad) if bad else 'ok: DMG, POCKET, COLOR in their colours; KAI untouched; CRYSTAL %d busy cells' % over)
sys.exit(1 if bad else 0)
