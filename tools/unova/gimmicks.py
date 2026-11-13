#!/usr/bin/env python3
"""Black's gym machinery (docs/UNOVA_PARITY.md 1): each gimmick overlay's field expansion objects (ov021
fld_exp_obj.c FLD_EXP_OBJ_AddUnitFromArchive), read as G3D_UTIL_AddUnitFromArchive reads its G3dUtilSetup:

    G3dUtilSetup     u32 resTbl, u16 resCount, u32 objTbl, u16 objCount
    G3dUtilResSetup  u32 archive id, u16 member, u16 pad, u32 type        (8 + 4 bytes; every member of resTbl[0]'s
                                                                           archive)
    G3dUtilObjSetup  u16 model res, u16 model member (a NSBMD's model), u16 texture res, u16 pad,
                     u32 anmTbl ({u16 anim res, u16 anim member} each), u16 anmCount

and each gimmick's model places (its own tables, named in aletheia's src/game/ov0NN). Archive N is a/{N/100}/{N/10%10}/
{N%10} (narc.py).

    python3 -I tools/unova/gimmicks.py           (each gimmick's models, animations and their frames)
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(os.path.dirname(HERE), 'sinnoh'))
import narc  # noqa: E402
import nitro  # noqa: E402
import scrcmd  # noqa: E402

# overlay: (name, its G3dUtilSetup)
SETUPS = {
    38: ('plate doors (BURGH)', 0x021F464C),
    39: ('cart lanes (ELESA)', 0x021F4C18),
    40: ('flag anime (NACRENE)', 0x021F38EC),
    41: ('cannons (SKYLA)', 0x021F40A4),
    42: ('button states (STRIATON)', 0x021F3A58),
    43: ('lift elevator (CLAY, the mine)', 0x021F4318),
    44: ('lift (CLAY, the lobby)', 0x021F39E8),
    45: ('switch toggle (BRYCEN)', 0x021F3B50),
    46: ('rotating room (DRAYDEN)', 0x021F4120),
}


def arc_path(n):
    return 'a/%d/%d/%d' % (n // 100, n // 10 % 10, n % 10)


class Image:
    """An overlay's bytes by RAM address"""
    def __init__(self, ov):
        self.ram, self.d = scrcmd.ov_image(ov)

    def has(self, a):
        return self.ram <= a < self.ram + len(self.d)

    def u16(self, a):
        return struct.unpack_from('<H', self.d, a - self.ram)[0]

    def u32(self, a):
        return struct.unpack_from('<I', self.d, a - self.ram)[0]

    def s32(self, a):
        return struct.unpack_from('<i', self.d, a - self.ram)[0]

    def vec(self, a):
        return tuple(self.s32(a + 4 * k) / 4096.0 for k in range(3))


def setup(img, at):
    """{'res': [(archive, member, type)], 'objs': [(model res, model member, tex res, [(anim res, anim member)])]}"""
    res_tbl, res_n, obj_tbl, obj_n = img.u32(at), img.u16(at + 4), img.u32(at + 8), img.u16(at + 12)
    res = [(img.u32(res_tbl + 12 * i), img.u16(res_tbl + 12 * i + 4), img.u32(res_tbl + 12 * i + 8))
           for i in range(res_n)]
    objs = []
    for i in range(obj_n):
        o = obj_tbl + 16 * i
        anm, anm_n = img.u32(o + 8), img.u16(o + 12)
        objs.append((img.u16(o), img.u16(o + 2), img.u16(o + 4),
                     [(img.u16(anm + 4 * k), img.u16(anm + 4 * k + 2)) for k in range(anm_n)]))
    return {'res': res, 'objs': objs}


def member(s, res):
    """the bytes of a resource of a setup (its own archive: FLD_EXP_OBJ_AddUnit's setups name one a resource;
    AddUnitFromArchive's all share the first's)"""
    arc, dat, _ = s['res'][res]
    return narc.arc(arc_path(arc))[dat]


def main():
    for ov, (name, at) in SETUPS.items():
        if at is None:
            print('ov%03d %s: setup not found yet' % (ov, name))
            continue
        img = Image(ov)
        s = setup(img, at)
        print('ov%03d %s: archive %s, %d resources, %d models' % (ov, name, arc_path(s['res'][0][0]), len(s['res']),
                                                                 len(s['objs'])))
        for i, (mr, md, tr, anims) in enumerate(s['objs']):
            d = member(s, mr)
            kinds = []
            for ar, ad in anims:
                a = member(s, ar)
                kinds.append('%s%s' % (a[:4].decode('latin-1'), '/%d' % nitro.Bca0(a).anims[0].frames
                                       if a[:4] == b'BCA0' else ''))
            print('  model %2d: res %d (%s, %d bytes), tex res %d, anims %s' % (
                i, mr, d[:4].decode('latin-1'), len(d), tr, ' '.join(kinds) or '-'))


