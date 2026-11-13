#!/usr/bin/env python3
"""The UNOVA 3D pack (docs/UNOVA_HANDOFF.md): Black's map blocks, their buildings and textures (each season's),
flattened for UNOVA's own 3D engine (src/unova/u3d.c reads it). Read from the user's Black ROM unpacked by aletheia's
tools (tools/unova/narc.py: ~/.cache/moemon/unova/extract); nothing of it is in the repository.

    python3 -I tools/unova/build_pack.py OUT.pak [MATRIX...]      (default: every matrix a field zone uses)

The file is "U3DZ", u32 the pack's size, then the pack deflated with zlib.
Format (little endian), version 1: as SINNOH's pack (tools/sinnoh/build_pack.py, its texture, mesh and light layouts),
with Black's matrices:
  "U3DP" u32 version
  u32 textures:  each u16 w, u16 h, then w*h u32 RGBA (alpha 0-255)
  u32 meshes:    each f32 centre x y z, f32 radius, u32 groups: each s16 texture, u8 repeat S, T, flip S, T, alpha,
                 polygon id, cull, lights, mode, pad; u32 triangles; 3 vertices each: f32 x y z, f32 u v (texels),
                 u8 r g b (0-31) pad, f32 nx ny nz
  u32 light sets: each u32 blocks: u32 end time (2-second units), 4 lights (u8 on r g b, f32 dx dy dz), diffuse, ambient,
                 specular, emission (u8 r g b each), the clear colour (u8 r g b, 1); a set a Black light table and
                 season: table * 4 + season
  u32 matrices:  each u16 Black's matrix number, u16 width, height, u16 x0, z0, (version 2) u16 scale (Black's block
                 is scale x scale of these 512-unit cells: SKYARROW BRIDGE's 2048, its map type's; the block's meshes are
                 in the first of them, drawn at the block's middle); per cell (row by row): u16 zone (0xFFFF
                 none), s16 mesh x 4 (spring, summer, autumn, winter: -1 none; the same mesh where the seasons share
                 textures), u16 light table (| 0x8000: the area's own a season, the table + the season), u16 props: s16 mesh x 4 (seasons), f32 3x4 transform (rows); then the cell's
                 32x32 field blocks (u16: metatile | collision << 10 | elevation << 12, the metatile 0x300 + the
                 game's behavior), u8 count (0-2) and f32 heights x 2 per tile (the ground at its centre, lowest first:
                 a "GC" block's second grid is a bridge or a floor over it), (version 3) u8 1 if the cell has rail
                 cameras, then u16 pitch, yaw, distance per tile (0, 0, 0: the zone's own camera): Black's camera of
                 the rail the tile is on (tools/unova/rails.py camera_at), (version 6) u8 the cell's battle column
                 (0xFF: one per tile follows, 1024 bytes): Black's MAP_ATTR_GetBattleBackground of the tile's terrain,
                 the stage table's column of UNOVA's battle backgrounds (tools/unova/battle_bg.py)
"""
import importlib.util
import math
import os
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(os.path.dirname(HERE), 'sinnoh'))
import narc  # noqa: E402
import matrix as mxr  # noqa: E402
import terrain  # noqa: E402
import zones as zr  # noqa: E402
import nitro  # noqa: E402
import rails  # noqa: E402
import gen_gyms  # noqa: E402
import scrcmd  # noqa: E402
import events as evr  # noqa: E402
import gimmicks  # noqa: E402

_spec = importlib.util.spec_from_file_location('sinnoh_build_pack', os.path.join(os.path.dirname(HERE), 'sinnoh',
                                                                                 'build_pack.py'))
_sinnoh = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_sinnoh)
Pack = _sinnoh.Pack      # (texture and mesh tables, shared as a tool: never the games' engines)

VERSION = 6   # (4: Black's animated map textures, an area's ITP frames and ITA scrolling; 5: the gyms' machinery;
              #  6: each tile's battle background column)
ITA_ARC, ITP_ARC = 'a/0/6/9', 'a/0/7/0'   # (an area record's ita / itp: zonedata.c ZONEDATA_GetAreaItaAnimeId...)


def _align4(n):
    return (n + 3) & ~3


_itp_cache, _ita_cache = {}, {}


