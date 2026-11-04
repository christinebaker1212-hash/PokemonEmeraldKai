#!/usr/bin/env python3
"""Read ORRE's packs back (tools/orre/build_pack.py writes them): what the game's loader does, in Python, for tools
and tests.

  python3 -I tools/orre/pak.py ORRE_DIR                  (what the packs hold)
  python3 -I tools/orre/pak.py ORRE_DIR render MAP OUT.png [TILT]
  python3 -I tools/orre/pak.py ORRE_DIR text FILE        (a text file's strings, e.g. S1_out/S1_out.msg)
"""
import os
import struct
import sys
import zlib

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gxtex  # noqa: E402


class Mesh:
    pass


class Pack3D:
    def __init__(self, path):
        d = open(path, "rb").read()
        magic, ver, nt, nm = struct.unpack_from("<4sIII", d, 0)
        if magic != b"ORZ3":
            raise ValueError("not an orre3d.pak")
        o = 16
        self.tex = []
        for _ in range(nt):
            self.tex.append(struct.unpack_from("<BBHHHII", d, o))
            o += 16
        self.maps = {}
        for _ in range(nm):
            name, off, zs, size = struct.unpack_from("<32sIII", d, o)
            self.maps[name.rstrip(b"\0").decode()] = (off, zs, size)
            o += 44
        self.base = o
        self.d = d
        self._img = {}

    def image(self, i):
        if i not in self._img:
            fmt, tfmt, w, h, n, off, size = self.tex[i]
            blob = self.d[self.base + off:self.base + off + size]
            texels, lut = blob[:size - n * 2], blob[size - n * 2:]
            pal = gxtex.palette(lut, tfmt, n) if n else None
            self._img[i] = gxtex.decode(texels, fmt, w, h, pal)
        return self._img[i]

    def map(self, name):
        """[(kind, [Mesh])]"""
        off, zs, size = self.maps[name]
        b = zlib.decompress(self.d[self.base + off:self.base + off + zs])
        assert len(b) == size
        o = 0
        (ns,) = struct.unpack_from("<I", b, o)
        o += 4
        sets = []
        for _ in range(ns):
            kind, nmesh = struct.unpack_from("<B3xI", b, o)
            o += 8
            meshes = []
            for _ in range(nmesh):
                h = struct.unpack_from("<iIBBBBffBBBBfII", b, o)
                o += struct.calcsize("<iIBBBBffBBBBfII")
                m = Mesh()
                m.tex, rm, m.flags, ws, wt, _, us, vs = h[:8]
                diff, alpha, nv, ni = h[8:12], h[12], h[13], h[14]
                v = np.frombuffer(b, dtype=[("p", "<f4", 3), ("uv", "<f4", 2), ("c", "u1", 4), ("n", "i1", 3), ("x", "u1")],
                                  count=nv, offset=o)
                o += nv * 28
                it = "<u4" if nv > 65535 else "<u2"
                idx = np.frombuffer(b, dtype=it, count=ni, offset=o)
                o += ni * np.dtype(it).itemsize
                m.pos, m.uv, m.col = v["p"][idx], v["uv"][idx], v["c"][idx]
                m.nrm = v["n"][idx].astype(np.float32) / 127
                m.has_clr = bool(m.flags & 8)
                m.material = {"rendermode": rm, "diffuse": diff, "alpha": alpha, "uvscale": (us, vs), "wrap": (ws, wt)}
                meshes.append(m)
            sets.append((kind, meshes))
        return sets


class PackData:
    def __init__(self, path):
        d = open(path, "rb").read()
        magic, ver, n = struct.unpack_from("<4sII", d, 0)
        if magic != b"ORDT":
            raise ValueError("not an orre_data.pak")
        o = 12
        self.files = {}
        for _ in range(n):
            name, off, size, zs = struct.unpack_from("<48sIII", d, o)
            self.files[name.rstrip(b"\0").decode()] = (off, size, zs)
            o += 60
        self.base = o
        self.d = d

    def read(self, name):
        off, size, zs = self.files[name]
        if zs:
            return zlib.decompress(self.d[self.base + off:self.base + off + zs])
        return self.d[self.base + off:self.base + off + size]


if __name__ == "__main__":
    a = sys.argv[1:]
    if not a:
        print(__doc__)
        sys.exit(1)
    p3 = Pack3D(os.path.join(a[0], "orre3d.pak"))
    pd = PackData(os.path.join(a[0], "orre_data.pak"))
    if len(a) == 1:
        print("%d maps, %d textures; %d data files" % (len(p3.maps), len(p3.tex), len(pd.files)))
    elif a[1] == "render":
        import preview
        sets = p3.map(a[2])
        meshes = [m for kind, s in sets if kind != 2 for m in s]
        frame = None
        imgs = {}
        for m in meshes:
            if m.tex >= 0 and m.tex not in imgs:
                imgs[m.tex] = p3.image(m.tex)
        preview.render(meshes, imgs, 600, float(a[4]) if len(a) > 4 else 50.0, 0.0, frame or None).save(a[3])
    elif a[1] == "text":
        import msg
        for k, v in msg.parse(pd.read(a[2])).items():
            print("%d: %s" % (k, msg.show(v).replace("\n", "\\n")))
