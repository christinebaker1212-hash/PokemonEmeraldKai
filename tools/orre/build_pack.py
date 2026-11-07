#!/usr/bin/env python3
"""ORRE's packs, ripped from Pokemon Colosseum's disc (unpacked by tools/orre/fsys.py), so the game needs no disc:

  orre/orre3d.pak    every map's model (all 174): meshes and textures for the 3D engine
  orre/orre_data.pak the rest the conversions read, as the game's own bytes: text (.msg), collision (.col), scripts
                     (.scd), floor data (.flr), camera shots (.cam), the map modules' tables (.rel), the character
                     and object models (.dat, HSD: for their sprites), and the music and sounds: the songs (.snd),
                     the MusyX projects (common/snd_*_proj / _pool / _sdir.bin) and their samples (sound/*.samp,
                     the disc's files/sound/, copied in by make_pack.sh), the world map's textures, index/ids.txt
                     (every member's resource id and file) and dol/menu_items.bin, dol/menu_sprites.bin,
                     dol/wm_desc_order.bin (main.dol's menu tables, as they are: copied in as sys/main.dol by make_pack.sh)

    python3 -I tools/orre/build_pack.py UNPACKED_DIR OUT_DIR

orre3d.pak (little endian): "ORZ3" u32 version, u32 textures, u32 maps, then
  textures: each u8 GX format, u8 TLUT format, u16 w, u16 h, u16 TLUT entries, u32 offset, u32 bytes (from the
            data start: the texels in the disc's own GX encoding, then the TLUT; src decodes them as tools/orre/gxtex.py)
  maps:     each char name[32] (the map file's), u32 offset, u32 deflated bytes, u32 bytes; the body zlib'd:
            u32 sets; each: u8 kind (0 the place, 2 the sky), u8 pad x3, u32 meshes; each mesh:
              s32 texture (-1 none), u32 render mode (HSD MObj), u8 flags (1 see-through, 2 cull back, 4 cull front,
              8 vertex colours), u8 wrap S, T (0 clamp, 1 repeat, 2 mirror), u8 pad, f32 uv scale S, T,
              u8 diffuse r g b a, f32 alpha, u32 vertices, u32 indices;
              vertices: f32 x y z, f32 u v, u8 r g b a, s8 nx ny nz, u8 pad (28 bytes); indices u16, or u32 past 65535
              vertices (triangles; Colosseum's units, Y up)
            (version 2) u32 grid (0: none; 1: the map is in the game, tools/orre/maps.py): u16 w, h (tiles, whole
              blocks of 32), f32 tile (Colosseum units a tile), x0, z0 (the grid's corner); then per tile (row by row):
              u8 flags (1 walkable, 2 a trigger), s8 the trigger's collision id (the game's event id: -1 none), u8 heights (0-3),
              u8 the tile's behavior (the field engine's: 0 plain, 0x60 a warp stepped on: tools/orre/maps.py warps),
              (2 too: the grid of every field room, all 121)
              f32 heights x3 (the ground's, lowest first; Colosseum units) (tools/orre/grid.py)
  then the data: textures one after another.
orre_data.pak: "ORDT" u32 version, u32 files, each char name[48] ("<archive>/<member>.<type>"), u32 offset, u32 bytes,
  u32 deflated bytes (0: stored); offsets from the data start (files that are the same share one copy).
"""
import hashlib
import os
import struct
import sys
import zlib

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hsd  # noqa: E402
import grid  # noqa: E402
import maps as orre_maps  # noqa: E402

VERSION = 2
DATA_TYPES = ("msg", "col", "scd", "flr", "cam", "rel", "dat", "snd", "samp")


TEX_ARCHIVES = ("world_map",)   # archives whose textures (.tex) are kept: the screens ORRE draws from them


def is_data(arc, fn):
    """A file orre_data.pak keeps: the types above, the sound projects (common's snd_*_proj / _pool / _sdir), and the
    textures of TEX_ARCHIVES."""
    ext = fn.rsplit(".", 1)[-1]
    return (ext in DATA_TYPES or (ext == "bin" and fn.startswith("snd_"))
            or (ext == "tex" and arc in TEX_ARCHIVES))


