#!/usr/bin/env python3
"""UNOVA's battle backgrounds: Black's own battle scenes (its field, the sky and the land round the battle, and the two
round stages the POKeMON stand on), drawn through Black's battle camera and fitted to Kai's battle screen as Emerald's
own backgrounds are: a 4bpp background of at most 511 tiles and three 16-colour palettes (BG3, src/battle_bg.c).

    python3 -I tools/unova/battle_bg.py OUT.pak [--png DIR]        (DIR: each scene as a picture, for checking)

Black (aletheia's ov094/btlv_effect.c BTLV_EFFECT_Init): the battle's situation (ov021 field_encount.c
FIELD_ENCOUNT_SetFieldSituation) is the zone's field effect set (its header's u16 at 0x1E, bits 5-9: the stage table's
row) and the ground under the player (MAP_ATTR_GetBattleBackground: the terrain id through ov021's 37 pairs at
0x021D8EA0, the row's column); archive 0x98 (a/1/5/2) member 0 is the stage table (0x24 bytes a row: u8 has light,
u8 has variant, u8 field id x 17, u8 stage id x 17), member 1 the fields (s32 file [4 kinds][4 variants]: kind 0 the
model, 1-3 its animations; variant = the season where the row has variants, -1 = the first), member 2 the stages
(u32, then the same 4 x 4). The models are archive 11 (a/0/1/1). The field is drawn at the origin, the stage twice:
the player's at z 0x572F, the foe's at z -0xCB7D (ov094 btlv_stage.c). The camera (ov094 btlv_camera.c): eye
(6.7, 6.7, 17.3), target (0, 2.6, 0), sin / cos of half the field of view 0x399 / 0xF97. A row with a light sets
light 0 from the zone's light table at the time (ZONEDATA_GetTimeLightColors), from straight above: the scene is drawn
under white and the game tints the palette by that light's colour (src/unova/u3d_battle.c).

Kai's battle screen puts the foe's stage at 112-240 x 48-80 (the stock backgrounds' ellipse; its POKeMON at 176, 40):
Black's camera is kept and the picture only scaled (x 0.883) and centred so Black's far stage lands there; Black's
near stage then comes out where Kai's own is (its top at y 106, under the player's back picture).

The file is "U3DB" u32 version, u32 the size unpacked, then zlib: u16 zones, u8 field effect set per zone; u8 rows, per row u8 has light,
u8 has variant, u16 background [17 columns][4 variants]; u16 backgrounds, each: u16 tiles, tiles x 32 bytes (4bpp),
the map 64 x 16 u16 (tile | flips << 10 | palette 0-2 << 12: the scene, then its field alone mirrored), 48 u16 colours (BGR555; each palette's colour 0 unused).
"""
import math
import os
import struct
import sys
import zlib

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(os.path.dirname(HERE), 'sinnoh'))
import narc  # noqa: E402
import nitro  # noqa: E402
import zones as zr  # noqa: E402
import importlib.util  # noqa: E402
_spec = importlib.util.spec_from_file_location('sinnoh_render_ref', os.path.join(os.path.dirname(HERE), 'sinnoh',
                                                                                 'render_ref.py'))
rr = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(rr)

VERSION = 2   # (2: the second half, the field alone)
MODELS = 'a/0/1/1'
TABLES = 'a/1/5/2'
EYE, TARGET = (0x6B33 / 4096, 0x6B33 / 4096, 0x114CD / 4096), (0.0, 0x299A / 4096, 0.0)
STAGE_Z = (0x572F / 4096, -0xCB7D / 4096)
# Black's camera kept, scaled and centred so its far stage is Kai's (fitted: tools/unova/battle_bg.py fit())
FOCAL, CX, CY = 367.2895, 120.1707, 65.4739
OUT_W, OUT_H = 256, 128          # 32 x 16 tiles: the 240 x 112 above the text box, and a little past it
SS = 2                           # drawn at twice the size, then averaged (the DS's battle edges are smoothed)
MAX_TILES = 511                  # (BG3's tiles at 0x8000, BG0's map at 0xC000)
COLUMNS = 17


def look(eye, target):
    eye = np.array(eye, float)
    f = np.array(target, float) - eye
    f /= np.linalg.norm(f)
    r = np.cross(f, [0.0, 1.0, 0.0])
    r /= np.linalg.norm(r)
    u = np.cross(r, f)
    m = np.eye(4)
    m[0, :3], m[1, :3], m[2, :3] = r, u, -f
    m[:3, 3] = -m[:3, :3] @ eye
    return m


