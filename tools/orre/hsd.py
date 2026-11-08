#!/usr/bin/env python3
"""HAL's HSD model files (Pokemon Colosseum's .dat, map and character models): read the joint tree, materials,
textures and display lists into flat triangle meshes.

Struct layouts are the Colosseum decomp's (pkmn-colosseum include/hsd/hsd_jobj.h, hsd_dobj.h, hsd_mobj.h,
hsd_pobj.h, hsd_tobj.h); a map file's root is "scene_data" (floor.c floorInitScene: a NULL-ended list of model sets,
each a root joint first).
"""
import math
import struct

import numpy as np
import sys
sys.setrecursionlimit(10000)

import gxtex

JOBJ_SKELETON = 0x1
JOBJ_PTCL = 0x20
JOBJ_SPLINE = 0x4000
JOBJ_HIDDEN = 0x10
POBJ_TYPE_MASK = 0x3000
POBJ_SHAPEANIM = 0x1000
POBJ_ENVELOPE = 0x2000

GX_VA_PNMTXIDX, GX_VA_POS, GX_VA_NRM, GX_VA_CLR0, GX_VA_CLR1, GX_VA_TEX0, GX_VA_NBT, GX_VA_NULL = 0, 9, 10, 11, 12, 13, 25, 0xFF
GX_DIRECT, GX_INDEX8, GX_INDEX16 = 1, 2, 3


class HsdFile:
    def __init__(self, data, decode_textures=True):
        self.decode_textures = decode_textures
        self.d = data
        fs, ds, rc, rtc, xc = struct.unpack_from(">5I", data, 0)
        self.base = 0x20
        self.data_size = ds
        st = self.base + ds + rc * 4
        strings = st + (rtc + xc) * 8
        self.roots = {}
        for i in range(rtc):
            off, so = struct.unpack_from(">II", data, st + i * 8)
            name = data[strings + so:data.index(b"\0", strings + so)].decode("latin-1")
            self.roots[name] = off
        self.textures = {}   # (image offset, tlut offset) -> index into self.images
        self.images = []     # numpy RGBA arrays
        self.raw_images = [] # (gx format, w, h, texel bytes, tlut format, tlut bytes): the disc's own encoding

    # reads at data offsets (pointers in the file are offsets from the data start; 0 = NULL)
    def u8(self, o): return self.d[self.base + o]
    def u16(self, o): return struct.unpack_from(">H", self.d, self.base + o)[0]
    def u32(self, o): return struct.unpack_from(">I", self.d, self.base + o)[0]
    def f32(self, o): return struct.unpack_from(">f", self.d, self.base + o)[0]
    def raw(self, o, n): return self.d[self.base + o:self.base + o + n]

    def cstr(self, o):
        if not o:
            return ""
        s = self.base + o
        return self.d[s:self.d.index(b"\0", s)].decode("latin-1")

    def ptr_list(self, o):
        out = []
        while o and self.base + o + 4 <= len(self.d):
            p = self.u32(o)
            if not p:
                break
            out.append(p)
            o += 4
        return out

    # ---- textures
    def texture(self, tobj):
        img = self.u32(tobj + 0x4C)
        tlut = self.u32(tobj + 0x50)
        if not img:
            return -1
        key = (img, tlut)
        if key in self.textures:
            return self.textures[key]
        ptr, w, h, fmt = self.u32(img), self.u16(img + 4), self.u16(img + 6), self.u32(img + 8)
        pal = None
        tfmt, lutraw = 0, b""
        if tlut:
            lut, tfmt, n = self.u32(tlut), self.u32(tlut + 4), self.u16(tlut + 12)
            lutraw = self.raw(lut, n * 2)
            pal = gxtex.palette(lutraw, tfmt, n)
        size = gxtex.size(fmt, w, h)
        texels = self.raw(ptr, size)
        rgba = gxtex.decode(texels, fmt, w, h, pal) if self.decode_textures else None
        idx = len(self.images)
        self.images.append(rgba)
        self.raw_images.append((fmt, w, h, texels, tfmt, lutraw))
        self.textures[key] = idx
        return idx


