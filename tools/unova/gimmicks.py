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
    48: ('gate news board (the gates)', 0x021F4EA4),   # (gGateExpObjSetups[1]: the English game's, text mode 0)
    58: ('the lane traffic (ROUTE 4, the DESERT RESORT: zones 326 / 157)', 0x021F3B60),   # (sLaneTrainsUnitSetup)
    49: ("the LEAGUE's lamps (zone 137)", 0x021F3894),   # (gModelsASetup)
    62: ("the LIBERTY ship at CASTELIA's pier (zone 235)", 0x021F3768),   # (gFieldGimmick62UnitSetup)
    59: ("the special POKeMON's ball and ZORUA's smoke (CASTELIA, zone 47)", 0x021F3E74),   # (sSpPokeGimmickSetup)
    159: ("the special POKeMON's ball (zones 53 / 346)", 0x021F3E84, 59),   # (sSpPokeGimmickPairBSetup: A's alike)
    50: ("the LEAGUE's landing (zone 138)", 0x021F38A4),   # (gModelsBSetup)
    53: ("SKYARROW BRIDGE's traffic (zone 249)", 0x021F3D60),   # (sSoundObjsUnitSetup: two cars each way, the ship)
    54: ("TUBELINE BRIDGE's trains (zone 254)", 0x021F39A0),   # (sSoundObjsBUnitSetup)
    52: ("the BATTLE SUBWAY's trains at the GEAR STATION's platforms (zones 67-76)", 'btrain'),
    248: ("the object codes Black draws as 3D models (boulders, the DRAGON SKULL...: OBJMODEL_OV)", 'mmdl'),
    171: ('the ELITE FOUR rooms\' entrance scene, slot 1 (zone 140)', 'fk'),
    172: ('the ELITE FOUR rooms\' entrance scene, slot 2 (zone 141)', 'fk'),
    173: ('the ELITE FOUR rooms\' entrance scene, slot 3 (zone 142)', 'fk'),
    174: ('the ELITE FOUR rooms\' entrance scene, slot 4 (zone 143)', 'fk'),
    151: ('CHARGESTONE CAVE 1F, its stones (zone 195)', 'd06'),
    152: ('CHARGESTONE CAVE B1F, its stones (zone 196)', 'd06'),
    153: ('CHARGESTONE CAVE B2F, its stones (zone 197)', 'd06'),   # (gGateExpObjSetups[1]: the English game's, text mode 0)
    64: ('arc effect, the LIGHT STONE (N\'s Castle, zone 278)', 0x021F3C30),   # (gArcEffectDefaultUnitSetup)
    65: ('time effect, by day (zone 389)', 0x021F38F8),
    165: ('time effect, by night (zone 389)', 0x021F3918, 65),   # (ov065's night setup: its image, a number of its own)
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
    for ov, sv in SETUPS.items():
        name, at = sv[:2]
        if not isinstance(at, int):
            continue
        if at is None:
            print('ov%03d %s: setup not found yet' % (ov, name))
            continue
        img = Image(sv[2] if len(sv) > 2 else ov)
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
        _drawn.vis.append(model.vis_node)
        return p
    _drawn.vis = []
    nitro.mat_apply = record
    try:
        drawn = model.draw(node_mats)
    finally:
        nitro.mat_apply = keep
    _drawn.points = [p for _, p in seen]
    return drawn, [m for m, _ in seen], index


