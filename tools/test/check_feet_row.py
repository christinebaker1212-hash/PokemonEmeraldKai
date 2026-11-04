#!/usr/bin/env python3
"""sinnoh_feet_row: the player (screen middle) and the person a tile to the right have their lowest outline pixel on
the same row of side.png (2x)"""
import os
import sys
from PIL import Image

im = Image.open(os.path.join(sys.argv[1], 'side.png')).convert('RGB')
px = im.load()


def lowest(x0, x1):
    for y in range(200, 100, -1):
        for x in range(x0, x1):
            r, g, b = px[x, y]
            if r < 50 and g < 50 and b < 50:
                return y
    return None


player, npc = lowest(225, 252), lowest(258, 290)
print('feet rows: player %s, beside %s' % (player, npc))
sys.exit(0 if player is not None and player == npc else 1)