# main.dol's menu tables (the Colosseum decomp: src/game/menu/menu_bios_range_8005D7F8.c): (address, record size,
# the address of their count): every menu's items (position, size, two sprites, the next item) and sprites (a
# rectangle on a texture, by its resource id); a count under 0x80000000 is the count itself: the world map's order of
# its places' descriptions (lbl_802E4F58: src/game/gs_title.c wmapDrawInfo)
DOL_TABLES = {"menu_items": (0x802EF0A8, 0x1C, 0x80478968), "menu_sprites": (0x802E7CE8, 0x18, 0x80478908),
              "wm_desc_order": (0x802E4F58, 1, 9)}


def dol_tables(path):
    d = open(path, "rb").read()
    offs, addrs, sizes = (struct.unpack_from(">18I", d, o) for o in (0, 0x48, 0x90))

    def at(a):
        for o, ad, sz in zip(offs, addrs, sizes):
            if ad <= a < ad + sz:
                return o + a - ad
        raise KeyError(hex(a))
    out = {}
    for name, (addr, size, count) in DOL_TABLES.items():
        n = count if count < 0x80000000 else struct.unpack_from(">I", d, at(count))[0]
        out[name] = d[at(addr):at(addr) + n * size]
    return out


def mesh_bytes(m, tex_index):
    mat = m.material or {}
    rm = mat.get("rendermode", 0)
    flags = 0
    if rm & 0x40000000 or mat.get("alpha", 1.0) < 1.0:
        flags |= 1
    if m.flags & 0x8000:
        flags |= 2
    if m.flags & 0x4000:
        flags |= 4
    if m.has_clr:
        flags |= 8
    ws, wt = mat.get("wrap", (1, 1))
    us, vs = mat.get("uvscale", (1.0, 1.0))
    diff = mat.get("diffuse", (255, 255, 255, 255))
    # vertices, shared ones once
    nrm = np.clip(np.round(m.nrm * 127), -127, 127).astype(np.int8)
    rec = np.zeros(len(m.pos), dtype=[("p", "<f4", 3), ("uv", "<f4", 2), ("c", "u1", 4), ("n", "i1", 3), ("x", "u1")])
    rec["p"], rec["uv"], rec["c"], rec["n"] = m.pos, m.uv, m.col, nrm
    uniq, inv = np.unique(rec.view(np.void(28)), return_inverse=True)
    idx = inv.astype("<u4" if len(uniq) > 65535 else "<u2")
    head = struct.pack("<iIBBBBffBBBBfII", tex_index, rm, flags, ws & 3, wt & 3, 0, us, vs, *diff,
                       mat.get("alpha", 1.0), len(uniq), len(idx))
    return head + uniq.tobytes() + idx.tobytes()


