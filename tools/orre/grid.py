#!/usr/bin/env python3
"""A Colosseum map's collision (.col) as the field engine's tile grid: which tiles can be walked on, and the ground's
heights there, for the 3D engine (src/sinnoh/s3d.c: ORRE's maps are on Emerald's grid like SINNOH's).

The .col layout is the game's (the Colosseum decomp: GScolsys2Walk.c ColGrid / ColTri, gs_colsys*.c): u32 offset of
the objects, u32 their count; each object 0x40 bytes: f32 translate xyz, rotate xyz (radians), scale xyz, then three
triangle grids (0 = none): +0x24 the floor (walk), +0x28 walls (hit), +0x2C event triggers (thru); u16 flags at
+0x3C. A grid: u32 triangles, u32 count, ... ; a triangle 0x34 bytes: f32 three corners, f32 normal, u8 surface/attr,
u8 layer, u16 id: a trigger's id is its event's collision id (the event table's: tools/orre/colo_rooms.py).

    python3 -I tools/orre/grid.py MAP.col [START_X START_Z]  (prints the grid: . walkable, # blocked, D trigger)
"""
import math
import struct
import sys

import numpy as np

TILE = 8.0          # Colosseum units a tile (a person is about two tiles tall: Kai's sprites are 32 pixels)
PERSON = 10.0       # a wall this high above the ground blocks a step
RADIUS = 2.5        # a person's half width (Colosseum units): a wall nearer a tile's centre than this blocks it
STEEP = 0.6         # floor normals flatter than this are too steep to walk
BLOCK_TILES = 32    # the field engine's maps are whole blocks of 32 x 32 tiles (s3d.c S3D_LayoutBlocks)


def _mtx(f):
    tx, ty, tz, rx, ry, rz, sx, sy, sz = f
    cx, snx, cy, sny, cz, snz = math.cos(rx), math.sin(rx), math.cos(ry), math.sin(ry), math.cos(rz), math.sin(rz)
    Rx = np.array([[1, 0, 0], [0, cx, -snx], [0, snx, cx]])
    Ry = np.array([[cy, 0, sny], [0, 1, 0], [-sny, 0, cy]])
    Rz = np.array([[cz, -snz, 0], [snz, cz, 0], [0, 0, 1]])
    return Rz @ Ry @ Rx @ np.diag([sx, sy, sz]), np.array([tx, ty, tz])


def read(data, open_gates=()):
    """{'floor': (n,3,3), 'wall': (n,3,3), 'trigger': [(id, (n,3,3))]} in the map's world (a trigger's id: its
    triangles' u16 id, the collision id the game's events name). OPEN_GATES: collision objects whose walls are left
    out (the story gates a room's scripts switch on and off: tools/orre/colo_rooms.py Room.gates)."""
    off, n = struct.unpack_from(">II", data, 0)
    out = {"floor": [], "wall": [], "trigger": []}
    for i in range(n):
        o = off + i * 0x40
        rot, t = _mtx(struct.unpack_from(">9f", data, o))
        for slot, kind in ((0x24, "floor"), (0x28, "wall"), (0x2C, "trigger")):
            p = struct.unpack_from(">I", data, o + slot)[0]
            if not p or (kind == "wall" and i in open_gates):
                continue
            tris, cnt = struct.unpack_from(">II", data, p)
            if cnt == 0 or tris + cnt * 0x34 > len(data):
                continue
            T = np.array([struct.unpack_from(">9f", data, tris + k * 0x34) for k in range(cnt)]).reshape(-1, 3, 3)
            T = T @ rot.T + t
            if kind == "trigger":
                ids = np.array([struct.unpack_from(">H", data, tris + k * 0x34 + 0x32)[0] for k in range(cnt)])
                for tid in sorted(set(ids.tolist())):
                    out["trigger"].append((tid, T[ids == tid]))
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


def _dist_many(p, A, B):
    """The distances from p to every segment A[i]-B[i] (x, z), as _dist."""
    d = B - A
    L = (d * d).sum(1)
    t = np.where(L == 0, 0.0, np.clip(((p[0] - A[:, 0]) * d[:, 0] + (p[1] - A[:, 1]) * d[:, 1]) / np.where(L == 0, 1, L), 0, 1))
    return np.hypot(p[0] - A[:, 0] - t * d[:, 0], p[1] - A[:, 1] - t * d[:, 1])


