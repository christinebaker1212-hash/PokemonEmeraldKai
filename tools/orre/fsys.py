#!/usr/bin/env python3
"""Pokemon Colosseum's FSYS archives: list and unpack every member, named by its file type.

The archive layout and the LZSS are the game's own loader's, as the Colosseum decomp
(pkmn-colosseum, MIT: tools/fsys_extract.py, src/game/fsys/fsys_file.c) reads them; this is
a copy of its reader that keeps members of one name but different types apart.

  python3 tools/orre/fsys.py unpack DISC_FILES_DIR OUT_DIR   (every *.fsys -> OUT_DIR/<archive>/)
  python3 tools/orre/fsys.py ids DISC_FILES_DIR OUT_DIR      (only each archive's _ids.txt)
Each archive's folder also gets _ids.txt: "<member id, hex> <file>" a line, the ids the game's resources go by
(a menu sprite's texture, a destination's picture...), and _archive_id.txt: the archive's own id (the floor
table's group).
"""
import os
import struct
import sys

# Member file types (byte 2 of the member id), as the game's resource groups use them.
TYPE_EXT = {
    0x00: "bin", 0x02: "map", 0x04: "dat", 0x06: "col", 0x08: "snd", 0x0A: "msg", 0x0C: "flr",
    0x0E: "scd", 0x12: "tex", 0x14: "ptl", 0x18: "cam", 0x1C: "rel", 0x1E: "pkx", 0x20: "wzx",
    0x22: "bin22", 0x24: "bin24", 0x26: "bin26",
}
FLAG_PACKED = 0x80000000


def u32(d, o):
    return struct.unpack_from(">I", d, o)[0]


def c_string(d, o):
    e = d.index(b"\0", o)
    return d[o:e].decode("latin-1")


def members(d):
    """[(index, id, name, offset, size, packed_size, flags)] in archive order."""
    if d[:4] != b"FSYS":
        raise ValueError("not an FSYS archive")
    count = u32(d, 0x0C)
    tables = u32(d, 0x18)
    entries = u32(d, tables)
    out = []
    for i in range(count):
        e = u32(d, entries + i * 4)
        mid, data_off, size, flags = u32(d, e), u32(d, e + 4), u32(d, e + 8), u32(d, e + 0x0C)
        packed, name_off = u32(d, e + 0x14), u32(d, e + 0x24)
        out.append((i, mid, c_string(d, name_off), data_off, size, packed, flags))
    return out


def lzss(stream, out_size):
    """The game's LZSS (4096-byte window, 18-byte matches, ring start 0xFEE)."""
    ring = bytearray(b"\0" * 4096)
    r = 0xFEE
    out = bytearray()
    i = 0
    flags = 0
    n = len(stream)
    while len(out) < out_size and i < n:
        flags >>= 1
        if not flags & 0x100:
            flags = stream[i] | 0xFF00
            i += 1
            if i >= n:
                break
        if flags & 1:
            c = stream[i]
            i += 1
            out.append(c)
            ring[r] = c
            r = (r + 1) & 0xFFF
        else:
            if i + 1 >= n:
                break
            a, b = stream[i], stream[i + 1]
            i += 2
            pos = a | ((b & 0xF0) << 4)
            ln = (b & 0x0F) + 3
            for k in range(ln):
                c = ring[(pos + k) & 0xFFF]
                out.append(c)
                ring[r] = c
                r = (r + 1) & 0xFFF
    return bytes(out[:out_size])


def member_data(d, m):
    _, _, _, off, size, packed, flags = m
    if flags & FLAG_PACKED:
        # 'LZSS', the unpacked size, the packed size (header included), then the stream at 0x10
        raw = d[off:off + packed]
        if raw[:4] != b"LZSS":
            raise ValueError("packed member lacks an LZSS header")
        out = lzss(raw[0x10:u32(raw, 8)], size)
        if len(out) != size:
            raise ValueError("LZSS size mismatch")
        return out
    return d[off:off + size]


def unpack(src_dir, out_dir, ids_only=False):
    total = 0
    for fn in sorted(os.listdir(src_dir)):
        if not fn.endswith(".fsys"):
            continue
        d = open(os.path.join(src_dir, fn), "rb").read()
        dst = os.path.join(out_dir, fn[:-5])
        os.makedirs(dst, exist_ok=True)
        seen = set()
        ids = []
        for m in members(d):
            ext = TYPE_EXT.get((m[1] >> 8) & 0xFF, "t%02x" % ((m[1] >> 8) & 0xFF))
            name = "%s.%s" % (m[2], ext)
            if name in seen:
                name = "%s_%d.%s" % (m[2], m[0], ext)
            seen.add(name)
            ids.append("%08x %s" % (m[1], name))
            if not ids_only:
                with open(os.path.join(dst, name), "wb") as f:
                    f.write(member_data(d, m))
            total += 1
        with open(os.path.join(dst, "_ids.txt"), "w") as f:
            f.write("\n".join(ids) + "\n")
        with open(os.path.join(dst, "_archive_id.txt"), "w") as f:   # (the archive's own id: header + 8)
            f.write("%08x\n" % u32(d, 8))
    return total


if __name__ == "__main__":
    if len(sys.argv) == 4 and sys.argv[1] in ("unpack", "ids"):
        print(unpack(sys.argv[2], sys.argv[3], sys.argv[1] == "ids"), "members")
    else:
        print(__doc__)
        sys.exit(1)
