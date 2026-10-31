#!/usr/bin/env python3
"""Offline reference picture of a Platinum map with its own 3D models, at the game's 240x160 (slow, for checking).

    python3 -I tools/sinnoh/render_ref.py ROM OUT.png [MATRIX TILE_X TILE_Z [HOUR]]
        default: Twinleaf Town (matrix 0, the player's house door), noon

The camera is Platinum's default field camera (pokeplatinum src/overlay005/field_camera.c: 666.92 units away, 59.05
degrees down) at the DS's own pixel scale (one unit a pixel at the target: 16 pixels a tile, as Emerald's maps), so
240x160 shows a narrower slice than the DS's 256x192. The in-game engine (src/sinnoh/) must match this picture.
"""
import math
import os
import struct
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from nds import Rom  # noqa: E402
import nitro  # noqa: E402
from platinum import (Land, Matrix, LAND, MATRIX, AREA, MAP_TEX, AREA_BUILD, AREA_BM_TEX, BUILD_MODEL)  # noqa: E402

W, H = 240, 160
DIST = 666.922119140625
PITCH = math.radians(59.051513671875)
HALF_FOV_DS = math.radians(8.0914306640625)
FOCAL = 96.0 / math.tan(HALF_FOV_DS)   # pixels: the DS's 192-pixel height spans 2 * tan(half fov)
BLOCK = 512.0                          # 32 tiles of 16 units


def parse_lights(text):
    """Platinum's area light file: blocks of (end time, 4 lights, diffuse, ambient, specular, emission)."""
    blocks = []
    lines = [ln.strip().rstrip(',') for ln in text.replace('\r', '').split('\n')]
    i = 0
    while i < len(lines):
        if not lines[i]:
            i += 1
            continue
        if lines[i].startswith('EOF'):
            break
        t = int(lines[i].split(',')[0])
        lights = []
        for k in range(4):
            v = [int(x) for x in lines[i + 1 + k].split(',')]
            lights.append((v[0], (v[1], v[2], v[3]), (v[4] / 4096.0, v[5] / 4096.0, v[6] / 4096.0)))
        cols = [tuple(int(x) for x in lines[i + 5 + k].split(',')) for k in range(4)]
        blocks.append((t, lights, cols))
        i += 9
    return blocks


class Scene:
    def __init__(self, rom):
        self.rom = rom
        self.land = rom.narc(LAND)
        self.texsets = rom.narc(MAP_TEX)
        self.builds = rom.narc(BUILD_MODEL)
        self.bm_tex = rom.narc(AREA_BM_TEX)
        self.area = rom.narc(AREA)
        self.lights = rom.narc('data/arealight.narc')
        self.tex_cache = {}

    def texture(self, tex0, key, mat):
        k = (id(tex0), mat.texture, mat.palette)
        if k not in self.tex_cache:
            if tex0 is None or mat.texture not in tex0.textures:
                self.tex_cache[k] = None
            else:
                w, h, px = tex0.decode(mat.texture, mat.palette)
                self.tex_cache[k] = np.array(px, dtype=np.uint8).reshape(h, w, 4)
        return self.tex_cache[k]


def look_matrix(target):
    """World -> camera (x right, y up, z towards the viewer: things in front have negative z)."""
    cp, sp = math.cos(PITCH), math.sin(PITCH)
    eye = np.array([target[0], target[1] + DIST * sp, target[2] + DIST * cp])
    f = np.array(target) - eye
    f /= np.linalg.norm(f)
    up = np.array([0.0, 1.0, 0.0])
    r = np.cross(f, up)
    r /= np.linalg.norm(r)
    u = np.cross(r, f)
    m = np.eye(4)
    m[0, :3], m[1, :3], m[2, :3] = r, u, -f
    m[:3, 3] = -m[:3, :3] @ eye
    return m


def lit(color_mats, light_block, normal, vtx_color):
    _, lights, (dif, amb, spe, emi) = light_block
    acc = np.array(emi, dtype=float)
    for on, lc, ld in lights:
        if not on:
            continue
        lc = np.array(lc, dtype=float)
        level = max(0.0, -float(np.dot(ld, normal)))
        acc += np.array(dif) * lc * level / 31.0 + np.array(amb) * lc / 31.0
    return np.minimum(acc, 31.0)


