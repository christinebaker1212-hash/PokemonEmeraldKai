#!/usr/bin/env python3
"""Renders the fonts Moemon Music Studio draws with (both checked in):
  src/font.h       the 8x16 pixel font (1 bit per pixel)
  src/fonts.bin    the Smooth and Bold fonts, anti-aliased, at 1x, 2x and 3x (one alpha byte per pixel),
                   so text stays sharp however much the window is scaled
  python3 tools/moemon_music/gen_font.py     (DejaVu Sans Mono, Bitstream Vera license)"""
import os
from PIL import Image, ImageDraw, ImageFont

DEJAVU = '/usr/share/fonts/truetype/dejavu/'
HERE = os.path.dirname(os.path.abspath(__file__))
W, H = 8, 16
SCALES = (1, 2, 3)

def glyphs(ttf, scale):
    font = ImageFont.truetype(ttf, 13 * scale)
    for c in range(32, 127):
        img = Image.new('L', (W * scale, H * scale), 0)
        ImageDraw.Draw(img).text((0, 0), chr(c), font=font, fill=255)
        yield img

# the pixel font
rows = []
for img in glyphs(DEJAVU + 'DejaVuSansMono.ttf', 1):
    px = img.load()
    for y in range(H):
        rows.append(sum(0x80 >> x for x in range(W) if px[x, y] >= 110))
with open(os.path.join(HERE, 'src', 'font.h'), 'w') as f:
    f.write('// 8x16 font for ASCII 32-126, rendered from DejaVu Sans Mono by gen_font.py.\n')
    f.write('// DejaVu fonts: Bitstream Vera license (free to use and redistribute).\n')
    f.write('static const unsigned char kFont[95][16] = {\n')
    for i in range(95):
        f.write('  {' + ','.join('0x%02x' % b for b in rows[i * 16:(i + 1) * 16]) + '},\n')
    f.write('};\n')

# the smooth fonts: for each font, for each scale, 95 glyphs of (8*scale) x (16*scale) alpha bytes, row by row
out = bytearray()
for ttf in ('DejaVuSansMono.ttf', 'DejaVuSansMono-Bold.ttf'):
    for s in SCALES:
        for img in glyphs(DEJAVU + ttf, s):
            out += img.tobytes()
with open(os.path.join(HERE, 'src', 'fonts.bin'), 'wb') as f:
    f.write(out)
print('wrote src/font.h and src/fonts.bin (%d bytes)' % len(out))
