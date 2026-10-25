#!/usr/bin/env python3
"""Renders the 8x16 font Moemon Music Studio draws with (src/font.h, checked in).
  python3 tools/moemon_music/gen_font.py [TTF]     default: DejaVu Sans Mono"""
import os, sys
from PIL import Image, ImageDraw, ImageFont
ttf = sys.argv[1] if len(sys.argv) > 1 else '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf'
W, H = 8, 16
font = ImageFont.truetype(ttf, 13)
rows = []
for c in range(32, 127):
    img = Image.new('L', (W, H), 0)
    d = ImageDraw.Draw(img)
    d.text((0, 0), chr(c), font=font, fill=255)
    px = img.load()
    for y in range(H):
        bits = 0
        for x in range(W):
            if px[x, y] >= 110:
                bits |= 0x80 >> x
        rows.append(bits)
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'src', 'font.h')
with open(out, 'w') as f:
    f.write('// 8x16 font for ASCII 32-126, rendered from DejaVu Sans Mono by gen_font.py.\n')
    f.write('// DejaVu fonts: Bitstream Vera license (free to use and redistribute).\n')
    f.write('static const unsigned char kFont[95][16] = {\n')
    for i in range(95):
        f.write('  {' + ','.join('0x%02x' % b for b in rows[i * 16:(i + 1) * 16]) + '},\n')
    f.write('};\n')
print('wrote', out)
