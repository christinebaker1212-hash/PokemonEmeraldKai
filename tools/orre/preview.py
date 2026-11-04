#!/usr/bin/env python3
"""A quick look at a Colosseum model: a textured software render to PNG (top-down or tilted), for checking the
ripped meshes and textures by eye. Not the game's renderer (that is src/sinnoh/s3d.c).

  python3 tools/orre/preview.py MODEL OUT.png [--tilt DEG] [--size N]
"""
import argparse
import math
import sys
import os

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hsd  # noqa: E402


def render(meshes, images, size=800, tilt=55.0, yaw=0.0, frame=None):
    allp = np.concatenate([m.pos for m in (frame or meshes)])
    lo, hi = np.percentile(allp, 0.5, axis=0), np.percentile(allp, 99.5, axis=0)
    c = (lo + hi) / 2
    t, y = math.radians(tilt), math.radians(yaw)
    # view: yaw round Y, then tilt round X (camera looking down at the ground); Y up in Colosseum
    Ry = np.array([[math.cos(y), 0, -math.sin(y)], [0, 1, 0], [math.sin(y), 0, math.cos(y)]])
    Rx = np.array([[1, 0, 0], [0, math.cos(t), math.sin(t)], [0, -math.sin(t), math.cos(t)]])
    R = Rx @ Ry
    # screen: x right, y = -(rotated y) down, depth = rotated z
    def proj(p):
        q = (p - c) @ R.T
        return q
    corners = np.array([[a, b, d] for a in (lo[0], hi[0]) for b in (lo[1], hi[1]) for d in (lo[2], hi[2])])
    qc = proj(corners)
    span = max(qc[:, 0].max() - qc[:, 0].min(), qc[:, 1].max() - qc[:, 1].min())
    scale = size / span * 0.98
    W = H = size
    col = np.zeros((H, W, 3), dtype=np.float32)
    col[:] = (40, 40, 60)
    zb = np.full((H, W), -1e30, dtype=np.float32)
    # opaque first, then the see-through ones (render mode XLU bit 0x40000000 / alpha < 1)
    order = sorted(meshes, key=lambda m: 1 if (m.material and (m.material["rendermode"] & 0x40000000 or m.material["alpha"] < 1)) else 0)
    for m in order:
        q = proj(m.pos)
        sx = q[:, 0] * scale + W / 2
        sy = -q[:, 1] * scale + H / 2
        sz = -q[:, 2]   # nearer the camera = larger
        tex = (images.get(m.tex) if isinstance(images, dict) else (images[m.tex] if 0 <= m.tex < len(images) else None)) if m.tex >= 0 else None
        mat = m.material or {}
        diff = np.array(mat.get("diffuse", (255, 255, 255, 255))[:3], dtype=np.float32) / 255
        us, vs = mat.get("uvscale", (1, 1)) if mat else (1, 1)
        for k in range(0, len(q) - 2, 3):
            xs, ys, zs = sx[k:k + 3], sy[k:k + 3], sz[k:k + 3]
            x0, x1 = int(max(0, math.floor(xs.min()))), int(min(W - 1, math.ceil(xs.max())))
            y0, y1 = int(max(0, math.floor(ys.min()))), int(min(H - 1, math.ceil(ys.max())))
            if x0 > x1 or y0 > y1:
                continue
            den = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2])
            if abs(den) < 1e-9:
                continue
            gx, gy = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
            w0 = ((ys[1] - ys[2]) * (gx - xs[2]) + (xs[2] - xs[1]) * (gy - ys[2])) / den
            w1 = ((ys[2] - ys[0]) * (gx - xs[2]) + (xs[0] - xs[2]) * (gy - ys[2])) / den
            w2 = 1 - w0 - w1
            inside = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
            if not inside.any():
                continue
            z = w0 * zs[0] + w1 * zs[1] + w2 * zs[2]
            sub = zb[y0:y1 + 1, x0:x1 + 1]
            vis = inside & (z > sub)
            if not vis.any():
                continue
            vc = m.col[k:k + 3].astype(np.float32) / 255
            shade = (w0[..., None] * vc[0] + w1[..., None] * vc[1] + w2[..., None] * vc[2])
            rgb = shade[..., :3] * (diff if not m.has_clr else 1.0)
            a = shade[..., 3]
            if tex is not None:
                uv = m.uv[k:k + 3]
                u = (w0 * uv[0, 0] + w1 * uv[1, 0] + w2 * uv[2, 0]) * us
                v = (w0 * uv[0, 1] + w1 * uv[1, 1] + w2 * uv[2, 1]) * vs
                th, tw = tex.shape[:2]
                tx = np.floor(u * tw).astype(np.int64) % tw
                ty = np.floor(v * th).astype(np.int64) % th
                tc = tex[ty, tx].astype(np.float32) / 255
                rgb = np.minimum(rgb * tc[..., :3] * 1.4, 1.0) if m.has_clr else tc[..., :3]
                a = a * tc[..., 3]
            xlu = bool(mat.get("rendermode", 0) & 0x40000000)
            if xlu:   # blended over what is drawn, the depth left alone (shadows, glass)
                vis &= a > 0.01
                dst = col[y0:y1 + 1, x0:x1 + 1]
                dst[vis] = dst[vis] * (1 - a[vis, None]) + (rgb * 255)[vis] * a[vis, None]
            else:
                vis &= a > 0.5
                zb[y0:y1 + 1, x0:x1 + 1][vis] = z[vis]
                col[y0:y1 + 1, x0:x1 + 1][vis] = (rgb * 255)[vis]
    return Image.fromarray(np.clip(col, 0, 255).astype(np.uint8))


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("model")
    ap.add_argument("out")
    ap.add_argument("--tilt", type=float, default=90.0)
    ap.add_argument("--yaw", type=float, default=0.0)
    ap.add_argument("--size", type=int, default=800)
    ap.add_argument("--sets", default="", help="model sets to draw, e.g. 0,1,3 (default: all but the sky)")
    a = ap.parse_args()
    f = hsd.HsdFile(open(a.model, "rb").read())
    meshes = []
    sets = hsd.model_sets(f)
    want = [int(x) for x in a.sets.split(",")] if a.sets else [i for i, m in enumerate(sets) if not hsd.is_sky(m)]
    frame = []
    for i in want:
        meshes += sets[i]
        p = np.concatenate([m.pos for m in sets[i]]) if sets[i] else None
        if p is not None and np.ptp(p[:, 0]) < 1000:   # frame on the place, not the far ground around it
            frame += sets[i]
    tris = sum(len(m.pos) // 3 for m in meshes)
    print("%d meshes, %d triangles, %d textures" % (len(meshes), tris, len(f.images)))
    render(meshes, f.images, a.size, a.tilt, a.yaw, frame or None).save(a.out)
