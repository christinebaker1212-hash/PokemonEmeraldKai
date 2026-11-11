#!/usr/bin/env python3
"""UNOVA's ground (docs/UNOVA_HANDOFF.md): a map block's collision grid(s) as each tile's attribute and the height of
the ground at its centre, as the game reads them (aletheia's src/game/ov021/field_g3d_tile_plane.c
FIELD_G3D_MAPPER_GetTilePlane): u16 width, u16 height, then 8-byte tiles: u16 type:2 | normal index:14, u16 distance
index, u32 attribute:31 | diagonal:1. Type 0: one plane; type 2: two (the record past the grid, distIndex-th, 8 bytes:
u16 type:2 | normal A:14, u16 normal B, u16 distance A, u16 distance B), the tile's two triangles split along its
diagonal; other types: flat at 0. The normals (x, y, -z, 4096 = 1) and plane distances (fx32) are overlay 21's tables.

The attribute's low 16 bits are the terrain id (map_attr_types.h: grass, dark grass, water, ledges...), its high
16 bits flags (bit 0: blocked)."""
import functools
import os
import struct
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402

OV021_RAM = 0x02187F00
NORMALS, DISTANCES = 0x021DB9A0, 0x021DC158
CACHE = os.path.expanduser('~/.cache/moemon/unova')


@functools.lru_cache(None)
def _ov021():
    """Overlay 21, decompressed (aletheia's tools/blz.py; BLZ is the NitroSDK's backwards LZ)"""
    out = os.path.join(CACHE, 'ov021.bin')
    if not os.path.exists(out):
        subprocess.check_call([sys.executable, '-I', os.path.join(CACHE, 'aletheia', 'tools', 'blz.py'), 'decompress',
                               os.path.join(narc.EXTRACT, 'overlays', 'y9_0021.bin'), '-o', out],
                              stdout=subprocess.DEVNULL)
    return open(out, 'rb').read()


def normal(i):
    d = _ov021()
    x, y, z = struct.unpack_from('<3h', d, NORMALS - OV021_RAM + 6 * i)
    return x / 4096.0, y / 4096.0, -z / 4096.0


def distance(i):
    return struct.unpack_from('<i', _ov021(), DISTANCES - OV021_RAM + 4 * i)[0] / 4096.0


def layers(block):
    """A map block's collision grids (one in a "WB" block, two in a "GC": the second a bridge or upper floor)"""
    magic = block[:2]
    o = struct.unpack_from('<5I', block, 4)
    if magic == b'WB':
        return [block[o[1]:o[2]]]
    if magic == b'GC':
        return [block[o[1]:o[2]], block[o[2]:o[3]]]
    return []


def grid(layer):
    """[(attribute, height at the centre or None)] for the 32 x 32 tiles, row by row"""
    w, h = struct.unpack_from('<HH', layer, 0)
    out = []
    for tz in range(h):
        for tx in range(w):
            a, dist_i, attr = struct.unpack_from('<HHI', layer, 4 + 8 * (tz * w + tx))
            kind, ni = a & 3, a >> 2
            x, z = tx * 16 - w * 8 + 8.0, tz * 16 - h * 8 + 8.0   # (the tile's centre, from the block's centre)
            if kind == 0:
                planes = [(ni, dist_i)]
            elif kind == 2:
                s = 4 + 8 * w * h + 8 * dist_i
                sa, nb, da, db = struct.unpack_from('<HHHH', layer, s)
                planes = [(sa >> 2, da), (nb, db)]
            else:
                planes = None
            if planes is None:
                height = 0.0
            else:
                hs = []
                for n_i, d_i in planes:
                    nx, ny, nz = normal(n_i)
                    hs.append(-(distance(d_i) + nx * x + nz * z) / ny if ny else 0.0)
                height = sum(hs) / len(hs)
            out.append((attr & 0x7FFFFFFF, round(height, 2)))
    return out
