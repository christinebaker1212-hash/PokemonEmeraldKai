#!/usr/bin/env python3
"""The SINNOH 3D pack: Platinum's map blocks, their props and textures, flattened for the game's 3D engine
(src/sinnoh/s3d.c reads it; docs/SINNOH_HANDOFF.md).

    python3 -I tools/sinnoh/build_pack.py POKEPLATINUM_DIR OUT.pak [X0 Z0 X1 Z1]
    python3 -I tools/sinnoh/build_pack.py --rom ROM POKEPLATINUM_DIR OUT.pak [...]   (the same from the cartridge)
        the cells of map matrix 0 from (X0, Z0) to (X1, Z1) (default: all of it)

POKEPLATINUM_DIR is a pret/pokeplatinum checkout (source.py): it holds every map block, texture set, prop model and
light set Platinum has, so no ROM is needed; the release builds the pack from it (tools/sinnoh/fetch_decomp.sh). Built
from the ROM instead, the pack is the same, byte for byte (tools/sinnoh/test_poc.sh checks it when given a ROM).

The file is "S3DZ", u32 the pack's size, then the pack deflated with zlib (src/sinnoh/s3d.c inflates it at load).
Format (little endian), version 3:
  "S3DP" u32 version
  u32 textures:  each u16 w, u16 h, then w*h u32 RGBA (alpha 0-255)
  u32 meshes:    each f32 centre x y z, f32 radius, u32 groups:
                   each group: s16 texture (-1 none), u8 repeat S, repeat T, flip S, flip T, alpha (0-31), polygon id,
                   cull (0 none, 1 back faces, 2 front faces, 3 all), lights (bits), mode, pad; u32 triangles;
                   then 3 vertices each: f32 x y z, f32 u v (texels), u8 r g b (0-31) pad, f32 nx ny nz
  u32 light sets: each u32 blocks: u32 end time (2-second units of the day), 4 lights (u8 on r g b, f32 dx dy dz),
                   then diffuse, ambient, specular, emission (u8 r g b each), u8 pad x 4
  u32 matrices: each u16 Platinum's matrix number (0: the region; others: buildings, tools/sinnoh/places.py), u16 width,
          height, u16 x0, z0 (the cells kept start here); per kept cell (row by row):
          s16 mesh (-1 none), s16 altitude, u16 light set, u16 props; props: s16 mesh, f32 3x4 transform (rows);
          then the cell's 32x32 map blocks (u16, the field engine's: metatile | collision << 10 | elevation << 12)
          from Platinum's terrain attributes (bit 15 collision, low byte the behavior: Platinum's behaviors share
          Emerald's numbers for the common ones; the metatile id is 0x300 + the behavior, BEHAVIOR below);
          then the ground's heights at each tile's centre (row by row): u8 count (0-3) per tile, then f32 heights x3
          per tile, lowest first (more than one: a bridge over the ground). From the block's BDHC (its plates: a
          rectangle in x, z around the block's centre and the plane the ground follows there); world heights.
  (version 4) u32 named meshes: each u16 Platinum's prop model number, s16 mesh: the models Platinum's gyms load at
          run time (tools/sinnoh/gymprops.py), each with its gym's textures
  (version 5) u32 animated textures: each u16 the texture, u16 frames, then per frame u16 the texture shown, u16 for
          how many of Platinum's field frames (30 a second); the map's water, shores, lamps... (Platinum's fldtanime:
          a texture's frames drawn with its own palette, swapped in turn)
  (version 6) u32 moving props: each u16 the prop's mesh, u16 frames, u16 Platinum frames each, then the frames' meshes
          (Platinum's looping joint animations, bm_anime: windmills, the steamship's screw...)
"""
import hashlib
import math
import re
import os
import struct
import zlib
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nitro  # noqa: E402
import gymprops  # noqa: E402
from platinum import Land  # noqa: E402
from source import open_source, NONE  # noqa: E402
from places import places, events  # noqa: E402

VERSION = 6

