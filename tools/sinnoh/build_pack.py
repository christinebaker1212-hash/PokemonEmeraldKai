#!/usr/bin/env python3
"""The SINNOH 3D pack: Platinum's map blocks, their props and textures, flattened for the game's 3D engine
(src/sinnoh/s3d.c reads it; docs/SINNOH_HANDOFF.md).

    python3 -I tools/sinnoh/build_pack.py POKEPLATINUM_DIR OUT.pak [X0 Z0 X1 Z1]
    python3 -I tools/sinnoh/build_pack.py --rom ROM POKEPLATINUM_DIR OUT.pak [...]   (the same from the cartridge)
        the cells of map matrix 0 from (X0, Z0) to (X1, Z1) (default: all of it)

POKEPLATINUM_DIR is a pret/pokeplatinum checkout (source.py): it holds every map block, texture set, prop model and
light set Platinum has, so no ROM is needed; the release builds the pack from it (tools/sinnoh/fetch_decomp.sh). Built
from the ROM instead, the pack is the same, byte for byte (tools/sinnoh/test_poc.sh checks it when given a ROM).

Format (little endian), version 2:
  "S3DP" u32 version
  u32 textures:  each u16 w, u16 h, then w*h u32 RGBA (alpha 0-255)
  u32 meshes:    each f32 centre x y z, f32 radius, u32 groups:
                   each group: s16 texture (-1 none), u8 repeat S, repeat T, flip S, flip T, alpha (0-31), polygon id,
                   cull (0 none, 1 back faces, 2 front faces, 3 all), lights (bits), mode, pad; u32 triangles;
                   then 3 vertices each: f32 x y z, f32 u v (texels), u8 r g b (0-31) pad, f32 nx ny nz
  u32 light sets: each u32 blocks: u32 end time (2-second units of the day), 4 lights (u8 on r g b, f32 dx dy dz),
                   then diffuse, ambient, specular, emission (u8 r g b each), u8 pad x 4
  matrix: u16 width, height, u16 x0, z0 (the cells kept start here); per kept cell (row by row):
          s16 mesh (-1 none), s16 altitude, u16 light set, u16 props; props: s16 mesh, f32 3x4 transform (rows);
          then the cell's 32x32 map blocks (u16, the field engine's: metatile | collision << 10 | elevation << 12)
          from Platinum's terrain attributes (bit 15 collision, low byte the behavior: Platinum's behaviors share
          Emerald's numbers for the common ones, so the metatile is one of the General tileset's with that behavior);
          then the ground's heights at each tile's centre (row by row): u8 count (0-3) per tile, then f32 heights x3
          per tile, lowest first (more than one: a bridge over the ground). From the block's BDHC (its plates: a
          rectangle in x, z around the block's centre and the plane the ground follows there); world heights.
"""
import hashlib
import math
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nitro  # noqa: E402
from platinum import Land  # noqa: E402
from source import open_source, NONE  # noqa: E402

VERSION = 2

# Platinum behavior -> a General tileset metatile with the same Emerald behavior (tall grass, water, sand, ledges...)
BEHAVIOR_METATILE = {0x02: 0x00D, 0x03: 0x015, 0x10: 0x02D, 0x15: 0x02C, 0x16: 0x0C8, 0x17: 0x13B, 0x21: 0x11C,
                     0x38: 0x086, 0x39: 0x085, 0x3B: 0x087}
WATER = (0x10, 0x15)


