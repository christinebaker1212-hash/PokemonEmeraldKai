"""ORRE people as GBA overworld sprites rendered from Colosseum's field models (a dry run, 2026-11-07):
posed by hsd_anim (idle 1, walk 2, run 3 in every field model's table), GBA proportions by scaling the head joint,
rendered at 8x, each pixel the part covering most of it (its lit half, small dark details kept), 15 colours shared
by the sheet (k-means, each part weighing the same), a silhouette outline and inner lines at depth jumps.
Try it: python3 tools/orre/people_sprites_demo.py OUTDIR (needs the unpacked disc in ~/.cache/moemon/orre)."""
import math, sys
import numpy as np
from PIL import Image
sys.path.insert(0, __import__('os').path.dirname(__import__('os').path.abspath(__file__)))
import hsd_anim as anim, hsd

def render_rgba(meshes, images, W, H, ground, ppu, tilt, yaw):
    """Like preview.render, but one fixed scale (ppu pixels per unit) with the ground point at `ground`
    (screen x, y), a transparent background, and no see-through meshes (the game draws its own shadow)."""
    t, y = math.radians(tilt), math.radians(yaw)
    Ry = np.array([[math.cos(y), 0, -math.sin(y)], [0, 1, 0], [math.sin(y), 0, math.cos(y)]])
    Rx = np.array([[1, 0, 0], [0, math.cos(t), math.sin(t)], [0, -math.sin(t), math.cos(t)]])
    R = Rx @ Ry
    col = np.zeros((H, W, 4), dtype=np.float32)
    zb = np.full((H, W), -1e30, dtype=np.float32)
    ib = np.full((H, W), -1, dtype=np.int32)
    for mi, m in enumerate(meshes):
        mat = m.material or {}
        if mat.get("rendermode", 0) & 0x40000000 or mat.get("alpha", 1) < 1:
            continue
        q = m.pos @ R.T
        sx, sy, sz = q[:, 0] * ppu + ground[0], -q[:, 1] * ppu + ground[1], -q[:, 2]
        tex = images[m.tex] if 0 <= m.tex < len(images) else None
        diff = np.array(mat.get("diffuse", (255, 255, 255, 255))[:3], dtype=np.float32) / 255
        us, vs = mat.get("uvscale", (1, 1))
        for k in range(0, len(q) - 2, 3):
            xs, ys, zs = sx[k:k + 3], sy[k:k + 3], sz[k:k + 3]
            x0, x1 = int(max(0, math.floor(xs.min()))), int(min(W - 1, math.ceil(xs.max())))
            y0, y1 = int(max(0, math.floor(ys.min()))), int(min(H - 1, math.ceil(ys.max())))
            if x0 > x1 or y0 > y1: continue
            den = (ys[1] - ys[2]) * (xs[0] - xs[2]) + (xs[2] - xs[1]) * (ys[0] - ys[2])
            if abs(den) < 1e-9: continue
            gx, gy = np.meshgrid(np.arange(x0, x1 + 1) + 0.5, np.arange(y0, y1 + 1) + 0.5)
            w0 = ((ys[1] - ys[2]) * (gx - xs[2]) + (xs[2] - xs[1]) * (gy - ys[2])) / den
            w1 = ((ys[2] - ys[0]) * (gx - xs[2]) + (xs[0] - xs[2]) * (gy - ys[2])) / den
            w2 = 1 - w0 - w1
            inside = (w0 >= 0) & (w1 >= 0) & (w2 >= 0)
            if not inside.any(): continue
            z = w0 * zs[0] + w1 * zs[1] + w2 * zs[2]
            vis = inside & (z > zb[y0:y1 + 1, x0:x1 + 1])
            if not vis.any(): continue
            vc = m.col[k:k + 3].astype(np.float32) / 255
            shade = w0[..., None] * vc[0] + w1[..., None] * vc[1] + w2[..., None] * vc[2]
            rgb, a = shade[..., :3] * (diff if not m.has_clr else 1.0), shade[..., 3]
            if tex is not None:
                uv = m.uv[k:k + 3]
                u = (w0 * uv[0, 0] + w1 * uv[1, 0] + w2 * uv[2, 0]) * us
                v = (w0 * uv[0, 1] + w1 * uv[1, 1] + w2 * uv[2, 1]) * vs
                th, tw = tex.shape[:2]
                tc = tex[np.floor(v * th).astype(np.int64) % th, np.floor(u * tw).astype(np.int64) % tw].astype(np.float32) / 255
                rgb = np.minimum(rgb * tc[..., :3] * 1.4, 1.0) if m.has_clr else tc[..., :3]
                a = a * tc[..., 3]
            vis &= a > 0.5
            zb[y0:y1 + 1, x0:x1 + 1][vis] = z[vis]
            c = col[y0:y1 + 1, x0:x1 + 1]
            c[vis, :3] = (rgb * 255)[vis]; c[vis, 3] = 255
            ib[y0:y1 + 1, x0:x1 + 1][vis] = mi * 4 + (tc_part(tex, u, v)[vis] if tex is not None else 0)
    return col, zb, ib