def render(rom, out, matrix_id=0, tile_x=None, tile_z=None, hour=12):
    sc = Scene(rom)
    mx = Matrix(sc.rom.narc(MATRIX)[matrix_id])
    if tile_x is None:
        tile_x, tile_z = 3 * 32 + 12, 27 * 32 + 9   # Twinleaf Town, by the player's house
    area_id = 6
    a = sc.area[area_id]
    prop_set, tex_set, light_set = struct.unpack_from('<HHH', a, 0)
    tex0 = nitro.btx0(sc.texsets[tex_set])
    bm_tex0 = nitro.btx0(sc.bm_tex[prop_set])
    blocks = parse_lights(sc.lights[light_set].decode('latin-1'))
    secs = hour * 3600 // 2
    lb = next((b for b in blocks if b[0] > secs), blocks[-1])

    target = np.array([tile_x * 16 + 8.0, 0.0, tile_z * 16 + 8.0])
    view = look_matrix(target)
    color = np.zeros((H, W, 3), dtype=np.float32)
    color[:] = (0, 0, 0)
    depth = np.full((H, W), np.inf, dtype=np.float32)
    ids = np.full((H, W), -1, dtype=np.int16)   # the translucent polygon id last drawn (the DS draws one a pixel)

    tris = []   # (world verts, uvs, colors, texture image, material)
    bx0, bz0 = tile_x // 32, tile_z // 32
    for bz in range(bz0 - 1, bz0 + 2):
        for bx in range(bx0 - 1, bx0 + 2):
            if not (0 <= bx < mx.width and 0 <= bz < mx.height):
                continue
            lid, _, alt = mx.cell(bx, bz)
            if lid == 0xFFFF:
                continue
            land = Land(sc.land[lid])
            centre = np.array([bx * BLOCK + 256.0, alt * 16.0, bz * BLOCK + 256.0])
            for model in nitro.Bmd0(land.model).models:
                for mi, ts in model.draw().items():
                    mat = model.materials[mi]
                    tris.append((centre, None, ts, sc.texture(tex0, 'map', mat), mat))
            for pr in land.props:
                if pr.model >= len(sc.builds):
                    continue
                bmd = nitro.Bmd0(sc.builds[pr.model])
                ptex = bmd.tex or bm_tex0
                ry = math.radians(pr.rot[1])
                rot = np.array([[math.cos(ry), 0, math.sin(ry)], [0, 1, 0], [-math.sin(ry), 0, math.cos(ry)]])
                place = (centre + np.array(pr.pos), rot @ np.diag(pr.scale))
                for model in bmd.models:
                    for mi, ts in model.draw().items():
                        mat = model.materials[mi]
                        tris.append((place[0], place[1], ts, sc.texture(ptex, 'bm', mat), mat))

    opaque, clear = [], []
    for item in tris:
        (opaque if item[4].alpha == 31 else clear).append(item)
    for group in (opaque, clear):
        for origin, rot, ts, img, mat in group:
            for tri in ts:
                draw_tri(tri, origin, rot, img, mat, view, lb, color, depth, ids)
    img = Image.fromarray(np.clip(color * 255.0 / 31.0, 0, 255).astype(np.uint8), 'RGB')
    img.save(out)
    print('saved', out, 'triangles', sum(len(t[2]) for t in tris))