# A block's metatile id carries the tile's behavior: S3D_METATILE_BEHAVIOR (0x300, include/s3d.h) + the game's behavior
# (fieldmap.c GetMetatileAttributesById), so any of the game's behaviors can be used without a tileset that has it.
# Platinum's behaviors share the game's numbers for the common ones (grass, cave floors with encounters, water,
# waterfalls, ice, sand, the one-way tiles, ledges, forced walking, arrow warps, counters, PCs, the town map, TVs,
# bookshelves, shop shelves, trash cans); the rest are plain ground (0).
METATILE_BEHAVIOR = 0x300
SAME = {0x02, 0x03, 0x08, 0x0B, 0x0C, 0x10, 0x13, 0x15, 0x16, 0x17, 0x20, 0x21, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
        0x37, 0x38, 0x39, 0x3A, 0x3B, 0x40, 0x41, 0x42, 0x43, 0x62, 0x63, 0x64, 0x65, 0x80, 0x83, 0x85, 0x86, 0xE0,
        0xE1, 0xE2, 0xE4, 0xE5}
BEHAVIOR = {b: b for b in SAME}
BEHAVIOR.update({0x5A: 0x3A, 0x5B: 0x3B, 0x5C: 0x39, 0x5D: 0x38,   # (ledges jumped twice: as ledges)
                 0xEB: 0xE5, 0xEC: 0xE5})                           # (other shop shelves)
# Platinum's bridges (0x71-0x75: the deck; 0x70 where you get on, plain ground) as the game's unused 0x1D: Platinum keeps
# a walker on the deck, and so does s3d_gyms.c S3D_Gyms_StepBlocked (CYCLING ROAD's lanes over ROUTE 206's valley)
S3D_BRIDGE = 0x1D
BEHAVIOR.update({b: S3D_BRIDGE for b in range(0x71, 0x76)})
ROCK_CLIMB = (0x4B, 0x4C)      # walkable: this game has no HM restrictions (and no ROCK CLIMB of its own)
WATER = (0x10, 0x13, 0x15)     # surfable (0x13: a waterfall, climbed with WATERFALL)


def heights(land, ox=0, oz=0):
    """The ground's heights at each tile's centre from the block's BDHC: every plate whose rectangle holds the point
    gives the height of its plane there (x, z from the block's centre, 16 units a tile). Up to 3, lowest first: with more,
    the lowest and the two highest (a tunnel's surfaces under a bridge never push the bridge out). (OX,
    OZ: at that offset from each centre instead)"""
    b = land.bdhc
    out = [[] for _ in range(1024)]
    if len(b) < 16 or b[:4] != b'BDHC':
        return out
    npt, nnrm, ncon, npl = struct.unpack_from('<4H', b, 4)
    p = 16
    pts = [struct.unpack_from('<2i', b, p + 8 * i) for i in range(npt)]
    p += 8 * npt
    nrm = [struct.unpack_from('<3i', b, p + 12 * i) for i in range(nnrm)]
    p += 12 * nnrm
    con = [struct.unpack_from('<i', b, p + 4 * i)[0] for i in range(ncon)]
    p += 4 * ncon
    plates = []
    for i in range(npl):
        a, c, ni, ci = struct.unpack_from('<4H', b, p + 8 * i)
        if a >= npt or c >= npt or ni >= nnrm or ci >= ncon or nrm[ni][1] == 0:
            continue
        (ax, az), (cx, cz) = pts[a], pts[c]
        plates.append((min(ax, cx) / 4096, max(ax, cx) / 4096, min(az, cz) / 4096, max(az, cz) / 4096,
                       nrm[ni][0] / 4096, nrm[ni][1] / 4096, nrm[ni][2] / 4096, con[ci] / 4096))
    for tz in range(32):
        z = tz * 16 - 256 + 8 + oz
        for tx in range(32):
            x = tx * 16 - 256 + 8 + ox
            hs = []
            for x0, x1, z0, z1, nx, ny, nz, k in plates:
                if x0 <= x <= x1 and z0 <= z <= z1:
                    h = round(-(nx * x + nz * z + k) / ny, 2)
                    if all(abs(h - o) > 2 for o in hs):
                        hs.append(h)
            hs = sorted(hs)
            if len(hs) > 3:   # (the floor and the two highest: a bridge over a tunnel keeps its deck, CYCLING ROAD's)
                hs = [hs[0]] + hs[-2:]
            out[tz * 32 + tx] = hs
    return out