if __name__ == '__main__':
    main()


# ------------------------------------------------------------------------------------------------ parts and frames
def _inv(m):
    """inverse of a 4x4 affine matrix (rows), its last row 0 0 0 1"""
    a = [r[:3] for r in m[:3]]
    det = (a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) - a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0])
           + a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]))
    if abs(det) < 1e-12:
        return None
    inv = [[0.0] * 3 for _ in range(3)]
    for i in range(3):
        for j in range(3):
            r = [k for k in range(3) if k != j]
            c = [k for k in range(3) if k != i]
            minor = a[r[0]][c[0]] * a[r[1]][c[1]] - a[r[0]][c[1]] * a[r[1]][c[0]]
            inv[i][j] = (-1) ** (i + j) * minor / det
    t = [-sum(inv[i][k] * m[k][3] for k in range(3)) for i in range(3)]
    return [inv[0] + [t[0]], inv[1] + [t[1]], inv[2] + [t[2]], [0.0, 0.0, 0.0, 1.0]]


def _drawn(model, node_mats):
    """(model.draw's {material: triangles}, [each emitted vertex's matrix, in order], {id(pos): its index})"""
    seen, index = [], {}
    keep = nitro.mat_apply

    def record(m, v):
        p = keep(m, v)
        index[id(p)] = len(seen)
        seen.append((m, p))
        return p
    nitro.mat_apply = record
    try:
        drawn = model.draw(node_mats)
    finally:
        nitro.mat_apply = keep
    return drawn, [m for m, _ in seen], index


def parts(model, anims):
    """A model as rigid parts: (rest {material: triangles} per part, frames[anim][frame][part] = 3x4 row-major matrix
    from the part's rest place to its place at that frame). ANIMS: nitro JointAnims"""
    rest, mats, index = _drawn(model, None)
    src_of, rep = {}, []
    vsrc = []
    for m in mats:   # (a source: the matrix object the vertices were emitted under)
        if id(m) not in src_of:
            src_of[id(m)] = len(rep)
            rep.append(len(vsrc))
        vsrc.append(src_of[id(m)])
    inv = [_inv(mats[r]) for r in rep]
    per_anim = []
    for a in anims:
        fr = []
        for f in range(a.frames):
            _, fm, _ = _drawn(model, a.node_matrices(model, f))
            row = []
            for s, r in enumerate(rep):
                d = nitro.mat_mul(fm[r], inv[s]) if inv[s] else nitro.mat_identity()
                row.append(tuple(round(d[i][j], 4) for i in range(3) for j in range(4)))
            fr.append(row)
        per_anim.append(fr)
    # (sources moving alike are one part)
    sig = {}
    part_of = []
    for s in range(len(rep)):
        k = tuple(tuple(fr[f][s] for f in range(len(fr))) for fr in per_anim)
        if k not in sig:
            sig[k] = len(sig)
        part_of.append(sig[k])
    first = {}
    for s, p in enumerate(part_of):
        first.setdefault(p, s)
    frames = [[[fr[f][first[p]] for p in range(len(sig))] for f in range(len(fr))] for fr in per_anim]
    # (a vertex keeps its part in its colour's fourth byte: a triangle can join two parts, Black's model bending there)
    out = {}
    for mi, tris in rest.items():
        out[mi] = [tuple((pos, uv, (col[0], col[1], col[2], part_of[vsrc[index[id(pos)]]]), nrm)
                         for pos, uv, col, nrm in t) for t in tris]
    return out, frames


# ------------------------------------------------------------------------------------------------ into the pack
def _tex_of(s, res):
    d = member(s, res)
    if d[:4] == b'BTX0':
        return nitro.btx0(d)
    return nitro.Bmd0(d).tex


def _part_mesh(pack, key, model, drawn, tex0):
    """a model's triangles as a pack mesh (as Pack.mesh builds one: a group a material)"""
    import math
    if key in pack.mesh_ids:
        return pack.mesh_ids[key]
    groups, pts = [], []
    for mi, tris in sorted(drawn.items()):
        mat = model.materials[mi]
        cull = {'none': 0, 'back': 1, 'front': 2, 'all': 3}[mat.cull]
        rs, rt, fs, ft = mat.repeat
        groups.append((pack.texture(tex0, mat), rs, rt, fs, ft, mat.alpha, (mat.poly_attr >> 24) & 63, cull,
                       mat.lights, mat.mode, tris))
        pts.extend(v[0] for t in tris for v in t)
    c = [sum(p[i] for p in pts) / len(pts) for i in range(3)] if pts else [0.0, 0.0, 0.0]
    r = max(math.dist(c, p) for p in pts) if pts else 0.0
    pack.mesh_ids[key] = len(pack.meshes)
    pack.meshes.append((c, r, groups))
    return pack.mesh_ids[key]


