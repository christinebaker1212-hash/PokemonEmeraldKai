#!/usr/bin/env python3
"""Pokemon Colosseum's TVs. A room with a TV has a table (its floor record's + 0x48: the decomp's tvInitFloor,
field_camera_exact_80117AE4.c std171_mapSetTextureChange / tvRender): entries of 0x18 bytes

    u16 width, height (256 x 256), u32 id (the channel std171 asks for), u32 the room model's set whose texture is
    changed (resource | 0x1000 + set), u32 the programme's model (a news studio: resource), u32 its animation,
    u32 its camera (a .cam, a still one)

and each frame Colosseum draws the programme's model through its camera into the texture on the TV's screen.

Here the pictures are drawn once, as the pack is built (a still of each programme: its model posed at its animation's
first frame, through its camera), and kept in orre_data.pak as "tv/<model>_<anim>.bin": 64 x 64 RGBA. The game puts
one on the room's screen mesh (src/orre/o3d.c O3D_TvSet; the table src/data/orre_tv.h from gen_tv.py).

    python3 -I tools/orre/tv.py UNPACKED            (lists every room's table)
    python3 -I tools/orre/tv.py UNPACKED OUT.png    (a sheet of every picture)
"""
import os
import struct
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import colo_data  # noqa: E402
import colo_rooms  # noqa: E402
import hsd  # noqa: E402
import hsd_anim  # noqa: E402
import hsd_cam  # noqa: E402

SIZE = 64          # the picture kept (the TV's screen is a few pixels across on Kai's screen)
SS = 3             # drawn this many times bigger and averaged down
ASPECT = 4.0 / 3.0  # the screen the programme was made for


def _ids(unpacked, arc):
    out = {}
    p = os.path.join(unpacked, arc, "_ids.txt")
    if os.path.exists(p):
        for line in open(p).read().split("\n"):
            if line.strip():
                i, name = line.split()[:2]
                out[int(i, 16) >> 16] = name
    return out


def tables(unpacked):
    """{room file: [(id, set, model file stem, anim, camera file)]}"""
    common = open(os.path.join(unpacked, "common", "common_rel.rel"), "rb").read()
    crel = colo_rooms.rel_relocs(common)
    out = {}
    for fl in colo_data.floors(unpacked):
        if not fl["file"]:
            continue
        rec = colo_data.FLOOR_TABLE + fl["index"] * colo_data.FLOOR_SIZE
        slot = crel.get((colo_data.DATA_SECTION, rec + 0x48))
        d = colo_rooms._member(unpacked, fl["file"], "rel")
        if slot is None or slot[0] != 0 or d is None:
            continue
        stores = colo_rooms.rel_prolog_stores(d)
        if slot[3] not in stores:
            continue
        sec, desc = stores[slot[3]]
        secs, rr = colo_rooms.rel_sections(d), colo_rooms.rel_relocs(d)
        cnt, ent = rr.get((sec, desc)), rr.get((sec, desc + 4))
        if not cnt or not ent:
            continue
        n = struct.unpack_from(">I", d, secs[cnt[2]][0] + cnt[3])[0]
        base = secs[ent[2]][0] + ent[3]
        ids = _ids(unpacked, fl["file"])
        rows = []
        for k in range(n):
            _w, _h, i, tex, model, anim, cam = struct.unpack_from(">HHIIIII", d, base + 0x18 * k)
            m, c = ids.get(model >> 16), ids.get(cam >> 16)
            if m is None or c is None:
                continue
            rows.append((i, tex & 0xFFF, m[:-4], anim, c))
        if rows:
            out[fl["file"]] = (fl["room"], rows)
    return out


def _find(unpacked, room, name):
    p = os.path.join(unpacked, room, name)
    if os.path.exists(p):
        return p
    for arc in sorted(os.listdir(unpacked)):
        p = os.path.join(unpacked, arc, name)
        if os.path.exists(p):
            return p
    return None


