#!/usr/bin/env python3
"""FireRed's S.S. ANNE (pokefirered graphics/object_events/pics/misc/ss_anne.png, its palette ss_anne.pal: copied to
graphics/moemon/kanto/) as a map picture for src/mp/mp_mappic.c: RGB555 pixels, bit 15 set where drawn (color 0 is
clear). Writes graphics/moemon/kanto/ss_anne.bin.

    python3 -I tools/quetzal_port/gen_ss_anne.py
"""
import struct
from PIL import Image

D = 'graphics/moemon/kanto/'


def main():
    pal = [tuple(int(v) for v in l.split()) for l in open(D + 'ss_anne.pal').read().split('\n')[3:] if l.strip()]
    im = Image.open(D + 'ss_anne.png')
    w, h = im.size
    out = bytearray()
    for y in range(h):
        for x in range(w):
            i = im.getpixel((x, y))
            if i == 0:
                out += struct.pack('<H', 0)
            else:
                r, g, b = pal[i]
                out += struct.pack('<H', 0x8000 | (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))
    open(D + 'ss_anne.bin', 'wb').write(out)
    print('%dx%d' % (w, h))


if __name__ == '__main__':
    main()
