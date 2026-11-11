#!/usr/bin/env python3
"""Offline reference picture of an UNOVA map from Black's own 3D models, at the game's 240x160 (slow; for checking
the readers and, later, src/unova/'s engine against it). Uses SINNOH's offline rasterizer (tools/sinnoh/render_ref.py:
draw_tri, look_matrix) on Black's data: the matrix (a/0/0/9), its blocks (a/0/0/8 "WB" / "GC" files: the model
first), the area's texture set (a/0/1/3 area data -> a/0/1/4).

    python3 -I tools/unova/render_ref.py OUT.png [ZONE [SEASON]]      (default NUVEMA TOWN, spring)
"""
import os
import struct
import sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(os.path.dirname(HERE), 'sinnoh'))
import narc  # noqa: E402
import matrix as mxr  # noqa: E402
import zones as zr  # noqa: E402
import nitro  # noqa: E402
import importlib.util  # noqa: E402
_spec = importlib.util.spec_from_file_location('sinnoh_render_ref', os.path.join(os.path.dirname(HERE), 'sinnoh',
                                                                                 'render_ref.py'))
rr = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(rr)

BLOCK = 512.0
# (until Black's own lights are read: a light from above and a soft ambient)
LIGHT = (0, [(1, (31, 31, 31), (-0.3, -0.85, -0.4)), (0, (0, 0, 0), (0, -1, 0)), (0, (0, 0, 0), (0, -1, 0)),
             (0, (0, 0, 0), (0, -1, 0))], ((20, 20, 20), (12, 12, 12), (0, 0, 0), (0, 0, 0)))


def area(area_id, season=0):
    """Black's area record (a/0/1/3, 10 bytes): model set, texture set, animations, inside / outside, light type"""
    rec = area_id + season if 2 <= area_id < 0xD2 else area_id
    d = open(os.path.join(narc.EXTRACT, 'data', 'a/0/1/3'), 'rb').read()[rec * 10:rec * 10 + 10]
    model_set, tex_set, ita, itp, inner, light, edge, bbd = struct.unpack('<HHBBBBBB', d)
    return {'model_set': model_set, 'tex_set': tex_set, 'ita': ita, 'itp': itp, 'inner': inner, 'light': light}


def block_model(data):
    """A map block's model (its first section: a BMD0)"""
    return data[struct.unpack_from('<I', data, 4)[0]:struct.unpack_from('<I', data, 8)[0]]


def building_set(area_id, inner):
    """{table id: BMD0 bytes} and the textures of an area's buildings (field_buildmodel.c: archive 0xE5 / 0xE6,
    a/2/2/9 inside, a/2/3/0 outside; textures 0xB0 / 0xB1, a/1/7/6 / a/1/7/7; the member (area - 2) / 4 for the
    seasons' areas, area - 0xD2 past them)"""
    member = area_id - 0xD2 if area_id >= 0xD2 else (area_id - 2) // 4 if area_id >= 2 else 0
    arc = narc.arc('a/2/2/9' if inner else 'a/2/3/0')[member]
    tex = nitro.btx0(narc.arc('a/1/7/6' if inner else 'a/1/7/7')[member])
    n = arc[2] // 2
    models = {}
    for i in range(n):
        d = struct.unpack_from('<I', arc, 4 + 4 * i)[0]
        r = struct.unpack_from('<I', arc, 4 + 4 * (n + i))[0]
        end = struct.unpack_from('<I', arc, 4 + 4 * (n + i + 1))[0] if i + 1 < n else len(arc)
        models[struct.unpack_from('<H', arc, d)[0]] = arc[r:end]
    return models, tex


