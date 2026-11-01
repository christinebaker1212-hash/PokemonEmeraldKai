#!/usr/bin/env python3
"""Nintendo DS 3D files (NitroSystem G3D): BMD0 models (MDL0 + optional TEX0) and BTX0 texture sets.

Used by the SINNOH converter (docs/SINNOH_HANDOFF.md) on Platinum's map blocks, buildings and their textures:
  Model.draw()  runs a model's render commands and display lists and returns its triangles (positions after the
                node matrices and the position scale, texture coordinates in texels, vertex colors and normals),
                grouped by material;
  Tex0.decode() turns a texture + palette into RGBA (every DS format, 4x4 compressed included).
"""
import math
import struct

FX12 = 4096.0


def u8(d, o):
    return d[o]


def u16(d, o):
    return struct.unpack_from('<H', d, o)[0]


def s16(d, o):
    return struct.unpack_from('<h', d, o)[0]


def u32(d, o):
    return struct.unpack_from('<I', d, o)[0]


def s32(d, o):
    return struct.unpack_from('<i', d, o)[0]


def sign_extend(v, bits):
    return v - (1 << bits) if v & (1 << (bits - 1)) else v


def rgb555(c):
    return (c & 31, (c >> 5) & 31, (c >> 10) & 31)


def read_dict(d, o):
    """A G3D dictionary: [(name, entry bytes)]."""
    count = d[o + 1]
    unk_size = u16(d, o + 6)
    p = o + unk_size   # (the size counts from the dictionary's start)
    entry_size = u16(d, p)
    p += 4
    entries = [d[p + i * entry_size:p + (i + 1) * entry_size] for i in range(count)]
    p += count * entry_size
    names = [d[p + 16 * i:p + 16 * i + 16].split(b'\0')[0].decode('latin-1') for i in range(count)]
    return list(zip(names, entries))


# ------------------------------------------------------------------------------------------------ matrices (4x4)
def mat_identity():
    return [[1.0, 0, 0, 0], [0, 1.0, 0, 0], [0, 0, 1.0, 0], [0, 0, 0, 1.0]]


def mat_mul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4)] for i in range(4)]


def mat_apply(m, v):
    x, y, z = v
    return (m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3],
            m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3],
            m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3])


def mat_apply3(m, v):
    x, y, z = v
    return (m[0][0] * x + m[0][1] * y + m[0][2] * z,
            m[1][0] * x + m[1][1] * y + m[1][2] * z,
            m[2][0] * x + m[2][1] * y + m[2][2] * z)


def mat_trs(t, r, s):
    """Translation * rotation (3x3, rows) * scale."""
    m = mat_identity()
    for i in range(3):
        for j in range(3):
            m[i][j] = r[i][j] * s[j]
        m[i][3] = t[i]
    return m


def gx_matrix(vals, rows, cols):
    """A GX matrix command's fx32 values (column-major in GX's row-vector maths) as a 4x4 acting on column vectors."""
    m = mat_identity()
    k = 0
    for r in range(rows):
        for c in range(cols):
            # GX stores m[r][c] for row vectors: v' = v * M, so our column form is the transpose
            m[c][r] = vals[k] / FX12
            k += 1
    if rows == 4 and cols == 3:
        pass
    return m


def pivot_matrix(select, neg, a, b):
    """NNS's compressed rotation: one row / column is +-1, the rest [a b; c d] (pokeplatinum uses it rarely)."""
    o = -1.0 if neg & 1 else 1.0
    c = -b if neg & 2 else b
    d = -a if neg & 4 else a
    m = [[0.0] * 3 for _ in range(3)]
    row, col = select // 3, select % 3
    m[row][col] = o
    rows = [i for i in range(3) if i != row]
    cols = [j for j in range(3) if j != col]
    m[rows[0]][cols[0]] = a
    m[rows[0]][cols[1]] = b
    m[rows[1]][cols[0]] = c
    m[rows[1]][cols[1]] = d
    return m


