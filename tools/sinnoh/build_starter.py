#!/usr/bin/env python3
"""Platinum's briefcase on ROUTE 201 (its starter screen, pokeplatinum src/choose_starter/choose_starter_app.c) drawn
ahead of time from its own models and animations (res/prebuilt/graphic/ev_pokeselect.narc), for the game's SINNOH
starter screen (src/sinnoh/s3d_starter.c). Pure Python (the release's runner has no Pillow or numpy).

    python3 -I tools/sinnoh/build_starter.py POKEPLATINUM_DIR [--preview DIR]   (section() for build_gfx.py)

The NARC: 0 the case's opening (joint animation, 41 frames), 1 the closed case, 2/4/6 the balls' bounce (73 frames),
3/5/7 the three balls, 8 the open case, 9 the ground; 10-13 the cursor (char, palette, cells, animation), 14/15 the
preview window, 16/17 palettes. Platinum's camera: 22 degrees of half field of view, looking down 30 degrees from 300
units at the case, then in six frames 50 degrees from 200 units at (0, 0, 36); one unit a pixel at the DS's scale, the
game's 240x160 the middle of the DS's 256x192 (as the field's 3D, render_ref.py). Lit by a white light straight ahead,
dark gray edges (the DS's edge marking) between polygon groups and around the scene.

Section "STRT" (little endian): u32 size of what follows; u16 w, h (240, 160); u16 opening frames, camera frames;
the opening's frames then the camera's, each w*h RGB555 (bit 15: drawn; clear: the backdrop); u16 bounce frames;
for each ball: u16 x, y, w, h (the patch over the last camera frame), then its bounce frames' patches (w*h RGB555);
s16 the glove's middle x, y for each ball (the DS's spots across less 8; 10 above the ball's patch).
"""
import math
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nds    # noqa: E402
import nitro  # noqa: E402

W, H = 240, 160
DS_W, DS_H = 256, 192
HALF_FOV = math.radians(22.0)
FOCAL = (DS_H / 2) / math.tan(HALF_FOV)
EDGE = (8, 8, 8)            # the edge color (COLOR_DARK_GRAY), 0-31 each
BALLS = [(-44, -4, 32), (0, -4, 62), (38, -4, 26)]   # app->selectionMatrix
CURSOR = [(78, 55), (130, 82), (172, 50)]           # app->otherSelectionMatrix (the DS's screen)
NARC = 'res/prebuilt/graphic/ev_pokeselect.narc'


class Obj:
    def __init__(self, bmd, anim=None, pos=(0, 0, 0), scale=(1, 1, 1), rot_y=0.0):
        self.bmd = bmd
        self.model = bmd.models[0]
        self.anim = anim
        self.pos, self.scale, self.rot_y = pos, scale, rot_y
        self.tex = {}

    def texture(self, mat):
        key = (mat.texture, mat.palette)
        if key not in self.tex:
            if mat.texture is None or self.bmd.tex is None:
                self.tex[key] = None
            else:
                self.tex[key] = self.bmd.tex.decode(mat.texture, mat.palette)
        return self.tex[key]


def camera(pitch_deg, dist, target):
    """(eye, right, up, fwd): Platinum's camera at PITCH (negative: looking down) around TARGET"""
    a = math.radians(-pitch_deg)
    eye = (target[0], target[1] + dist * math.sin(a), target[2] + dist * math.cos(a))
    f = [target[i] - eye[i] for i in range(3)]
    n = math.sqrt(sum(x * x for x in f))
    f = [x / n for x in f]
    up0 = (0.0, 1.0, 0.0)
    r = [f[1] * up0[2] - f[2] * up0[1], f[2] * up0[0] - f[0] * up0[2], f[0] * up0[1] - f[1] * up0[0]]
    n = math.sqrt(sum(x * x for x in r))
    r = [x / n for x in r]
    u = [r[1] * f[2] - r[2] * f[1], r[2] * f[0] - r[0] * f[2], r[0] * f[1] - r[1] * f[0]]
    return eye, r, u, f


def project(cam, p):
    eye, r, u, f = cam
    d = [p[i] - eye[i] for i in range(3)]
    z = sum(d[i] * f[i] for i in range(3))
    if z < 1.0:
        return None
    x = sum(d[i] * r[i] for i in range(3))
    y = sum(d[i] * u[i] for i in range(3))
    return (W / 2 + FOCAL * x / z, H / 2 - FOCAL * y / z, z)


