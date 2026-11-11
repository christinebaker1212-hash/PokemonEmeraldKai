#!/usr/bin/env python3
"""UNOVA's events (docs/UNOVA_HANDOFF.md): a zone's event file (archive 0x7D, a/1/2/5, by its event data id), as
aletheia's src/game/ov010/eventdata_system.c EVENTDATA_SYSTEM_ReadEventFile reads it: u32 size, u8 counts x4 (bg events,
people, connections, position events), then the tables: bg events 0x14 (BgEventData: u16 script, type (2 hidden item),
dir, pos type (0 grid, 1 rail), grid x, z, y s32), people 0x24 (MmdlHeader: u16 id, object code, move code, event type,
event flag, event id (script), dir, params x3, s16 move limits x2, u32 pos type, u16 pos x4: x, z, y?), connections
0x14 (ConnectData: u16 zone, exit, u8 exit dir, exit type, u16 pos type, grid s16 x, y, z, u16 size x, z), position
events 0x14 (EventRec: u16 script, work value, work id, check type, pos type, u16 x, z, size x, size z, y, spare).
Grid positions are tiles of the zone's matrix.

    python3 -I tools/unova/events.py ZONE
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402
import zones as zr  # noqa: E402


def events(zone_id):
    z = zr.zones()[zone_id]
    d = narc.arc('a/1/2/5')[z['event']]
    nbg, nppl, ncon, npos = struct.unpack_from('<4B', d, 4)
    o = 8
    bgs, people, cons, poss = [], [], [], []
    for i in range(nbg):
        script, typ, dirs, pt, x, zz, y = struct.unpack_from('<4Hiii', d, o)
        bgs.append({'script': script, 'type': typ, 'dir': dirs, 'rail': pt, 'x': x, 'z': zz, 'y': y})
        o += 0x14
    for i in range(nppl):
        (oid, code, move, etype, flag, script, dirh, p0, p1, p2, lx, lz, pt, a, b, c, e) = struct.unpack_from(
            '<10Hhhi4H', d, o)
        people.append({'id': oid, 'gfx': code, 'move': move, 'type': etype, 'flag': flag, 'script': script, 'dir': dirh,
                       'params': (p0, p1, p2), 'limit': (lx, lz), 'rail': pt, 'x': a, 'z': b, 'y': c, 'w': e})
        o += 0x24
    for i in range(ncon):
        zone, exit_id, edir, etype, pt, x, y, zz, sx, sz = struct.unpack_from('<HHBBHhhhHH', d, o)
        cons.append({'zone': zone, 'exit': exit_id, 'dir': edir, 'type': etype, 'rail': pt, 'x': x, 'y': y, 'z': zz,
                     'w': sx, 'h': sz})
        o += 0x14
    for i in range(npos):
        script, value, work, check, pt, x, zz, sx, sz, y, _ = struct.unpack_from('<5H5hH', d, o)
        poss.append({'script': script, 'value': value, 'work': work, 'check': check, 'rail': pt, 'x': x, 'z': zz,
                     'w': sx, 'h': sz, 'y': y})
        o += 0x16
    return {'bg': bgs, 'people': people, 'connect': cons, 'pos': poss}


def main():
    ev = events(int(sys.argv[1]))
    for k in ('connect', 'people', 'bg', 'pos'):
        for e in ev[k]:
            print(k, e)


if __name__ == '__main__':
    main()
