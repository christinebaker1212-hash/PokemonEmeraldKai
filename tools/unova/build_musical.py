#!/usr/bin/env python3
"""UNOVA's POKeMON MUSICALS (docs/UNOVA_HANDOFF.md, src/unova/u3d_musical.c): Black's own props, shows and performers,
read from the ROM as the pack is built (make_pack.sh; never committed).

    python3 -I tools/unova/build_musical.py unova/black_musical.pak

Pack "UMUS", u16 version, u16 props, u16 shows, then:
  props (a/0/4/5: 100 BTX0 textures, member 101 the 12-byte records, ov112 mus_item_data.c McssElemData):
    u8 width, height, position (0 ear, 1 head, 2 eyes, 3 face, 4 body, 5 waist, 6 hand), category (0 cool,
    1 cute, 2 elegant, 3 quirky: bank 75's 4-7), u16 the positions it may be worn at (a bit each, as position),
    s8 x, y (its offset from the body point; y up), then width x height u32 ARGB
  shows (a/1/1/1-4, ov112 musical_system.c: member 0 the program, member 1 its words):
    u8 bonus[4] (a category's share of the judges' points), u8 bonusTotal (dealt out at random), u8 propRow (the
    fans' reward row), u8 title length, title (UTF-8), u8 rivals, then each rival (an NPC performer, 0x2C bytes from
    0x0C; ov010 musical_program.c): u16 national, u8 portrait class, u8 name id (banks 76 / 77), u16 appealBase,
    u16 level required (fan points), 8 x {u16 prop (0x1FE / 0x1FF none), u8 body slot}
  fans (ov010 musical_event.c MUSICAL_EVENT_RollProgramRewards): u16 thresholds[10] (fan points: d_0216FF14), then
    7 rows of 40 props (d_0216FF28)
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'sinnoh'))
import narc  # noqa: E402
import nitro  # noqa: E402
import scrcmd  # noqa: E402
import text as tx  # noqa: E402

VERSION = 1
PROPS, PROP_ARC = 100, 'a/0/4/5'
SHOW_ARCS = ['a/1/1/1', 'a/1/1/2', 'a/1/1/3', 'a/1/1/4']
FAN_THRESHOLDS, FAN_ROWS = 0x0216FF14, 0x0216FF28


def props():
    arc = narc.arc(PROP_ARC)
    recs = bytes(arc[101])
    out = []
    for i in range(PROPS):
        t = nitro.btx0(bytes(arc[i]))
        w, h, px = t.decode(t.order[0], list(t.palettes)[0])
        _sf, ox, oy, bits, grp, cat, _anim, _ = struct.unpack_from('<IbbHBBBB', recs, i * 12)
        b = struct.pack('<BBBBHbb', w, h, grp, cat, bits & 0x1FF, ox, oy)
        for (r, g, bl, a) in px:
            b += struct.pack('<I', (a << 24) | (r << 16) | (g << 8) | bl)
        out.append(b)
    return out


def shows():
    out = []
    for path in SHOW_ARCS:
        arc = narc.arc(path)
        d = bytes(arc[0])
        title = tx.bank(bytes(arc[1]))[0].encode('utf-8')
        bonus = list(d[1:5])
        b = struct.pack('<4BBBB', *bonus, d[5], d[6], len(title)) + title
        rivals = []
        for i in range((len(d) - 0x0C) // 0x2C):
            o = 0x0C + i * 0x2C
            national, portrait, name_id = struct.unpack_from('<HBB', d, o)
            appeal, level = struct.unpack_from('<HH', d, o + 6)
            pr = b''
            for k in range(8):
                prop, slot = struct.unpack_from('<HB', d, o + 0x0C + 4 * k)
                pr += struct.pack('<HB', prop, slot)
            rivals.append(struct.pack('<HBBHH', national, portrait, name_id, appeal, level) + pr)
        b += struct.pack('<B', len(rivals)) + b''.join(rivals)
        out.append(b)
    return out


def fans():
    ram, d = scrcmd.ov_image(10)
    thr = d[FAN_THRESHOLDS - ram:FAN_THRESHOLDS - ram + 20]
    rows = d[FAN_ROWS - ram:FAN_ROWS - ram + 7 * 40]
    return thr + rows


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    p, s = props(), shows()
    out = b'UMUS' + struct.pack('<HHH', VERSION, len(p), len(s)) + b''.join(p) + b''.join(s) + fans()
    with open(sys.argv[1], 'wb') as f:
        f.write(out)
    print('%s: %d props, %d shows, %d bytes' % (sys.argv[1], len(p), len(s), len(out)))


if __name__ == '__main__':
    main()