def heights(land):
    """The ground's heights at each tile's centre from the block's BDHC: every plate whose rectangle holds the point
    gives the height of its plane there (x, z from the block's centre, 16 units a tile). Up to 3, lowest first."""
    b = land.bdhc
    out = [[] for _ in range(1024)]
    if len(b) < 16 or b[:4] != b'BDHC':
        return out
    npt, nnrm, ncon, npl = struct.unpack_from('<4H', b, 4)
    p = 16
    pts = [struct.unpack_from('<2i', b, p + 8 * i) for i in range(npt)]
    p += 8 * npt
    nrm = [struct.unpack_from('<3i', b, p + 12 * i) for i in range(nnrm)]
    p += 12 * nnrm
    con = [struct.unpack_from('<i', b, p + 4 * i)[0] for i in range(ncon)]
    p += 4 * ncon
    plates = []
    for i in range(npl):
        a, c, ni, ci = struct.unpack_from('<4H', b, p + 8 * i)
        if a >= npt or c >= npt or ni >= nnrm or ci >= ncon or nrm[ni][1] == 0:
            continue
        (ax, az), (cx, cz) = pts[a], pts[c]
        plates.append((min(ax, cx) / 4096, max(ax, cx) / 4096, min(az, cz) / 4096, max(az, cz) / 4096,
                       nrm[ni][0] / 4096, nrm[ni][1] / 4096, nrm[ni][2] / 4096, con[ci] / 4096))
    for tz in range(32):
        z = tz * 16 - 256 + 8
        for tx in range(32):
            x = tx * 16 - 256 + 8
            hs = []
            for x0, x1, z0, z1, nx, ny, nz, k in plates:
                if x0 <= x <= x1 and z0 <= z <= z1:
                    h = round(-(nx * x + nz * z + k) / ny, 2)
                    if all(abs(h - o) > 2 for o in hs):
                        hs.append(h)
            out[tz * 32 + tx] = sorted(hs)[:3]
    return out


def blocks(land):
    out = []
    for a in land.attrs:
        b = a & 0xFF
        mt = BEHAVIOR_METATILE.get(b, 0x001)
        if b in WATER:
            out.append(mt | (1 << 12))                       # surfable: no collision, elevation 1
        else:
            out.append(mt | ((1 if a & 0x8000 else 0) << 10) | (3 << 12))
    return out


class Pack:
    def __init__(self):
        self.textures = []
        self.tex_ids = {}
        self.meshes = []
        self.mesh_ids = {}

    def texture(self, tex0, mat):
        if tex0 is None or not mat.texture or mat.texture not in tex0.textures:
            return -1
        if not hasattr(tex0, 'digest'):   # (by content: an object's id() is reused once it's freed)
            tex0.digest = hashlib.sha1(tex0.d[tex0.o:]).digest()
        key = (tex0.digest, mat.texture, mat.palette)
        if key not in self.tex_ids:
            w, h, px = tex0.decode(mat.texture, mat.palette)
            self.tex_ids[key] = len(self.textures)
            self.textures.append((w, h, px))
        return self.tex_ids[key]

    def mesh(self, key, bmd, tex0):
        if key in self.mesh_ids:
            return self.mesh_ids[key]
        groups = []
        pts = []
        for model in bmd.models:
            for mi, tris in sorted(model.draw().items()):
                mat = model.materials[mi]
                cull = {'none': 0, 'back': 1, 'front': 2, 'all': 3}[mat.cull]
                rs, rt, fs, ft = mat.repeat
                groups.append((self.texture(tex0, mat), rs, rt, fs, ft, mat.alpha, (mat.poly_attr >> 24) & 63, cull,
                               mat.lights, mat.mode, tris))
                pts.extend(v[0] for t in tris for v in t)
        if pts:
            c = [sum(p[i] for p in pts) / len(pts) for i in range(3)]
            r = max(math.dist(c, p) for p in pts)
        else:
            c, r = [0.0, 0.0, 0.0], 0.0
        self.mesh_ids[key] = len(self.meshes)
        self.meshes.append((c, r, groups))
        return self.mesh_ids[key]


