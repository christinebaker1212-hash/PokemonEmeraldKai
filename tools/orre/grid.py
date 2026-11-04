#!/usr/bin/env python3
"""A Colosseum map's collision (.col) as the field engine's tile grid: which tiles can be walked on, and the ground's
heights there, for the 3D engine (src/sinnoh/s3d.c: ORRE's maps are on Emerald's grid like SINNOH's).

The .col layout is the game's (the Colosseum decomp: GScolsys2Walk.c ColGrid / ColTri, gs_colsys*.c): u32 offset of
the objects, u32 their count; each object 0x40 bytes: f32 translate xyz, rotate xyz (radians), scale xyz, then three
triangle grids (0 = none): +0x24 the floor (walk), +0x28 walls (hit), +0x2C event triggers (thru); u16 flags at
+0x3C. A grid: u32 triangles, u32 count, ... ; a triangle 0x34 bytes: f32 three corners, f32 normal, u8 surface/attr,
u8 layer, u16 id.

    python3 -I tools/orre/grid.py MAP.col [START_X START_Z]  (prints the grid: . walkable, # blocked, D trigger)
"""
import math
import struct
import sys

import numpy as np

TILE = 8.0          # Colosseum units a tile (a person is about two tiles tall: Kai's sprites are 32 pixels)
PERSON = 10.0       # a wall this high above the ground blocks a step
STEEP = 0.6         # floor normals flatter than this are too steep to walk
BLOCK_TILES = 32    # the field engine's maps are whole blocks of 32 x 32 tiles (s3d.c S3D_LayoutBlocks)


def _mtx(f):
    tx, ty, tz, rx, ry, rz, sx, sy, sz = f
    cx, snx, cy, sny, cz, snz = math.cos(rx), math.sin(rx), math.cos(ry), math.sin(ry), math.cos(rz), math.sin(rz)
    Rx = np.array([[1, 0, 0], [0, cx, -snx], [0, snx, cx]])
    Ry = np.array([[cy, 0, sny], [0, 1, 0], [-sny, 0, cy]])
    Rz = np.array([[cz, -snz, 0], [snz, cz, 0], [0, 0, 1]])
    return Rz @ Ry @ Rx @ np.diag([sx, sy, sz]), np.array([tx, ty, tz])


def read(data):
    """{'floor': (n,3,3), 'wall': (n,3,3), 'trigger': [(id, (n,3,3))]} in the map's world."""
    off, n = struct.unpack_from(">II", data, 0)
    out = {"floor": [], "wall": [], "trigger": []}
    for i in range(n):
        o = off + i * 0x40
        rot, t = _mtx(struct.unpack_from(">9f", data, o))
        for slot, kind in ((0x24, "floor"), (0x28, "wall"), (0x2C, "trigger")):
            p = struct.unpack_from(">I", data, o + slot)[0]
            if not p:
                continue
            tris, cnt = struct.unpack_from(">II", data, p)
            if cnt == 0 or tris + cnt * 0x34 > len(data):
                continue
            T = np.array([struct.unpack_from(">9f", data, tris + k * 0x34) for k in range(cnt)]).reshape(-1, 3, 3)
            T = T @ rot.T + t
            if kind == "trigger":
                out["trigger"].append((i, T))
            else:
                out[kind].append(T)
    for k in ("floor", "wall"):
        out[k] = np.concatenate(out[k]) if out[k] else np.zeros((0, 3, 3))
    return out


def _heights(floor, x, z):
    """The floor's heights under (x, z), lowest first, and whether each is flat enough to stand on."""
    a, b, c = floor[:, 0], floor[:, 1], floor[:, 2]
    v0x, v0z = c[:, 0] - a[:, 0], c[:, 2] - a[:, 2]
    v1x, v1z = b[:, 0] - a[:, 0], b[:, 2] - a[:, 2]
    v2x, v2z = x - a[:, 0], z - a[:, 2]
    den = v0x * v1z - v1x * v0z
    ok = np.abs(den) > 1e-9
    den = np.where(ok, den, 1.0)
    u = (v2x * v1z - v1x * v2z) / den
    v = (v0x * v2z - v2x * v0z) / den
    inside = ok & (u >= -1e-4) & (v >= -1e-4) & (u + v <= 1 + 1e-4)
    if not inside.any():
        return []
    y = a[:, 1] + u * (c[:, 1] - a[:, 1]) + v * (b[:, 1] - a[:, 1])
    n = np.cross(b - a, c - a)
    ny = np.abs(n[:, 1]) / np.maximum(np.linalg.norm(n, axis=1), 1e-9)
    hs = sorted(set((round(float(y[k]), 2), bool(ny[k] >= STEEP)) for k in np.nonzero(inside)[0]))
    merged = []
    for h, flat in hs:   # (one height per surface: triangles meeting at an edge give the same one)
        if merged and abs(merged[-1][0] - h) < 1.0:
            merged[-1] = (merged[-1][0], merged[-1][1] and flat)
        else:
            merged.append((h, flat))
    return merged


