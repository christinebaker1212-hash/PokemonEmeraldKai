#!/usr/bin/env python3
"""UNOVA's zones (docs/UNOVA_HANDOFF.md): Black's zone headers (a/0/1/2 file 0, 0x30 bytes each: aletheia's
src/game/arm9/zonedata.c struct ZoneHeader) with their place names (a/0/0/2 bank 89).

    python3 -I tools/unova/zones.py            (every zone, a line each)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402
import text  # noqa: E402

PLACE_NAMES = 89


def zones():
    data = narc.arc('a/0/1/2')[0]
    out = []
    for i in range(len(data) // 0x30):
        r = data[i * 0x30:(i + 1) * 0x30]
        (map_type, sub, area, matrix, script, init_script, msg, b0, b1, b2, b3, encount, event, parent, pn, w,
         flags, cam_area, _pad, x, y, z) = struct.unpack('<BBHHHHH4HHHHHHHHHiii', r)
        out.append({'id': i, 'type': map_type, 'sub': sub, 'area': area, 'matrix': matrix, 'script': script,
                    'init_script': init_script, 'msg': msg, 'bgm': (b0, b1, b2, b3), 'encount': encount,
                    'event': event, 'parent': parent, 'place': pn & 0x3FF, 'weather': w & 0x3F,
                    'camera': (w >> 9) & 0x7F, 'flags': flags, 'cam_area': cam_area, 'start': (x, y, z)})
    return out


def main():
    names = text.bank(narc.arc(text.SYSTEM)[PLACE_NAMES])
    for z in zones():
        print('%3d %-24s type %d matrix %3d area %3d script %4d msg %4d event %3d encount %4d bgm %s' % (
            z['id'], names[z['place']] if z['place'] < len(names) else '?', z['type'], z['matrix'], z['area'],
            z['script'], z['msg'], z['event'], z['encount'], '/'.join(str(b) for b in z['bgm'])))


if __name__ == '__main__':
    main()