def itp_anims(itp_id):
    """An area's ITP (a/0/7/0): {map texture name: (Tex0 of its frames, [(frame texture, frame palette, duration in
    Black's field frames)...])}. A file: u32 count, then (u32 the animation's offset, u32 its BTX0's) each; an
    animation: u32 frames, u16 start times, u8 textures, u8 palettes (each run padded to 4), u32 loop, u32, u32 end"""
    if itp_id in _itp_cache:
        return _itp_cache[itp_id]
    out = {}
    files = narc.arc(ITP_ARC)
    if itp_id < len(files):
        f = files[itp_id]
        for k in range(struct.unpack_from('<I', f, 0)[0]):
            ao, bo = struct.unpack_from('<II', f, 4 + 8 * k)
            n = struct.unpack_from('<I', f, ao)[0]
            q = ao + 4
            times = struct.unpack_from('<%dH' % n, f, q)
            q += _align4(2 * n)
            texs = f[q:q + n]
            q += _align4(n)
            pals = f[q:q + n]
            q += _align4(n)
            end = struct.unpack_from('<3I', f, q)[2]
            t = nitro.btx0(f[bo:])
            if t is None or not t.order:
                continue
            steps = [(texs[i], pals[i], (times[i + 1] if i + 1 < n else end) - times[i]) for i in range(n)]
            out[t.order[0]] = out[t.order[0].rsplit('.', 1)[0]] = (t, steps)   # (the map's texture: the first frame's
                                                                                # name, 'hana01.1')
    _itp_cache[itp_id] = out
    return out


def ita_anims(ita_id):
    """An area's ITA (a/0/6/9, NSBTA): its texture SRT animations (nitro.SrtAnim: the water's scrolling)"""
    if ita_id not in _ita_cache:
        files = narc.arc(ITA_ARC)
        _ita_cache[ita_id] = nitro.Bta0(files[ita_id]).anims if ita_id < len(files) else []
    return _ita_cache[ita_id]


class UnovaPack(Pack):
    """SINNOH's tables, with Black's animated map textures: an ITP's frames (each with its own palette) and an ITA's
    scrolling (a group's srt: 1 + its index in srts, each [(translate s, translate t) a frame])"""
    def __init__(self):
        super().__init__()
        self.srts = []
        self.srt_ids = {}
        self.ita = (255, [])

    def animate(self, tex0, mat, base, w, h):
        frames_tex, steps = self.tex_anims[mat.texture]
        pals = list(frames_tex.palettes)
        ids, out = {}, []
        for ti, pi, dur in steps:
            if ti >= len(frames_tex.order):
                return
            if (ti, pi) not in ids:
                name = frames_tex.order[ti]
                try:
                    if mat.palette and pals:
                        fw, fh, fpx = frames_tex.decode(name, pals[min(pi, len(pals) - 1)])
                    else:
                        fw, fh, fpx = tex0.decode(mat.texture, mat.palette, frame=(frames_tex, name))
                except (IndexError, KeyError, struct.error):
                    print('animated texture %s: frame %s unreadable; still' % (mat.texture, name))
                    return
                if (fw, fh) != (w, h):
                    self.anim_skipped.add(mat.texture)
                    return
                ids[(ti, pi)] = len(self.textures)
                self.textures.append((fw, fh, fpx))
            out.append((ids[(ti, pi)], max(1, dur)))
        if out:
            self.anims.append((base, out))

    def srt_of(self, mat):
        ita_id, anims = self.ita
        for k, a in enumerate(anims):
            if mat.name in a.mats and a.frames > 1:
                key = (ita_id, k, mat.name)
                if key not in self.srt_ids:
                    self.srts.append([a.value(mat.name, f)[2:] for f in range(a.frames)])
                    self.srt_ids[key] = len(self.srts)
                return self.srt_ids[key]
        return 0

    def mesh(self, key, bmd, tex0, anim=None, frame=0, srt=None, matshp=None):
        if key in self.mesh_ids:
            return self.mesh_ids[key]
        groups, pts = [], []
        for model in bmd.models:
            for mi, tris in sorted(model.draw(None).items()):
                mat = model.materials[mi]
                cull = {'none': 0, 'back': 1, 'front': 2, 'all': 3}[mat.cull]
                rs, rt, fs, ft = mat.repeat
                groups.append((self.texture(tex0, mat), rs, rt, fs, ft, mat.alpha, (mat.poly_attr >> 24) & 63, cull,
                               mat.lights, mat.mode, tris, self.srt_of(mat)))
                pts.extend(v[0] for t in tris for v in t)
        if pts:
            c = [sum(p[i] for p in pts) / len(pts) for i in range(3)]
            r = max(math.dist(c, p) for p in pts)
        else:
            c, r = [0.0, 0.0, 0.0], 0.0
        self.mesh_ids[key] = len(self.meshes)
        self.meshes.append((c, r, groups))
        return self.mesh_ids[key]
SEASONS = 4
METATILE_BEHAVIOR = 0x300   # (include/u3d.h U3D_METATILE_BEHAVIOR; fieldmap.c GetMetatileAttributesById)
BLOCKED = 0x0001 | (1 << 10) | (3 << 12)