def _seg_hits_box(p0, p1, x0, z0, x1, z1):
    """Does the segment p0-p1 (x, z) cross the rectangle? (Liang-Barsky)"""
    dx, dz = p1[0] - p0[0], p1[1] - p0[1]
    t0, t1 = 0.0, 1.0
    for p, q in ((-dx, p0[0] - x0), (dx, x1 - p0[0]), (-dz, p0[1] - z0), (dz, z1 - p0[1])):
        if abs(p) < 1e-12:
            if q < 0:
                return False
        else:
            r = q / p
            if p < 0:
                t0 = max(t0, r)
            else:
                t1 = min(t1, r)
            if t0 > t1:
                return False
    return True


def _seg_cross(a0, a1, b0, b1):
    """Do the segments a0-a1 and b0-b1 (x, z) cross?"""
    def orient(p, q, r):
        return (q[0] - p[0]) * (r[1] - p[1]) - (q[1] - p[1]) * (r[0] - p[0])
    d1, d2 = orient(b0, b1, a0), orient(b0, b1, a1)
    d3, d4 = orient(a0, a1, b0), orient(a0, a1, b1)
    return ((d1 > 0) != (d2 > 0)) and ((d3 > 0) != (d4 > 0))


def _dist(p, a, b):
    """The distance from p to the segment a-b (x, z)."""
    ax, az, bx, bz = a[0], a[1], b[0], b[1]
    dx, dz = bx - ax, bz - az
    L = dx * dx + dz * dz
    t = 0.0 if L == 0 else max(0.0, min(1.0, ((p[0] - ax) * dx + (p[1] - az) * dz) / L))
    return math.hypot(p[0] - ax - t * dx, p[1] - az - t * dz)