def block_buildings(data):
    """A map block's placed buildings: (x, y, z, rotation in 1/65536 turns, table id) (field_buildmodel.c
    BuildModelObjectDesc, after a u32 count; FIELD_BUILDMODEL_RegisterObjects adds x, y and takes z)"""
    o = struct.unpack_from('<4I', data, 8)
    sec = data[o[1]:o[2]] if data[:2] == b'WB' else data[o[2]:o[3]]
    out = []
    for i in range(struct.unpack_from('<I', sec, 0)[0] if len(sec) >= 4 else 0):
        x, y, z, rot, hi, lo = struct.unpack_from('<iiiHBB', sec, 4 + 16 * i)
        out.append((x / 4096.0, y / 4096.0, -z / 4096.0, rot, (hi << 8) | lo))
    return out


def render(out, zone_id=389, season=0):
    zs = zr.zones()
    z = zs[zone_id]
    mx = mxr.matrix(z['matrix'], zone_id)
    cells = [i for i in range(mx['w'] * mx['h']) if mx['zones'][i] == zone_id] or [0]
    cx, cz = cells[0] % mx['w'], cells[0] // mx['w']
    tex = nitro.btx0(narc.arc('a/0/1/4')[area(z['area'], season)['tex_set']])
    blocks = narc.arc('a/0/0/8')
    target = np.array([cx * BLOCK + 256.0, 0.0, cz * BLOCK + 256.0])
    view = rr.look_matrix(target)
    color = np.zeros((rr.H, rr.W, 3), dtype=np.float32)
    depth = np.full((rr.H, rr.W), np.inf, dtype=np.float32)
    ids = np.full((rr.H, rr.W), -1, dtype=np.int16)
    sc = rr.Scene(None)
    bsets = {}
    tris = []
    for bz in range(cz - 1, cz + 2):
        for bx in range(cx - 1, cx + 2):
            if not (0 <= bx < mx['w'] and 0 <= bz < mx['h']):
                continue
            b = mx['blocks'][bz * mx['w'] + bx]
            if b == mxr.NONE:
                continue
            ztex = tex
            bzone = mx['zones'][bz * mx['w'] + bx]
            if bzone < len(zs) and zs[bzone]['area'] != z['area']:
                ztex = nitro.btx0(narc.arc('a/0/1/4')[area(zs[bzone]['area'], season)['tex_set']])
            centre = np.array([bx * BLOCK + 256.0, 0.0, bz * BLOCK + 256.0])
            for model in nitro.Bmd0(block_model(blocks[b])).models:
                for mi, ts in model.draw().items():
                    mat = model.materials[mi]
                    tris.append((centre, None, ts, sc.texture(ztex, 'map', mat), mat))
            bz_area = zs[bzone]['area'] if bzone < len(zs) else z['area']
            if bz_area not in bsets:
                bsets[bz_area] = building_set(bz_area, area(bz_area, season)['inner'])
            bmodels, btex = bsets[bz_area]
            for x, y, zz, rot, tid in block_buildings(blocks[b]):
                if tid not in bmodels:
                    continue
                bmd = nitro.Bmd0(bmodels[tid])
                a = rot / 65536.0 * 2 * np.pi
                r = np.array([[np.cos(a), 0, np.sin(a)], [0, 1, 0], [-np.sin(a), 0, np.cos(a)]])
                for model in bmd.models:
                    for mi, ts in model.draw().items():
                        mat = model.materials[mi]
                        tris.append((centre + np.array([x, y, zz]), r, ts, sc.texture(bmd.tex or btex, 'bm', mat), mat))
    opaque = [t for t in tris if t[4].alpha == 31]
    clear = [t for t in tris if t[4].alpha != 31]
    for group in (opaque, clear):
        for origin, rot, ts, img, mat in group:
            for tri in ts:
                rr.draw_tri(tri, origin, rot, img, mat, view, LIGHT, color, depth, ids)
    Image.fromarray(np.clip(color * 255.0 / 31.0, 0, 255).astype(np.uint8), 'RGB').save(out)
    print('saved', out, 'block', cx, cz, 'triangles', sum(len(t[2]) for t in tris))


if __name__ == '__main__':
    render(sys.argv[1], int(sys.argv[2]) if len(sys.argv) > 2 else 389, int(sys.argv[3]) if len(sys.argv) > 3 else 0)