def parts(model, anims, vis=None):
    """A model as rigid parts: (rest {material: triangles} per part, frames[anim][frame][part] = 3x4 row-major matrix
    from the part's rest place to its place at that frame). ANIMS: nitro JointAnims; VIS: a nitro VisAnim (an NSBVA's)
    hiding nodes by the joint animations' frames (a hidden part's matrix all zero)"""
    rest, mats, index = _drawn(model, None)
    vnode = list(_drawn.vis)
    src_of, rep = {}, []
    vsrc, src_vis = [], []
    for k, m in enumerate(mats):   # (a source: the matrix object the vertices were emitted under, and their node)
        key = (id(m), vnode[k] if vis is not None else -1)
        if key not in src_of:
            src_of[key] = len(rep)
            rep.append(k)
            src_vis.append(key[1])
        vsrc.append(src_of[key])
    inv = [_inv(mats[r]) for r in rep]
    # (a source flat at rest, a node scaled to nothing there (ov064's glow, its rays): its vertices taken where it
    # first has a shape, the part's rest that place)
    flat = [s for s in range(len(rep)) if inv[s] is None]
    if flat and anims:
        moved = {}
        for a in anims:
            for f in range(a.frames):
                if not flat:
                    break
                _, fm, _ = _drawn(model, a.node_matrices(model, f))
                pts = _drawn.points
                for s in list(flat):
                    m = _inv(fm[rep[s]])
                    if m is not None:
                        inv[s] = m
                        for k, src in enumerate(vsrc):
                            if src == s:
                                moved[k] = pts[k]
                        flat.remove(s)
        if moved:   # (the vertices keyed by their number: the moved ones are another draw's points)
            rest = {mi: [tuple((moved.get(index[id(pos)], pos), uv, col, nrm) for pos, uv, col, nrm in t)
                         for t in tris] for mi, tris in rest.items()}
            for k, p in moved.items():
                index[id(p)] = k
    per_anim = []
    for a in anims:
        fr = []
        for f in range(a.frames):
            _, fm, _ = _drawn(model, a.node_matrices(model, f))
            row = []
            for s, r in enumerate(rep):
                if vis is not None and src_vis[s] >= 0 and not vis.shown(src_vis[s], f):
                    row.append((0.0,) * 12)   # (its node hidden this frame)
                    continue
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


def _texture_frames(pack, model, drawn, tex0, d):
    """A texture animation (NSBTP / NSBTA / NSBMA: D, its file) baked for the mesh's groups (one a drawn material, in
    _part_mesh's order): (frames, [group], [[(texture, alpha 0-31 or 255, du, dv, (r, g, b) multipliers) a touched
    group] a frame]); None when it touches none of them"""
    import copy
    order = [mi for mi, _ in sorted(drawn.items())]
    if d[:4] == b'BTP0':
        anims, kind = nitro.Btp0(d).anims, 'pat'
    elif d[:4] == b'BTA0':
        anims, kind = nitro.Bta0(d).anims, 'srt'
    elif d[:4] == b'BMA0':
        anims, kind = nitro.Bma0(d).anims, 'mat'
    else:
        return None
    if not anims:
        return None
    a = anims[0]
    groups = [gi for gi, mi in enumerate(order) if model.materials[mi].name in a.mats]
    if not groups or not a.frames:
        return None
    out = []
    for f in range(a.frames):
        row = []
        for gi in groups:
            mat = model.materials[order[gi]]
            tex, alpha, du, dv, mul = -1, 255, 0.0, 0.0, (1.0, 1.0, 1.0)
            if kind == 'pat':
                pick = a.pick(mat.name, f)
                if pick:
                    m2 = copy.copy(mat)
                    m2.texture, m2.palette = pick
                    tex = pack.texture(tex0, m2)
            elif kind == 'srt':
                v = a.value(mat.name, f)
                if v:
                    du, dv = v[2], v[3]
            else:
                v = a.value(mat.name, f)
                if v:
                    dif, alpha = v
                    rest = mat.dif_amb & 0x7FFF
                    mul = tuple((((dif >> (5 * c)) & 31) + 1) / (((rest >> (5 * c)) & 31) + 1) for c in range(3))
            row.append((tex, alpha, du, dv, mul))
        out.append(row)
    return a.frames, groups, out


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


# BURGH's (field_gimmick_plate_doors.c FIELD_GIMMICK_PLATE_DOORS_Init): eight switches, ten plates, seven doors shut
# (18-24) and open (25-31) at one place; the effect models (32-34) hidden until an event places them
def _place38(img):
    doors = [(img.vec(0x021F4694 + 12 * k), 0) for k in range(7)]
    return ([(img.vec(0x021F46E8 + 12 * k), 0) for k in range(8)] + [(img.vec(0x021F4748 + 12 * k), 0) for k in range(10)]
            + doors + doors + [((0.0, 0.0, 0.0), 0, 1)] * 3)


# ELESA's (field_gimmick_cart_lanes.c FIELD_GIMMICK_CART_LANES_Setup): each model at its own place (sModelPos00-11)
def _place39(img):
    return [(img.vec(a), 0) for a in (0x021F4D60, 0x021F4D0C, 0x021F4D6C, 0x021F4D18, 0x021F4D24, 0x021F4D3C, 0x021F4D00,
                                     0x021F4CD0, 0x021F4CDC, 0x021F4D30, 0x021F4D48, 0x021F4CE8)]


