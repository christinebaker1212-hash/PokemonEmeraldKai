#!/usr/bin/env python3
"""The ball effects' pieces from one DS game's move animations (ds_pieces.py), read from the user's ROM, as the pack the
game loads beside its own art: sinnoh/ballfx_platinum.pak (tools/sinnoh/rom_pack.sh) and unova/ballfx_black.pak
(tools/unova/make_pack.sh). Never committed (sinnoh/ and unova/ are ignored); without them the game draws each piece's
stand-in from its own art.

    python3 -I tools/moemon_ballfx/build_ds_pack.py platinum|black ROM OUT.pak

Format (little-endian): "MFXD", u16 version 1, u16 count; per piece u16 w, u16 h, u32 offset of its pixels (from the
file's start): w*h u16 colors (RGB555), then w*h u8 alphas (0-16).
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ds_particles  # noqa: E402
import ds_pieces  # noqa: E402

PREFIX = {'platinum': 'pl', 'black': 'bw'}


def main():
    game, rom, out = sys.argv[1:4]
    tex = ds_particles.textures(game, rom)
    pieces = [p for p in ds_pieces.PIECES if p[1].startswith(PREFIX[game] + ':')]
    head = bytearray(b'MFXD' + struct.pack('<HH', 1, len(pieces)))
    body = bytearray()
    base = len(head) + 8 * len(pieces)
    table = bytearray()
    for name, key, _ in pieces:
        if key not in tex:
            sys.exit('%s: %s is not in the ROM (another version?)' % (name, key))
        w, h, px = tex[key]
        table += struct.pack('<HHI', w, h, base + len(body))
        for r, g, b, a in px:
            body += struct.pack('<H', (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))
        body += bytes((a * 16 + 127) // 255 for r, g, b, a in px)
        if len(body) % 2:
            body += b'\0'
    open(out, 'wb').write(head + table + body)
    print('%s: %d pieces, %d bytes' % (out, len(pieces), len(head) + len(table) + len(body)))


if __name__ == '__main__':
    main()
