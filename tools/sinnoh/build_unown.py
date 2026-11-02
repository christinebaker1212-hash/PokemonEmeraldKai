"""Platinum's UNOWN font for sinnoh/platinum_gfx.pak (build_gfx.py appends it): res/fonts/font_unown.png and .json
from pret/pokeplatinum (never committed), in the game's own latin font layout (tools/gbagfx/font.c ConvertToLatinFont:
four 8x8 tiles a glyph, a row of 8 pixels a u16, 2 bits a pixel, the first pixel highest), the same colors as the
game's fonts. src/text.c FONT_UNOWN prints Platinum's MessageUnown inscriptions with it (the SOLACEON RUINS).

Section "UNWN": u16 count, u8 widths[count], then count x 32 u16 glyphs. Platinum's glyph numbers: its character
code - 1 (A 0x12B, a 0x145, ! 0x1AB, ? 0x1AC, space 0x1DE).
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import png8  # noqa: E402


def section(root):
    png = os.path.join(root, 'res/fonts/font_unown.png')
    if not os.path.exists(png):
        return b''
    im = png8.Image(png)
    widths = json.load(open(os.path.join(root, 'res/fonts/font_unown.json')))['glyphWidths']
    count = min(len(widths), (im.w // 16) * (im.h // 16))
    glyphs = bytearray()
    for g in range(count):
        gx, gy = (g % 16) * 16, (g // 16) * 16
        for tile in range(4):
            tx, ty = gx + (tile & 1) * 8, gy + (tile >> 1) * 8
            for y in range(8):
                v = 0
                for x in range(8):
                    v = (v << 2) | (im.px(tx + x, ty + y) & 3)
                glyphs += struct.pack('<H', v)
    return b'UNWN' + struct.pack('<H', count) + bytes(min(w, 255) for w in widths[:count]) + bytes(glyphs)


if __name__ == '__main__':
    print('unown font: %d bytes' % len(section(sys.argv[1])))