# the time effect (ov065 field_gimmick_time_effect.c): its one model at sTimeEffectEffectPosition
def _place65(img):
    return [(img.vec(0x021F3970), 0)]


# the LIGHT STONE's arc effect (ov064 field_gimmick_arc_effect.c): both models hidden until the script places them
def _place64(img):
    return [((0.0, 0.0, 0.0), 0, 1)] * 2


# CHARGESTONE CAVE's stones (ov051 field_gimmick_d06.c, its effect ov021 fldeff_d06denki.c: the field effects' archive
# 0x4B, sD06DenkiModelFiles / AnimeFiles): a stone (model 78: its float 120, its ending 121) and its shadow (81) for
# each of the floor's stones (gGimmickD06StoneTable), hidden until the VM places them, and a panel's pulse (79: its
# idle 122, its flash 123) at each of the floor's electric panels (gGimmickD06PanelTable)
D06_SETUP = {'res': [(75, 78, 0), (75, 120, 0), (75, 121, 0), (75, 79, 0), (75, 122, 0), (75, 123, 0), (75, 81, 0)],
             'objs': [(0, 0, 0, [(1, 0), (2, 0)]), (6, 0, 6, []), (3, 0, 3, [(4, 0), (5, 0)])]}
D06_ZONES = {151: 195, 152: 196, 153: 197}

# the 3D models Black draws for the object codes past its people (FLDMMDL_ObjCodeToTableIndex 792-798: drawType 2, the
# model archive 0x31 = a/0/4/9, ov021 fldmmdl_g3dobj.c FLDMMDL_G3DOBJ_LoadRes): the STRENGTH boulders bigrock /
# bigrock2, NACRENE's DRAGON SKULL, PINWHEEL FOREST's and TWIST MOUNTAIN's ground patches, the ABYSSAL RUINS' cube.
# No place of their own: src/unova/u3d.c ObjModelsDraw draws each where its object stands (gimmick OBJMODEL_OV)
OBJMODEL_OV = 248
OBJMODEL_SETUP = {'res': [(49, k, 0) for k in range(6)], 'objs': [(k, 0, k, []) for k in range(6)]}
D06_STONES, D06_PANELS = 12, 14


# the BATTLE SUBWAY's trains (ov021 fldeff_btrain.c, started by the subway's script commands 18 / 34, ov052 sets the
# zones up): its eight trains (sBtrainModelFiles, the effects' archive 0x4B), each with the four animations they all
# share (sBtrainAnimeFiles: its doors), hidden until a script brings one in
BTRAIN_SETUP = {'res': [(75, f, 0) for f in (85, 86, 87, 88, 89, 90, 91, 92)] + [(75, f, 0) for f in (124, 125, 126, 127)],
                'objs': [(k, 0, k, [(8, 0), (9, 0), (10, 0), (11, 0)]) for k in range(8)]}


# the ELITE FOUR rooms' entrance scenes (ov056 field_fk_demo.c FIELD_FK_DEMO_InitModel, archive 0x96): a slot's set
# with its hero, for a male (model 0: files 6 * slot + 3, his hero's frames + 4) and a female (model 1: + 1, + 2), both
# with the set's joint animation (+ 5), at gFkDemoOrigin and hidden until the scene (tools/unova/gen_fkdemo.py)
def fk_setup(slot):
    b = 6 * slot
    return {'res': [(150, b + 3, 0), (150, b + 1, 0), (150, b + 5, 0), (150, b + 4, 0), (150, b + 2, 0)],
            'objs': [(0, 0, 0, [(2, 0), (3, 0)]), (1, 0, 1, [(2, 0), (4, 0)])]}


# Black's 3D cut-ins (ov021 fld3d_ci.c FLD3D_CI_LoadSequence): a sequence's record in archive 0x75 (u32 particle effect,
# u16 two start delays, two models, two wait delays, model 0's four animations, model 1's four, u32 the field
# darkened), its models and animations members of archive 0x73 (three a model read), both models at the origin under
# the cut-in's own camera (u3d.c U3D_Ci*); gimmick CI_BASE + the sequence
CI_BASE = 1000
CI_SEQS = 53


def ci_record(seq):
    v = struct.unpack_from('<I2H2H2H4H4HI', narc.arc('a/1/1/7')[seq])
    return {'effect': v[0], 'start': v[1:3], 'models': [m for m in v[3:5] if m != 0xFFFF], 'wait': v[5:7],
            'anims': [[a for a in v[7:10] if a != 0xFFFF], [a for a in v[11:14] if a != 0xFFFF]], 'dark': v[15]}