# Warp tiles (from every kept place's events): Platinum's entrance mats walked off southward, eastward or westward keep
# Emerald's arrow warps (0x65, 0x62, 0x63: the same numbers in both games); every other warp (doors, cave mouths,
# stairs, escalators...) warps when stepped on, as Emerald's non-animated door does, and the player walks on the way
# they were going (field_screen_effect.c SinnohArrivalDir)
ARROW_MATS = (0x62, 0x63, 0x65)   # MB_EAST / WEST / SOUTH_ARROW_WARP (Platinum's WARP_ENTRANCE_EAST / WEST / SOUTH)
STEP_WARP = 0x60       # MB_NON_ANIMATED_DOOR
# Platinum's stairs (0x5E WARP_STAIRS_EAST / 0x5F WARP_STAIRS_WEST: the tile beside the stairs, walked off east or west
# into them, as an arrow warp) and escalators (0x6A / 0x6B) by where they lead: up or down a floor (the maps' floor
# names). Stairs: Emerald's unused 0x5C / 0x5D (up, east / west) and 0x5E / 0x5F (down, east / west), arrow warps
# (metatile_behavior.c MetatileBehavior_IsSinnohStairs) with a step up or down on the way (field_screen_effect.c);
# escalators: Emerald's MB_UP_ESCALATOR / MB_DOWN_ESCALATOR
STAIRS = {(0x5E, True): 0x5C, (0x5F, True): 0x5D, (0x5E, False): 0x5E, (0x5F, False): 0x5F}
ESCALATOR_UP, ESCALATOR_DOWN = 0x6A, 0x6B


def floor_of(header):
    m = re.search(r'_(B?)(\d+)F(?:_|$)', header)
    if m:
        return -int(m.group(2)) if m.group(1) else int(m.group(2))
    return 9 if 'ROOFTOP' in header or 'ROOF' in header else 1


def warp_behavior(b, header, dest):
    """The behavior a warp tile gets (Platinum's B, on HEADER's map, leading to DEST's)"""
    up = floor_of(dest) > floor_of(header)
    if b in ARROW_MATS:
        return b
    if b in (0x5E, 0x5F):
        return STAIRS[(b, up)]
    if b in (0x6A, 0x6B):
        return ESCALATOR_UP if up else ESCALATOR_DOWN
    return STEP_WARP


def blocks(land, warps=None):
    """warps: {tile index: Platinum behavior} of the warp tiles in this block"""
    out = []
    for i, a in enumerate(land.attrs):
        if warps and i in warps:
            out.append((METATILE_BEHAVIOR + warps[i]) | (3 << 12))
            continue
        b = a & 0xFF
        mt = METATILE_BEHAVIOR + BEHAVIOR.get(b, 0)
        if b in WATER:
            out.append(mt | (1 << 12))                       # surfable: no collision, elevation 1
        elif b in ROCK_CLIMB:
            out.append(mt | (3 << 12))
        else:
            out.append(mt | ((1 if a & 0x8000 else 0) << 10) | (3 << 12))
    return out


