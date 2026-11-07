"""HSD joint animation (AnimJoint / AObjDesc / FObjDesc): decoding, sampling, posing a Colosseum model,
and the two-bone IK HAL solves at run time (legs: JOINT1 / JOINT2 / EFFECTOR joints and their RObjs).
Keyframe values are little-endian in Colosseum's files, unlike the rest. Used by people_sprites.py."""
import math, struct, sys
import numpy as np
sys.path.insert(0, __import__('os').path.dirname(__import__('os').path.abspath(__file__)))
import hsd

CON, LIN, SPL0, SPL, SLP, KEY = 1, 2, 3, 4, 5, 6
TRACK = {1: 'rx', 2: 'ry', 3: 'rz', 5: 'tx', 6: 'ty', 7: 'tz', 8: 'sx', 9: 'sy', 10: 'sz'}

def read_val(f, o, fmt):
    t, sh = fmt >> 5, fmt & 0x1F
    sc = float(1 << sh)
    # Colosseum's keyframe values are little-endian, unlike the rest of the file.
    if t == 0: return struct.unpack_from('<f', f.d, f.base + o)[0], 4
    if t == 1: return struct.unpack_from('<h', f.d, f.base + o)[0] / sc, 2
    if t == 2: return struct.unpack_from('<H', f.d, f.base + o)[0] / sc, 2
    if t == 3: return struct.unpack_from('<b', f.d, f.base + o)[0] / sc, 1
    if t == 4: return f.d[f.base + o] / sc, 1
    raise ValueError(fmt)

def decode_fobj(f, fo):
    length, start = f.u32(fo + 4), f.f32(fo + 8)
    typ, fv, fs = f.d[f.base + fo + 0xC], f.d[f.base + fo + 0xD], f.d[f.base + fo + 0xE]
    ad = f.u32(fo + 0x10)
    p, end = ad, ad + length
    t = start
    keys = []
    while p < end:
        b = f.d[f.base + p]; p += 1
        op, n = b & 0xF, (b >> 4) & 7
        sh = 3
        while b & 0x80:
            b = f.d[f.base + p]; p += 1
            n += (b & 0x7F) << sh; sh += 7
        n += 1
        for _ in range(n):
            val = tan = 0.0; wait = 0
            if op in (CON, LIN, SPL0, SPL, KEY):
                val, k = read_val(f, p, fv); p += k
            if op in (SPL, SLP):
                tan, k = read_val(f, p, fs); p += k
            if op in (CON, LIN, SPL0, SPL):
                sh2 = 0
                while True:
                    b = f.d[f.base + p]; p += 1
                    wait |= (b & 0x7F) << sh2; sh2 += 7
                    if not b & 0x80: break
            if op == SLP:
                if keys: keys[-1] = keys[-1][:3] + (keys[-1][3], tan)   # (the outgoing slope of the last key)
                continue
            keys.append((t, val, op, tan, tan))
            t += wait
    return typ, keys

def sample(keys, frame):
    if not keys: return None
    if frame <= keys[0][0] or len(keys) == 1: return keys[0][1]
    for i in range(len(keys) - 1):
        t0, v0, op, _, m0 = keys[i]
        t1, v1, _, m1, _ = keys[i + 1]
        if t0 <= frame < t1:
            if op == CON or t1 == t0: return v0
            u = (frame - t0) / (t1 - t0)
            if op == LIN: return v0 + (v1 - v0) * u
            if op == SPL0: m0 = m1 = 0.0
            d = t1 - t0
            h00, h10, h01, h11 = 2*u**3-3*u**2+1, u**3-2*u**2+u, -2*u**3+3*u**2, u**3-u**2
            return h00*v0 + h10*d*m0 + h01*v1 + h11*d*m1
    return keys[-1][1]

def anim_tree(f, j, a, out):
    """Pairs the animation tree with the joint tree: out[joint] = {component: keys}."""
    while j and a:
        aobj = f.u32(a + 8)
        if aobj:
            tr = {}
            fo = f.u32(aobj + 8)
            while fo:
                typ, keys = decode_fobj(f, fo)
                if typ in TRACK: tr[TRACK[typ]] = keys
                fo = f.u32(fo)
            out[j] = (tr, f.f32(aobj + 4))
        jc, ac = f.u32(j + 8), f.u32(a)
        if jc and ac: anim_tree(f, jc, ac, out)
        j, a = f.u32(j + 0xC), f.u32(a + 4)
    return out

def posed_srt(f, j, pose, frame):
    vals = {k: f.f32(j + o) for k, o in zip(('rx','ry','rz','sx','sy','sz','tx','ty','tz'), range(0x14, 0x38, 4))}
    if pose and j in pose:
        for k, keys in pose[j][0].items():
            v = sample(keys, frame)
            if v is not None: vals[k] = v
    cx, sxr = math.cos(vals['rx']), math.sin(vals['rx'])
    cy, syr = math.cos(vals['ry']), math.sin(vals['ry'])
    cz, szr = math.cos(vals['rz']), math.sin(vals['rz'])
    Rx = np.array([[1,0,0],[0,cx,-sxr],[0,sxr,cx]]); Ry = np.array([[cy,0,syr],[0,1,0],[-syr,0,cy]])
    Rz = np.array([[cz,-szr,0],[szr,cz,0],[0,0,1]])
    m = np.eye(4); m[:3,:3] = Rz @ Ry @ Rx @ np.diag([vals['sx'], vals['sy'], vals['sz']]); m[:3,3] = [vals['tx'], vals['ty'], vals['tz']]
    return m

