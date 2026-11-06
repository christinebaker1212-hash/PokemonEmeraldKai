#!/usr/bin/env python3
"""POKéMON STADIUM's trainers and rentals from the user's own ROM (docs/STADIUM.md). The ROM is never committed or
shipped: keep it outside the repository (~/.cache/moemon/stadium).

The table of tables at 0x898000 (Data Crystal's ROM map): 62 blocks of 16 bytes (offset, length) after a 16-byte header.
A CPU block: u32 trainer count, then per trainer a 0x38-byte header (name at +0, class name at +0xC, sprite at +0x34,
POKéMON count at +0x37) and its POKéMON. A rental block: u32 count, then the POKéMON. A POKéMON is 84 bytes: the Gen 1
party structure with the dex number first (+0 dex, +2 HP, +4 level, +6/7 types, +9..12 moves, +13 OT ID, +18..27 stat
EXP, +28 DVs, +32..35 PP, +36 level, +38 max HP, +40 ATK, +42 DEF, +44 SPD, +46 SPC), then the nickname (11 bytes) and
the OT name (11 bytes).

    python3 tools/stadium/extract.py ROM.z64 OUT.json
"""
import json
import struct
import sys

TABLE = 0x898000
NAMES = [
    'cpu Petit Cup R1', 'cpu Pika Cup R1', 'cpu Prime Cup Poke Ball R1', 'cpu Prime Cup Great Ball R1',
    'cpu Prime Cup Ultra Ball R1', 'cpu Prime Cup Master Ball R1', 'cpu Poke Cup Poke Ball R1', 'cpu Poke Cup Great Ball R1',
    'cpu Poke Cup Poke Ball R1 (dup)', 'cpu Poke Cup Great Ball R1 (dup)', 'cpu Poke Cup Ultra Ball R1',
    'cpu Poke Cup Master Ball R1'] + ['cpu Gym Leader Castle Gym %d R1' % i for i in range(1, 9)] + [
    'cpu Gym Leader Castle Elite Four R1', 'cpu Gym Leader Castle Rival R1', 'cpu Mewtwo R1', 'rental Petit Cup',
    'rental Pika Cup', 'rental Prime Cup / Mewtwo R1', 'rental Poke Cup', 'rental Poke Cup (2)', 'rental Poke Cup (3)',
    'cpu Battle Now R1', 'rental Gym Leader Castle']
NAMES += [n.replace(' R1', ' R2') for n in NAMES]


def cstr(b):
    b = b.split(b'\0')[0]
    return b.decode('latin-1')


def mon(b):
    u16 = lambda o: struct.unpack('>H', b[o:o + 2])[0]
    return {
        'dex': b[0], 'hp': u16(2), 'level': b[4], 'types': [b[6], b[7]],
        'moves': [m for m in b[9:13] if m], 'otid': u16(13),
        'statexp': [u16(18 + 2 * i) for i in range(5)], 'dvs': u16(28),
        'pp': list(b[32:36]), 'stats': {'hp': u16(38), 'atk': u16(40), 'def': u16(42), 'spd': u16(44), 'spc': u16(46)},
        'nickname': cstr(b[48:59]), 'ot': cstr(b[59:70]),
    }


def main():
    d = open(sys.argv[1], 'rb').read()
    out = []
    for k in range(62):
        off, ln = struct.unpack('>II', d[TABLE + 16 + 16 * k:TABLE + 24 + 16 * k])
        blk = d[TABLE + off:TABLE + off + ln]
        n = struct.unpack('>I', blk[:4])[0]
        entry = {'index': k, 'name': NAMES[k], 'offset': off}
        if NAMES[k].startswith('rental'):
            entry['pokemon'] = [mon(blk[4 + 84 * i:4 + 84 * (i + 1)]) for i in range(n)]
        else:
            trainers, p = [], 4
            for t in range(n):
                h = blk[p:p + 0x38]
                cnt = h[0x37]
                tr = {'name': cstr(h[0:0xC]), 'class': cstr(h[0xC:0x20]), 'sprite': h[0x34], 'unknown': list(h[0x35:0x37]),
                      'pokemon': [mon(blk[p + 0x38 + 84 * i:p + 0x38 + 84 * (i + 1)]) for i in range(cnt)]}
                trainers.append(tr)
                p += 0x38 + 84 * cnt
            entry['trainers'] = trainers
        out.append(entry)
    json.dump(out, open(sys.argv[2], 'w'), indent=1)


if __name__ == '__main__':
    main()