def srt(j, f):
    """HSD_MtxSRT: T * Rz * Ry * Rx * S."""
    rx, ry, rz = f.f32(j + 0x14), f.f32(j + 0x18), f.f32(j + 0x1C)
    sx, sy, sz = f.f32(j + 0x20), f.f32(j + 0x24), f.f32(j + 0x28)
    tx, ty, tz = f.f32(j + 0x2C), f.f32(j + 0x30), f.f32(j + 0x34)
    cx, sxr = math.cos(rx), math.sin(rx)
    cy, syr = math.cos(ry), math.sin(ry)
    cz, szr = math.cos(rz), math.sin(rz)
    Rx = np.array([[1, 0, 0], [0, cx, -sxr], [0, sxr, cx]])
    Ry = np.array([[cy, 0, syr], [0, 1, 0], [-syr, 0, cy]])
    Rz = np.array([[cz, -szr, 0], [szr, cz, 0], [0, 0, 1]])
    m = np.eye(4)
    m[:3, :3] = Rz @ Ry @ Rx @ np.diag([sx, sy, sz])
    m[:3, 3] = [tx, ty, tz]
    return m


class Mesh:
    """One PObj's triangles: world positions (n,3), uvs (n,2) or None, colors (n,4) 0-255 or None, normals."""
    def __init__(self):
        self.pos = []
        self.uv = []
        self.col = []
        self.nrm = []
        self.tex = -1
        self.material = None   # (diffuse rgba, ambient rgba, alpha)
        self.rendermode = 0
        self.wrap = (1, 1)     # GX wrap modes s, t (0 clamp, 1 repeat, 2 mirror)
        self.uvscale = (1.0, 1.0)
        self.flags = 0
        self.joint = 0
        self.pe = None


def read_attr(f, a):
    attr, atype, cnt, ctype = f.u32(a), f.u32(a + 4), f.u32(a + 8), f.u32(a + 12)
    frac, stride, vtx = f.u8(a + 0x10), f.u16(a + 0x12), f.u32(a + 0x14)
    return attr, atype, cnt, ctype, frac, stride, vtx


COMP_SIZE = {0: 1, 1: 1, 2: 2, 3: 2, 4: 4}
COMP_FMT = {0: "B", 1: "b", 2: "H", 3: "h", 4: "f"}
CLR_SIZE = {0: 2, 1: 3, 2: 4, 3: 2, 4: 3, 5: 4}


def comps(attr, cnt):
    if attr == GX_VA_POS:
        return 2 if cnt == 0 else 3
    if attr == GX_VA_NRM:
        return 3 if cnt == 0 else 9
    if attr == GX_VA_NBT:
        return 9
    if GX_VA_TEX0 <= attr <= GX_VA_TEX0 + 7:
        return 1 if cnt == 0 else 2
    return 0