def tables():
    a = narc.arc(TABLES)
    rows = []
    for r in range(len(a[0]) // 0x24):
        d = a[0][r * 0x24:(r + 1) * 0x24]
        rows.append((d[0], d[1], list(d[2:19]), list(d[19:36])))
    fields = [[list(struct.unpack_from('<4i', a[1], g * 64 + k * 16)) for k in range(4)] for g in range(len(a[1]) // 64)]
    stages = [[list(struct.unpack_from('<4i', a[2], g * 68 + 4 + k * 16)) for k in range(4)]
              for g in range(len(a[2]) // 68)]
    return rows, fields, stages


def model_file(table, group, variant):
    files = table[group][0]
    return files[variant] if files[variant] != -1 else files[0]


def rgb555(c):
    return (c & 31, (c >> 5) & 31, (c >> 10) & 31)


def light_block(mat):
    """The material's own colours (diffuse, ambient, specular, emission) under one white light from straight above"""
    dif, amb = rgb555(mat.dif_amb), rgb555(mat.dif_amb >> 16)
    spe, emi = rgb555(mat.spe_emi), rgb555(mat.spe_emi >> 16)
    lights = [(1, (31, 31, 31), (0.0, -1.0, 0.0))] + [(0, (0, 0, 0), (0.0, -1.0, 0.0))] * 3
    return (0, lights, (dif, amb, spe, emi))


def render(field, stage):
    """Black's scene (field model file, stage model file; None: the field alone) as a float RGB picture (0-31) at
    OUT_W x OUT_H"""
    models = narc.arc(MODELS)
    W, H = OUT_W * SS, OUT_H * SS
    rr.W, rr.H, rr.FOCAL, rr.CX, rr.CY = W, H, FOCAL * SS, CX * SS, CY * SS
    view = look(EYE, TARGET)
    color = np.zeros((H, W, 3), np.float32)
    depth = np.full((H, W), np.inf, np.float32)
    ids = np.full((H, W), -1, np.int16)
    sc = rr.Scene(None)
    tris = []
    for fid, origins in ((field, [(0.0, 0.0, 0.0)]), (stage, [(0.0, 0.0, z) for z in STAGE_Z])):
        if fid is None:
            continue
        bmd = nitro.Bmd0(models[fid])
        for o in origins:
            for model in bmd.models:
                for mi, ts in model.draw().items():
                    mat = model.materials[mi]
                    if not mat.lights and (mat.dif_amb >> 15) & 1:   # (the diffuse as the vertex colour)
                        d = rgb555(mat.dif_amb)
                        ts = [[(p, uv, d, n) for p, uv, _, n in t] for t in ts]
                    tris.append((np.array(o), ts, sc.texture(bmd.tex, 'b', mat), mat))
    for group in ([t for t in tris if t[3].alpha == 31], [t for t in tris if t[3].alpha != 31]):
        for origin, ts, img, mat in group:
            lb = light_block(mat)
            for tri in ts:
                rr.draw_tri(tri, origin, None, img, mat, view, lb, color, depth, ids)
    return color.reshape(OUT_H, SS, OUT_W, SS, 3).mean(axis=(1, 3))


# --- Emerald's background format: tiles of 8 x 8 in one of three 15-colour palettes ---

def kmeans(points, weights, k, iters=12, seed=0):
    """Weighted k-means (points N x 3): k centres"""
    uniq = np.unique(points, axis=0)
    if len(uniq) <= k:
        return uniq.astype(np.float64)
    rng = np.random.default_rng(seed)
    # k-means++ start
    c = [points[rng.choice(len(points), p=weights / weights.sum())]]
    for _ in range(k - 1):
        d = np.min(((points[:, None, :] - np.array(c)[None]) ** 2).sum(-1), axis=1) * weights
        c.append(points[rng.choice(len(points), p=d / d.sum())] if d.sum() > 0 else points[rng.integers(len(points))])
    c = np.array(c, np.float64)
    for _ in range(iters):
        lab = np.argmin(((points[:, None, :] - c[None]) ** 2).sum(-1), axis=1)
        for j in range(k):
            m = lab == j
            if m.any():
                c[j] = (points[m] * weights[m, None]).sum(0) / weights[m].sum()
    return c


def quantize(img):
    """(tiles, map, 48 colours) of a picture (H x W x 3, 0-31)"""
    px = np.clip(np.rint(img), 0, 31).astype(np.int64)
    th, tw = img.shape[0] // 8, img.shape[1] // 8
    tiles = px.reshape(th, 8, tw, 8, 3).transpose(0, 2, 1, 3, 4).reshape(th * tw, 64, 3)
    # the picture's colours, weighted by use
    allc, inv, cnt = np.unique(px.reshape(-1, 3), axis=0, return_inverse=True, return_counts=True)
    # start: tiles grouped by their mean colour
    means = tiles.mean(1)
    cent = kmeans(means, np.ones(len(means)), 3)
    group = np.argmin(((means[:, None] - cent[None]) ** 2).sum(-1), axis=1)
    pals = []
    for it in range(6):
        pals = []
        for g in range(3):
            sel = tiles[group == g].reshape(-1, 3) if (group == g).any() else tiles.reshape(-1, 3)
            u, w = np.unique(sel, axis=0, return_counts=True)
            p = kmeans(u.astype(np.float64), w.astype(np.float64), 15, seed=it)
            pals.append(np.clip(np.rint(p), 0, 31))
        # each tile to the palette that draws it best
        err = np.zeros((len(tiles), 3))
        for g in range(3):
            d = ((tiles[:, :, None, :] - pals[g][None, None]) ** 2).sum(-1).min(-1)
            err[:, g] = d.sum(1)
        ng = np.argmin(err, axis=1)
        if (ng == group).all():
            break
        group = ng
    pal = np.zeros((48, 3), np.int64)
    for g in range(3):
        pal[g * 16 + 1:g * 16 + 1 + len(pals[g])] = pals[g]
    idx = np.zeros((len(tiles), 64), np.int64)
    for i, t in enumerate(tiles):
        p = pals[group[i]]
        idx[i] = np.argmin(((t[:, None] - p[None]) ** 2).sum(-1), axis=1) + 1
    # the same tile (or its mirror) once
    out_tiles, seen, tmap = [], {}, []
    for i in range(len(tiles)):
        t = idx[i].reshape(8, 8)
        key = None
        for flips, v in ((0, t), (1, t[:, ::-1]), (2, t[::-1, :]), (3, t[::-1, ::-1])):
            b = v.astype(np.uint8).tobytes()
            if b in seen:
                key = seen[b] | (flips << 10)
                break
        if key is None:
            seen[t.astype(np.uint8).tobytes()] = len(out_tiles)
            key = len(out_tiles)
            out_tiles.append(t)
        tmap.append(key | (int(group[i]) << 12))
    while len(out_tiles) > MAX_TILES:   # (too many: the two most alike tiles of a palette made one)
        out_tiles, tmap = merge_closest(out_tiles, tmap, pal)
    return out_tiles, tmap, pal


def merge_closest(out_tiles, tmap, pal):
    pal_of = {}
    for e in tmap:
        pal_of.setdefault(e & 0x3FF, (e >> 12) & 3)
    flat = np.array([pal[pal_of[i] * 16 + t.reshape(-1)] for i, t in enumerate(out_tiles)], np.float64)
    best = None
    n = len(out_tiles)
    for i in range(n):
        d = ((flat[i + 1:] - flat[i]) ** 2).sum((1, 2))
        same = np.array([pal_of[j] == pal_of[i] for j in range(i + 1, n)])
        if not same.any():
            continue
        d = np.where(same, d, np.inf)
        j = int(np.argmin(d))
        if best is None or d[j] < best[0]:
            best = (d[j], i, i + 1 + j)
    _, keep, drop = best
    remap = {}
    for k in range(n):
        remap[k] = keep if k == drop else (k if k < drop else k - 1)
    tiles = [t for k, t in enumerate(out_tiles) if k != drop]
    out = []
    for e in tmap:
        t = e & 0x3FF
        out.append(remap[t] | (e & ~0x3FF & 0xFFFF) if t != drop else remap[t] | (e & 0xF000))
    return tiles, out


def picture(tiles, tmap, pal):
    """The background as an RGB picture (for checking)"""
    th, tw = OUT_H // 8, len(tmap) // (OUT_H // 8)
    img = np.zeros((OUT_H, tw * 8, 3), np.uint8)
    for i, e in enumerate(tmap):
        t = tiles[e & 0x3FF]
        if e & 0x400:
            t = t[:, ::-1]
        if e & 0x800:
            t = t[::-1, :]
        c = pal[((e >> 12) & 3) * 16 + t]
        y, x = (i // tw) * 8, (i % tw) * 8
        img[y:y + 8, x:x + 8] = (c * 255 // 31).astype(np.uint8)
    return img


def encode(tiles, tmap, pal):
    out = bytearray(struct.pack('<H', len(tiles)))
    for t in tiles:
        for row in t:
            out += bytes((int(row[x]) | (int(row[x + 1]) << 4)) for x in range(0, 8, 2))
    out += struct.pack('<%dH' % len(tmap), *tmap)
    out += struct.pack('<48H', *[int(r) | (int(g) << 5) | (int(b) << 10) for r, g, b in pal])
    return bytes(out)


def build(png_dir=None):
    rows, fields, stages = tables()
    scenes = {}      # (field file, stage file) -> background number
    order = []
    row_out = []
    for has_light, has_variant, field_ids, stage_ids in rows:
        cols = []
        for c in range(COLUMNS):
            vs = []
            for v in range(4):
                vv = v if has_variant else 0
                f = model_file(fields, field_ids[c], vv) if field_ids[c] != 255 else model_file(fields, field_ids[0], vv)
                s = model_file(stages, stage_ids[c], vv)
                if (f, s) not in scenes:
                    scenes[(f, s)] = len(order)
                    order.append((f, s))
                vs.append(scenes[(f, s)])
            cols.append(vs)
        row_out.append((has_light, has_variant, cols))
    bgs = []
    for n, (f, s) in enumerate(order):
        # (64 x 16 tiles: the scene, then the field alone mirrored, as Kai's own backgrounds keep their second half
        # free of the platforms: the intro slides the halves in from either side over all 512, seamless so)
        img = render(f, s)
        img = np.concatenate([img, render(f, None)[:, ::-1]], axis=1)
        tiles, tmap, pal = quantize(img)
        bgs.append(encode(tiles, tmap, pal))
        print('scene %d: field %d stage %d, %d tiles' % (n, f, s, len(tiles)), flush=True)
        if png_dir:
            from PIL import Image
            os.makedirs(png_dir, exist_ok=True)
            Image.fromarray(picture(tiles, tmap, pal)).save(os.path.join(png_dir, 'scene_%02d_f%d_s%d.png' % (n, f, s)))
            Image.fromarray(np.clip(img * 255 / 31, 0, 255).astype(np.uint8)).save(
                os.path.join(png_dir, 'scene_%02d_drawn.png' % n))
    zs = zr.zones()
    out = bytearray(struct.pack('<H', len(zs)))
    out += bytes(((z['flags'] >> 5) & 31) for z in zs)
    out += struct.pack('<B', len(row_out))
    for has_light, has_variant, cols in row_out:
        out += struct.pack('<BB', has_light, has_variant)
        for vs in cols:
            out += struct.pack('<4H', *vs)
    out += struct.pack('<H', len(bgs))
    for b in bgs:
        out += b
    return b'U3DB' + struct.pack('<II', VERSION, len(out)) + zlib.compress(bytes(out), 9)


def fit():
    """Black's camera, scaled and centred so its far stage covers Kai's (112-240, 48-80): FOCAL, CX, CY"""
    R, Y = 5.3293457, 0.3
    view = look(EYE, TARGET)

    def box(q):
        a = np.linspace(0, 2 * math.pi, 64, endpoint=False)
        p = np.stack([R * np.cos(a), np.full(64, Y), STAGE_Z[1] + R * np.sin(a), np.ones(64)])
        c = view @ p
        x = q[1] + q[0] * c[0] / -c[2]
        y = q[2] - q[0] * c[1] / -c[2]
        return np.array([x.min(), x.max(), y.min(), y.max()]) - (112, 240, 48, 80)
    q = np.array([400.0, 120.0, 80.0])
    for _ in range(50):
        r = box(q)
        J = np.zeros((4, 3))
        for k in range(3):
            d = np.zeros(3)
            d[k] = 1e-3
            J[:, k] = (box(q + d) - r) / 1e-3
        q = q + np.linalg.lstsq(J, -r, rcond=None)[0]
    return q


def main():
    if len(sys.argv) > 1 and sys.argv[1] == '--fit':
        print('FOCAL, CX, CY = %.4f, %.4f, %.4f' % tuple(fit()))
        return
    out = sys.argv[1]
    png = sys.argv[sys.argv.index('--png') + 1] if '--png' in sys.argv else None
    data = build(png)
    open(out, 'wb').write(data)
    print('%s: %d bytes' % (out, len(data)))


if __name__ == '__main__':
    main()
