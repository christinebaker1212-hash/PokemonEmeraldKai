#!/usr/bin/env python3
"""UNOVA's scripts: Black's own script files, its shared script ranges, its zones' script / init / message / event ids
and its event files, as unova/black_scripts.pak beside the game (docs/UNOVA_HANDOFF.md "Black's words and scripts").
Made from the user's Black ROM when the release is built; nothing of it is in this repository. UNOVA's VM
(src/unova/u3d_vm.c) runs the scripts as Black does, its commands' arguments from tools/unova/scrcmd.py
(src/unova/u3d_cmds.h).

    python3 -I tools/unova/build_scripts.py OUT.pak

File: "U3SZ", u32 the size of the pack, the pack deflated (zlib). Pack (little endian): "U3DS" u32 version 1, then
  u32 script files: u32 offset, u32 size each (from the pack's start), the files (a/0/5/7)
  u32 ranges: u16 first id, last id, file, text arc, text file each (ov010's table)
  u32 zones: u16 script file, init file, message file, event file, matrix, parent zone, map type, place name, (version 4)
             the encounter id each
  u32 event files: u32 offset, u32 size each, the files (a/1/2/5)
  (version 2) the trainers (a/0/9/2: 20 bytes, u8 flags (1 moves, 2 items), class, battle kind (0 single, 1 double,
  2 triple, 3 rotation), count, u16 items x 4, u32 AI, u8 healer, cash, u16 prize item) and their teams (a/0/9/3: per
  POKéMON u8 strength (IVs 0-255), u8 Ability / gender, u8 level, u8 pad, u16 species, u16 form, [u16 item],
  [u16 moves x 4]), as two more file blocks
  (version 3) u32 trainers, u8 each: the song kind of its battle (song_kinds(): 0 a trainer, 1 a Gym Leader, 2 CHEREN
  or BIANCA, 3 Team Plasma, 4 the Elite Four, 5 the Champion, 6 N, 7 Ghetsis, 8 a Subway Boss), from its class and name
  (version 4) the wild POKéMON (a/1/2/6, by a zone's encounter id: one 0xE8-byte row, or four, a season each: u8 rates
  of grass, dark grass, rustling grass, surfing, rippling water, fishing, rippling fishing, pad; then 4-byte slots,
  u16 species (11 bits) and form (5), u8 lowest and highest level: grass 12, dark grass 12, rustling 12, surfing 5,
  rippling 5, fishing 5, rippling fishing 5), as a file block
  (version 5) the trainers' words (a/0/9/0: u16 trainer, u16 kind a record; a record's place is its message in
  system bank 189: TPOKE_DATA_ReadRecordString) as a file block, then the trainer classes' encounter songs (arm9's
  sTrainerClassBgmTable: u32 count, u16 class, u16 Black's sequence each; a class not in it: 1114)
  (version 6) Black's warp points (ov010's table: GAME_DATA_SetWarpId, where a whiteout returns): u32 count, 20 bytes
  each: u16 arrival flag, kind (1 a fly point, 2 its flag set on entering), zone inside, its tile (u8 x, z), zone
  outside, u16 x, z, then those three again
"""
import os
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import narc  # noqa: E402
import scripts as scr  # noqa: E402
import zones as zr  # noqa: E402

VERSION = 6
WARPS_AT, WARPS = 0x02170FEC, 17   # (version 6: ov010's warp points, MAP_REPLACE_ApplyWarpLocation)
TRMSG_ARC = 'a/0/9/0'   # (version 5: the trainers' words)
CLASS_BGM, ARM9_BASE = 0x020A75A8, 0x02004000
ENCOUNT_ARC = 'a/1/2/6'   # (version 4: the zones' wild POKéMON)
EVENT_ARC = 'a/1/2/5'
TRDATA_ARC, TRPOKE_ARC = 'a/0/9/2', 'a/0/9/3'   # (Black's trainers and their teams: version 2)


def files_block(files, base):
    """u32 count, (offset, size) each from the pack's start (BASE: where this block begins), the files"""
    head = struct.pack('<I', len(files))
    at = base + len(head) + 8 * len(files)
    table, body = bytearray(), bytearray()
    for f in files:
        table += struct.pack('<II', at + len(body), len(f))
        body += f
        body += b'\0' * (-len(body) % 4)
    return head + bytes(table) + bytes(body)


def song_kinds():
    """Each trainer's battle song kind, by Black's class and trainer names (banks 191, 190)"""
    import text as tx
    names = tx.bank(narc.arc(tx.SYSTEM)[190])
    classes = tx.bank(narc.arc(tx.SYSTEM)[191])
    out = bytearray()
    for i, t in enumerate(narc.arc(TRDATA_ARC)):
        cls = classes[t[1]] if len(t) > 1 and t[1] < len(classes) else ''
        name = names[i] if i < len(names) else ''
        kind = 0
        if cls == 'Leader':
            kind = 1
        elif name in ('Cheren', 'Bianca'):
            kind = 2
        elif name == 'N':
            kind = 6
        elif name == 'Ghetsis':
            kind = 7
        elif cls == 'Team Plasma':
            kind = 3
        elif cls == 'Elite Four':
            kind = 4
        elif cls == 'Champion':
            kind = 5
        elif cls == 'Subway Boss':
            kind = 8
        out.append(kind)
    return struct.pack('<I', len(out)) + bytes(out) + b'\0' * (-len(out) % 4)


def main():
    out_path = sys.argv[1]
    pack = bytearray(b'U3DS' + struct.pack('<I', VERSION))
    pack += files_block(narc.arc(scr.ARC), len(pack))
    rs = scr.ranges()
    pack += struct.pack('<I', len(rs)) + b''.join(struct.pack('<5H', *r) for r in rs)
    zs = zr.zones()
    pack += struct.pack('<I', len(zs))
    for z in zs:
        pack += struct.pack('<9H', z['script'], z['init_script'], z['msg'], z['event'], z['matrix'], z['parent'],
                            z['type'], z['place'], z['encount'])
    pack += b'\0' * (-len(pack) % 4)   # (the next block starts on 4 bytes)
    pack += files_block(narc.arc(EVENT_ARC), len(pack))
    pack += files_block(narc.arc(TRDATA_ARC), len(pack))
    pack += files_block(narc.arc(TRPOKE_ARC), len(pack))
    pack += song_kinds()
    pack += files_block(narc.arc(ENCOUNT_ARC), len(pack))
    pack += files_block(narc.arc(TRMSG_ARC), len(pack))
    arm = open(os.path.join(os.path.dirname(narc.EXTRACT), 'arm9_dec.bin'), 'rb').read()
    pairs, o = [], CLASS_BGM - ARM9_BASE
    while struct.unpack_from('<H', arm, o)[0] != 0x69:
        pairs.append(struct.unpack_from('<HH', arm, o))
        o += 4
    pack += struct.pack('<I', len(pairs)) + b''.join(struct.pack('<HH', *p) for p in pairs)
    ram, ov = scr.scrcmd.ov_image(10)
    pack += struct.pack('<I', WARPS) + ov[WARPS_AT - ram:WARPS_AT - ram + 20 * WARPS]
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    open(out_path, 'wb').write(b'U3SZ' + struct.pack('<I', len(pack)) + zlib.compress(bytes(pack), 6))
    print('%s: %d script files, %d zones, %d event files, %d bytes' % (out_path, len(narc.arc(scr.ARC)), len(zs),
                                                                       len(narc.arc(EVENT_ARC)), len(pack)))


if __name__ == '__main__':
    main()
