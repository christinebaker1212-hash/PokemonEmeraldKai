#!/usr/bin/env python3
"""Moemon VMU pet: an edition's PICTURES, the data file that gives a hatched MOeMON its colour file icon (the user:
"use the normal portrait for the VMU icon").

usage: gen_icons.py kanto OUT.VMS [PREVIEW.png]

The 151 PMD Collab Normal portraits (tools/fetch_portraits.sh puts them in .tools/portraits) at 32x32 in 4 colours
each: 40 KB, more than the pet itself could carry (a VMU program is 64 KB at most), so they're their own file, which
the VMU's file menu copies like a save. Without it the pet's icon is its LCD sprite.

The file (a VMS data file; its blocks may lie anywhere: the pet follows the FAT):
  $000  the VMS header: names, one icon (the edition's first starter), the CRC
  $280  "MOEPICS" + format 1, the edition (1 KANTO), how many
  $400  (file blocks 2-4) the palettes: 8 bytes each, 4 ARGB4444 colours, little-endian
  $a00  (file block 5 on) the pictures: 256 bytes each (32 rows of 8 bytes, 2 bits a pixel, the left pixel high),
        two a block: picture i in file block 5 + i / 2 at (i % 2) * 256
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from vmufs import crc16  # noqa: E402

EDITIONS = {'kanto': (1, list(range(1, 152)), 1)}   # edition number, its MOeMON, the file's own icon
PORTRAITS = os.path.join(HERE, '..', '.tools', 'portraits')
PAL_AT, PIC_BLOCK = 0x400, 5


def argb(c):
    r, g, b = (min(15, (v + 8) // 17) for v in c[:3])
    return 0xF000 | (r << 8) | (g << 4) | b


def portrait(dex, colours):
    """The portrait at 32x32 in `colours` colours: (palette as ARGB4444, indices row by row)."""
    from PIL import Image
    path = os.path.join(PORTRAITS, '%04d.png' % dex)
    if not os.path.exists(path):
        sys.exit('no portrait %s: run vmu_pet/tools/fetch_portraits.sh' % path)
    im = Image.open(path).convert('RGB').resize((32, 32), Image.LANCZOS)
    q = im.quantize(colours, method=Image.Quantize.MEDIANCUT)
    pal = q.getpalette()[:colours * 3]
    idx = list(q.get_flattened_data()) if hasattr(q, 'get_flattened_data') else list(q.getdata())
    cols = [argb(pal[i * 3:i * 3 + 3]) for i in range(colours)]
    return cols, idx


def build(ed, out, preview=None):
    num, dex, own = EDITIONS[ed]
    pics = [portrait(d, 4) for d in dex]
    blocks = PIC_BLOCK + (len(dex) + 1) // 2
    data = bytearray(blocks * 512)
    hdr = bytearray(0x280)
    hdr[0:16] = b'MOEMON PICTURES'.ljust(16)
    hdr[16:48] = ('MOEMON VMU PET %s PICTURES' % ed.upper()).encode()[:32].ljust(32)
    hdr[48:64] = b'MOEMON KAI'.ljust(16, b'\0')
    struct.pack_into('<HHHHI', hdr, 0x40, 1, 0, 0, 0, len(data) - 0x280)
    cols, idx = portrait(own, 16)
    struct.pack_into('<16H', hdr, 0x60, *cols)
    for i in range(0, 1024, 2):
        hdr[0x80 + i // 2] = (idx[i] << 4) | idx[i + 1]
    data[:0x280] = hdr
    data[0x280:0x28b] = b'MOEPICS' + bytes([1, num, len(dex) & 255, 0])
    for i, (pal, ix) in enumerate(pics):
        struct.pack_into('<4H', data, PAL_AT + i * 8, *pal)
        at = (PIC_BLOCK + i // 2) * 512 + (i % 2) * 256
        for p in range(0, 1024, 4):
            data[at + p // 4] = (ix[p] << 6) | (ix[p + 1] << 4) | (ix[p + 2] << 2) | ix[p + 3]
    struct.pack_into('<H', data, 0x46, crc16(data))
    open(out, 'wb').write(data)
    if preview:
        from PIL import Image
        cols_n = 16
        img = Image.new('RGB', (cols_n * 34, ((len(dex) + cols_n - 1) // cols_n) * 34), (200, 200, 200))
        for i, (pal, ix) in enumerate(pics):
            rgb = [((c >> 8 & 15) * 17, (c >> 4 & 15) * 17, (c & 15) * 17) for c in pal]
            tile = Image.new('RGB', (32, 32))
            tile.putdata([rgb[v] for v in ix])
            img.paste(tile, ((i % cols_n) * 34 + 1, (i // cols_n) * 34 + 1))
        img.resize((img.width * 2, img.height * 2), Image.NEAREST).save(preview)
    print('%s PICTURES: %d portraits, %d blocks' % (ed, len(dex), blocks))


if __name__ == '__main__':
    build(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else None)