def _place46(img):
    """DRAYDEN's (field_gimmick_rotating_room.c FIELD_GIMMICK_ROTATING_ROOM_Init): per unit its body, two levers (at
    one place, turned as the body), two buttons; model 15 the extra"""
    out = []
    for u in range(3):
        yaw = img.u16(0x021F4108 + 2 * u)
        body, lever = img.vec(0x021F4194 + 12 * u), img.vec(0x021F4170 + 12 * u)
        out += [(body, yaw), (lever, yaw), (lever, yaw), (img.vec(0x021F4204 + 24 * u), 0),
                (img.vec(0x021F4204 + 24 * u + 12), 0)]
    out.append((img.vec(0x021F41B8), 0))
    return out


# BRYCEN's (field_gimmick_switch_toggle.c FIELD_GIMMICK_SWITCH_TOGGLE_Init): three switches' models, then their
# secondary models (3 + the switch)
def _place45(img):
    return ([(img.vec(0x021F3B80 + 12 * k), 0) for k in range(3)]
            + [(img.vec(0x021F3BA4 + 12 * k), 0) for k in range(3)])


# NACRENE's (field_gimmick_flag_anime.c FIELD_GIMMICK_FLAG_ANIME_Init): one model at x, z (fx32 each), on the ground
def _place40(img):
    return [((img.s32(0x021F38D4) / 4096.0, 0.0, img.s32(0x021F38D8) / 4096.0), 0)]


# STRIATON's (field_gimmick_button_states.c FIELD_GIMMICK_BUTTON_STATES_Init): nine state models, three one-shots
def _place42(img):
    return [(img.vec(0x021F3AD4 + 12 * k), 0) for k in range(9)] + [(img.vec(0x021F3A68 + 12 * k), 0) for k in range(3)]


# CLAY's mine (field_gimmick_lift_elevator.c FIELD_GIMMICK_LIFT_ELEVATOR_Init): six lifts at their starts (their
# heights the VM's: a state each), the obstacle the big lift breaks (6), the elevator (7)
def _place43(img):
    return ([(img.vec(0x021F4358 + 12 * k), 0) for k in range(6)]
            + [(img.vec(0x021F4340), 0), (img.vec(0x021F434C), 0)])


# CLAY's lobby (field_gimmick_lift.c FIELD_GIMMICK_LIFT_Init): its lift
def _place44(img):
    return [(img.vec(0x021F3A1C), 0)]


# SKYLA's (field_gimmick_cannons.c FIELD_GIMMICK_CANNONS_Init): ten cannons, each turned by its quarter turns
def _place41(img):
    return [(img.vec(0x021F4208 + 12 * k), (img.d[0x021F4660 - img.ram + k] << 14) & 0xFFFF) for k in range(10)]


PLACES = {40: _place40, 41: _place41, 42: _place42, 43: _place43, 44: _place44, 45: _place45,
          46: _place46}   # (the gimmicks drawn so far)


def add(pack):
    """(kinds, gimmicks) for build_pack.write: kinds [(mesh (its vertices' parts in their colour's fourth byte), parts,
    [(frames, [[12 floats a part] a frame])] an animation in Black's order)], gimmicks [(overlay, [(kind, (x, y, z),
    yaw, hidden)])]"""
    kinds, kind_ids, gims = [], {}, []
    for ov, place in sorted(PLACES.items()):
        img = Image(ov)
        s = setup(img, SETUPS[ov][1])
        models = []
        for i, ((pos, yaw), (mr, md, tr, anims)) in enumerate(zip(place(img), s['objs'])):
            k = (s['res'][0][0], mr, md, tr, tuple(anims))
            if k not in kind_ids:
                bmd = nitro.Bmd0(member(s, mr))
                model = bmd.models[md] if md < len(bmd.models) else bmd.models[0]
                tex0 = _tex_of(s, tr) or bmd.tex
                joint = []
                for ar, ad in anims:
                    a = member(s, ar)
                    joint.append(nitro.Bca0(a).anims[min(ad, len(nitro.Bca0(a).anims) - 1)] if a[:4] == b'BCA0'
                                 else None)
                ps, frames = parts(model, [j for j in joint if j is not None])
                nparts = max((v[2][3] for tris in ps.values() for t in tris for v in t), default=0) + 1
                mesh = _part_mesh(pack, ('gimmick',) + k, model, ps, tex0)
                it = iter(frames)   # (a joint animation's frames; another kind's (texture) animation: none)
                kind_anims = []
                for j in joint:
                    f = next(it) if j is not None else []
                    kind_anims.append((len(f), f))
                kind_ids[k] = len(kinds)
                kinds.append((mesh, nparts, kind_anims))
                print('gimmick ov%03d model %d: %d parts, animations %s' % (
                    ov, i, nparts, [n for n, _ in kind_anims]))
            models.append((kind_ids[k], pos, yaw, 0))
        gims.append((ov, models))
    return kinds, gims
