#!/usr/bin/env python3
"""Crystal's catching-tutorial DUDE as a battle back picture (ROUTE 29's demonstration, docs/REGIONS_AUDIT.md 133).

    python3 tools/quetzal_port/gen_dude_back.py [DUDE_PNG]

Reads pokecrystal gfx/battle/dude.png (48x48, four grays; from POKECRYSTAL_DIR or GitHub), scales it to the game's
64x64 back picture (nearest neighbour) and writes graphics/moemon/dude_back.bin: four identical 64x64 4bpp frames
(the throw's frames: Crystal's DUDE has one), then 16 colors (GBA BGR555; color 0, Crystal's white, is transparent).
"""
import io
import os
import struct
import sys
import urllib.request

from PIL import Image

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
LOCAL = os.environ.get('POKECRYSTAL_DIR', '/tmp/claude-0/frlg/pokecrystal')
URL = 'https://raw.githubusercontent.com/pret/pokecrystal/master/gfx/battle/dude.png'
# Crystal's four shades: white (transparent), skin, clothes, outline
PAL = [(255, 255, 255), (248, 200, 152), (168, 104, 56), (24, 24, 32), (248, 248, 248)]   # (4: his own white)


def bgr555(rgb):
    r, g, b = (c >> 3 for c in rgb)
    return r | (g << 5) | (b << 10)


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(LOCAL, 'gfx/battle/dude.png')
    data = open(path, 'rb').read() if os.path.exists(path) else urllib.request.urlopen(URL).read()
    img = Image.open(io.BytesIO(data)).convert('L')
    assert img.size == (48, 48), img.size
    # the grays as 0..3 (white 0 .. black 3)
    shades = sorted(set(img.get_flattened_data() if hasattr(img, 'get_flattened_data') else img.getdata()), reverse=True)
    index = {v: i for i, v in enumerate(shades)}
    src = [[index[img.getpixel((x, y))] for x in range(48)] for y in range(48)]
    # white is both the background and his face on the Game Boy: only white reached from the edges is transparent
    # (0); white inside him is opaque white (4)
    outside = set()
    todo = [(x, y) for x in range(48) for y in (0, 47)] + [(x, y) for y in range(48) for x in (0, 47)]
    while todo:
        x, y = todo.pop()
        if (x, y) in outside or not (0 <= x < 48 and 0 <= y < 48) or src[y][x] != 0:
            continue
        outside.add((x, y))
        todo += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    for y in range(48):
        for x in range(48):
            if src[y][x] == 0 and (x, y) not in outside:
                src[y][x] = 4
    px = [[src[y * 48 // 64][x * 48 // 64] for x in range(64)] for y in range(64)]
    frame = bytearray()
    for ty in range(8):
        for tx in range(8):
            for y in range(8):
                for x in range(0, 8, 2):
                    lo = px[ty * 8 + y][tx * 8 + x]
                    hi = px[ty * 8 + y][tx * 8 + x + 1]
                    frame.append(lo | (hi << 4))
    pal = [bgr555(c) for c in PAL] + [0] * (16 - len(PAL))
    out = bytes(frame) * 4 + struct.pack('<16H', *pal)
    dst = os.path.join(REPO, 'graphics/moemon/dude_back.bin')
    open(dst, 'wb').write(out)
    print('%s: %d bytes' % (dst, len(out)))


if __name__ == '__main__':
    main()