class Pack:
    def __init__(self, tex_anims=None):
        self.textures = []
        self.tex_ids = {}
        self.meshes = []
        self.mesh_ids = {}
        self.tex_anims = tex_anims or {}   # texture name: (Tex0 with its frames, [(frame index, duration)...])
        self.anims = []                    # (texture, [(frame texture, duration)...])
        self.anim_skipped = set()          # (a map texture of that name another size than the frames: not Platinum's)
        self.mesh_anims = {}               # a looping prop's mesh: [its frames' meshes] (Platinum's bm_anime, NSBCA)

    def texture(self, tex0, mat):
        if tex0 is None or not mat.texture or mat.texture not in tex0.textures:
            return -1
        if not hasattr(tex0, 'digest'):   # (by content: an object's id() is reused once it's freed)
            tex0.digest = hashlib.sha1(tex0.d[tex0.o:]).digest()
        key = (tex0.digest, mat.texture, mat.palette)
        if key not in self.tex_ids:
            try:
                w, h, px = tex0.decode(mat.texture, mat.palette)
            except KeyError as e:   # (a palette the set doesn't have: that surface is drawn with its colors only)
                print('texture %s: no palette %s in its set; untextured' % (mat.texture, e))
                return -1
            self.tex_ids[key] = len(self.textures)
            self.textures.append((w, h, px))
            if mat.texture in self.tex_anims:
                self.animate(tex0, mat, self.tex_ids[key], w, h)
        return self.tex_ids[key]

    def animate(self, tex0, mat, base, w, h):
        frames_tex, steps = self.tex_anims[mat.texture]
        ids = {}
        out = []
        for idx, dur in steps:
            if idx >= len(frames_tex.order):
                return
            if idx not in ids:
                name = frames_tex.order[idx]
                if tex_bytes(frames_tex.textures[name]) != tex_bytes(tex0.textures[mat.texture]):
                    self.anim_skipped.add(mat.texture)
                    return
                # (Platinum copies the frame's bytes over the map texture's as they are: read with the map texture's
                # size and kind, whatever the frame's own header says)
                try:
                    fw, fh, fpx = tex0.decode(mat.texture, mat.palette, frame=(frames_tex, name))
                except (IndexError, KeyError, struct.error):
                    print('animated texture %s: frame %s is short; still' % (mat.texture, name))
                    return
                ids[idx] = len(self.textures)
                self.textures.append((fw, fh, fpx))
            out.append((ids[idx], max(1, dur)))
        if out:
            self.anims.append((base, out))

    def mesh(self, key, bmd, tex0, anim=None, frame=0):
        if key in self.mesh_ids:
            return self.mesh_ids[key]
        groups = []
        pts = []
        for model in bmd.models:
            mats = anim.node_matrices(model, frame) if anim is not None else None
            for mi, tris in sorted(model.draw(mats).items()):
                mat = model.materials[mi]
                cull = {'none': 0, 'back': 1, 'front': 2, 'all': 3}[mat.cull]
                rs, rt, fs, ft = mat.repeat
                groups.append((self.texture(tex0, mat), rs, rt, fs, ft, mat.alpha, (mat.poly_attr >> 24) & 63, cull,
                               mat.lights, mat.mode, tris))
                pts.extend(v[0] for t in tris for v in t)
        if pts:
            c = [sum(p[i] for p in pts) / len(pts) for i in range(3)]
            r = max(math.dist(c, p) for p in pts)
        else:
            c, r = [0.0, 0.0, 0.0], 0.0
        self.mesh_ids[key] = len(self.meshes)
        self.meshes.append((c, r, groups))
        return self.mesh_ids[key]


PROP_FRAME_STEP = 2      # a prop's frames drawn ahead, every second one of Platinum's (15 a second)
PROP_MAX_FRAMES = 48