def build(src, out):
    os.makedirs(out, exist_ok=True)
    map_conf = {m["file"]: m for m in orre_maps.load(src)}
    tex_keys = {}
    tex_list = []
    maps = []
    seen = {}
    for arc in sorted(os.listdir(src)):
        for fn in sorted(os.listdir(os.path.join(src, arc))):
            if not fn.endswith(".map"):
                continue
            data = open(os.path.join(src, arc, fn), "rb").read()
            h = hashlib.sha1(data).hexdigest()
            name = fn[:-4]
            if name in seen:
                if seen[name] != h:
                    print("note: %s/%s differs from the first %s; the first is kept" % (arc, fn, name))
                continue
            seen[name] = h
            f = hsd.HsdFile(data, decode_textures=False)
            sets = hsd.model_sets(f)
            remap = []
            for raw in f.raw_images:
                k = hashlib.sha1(repr(raw[:3]).encode() + raw[3] + bytes([raw[4]]) + raw[5]).digest()
                if k not in tex_keys:
                    tex_keys[k] = len(tex_list)
                    tex_list.append(raw)
                remap.append(tex_keys[k])
            body = [struct.pack("<I", len(sets))]
            for s in sets:
                kind = 2 if hsd.is_sky(s) else 0
                body.append(struct.pack("<B3xI", kind, len(s)))
                for m in s:
                    body.append(mesh_bytes(m, remap[m.tex] if 0 <= m.tex < len(remap) else -1))
            m_conf = map_conf.get(name)
            if m_conf:
                g = orre_maps.build_grid(src, m_conf)
                body.append(struct.pack("<IHHfff", 1, g["w"], g["h"], g["tile"], g["x0"], g["z0"]))
                for y in range(g["h"]):
                    for x in range(g["w"]):
                        hs = (g["heights"][y][x] + [0.0, 0.0, 0.0])[:3]
                        fl = (1 if g["walk"][y, x] else 0) | (2 if g["trigger"][y, x] >= 0 else 0)
                        tid = int(g["trigger"][y, x])
                        beh = orre_maps.STEP_WARP if g["walk"][y, x] and tid in m_conf.get("warps", {}) else 0
                        body.append(struct.pack("<BbBBfff", fl, tid, len(g["heights"][y][x][:3]), beh, *hs))
            else:
                body.append(struct.pack("<I", 0))
            raw_body = b"".join(body)
            maps.append((name, zlib.compress(raw_body, 9), len(raw_body)))
    # orre3d.pak
    head = struct.pack("<4sIII", b"ORZ3", VERSION, len(tex_list), len(maps))
    tex_table, tex_data = [], []
    off = 0
    for fmt, w, h, texels, tfmt, lut in tex_list:
        blob = texels + lut
        tex_table.append(struct.pack("<BBHHHII", fmt, tfmt, w, h, len(lut) // 2, off, len(blob)))
        tex_data.append(blob)
        off += len(blob)
    map_table = []
    for name, z, size in maps:
        map_table.append(struct.pack("<32sIII", name.encode()[:31], off, len(z), size))
        tex_data.append(z)
        off += len(z)
    with open(os.path.join(out, "orre3d.pak"), "wb") as fo:
        fo.write(head + b"".join(tex_table) + b"".join(map_table) + b"".join(tex_data))
    # orre_data.pak
    files = []
    for arc in sorted(os.listdir(src)):
        for fn in sorted(os.listdir(os.path.join(src, arc))):
            if is_data(arc, fn):
                data = open(os.path.join(src, arc, fn), "rb").read()
                z = zlib.compress(data, 9)
                if len(z) >= len(data):
                    z = None
                files.append(("%s/%s" % (arc, fn), data, z))
    # every member's resource id (fsys.py's _ids.txt), and main.dol's menu tables
    ids = []
    for arc in sorted(os.listdir(src)):
        p = os.path.join(src, arc, "_ids.txt")
        if os.path.exists(p):
            ids += ["%s %s/%s" % (l.split()[0], arc, l.split()[1]) for l in open(p).read().split("\n") if l.strip()]
    if ids:
        data = ("\n".join(ids) + "\n").encode()
        files.append(("index/ids.txt", data, zlib.compress(data, 9)))
    dol = os.path.join(src, "sys", "main.dol")
    if os.path.exists(dol):
        for name, data in dol_tables(dol).items():
            files.append(("dol/%s.bin" % name, data, zlib.compress(data, 9)))
    table, blobs, off, stored = [], [], 0, {}
    for name, data, z in files:
        if len(name.encode()) > 47:
            raise SystemExit("name too long for the pack: " + name)
        blob = z if z is not None else data
        k = hashlib.sha1(data).digest()
        if k not in stored:   # one copy of a file every map archive carries (the same model in many places)
            stored[k] = off
            blobs.append(blob)
            off += len(blob)
        table.append(struct.pack("<48sIII", name.encode(), stored[k], len(data), len(z) if z is not None else 0))
    with open(os.path.join(out, "orre_data.pak"), "wb") as fo:
        fo.write(struct.pack("<4sII", b"ORDT", VERSION, len(files)) + b"".join(table) + b"".join(blobs))
    tri = sum(s for _, _, s in maps)
    print("%d maps, %d textures, %d data files" % (len(maps), len(tex_list), len(files)))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    build(sys.argv[1], sys.argv[2])