def wrap(i, size, repeat, flip):
    if not repeat:
        return 0 if i < 0 else size - 1 if i >= size else i
    if flip:
        m = i % (2 * size)
        return 2 * size - 1 - m if m >= size else m
    return i % size


class Canvas:
    def __init__(self, clip=(0, 0, W, H)):
        self.clip = clip
        self.color = [None] * (W * H)   # (r, g, b) 0-31, None: the backdrop
        self.depth = [math.inf] * (W * H)
        self.pid = [-1] * (W * H)       # the polygon group drawn (edge marking)

    def draw(self, cam, obj, frame, gid):
        m = obj.model
        mats = obj.anim.node_matrices(m, frame) if obj.anim is not None else None
        cy, sy = math.cos(obj.rot_y), math.sin(obj.rot_y)
        light = (0.0, 0.0, -1.0)
        groups = []
        for mi, tris in m.draw(mats).items():
            groups.append((m.materials[mi], tris))
        groups.sort(key=lambda g: g[0].alpha < 31)   # opaque first
        for mat, tris in groups:
            img = obj.texture(mat)
            dif = nitro.rgb555(mat.dif_amb & 0x7FFF)
            amb = nitro.rgb555((mat.dif_amb >> 16) & 0x7FFF)
            emi = nitro.rgb555((mat.spe_emi >> 16) & 0x7FFF)
            vtx_dif = bool(mat.dif_amb & 0x8000)   # (the diffuse is the vertex color)
            pid = gid * 64 + ((mat.poly_attr >> 24) & 63)
            for tri in tris:
                pts = []
                for pos, uv, vcol, nrm in tri:
                    x, y, z = pos[0] * obj.scale[0], pos[1] * obj.scale[1], pos[2] * obj.scale[2]
                    x, z = cy * x + sy * z, -sy * x + cy * z
                    w = (x + obj.pos[0], y + obj.pos[1], z + obj.pos[2])
                    s = project(cam, w)
                    if s is None:
                        break
                    col = vcol
                    if mat.lights & 1:
                        nx, nz = cy * nrm[0] + sy * nrm[2], -sy * nrm[0] + cy * nrm[2]
                        ny = nrm[1]
                        ln = math.sqrt(nx * nx + ny * ny + nz * nz) or 1.0
                        level = max(0.0, -(light[0] * nx + light[1] * ny + light[2] * nz) / ln)
                        d = vcol if vtx_dif else dif
                        col = tuple(min(31.0, emi[k] + d[k] * level + amb[k]) for k in range(3))
                    pts.append((s[0], s[1], s[2], uv, col))
                if len(pts) == 3:
                    self.tri(pts, img, mat, pid)

    def tri(self, pts, img, mat, pid):
        (x0, y0, z0, uv0, c0), (x1, y1, z1, uv1, c1), (x2, y2, z2, uv2, c2) = pts
        area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)
        if area == 0:
            return
        cull = mat.cull
        if (cull == 'back' and area > 0) or (cull == 'front' and area < 0) or cull == 'all':
            return
        cx0, cy0, cx1, cy1 = self.clip
        minx = max(cx0, int(math.floor(min(x0, x1, x2))))
        maxx = min(cx1 - 1, int(math.ceil(max(x0, x1, x2))))
        miny = max(cy0, int(math.floor(min(y0, y1, y2))))
        maxy = min(cy1 - 1, int(math.ceil(max(y0, y1, y2))))
        if minx > maxx or miny > maxy:
            return
        iz0, iz1, iz2 = 1 / z0, 1 / z1, 1 / z2
        alpha_m = mat.alpha / 31.0
        translucent = mat.alpha < 31
        mode = mat.mode
        if img is not None:
            tw, th, texels = img
            rs, rt, fs, ft = mat.repeat
        color, depth, pids = self.color, self.depth, self.pid
        inv = 1.0 / area
        for py in range(miny, maxy + 1):
            ys = py + 0.5
            for px in range(minx, maxx + 1):
                xs = px + 0.5
                w0 = ((x1 - xs) * (y2 - ys) - (x2 - xs) * (y1 - ys)) * inv
                if w0 < 0:
                    continue
                w1 = ((x2 - xs) * (y0 - ys) - (x0 - xs) * (y2 - ys)) * inv
                if w1 < 0:
                    continue
                w2 = 1.0 - w0 - w1
                if w2 < 0:
                    continue
                q0, q1, q2 = w0 * iz0, w1 * iz1, w2 * iz2
                iz = q0 + q1 + q2
                z = 1.0 / iz
                k = py * W + px
                if z >= depth[k]:
                    continue
                q0, q1, q2 = q0 * z, q1 * z, q2 * z
                shade = (q0 * c0[0] + q1 * c1[0] + q2 * c2[0], q0 * c0[1] + q1 * c1[1] + q2 * c2[1],
                         q0 * c0[2] + q1 * c1[2] + q2 * c2[2])
                a = 1.0
                if img is not None:
                    u = q0 * uv0[0] + q1 * uv1[0] + q2 * uv2[0]
                    v = q0 * uv0[1] + q1 * uv1[1] + q2 * uv2[1]
                    t = texels[wrap(int(math.floor(v)), th, rt, ft) * tw + wrap(int(math.floor(u)), tw, rs, fs)]
                    if t[3] == 0:
                        continue
                    rgb = (t[0] * 31 / 255, t[1] * 31 / 255, t[2] * 31 / 255)
                    if mode == 0:
                        out = tuple((rgb[i] + 1) * (shade[i] + 1) / 32 - 1 for i in range(3))
                    else:
                        out = rgb
                    a = t[3] / 255
                else:
                    out = shade
                a *= alpha_m
                if translucent or a < 1:
                    if translucent and pids[k] == pid:
                        continue
                    under = color[k] or (0, 0, 0)
                    out = tuple(out[i] * a + under[i] * (1 - a) for i in range(3))
                    if color[k] is None and a < 0.5:
                        continue
                color[k] = out
                if not translucent:
                    depth[k] = z
                    pids[k] = pid

    def edges(self):
        """The DS's edge marking: a pixel whose group differs from a neighbor's that is farther (or the backdrop)"""
        cx0, cy0, cx1, cy1 = self.clip
        out = list(self.color)
        for py in range(cy0, cy1):
            for px in range(cx0, cx1):
                k = py * W + px
                if self.color[k] is None:
                    continue
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    qx, qy = px + dx, py + dy
                    if not (0 <= qx < W and 0 <= qy < H):
                        continue
                    q = qy * W + qx
                    if self.pid[q] != self.pid[k] and self.depth[q] > self.depth[k]:
                        out[k] = EDGE
                        break
        return out