# Black's terrain ids (aletheia's map_attr_types.h) -> the game's behaviors
MB_TALL_GRASS, MB_LONG_GRASS, MB_DEEP_SAND, MB_CAVE, MB_POND_WATER, MB_DEEP_WATER = 0x02, 0x03, 0x06, 0x08, 0x10, 0x12
MB_WATERFALL, MB_PUDDLE, MB_SHALLOW_WATER, MB_ICE, MB_SAND, MB_COUNTER = 0x13, 0x16, 0x17, 0x20, 0x21, 0x80
MB_JUMP_EAST, MB_JUMP_WEST, MB_JUMP_NORTH, MB_JUMP_SOUTH, MB_FOOTPRINTS = 0x38, 0x39, 0x3A, 0x3B, 0x25
U3D_DARK_GRASS = 0x03       # (Black's "double" grass: two at once; u3d.c knows it by the tile's own terrain)
BEHAVIOR = {
    0x02: MB_PUDDLE,
    0x04: MB_TALL_GRASS, 0x05: MB_TALL_GRASS, 0x21: MB_TALL_GRASS,                 # grass
    0x06: U3D_DARK_GRASS, 0x07: U3D_DARK_GRASS, 0x22: U3D_DARK_GRASS,              # dark grass
    0x08: MB_TALL_GRASS, 0x09: U3D_DARK_GRASS,                                       # tall (seasonal) grass
    0x0A: MB_CAVE, 0x0B: MB_SAND, 0x0C: MB_SAND, 0x19: MB_SAND, 0x23: MB_SAND,
    0x0E: MB_FOOTPRINTS, 0x0F: MB_DEEP_SAND,                                         # snow, deep snow
    0x10: MB_ICE, 0x18: MB_ICE,
    0x14: MB_SHALLOW_WATER, 0x17: MB_SHALLOW_WATER, 0x1C: MB_SHALLOW_WATER,
    0x1E: MB_DEEP_SAND,                                                              # mud
    0x3D: MB_POND_WATER, 0x3E: MB_POND_WATER, 0x42: MB_POND_WATER,
    0x3F: MB_DEEP_WATER, 0x43: MB_DEEP_WATER, 0x40: MB_WATERFALL,
    0x72: MB_JUMP_EAST, 0x73: MB_JUMP_WEST, 0x74: MB_JUMP_NORTH, 0x75: MB_JUMP_SOUTH,
    0xBE: MB_DEEP_SAND, 0xBF: MB_DEEP_SAND,
    0xD4: MB_COUNTER,
}
WATER = {0x3D, 0x3E, 0x3F, 0x40, 0x42, 0x43}


_BATTLE_BG = None


def battle_column(attr):
    """Black's battle background column of a tile (ov021 map_attr.c MAP_ATTR_GetBattleBackground: its terrain id
    through the 37 pairs at 0x021D8EA0, 0 if none)"""
    global _BATTLE_BG
    if _BATTLE_BG is None:
        im = gimmicks.Image(21)
        _BATTLE_BG = {im.d[0x021D8EA0 - im.ram + 2 * i]: im.d[0x021D8EA0 - im.ram + 2 * i + 1] for i in range(0x25)}
    return _BATTLE_BG.get(attr & 0xFFFF, 0) if attr != 0x7FFFFFFF else 0


def field_block(attr):
    """A tile's field block: its behavior, blocked or not (Black's flag bit 0), surfable water at elevation 1"""
    terrain_id, flags = attr & 0xFFFF, attr >> 16
    if attr == 0x7FFFFFFF or terrain_id == 0xFF:
        return BLOCKED
    mt = METATILE_BEHAVIOR + BEHAVIOR.get(terrain_id, 0)
    if terrain_id in WATER:
        return mt | (1 << 12)
    return mt | ((flags & 1) << 10) | (3 << 12)


# Black's connections (tools/unova/events.py) as warp tiles: a door walked up into warps as it's stepped on (Emerald's
# non-animated door: the player walks out of it on arrival), a mat or a passage walked off down, right or left is an
# arrow warp (the same numbers in both games' meaning: MB_SOUTH / EAST / WEST_ARROW_WARP)
STEP_WARP = 0x60
ARROW_BY_DIR = {1: 0x65, 3: 0x62, 4: 0x63}
# a rail map's way out (a street's end into the next zone, a building's door on a rail): Black changes zone as the
# player walks over it, whichever way they come: warps as it's stepped on (u3d.h U3D_RAIL_WARP, field_control_avatar.c)
RAIL_WARP = 0x49