def tc_part(tex, u, v):
    # which quarter of the texture: an atlas holds several parts (hair, face, boots), told apart this way
    return ((np.floor(u * 2).astype(np.int64) % 2) * 2 + (np.floor(v * 2).astype(np.int64) % 2))

def chibi(neck, head=1.7, body=0.72):
    """A joint deform: the head (the neck joint's subtree) larger, everything squashed in height, so the figure
    has a GBA sprite's proportions (about 2.5 heads tall)."""
    def deform(f, joints):
        sub = set(anim.subtree(f, neck, [neck]))
        n = joints[neck][:3, 3].copy()
        A = np.eye(4); A[:3, :3] = np.diag([head, head / body, head]); A[:3, 3] = n - A[:3, :3] @ n
        Sq = np.diag([1.0, body, 1.0, 1.0])
        for j in joints:
            joints[j] = Sq @ (A @ joints[j] if j in sub else joints[j])
    return deform

def downsample(big, ss, cover=0.4):
    H, W = big.shape[0] // ss, big.shape[1] // ss
    blk = big.reshape(H, ss, W, ss, 4).transpose(0, 2, 1, 3, 4).reshape(H, W, ss * ss, 4)
    a = blk[..., 3] > 0
    n = a.sum(-1)
    rgb = (blk[..., :3] * a[..., None]).sum(-2) / np.maximum(n, 1)[..., None]
    return rgb, n >= cover * ss * ss

