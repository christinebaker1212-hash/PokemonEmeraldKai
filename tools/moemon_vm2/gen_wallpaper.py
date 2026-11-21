#!/usr/bin/env python3
"""Moemon Kai: the VM2 box's wallpaper (pokemon_storage_system.c draws it on the VM2's box: src/mp/mp_vm2.c), as the
user approved it in docs/vmu_pet/kai_vm2_mockups.png: a VMU drawn after the user's photo on a soft lattice, its
screen over the box's place 14 (row 2, column 2: mp_vm2.c VM2_SLOT), the name plate plain.

usage: gen_wallpaper.py OUT_DIR      (writes frame.png: the plate's tiles, palette 1; bg.png: the picture's, palette
                                      2; tilemap.bin: 20x18 like every wallpaper's)

The picture is the VMU drawing of vmu_pet/tools/kai_mockups.py at GBA size, in 15 colours.
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'vmu_pet', 'tools'))
from PIL import Image  # noqa: E402
import kai_mockups  # noqa: E402

PLATE = [(200, 216, 248), (224, 232, 252), (152, 168, 216)]


def gba(c):
    return tuple(v & 0xF8 for v in c[:3])


def tiles_of(img, palette):
    """img (RGB) cut into 8x8 tiles of palette indices (index 0 left clear)"""
    idx = {c: i + 1 for i, c in enumerate(palette)}
    out = []
    for ty in range(img.height // 8):
        for tx in range(img.width // 8):
            t = tuple(idx[gba(img.getpixel((tx * 8 + x, ty * 8 + y)))] for y in range(8) for x in range(8))
            out.append(t)
    return out


def strip(tiles, palette, path):
    """tiles as one row of an indexed PNG, exactly as many as there are (gbagfx turns every tile of it into one)"""
    cols = len(tiles)
    rows = 1
    im = Image.new('P', (cols * 8, rows * 8), 0)
    flat = [(255, 0, 255)] + list(palette)
    pal = []
    for c in flat + [(0, 0, 0)] * (16 - len(flat)):
        pal += list(c)
    im.putpalette(pal)
    for n, t in enumerate(tiles):
        for i, v in enumerate(t):
            im.putpixel(((n % cols) * 8 + i % 8, (n // cols) * 8 + i // 8), v)
    im.save(path)


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    # the picture: rows 3-17 of the wallpaper (160x120), the VMU's screen centred on place 14
    pic, _ = kai_mockups.vmu_wallpaper(160, 120, 35, 6)
    pic = pic.convert('RGB')
    colours = sorted({gba(c) for c in (pic.get_flattened_data() if hasattr(pic, 'get_flattened_data') else pic.getdata())})
    if len(colours) > 15:
        sys.exit('the picture has %d colours (15 at most)' % len(colours))
    # the plate: rows 0-2, columns 2-17 (128x24), plain, the box's name drawn on it by the game
    plate = Image.new('RGB', (128, 24), PLATE[0])
    for x in range(128):
        plate.putpixel((x, 0), PLATE[1])
        plate.putpixel((x, 23), PLATE[2])
    for y in range(24):
        plate.putpixel((0, y), PLATE[2])
        plate.putpixel((127, y), PLATE[2])
    plate_pal = [gba(c) for c in PLATE]
    # frame.png: tile 0 clear, then the plate's tiles (each kept once); bg.png: the picture's
    frame_tiles = [tuple([0] * 64)]
    tm = [0] * (20 * 18)
    for n, t in enumerate(tiles_of(plate, plate_pal)):
        if t not in frame_tiles:
            frame_tiles.append(t)
        r, c = n // 16, n % 16
        tm[r * 20 + 2 + c] = frame_tiles.index(t) | (1 << 12)
    bg_tiles = []
    for n, t in enumerate(tiles_of(pic, colours)):
        if t not in bg_tiles:
            bg_tiles.append(t)
        r, c = n // 20, n % 20
        tm[(3 + r) * 20 + c] = (len(frame_tiles) + bg_tiles.index(t)) | (2 << 12)
    if len(frame_tiles) + len(bg_tiles) > 256:
        sys.exit('%d tiles (a wallpaper has 256)' % (len(frame_tiles) + len(bg_tiles)))
    strip(frame_tiles, plate_pal, os.path.join(out, 'frame.png'))
    strip(bg_tiles, colours, os.path.join(out, 'bg.png'))
    open(os.path.join(out, 'tilemap.bin'), 'wb').write(struct.pack('<360H', *tm))
    print('VM2 wallpaper: %d + %d tiles, %d colours' % (len(frame_tiles), len(bg_tiles), len(colours)))


if __name__ == '__main__':
    main()