def rgb555(c):
    if c is None:
        return 0
    r, g, b = (max(0, min(31, int(round(x)))) for x in c)
    return r | g << 5 | b << 10 | 0x8000


class Scene:
    def __init__(self, root):
        narc = nds.Narc(open(os.path.join(root, NARC), 'rb').read())
        self.narc = narc
        bmd = lambda i: nitro.Bmd0(narc[i])
        bca = lambda i: nitro.Bca0(narc[i]).anims[0]
        self.closed = Obj(bmd(1), bca(0))
        self.open = Obj(bmd(8))
        self.balls = [Obj(bmd(3 + 2 * i), bca(2 + 2 * i), pos=BALLS[i]) for i in range(3)]
        self.ground = Obj(bmd(9), pos=(0, -28, 40), scale=(3.5, 1, 3.5), rot_y=math.pi)

    def frame(self, cam, objs, clip=(0, 0, W, H)):
        c = Canvas(clip)
        for gid, (o, f) in enumerate(objs):
            c.draw(cam, o, f, gid)
        return [rgb555(x) for x in c.edges()]


def camera_at(i, n=6):
    """the camera's i-th frame of its move (0: before it, n: done), as AdvanceStarterMovement steps it"""
    t = min(i, n) / n
    pitch = -30 + (-50 + 30) * t
    dist = 300 + (200 - 300) * t
    return camera(pitch, dist, (0.0, 0.0, 36 * t))


def save_png(path, px, w=W, h=H):
    import zlib
    raw = b''.join(b'\0' + bytes(v for p in px[y * w:(y + 1) * w]
                                 for v in ((p & 31) * 255 // 31, (p >> 5 & 31) * 255 // 31, (p >> 10 & 31) * 255 // 31))
                   for y in range(h))
    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xFFFFFFFF)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))