def _cross_many(a0, a1, B0, B1):
    """Which segments B0[i]-B1[i] the segment a0-a1 crosses, as _seg_cross."""
    def orient(p, q, r):
        return (q[..., 0] - p[..., 0]) * (r[..., 1] - p[..., 1]) - (q[..., 1] - p[..., 1]) * (r[..., 0] - p[..., 0])
    a0, a1 = np.asarray(a0, float), np.asarray(a1, float)
    d1, d2 = orient(B0, B1, a0), orient(B0, B1, a1)
    d3, d4 = orient(a0, a1, B0), orient(a0, a1, B1)
    return ((d1 > 0) != (d2 > 0)) & ((d3 > 0) != (d4 > 0))


FINE = 1.0   # the reach is worked out on a grid of FINE Colosseum units, walls thickened by RADIUS


def _reach(starts, tile, x0, z0, w, h, walk, heights, trig, segs, tsegs):
    """The tiles a person reaches from STARTS: a flood fill on a fine grid where every wall at a person's height is
    RADIUS thick (a gap passes only if a person fits through it), the floor's tiles only, no step over 6 units;
    trigger cells (doors, exits) are reached, not walked through. A tile is reached when the flood comes within 2
    units of its centre."""
    from collections import deque
    k = int(round(tile / FINE))
    W, H = w * k, h * k
    block = np.zeros((H, W), dtype=bool)
    trigf = np.full((H, W), -1, dtype=np.int32)
    ground = np.array([[heights[y][x][0] if heights[y][x] else np.nan for x in range(w)] for y in range(h)])
    usable = ~np.isnan(ground)   # (tiles with floor that isn't too steep: walk before walls)
    usable &= np.array([[bool(heights[y][x]) for x in range(w)] for y in range(h)])

    def cell(px, pz):
        return int((pz - z0) // FINE), int((px - x0) // FINE)

    for p0, p1, y0, y1 in segs:
        L = math.hypot(p1[0] - p0[0], p1[1] - p0[1])
        for t in np.linspace(0.0, 1.0, max(2, int(L / (FINE * 0.5)) + 1)):
            px, pz = p0[0] + (p1[0] - p0[0]) * t, p0[1] + (p1[1] - p0[1]) * t
            cz, cx = cell(px, pz)
            if not (0 <= cx < W and 0 <= cz < H):
                continue
            g = ground[cz // k, cx // k]
            if g != g or y1 < g + 1.0 or y0 > g + PERSON:
                continue
            block[cz, cx] = True
    r = int(math.ceil(RADIUS / FINE))
    grown = block.copy()
    for dz in range(-r, r + 1):
        for dx in range(-r, r + 1):
            if dx * dx + dz * dz <= (RADIUS / FINE) ** 2 and (dx or dz):
                grown |= np.roll(np.roll(block, dz, 0), dx, 1)
    block = grown
    for p0, p1, tid in tsegs:
        L = math.hypot(p1[0] - p0[0], p1[1] - p0[1])
        for t in np.linspace(0.0, 1.0, max(2, int(L / (FINE * 0.5)) + 1)):
            cz, cx = cell(p0[0] + (p1[0] - p0[0]) * t, p0[1] + (p1[1] - p0[1]) * t)
            for dz in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    if 0 <= cz + dz < H and 0 <= cx + dx < W:
                        trigf[cz + dz, cx + dx] = tid
    for y in range(h):
        for x in range(w):
            if not (heights[y][x]) or not usable[y, x]:
                block[y * k:(y + 1) * k, x * k:(x + 1) * k] = True
    seen = np.zeros((H, W), dtype=bool)
    q = deque()
    for start in starts:
        sz, sx = cell(*start)
        if not (0 <= sz < H and 0 <= sx < W):
            continue
        if block[sz, sx]:   # (an entry against a railing: the nearest spot a person fits, a few units off)
            near = [(abs(dz) + abs(dx), sz + dz, sx + dx) for dz in range(-6, 7) for dx in range(-6, 7)
                    if 0 <= sz + dz < H and 0 <= sx + dx < W and not block[sz + dz, sx + dx]]
            if not near:
                continue   # (an entry in a wall or off the floor: a place only a scene uses)
            _, sz, sx = min(near)
        seen[sz, sx] = True
        q.append((sz, sx))
    if not q:
        raise ValueError("every start is in a wall or off the floor: %s" % (starts,))
    while q:
        cz, cx = q.popleft()
        if trigf[cz, cx] >= 0:
            continue
        g0 = ground[cz // k, cx // k]
        for nz, nx in ((cz + 1, cx), (cz - 1, cx), (cz, cx + 1), (cz, cx - 1)):
            if 0 <= nz < H and 0 <= nx < W and not seen[nz, nx] and (not block[nz, nx] or trigf[nz, nx] >= 0):
                if abs(ground[nz // k, nx // k] - g0) > 6.0:
                    continue
                seen[nz, nx] = True
                q.append((nz, nx))
    out = np.zeros((h, w), dtype=bool)
    c = k // 2
    near = max(1, int(round(2.0 / FINE)))
    for y in range(h):
        for x in range(w):
            if seen[y * k + c - near:y * k + c + near + 1, x * k + c - near:x * k + c + near + 1].any():
                out[y, x] = True
            elif trig[y, x] >= 0:
                # (a door or an exit the flood touched: the trigger is drawn 3 cells thick and the flood stops at its
                # first cell, so a cell past the tile's edge counts too)
                zs, xs = slice(max(0, y * k - 1), (y + 1) * k + 1), slice(max(0, x * k - 1), (x + 1) * k + 1)
                if (seen[zs, xs] & (trigf[zs, xs] == trig[y, x])).any():
                    out[y, x] = True
    return out


def build(col, tile=TILE, start=None, stops=None):
    """-> dict: w, h (tiles, whole blocks), tile, x0, z0 (the grid's corner in Colosseum units), walk (h, w) bool,
    heights (h, w) list of floats, trigger (h, w) collision id or -1. START (x, z in Colosseum units, or a list of
    them: a room's entries): only what can be reached from there is walkable (the insides of props' wall outlines,
    the world past a map's exit triggers); STOPS (collision ids: the triggers that lead somewhere, every trigger when
    None) are reached but not walked through, and only they mark their tiles."""
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
    SA = np.array([s_[0] for s_ in segs]).reshape(-1, 2)
    SB = np.array([s_[1] for s_ in segs]).reshape(-1, 2)
    SY0 = np.array([s_[2] for s_ in segs])
    SY1 = np.array([s_[3] for s_ in segs])
    SLO, SHI = np.minimum(SA, SB), np.maximum(SA, SB)
    tsegs = []
    for tid, TT in col["trigger"]:
        if stops is not None and tid not in stops:
            continue
        for T in TT:
            pts = T[:, [0, 2]]
            for a, b in ((0, 1), (1, 2), (2, 0)):
                tsegs.append((pts[a], pts[b], tid))
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
            # (a person standing at the centre: a wall within RADIUS of it; narrow aisles of Colosseum's rooms stay
            # open, where a test of the whole tile closed them)
            near = ((SY1 >= g + 1.0) & (SY0 <= g + PERSON) & (SLO[:, 0] <= cx + RADIUS) & (SHI[:, 0] >= cx - RADIUS)
                    & (SLO[:, 1] <= cz + RADIUS) & (SHI[:, 1] >= cz - RADIUS))
            blocked = bool(near.any() and (_dist_many((cx, cz), SA[near], SB[near]) < RADIUS).any())
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
        lo, hi = np.minimum(a, b), np.maximum(a, b)
        near = np.nonzero((SY1 >= g + 1.0) & (SY0 <= g + PERSON) & (SLO[:, 0] <= hi[0]) & (SHI[:, 0] >= lo[0])
                          & (SLO[:, 1] <= hi[1]) & (SHI[:, 1] >= lo[1]))[0]
        if len(near):
            hit = near[_cross_many(a, b, SA[near], SB[near])]
            if len(hit):
                return (SA[hit[0]], SB[hit[0]])
        return None

    if start is not None:
        starts = [start] if isinstance(start[0], (int, float)) else list(start)
        walk = _reach(starts, tile, x0, z0, w, h, walk, heights, trig, segs, tsegs)
        # Emerald's grid has no walls between tiles: where one still parts two walkable tiles, the tile nearer the
        # wall is blocked
        for y in range(h):
            for x in range(w):
                for nx, ny in ((x + 1, y), (x, y + 1)):
                    if nx < w and ny < h and walk[y, x] and walk[ny, nx]:
                        if trig[y, x] >= 0 or trig[ny, nx] >= 0:
                            continue   # (a door's frame: its trigger is stepped onto across it)
                        wseg = wall_between(x, y, nx, ny)
                        if wseg:
                            da, db = _dist(centre(x, y), *wseg), _dist(centre(nx, ny), *wseg)
                            if db <= da:
                                walk[ny, nx] = False
                            else:
                                walk[y, x] = False
        _join_diagonals(walk, heights, trig)
        if stops:
            _connect(walk, heights, trig, stops)
    return {"w": w, "h": h, "tile": tile, "x0": x0, "z0": z0, "walk": walk, "heights": heights, "trigger": trig}


def _components(walk):
    """Each walkable tile's 4-connected area (-1: not walkable)."""
    from collections import deque
    h, w = walk.shape
    comp = np.full((h, w), -1, dtype=np.int32)
    n = 0
    for y0, x0_ in np.argwhere(walk):
        if comp[y0, x0_] >= 0:
            continue
        comp[y0, x0_] = n
        q = deque([(y0, x0_)])
        while q:
            cy, cx = q.popleft()
            for ny, nx in ((cy + 1, cx), (cy - 1, cx), (cy, cx + 1), (cy, cx - 1)):
                if 0 <= ny < h and 0 <= nx < w and walk[ny, nx] and comp[ny, nx] < 0:
                    comp[ny, nx] = n
                    q.append((ny, nx))
        n += 1
    return comp


def _join_diagonals(walk, heights, trig):
    """Colosseum's player walks any way; Emerald's steps are up, down, left and right. Where the flood reached two
    areas that touch only at a corner (a stair's turn, a plaza's angled edge: PHENAC's stadium plaza), one of the two
    tiles beside that corner is opened, the one whose floor lies nearest both areas' (or, floorless, given theirs),
    so the two are one walk apart."""
    h, w = walk.shape
    changed = True
    while changed:
        changed = False
        comp = _components(walk)
        for y in range(h - 1):
            for x in range(w - 1):
                for (ay, ax), (by, bx), cands in (((y, x), (y + 1, x + 1), ((y, x + 1), (y + 1, x))),
                                                  ((y, x + 1), (y + 1, x), ((y, x), (y + 1, x + 1)))):
                    if not (walk[ay, ax] and walk[by, bx]) or comp[ay, ax] == comp[by, bx]:
                        continue
                    if any(walk[cy, cx] for cy, cx in cands):
                        continue
                    ga, gb = heights[ay][ax][0], heights[by][bx][0]
                    best, cost = None, None
                    for cy, cx in cands:
                        if trig[cy, cx] >= 0:
                            continue
                        hs = heights[cy][cx]
                        c = min(abs(hh - ga) + abs(hh - gb) for hh in hs) if hs else abs(ga - gb) + 1.0
                        if cost is None or c < cost:
                            best, cost = (cy, cx), c
                    if best is None or abs(ga - gb) > 6.0:   # (a ledge or a floor above another: not a corner)
                        continue
                    cy, cx = best
                    if not heights[cy][cx]:
                        heights[cy][cx] = [(ga + gb) / 2]
                    walk[cy, cx] = True
                    changed = True
                    comp = _components(walk)


CONNECT = 3   # tiles: how far a door's trigger may lie from what is reached and still be joined to it


def _connect(walk, heights, trig, stops):
    """Doors whose trigger the reach missed: it sits in a door frame the thickened walls keep the flood out of, a
    tile or so from what was reached. The shortest way (CONNECT tiles at most) from the reached tiles to one of its
    tiles is opened, every tile of it given the ground of the tile it was entered from (a door frame has no floor of
    its own in some rooms); a trigger tile so reached is walkable, and stepped on it warps."""
    from collections import deque
    h, w = walk.shape
    for tid in sorted(stops):
        tiles = trig == tid
        if not tiles.any():
            continue
        # (entered when one of its walkable tiles is beside a walkable tile that isn't a trigger)
        plain = walk & (trig < 0)
        beside = np.zeros_like(walk)
        beside[1:] |= plain[:-1]
        beside[:-1] |= plain[1:]
        beside[:, 1:] |= plain[:, :-1]
        beside[:, :-1] |= plain[:, 1:]
        if (walk & tiles & beside).any():
            continue
        prev = {}
        dist = {(int(y), int(x)): 0 for y, x in np.argwhere(plain)}
        q = deque(dist)
        found = None
        while q and found is None:
            cy, cx = q.popleft()
            if dist[(cy, cx)] >= CONNECT:
                continue
            for ny, nx in ((cy + 1, cx), (cy - 1, cx), (cy, cx + 1), (cy, cx - 1)):
                if 0 <= ny < h and 0 <= nx < w and (ny, nx) not in dist and (trig[ny, nx] < 0 or trig[ny, nx] == tid):
                    dist[(ny, nx)] = dist[(cy, cx)] + 1
                    prev[(ny, nx)] = (cy, cx)
                    if tiles[ny, nx]:
                        found = (ny, nx)
                        break
                    q.append((ny, nx))
        if found is None:
            continue
        path = [found]
        while path[-1] in prev:
            path.append(prev[path[-1]])
        for k in range(len(path) - 2, -1, -1):   # (from the reached tile out to the trigger)
            y, x = path[k]
            py, px = path[k + 1]
            if not heights[y][x]:
                heights[y][x] = list(heights[py][px])
            walk[y, x] = True


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