def prop_frames(src, pack, model, prop_set, bmd, ptex):
    """A prop that moves all the time in Platinum (its joint animation, NSBCA, loaded with the field: bm_anime_list):
    (its meshes frame by frame, Platinum's frames each); ([], 0) for one that doesn't"""
    got = src.prop_anims(model) if hasattr(src, 'prop_anims') else None
    if not got or not got[0]:
        return ([], 0)
    for data in got[1]:
        if data[:4] != b'BCA0':
            continue
        anims = nitro.Bca0(data).anims
        if not anims or anims[0].frames < 2:
            continue
        a = anims[0]
        step = max(PROP_FRAME_STEP, (a.frames + PROP_MAX_FRAMES - 1) // PROP_MAX_FRAMES)
        try:
            return ([pack.mesh(('prop', model, prop_set, 'frame', f), bmd, ptex, a, f) for f in range(0, a.frames, step)],
                    step)
        except Exception as e:   # (an animation this reader can't follow: the prop stays still)
            print('prop %d: its animation left out (%s)' % (model, e))
            return ([], 0)
    return ([], 0)


def tex_bytes(t):
    bits = {1: 8, 2: 2, 3: 4, 4: 8, 5: 2, 6: 8, 7: 16}.get(t.format, 0)
    return t.width * t.height * bits // 8


def tex_anims(src):
    """Platinum's animated map textures (fldtanime.narc): {name: (Tex0 of its frames, [(frame, duration)...])}"""
    narc = src.tex_anims()
    if narc is None:
        print('no fldtanime (fetch_decomp.sh PARTS 8): the water stands still')
        return {}
    d = narc[0]
    count, = struct.unpack_from('<I', d, 0)
    out = {}
    for i in range(count):
        o = 4 + 52 * i
        name = d[o:o + 16].split(b'\0')[0].decode('latin-1')
        steps = []
        for k in range(18):
            idx, dur = d[o + 16 + 2 * k], d[o + 17 + 2 * k]
            if idx == 0xFF:
                break
            steps.append((idx, dur))
        tex = nitro.btx0(narc[i + 1])
        if tex is not None and steps:
            out[name] = (tex, steps)
    return out


def write(path, pack, light_sets, matrices, named=()):
    out = bytearray(b'S3DP' + struct.pack('<I', VERSION))
    out += struct.pack('<I', len(pack.textures))
    for w, h, px in pack.textures:
        out += struct.pack('<HH', w, h)
        out += b''.join(struct.pack('<BBBB', *p) for p in px)
    out += struct.pack('<I', len(pack.meshes))
    for c, r, groups in pack.meshes:
        out += struct.pack('<4fI', c[0], c[1], c[2], r, len(groups))
        for tex, rs, rt, fs, ft, alpha, pid, cull, lights, mode, tris in groups:
            out += struct.pack('<hBBBBBBBBBBI', tex, rs, rt, fs, ft, alpha, pid, cull, lights, mode, 0, len(tris))
            for tri in tris:
                for pos, uv, col, nrm in tri:
                    out += struct.pack('<5f4B3f', pos[0], pos[1], pos[2], uv[0], uv[1], col[0], col[1], col[2], 0,
                                       nrm[0], nrm[1], nrm[2])
    out += struct.pack('<I', len(light_sets))
    for blocks in light_sets:
        out += struct.pack('<I', len(blocks))
        for t, lights, cols in blocks:
            out += struct.pack('<I', t)
            for on, lc, ld in lights:
                out += struct.pack('<4B3f', on, lc[0], lc[1], lc[2], ld[0], ld[1], ld[2])
            for col in cols:
                out += struct.pack('<3B', *col)
            out += b'\0' * 4
    out += struct.pack('<I', len(matrices))
    for number, (x0, z0, x1, z1), cells in matrices:
        out += struct.pack('<5H', number, x1 - x0 + 1, z1 - z0 + 1, x0, z0)
        for mesh, alt, light, props, blk, hts in cells:
            out += struct.pack('<hhHH', mesh, alt, light, len(props))
            for pm, m in props:
                out += struct.pack('<h12f', pm, *m)
            out += struct.pack('<1024H', *blk)
            out += bytes(len(h) for h in hts)
            for h in hts:
                out += struct.pack('<3f', *(h + [0.0] * (3 - len(h))))
    out += struct.pack('<I', len(named))
    for model, mesh in named:
        out += struct.pack('<Hh', model, mesh)
    out += struct.pack('<I', len(pack.anims))
    for base, steps in pack.anims:
        out += struct.pack('<HH', base, len(steps))
        for tex, dur in steps:
            out += struct.pack('<HH', tex, dur)
    moving = [(m, f, st) for m, (f, st) in sorted(pack.mesh_anims.items()) if f]
    out += struct.pack('<I', len(moving))
    for mesh, frames, st in moving:
        out += struct.pack('<HHH', mesh, len(frames), st)
        out += struct.pack('<%dH' % len(frames), *frames)
    # stored compressed: "S3DZ", u32 the size of the pack, then the pack deflated (zlib): 88 MB -> about 6
    packed = b'S3DZ' + struct.pack('<I', len(out)) + zlib.compress(bytes(out), 6)
    open(path, 'wb').write(packed)
    return len(packed)


def main():
    args = sys.argv[1:]
    rom = None
    if args[0] == '--rom':
        rom, args = args[1], args[2:]
    decomp = args[0]
    src = open_source(rom, decomp=decomp) if rom else open_source(decomp)
    out_path = args[1]
    mx = src.matrix(0)
    # matrix 0's cells from (X0, Z0) to (X1, Z1) (default: all of it), then every building's matrix
    window = [int(x) for x in args[2:6]] if len(args) >= 6 else [0, 0, mx.width - 1, mx.height - 1]
    pack = Pack(tex_anims(src))
    light_index = {}
    light_sets = []
    tex_cache = {}

    # every kept place's warp tiles: {(matrix, cell x, cell z): {tile index: behavior}}
    kept = places(src, decomp)
    warp_tiles = {}
    for p in kept:
        for w in events(decomp, p)['warp_events']:
            x, z = w['x'], w['z']
            mt = src.matrix(p['matrix'])
            if x // 32 >= mt.width or z // 32 >= mt.height or mt.cell(x // 32, z // 32)[0] == NONE:
                continue
            land = Land(src.land(mt.cell(x // 32, z // 32)[0]))
            warp_tiles.setdefault((p['matrix'], x // 32, z // 32), {})[(z % 32) * 32 + x % 32] = \
                warp_behavior(land.attrs[(z % 32) * 32 + x % 32] & 0xFF, p['header'], w['dest_header_id'])

    def cell(lid, header, alt, where=None):
        if lid == NONE or header is None:
            return (-1, 0, 0, [], [0x0001 | (1 << 10) | (3 << 12)] * 1024, [[] for _ in range(1024)])
        prop_set, tex_set, light_set = src.area_of(header)
        if light_set not in light_index:
            light_index[light_set] = len(light_sets)
            light_sets.append(src.lights(light_set))
        if ('map', tex_set) not in tex_cache:
            tex_cache[('map', tex_set)] = nitro.btx0(src.map_textures(tex_set))
        if ('bm', prop_set) not in tex_cache:
            tex_cache[('bm', prop_set)] = nitro.btx0(src.prop_textures(prop_set))
        land = Land(src.land(lid))
        mesh = pack.mesh(('land', lid, tex_set), nitro.Bmd0(land.model), tex_cache[('map', tex_set)])
        props = []
        for pr in land.props:
            if pr.model >= src.prop_count():
                continue
            bmd = nitro.Bmd0(src.prop_model(pr.model))
            ptex = bmd.tex or tex_cache[('bm', prop_set)]
            pm = pack.mesh(('prop', pr.model, prop_set), bmd, ptex)
            if pm not in pack.mesh_anims:
                pack.mesh_anims[pm] = prop_frames(src, pack, pr.model, prop_set, bmd, ptex)
            ry = math.radians(pr.rot[1])
            cy, sy = math.cos(ry), math.sin(ry)
            sx, sy_, sz = pr.scale
            m = [cy * sx, 0.0, sy * sz, pr.pos[0],
                 0.0, sy_, 0.0, pr.pos[1],
                 -sy * sx, 0.0, cy * sz, pr.pos[2]]
            props.append((pm, m))
        return (mesh, alt, light_index[light_set], props, blocks(land, warp_tiles.get(where)), heights(land))

    matrices = []
    x0, z0, x1, z1 = window
    matrices.append((0, (x0, z0, x1, z1), [cell(*mx.cell(x, z), where=(0, x, z))
                                           for z in range(z0, z1 + 1) for x in range(x0, x1 + 1)]))
    # the buildings: each matrix once, its area from the first kept place that uses it (shared ones look alike)
    owner = {}
    for p in kept:
        if not p['outdoor'] and p['matrix'] not in owner:
            owner[p['matrix']] = p['header']
    for n in sorted(owner):
        m = src.matrix(n)
        cells = []
        for z in range(m.height):
            for x in range(m.width):
                lid, _, alt = m.cell(x, z)
                cells.append(cell(lid, owner[n], alt, where=(n, x, z)))
        matrices.append((n, (0, 0, m.width - 1, m.height - 1), cells))
    # the models the gyms load at run time, with their gym's textures
    named = []
    for header, names in gymprops.GYM_MODELS.items():
        prop_set = src.area_of(header)[0]
        if ('bm', prop_set) not in tex_cache:
            tex_cache[('bm', prop_set)] = nitro.btx0(src.prop_textures(prop_set))
        for name in names:
            model = gymprops.number(src, name)
            if name.startswith('fx:'):
                data = src.field_effect(name[3:])
                if data is None:
                    print('no %s (fetch_decomp.sh PARTS 6): left out' % name)
                    continue
                bmd = nitro.Bmd0(data)
            else:
                bmd = nitro.Bmd0(src.prop_model(model))
            named.append((model, pack.mesh(('prop', model, prop_set), bmd, bmd.tex or tex_cache[('bm', prop_set)])))
    size = write(out_path, pack, light_sets, matrices, named)
    if pack.anim_skipped:
        print('left still (another size than their frames): %s' % ', '.join(sorted(pack.anim_skipped)))
    print('%s: %d textures (%d animated), %d meshes, %d light sets, %d matrices, %d bytes' % (
        out_path, len(pack.textures), len(pack.anims), len(pack.meshes), len(light_sets), len(matrices), size))


if __name__ == '__main__':
    main()
