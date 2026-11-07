#!/usr/bin/env python3
"""Pokemon Colosseum's own tables, read from the disc's common_rel module (common.fsys, unpacked by tools/orre/fsys.py):

  floors(unpacked) -> the floor table: every room the game has, in its order: dict(room, area, kind, archive, file)
     196 records of 0x4C bytes (the Colosseum decomp: src/game/floor_data.c floorDataBiosGetPtr / GetArea; the table
     and its count are what common_rel.c's prolog publishes: .data + 0x8BCF4, count at .data + 0x14A670).
     area: the game's coarse area (1 the OUTSKIRT STAND's two rooms, 2 PHENAC CITY's...); archive: the FSYS group
     id, matched to the disc's archives by the id at each archive's header + 8.
  common_text(unpacked) -> {key: parts} of common_rel's message block (.data + 0x58044, the .msg layout: msg.py):
     the game's place names among them (3501-3527: AGATE VILLAGE ... REALGAMTWR LOBBY)

    python3 -I tools/orre/colo_data.py UNPACKED_DIR ISO_FILES_DIR     (prints both)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import msg  # noqa: E402

DATA_SECTION = 4                    # common_rel's .data
FLOOR_TABLE, FLOOR_COUNT, FLOOR_SIZE = 0x8BCF4, 0x14A670, 0x4C
TEXT_BLOCK = 0x58044
PLACE_NAMES = range(3501, 3528)     # the game's place names


def _data(unpacked):
    d = open(os.path.join(unpacked, "common", "common_rel.rel"), "rb").read()
    n, st = struct.unpack_from(">II", d, 0x0C)
    off, size = struct.unpack_from(">II", d, st + 8 * DATA_SECTION)
    off &= ~3
    return d[off:off + size]


def archive_ids(iso_files):
    """{FSYS group id: archive name} from the disc's archives (header + 8)."""
    out = {}
    for fn in os.listdir(iso_files):
        if fn.endswith(".fsys"):
            with open(os.path.join(iso_files, fn), "rb") as f:
                out[struct.unpack_from(">I", f.read(16), 8)[0]] = fn[:-5]
    return out


def floors(unpacked, iso_files=None):
    data = _data(unpacked)
    count = struct.unpack_from(">I", data, FLOOR_COUNT)[0]
    names = archive_ids(iso_files) if iso_files else {}
    out = []
    for i in range(count):
        r = FLOOR_TABLE + i * FLOOR_SIZE
        grp, res, room = struct.unpack_from(">III", data, r + 4)
        out.append({"index": i, "kind": data[r] >> 5, "area": data[r + 1], "archive": grp,
                    "file": names.get(grp), "room": room})
    return out


def common_text(unpacked):
    return msg.parse(_data(unpacked)[TEXT_BLOCK:])


def plain(parts):
    """A message's text without its controls."""
    return "".join(p for p in parts if isinstance(p, str))


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    t = common_text(sys.argv[1])
    print("place names:", ", ".join("%d %s" % (k, plain(t[k])) for k in PLACE_NAMES if k in t))
    for f in floors(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None):
        print("%3d room %3d area %2d kind %d %s" % (f["index"], f["room"], f["area"], f["kind"], f["file"]))