def write(path, pack, light_sets, cells, window):
    out = bytearray(b'S3DP' + struct.pack('<I', VERSION))
    out += struct.pack('<I', len(pack.textures))
    for w, h, px in pack.textures:
        out += struct.pack('<HH', w, h)
        out += b''.join(struct.pack('<BBBB', *p) for p in px)
    out += struct.pack('<I', len(pack.meshes))
    for c, r, groups in pack.meshes:
        out += struct.pack('<4fI', c[0], c[1], c[2], r, len(groups))
        for tex, rs, rt, fs, ft, alpha, pid, cull, lights, mode, tris in groups:
            out += struct.pack('<hBBBBBBBBBBI', tex, rs, rt, fs, ft, alpha, pid, cull, lights, mode, 0, len(tris))
            for tri in tris:
                for pos, uv, col, nrm in tri:
                    out += struct.pack('<5f4B3f', pos[0], pos[1], pos[2], uv[0], uv[1], col[0], col[1], col[2], 0,
                                       nrm[0], nrm[1], nrm[2])
    out += struct.pack('<I', len(light_sets))
    for blocks in light_sets:
        out += struct.pack('<I', len(blocks))
        for t, lights, cols in blocks:
            out += struct.pack('<I', t)
            for on, lc, ld in lights:
                out += struct.pack('<4B3f', on, lc[0], lc[1], lc[2], ld[0], ld[1], ld[2])
            for col in cols:
                out += struct.pack('<3B', *col)
            out += b'\0' * 4
    x0, z0, x1, z1 = window
    out += struct.pack('<4H', x1 - x0 + 1, z1 - z0 + 1, x0, z0)
    for mesh, alt, light, props, blk, hts in cells:
        out += struct.pack('<hhHH', mesh, alt, light, len(props))
        for pm, m in props:
            out += struct.pack('<h12f', pm, *m)
        out += struct.pack('<1024H', *blk)
        out += bytes(len(h) for h in hts)
        for h in hts:
            out += struct.pack('<3f', *(h + [0.0] * (3 - len(h))))
    open(path, 'wb').write(out)
    return len(out)


def main():
    args = sys.argv[1:]
    rom = None
    if args[0] == '--rom':
        rom, args = args[1], args[2:]
    src = open_source(rom, decomp=args[0]) if rom else open_source(args[0])
    out_path = args[1]
    mx = src.matrix(0)
    # the cells of map matrix 0 from (X0, Z0) to (X1, Z1); default: all of it
    window = [int(x) for x in args[2:6]] if len(args) >= 6 else [0, 0, mx.width - 1, mx.height - 1]
    x0, z0, x1, z1 = window
    pack = Pack()
    light_index = {}
    light_sets = []
    tex_cache = {}
    cells = []
    for z in range(z0, z1 + 1):
        for x in range(x0, x1 + 1):
            lid, header, alt = mx.cell(x, z)
            if lid == NONE:
                cells.append((-1, 0, 0, [], [0x0001 | (1 << 10) | (3 << 12)] * 1024, [[] for _ in range(1024)]))
                continue
            prop_set, tex_set, light_set = src.area_of(header)
            if light_set not in light_index:
                light_index[light_set] = len(light_sets)
                light_sets.append(src.lights(light_set))
            if ('map', tex_set) not in tex_cache:
                tex_cache[('map', tex_set)] = nitro.btx0(src.map_textures(tex_set))
            if ('bm', prop_set) not in tex_cache:
                tex_cache[('bm', prop_set)] = nitro.btx0(src.prop_textures(prop_set))
            land = Land(src.land(lid))
            mesh = pack.mesh(('land', lid, tex_set), nitro.Bmd0(land.model), tex_cache[('map', tex_set)])
            props = []
            for pr in land.props:
                if pr.model >= src.prop_count():
                    continue
                bmd = nitro.Bmd0(src.prop_model(pr.model))
                ptex = bmd.tex or tex_cache[('bm', prop_set)]
                pm = pack.mesh(('prop', pr.model, prop_set), bmd, ptex)
                ry = math.radians(pr.rot[1])
                cy, sy = math.cos(ry), math.sin(ry)
                sx, sy_, sz = pr.scale
                m = [cy * sx, 0.0, sy * sz, pr.pos[0],
                     0.0, sy_, 0.0, pr.pos[1],
                     -sy * sx, 0.0, cy * sz, pr.pos[2]]
                props.append((pm, m))
            cells.append((mesh, alt, light_index[light_set], props, blocks(land), heights(land)))
            print('cell (%d,%d): land %d %s (textures %d, props %d, lights %d), %d props'
                  % (x, z, lid, header, tex_set, prop_set, light_set, len(props)))
    size = write(out_path, pack, light_sets, cells, window)
    print('%s: %d textures, %d meshes, %d light sets, %d bytes' % (out_path, len(pack.textures), len(pack.meshes),
                                                                 len(light_sets), size))


if __name__ == '__main__':
    main()