def draw_tri(tri, origin, rot, img, mat, view, lb, color, depth, ids):
    pts = []
    for pos, uv, vcol, nrm in tri:
        p = np.array(pos, dtype=float)
        n = np.array(nrm, dtype=float)
        if rot is not None:
            p = rot @ p
            n = rot @ n
        p = p + origin
        c = view @ np.array([p[0], p[1], p[2], 1.0])
        if c[2] > -1.0:
            return
        sx = W / 2 + FOCAL * c[0] / -c[2]
        sy = H / 2 - FOCAL * c[1] / -c[2]
        ln = np.linalg.norm(n)
        if mat.lights and ln > 0:
            col = lit(mat, lb, n / ln, vcol)
        else:
            col = np.array(vcol, dtype=float)
        pts.append((sx, sy, -c[2], uv, col))
    (x0, y0, z0, uv0, c0), (x1, y1, z1, uv1, c1), (x2, y2, z2, uv2, c2) = pts
    area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)
    if area == 0:
        return
    cull = mat.cull
    if cull == 'back' and area > 0:
        return
    if cull == 'front' and area < 0:
        return
    minx, maxx = max(0, int(math.floor(min(x0, x1, x2)))), min(W - 1, int(math.ceil(max(x0, x1, x2))))
    miny, maxy = max(0, int(math.floor(min(y0, y1, y2)))), min(H - 1, int(math.ceil(max(y0, y1, y2))))
    if minx > maxx or miny > maxy:
        return
    xs, ys = np.meshgrid(np.arange(minx, maxx + 1) + 0.5, np.arange(miny, maxy + 1) + 0.5)
    w0 = ((x1 - xs) * (y2 - ys) - (x2 - xs) * (y1 - ys)) / area
    w1 = ((x2 - xs) * (y0 - ys) - (x0 - xs) * (y2 - ys)) / area
    w2 = 1.0 - w0 - w1
    inside = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
    if not inside.any():
        return
    iz = w0 / z0 + w1 / z1 + w2 / z2
    z = 1.0 / np.where(inside, iz, 1.0)
    reg_depth = depth[miny:maxy + 1, minx:maxx + 1]
    vis = inside & (z < reg_depth)
    if not vis.any():
        return
    pc = (w0 / z0)[..., None], (w1 / z1)[..., None], (w2 / z2)[..., None]
    zz = z[..., None]
    shade = (pc[0] * c0 + pc[1] * c1 + pc[2] * c2) * zz
    alpha = np.full(xs.shape, 1.0)
    if img is not None:
        u = (pc[0][..., 0] * uv0[0] + pc[1][..., 0] * uv1[0] + pc[2][..., 0] * uv2[0]) * z
        v = (pc[0][..., 0] * uv0[1] + pc[1][..., 0] * uv1[1] + pc[2][..., 0] * uv2[1]) * z
        th, tw = img.shape[:2]
        rs, rt, fs, ft = mat.repeat
        tu = wrap(np.floor(u).astype(int), tw, rs, fs)
        tv = wrap(np.floor(v).astype(int), th, rt, ft)
        texel = img[tv, tu].astype(np.float32)
        vis &= texel[..., 3] > 0
        rgb = texel[..., :3] * 31.0 / 255.0
        out = (rgb + 1) * (shade + 1) / 32.0 - 1 if mat.mode == 0 else rgb
        alpha = texel[..., 3] / 255.0
    else:
        out = shade
    a = alpha * (mat.alpha / 31.0)
    pid = (mat.poly_attr >> 24) & 63
    reg_ids = ids[miny:maxy + 1, minx:maxx + 1]
    if mat.alpha < 31:
        vis &= reg_ids != pid
    reg = color[miny:maxy + 1, minx:maxx + 1]
    blend = out * a[..., None] + reg * (1 - a[..., None])
    reg[vis] = blend[vis]
    if mat.alpha == 31:
        reg_depth[vis] = z[vis]
        reg_ids[vis] = -1
    else:
        reg_ids[vis] = pid


def wrap(i, size, repeat, flip):
    if not repeat:
        return np.clip(i, 0, size - 1)
    if flip:
        m = np.mod(i, 2 * size)
        return np.where(m >= size, 2 * size - 1 - m, m)
    return np.mod(i, size)


def main():
    rom = Rom(sys.argv[1])
    args = [int(x) for x in sys.argv[3:]]
    if len(args) >= 3:
        render(rom, sys.argv[2], args[0], args[1], args[2], args[3] if len(args) > 3 else 12)
    else:
        render(rom, sys.argv[2])


if __name__ == '__main__':
    main()