def ci_setup(seq):
    r = ci_record(seq)
    res, objs = [], []
    for k, m in enumerate(r['models']):
        res.append((0x73, m, 0))
    for k in range(len(r['models'])):
        anims = []
        for a in r['anims'][k]:
            anims.append((len(res), 0))
            res.append((0x73, a, 0))
        objs.append((k, 0, k, anims))
    return {'res': res, 'objs': objs}


def _place_ci(img):
    return [((0.0, 0.0, 0.0), 0, 0), ((0.0, 0.0, 0.0), 0, 0)]


def _place_fk(img):
    return [((256.0, 0.0, 256.0), 0, 1)] * 2


# the special POKeMON's scenes (ov059 sp_poke_gimmick.c): the ball's way out (model 0) and in (1), and in zone 47
# ZORUA's smoke (2), all hidden until a script command places them
# the LIBERTY ship (ov062 field_gimmick_62.c): at gFieldGimmick62ModelPos, its water animation on
def _place62(img):
    return [(img.vec(0x021F3790), 0)]


def _place59(img):
    return [((0.0, 0.0, 0.0), 0, 1)] * 3


def _place159(img):
    return [((0.0, 0.0, 0.0), 0, 1)] * 2


def _place_btrain(img):
    return [((0.0, 0.0, 0.0), 0, 1)] * 8


# ROUTE 4's lane traffic (ov058 field_gimmick_lane_trains.c FIELD_GIMMICK_LANE_TRAINS_PlaceLaneTrains): three lanes'
# four models (east lead and trail, west lead and trail) at the track's ends, x 6000 / 7264 (the trail 96 behind), y 80,
# z 8448 east, 8368 west; the VM runs them
def _place58(img):
    out = []
    for lane in range(3):
        out += [((6000.0, 80.0, 8448.0), 0x4000), ((6000.0 - 96, 80.0, 8448.0), 0x4000),
                ((7264.0, 80.0, 8368.0), 0xC000), ((7264.0 + 96, 80.0, 8368.0), 0xC000)]
    return out


# SKYARROW's and TUBELINE's traffic (ov053 field_gimmick_sound_objs.c, ov054 _b.c): moved along their paths each frame
# by the VM (tools/unova/gen_soundobjs.py), drawn from the start
def _place53(img):
    return [((0.0, -4096.0, 0.0), 0)] * 5


def _place54(img):
    return [((0.0, -4096.0, 0.0), 0)] * 2


# the LEAGUE (ov049 field_gimmick_models_a.c, ov050 _b.c): their models at gModelsAModelPos / gModelsBModelPos; ov050's
# model 1 hidden until the landing shows it
def _place49(img):
    return [(img.vec(0x021F38A4 + 12 * i), 0) for i in range(6)]


def _place50(img):
    return [(img.vec(0x021F38B4), 0), (img.vec(0x021F38B4 + 12), 0, 1)]


def d06_tables():
    """{zone: (stones [(x, y, z)], panels [(x, y, z)])} from ov051's tables"""
    img = Image(51)
    out = {}
    for k in range(3):
        rows = []
        for at in (0x021F3DB4, 0x021F3DFC):
            z, n, p = img.s32(at + 12 * k), img.s32(at + 12 * k + 4), img.u32(at + 12 * k + 8)
            rows.append((z, [(img.s32(p + 12 * i), img.s32(p + 12 * i + 4), img.s32(p + 12 * i + 8)) for i in range(n)]))
        out[rows[0][0]] = (rows[0][1], rows[1][1])
    return out


def _d06_place(zone):
    def place(img):
        stones, panels = d06_tables()[zone]
        hidden = [((0.0, 0.0, 0.0), 0, 1)]
        out = hidden * (2 * D06_STONES)
        out += [((x * 16.0 + 8, y * 16.0 + 8, z * 16.0 + 8), 0) for x, y, z in panels]
        return out + hidden * (D06_PANELS - len(panels))
    return place


def _d06_objs():
    """the models' setups in the order the places give them: the stones, their shadows, the panels"""
    st, sh, pa = D06_SETUP['objs']
    return [st] * D06_STONES + [sh] * D06_STONES + [pa] * D06_PANELS


# a gate's news board (ov048 FIELD_GIMMICK_GATE_PlaceBoardModel): placed by the zone's record, hidden until then
def _place48(img):
    return [((0.0, 0.0, 0.0), 0, 1)]


