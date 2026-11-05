#!/usr/bin/env python3
"""FireRed's VIRIDIAN OLD MAN as a battle back picture (his catching demonstration, docs/REGIONS_AUDIT.md 120).

    python3 -I tools/quetzal_port/gen_oldman_back.py [OLD_MAN_PNG OLD_MAN_PAL]

Reads pokefirered graphics/trainers/back_pics/old_man_back_pic.png (64x256: four 64x64 frames, indexed) and
graphics/trainers/palettes/old_man_back_pic.pal (JASC; from the paths given or GitHub) and writes
graphics/moemon/oldman_back.bin as battle_setup.c's DUDE picture is: four 64x64 4bpp frames, then 16 colors (BGR555).
"""
import io
import os
import struct
import sys
import urllib.request

from PIL import Image

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RAW = 'https://raw.githubusercontent.com/pret/pokefirered/master/'


def read(path, rel):
    if path and os.path.exists(path):
        return open(path, 'rb').read()
    return urllib.request.urlopen(RAW + rel, timeout=30).read()


def main():
    png = read(sys.argv[1] if len(sys.argv) > 1 else None, 'graphics/trainers/back_pics/old_man_back_pic.png')
    pal = read(sys.argv[2] if len(sys.argv) > 2 else None, 'graphics/trainers/palettes/old_man_back_pic.pal')
    img = Image.open(io.BytesIO(png))
    assert img.size == (64, 256) and img.mode == 'P', (img.size, img.mode)
    lines = pal.decode().split()
    assert lines[0] == 'JASC-PAL'
    n = int(lines[2])
    colors = [tuple(int(v) for v in lines[3 + 3 * i:6 + 3 * i]) for i in range(n)]
    out = bytearray()
    for f in range(4):
        for ty in range(8):
            for tx in range(8):
                for y in range(8):
                    for x in range(0, 8, 2):
                        lo = img.getpixel((tx * 8 + x, f * 64 + ty * 8 + y)) & 15
                        hi = img.getpixel((tx * 8 + x + 1, f * 64 + ty * 8 + y)) & 15
                        out.append(lo | (hi << 4))
    words = [(r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) for r, g, b in colors] + [0] * (16 - n)
    out += struct.pack('<16H', *words[:16])
    dst = os.path.join(REPO, 'graphics/moemon/oldman_back.bin')
    open(dst, 'wb').write(out)
    print('%s: %d bytes' % (dst, len(out)))


if __name__ == '__main__':
    main()
