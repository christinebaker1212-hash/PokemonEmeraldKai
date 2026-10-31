#!/usr/bin/env python3
"""Pokémon Platinum's map data (read from the user's ROM through nds.py; nothing from it is committed).

    python3 -I tools/sinnoh/platinum.py ROM matrix 0              # map matrix 0: land data ids per cell
    python3 -I tools/sinnoh/platinum.py ROM land 0                # land data 0: collision / behaviours, props, sizes

A land data file (one 32x32-tile block; pokeplatinum src/overlay005/land_data.c): a header of four sizes (terrain
attributes, props, model, BDHC), then the terrain attributes (u16 a tile: bit 15 collision, low byte the tile
behaviour), the props (48 bytes each: model id, position, rotation, scale as fx32), the block's model (NSBMD) and its
BDHC height data.
"""
import struct
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from nds import Rom  # noqa: E402

LAND = 'fielddata/land_data/land_data.narc'
MATRIX = 'fielddata/mapmatrix/map_matrix.narc'
AREA = 'fielddata/areadata/area_data.narc'
MAP_TEX = 'fielddata/areadata/area_map_tex/map_tex_set.narc'
AREA_BUILD = 'fielddata/areadata/area_build_model/area_build.narc'
AREA_BM_TEX = 'fielddata/areadata/area_build_model/areabm_texset.narc'
BUILD_MODEL = 'fielddata/build_model/build_model.narc'

FX = 4096.0


def fx(v):
    return v / FX


class Matrix:
    def __init__(self, data):
        self.width, self.height, has_headers, has_alt, prefix_len = struct.unpack_from('<BBBBB', data, 0)
        p = 5
        self.prefix = data[p:p + prefix_len].decode('latin-1')
        p += prefix_len
        n = self.width * self.height
        self.headers = None
        self.altitudes = None
        if has_headers:
            self.headers = list(struct.unpack_from('<%dH' % n, data, p))
            p += 2 * n
        if has_alt:
            self.altitudes = list(data[p:p + n])
            p += n
        self.land = list(struct.unpack_from('<%dH' % n, data, p))

    def cell(self, x, y):
        i = y * self.width + x
        return (self.land[i], self.headers[i] if self.headers else None, self.altitudes[i] if self.altitudes else 0)


class Prop:
    def __init__(self, data, p):
        v = struct.unpack_from('<i9i', data, p)
        self.model = v[0]
        self.pos = tuple(fx(x) for x in v[1:4])
        self.rot = tuple(fx(x) for x in v[4:7])
        self.scale = tuple(fx(x) for x in v[7:10])


class Land:
    def __init__(self, data):
        ta, props, model, bdhc = struct.unpack_from('<4i', data, 0)
        p = 16
        self.attrs = list(struct.unpack_from('<1024H', data, p))
        p += ta
        self.props = [Prop(data, p + 48 * i) for i in range(props // 48)]
        p += props
        self.model = data[p:p + model]
        p += model
        self.bdhc = data[p:p + bdhc]
        self.sizes = (ta, props, model, bdhc)

    def collides(self, x, z):
        return bool(self.attrs[z * 32 + x] & 0x8000)

    def behavior(self, x, z):
        return self.attrs[z * 32 + x] & 0xFF


def main():
    rom = Rom(sys.argv[1])
    what, n = sys.argv[2], int(sys.argv[3])
    if what == 'matrix':
        m = Matrix(rom.narc(MATRIX)[n])
        print('matrix %d: %dx%d prefix %r' % (n, m.width, m.height, m.prefix))
        for y in range(m.height):
            print(' '.join('%4s' % ('.' if m.land[y * m.width + x] == 0xFFFF else m.land[y * m.width + x])
                           for x in range(m.width)))
    elif what == 'land':
        land = Land(rom.narc(LAND)[n])
        print('land %d: sizes attrs %d props %d model %d bdhc %d; model %r' % ((n,) + land.sizes + (land.model[:4],)))
        for z in range(32):
            print(''.join('#' if land.collides(x, z) else ('.' if land.behavior(x, z) == 0 else '%x' % (land.behavior(x, z) & 0xF))
                          for x in range(32)))
        for pr in land.props:
            print('prop model %3d at (%.1f %.1f %.1f) rot (%.0f %.0f %.0f) scale (%.2f %.2f %.2f)'
                  % ((pr.model,) + pr.pos + pr.rot + pr.scale))


if __name__ == '__main__':
    main()
