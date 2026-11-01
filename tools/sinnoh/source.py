#!/usr/bin/env python3
"""Where SINNOH's data comes from: a pret/pokeplatinum checkout (the default: it holds every map block, texture set,
prop model and light set as files, so no ROM is needed; the release builds the 3D pack from it) or a Platinum ROM
(the same files, byte for byte, inside the cartridge's NARCs; kept for cross-checks). Both answer the same questions:

    src = open_source(PATH)          # a directory: pokeplatinum; a file: a ROM (then POKEPLATINUM is still needed for
                                     # the map header table: open_source(ROM, decomp=DIR))
    src.matrix(0)                    # .width, .height, .cell(x, z) -> (land id or 0xFFFF, header name, altitude)
    src.land(id) -> bytes            # a land data file (platinum.Land)
    src.area_of(header name) -> (prop set, texture set, light set)
    src.map_textures(set) / src.prop_textures(set) / src.prop_model(id) -> bytes (BTX0 / BMD0)
    src.lights(set) -> [(end time, [(on, (r, g, b), (dx, dy, dz)) x 4], [diffuse, ambient, specular, emission])]
"""
import json
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

NONE = 0xFFFF


def _order(path):
    return [ln.strip() for ln in open(path) if ln.strip()]


def _num(name):
    return int(re.search(r'(\d+)$', name).group(1))


class _Matrix:
    def __init__(self, width, height, land, headers, alt):
        self.width, self.height = width, height
        self._land, self._headers, self._alt = land, headers, alt

    def cell(self, x, z):
        i = z * self.width + x
        return self._land[i], self._headers[i], self._alt[i]


class Decomp:
    def __init__(self, root):
        self.root = root
        r = lambda *p: os.path.join(root, *p)   # noqa: E731
        self.headers = _order(r('generated/map_headers.txt'))
        text = open(r('include/data/map_headers.h')).read()
        self._area = {m.group(1): int(m.group(2)) for m in
                      re.finditer(r'\[(MAP_HEADER_\w+)\] = \{\s*\.areaDataArchiveID = area_data_(\d+)', text)}
        self._matrices = _order(r('res/field/matrices/map_matrices.order'))
        self._land = _order(r('res/field/maps/data/map_data.order'))
        self._map_tex = [n for n in _order(r('res/field/maps/texture_sets/map_texture_sets.order'))]
        self._props = _order(r('res/field/props/models/map_prop_models.order'))

    def _path(self, *p):
        return os.path.join(self.root, *p)

    def matrix(self, n):
        m = json.load(open(self._path('res/field/matrices', self._matrices[n] + '.json')))
        h, w = len(m['maps']), len(m['maps'][0])
        land, headers, alt = [], [], []
        for z in range(h):
            for x in range(w):
                name = m['maps'][z][x]
                land.append(NONE if name == 'MAP_NONE' else _num(name))
                # (a building's matrix has no per-cell headers or altitudes: the header that uses it gives its area)
                hs, al = m.get('headers') or [], m.get('altitudes') or []
                headers.append(hs[z][x] if z < len(hs) and x < len(hs[z]) else None)
                alt.append(al[z][x] if z < len(al) and x < len(al[z]) else 0)
        return _Matrix(w, h, land, headers, alt)

    def land(self, n):
        return open(self._path('res/field/maps/data', self._land[n]), 'rb').read()

    def area_of(self, header):
        a = json.load(open(self._path('res/field/area_data', 'area_data_%03d.json' % self._area.get(header, 0))))
        return _num(a['mapPropSet']), _num(a['mapTextureSet']), _num(a['lightingSet'])

    def map_textures(self, n):
        name = self._map_tex[n]
        if not name.endswith('.nsbtx'):
            name += '.nsbtx'
        return open(self._path('res/field/maps/texture_sets', name), 'rb').read()

    def prop_textures(self, n):
        return open(self._path('res/field/props/texture_sets', 'prop_texture_set_%03d.nsbtx' % n), 'rb').read()

    def prop_count(self):
        return len(self._props)

    def prop_model(self, n):
        return open(self._path('res/field/props/models', self._props[n]), 'rb').read()

    def field_effect(self, name):
        """A field effect model of Platinum's (res/graphics/field_effects; None: not fetched)"""
        path = self._path('res/graphics/field_effects', name)
        return open(path, 'rb').read() if os.path.exists(path) else None

    def lights(self, n):
        blocks = []
        for b in json.load(open(self._path('res/field/lighting', 'lighting_set_%03d.json' % n))):
            lights = [(1 if l['enabled'] else 0, (l['color']['red'], l['color']['green'], l['color']['blue']),
                       (l['direction']['x'], l['direction']['y'], l['direction']['z'])) for l in b['lights']]
            cols = [(b[k]['red'], b[k]['green'], b[k]['blue'])
                    for k in ('diffuseColor', 'ambientColor', 'specularColor', 'emissionColor')]
            blocks.append((b['endTime'], lights, cols))
        return blocks


class RomSource:
    """The same answers from the cartridge (pokeplatinum still gives the header names and areas)."""

    def __init__(self, rom_path, decomp):
        from nds import Rom
        self.rom = Rom(rom_path)
        self.decomp = decomp
        self._narc = {}

    def _n(self, path):
        if path not in self._narc:
            self._narc[path] = self.rom.narc(path)
        return self._narc[path]

    def matrix(self, n):
        from platinum import Matrix, MATRIX
        m = Matrix(self._n(MATRIX)[n])
        names = self.decomp.headers
        headers = [names[h] if m.headers and h < len(names) else None for h in (m.headers or [0] * len(m.land))]
        return _Matrix(m.width, m.height, m.land, headers, m.altitudes or [0] * len(m.land))

    def land(self, n):
        from platinum import LAND
        return self._n(LAND)[n]

    def area_of(self, header):
        from platinum import AREA
        area = self.decomp._area.get(header, 0)
        return struct.unpack_from('<HHH', self._n(AREA)[area], 0)

    def map_textures(self, n):
        from platinum import MAP_TEX
        return self._n(MAP_TEX)[n]

    def prop_textures(self, n):
        from platinum import AREA_BM_TEX
        return self._n(AREA_BM_TEX)[n]

    def prop_count(self):
        from platinum import BUILD_MODEL
        return len(self._n(BUILD_MODEL))

    def prop_model(self, n):
        from platinum import BUILD_MODEL
        return self._n(BUILD_MODEL)[n]

    def field_effect(self, name):
        return self.decomp.field_effect(name)

    def lights(self, n):
        from render_ref import parse_lights
        return parse_lights(self._n('data/arealight.narc')[n].decode('latin-1'))


def open_source(path, decomp=None):
    if os.path.isdir(path):
        return Decomp(path)
    return RomSource(path, Decomp(decomp))
