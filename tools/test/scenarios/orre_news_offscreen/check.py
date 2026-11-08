#!/usr/bin/env python3
"""orre_news_offscreen: the TV news shot's last frame has no field sprite wrapped onto the TV (the player's white
hair: no near-white pixel above the message box)"""
import sys
from PIL import Image
im = Image.open(sys.argv[1] + '/news.png').convert('RGB')
w, h = im.size
n = sum(1 for y in range(int(h * 0.66)) for x in range(w) if min(im.getpixel((x, y))) > 235)
print(('ok  ' if n == 0 else 'FAIL') + ' %d near-white pixels over the TV (want 0)' % n)
sys.exit(0 if n == 0 else 1)