def warp_tiles(zone_ids):
    """{(matrix tile x, z): behavior} of the zones' connections"""
    out = {}
    for z in sorted(set(zone_ids)):
        try:
            cons = evr.events(z)['connect']
        except (IndexError, KeyError, struct.error):
            continue
        for c in cons:
            if c['rail']:
                continue
            b = ARROW_BY_DIR.get(c['dir'], STEP_WARP)
            for dz in range(max(1, c['h'])):
                for dx in range(max(1, c['w'])):
                    out[(c['x'] // 16 + dx, c['z'] // 16 + dz)] = b
    return out


def area(area_id, season=0):
    rec = area_id + season if 2 <= area_id < 0xD2 else area_id
    d = open(os.path.join(narc.EXTRACT, 'data', 'a/0/1/3'), 'rb').read()[rec * 10:rec * 10 + 10]
    model_set, tex_set, ita, itp, inner, light, edge, bbd = struct.unpack('<HHBBBBBB', d)
    return {'tex_set': tex_set, 'inner': inner, 'light': light, 'ita': ita, 'itp': itp}


def buildings(area_id, inner):
    """{table id: BMD0} and the textures of an area's buildings (field_buildmodel.c)"""
    member = area_id - 0xD2 if area_id >= 0xD2 else (area_id - 2) // 4 if area_id >= 2 else 0
    arc = narc.arc('a/2/2/9' if inner else 'a/2/3/0')
    texs = narc.arc('a/1/7/6' if inner else 'a/1/7/7')
    if member >= len(arc):
        return {}, None
    d = arc[member]
    n = d[2] // 2
    models = {}
    for i in range(n):
        o = struct.unpack_from('<I', d, 4 + 4 * i)[0]
        r = struct.unpack_from('<I', d, 4 + 4 * (n + i))[0]
        end = struct.unpack_from('<I', d, 4 + 4 * (n + i + 1))[0] if i + 1 < n else len(d)
        models[struct.unpack_from('<H', d, o)[0]] = d[r:end]
    return models, nitro.btx0(texs[member]) if member < len(texs) else None


def building_scrolls(area_id, inner):
    """{table id: [its texture scrollings a season (nitro.SrtAnim lists)]} of an area's buildings that scroll their
    textures all the time (field_buildmodel.c: a definition's +0x10, u8 kind, u8, u8, u8, then four u32 offsets from
    there, -1 none; kind 1 plays them all looping, kind 3 the season's own, kind 2 only when a script says: doors)"""
    member = area_id - 0xD2 if area_id >= 0xD2 else (area_id - 2) // 4 if area_id >= 2 else 0
    arc = narc.arc('a/2/2/9' if inner else 'a/2/3/0')
    if member >= len(arc):
        return {}
    d = arc[member]
    n = d[2] // 2
    out = {}
    for i in range(n):
        o = struct.unpack_from('<I', d, 4 + 4 * i)[0]
        ad = o + 0x10
        kind = d[ad]
        offs = struct.unpack_from('<4i', d, ad + 4)
        res = [d[ad + of:] if of != -1 else None for of in offs]

        def srts(r):
            if r is None or r[:4] != b'BTA0':
                return []
            try:
                return nitro.Bta0(r).anims
            except (IndexError, struct.error, AssertionError):
                return []
        if kind == 1:
            every = [a for r in res for a in srts(r)]
            seasons = [every] * SEASONS
        elif kind == 3:
            seasons = [srts(res[k]) for k in range(SEASONS)]
        else:
            continue
        if any(seasons):
            out[struct.unpack_from('<H', d, o)[0]] = seasons
    return out


def block_buildings(data):
    o = struct.unpack_from('<5I', data, 4)
    sec = data[o[2]:o[3]] if data[:2] == b'WB' else data[o[3]:o[4]] if data[:2] == b'GC' \
        else data[o[1]:o[2]] if data[:2] == b'NG' else b''   # (NG, a rail map's: the model, then the buildings)
    out = []
    for i in range(struct.unpack_from('<I', sec, 0)[0] if len(sec) >= 4 else 0):
        x, y, z, rot, hi, lo = struct.unpack_from('<iiiHBB', sec, 4 + 16 * i)
        out.append((x / 4096.0, y / 4096.0, -z / 4096.0, rot, (hi << 8) | lo))
    return out


# Black's lights (aletheia's ov021/field_light.c): archive 0x3D (a/0/6/1), a table per area (a season area's record
# for each season names its own), keyframes of 0x34 bytes: u16 time zone, s16 minutes after it, u8 lights on x 4, u16
# colours x 4, fx16 directions x 4 (x y z), u16 diffuse, ambient, specular, emission, fog, clear colours. A time zone's
# hour is the season's (TIMEZONE_GetLight: _TimeZoneLightTable, ARM9 0x0209DF10, 5 a season: morning, noon, evening,
# night, midnight). Black blends a second between keyframes; the pack keeps each keyframe until the next.
LIGHT_ARC = 'a/0/6/1'
TZ_LIGHT = 0x0209DF10


def rgb(c):
    return (c & 31, (c >> 5) & 31, (c >> 10) & 31)


def light_sets():
    """[blocks] for each table * 4 + season (the season the table's hours are read in): blocks as the pack keeps them:
    (end in 2-second units, 4 lights, the material's 4 colours, the clear colour)"""
    arm9 = open(os.path.join(os.path.dirname(narc.EXTRACT), 'arm9_dec.bin'), 'rb').read()
    tz = [arm9[TZ_LIGHT - 0x02004000 + 5 * s:TZ_LIGHT - 0x02004000 + 5 * s + 5] for s in range(4)]
    out = []
    for table in narc.arc(LIGHT_ARC):
        keys = []
        for i in range(len(table) // 0x34):
            k = struct.unpack_from('<Hh4B4H12h6H', table, i * 0x34)
            lights = []
            for j in range(4):
                d = k[10 + 3 * j:13 + 3 * j]
                n = math.sqrt(sum(v * v for v in d)) or 1.0
                lights.append((k[2 + j], rgb(k[6 + j]), tuple(v / n for v in d)))
            keys.append((k[0], k[1], lights, [rgb(c) for c in k[22:26]], rgb(k[27])))
        for season in range(4):
            timed = sorted(((tz[season][z] * 3600 + m * 60) % 86400, li, cols, clear)
                           for z, m, li, cols, clear in keys if z < 5)
            blocks = []
            if timed:
                # (from midnight to the first keyframe: the day's last)
                blocks.append((timed[0][0] // 2, timed[-1][1], timed[-1][2], timed[-1][3]))
                for i, (sec, li, cols, clear) in enumerate(timed):
                    end = timed[i + 1][0] if i + 1 < len(timed) else 86400
                    blocks.append((end // 2, li, cols, clear))
            out.append([b for b in blocks if b[0] > 0] or blocks[-1:])
    return out


def cell_light(area_id):
    """The cell's light: its season-0 table, 0x8000 when the area has a record a season (the table + the season)"""
    base = area(area_id)['light']
    return base | (0x8000 if 2 <= area_id < 0xD2 else 0)


def field_matrices():
    """The matrices the field zones use: {matrix: [zones]}"""
    out = {}
    for z in zr.zones():
        out.setdefault(z['matrix'], []).append(z['id'])
    return out


MAP_TYPES, MAP_TYPE_COUNT = 0x021D3700, 21   # (ov021's sMapTypeTable: 0x48 bytes a type, its block size first, fx32)


def scale_of(zone_ids):
    """how many 512-unit cells a side the matrix's blocks are: its zones' map type's block size (SKYARROW BRIDGE's
    type 1: 2048)"""
    ram, d = scrcmd.ov_image(21)
    k = 1
    for z in zone_ids:
        t = zr.zones()[z]['type']
        if t < MAP_TYPE_COUNT:
            k = max(k, struct.unpack_from('<I', d, MAP_TYPES - ram + 0x48 * t)[0] // (512 * 4096))
    return k


def lay_rails(grid, zone_ids, warps, cams):
    """Black's rails (tools/unova/rails.py) onto the cells' field blocks: each rail tile walkable at the rail's height
    (a cell made where a rail runs past the matrix's blocks); the rails' doors as warp tiles"""
    rail_warps = {}
    for z in sorted(set(zone_ids)):
        if z not in rails.rail_files():
            continue
        tiles = rails.cells(rails.rail_files()[z])
        for (tx, tz), (pitch, yaw, dist) in rails.tile_cameras(rails.rail_files()[z]).items():
            if tx >= 0 and tz >= 0:
                cams.setdefault((tx // 32, tz // 32), [None] * 1024)[(tz % 32) * 32 + tx % 32] = \
                    (pitch, yaw, min(65535, int(dist)))
        for c in evr.events(z)['connect']:
            if c['rail']:
                for t in rails.event_tiles(z, 'connect', c):
                    rail_warps[t] = RAIL_WARP
        gated = gen_gyms.gated_tiles() if z == gen_gyms.ICE_ZONE else {}
        for (tx, tz), hs in tiles.items():
            if tx < 0 or tz < 0:
                continue
            key = (tx // 32, tz // 32)
            if key not in grid:
                grid[key] = (z, [-1] * 4, 0, [], [BLOCKED] * 1024, [[] for _ in range(1024)],
                             [battle_column(0)] * 1024)   # (a rail map's ground: Black's plain terrain)
            cell = grid[key]
            t = (tz % 32) * 32 + tx % 32
            if (tx, tz) in gated:
                # (BRYCEN's ice slides: ridden when the switches let them (u3d_vm.c IceRide), never walked; the ice
                # below them as Black's grid has it, a blocked tile keeping the slide's height for the ride)
                if cell[4][t] == BLOCKED:
                    cell[5][t] = [round(gated[(tx, tz)], 2)]
                continue
            w = rail_warps.get((tx, tz), warps.get((tx, tz)))
            cell[4][t] = METATILE_BEHAVIOR + w | (3 << 12) if w is not None else field_block(0)
            cell[5][t] = sorted(round(h, 2) for h in hs)[:2]


REPLACE_ARC = 'a/0/1/0'   # (map replacement: ov010 map_replace.c, MAP_MATRIX_ApplyReplace)


def replace_records():
    """Black's map replacement records: (matrix, kind (0 a block swapped, 1 the matrix swapped), selector, ids)"""
    d = narc.arc(REPLACE_ARC)[0]
    return [(lambda r: (r[0], r[1], r[2], r[3:]))(struct.unpack_from('<HBB6H', d, 16 * i)) for i in range(len(d) // 16)]


def always_replaced(mn):
    """(the matrix to build MN from, {block: block}) by the replacements a game of Black always has: selector 1 (the
    field map's own, 1 but in one place) and 2 (no link: 1) pick ids[1]; selector 0 (a link partner's) ids[0]"""
    src, blocks = mn, {}
    for m, kind, sel, ids in replace_records():
        if m == mn and sel in (1, 2):
            if kind == 1:
                src = ids[1]
            else:
                blocks[ids[0]] = ids[1]
    return src, blocks


def flag_replacements():
    """[(matrix, its replacement)] of the replacements a story flag switches on (selectors 3-12: kind 1)"""
    return [(m, ids[1]) for m, kind, sel, ids in replace_records() if kind == 1 and sel >= 3]


def build(matrix_ids=None):
    zs = zr.zones()
    blocks = narc.arc('a/0/0/8')
    texsets = narc.arc('a/0/1/4')
    pack = UnovaPack()
    tex_cache, bld_cache = {}, {}
    mats = []
    wanted = matrix_ids if matrix_ids is not None else sorted(field_matrices())
    alternate_of = {alt: orig for orig, alt in flag_replacements() if orig in wanted}
    wanted = list(wanted) + sorted(a for a in alternate_of if a not in wanted)
    for mn in wanted:
        owners = field_matrices().get(alternate_of.get(mn, mn), [])
        src, swaps = always_replaced(mn)
        mx = mxr.matrix(src, owners[0] if owners else 0xFFFF)
        if src != mn and owners:   # (the replacement's blocks, the zones the matrix itself has)
            mx['zones'] = mxr.matrix(mn, owners[0])['zones']
        mx['blocks'] = [swaps.get(b, b) for b in mx['blocks']]
        cells = []
        used = [i for i in range(mx['w'] * mx['h']) if mx['blocks'][i] != mxr.NONE]
        if not used:
            continue
        x0, z0 = min(i % mx['w'] for i in used), min(i // mx['w'] for i in used)
        x1, z1 = max(i % mx['w'] for i in used), max(i // mx['w'] for i in used)
        warps = warp_tiles(owners + [mx['zones'][i] for i in used if mx['zones'][i] < len(zs)])
        for cz in range(z0, z1 + 1):
            for cx in range(x0, x1 + 1):
                i = cz * mx['w'] + cx
                b, zone = mx['blocks'][i], mx['zones'][i]
                if b == mxr.NONE or b >= len(blocks):
                    cells.append((0xFFFF, [-1] * 4, 0, [], [BLOCKED] * 1024, [[] for _ in range(1024)], 32, [0] * 1024))
                    continue
                zrec = zs[zone] if zone < len(zs) else (zs[owners[0]] if owners else None)
                area_id = zrec['area'] if zrec else 0
                data = blocks[b]
                bmd = nitro.Bmd0(data[struct.unpack_from('<I', data, 4)[0]:struct.unpack_from('<I', data, 8)[0]])
                meshes = []
                for season in range(SEASONS):
                    ar = area(area_id, season)
                    ts = ar['tex_set']
                    if ts not in tex_cache:
                        tex_cache[ts] = nitro.btx0(texsets[ts]) if ts < len(texsets) else None
                    pack.tex_anims = itp_anims(ar['itp']) if ar['itp'] != 255 else {}
                    pack.ita = (ar['ita'], ita_anims(ar['ita']) if ar['ita'] != 255 else [])
                    meshes.append(pack.mesh(('block', b, ts, ar['ita'], ar['itp']), bmd, tex_cache[ts]))
                pack.tex_anims, pack.ita = {}, (255, [])
                inner = area(area_id)['inner']
                if (area_id, inner) not in bld_cache:
                    bld_cache[(area_id, inner)] = buildings(area_id, inner)
                bmodels, btex = bld_cache[(area_id, inner)]
                if (area_id, inner, 'srt') not in bld_cache:
                    bld_cache[(area_id, inner, 'srt')] = building_scrolls(area_id, inner)
                bscroll = bld_cache[(area_id, inner, 'srt')]
                props = []
                for x, y, z, rot, tid in block_buildings(data):
                    if tid not in bmodels:
                        continue
                    pbmd = nitro.Bmd0(bmodels[tid])
                    pms = []
                    for season in range(SEASONS):   # (its scrolling textures: the falling petals of spring...)
                        sc = bscroll.get(tid, [[]] * SEASONS)[season]
                        pack.ita = (('bld', area_id, inner, tid, season if sc else -1), sc)
                        pms.append(pack.mesh(('bld', area_id, inner, tid, season if sc else -1), pbmd,
                                             pbmd.tex or btex))
                    pack.ita = (255, [])
                    a = rot / 65536.0 * 2 * math.pi
                    c, s = math.cos(a), math.sin(a)
                    props.append((pms, [c, 0.0, s, x, 0.0, 1.0, 0.0, y, -s, 0.0, c, z]))
                layers = terrain.layers(data)
                grids = [terrain.grid(layer) for layer in layers]
                # (a bigger block's grid is wider than 32: the LEAGUE's 64 x 64, laid over its k x k cells below)
                gw = struct.unpack_from('<H', layers[0], 0)[0] if layers else 32
                fb, hts, bgs = [], [], []
                for t in range(gw * gw):
                    if grids and t < len(grids[0]):
                        attr = grids[0][t][0]
                        if len(grids) > 1 and t < len(grids[1]):
                            a2 = grids[1][t][0]
                            if (attr >> 16) & 1 and not (a2 >> 16) & 1:
                                attr = a2   # (a floor over a blocked one: the bridge is the way)
                        w = warps.get((cx * gw + t % gw, cz * gw + t // gw))
                        if w is not None:   # (a warp: walkable, whatever Black's grid says of the tile)
                            fb.append(METATILE_BEHAVIOR + w | (field_block(attr) & (3 << 12) or 3 << 12))
                        else:
                            fb.append(field_block(attr))
                        hs = sorted({round(g[t][1], 2) for g in grids if t < len(g)})
                        hts.append(hs[:2])
                        bgs.append(battle_column(attr))
                    else:
                        fb.append(BLOCKED)
                        hts.append([])
                        bgs.append(0)
                cells.append((zone, meshes, cell_light(area_id), props, fb, hts, gw, bgs))
        k = scale_of(owners)
        grid = {}
        for i, c in enumerate(cells):
            cx, cz = x0 + i % (x1 - x0 + 1), z0 + i // (x1 - x0 + 1)
            gw = c[6]
            for sz in range(k):
                for sx in range(k):
                    if gw >= 32 * (max(sx, sz) + 1):   # (this 32 x 32 part of the block's own grid)
                        fb = [c[4][(sz * 32 + ty) * gw + sx * 32 + tx] for ty in range(32) for tx in range(32)]
                        hts = [c[5][(sz * 32 + ty) * gw + sx * 32 + tx] for ty in range(32) for tx in range(32)]
                        bgs = [c[7][(sz * 32 + ty) * gw + sx * 32 + tx] for ty in range(32) for tx in range(32)]
                    else:
                        fb, hts, bgs = [BLOCKED] * 1024, [[] for _ in range(1024)], [0] * 1024
                    first = sx == 0 and sz == 0
                    grid[(cx * k + sx, cz * k + sz)] = (c[0], c[1] if first else [-1] * 4, c[2],
                                                        c[3] if first else [], fb, hts, bgs)
        cams = {}
        lay_rails(grid, owners, warps, cams)
        gx, gz = [g[0] for g in grid], [g[1] for g in grid]
        x0, z0, x1, z1 = min(gx), min(gz), max(gx), max(gz)
        cells = [grid.get((cx, cz)) or (0xFFFF, [-1] * 4, 0, [], [BLOCKED] * 1024, [[] for _ in range(1024)], [0] * 1024)
                 for cz in range(z0, z1 + 1) for cx in range(x0, x1 + 1)]
        cell_cams = [cams.get((cx, cz)) for cz in range(z0, z1 + 1) for cx in range(x0, x1 + 1)]
        mats.append((mn, (x0, z0, x1, z1), cells, k, cell_cams))
        print('matrix %d: %dx%d cells' % (mn, x1 - x0 + 1, z1 - z0 + 1))
    return pack, light_sets(), mats


SPLIT = 192.0   # (a triangle longer than this is drawn as smaller ones: u3d.c drops one with a corner behind the eye,
                #  and a rail map's bridge decks and streets are a few long triangles)


def _mid(a, b):
    return tuple(tuple((x + y) // 2 if k == 2 else (x + y) / 2 for x, y in zip(a[k], b[k])) for k in range(4))


def split(tri):
    """TRI ((pos, uv, colour, normal) x 3) as triangles no side of which is longer than SPLIT"""
    lens = [math.dist(tri[i][0], tri[(i + 1) % 3][0]) for i in range(3)]
    i = max(range(3), key=lambda k: lens[k])
    if lens[i] <= SPLIT or len({v[2][3] for v in tri if len(v[2]) > 3}) > 1:
        return [tri]   # (... or one joining two of a gym model's parts: a middle point belongs to neither)
    a, b, c = tri[i], tri[(i + 1) % 3], tri[(i + 2) % 3]
    m = _mid(a, b)
    return split((a, m, c)) + split((m, b, c))


def write(path, pack, light_sets, mats, gims=([], [])):
    out = bytearray(b'U3DP' + struct.pack('<I', VERSION))
    out += struct.pack('<I', len(pack.textures))
    for w, h, px in pack.textures:
        out += struct.pack('<HH', w, h)
        out += b''.join(struct.pack('<BBBB', *p) for p in px)
    out += struct.pack('<I', len(pack.meshes))
    for c, r, groups in pack.meshes:
        out += struct.pack('<4fI', c[0], c[1], c[2], r, len(groups))
        for tex, rs, rt, fs, ft, alpha, pid, cull, lights, mode, tris, *srt in groups:
            tris = [t for tri in tris for t in split(tri)]
            out += struct.pack('<hBBBBBBBBBBIH', tex, rs, rt, fs, ft, alpha, pid, cull, lights, mode, 0, len(tris),
                               srt[0] if srt else 0)
            for tri in tris:
                for pos, uv, col, nrm in tri:
                    out += struct.pack('<5f4B3f', pos[0], pos[1], pos[2], uv[0], uv[1], col[0], col[1], col[2],
                                       col[3] if len(col) > 3 else 0, nrm[0], nrm[1], nrm[2])   # (a gym model's part)
    out += struct.pack('<I', len(light_sets))
    for blocks in light_sets:
        out += struct.pack('<I', len(blocks))
        for t, lights, cols, clear in blocks:
            out += struct.pack('<I', t)
            for on, lc, ld in lights:
                out += struct.pack('<4B3f', on, lc[0], lc[1], lc[2], ld[0], ld[1], ld[2])
            for col in cols:
                out += struct.pack('<3B', *col)
            out += struct.pack('<4B', clear[0], clear[1], clear[2], 1)
    out += struct.pack('<I', len(mats))
    for number, (x0, z0, x1, z1), cells, scale, cell_cams in mats:
        out += struct.pack('<6H', number, x1 - x0 + 1, z1 - z0 + 1, x0, z0, scale)
        for (zone, meshes, light, props, fb, hts, bgs), cam in zip(cells, cell_cams):
            out += struct.pack('<H4hHH', zone, *meshes, light, len(props))
            for pms, m in props:
                out += struct.pack('<4h12f', *pms, *m)
            out += struct.pack('<1024H', *fb)
            out += bytes(len(h) for h in hts)
            for h in hts:
                out += struct.pack('<2f', *(h + [0.0] * (2 - len(h))))
            out += bytes([1 if cam else 0])
            if cam:
                out += b''.join(struct.pack('<3H', *(c or (0, 0, 0))) for c in cam)
            out += bytes([bgs[0]]) if len(set(bgs)) == 1 else bytes([0xFF]) + bytes(bgs)   # (6)
    out += struct.pack('<I', len(pack.anims))   # (4: the animated textures, each texture's frames in turn)
    for base, steps in pack.anims:
        out += struct.pack('<HH', base, len(steps))
        out += b''.join(struct.pack('<HH', t, d) for t, d in steps)
    out += struct.pack('<I', len(pack.srts))    # (... and the scrolling: a texture's shift a frame, in its sizes)
    for frames in pack.srts:
        out += struct.pack('<H', len(frames))
        out += b''.join(struct.pack('<2f', s_, t_) for s_, t_ in frames)
    kinds, gimmick_list = gims   # (5: the gyms' machinery, tools/unova/gimmicks.py)
    out += struct.pack('<I', len(kinds))
    for mesh, nparts, anims in kinds:
        out += struct.pack('<HHh', nparts, len(anims), mesh)
        for n, frames in anims:
            out += struct.pack('<H', n)
            for row in frames:
                for m in row:
                    out += struct.pack('<12f', *m)
    out += struct.pack('<I', len(gimmick_list))
    for ov, models in gimmick_list:
        out += struct.pack('<BBH', ov, 0, len(models))
        for kind, pos, yaw, hidden in models:
            out += struct.pack('<H3fHBB', kind, pos[0], pos[1], pos[2], yaw, hidden, 0)
    packed = b'U3DZ' + struct.pack('<I', len(out)) + zlib.compress(bytes(out), 6)
    open(path, 'wb').write(packed)
    return len(packed)


def main():
    out = sys.argv[1]
    wanted = [int(a) for a in sys.argv[2:]] or None
    pack, lights, mats = build(wanted)
    gims = gimmicks.add(pack)
    size = write(out, pack, lights, mats, gims)
    print('%s: %d textures, %d meshes, %d matrices, %d bytes' % (out, len(pack.textures), len(pack.meshes), len(mats),
                                                                size))


if __name__ == '__main__':
    main()