# ------------------------------------------------------------------------------------------------ textures
class Texture:
    def __init__(self, name, param, data_off):
        self.name = name
        self.param = param
        self.offset = (param & 0xFFFF) << 3
        self.width = 8 << ((param >> 20) & 7)
        self.height = 8 << ((param >> 23) & 7)
        self.format = (param >> 26) & 7
        self.color0_transparent = bool((param >> 29) & 1)


class Tex0:
    def __init__(self, d, o):
        assert d[o:o + 4] == b'TEX0', d[o:o + 4]
        self.d = d
        self.o = o
        self.tex_data = o + u32(d, o + 0x14)
        self.cmp_data = o + u32(d, o + 0x24)
        self.cmp_idx = o + u32(d, o + 0x28)
        self.pal_data = o + u32(d, o + 0x38)
        tex_dict = o + u16(d, o + 0x0E)
        pal_dict = o + u32(d, o + 0x34)
        self.textures = {}
        self.order = []
        for name, e in read_dict(d, tex_dict):
            self.textures[name] = Texture(name, u32(e, 0), 0)
            self.order.append(name)
        self.palettes = {}
        for name, e in read_dict(d, pal_dict):
            self.palettes[name] = u16(e, 0) << 3

    def palette(self, name, count):
        p = self.pal_data + self.palettes[name]
        return [rgb555(u16(self.d, p + 2 * i)) for i in range(count)]

    def decode(self, tex_name, pal_name, frame=None):
        """RGBA rows (0-255) for a texture with a palette (None for DIRECT textures). FRAME (another Tex0, a texture
        name in it): its texels drawn with this texture's palette (Platinum's animated map textures: fldtanime)."""
        t = self.textures[tex_name]
        pd = self.d   # (the palette's bytes)
        d, tex_data, cmp_data, cmp_idx, off = self.d, self.tex_data, self.cmp_data, self.cmp_idx, t.offset
        if frame is not None:
            src, name = frame
            d, tex_data, cmp_data, cmp_idx, off = src.d, src.tex_data, src.cmp_data, src.cmp_idx, src.textures[name].offset
        w, h, f = t.width, t.height, t.format
        out = [(0, 0, 0, 0)] * (w * h)

        def c8(c, a=255):
            r, g, b = c
            return (r * 255 // 31, g * 255 // 31, b * 255 // 31, a)

        if f == 7:   # direct color
            base = tex_data + off
            for i in range(w * h):
                v = u16(d, base + 2 * i)
                out[i] = c8(rgb555(v), 255 if v & 0x8000 else 0)
            return w, h, out
        palbase = self.pal_data + self.palettes[pal_name]

        def pal(i):
            return rgb555(u16(pd, palbase + 2 * i))

        if f == 5:   # 4x4 compressed
            texel = cmp_data + off
            idx = cmp_idx + off // 2
            bw = w // 4
            for by in range(h // 4):
                for bx in range(bw):
                    blk = by * bw + bx
                    bits = u32(d, texel + 4 * blk)
                    pi = u16(d, idx + 2 * blk)
                    mode = pi >> 14
                    pa = palbase + (pi & 0x3FFF) * 4
                    cols = [rgb555(u16(pd, pa + 2 * k)) for k in range(4)]
                    c0, c1 = cols[0], cols[1]
                    if mode == 0:
                        cl = [c8(cols[0]), c8(cols[1]), c8(cols[2]), (0, 0, 0, 0)]
                    elif mode == 1:
                        mid = tuple((a + b) // 2 for a, b in zip(c0, c1))
                        cl = [c8(c0), c8(c1), c8(mid), (0, 0, 0, 0)]
                    elif mode == 2:
                        cl = [c8(c) for c in cols]
                    else:
                        m1 = tuple((5 * a + 3 * b) // 8 for a, b in zip(c0, c1))
                        m2 = tuple((3 * a + 5 * b) // 8 for a, b in zip(c0, c1))
                        cl = [c8(c0), c8(c1), c8(m1), c8(m2)]
                    for yy in range(4):
                        for xx in range(4):
                            k = (bits >> (2 * (yy * 4 + xx))) & 3
                            out[(by * 4 + yy) * w + bx * 4 + xx] = cl[k]
            return w, h, out
        base = tex_data + off
        if f == 2:   # 4 colors
            for i in range(w * h):
                k = (d[base + i // 4] >> (2 * (i % 4))) & 3
                out[i] = (0, 0, 0, 0) if (k == 0 and t.color0_transparent) else c8(pal(k))
        elif f == 3:   # 16 colors
            for i in range(w * h):
                k = (d[base + i // 2] >> (4 * (i % 2))) & 15
                out[i] = (0, 0, 0, 0) if (k == 0 and t.color0_transparent) else c8(pal(k))
        elif f == 4:   # 256 colors
            for i in range(w * h):
                k = d[base + i]
                out[i] = (0, 0, 0, 0) if (k == 0 and t.color0_transparent) else c8(pal(k))
        elif f == 1:   # A3I5
            for i in range(w * h):
                v = d[base + i]
                a = (v >> 5) & 7
                out[i] = c8(pal(v & 31), (a * 4 + a // 2) * 255 // 31)
        elif f == 6:   # A5I3
            for i in range(w * h):
                v = d[base + i]
                out[i] = c8(pal(v & 7), (v >> 3) * 255 // 31)
        return w, h, out


def btx0(d):
    assert d[:4] in (b'BTX0', b'BMD0'), d[:4]
    nsec = u16(d, 0x0E)
    for i in range(nsec):
        so = u32(d, 0x10 + 4 * i)
        if d[so:so + 4] == b'TEX0':
            return Tex0(d, so)
    return None


# ------------------------------------------------------------------------------------------------ models
class Material:
    def __init__(self, name, d, o):
        self.name = name
        self.dif_amb = u32(d, o + 4)
        self.spe_emi = u32(d, o + 8)
        self.poly_attr = u32(d, o + 0x0C)
        self.poly_attr_mask = u32(d, o + 0x10)
        self.tex_param = u32(d, o + 0x14)
        self.flags = u16(d, o + 0x1E)
        self.width = u16(d, o + 0x20)
        self.height = u16(d, o + 0x22)
        p = o + 0x24
        if not (self.flags & 0x0010):   # ORIGWH_SAME not set: magnification
            p += 8
        self.tex_scale = (1.0, 1.0)
        self.tex_rot = (0.0, 1.0)
        self.tex_trans = (0.0, 0.0)
        if self.flags & 0x0001:   # TEXMTX_USE
            if not (self.flags & 0x0002):
                self.tex_scale = (s32(d, p) / FX12, s32(d, p + 4) / FX12)
                p += 8
            if not (self.flags & 0x0004):
                self.tex_rot = (s16(d, p) / FX12, s16(d, p + 2) / FX12)
                p += 4
            if not (self.flags & 0x0008):
                self.tex_trans = (s32(d, p) / FX12, s32(d, p + 4) / FX12)
                p += 8
        self.texture = None   # names, from the pairing dictionaries
        self.palette = None

    @property
    def alpha(self):
        return (self.poly_attr >> 16) & 31

    @property
    def lights(self):
        return self.poly_attr & 15

    @property
    def mode(self):
        return (self.poly_attr >> 4) & 3

    @property
    def cull(self):
        """'none', 'back' (front faces drawn) or 'front'."""
        front, back = (self.poly_attr >> 7) & 1, (self.poly_attr >> 6) & 1
        if front and back:
            return 'none'
        return 'back' if front else ('front' if back else 'all')

    @property
    def repeat(self):
        p = self.tex_param
        return ((p >> 16) & 1, (p >> 17) & 1, (p >> 18) & 1, (p >> 19) & 1)   # repeat S, T, flip S, T


class Node:
    def __init__(self, name, d, o):
        self.name = name
        flags = u16(d, o)
        m0 = s16(d, o + 2) / FX12
        p = o + 4
        t = (0.0, 0.0, 0.0)
        r = [[1.0, 0, 0], [0, 1.0, 0], [0, 0, 1.0]]
        s = (1.0, 1.0, 1.0)
        if not (flags & 1):
            t = (s32(d, p) / FX12, s32(d, p + 4) / FX12, s32(d, p + 8) / FX12)
            p += 12
        if not (flags & 2):
            if flags & 8:
                a, b = s16(d, p) / FX12, s16(d, p + 2) / FX12
                p += 4
                r = pivot_matrix((flags >> 4) & 15, (flags >> 8) & 15, a, b)
            else:
                v = [m0] + [s16(d, p + 2 * i) / FX12 for i in range(8)]
                p += 16
                r = [[v[0], v[3], v[6]], [v[1], v[4], v[7]], [v[2], v[5], v[8]]]
        if not (flags & 4):
            s = (s32(d, p) / FX12, s32(d, p + 4) / FX12, s32(d, p + 8) / FX12)
        self.t, self.r, self.s = t, r, s   # (an animation's BASE_* parts)
        self.matrix = mat_trs(t, r, s)


class Shape:
    def __init__(self, name, d, o):
        self.name = name
        self.dl = d[o + u32(d, o + 8):o + u32(d, o + 8) + u32(d, o + 12)]


GX_PARAMS = {0x10: 1, 0x11: 0, 0x12: 1, 0x13: 1, 0x14: 1, 0x15: 0, 0x16: 16, 0x17: 12, 0x18: 16, 0x19: 12, 0x1A: 9,
             0x1B: 3, 0x1C: 3, 0x20: 1, 0x21: 1, 0x22: 1, 0x23: 2, 0x24: 1, 0x25: 1, 0x26: 1, 0x27: 1, 0x28: 1,
             0x29: 1, 0x2A: 1, 0x2B: 1, 0x30: 1, 0x31: 1, 0x32: 1, 0x33: 1, 0x34: 32, 0x40: 1, 0x41: 0, 0x50: 1,
             0x60: 1, 0x70: 3, 0x71: 2, 0x72: 1, 0x00: 0}


class Model:
    def __init__(self, name, d, o):
        self.name = name
        self.d = d
        self.o = o
        sbc = o + u32(d, o + 4)
        mats = o + u32(d, o + 8)
        shps = o + u32(d, o + 0x0C)
        self.num_nodes = d[o + 0x17]
        self.up_scale = s32(d, o + 0x1C) / FX12
        self.down_scale = s32(d, o + 0x20) / FX12
        self.nodes = [Node(n, d, o + 0x40 + u32(e, 0)) for n, e in read_dict(d, o + 0x40)]
        # materials: [u16 tex pairing dict off, u16 pal pairing dict off], then the material dict
        tex_pair = mats + u16(d, mats)
        pal_pair = mats + u16(d, mats + 2)
        self.materials = [Material(n, d, mats + u32(e, 0)) for n, e in read_dict(d, mats + 4)]
        for name, e in read_dict(d, tex_pair):
            lst, num = u16(e, 0), e[2]
            for k in range(num):
                self.materials[d[mats + lst + k]].texture = name
        for name, e in read_dict(d, pal_pair):
            lst, num = u16(e, 0), e[2]
            for k in range(num):
                self.materials[d[mats + lst + k]].palette = name
        self.shapes = [Shape(n, d, shps + u32(e, 0)) for n, e in read_dict(d, shps)]
        self.sbc = d[sbc:mats]

    def draw(self, node_mats=None):
        """{material index: [triangle]}; a triangle = 3 vertices (pos, uv in texels, color 0-31 rgb, normal).
        node_mats: {node index: 4x4} in place of the nodes' own (an animation's frame, Bca0.node_matrices)"""
        node_mats = node_mats or {}
        stack = [mat_identity() for _ in range(32)]
        cur = mat_identity()
        mat = 0
        out = {}
        s = self.sbc
        p = 0
        while p < len(s):
            op = s[p]
            base = op & 0x1F
            var = op >> 5
            if base == 0x00:
                p += 1
            elif base == 0x01:
                break
            elif base == 0x02:
                p += 3
            elif base == 0x03:
                cur = [row[:] for row in stack[s[p + 1]]]
                p += 2
            elif base == 0x04:
                mat = s[p + 1]
                p += 2
            elif base == 0x05:
                tris = self.run_dl(self.shapes[s[p + 1]].dl, cur, stack)
                out.setdefault(mat, []).extend(tris)
                p += 2
            elif base == 0x06:
                obj = s[p + 1]
                n = 4
                store = restore = None
                if var == 1:
                    store = s[p + 4]
                    n = 5
                elif var == 2:
                    restore = s[p + 4]
                    n = 5
                elif var == 3:
                    store, restore = s[p + 4], s[p + 5]
                    n = 6
                if restore is not None:
                    cur = [row[:] for row in stack[restore]]
                cur = mat_mul(cur, node_mats.get(obj, self.nodes[obj].matrix))
                if store is not None:
                    stack[store] = [row[:] for row in cur]
                p += n
            elif base in (0x07, 0x08):
                p += 2 + (1 if var in (1, 2) else 2 if var == 3 else 0)
            elif base == 0x09:
                p += 3 + 3 * s[p + 2]
            elif base == 0x0A:
                p += 9
            elif base == 0x0B:
                k = self.up_scale if var == 0 else self.down_scale
                cur = mat_mul(cur, [[k, 0, 0, 0], [0, k, 0, 0], [0, 0, k, 0], [0, 0, 0, 1.0]])
                p += 1
            elif base in (0x0C, 0x0D):
                p += 3
            else:
                raise ValueError('render command %02x at %d in %s' % (op, p, self.name))
        return out

    def run_dl(self, dl, cur, stack):
        tris = []
        verts = []
        prim = 0
        color = (31, 31, 31)
        normal = (0.0, 1.0, 0.0)
        uv = (0.0, 0.0)
        last = (0.0, 0.0, 0.0)
        m = cur
        p = 0
        n = len(dl)

        def emit(v):
            nonlocal verts
            verts.append((mat_apply(m, v), uv, color, mat_apply3(m, normal)))
            if prim == 0 and len(verts) == 3:
                tris.append(tuple(verts))
                verts = []
            elif prim == 1 and len(verts) == 4:
                tris.append((verts[0], verts[1], verts[2]))
                tris.append((verts[0], verts[2], verts[3]))
                verts = []
            elif prim == 2 and len(verts) >= 3:
                k = len(verts)
                tri = (verts[k - 3], verts[k - 2], verts[k - 1]) if k % 2 == 1 else (verts[k - 2], verts[k - 3], verts[k - 1])
                tris.append(tri)
            elif prim == 3 and len(verts) >= 4 and len(verts) % 2 == 0:
                k = len(verts)
                a, b, c, dd = verts[k - 4], verts[k - 3], verts[k - 1], verts[k - 2]
                tris.append((a, b, c))
                tris.append((a, c, dd))

        while p + 4 <= n:
            cmds = dl[p:p + 4]
            p += 4
            for c in cmds:
                np_ = GX_PARAMS.get(c)
                if np_ is None:
                    raise ValueError('GX command %02x' % c)
                args = [u32(dl, p + 4 * i) for i in range(np_)]
                p += 4 * np_
                if c == 0x14:   # MTX_RESTORE
                    m = [row[:] for row in stack[args[0] & 31]]
                elif c == 0x20:
                    color = rgb555(args[0])
                elif c == 0x21:
                    a = args[0]
                    normal = (sign_extend(a & 0x3FF, 10) / 512.0, sign_extend((a >> 10) & 0x3FF, 10) / 512.0,
                              sign_extend((a >> 20) & 0x3FF, 10) / 512.0)
                elif c == 0x22:
                    a = args[0]
                    uv = (sign_extend(a & 0xFFFF, 16) / 16.0, sign_extend(a >> 16, 16) / 16.0)
                elif c == 0x23:
                    a, b = args
                    last = (sign_extend(a & 0xFFFF, 16) / FX12, sign_extend(a >> 16, 16) / FX12,
                            sign_extend(b & 0xFFFF, 16) / FX12)
                    emit(last)
                elif c == 0x24:
                    a = args[0]
                    last = (sign_extend(a & 0x3FF, 10) / 64.0, sign_extend((a >> 10) & 0x3FF, 10) / 64.0,
                            sign_extend((a >> 20) & 0x3FF, 10) / 64.0)
                    emit(last)
                elif c == 0x25:
                    a = args[0]
                    last = (sign_extend(a & 0xFFFF, 16) / FX12, sign_extend(a >> 16, 16) / FX12, last[2])
                    emit(last)
                elif c == 0x26:
                    a = args[0]
                    last = (sign_extend(a & 0xFFFF, 16) / FX12, last[1], sign_extend(a >> 16, 16) / FX12)
                    emit(last)
                elif c == 0x27:
                    a = args[0]
                    last = (last[0], sign_extend(a & 0xFFFF, 16) / FX12, sign_extend(a >> 16, 16) / FX12)
                    emit(last)
                elif c == 0x28:
                    a = args[0]
                    last = (last[0] + sign_extend(a & 0x3FF, 10) / FX12, last[1] + sign_extend((a >> 10) & 0x3FF, 10) / FX12,
                            last[2] + sign_extend((a >> 20) & 0x3FF, 10) / FX12)
                    emit(last)
                elif c == 0x40:
                    prim = args[0] & 3
                    verts = []
                elif c == 0x41:
                    verts = []
        return tris


class Bmd0:
    def __init__(self, d):
        assert d[:4] == b'BMD0', d[:4]
        self.d = d
        self.models = []
        self.tex = None
        for i in range(u16(d, 0x0E)):
            so = u32(d, 0x10 + 4 * i)
            if d[so:so + 4] == b'MDL0':
                for name, e in read_dict(d, so + 8):
                    self.models.append(Model(name, d, so + u32(e, 0)))
            elif d[so:so + 4] == b'TEX0':
                self.tex = Tex0(d, so)


# ------------------------------------------------------------------------------------------------ joint animations
class JointAnim:
    """One NSBCA joint animation (NNSG3dResJntAnm): per node translation, rotation, scale tracks by frame."""

    def __init__(self, name, d, o):
        assert d[o:o + 4] == b'J\0AC', d[o:o + 4]
        self.name = name
        self.d, self.o = d, o
        self.frames = u16(d, o + 4)
        n = u16(d, o + 6)
        self.rot3 = o + u32(d, o + 12)
        self.rot5 = o + u32(d, o + 16)
        self.tags = [o + u16(d, o + 20 + 2 * i) for i in range(n)]

    def _track(self, info, off, frame, fx16):
        d, o = self.d, self.o
        step = {0: 1, 1: 2, 2: 4}.get(info >> 30, 1)
        last = (info >> 16) & 0x1FFF
        size = 2 if fx16 else 4
        get = (lambda i: s16(d, o + off + 2 * i) / FX12) if fx16 else (lambda i: s32(d, o + off + 4 * i) / FX12)
        return self._sample(get, step, last, frame)

    @staticmethod
    def _sample(get, step, last, frame):
        if step == 1 or frame >= last:
            i = frame if step == 1 else last // step + (frame - last)
            return get(int(i))
        i, f = divmod(frame, step)
        a, b = get(int(i)), get(int(i) + 1)
        if isinstance(a, list):
            return [[a[r][c] + (b[r][c] - a[r][c]) * f / step for c in range(3)] for r in range(3)]
        return a + (b - a) * f / step

    def _rot(self, idx):
        d = self.d
        if idx & 0x8000:   # a pivot-compressed rotation (Rot3)
            p = self.rot3 + 6 * (idx & 0x7FFF)
            info = u16(d, p)
            m = pivot_matrix(info & 15, (info >> 4) & 15, s16(d, p + 2) / FX12, s16(d, p + 4) / FX12)
            return [[m[0][i], m[1][i], m[2][i]] for i in range(3)]   # (stored for row vectors, as below)
        p = self.rot5 + 10 * idx   # five u16: the first two rows, 13 bits each, and _12 from the low 3 bits
        v = [u16(d, p + 2 * i) for i in range(5)]
        a = [sign_extend(x >> 3, 13) / FX12 for x in v]
        # _12: the low 15 bits of a 1.15 number; the row's length (1) tells its sign bit
        t = (((v[0] & 7) << 12) | ((v[1] & 7) << 9) | ((v[2] & 7) << 6) | ((v[3] & 7) << 3) | (v[4] & 7)) / 32768.0
        rest = a[3] * a[3] + a[4] * a[4]
        m12 = min((t, t - 1.0), key=lambda m: abs(rest + m * m - 1.0))
        r0 = [a[0], a[1], a[2]]
        r1 = [a[3], a[4], m12]
        r2 = [r0[1] * r1[2] - r0[2] * r1[1], r0[2] * r1[0] - r0[0] * r1[2], r0[0] * r1[1] - r0[1] * r1[0]]
        return [[r0[i], r1[i], r2[i]] for i in range(3)]   # (stored for row vectors, as Node's full matrices)

    def node_trs(self, tag, node, frame):
        """(node index, (t, r, s)) of a tag at FRAME; node: the model's Node for BASE_* parts"""
        d = self.d
        flag = u32(d, tag)
        p = tag + 4
        nid = flag >> 24
        t, r, sc = list(node.t), node.r, list(node.s)
        if flag & 1:   # identity
            return nid, ((0.0, 0.0, 0.0), [[1.0, 0, 0], [0, 1.0, 0], [0, 0, 1.0]], (1.0, 1.0, 1.0))
        if flag & 2:
            t = [0.0, 0.0, 0.0]
        elif not (flag & 4):
            for k, const in enumerate((8, 16, 32)):
                if flag & const:
                    t[k] = s32(d, p) / FX12
                    p += 4
                else:
                    info, off = u32(d, p), u32(d, p + 4)
                    p += 8
                    t[k] = self._track(info, off, frame, bool(info & 0x20000000))
        if flag & 0x40:
            r = [[1.0, 0, 0], [0, 1.0, 0], [0, 0, 1.0]]
        elif not (flag & 0x80):
            if flag & 0x100:
                r = self._rot(u16(d, p))
                p += 4
            else:
                info, off = u32(d, p), u32(d, p + 4)
                p += 8
                step = {0: 1, 1: 2, 2: 4}.get(info >> 30, 1)
                last = (info >> 16) & 0x1FFF
                r = self._sample(lambda i: self._rot(u16(d, self.o + off + 2 * i)), step, last, frame)
        if flag & 0x200:
            sc = [1.0, 1.0, 1.0]
        elif not (flag & 0x400):
            for k, const in enumerate((0x800, 0x1000, 0x2000)):
                if flag & const:
                    sc[k] = s32(d, p) / FX12
                    p += 8
                else:
                    info, off = u32(d, p), u32(d, p + 4)
                    p += 8
                    fx16 = bool(info & 0x20000000)
                    step = {0: 1, 1: 2, 2: 4}.get(info >> 30, 1)
                    last = (info >> 16) & 0x1FFF
                    # (scale tracks hold the scale and its inverse side by side)
                    get = (lambda i, off=off, fx16=fx16: (s16(d, self.o + off + 4 * i) if fx16 else s32(d, self.o + off + 8 * i)) / FX12)
                    sc[k] = self._sample(get, step, last, frame)
        return nid, (tuple(t), r, tuple(sc))

    def node_matrices(self, model, frame):
        """{node index: 4x4} at FRAME for Model.draw"""
        out = {}
        for tag in self.tags:
            nid = u32(self.d, tag) >> 24
            if nid >= len(model.nodes):
                continue
            nid, (t, r, sc) = self.node_trs(tag, model.nodes[nid], frame)
            out[nid] = mat_trs(t, r, sc)
        return out


class Bca0:
    def __init__(self, d):
        assert d[:4] == b'BCA0', d[:4]
        self.anims = []
        for i in range(u16(d, 0x0E)):
            so = u32(d, 0x10 + 4 * i)
            if d[so:so + 4] == b'JNT0':
                for name, e in read_dict(d, so + 8):
                    self.anims.append(JointAnim(name, d, so + u32(e, 0)))
