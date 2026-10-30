#!/usr/bin/env python3
"""Moemon Quetzal's own multichoice menus into quetzal_rip/data/multichoice.json.

    python3 tools/quetzal_rip/multichoice.py QUETZAL.gba

Quetzal keeps Emerald's sMultichoiceLists (0x0920A5AC) and adds a second table at 0x0920AF0C ({list, count, pad},
107 entries). Its Kanto / Johto scripts name the second table's menus as ids 0x2C + n (the ferry's regions are
0x8D-0x93, the elevators 0x59 / 0x6E / 0x6F...): every multichoice id >= 0x2C in them is one of these.
"""
import json
import os
import sys

from qrom import Rom, REPO, decode, find_rom

TABLE = 0x120AF0C
COUNT = 107
FIRST_ID = 0x2C


def main():
    r = Rom(find_rom())
    out = []
    for i in range(COUNT):
        o = TABLE + i * 8
        lst, n = r.u32(o) - 0x08000000, r.u8(o + 4)
        out.append({'id': FIRST_ID + i, 'items': [decode(r, r.u32(lst + 8 * k) - 0x08000000, 64)[0] for k in range(n)]})
    path = os.path.join(REPO, 'quetzal_rip', 'data', 'multichoice.json')
    json.dump({'_note': 'Quetzal\'s second multichoice table: script id = 0x2C + index', 'first_id': FIRST_ID,
               'menus': out}, open(path, 'w'), indent=1, ensure_ascii=False)
    print('%d menus' % len(out))


if __name__ == '__main__':
    main()
