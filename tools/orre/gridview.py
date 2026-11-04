#!/usr/bin/env python3
"""A map's tile grid (tools/orre/grid.py) over a top-down picture of its model, for checking the grid by eye:
blocked tiles red, triggers blue, tile lines grey.

  python3 -I tools/orre/gridview.py MAP.map MAP.col OUT.png [PX_PER_TILE [START_X START_Z]]
"""
import os
import sys

import numpy as np
from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import grid  # noqa: E402
import hsd  # noqa: E402


def topdown(meshes, images, x0, z0, wu, hu, scale):
    W, H = int(wu * scale), int(hu * scale)
    col = np.zeros((H, W, 3), dtype=np.float32)
    col[:] = (30, 30, 40)
    zb = np.full((H, W), -1e30, dtype=np.float32)
    for m in meshes:
        mat = m.material or {}
        if mat.get("rendermode", 0) & 0x40000000:
            continue
        tex = images[m.tex] if 0 <= m.tex < len(images) else None
        us, vs = mat.get("uvscale", (1, 1))
        sx = (m.pos[:, 0] - x0) * scale
        sy = (m.pos[:, 2] - z0) * scale
        sz = m.pos[:, 1]
        for k in range(0, len(sx) - 2, 3):
            xs, ys, zs = sx[k:k + 3], sy[k:k + 3], sz[k:k + 3]
            ax0, ax1 = int(max(0, np.floor(xs.min()))), int(min(W - 1, np.ceil(xs.max())))
            ay0, ay1 = int(max(0, np.floor(ys.min()))), int(min(H - 1, np.ceil(ys.max())))
            if ax0 > ax1 or ay0 > ay1:
                continue
            den = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2])
            if abs(den) < 1e-9:
                continue
            gx, gy = np.meshgrid(np.arange(ax0, ax1 + 1) + 0.5, np.arange(ay0, ay1 + 1) + 0.5)
            w0 = ((ys[1] - ys[2]) * (gx - xs[2]) + (xs[2] - xs[1]) * (gy - ys[2])) / den
            w1 = ((ys[2] - ys[0]) * (gx - xs[2]) + (xs[0] - xs[2]) * (gy - ys[2])) / den
            w2 = 1 - w0 - w1
            inside = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
            z = w0 * zs[0] + w1 * zs[1] + w2 * zs[2]
            sub = zb[ay0:ay1 + 1, ax0:ax1 + 1]
            vis = inside & (z > sub)
            if not vis.any():
                continue
            vc = m.col[k:k + 3].astype(np.float32) / 255
            rgb = (w0[..., None] * vc[0] + w1[..., None] * vc[1] + w2[..., None] * vc[2])[..., :3]
            if tex is not None:
                uv = m.uv[k:k + 3]
                u = (w0 * uv[0, 0] + w1 * uv[1, 0] + w2 * uv[2, 0]) * us
                v = (w0 * uv[0, 1] + w1 * uv[1, 1] + w2 * uv[2, 1]) * vs
                th, tw = tex.shape[:2]
                tc = tex[np.floor(v * th).astype(np.int64) % th, np.floor(u * tw).astype(np.int64) % tw].astype(np.float32) / 255
                rgb = np.minimum(rgb * tc[..., :3] * 1.6, 1.0)
                vis &= tc[..., 3] > 0.5
            zb[ay0:ay1 + 1, ax0:ax1 + 1][vis] = z[vis]
            col[ay0:ay1 + 1, ax0:ax1 + 1][vis] = (rgb * 255)[vis]
    return Image.fromarray(np.clip(col, 0, 255).astype(np.uint8))


if __name__ == "__main__":
    mp, cp, out = sys.argv[1:4]
    px = int(sys.argv[4]) if len(sys.argv) > 4 else 12
    st = (float(sys.argv[5]), float(sys.argv[6])) if len(sys.argv) > 6 else None
    g = grid.build(grid.read(open(cp, "rb").read()), grid.TILE, st)
    f = hsd.HsdFile(open(mp, "rb").read())
    meshes = [m for s in hsd.model_sets(f) if not hsd.is_sky(s) for m in s]
    scale = px / g["tile"]
    img = topdown(meshes, f.images, g["x0"], g["z0"], g["w"] * g["tile"], g["h"] * g["tile"], scale).convert("RGBA")
    over = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(over)
    for y in range(g["h"]):
        for x in range(g["w"]):
            r = (x * px, y * px, x * px + px - 1, y * px + px - 1)
            if g["trigger"][y, x] >= 0:
                d.rectangle(r, fill=(40, 90, 255, 110))
            elif not g["walk"][y, x]:
                d.rectangle(r, fill=(255, 40, 40, 90))
            d.rectangle(r, outline=(160, 160, 160, 60))
    Image.alpha_composite(img, over).save(out)