def build(col, tile=TILE, start=None):
    """-> dict: w, h (tiles, whole blocks), tile, x0, z0 (the grid's corner in Colosseum units), walk (h, w) bool,
    heights (h, w) list of floats, trigger (h, w) object id or -1. START (x, z in Colosseum units): only what can be
    reached from there is walkable (the insides of props' wall outlines, the world past a map's exit triggers);
    trigger tiles (doors, exits) are reached but not walked through."""
    fl, wl = col["floor"], col["wall"]
    if len(fl) == 0:
        raise ValueError("no floor")
    P = fl.reshape(-1, 3)
    lo, hi = P.min(0), P.max(0)
    tw = int(math.ceil((hi[0] - lo[0]) / tile)) + 2
    th = int(math.ceil((hi[2] - lo[2]) / tile)) + 2
    w = max(1, math.ceil(tw / BLOCK_TILES)) * BLOCK_TILES
    h = max(1, math.ceil(th / BLOCK_TILES)) * BLOCK_TILES
    # the floor in the middle of the blocks
    x0 = (lo[0] + hi[0]) / 2 - w * tile / 2
    z0 = (lo[2] + hi[2]) / 2 - h * tile / 2
    x0, z0 = math.floor(x0 / tile) * tile, math.floor(z0 / tile) * tile
    walk = np.zeros((h, w), dtype=bool)
    heights = [[[] for _ in range(w)] for _ in range(h)]
    trig = np.full((h, w), -1, dtype=np.int32)
    # walls as segments on the ground plan, with the heights they span
    segs = []
    for T in wl:
        ys = T[:, 1]
        pts = T[:, [0, 2]]
        for a, b in ((0, 1), (1, 2), (2, 0)):
            segs.append((pts[a], pts[b], ys.min(), ys.max()))
    tsegs = []
    for tid, TT in col["trigger"]:
        for T in TT:
            pts = T[:, [0, 2]]
            for a, b in ((0, 1), (1, 2), (2, 0)):
                tsegs.append((pts[a], pts[b], tid))
    m = tile * 0.15   # (a wall right on a tile's edge blocks only the tile it leans into)
    for ty in range(h):
        for tx in range(w):
            cx, cz = x0 + (tx + 0.5) * tile, z0 + (ty + 0.5) * tile
            hs = _heights(fl, cx, cz)
            if not hs:
                continue
            heights[ty][tx] = [hh for hh, _ in hs][:3]
            g, flat = hs[0]
            if not flat:
                continue
            bx0, bz0, bx1, bz1 = cx - tile / 2 + m, cz - tile / 2 + m, cx + tile / 2 - m, cz + tile / 2 - m
            blocked = False
            for p0, p1, y0, y1 in segs:
                if y1 < g + 1.0 or y0 > g + PERSON:
                    continue
                if _seg_hits_box(p0, p1, bx0, bz0, bx1, bz1):
                    blocked = True
                    break
            walk[ty, tx] = not blocked
            for p0, p1, tid in tsegs:
                if _seg_hits_box(p0, p1, cx - tile / 2, cz - tile / 2, cx + tile / 2, cz + tile / 2):
                    trig[ty, tx] = tid
                    break
    # a step between two tiles is blocked by a wall that crosses it at a person's height (walls on a tile's edge
    # block the step, not either tile)
    def centre(x, y):
        return (x0 + (x + 0.5) * tile, z0 + (y + 0.5) * tile)

    def wall_between(x, y, nx, ny):
        a, b = centre(x, y), centre(nx, ny)
        g = min(heights[y][x][0], heights[ny][nx][0])
        for p0, p1, y0, y1 in segs:
            if y1 < g + 1.0 or y0 > g + PERSON:
                continue
            if _seg_cross(a, b, p0, p1):
                return (p0, p1)
        return None

    if start is not None:
        sx, sz = int((start[0] - x0) // tile), int((start[1] - z0) // tile)
        if not (0 <= sx < w and 0 <= sz < h) or not walk[sz, sx] or trig[sz, sx] >= 0:
            raise ValueError("the start (%d, %d) isn't a walkable tile" % (sx, sz))
        seen = np.zeros_like(walk)
        stack = [(sx, sz)]
        seen[sz, sx] = True
        while stack:
            x, y = stack.pop()
            if trig[y, x] >= 0:
                continue   # (a door or an exit: reached, not walked through)
            for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                if 0 <= nx < w and 0 <= ny < h and walk[ny, nx] and not seen[ny, nx]:
                    if abs(heights[ny][nx][0] - heights[y][x][0]) > 6.0:
                        continue   # (a step too high: a ledge, a wall the floor runs up)
                    if wall_between(x, y, nx, ny):
                        continue
                    seen[ny, nx] = True
                    stack.append((nx, ny))
        walk = seen
        # Emerald's grid has no walls between tiles: where one still parts two walkable tiles, the tile nearer the
        # wall is blocked
        for y in range(h):
            for x in range(w):
                for nx, ny in ((x + 1, y), (x, y + 1)):
                    if nx < w and ny < h and walk[y, x] and walk[ny, nx]:
                        wseg = wall_between(x, y, nx, ny)
                        if wseg:
                            da, db = _dist(centre(x, y), *wseg), _dist(centre(nx, ny), *wseg)
                            if trig[y, x] >= 0 or (db <= da and trig[ny, nx] < 0):
                                walk[ny, nx] = False
                            else:
                                walk[y, x] = False
    return {"w": w, "h": h, "tile": tile, "x0": x0, "z0": z0, "walk": walk, "heights": heights, "trigger": trig}


def show(g):
    rows = []
    for y in range(g["h"]):
        rows.append("".join("D" if g["trigger"][y, x] >= 0 else ("." if g["walk"][y, x] else "#") for x in range(g["w"])))
    return "\n".join(rows)


if __name__ == "__main__":
    st = (float(sys.argv[2]), float(sys.argv[3])) if len(sys.argv) > 3 else None
    g = build(read(open(sys.argv[1], "rb").read()), TILE, st)
    print("%d x %d tiles, corner %.1f, %.1f" % (g["w"], g["h"], g["x0"], g["z0"]))
    print(show(g))
