#!/usr/bin/env python3
"""Quetzal's badge marts: `pokemart 0` (no list) sells one of nine lists by the region's badge count.

    python3 -I tools/quetzal_rip/badge_marts.py QUETZAL_ROM

Its SetShopItemsForSale (0x081527B4) takes NULL to mean the list at 0x09204148[badges]. Writes
quetzal_rip/data/badge_marts.json (lists 0-8 of item names); tools/quetzal_port/gen_badge_marts.py makes the game's.
"""
import json
import os
import struct
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))


def main():
    rom = open(sys.argv[1], 'rb').read()
    names = {i: n for i, n, _ in json.load(open(os.path.join(REPO, 'quetzal_rip/data/names.json')))['items']}
    lists = []
    for k in range(9):
        o = struct.unpack_from('<I', rom, 0x01204148 + 4 * k)[0] - 0x08000000
        items = []
        while True:
            v = struct.unpack_from('<H', rom, o)[0]
            o += 2
            if v == 0:
                break
            items.append(names.get(v, 'ITEM_%d' % v))
        lists.append(items)
    json.dump({'_note': 'pokemart 0: the list for the badge count (0-8)', 'lists': lists},
              open(os.path.join(REPO, 'quetzal_rip/data/badge_marts.json'), 'w'), indent=1)
    print('9 lists, %d to %d items' % (len(lists[0]), len(lists[8])))


if __name__ == '__main__':
    main()