def load_posed(f, root, pose, frame):
    """hsd.load_model with the joints posed: hsd.srt swapped for the animated one while it runs."""
    old = hsd.srt
    hsd.srt = lambda j, ff: posed_srt(ff, j, pose, frame)
    try:
        return hsd.load_model(f, root)
    finally:
        hsd.srt = old

def model(path):
    f = hsd.HsdFile(open(path, 'rb').read())
    ms = f.ptr_list(f.u32(f.roots['scene_data']))[0]
    root, aj = f.u32(ms), f.u32(ms + 4)
    anims = [anim_tree(f, root, a, {}) for a in f.ptr_list(aj)] if aj else []
    return f, root, anims

JOINT1, JOINT2, EFFECTOR = 0x200000, 0x400000, 0x600000

def robjs(f, j):
    r, out = f.u32(j + 0x3C), []
    while r:
        out.append((f.u32(r + 4), f.u32(r + 8)))
        r = f.u32(r)
    return out

def ik_chains(f, joints):
    """(hip, knee, effector, target, len1, len2) for each two-bone chain HAL's IK solves at run time."""
    out = []
    for j in joints:
        if f.u32(j + 4) & 0x600000 != JOINT1: continue
        k = f.u32(j + 8)
        while k and f.u32(k + 4) & 0x600000 != JOINT2: k = f.u32(k + 0xC)
        if not k: continue
        e = f.u32(k + 8)
        while e and f.u32(e + 4) & 0x600000 != EFFECTOR: e = f.u32(e + 0xC)
        if not e: continue
        hint = lambda jj: next((f.f32(u) for fl, u in robjs(f, jj) if (fl >> 28) & 7 == 4), None)
        tgt = next((u for fl, u in robjs(f, e) if (fl >> 28) & 7 == 1), None)
        if tgt: out.append((j, k, e, tgt, hint(j), hint(k)))
    return out

def subtree(f, j, out):
    c = f.u32(j + 8)
    while c:
        out.append(c); subtree(f, c, out); c = f.u32(c + 0xC)
    return out

def solve_ik(f, joints, pose, frame):
    for hip, knee, eff, tgt, a, b in ik_chains(f, joints):
        if tgt not in joints: continue
        W1 = joints[hip]
        a = a or np.linalg.norm(joints[knee][:3, 3] - W1[:3, 3])
        b = b or np.linalg.norm(joints[eff][:3, 3] - joints[knee][:3, 3])
        H, T = W1[:3, 3], joints[tgt][:3, 3]
        d = T - H; dist = np.linalg.norm(d); u = d / dist
        dist = min(max(dist, abs(a - b) + 1e-4), a + b - 1e-4)
        # the bend's direction: where the knee points in the joint's own pose (its local +X bent about +Z)
        K0 = W1[:3, 3] + W1[:3, :3] @ np.array([a, 0, 0])
        v = (K0 - H) - np.dot(K0 - H, u) * u
        if np.linalg.norm(v) < 1e-5: v = W1[:3, 1] - np.dot(W1[:3, 1], u) * u
        v /= np.linalg.norm(v)
        ca = (a * a + dist * dist - b * b) / (2 * a * dist)
        K = H + a * (ca * u + math.sqrt(max(0.0, 1 - ca * ca)) * v)
        E = H + u * dist
        x1 = (K - H) / a; x2 = (E - K) / b
        z = np.cross(x1, x2)
        z = z / np.linalg.norm(z) if np.linalg.norm(z) > 1e-6 else W1[:3, 2]
        if np.dot(z, W1[:3, 2]) < 0: z = -z
        M1 = np.eye(4); M1[:3, :3] = np.column_stack([x1, np.cross(z, x1), z]); M1[:3, 3] = H
        M2 = np.eye(4); M2[:3, :3] = np.column_stack([x2, np.cross(z, x2), z]); M2[:3, 3] = K
        joints[hip], joints[knee] = M1, M2
        # the knee's other children (the ankle) follow it; the effector sits at the reached point
        def redo(p, Wp):
            c = f.u32(p + 8)
            while c:
                if c == eff:
                    Me = M2.copy(); Me[:3, 3] = E; joints[c] = Me
                else:
                    joints[c] = Wp @ posed_srt(f, c, pose, frame); redo(c, joints[c])
                c = f.u32(c + 0xC)
        redo(knee, M2)

def load_posed(f, root, pose, frame):
    """hsd.load_model with the joints posed (and the legs' IK solved)."""
    old_srt, old_walk = hsd.srt, hsd.walk
    hsd.srt = lambda j, ff: posed_srt(ff, j, pose, frame)
    def walk(ff, j, parent, out, joints, path=(), single=False):
        old_walk(ff, j, parent, out, joints, path, single)
        if not path and not single: solve_ik(ff, joints, pose, frame)
    hsd.walk = walk
    try:
        return hsd.load_model(f, root)
    finally:
        hsd.srt, hsd.walk = old_srt, old_walk
