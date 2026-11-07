#!/usr/bin/env python3
"""ORRE's packs against the disc they came from: every map's triangles (positions, uvs, colours) and every texture
read back from orre3d.pak equal what tools/orre/hsd.py reads from the unpacked disc, and every file of orre_data.pak
is the disc's own bytes.

  python3 -I tools/orre/test_pack.py UNPACKED_DIR ORRE_DIR
"""
import hashlib
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hsd  # noqa: E402
import pak  # noqa: E402
import build_pack  # noqa: E402
import grid  # noqa: E402


def main(src, out):
    p3 = pak.Pack3D(os.path.join(out, "orre3d.pak"))
    pd = pak.PackData(os.path.join(out, "orre_data.pak"))
    bad = 0
    seen = set()
    textures_checked = set()
    for arc in sorted(os.listdir(src)):
        for fn in sorted(os.listdir(os.path.join(src, arc))):
            path = os.path.join(src, arc, fn)
            if build_pack.is_data(arc, fn):
                if hashlib.sha1(pd.read("%s/%s" % (arc, fn))).digest() != hashlib.sha1(open(path, "rb").read()).digest():
                    print("FAIL data", arc, fn)
                    bad += 1
            if not fn.endswith(".map") or fn in seen:
                continue
            seen.add(fn)
            f = hsd.HsdFile(open(path, "rb").read())
            sets = hsd.model_sets(f)
            got = p3.map(fn[:-4])
            if len(got) != len(sets):
                print("FAIL sets", fn)
                bad += 1
                continue
            for s, (kind, g) in zip(sets, got):
                if len(s) != len(g):
                    print("FAIL meshes", fn)
                    bad += 1
                    continue
                for a, b in zip(s, g):
                    ok = (np.array_equal(a.pos, b.pos) and np.array_equal(a.uv, b.uv)
                          and np.array_equal(a.col, b.col))
                    if not ok:
                        print("FAIL mesh", fn)
                        bad += 1
                    if a.tex >= 0 and b.tex not in textures_checked:
                        textures_checked.add(b.tex)
                        if not np.array_equal(f.images[a.tex], p3.image(b.tex)):
                            print("FAIL texture", fn, a.tex)
                            bad += 1
    # the grids of the maps in the game, as grid.py makes them
    import maps as orre_maps
    for m in orre_maps.load(src):
        g = orre_maps.build_grid(src, m)
        p3.map(m["file"])
        got = p3.grid
        if got is None or got["w"] != g["w"] or got["h"] != g["h"] or not np.array_equal(got["cells"]["f"] & 1, g["walk"].astype(np.uint8)):
            print("FAIL grid", m["file"])
            bad += 1
    print("%d maps, %d textures, %d data files checked: %s" % (len(seen), len(textures_checked), len(pd.files),
                                                              "OK" if not bad else "%d FAILED" % bad))
    return 1 if bad else 0


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    sys.exit(main(sys.argv[1], sys.argv[2]))
