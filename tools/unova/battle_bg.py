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
the map 64 x 16 u16 (tile | flips << 10 | palette 0-2 << 12: the scene, then its field alone, beyond it), 48 u16 colours (BGR555; each palette's colour 0 unused), (4) u32 n and the scene in full colour, 512 x 128 u16 BGR555 deflated (n bytes: src/mp/mp_battlepic.c draws it).
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

VERSION = 4   # (2: the second half, the field alone; 3: the field beyond the scene, palettes by bands; 4: + the full
              #  colour picture)
MODELS = 'a/0/1/1'
TABLES = 'a/1/5/2'
EYE, TARGET = (0x6B33 / 4096, 0x6B33 / 4096, 0x114CD / 4096), (0.0, 0x299A / 4096, 0.0)
STAGE_Z = (0x572F / 4096, -0xCB7D / 4096)
# Black's camera kept, scaled and centred so its far stage is Kai's (fitted: tools/unova/battle_bg.py fit())
FOCAL, CX, CY = 367.2895, 120.1707, 65.4739
OUT_W, OUT_H = 256, 128          # 32 x 16 tiles: the 240 x 112 above the text box, and a little past it
SS = 2                           # drawn at twice the size, then averaged (the DS's battle edges are smoothed)
SLIDE_SPLIT = 80                 # (battle_intro.c: the top half's scanlines slide one way, the rest the other)
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


def render(field, stage, shift=0.0):
    """Black's scene (field model file, stage model file; None: the field alone) as a float RGB picture (0-31) at
    OUT_W x OUT_H"""
    models = narc.arc(MODELS)
    W, H = OUT_W * SS, OUT_H * SS
    rr.W, rr.H, rr.FOCAL, rr.CX, rr.CY = W, H, FOCAL * SS, (CX + shift) * SS, CY * SS   # (shift: the view moved
    view = look(EYE, TARGET)                                                           #  sideways, in pixels)
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
    """(tiles, map, 48 colours) of a picture (H x W x 3, 0-31). The palettes are bands of whole tile rows (with a row
    shared at each edge), the split that draws the picture best: Black's scenes are gradients from the sky down, and
    tiles side by side in one palette draw them without the 8 x 8 blocks a palette a tile leaves"""
    px = np.clip(np.rint(img), 0, 31).astype(np.int64)
    th, tw = img.shape[0] // 8, img.shape[1] // 8
    tiles = px.reshape(th, 8, tw, 8, 3).transpose(0, 2, 1, 3, 4).reshape(th * tw, 64, 3)
    rows = px.reshape(th, 8 * img.shape[1], 3)
    cache = {}

    def band(a, b):   # (rows a..b-1: its palette, from its rows and one more each side, and its error)
        if (a, b) not in cache:
            sel = rows[max(0, a - 1):min(th, b + 1)].reshape(-1, 3)
            u, w = np.unique(sel, axis=0, return_counts=True)
            p = np.clip(np.rint(kmeans(u.astype(np.float64), w.astype(np.float64), 15, iters=8)), 0, 31)
            own = rows[a:b].reshape(-1, 3)
            uo, wo = np.unique(own, axis=0, return_counts=True)
            err = float((((uo[:, None] - p[None]) ** 2).sum(-1).min(-1) * wo).sum())
            cache[(a, b)] = (p, err)
        return cache[(a, b)]
    best = None
    for a in range(1, th - 1):
        for b in range(a + 1, th):
            e = band(0, a)[1] + band(a, b)[1] + band(b, th)[1]
            if best is None or e < best[0]:
                best = (e, a, b)
    _, a, b = best
    pals = [band(0, a)[0], band(a, b)[0], band(b, th)[0]]
    group = np.array([0 if i // tw < a else 1 if i // tw < b else 2 for i in range(len(tiles))])
    pal = np.zeros((48, 3), np.int64)
    for g in range(3):
        pal[g * 16 + 1:g * 16 + 1 + len(pals[g])] = pals[g]
    idx = np.zeros((len(tiles), 64), np.int64)
    for i, t in enumerate(tiles):
        p = pals[group[i]]
        idx[i] = np.argmin(((t[:, None] - p[None]) ** 2).sum(-1), axis=1) + 1
    # the scene's own half (the first 32 columns) keeps its tiles; the rest takes the nearest of those or of a few of
    # its own, drawn in its own row's palette (BG3 has room for 511)
    first = np.array([(i % tw) < tw // 2 for i in range(len(tiles))])
    out_tiles, tmap = codebook(idx, group, first, pal, MAX_TILES)
    return out_tiles, tmap, pal


def _flips(t):
    t = t.reshape(8, 8)
    return [t, t[:, ::-1], t[::-1, :], t[::-1, ::-1]]


def codebook(idx, group, first, pal, budget):
    """Tiles (each 8 x 8 colour numbers) and the map: every distinct tile of FIRST once (fewer, the most used, if
    they're more than the budget allows), then up to the budget the others' most different ones; every place the
    nearest tile or mirror of those, as its palette draws it"""
    def key(t):
        return t.astype(np.uint8).tobytes()
    uniq, count = {}, {}
    for i in range(len(idx)):
        k = None
        for f, v in enumerate(_flips(idx[i])):
            if key(v) in uniq:
                k = key(v)
                break
        if k is None:
            k = key(idx[i])
            uniq[k] = idx[i].reshape(8, 8)
        count[k] = count.get(k, 0) + (1000 if first[i] else 1)
    keys_first = [k for k in uniq if count[k] >= 1000]
    keys_other = [k for k in uniq if count[k] < 1000]
    keys_first.sort(key=lambda k: -count[k])
    chosen = keys_first[:budget - min(len(keys_other), 64)]
    rest = [k for k in keys_first[len(chosen):]] + keys_other
    room = budget - len(chosen)
    # (the rest: the most different first, farthest from what's chosen, as palette 1 draws them)
    if rest and room > 0:
        g = pal[16:32].astype(np.float32)
        cand = np.array([g[uniq[k].reshape(-1)].reshape(-1) for k in rest], np.float32)
        have = np.array([g[uniq[k].reshape(-1)].reshape(-1) for k in chosen], np.float32) if chosen else None
        dmin = ((cand[:, None] - have[None]) ** 2).sum(-1).min(1) if have is not None else np.full(len(rest), 1e9)
        w = np.array([count[k] for k in rest], np.float32)
        for _ in range(min(room, len(rest))):
            j = int(np.argmax(dmin * w))
            if dmin[j] <= 0:
                break
            chosen.append(rest[j])
            dmin = np.minimum(dmin, ((cand - cand[j]) ** 2).sum(-1))
    tiles = [uniq[k] for k in chosen]
    pos = {k: n for n, k in enumerate(chosen)}
    # every place: its exact tile (or mirror) when chosen, else the nearest as its palette draws it
    flat = [np.stack([v.reshape(-1) for v in _flips(t)]) for t in tiles]     # (n, 4, 64)
    flat = np.stack(flat)
    tmap = []
    for i in range(len(idx)):
        exact = None
        for f, v in enumerate(_flips(idx[i])):
            if key(v) in pos:
                exact = pos[key(v)] | (f << 10)
                break
        if exact is None:
            p = pal[group[i] * 16:(group[i] + 1) * 16].astype(np.float32)
            want = p[idx[i]]                                                  # (64, 3)
            got = p[flat]                                                     # (n, 4, 64, 3)
            d = ((got - want[None, None]) ** 2).sum((-1, -2))
            n, f = np.unravel_index(int(np.argmin(d)), d.shape)
            # (a tile's flips map its own pixels back: the mirror of the chosen tile is the flip of the place)
            exact = int(n) | (int(f) << 10)
        tmap.append(exact | (int(group[i]) << 12))
    return tiles, tmap


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


def picture_blob(img):
    """The scene in full colour (the PC draws it past the GBA's tiles and palettes: src/mp/mp_battlepic.c): u32 the
    packed size, then 512 x 128 u16 BGR555 deflated"""
    px = np.clip(np.rint(img), 0, 31).astype(np.uint16)
    raw = (px[..., 0] | (px[..., 1] << 5) | (px[..., 2] << 10)).astype('<u2').tobytes()
    z = zlib.compress(raw, 9)
    return struct.pack('<I', len(z)) + z


def encode(tiles, tmap, pal):
    out = bytearray(struct.pack('<H', len(tiles)))
    for t in tiles:
        for row in t:
            out += bytes((int(row[x]) | (int(row[x + 1]) << 4)) for x in range(0, 8, 2))
    out += struct.pack('<%dH' % len(tmap), *tmap)
    out += struct.pack('<48H', *[int(r) | (int(g) << 5) | (int(b) << 10) for r, g, b in pal])
    return bytes(out)


def scene(job):
    """One scene, encoded (n, field file, stage file, picture folder or None)"""
    n, f, s, png_dir = job
    # (64 x 16 tiles: the scene, then Black's field alone, as Kai's own backgrounds keep their second half free
    # of the platforms. Kai's intro slides the top half (rows 0-9, BG3's offset 240 down to 0: the map's 0-480)
    # and the bottom half (-240 up to 0: 272-512, then 0-240) in from either side, so the second half is the field
    # further right for the top rows and further left for the bottom ones: every edge the slide crosses is
    # Black's own ground)
    img = render(f, s)
    right, left = render(f, None, -OUT_W), render(f, None, OUT_W)
    img = np.concatenate([img, np.concatenate([right[:SLIDE_SPLIT], left[SLIDE_SPLIT:]], axis=0)], axis=1)
    tiles, tmap, pal = quantize(img)
    print('scene %d: field %d stage %d, %d tiles' % (n, f, s, len(tiles)), flush=True)
    if png_dir:
        from PIL import Image
        os.makedirs(png_dir, exist_ok=True)
        Image.fromarray(picture(tiles, tmap, pal)).save(os.path.join(png_dir, 'scene_%02d_f%d_s%d.png' % (n, f, s)))
    return encode(tiles, tmap, pal) + picture_blob(img)


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
    import multiprocessing
    jobs = [(n, f, s, png_dir) for n, (f, s) in enumerate(order)]
    with multiprocessing.Pool(max(1, os.cpu_count() or 1)) as pool:   # (a scene a core: the release waits on it)
        bgs = pool.map(scene, jobs)
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