def to_gba(frames, colours=15, outline=0.38):
    """Shared palette for all frames: colours-1 from the art (median cut), plus outlines (the edge pixels'
    colours darkened), snapped to 5-bit GBA colour."""
    from PIL import Image
    edge = []
    for rgb, m in frames:
        pad = np.pad(m, 1)
        e = m & ~(pad[:-2, 1:-1] & pad[2:, 1:-1] & pad[1:-1, :-2] & pad[1:-1, 2:])
        edge.append(e)
    pix = np.concatenate([rgb[m & ~e] for (rgb, m), e in zip(frames, edge)]).astype(np.uint8)
    q = Image.fromarray(pix[None]).quantize(colours - 2, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = np.array(q.getpalette()[:3 * (colours - 2)]).reshape(-1, 3).astype(np.float32)
    dark = (pal.min(0) * 0.0 + np.array([24, 20, 32]))       # one outline colour, a near-black with a cool tint
    pal = np.vstack([pal, dark])
    pal = (np.round(pal / 8) * 8).clip(0, 248)
    out = []
    for (rgb, m), e in zip(frames, edge):
        d = ((rgb[..., None, :] - pal[None, None, :-1]) ** 2).sum(-1)
        idx = d.argmin(-1)
        img = np.zeros(m.shape + (4,), np.uint8)
        img[..., :3] = pal[idx]
        img[e, :3] = (pal[idx][e] * outline + pal[-1] * (1 - outline))
        img[..., :3] = (np.round(img[..., :3] / 8) * 8).clip(0, 248)
        img[m, 3] = 255
        out.append(img)
    return out, pal

def pose(f, root, a, frame, deform=None):
    old_walk = hsd.walk
    def walk(ff, j, parent, out, joints, path=(), single=False):
        old_walk(ff, j, parent, out, joints, path, single)
        if not path and not single and deform: deform(ff, joints)
    hsd.walk = walk
    try:
        return anim.load_posed(f, root, a, frame)
    finally:
        hsd.walk = old_walk

def downsample2(big, zb, ib, ss, cover=0.4, detail=0.25):
    """Each output pixel: the part covering most of it, its colour the mean of that part's samples only."""
    H, W = big.shape[0] // ss, big.shape[1] // ss
    r = lambda x: x.reshape(H, ss, W, ss, *x.shape[2:]).swapaxes(1, 2).reshape(H, W, ss * ss, *x.shape[2:])
    c, z, i = r(big), r(zb), r(ib)
    rgb = np.zeros((H, W, 3)); dep = np.full((H, W), -1e30); part = np.full((H, W), -1)
    mask = np.zeros((H, W), bool)
    for y in range(H):
        for x in range(W):
            ids = i[y, x]; ok = ids >= 0
            if ok.sum() < cover * ss * ss: continue
            vals, cnt = np.unique(ids[ok], return_counts=True)
            p = vals[cnt.argmax()]; sel = ids == p
            cs = c[y, x][sel, :3]; lum = cs @ [0.3, 0.59, 0.11]
            dark = lum < np.median(lum) - 80   # a small dark detail (an eye, a mouth, a seam) is kept, not averaged away
            if detail * sel.sum() <= dark.sum() <= 0.5 * sel.sum():
                rgb[y, x] = cs[dark].mean(0)
            else:
                rgb[y, x] = cs[lum >= np.median(lum)].mean(0)   # the lit colour, as a pixel artist picks it
            dep[y, x] = z[y, x][sel].mean(); part[y, x] = p
            mask[y, x] = True
    return rgb, mask, dep, part

def kmeans(pts, w, k, it=30, seed=1):
    rng = np.random.default_rng(seed)
    cen = pts[rng.choice(len(pts), k, replace=False, p=w / w.sum())]
    for _ in range(it):
        lab = ((pts[:, None] - cen[None]) ** 2).sum(-1).argmin(1)
        for j in range(k):
            s = lab == j
            if s.any(): cen[j] = (pts[s] * w[s, None]).sum(0) / w[s].sum()
    return cen

def to_gba2(frames, colours=15, sat=1.25, contrast=1.12, zjump=1.2):
    """15 colours shared by every frame: one outline colour, the rest from k-means over the art where each part
    weighs the same however big it is (so a face is not lost to a coat); a silhouette outline, inner lines where
    the depth jumps (on the farther side)."""
    def punch(rgb):
        g = rgb.mean(-1, keepdims=True)
        x = g + (rgb - g) * sat
        return np.clip((x - 128) * contrast + 128, 0, 255)
    lines = []
    for rgb, m, dep, part in frames:
        pad = np.pad(m, 1)
        edge = m & ~(pad[:-2, 1:-1] & pad[2:, 1:-1] & pad[1:-1, :-2] & pad[1:-1, 2:])
        inner = np.zeros_like(m)
        for dy, dx in ((0, 1), (1, 0), (0, -1), (-1, 0)):
            nb = np.roll(np.roll(dep, -dy, 0), -dx, 1); nm = np.roll(np.roll(m, -dy, 0), -dx, 1)
            inner |= m & nm & (nb - dep > zjump)          # the neighbour is nearer by a jump: this side is behind
        lines.append((edge, inner & ~edge))
    pts, parts = [], []
    for (rgb, m, dep, part), (e, inn) in zip(frames, lines):
        sel = m & ~e & ~inn
        pts.append(punch(rgb[sel])); parts.append(part[sel])
    pts = np.concatenate(pts); parts = np.concatenate(parts)
    u, cnt = np.unique(parts, return_counts=True)
    w = 1.0 / np.sqrt(cnt[np.searchsorted(u, parts)])
    pal = kmeans(pts, w, colours - 1)
    pal = np.vstack([pal, [[24, 20, 32]]])
    pal = (np.round(pal / 8) * 8).clip(0, 248)
    out = []
    for (rgb, m, dep, part), (e, inn) in zip(frames, lines):
        x = punch(rgb)
        x[inn] = x[inn] * 0.45                                  # inner lines: the darkest shade near that colour
        idx = ((x[..., None, :] - pal[None, None, :-1]) ** 2).sum(-1).argmin(-1)
        img = np.zeros(m.shape + (4,), np.uint8)
        img[..., :3] = pal[idx]
        img[e, :3] = pal[-1]
        img[m, 3] = 255
        out.append(img)
    return out, pal