def render(model_path, anim, cam_path, lights=None):
    """The programme's still: (SIZE, SIZE, 4) uint8"""
    f, root, anims = hsd_anim.model(model_path)
    if anims:
        ms = hsd_anim.load_posed(f, root, anims[min(anim, len(anims) - 1)], 0)
    else:
        ms = [m for st in hsd.model_sets(f) for m in st]
    base, _ctr, _etr, _ttr, _end = hsd_cam.load(open(cam_path, "rb").read())
    eye, tgt = np.array(base["eye"], dtype=np.float64), np.array(base["tgt"], dtype=np.float64)
    fov = base["fov"] if 1.0 < base["fov"] < 170.0 else 30.0
    fwd = tgt - eye
    fwd /= max(np.linalg.norm(fwd), 1e-9)
    right = np.cross(fwd, np.array([0.0, 1.0, 0.0]))
    right /= max(np.linalg.norm(right), 1e-9)
    up = np.cross(right, fwd)
    W = H = SIZE * SS
    focal = (H / 2.0) / np.tan(np.radians(fov) / 2.0)
    color = np.zeros((H, W, 3), dtype=np.float64)
    depth = np.full((H, W), 1e30)
    def clear(m):
        mat = m.material or {}
        return bool(mat.get("rendermode", 0) & 0x40000000) or mat.get("alpha", 1.0) < 1.0
    for m in sorted(ms, key=clear):   # (the see-through ones after the solid, blended over them)
        if not len(m.pos):
            continue
        if lights is not None:
            import build_pack
            build_pack.light_mesh(m, lights)
        mat = m.material or {}
        pos = np.asarray(m.pos, dtype=np.float64) - eye
        cx, cy, cz = pos @ right, pos @ up, pos @ fwd
        see, alpha = clear(m), float(mat.get("alpha", 1.0))
        if see and m.tex < 0 and cz.min() > 0.5:
            # (a plain sheet over the whole picture: a fade the programme's animation brings in and out, not there
            # in its still)
            bx = focal * cx / cz / ASPECT
            by = focal * cy / cz
            if bx.max() - bx.min() > 0.8 * W and by.max() - by.min() > 0.8 * H:
                continue
        img = f.images[m.tex] if 0 <= m.tex < len(f.images) else None
        us, vs = mat.get("uvscale", (1.0, 1.0))
        uv = np.asarray(m.uv, dtype=np.float64) if img is not None and len(m.uv) else None
        if getattr(m, "has_clr", False) and len(m.col):
            col = np.asarray(m.col, dtype=np.float64)[:, :3] / 128.0
        else:
            col = np.tile(np.array(mat.get("diffuse", (255, 255, 255, 255))[:3], dtype=np.float64) / 255.0, (len(pos), 1))
        for t in range(0, len(pos) - 2, 3):
            z = cz[t:t + 3]
            if z.min() < 0.5:
                continue
            sx = W / 2.0 + focal * cx[t:t + 3] / z / ASPECT   # (4:3 squeezed into the square texture)
            sy = H / 2.0 - focal * cy[t:t + 3] / z
            x0, x1 = int(max(np.floor(sx.min()), 0)), int(min(np.ceil(sx.max()), W - 1))
            y0, y1 = int(max(np.floor(sy.min()), 0)), int(min(np.ceil(sy.max()), H - 1))
            if x0 > x1 or y0 > y1:
                continue
            area = (sx[1] - sx[0]) * (sy[2] - sy[0]) - (sx[2] - sx[0]) * (sy[1] - sy[0])
            if abs(area) < 1e-9:
                continue
            px, py = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
            w0 = ((sx[1] - px) * (sy[2] - py) - (sx[2] - px) * (sy[1] - py)) / area
            w1 = ((sx[2] - px) * (sy[0] - py) - (sx[0] - px) * (sy[2] - py)) / area
            w2 = 1.0 - w0 - w1
            inside = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
            if not inside.any():
                continue
            iz = w0 / z[0] + w1 / z[1] + w2 / z[2]
            zz = 1.0 / np.maximum(iz, 1e-12)
            c = (w0[..., None] * col[t] / z[0] + w1[..., None] * col[t + 1] / z[1] + w2[..., None] * col[t + 2] / z[2]) * zz[..., None]
            ok = inside & (zz < depth[y0:y1 + 1, x0:x1 + 1])
            if uv is not None:
                u = (w0 * uv[t, 0] / z[0] + w1 * uv[t + 1, 0] / z[1] + w2 * uv[t + 2, 0] / z[2]) * zz * us
                v = (w0 * uv[t, 1] / z[0] + w1 * uv[t + 1, 1] / z[1] + w2 * uv[t + 2, 1] / z[2]) * zz * vs
                ih, iw = img.shape[:2]
                tu = np.mod(np.floor(u * iw).astype(np.int64), iw)
                tv = np.mod(np.floor(v * ih).astype(np.int64), ih)
                texel = img[tv, tu].astype(np.float64)
                ok &= texel[..., 3] >= 128
                c = c * texel[..., :3] / 255.0
            sub_c, sub_d = color[y0:y1 + 1, x0:x1 + 1], depth[y0:y1 + 1, x0:x1 + 1]
            if see:
                sub_c[ok] = sub_c[ok] * (1.0 - alpha) + np.clip(c[ok], 0.0, 1.0) * alpha
            else:
                sub_c[ok] = np.clip(c[ok], 0.0, 1.0)
                sub_d[ok] = zz[ok]
    small = color.reshape(SIZE, SS, SIZE, SS, 3).mean(axis=(1, 3))
    out = np.zeros((SIZE, SIZE, 4), dtype=np.uint8)
    out[..., :3] = np.clip(small * 255.0 + 0.5, 0, 255).astype(np.uint8)
    out[..., 3] = 255
    return out


def pictures(unpacked):
    """{"<model>_<anim>": (SIZE, SIZE, 4) uint8}: each programme once"""
    out = {}
    for room, (_num, rows) in sorted(tables(unpacked).items()):
        for _i, _set, model, anim, cam in rows:
            key = "%s_%d" % (model, anim)
            if key in out:
                continue
            mp, cp = _find(unpacked, room, model + ".dat"), _find(unpacked, room, cam)
            if mp is None or cp is None:
                continue
            out[key] = render(mp, anim, cp)
    return out


def main():
    unpacked = sys.argv[1]
    for room, (num, rows) in sorted(tables(unpacked).items()):
        print(room, num)
        for r in rows:
            print("   id %2d set %d model %s anim %d camera %s" % r)
    if len(sys.argv) > 2:
        from PIL import Image
        pics = pictures(unpacked)
        keys = sorted(pics)
        sheet = Image.new("RGB", (SIZE * 8, SIZE * ((len(keys) + 7) // 8)))
        for k, name in enumerate(keys):
            sheet.paste(Image.fromarray(pics[name][..., :3]), ((k % 8) * SIZE, (k // 8) * SIZE))
        sheet.resize((sheet.width * 3, sheet.height * 3), Image.NEAREST).save(sys.argv[2])
        print(len(keys), "pictures:", " ".join(keys))


if __name__ == "__main__":
    main()
