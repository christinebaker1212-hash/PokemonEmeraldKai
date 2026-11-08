#!/usr/bin/env python3
"""Pokemon Colosseum's camera shots (.cam: an HSD scene whose camera set is a perspective CObjDesc and its CameraAnim:
the decomp's include/hsd/hsd_cobj.h, hsd_wobj.h, hsd_fobj.h): the camera's eye and the point it looks at, its roll and
field of view, frame by frame. For looking at a shot; the game reads the same bytes itself (src/orre/o3d_cam.c).

    python3 -I tools/orre/hsd_cam.py FILE.cam [STEP]     (prints frame, eye x y z, target x y z, roll, fov)

Track types: an eye's / target's WObj 5 6 7 (x y z); the CObj's own 10 the field of view (degrees, vertical).
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hsd  # noqa: E402
import hsd_anim  # noqa: E402


def tracks(f, aobj):
    out, end = {}, 0.0
    if not aobj:
        return out, end
    end = f.f32(aobj + 4)
    fo = f.u32(aobj + 8)
    while fo:
        typ, keys = hsd_anim.decode_fobj(f, fo)
        out[typ] = keys
        fo = f.u32(fo)
    return out, end


def load(data):
    f = hsd.HsdFile(data, decode_textures=False)
    scene = f.roots['scene_data']
    cset = f.u32(scene + 4)                  # (SceneDesc: model sets, the camera set, light sets, fog)
    cobj, anims = f.u32(cset), f.u32(cset + 4)
    # (CObjDesc perspective: class name, u16 flags, u16 projection, viewport and scissor as 8 u16, eye + 0x18,
    # target + 0x1C (WObjDesc: class name, f32 x y z), roll + 0x20, up + 0x24, near + 0x28, far + 0x2C, fov + 0x30)
    eye_w, tgt_w = f.u32(cobj + 0x18), f.u32(cobj + 0x1C)
    base = {'eye': [f.f32(eye_w + 4 + 4 * i) for i in range(3)], 'tgt': [f.f32(tgt_w + 4 + 4 * i) for i in range(3)],
            'roll': f.f32(cobj + 0x20), 'fov': f.f32(cobj + 0x30), 'near': f.f32(cobj + 0x28), 'far': f.f32(cobj + 0x2C)}
    ca = f.u32(anims) if anims else 0
    ctr, end = tracks(f, f.u32(ca)) if ca else ({}, 0.0)
    etr, e2 = tracks(f, f.u32(f.u32(ca + 4))) if ca and f.u32(ca + 4) else ({}, 0.0)
    ttr, e3 = tracks(f, f.u32(f.u32(ca + 8))) if ca and f.u32(ca + 8) else ({}, 0.0)
    return base, ctr, etr, ttr, max(end, e2, e3)


def at(base, ctr, etr, ttr, frame):
    def v(tr, typ, d):
        s = hsd_anim.sample(tr.get(typ), frame)
        return d if s is None else s
    eye = [v(etr, 5 + i, base['eye'][i]) for i in range(3)]
    tgt = [v(ttr, 5 + i, base['tgt'][i]) for i in range(3)]
    return eye, tgt, ctr


def main():
    data = open(sys.argv[1], 'rb').read()
    step = int(sys.argv[2]) if len(sys.argv) > 2 else 15
    base, ctr, etr, ttr, end = load(data)
    print('base', base, 'end frame', end, 'cobj tracks', sorted(ctr), 'eye tracks', sorted(etr), 'target', sorted(ttr))
    for fr in range(0, int(end) + 1, step):
        e, t, _ = at(base, ctr, etr, ttr, fr)
        extra = ' '.join('%d:%.2f' % (k, hsd_anim.sample(ctr[k], fr)) for k in sorted(ctr))
        print('%4d eye %7.1f %7.1f %7.1f  target %7.1f %7.1f %7.1f  %s' % (fr, *e, *t, extra))


if __name__ == '__main__':
    main()