def cursor_image(narc):
    """Platinum's glove (NARC 10: 32x32 in 1D tiles, palette 11) as 32*32 RGB555 (bit 15: drawn)"""
    d = narc[10]
    size, off = struct.unpack_from('<II', d, 0x28)
    ch = d[0x10 + off:0x10 + off + size]
    pd = narc[11][0x28:]
    pal = [struct.unpack_from('<H', pd, 2 * i)[0] for i in range(16)]
    px = [0] * (32 * 32)
    for t in range(16):
        tx, ty = t % 4, t // 4
        for y in range(8):
            for x in range(8):
                b = ch[t * 32 + y * 4 + x // 2]
                v = (b >> 4) if x & 1 else b & 15
                if v:
                    px[(ty * 8 + y) * 32 + tx * 8 + x] = pal[v] | 0x8000
    return px


def ball_rect(sc, i, cam):
    """the screen rectangle the ball's bounce covers (with its shadow and outline), clipped to the screen"""
    xs, ys = [], []
    b = sc.balls[i]
    for f in range(0, b.anim.frames, 4):
        mats = b.anim.node_matrices(b.model, f)
        for ts in b.model.draw(mats).values():
            for tri in ts:
                for pos, _, _, _ in tri:
                    q = project(cam, (pos[0] + b.pos[0], pos[1] + b.pos[1], pos[2] + b.pos[2]))
                    if q:
                        xs.append(q[0])
                        ys.append(q[1])
    x0, y0 = max(0, int(min(xs)) - 3), max(0, int(min(ys)) - 3)
    x1, y1 = min(W, int(max(xs)) + 4), min(H, int(max(ys)) + 4)
    return x0, y0, x1 - x0, y1 - y0


def section(root, preview=None, log=print):
    """the STRT section (see the top)"""
    import time
    t0 = time.time()
    narc_sc = Scene(root)
    sc = narc_sc
    out = bytearray()
    frames = []
    for f in range(sc.closed.anim.frames):
        frames.append(sc.frame(camera_at(0), [(sc.ground, 0), (sc.closed, f)]))
    n_open = len(frames)
    for i in range(7):
        frames.append(sc.frame(camera_at(i), [(sc.ground, 0), (sc.open, 0)] + [(b, 0) for b in sc.balls]))
    out += struct.pack('<4H', W, H, n_open, len(frames) - n_open)
    for px in frames:
        out += struct.pack('<%dH' % len(px), *px)
    cam = camera_at(6)
    n_b = sc.balls[0].anim.frames
    out += struct.pack('<H', n_b)
    patches = []
    for i in range(3):
        x, y, w, h = ball_rect(sc, i, cam)
        clip = (max(0, x - 2), max(0, y - 2), min(W, x + w + 2), min(H, y + h + 2))
        out += struct.pack('<4H', x, y, w, h)
        for f in range(n_b):
            objs = [(sc.ground, 0), (sc.open, 0)] + [(b, f if k == i else 0) for k, b in enumerate(sc.balls)]
            px = sc.frame(cam, objs, clip)
            patch = [px[(y + r) * W + x + c] for r in range(h) for c in range(w)]
            out += struct.pack('<%dH' % len(patch), *patch)
            if f % 12 == 0:
                patches.append((i, f, x, y, w, h, patch))
    out += struct.pack('<2H', 32, 32) + struct.pack('<1024H', *cursor_image(sc.narc))
    # the glove: Platinum's spots across (less the DS's wider screen), and over each ball's top as its cell sits there
    for i, (cx, cy) in enumerate(CURSOR):
        x, y, w, h = ball_rect(sc, i, cam)
        out += struct.pack('<2h', cx - (DS_W - W) // 2, y - 10)
    if preview:
        for k in (0, 10, 20, 30, 40):
            save_png(os.path.join(preview, 'open%02d.png' % k), frames[k])
        for k in range(7):
            save_png(os.path.join(preview, 'cam%d.png' % k), frames[n_open + k])
        for i, f, x, y, w, h, patch in patches:
            save_png(os.path.join(preview, 'ball%d_%02d.png' % (i, f)), patch, w, h)
    log('briefcase: %d frames, %d bytes, %.0f s' % (len(frames), len(out), time.time() - t0))
    return b'STRT' + struct.pack('<I', len(out)) + bytes(out)


def main():
    root = sys.argv[1]
    out = sys.argv[sys.argv.index('--preview') + 1] if '--preview' in sys.argv else None
    section(root, out)


if __name__ == '__main__':
    main()