def read_color(b, ctype):
    if ctype == 0:
        v = struct.unpack(">H", b)[0]
        return ((v >> 11) * 255 // 31, ((v >> 5) & 63) * 255 // 63, (v & 31) * 255 // 31, 255)
    if ctype in (1, 2):
        return (b[0], b[1], b[2], 255)
    if ctype == 3:
        v = struct.unpack(">H", b)[0]
        return tuple(((v >> s) & 15) * 17 for s in (12, 8, 4, 0))
    if ctype == 4:
        v = (b[0] << 16) | (b[1] << 8) | b[2]
        return tuple(((v >> s) & 63) * 255 // 63 for s in (18, 12, 6, 0))
    return (b[0], b[1], b[2], b[3])


def read_value(f, attr, cnt, ctype, frac, raw_bytes):
    """A vector attribute's components from raw big-endian bytes."""
    n = comps(attr, cnt)
    fmt = ">" + COMP_FMT[ctype] * n
    vals = struct.unpack(fmt, raw_bytes[:COMP_SIZE[ctype] * n])
    if ctype != 4:
        sc = 1.0 / (1 << frac)
        vals = tuple(v * sc for v in vals)
    return vals


def decode_pobj(f, p, mats):
    """A PObj's display list -> list of (pnmtxidx, pos, nrm, clr, uv) per triangle corner."""
    attrs = []
    a = f.u32(p + 8)
    while a:
        at = read_attr(f, a)
        if at[0] == GX_VA_NULL:
            break
        attrs.append(at)
        a += 0x18
    n_disp = f.u16(p + 0x0E)
    disp = f.u32(p + 0x10)
    dl = f.raw(disp, n_disp * 32)
    i = 0
    corners = []
    while i < len(dl):
        op = dl[i]
        if op == 0:
            break
        prim = op & 0xF8
        count = struct.unpack_from(">H", dl, i + 1)[0]
        i += 3
        verts = []
        for _ in range(count):
            v = {"mtx": 0, "pos": (0, 0, 0), "nrm": None, "clr": None, "uv": None}
            for attr, atype, cnt, ctype, frac, stride, vtx in attrs:
                if attr == GX_VA_PNMTXIDX:
                    v["mtx"] = dl[i]
                    i += 1
                    continue
                if GX_VA_PNMTXIDX < attr < GX_VA_POS:   # texture matrix indices
                    i += 1
                    continue
                if atype == GX_DIRECT:
                    if attr in (GX_VA_CLR0, GX_VA_CLR1):
                        sz = CLR_SIZE[ctype]
                        val = read_color(dl[i:i + sz], ctype)
                    else:
                        sz = COMP_SIZE[ctype] * comps(attr, cnt)
                        val = read_value(f, attr, cnt, ctype, frac, dl[i:i + sz])
                    i += sz
                else:
                    if atype == GX_INDEX8:
                        idx = dl[i]
                        i += 1
                    else:
                        idx = struct.unpack_from(">H", dl, i)[0]
                        i += 2
                    if attr in (GX_VA_CLR0, GX_VA_CLR1):
                        val = read_color(f.raw(vtx + idx * stride, CLR_SIZE[ctype]), ctype)
                    else:
                        sz = COMP_SIZE[ctype] * comps(attr, cnt)
                        val = read_value(f, attr, cnt, ctype, frac, f.raw(vtx + idx * stride, sz))
                if attr == GX_VA_POS:
                    v["pos"] = val if len(val) == 3 else (val[0], val[1], 0.0)
                elif attr in (GX_VA_NRM, GX_VA_NBT):
                    v["nrm"] = val[:3]
                elif attr == GX_VA_CLR0:
                    v["clr"] = val
                elif attr == GX_VA_TEX0:
                    v["uv"] = val if len(val) == 2 else (val[0], 0.0)
            verts.append(v)
        if prim == 0x90:      # triangles
            for k in range(0, count - 2, 3):
                corners += [verts[k], verts[k + 1], verts[k + 2]]
        elif prim == 0x98:    # strip
            for k in range(count - 2):
                if k & 1:
                    corners += [verts[k + 1], verts[k], verts[k + 2]]
                else:
                    corners += [verts[k], verts[k + 1], verts[k + 2]]
        elif prim == 0xA0:    # fan
            for k in range(1, count - 1):
                corners += [verts[0], verts[k], verts[k + 1]]
        elif prim == 0x80:    # quads
            for k in range(0, count - 3, 4):
                q = verts[k:k + 4]
                corners += [q[0], q[1], q[2], q[0], q[2], q[3]]
        # lines / points (0xA8 / 0xB0 / 0xB8) carry no surface
    return corners


JOBJ_INSTANCE = 0x1000


def walk(f, j, parent, out, joints, path=(), single=False):
    """Every joint: world matrices into joints[offset]; meshes into out. An instance joint (JOBJ_INSTANCE) draws
    the joint its child points to again under its own transform; `path` stops a loop back up the tree."""
    while j:
        if j in path:
            return
        flags = f.u32(j + 4)
        m = parent @ srt(j, f)
        joints.setdefault(j, m)
        u = f.u32(j + 0x10)
        if u and not flags & (JOBJ_PTCL | JOBJ_SPLINE):
            d = u
            while d:
                mobj, pobj = f.u32(d + 8), f.u32(d + 12)
                mat = material(f, mobj)
                p = pobj
                while p:
                    out.append((j, d, p, mat, flags, m))
                    p = f.u32(p + 4)
                d = f.u32(d + 4)
        child = f.u32(j + 8)
        if child:
            if flags & JOBJ_INSTANCE:
                inst_out = []
                walk(f, child, m, inst_out, {}, path + (j,), single=True)   # that one joint, not its siblings
                out += [(jj, d, p, mat, fl, m2) for jj, d, p, mat, fl, m2 in inst_out]
            else:
                walk(f, child, m, out, joints, path + (j,))
        if single:
            return
        j = f.u32(j + 0x0C)


def material(f, mobj):
    if not mobj:
        return None
    rm = f.u32(mobj + 4)
    tobj = f.u32(mobj + 8)
    mat = f.u32(mobj + 12)
    pe = f.u32(mobj + 0x14)
    out = {"rendermode": rm, "tex": -1, "wrap": (1, 1), "repeat": (1, 1), "uvscale": (1.0, 1.0), "uvtrans": (0.0, 0.0),
           "diffuse": (255, 255, 255, 255), "ambient": (128, 128, 128, 255), "alpha": 1.0, "pe": None, "texflags": 0}
    if mat:
        out["ambient"] = tuple(f.raw(mat, 4))
        out["diffuse"] = tuple(f.raw(mat + 4, 4))
        out["alpha"] = f.f32(mat + 12)
    if pe:
        out["pe"] = tuple(f.raw(pe, 12))
    # the first texture that is a colour map (HSD TOBJ_COLORMAP bits 0xF in flags at 0x40); others are lightmaps
    t = tobj
    while t:
        tf = f.u32(t + 0x40)
        if (tf & 0xF) and out["tex"] < 0:
            out["tex"] = f.texture(t)
            out["wrap"] = (f.u32(t + 0x34), f.u32(t + 0x38))
            out["repeat"] = (f.u8(t + 0x3C), f.u8(t + 0x3D))
            out["uvscale"] = (f.f32(t + 0x1C), f.f32(t + 0x20))
            out["uvtrans"] = (f.f32(t + 0x28), f.f32(t + 0x2C))
            out["texflags"] = tf
            out["texid"] = f.u32(t + 8)   # (its GXTexMapID: which TexAnim of a MatAnim moves it)
        t = f.u32(t + 4)
    if out["tex"] < 0 and tobj:
        out["tex"] = f.texture(tobj)
        out["wrap"] = (f.u32(tobj + 0x34), f.u32(tobj + 0x38))
        out["repeat"] = (f.u8(tobj + 0x3C), f.u8(tobj + 0x3D))
        out["uvscale"] = (f.f32(tobj + 0x1C), f.f32(tobj + 0x20))
        out["uvtrans"] = (f.f32(tobj + 0x28), f.f32(tobj + 0x2C))
        out["texid"] = f.u32(tobj + 8)
    return out


def envelope_mtx(f, p, joints, inv):
    """Envelope PObj: per PNMTXIDX/3 the weighted joint matrices (bind pose: world @ inverse bind)."""
    lst = f.ptr_list(f.u32(p + 0x14))
    out = []
    for e in lst:
        ws = []
        o = e
        while True:
            jt = f.u32(o)
            if not jt:
                break
            ws.append((jt, f.f32(o + 4)))
            o += 8
        if len(ws) == 1 and abs(ws[0][1] - 1.0) < 1e-6:
            # one joint at full weight: the vertex is in that joint's own space
            out.append(joints.get(ws[0][0], np.eye(4)))
            continue
        m = np.zeros((4, 4))
        for jt, w in ws:   # blended: the vertex is in the bind pose (world @ inverse bind is the identity there)
            jm = joints.get(jt, np.eye(4))
            ib = inv.get(jt)
            m += w * (jm @ ib if ib is not None else np.eye(4))
        out.append(m)
    return out


def load_model(f, root_joint):
    """All meshes under a root joint, in world space (the model's bind pose)."""
    items = []
    joints = {}
    walk(f, root_joint, np.eye(4), items, joints)
    inv = {}
    for j in joints:
        mp = f.u32(j + 0x38)
        if mp:
            m = np.eye(4)
            m[:3, :] = np.array([f.f32(mp + 4 * k) for k in range(12)]).reshape(3, 4)
            inv[j] = m
    meshes = []
    for j, d, p, mat, jflags, jm in items:
        corners = decode_pobj(f, p, None)
        if not corners:
            continue
        pflags = f.u16(p + 0x0C)
        kind = pflags & POBJ_TYPE_MASK
        if kind == POBJ_ENVELOPE:
            mtxs = envelope_mtx(f, p, joints, inv)

            def xf(v):
                k = v["mtx"] // 3
                m = mtxs[k] if k < len(mtxs) else jm
                return m
        else:
            skin = f.u32(p + 0x14) if kind == 0 else 0
            base = joints.get(skin, jm) if skin else jm

            def xf(v, base=base):
                return base
        me = Mesh()
        me.joint = j
        me.dobj = d
        me.jm = jm
        me.flags = pflags
        me.jflags = jflags
        if mat:
            me.tex = mat["tex"]
            me.material = mat
        pos, uv, col, nrm = [], [], [], []
        for v in corners:
            m = xf(v)
            x, y, z = v["pos"]
            pos.append(m[:3, :3] @ (x, y, z) + m[:3, 3])
            uv.append(v["uv"] if v["uv"] is not None else (0.0, 0.0))
            col.append(v["clr"] if v["clr"] is not None else (255, 255, 255, 255))
            n = v["nrm"]
            nrm.append(m[:3, :3] @ n if n is not None else (0.0, 1.0, 0.0))
        me.pos = np.array(pos, dtype=np.float32)
        me.uv = np.array(uv, dtype=np.float32)
        me.col = np.array(col, dtype=np.uint8)
        me.nrm = np.array(nrm, dtype=np.float32)
        me.has_clr = any(v["clr"] is not None for v in corners)
        me.has_uv = any(v["uv"] is not None for v in corners)
        meshes.append(me)
    return meshes


def load_scene(data):
    """A map model file: every model set's meshes."""
    f = HsdFile(data)
    sd = f.roots.get("scene_data")
    meshes = []
    if sd is not None:
        for ms in f.ptr_list(f.u32(sd)):
            root = f.u32(ms)
            if root:
                meshes += load_model(f, root)
    else:
        for name, off in f.roots.items():
            if name.endswith("_joint"):
                meshes += load_model(f, off)
    return f, meshes


def model_sets(f):
    """A map file's model sets, each a list of meshes."""
    sd = f.roots.get("scene_data")
    if sd is None:
        return [load_model(f, off) for n, off in f.roots.items() if n.endswith("_joint")]
    return [load_model(f, f.u32(ms)) if f.u32(ms) else [] for ms in f.ptr_list(f.u32(sd))]


def is_sky(meshes):
    """The sky: a model set reaching thousands of units out, or a dome (a few meshes rising high over a wide area)."""
    if not meshes:
        return True
    p = np.concatenate([m.pos for m in meshes])
    if float(np.abs(p).max()) > 5000:
        return True
    w, h = float(np.ptp(p[:, 0])), float(np.ptp(p[:, 1]))
    return len(meshes) <= 4 and w > 1000 and h > 0.4 * w