PLACES = {248: lambda img: [((0.0, 0.0, 0.0), 0, 1)] * 6, 62: _place62, 59: _place59, 159: _place159, 171: _place_fk, 172: _place_fk, 173: _place_fk, 174: _place_fk, 49: _place49, 50: _place50, 53: _place53, 54: _place54, 58: _place58, 52: _place_btrain, 151: _d06_place(195), 152: _d06_place(196), 153: _d06_place(197), 48: _place48, 64: _place64, 65: _place65, 165: _place65, 38: _place38, 39: _place39, 40: _place40, 41: _place41, 42: _place42, 43: _place43, 44: _place44, 45: _place45,
          46: _place46}   # (the gimmicks drawn so far)
for _q in range(CI_SEQS):
    SETUPS[CI_BASE + _q] = ('a 3D cut-in, sequence %d' % _q, 'ci')
    PLACES[CI_BASE + _q] = _place_ci


def add(pack):
    """(kinds, gimmicks) for build_pack.write: kinds [(mesh (its vertices' parts in their colour's fourth byte), parts,
    [(frames, [[12 floats a part] a frame])] an animation in Black's order)], gimmicks [(overlay, [(kind, (x, y, z),
    yaw, hidden)])]"""
    kinds, kind_ids, gims = [], {}, []
    for ov, place in sorted(PLACES.items()):
        if SETUPS[ov][1] == 'd06':   # (a setup of our own: a field effect's models, CHARGESTONE's stones)
            img, s = None, dict(D06_SETUP, objs=_d06_objs())
        elif SETUPS[ov][1] == 'mmdl':
            img, s = None, OBJMODEL_SETUP
        elif SETUPS[ov][1] == 'btrain':
            img, s = None, BTRAIN_SETUP
        elif SETUPS[ov][1] == 'fk':
            img, s = None, fk_setup(ov - 171)
        elif SETUPS[ov][1] == 'ci':
            img, s = None, ci_setup(ov - CI_BASE)
        else:
            img = Image(SETUPS[ov][2] if len(SETUPS[ov]) > 2 else ov)
            s = setup(img, SETUPS[ov][1])
        models = []
        for i, (where, (mr, md, tr, anims)) in enumerate(zip(place(img), s['objs'])):
            pos, yaw, hidden = (where + (0,))[:3]
            k = (tuple(s['res'][mr][:2]), md, tuple(s['res'][tr][:2]),   # (by the members themselves: two setups
                 tuple((tuple(s['res'][ar][:2]), ad) for ar, ad in anims))   # of one overlay share none, ov065's)
            if k not in kind_ids:
                bmd = nitro.Bmd0(member(s, mr))
                model = bmd.models[md] if md < len(bmd.models) else bmd.models[0]
                tex0 = _tex_of(s, tr) or bmd.tex
                joint, vis = [], None
                for ar, ad in anims:
                    a = member(s, ar)
                    joint.append(nitro.Bca0(a).anims[min(ad, len(nitro.Bca0(a).anims) - 1)] if a[:4] == b'BCA0'
                                 else None)
                    if a[:4] == b'BVA0' and nitro.Bva0(a).anims:   # (a cut-in's: its nodes shown or hidden by frame)
                        vis = nitro.Bva0(a).anims[0]
                ps, frames = parts(model, [j for j in joint if j is not None], vis)
                nparts = max((v[2][3] for tris in ps.values() for t in tris for v in t), default=0) + 1
                mesh = _part_mesh(pack, ('gimmick',) + k, model, ps, tex0)
                it = iter(frames)   # (a joint animation's frames; a texture one's overrides of its groups)
                kind_anims = []
                for j, (ar, ad) in zip(joint, anims):
                    if j is not None:
                        f = next(it)
                        kind_anims.append(('joint', len(f), f))
                    else:
                        t = _texture_frames(pack, model, ps, tex0, member(s, ar))
                        kind_anims.append(('tex',) + t if t else ('joint', 0, []))
                kind_ids[k] = len(kinds)
                kinds.append((mesh, nparts, kind_anims))
                print('gimmick ov%03d model %d: %d parts, animations %s' % (
                    ov, i, nparts, ['%s%d' % ('t' if k[0] == 'tex' else '', k[1]) for k in kind_anims]))
            models.append((kind_ids[k], pos, yaw, hidden))
        gims.append((ov, models))
    return kinds, gims
