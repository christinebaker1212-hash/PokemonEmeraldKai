#!/usr/bin/env python3
"""UNOVA's map matrices (docs/UNOVA_HANDOFF.md): a/0/0/9, as aletheia's src/game/ov010/map_matrix.c
MAP_MATRIX_SetupFromFile reads one: u16 mode, u16, u16 width, u16 height, then width * height u32 map blocks
(a/0/0/8 indices, 0xFFFFFFFF none) and, in mode 1, width * height u32 zone ids (else every block is the zone's).
A block is 32 x 32 tiles of 16 units (field_g3d_tile_plane.c).

    python3 -I tools/unova/matrix.py N         (matrix N's zones as a picture)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402

NONE = 0xFFFFFFFF


def matrix(n, zone=0xFFFF):
    """{w, h, blocks [row-major], zones [row-major]}"""
    m = narc.arc('a/0/0/9')[n]
    mode, _, w, h = struct.unpack_from('<HHHH', m, 0)
    blocks = list(struct.unpack_from('<%dI' % (w * h), m, 8))
    if mode == 1:
        zones = [v & 0xFFFF for v in struct.unpack_from('<%dI' % (w * h), m, 8 + 4 * w * h)]
    else:
        zones = [zone] * (w * h)
    return {'w': w, 'h': h, 'mode': mode, 'blocks': blocks, 'zones': zones}


def main():
    mx = matrix(int(sys.argv[1]))
    for y in range(mx['h']):
        print(''.join('   .' if mx['blocks'][y * mx['w'] + x] == NONE else '%4d' % mx['zones'][y * mx['w'] + x]
                      for x in range(mx['w'])))


if __name__ == '__main__':
    main()
