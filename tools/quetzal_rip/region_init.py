#!/usr/bin/env python3
"""Quetzal's first-arrival setup of JOHTO and KANTO: the flags (hidden people and objects), variables and its guard
flag that its code 0x082B4204 (region in r0; called at new game and on each region trip) sets once per region, and the
flag 0x082B41D8 sets on every arrival. Writes quetzal_rip/data/region_init.json; tools/quetzal_port/gen_region_init.py
makes the game's table.

    python3 -I tools/quetzal_rip/region_init.py QUETZAL_ROM     (needs capstone)
"""
import json
import os
import struct
import sys

import capstone

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
FLAGSET, VARSET = 0x080E4788, 0x080E4610


def ops(rom, start, end):
    md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_THUMB)
    rd32 = lambda a: struct.unpack_from('<I', rom, a - 0x08000000)[0]
    flags, vars_, regs, cur = [], [], {}, start
    while cur < end:
        got = False
        for i in md.disasm(rom[cur - 0x08000000:end - 0x08000000], cur):
            got, cur = True, i.address + i.size
            if i.mnemonic == 'ldr' and '[pc' in i.op_str:
                off = int(i.op_str.split('#')[-1].rstrip(']'), 16) if '#' in i.op_str else 0
                regs[i.op_str.split(',')[0]] = rd32(((i.address + 4) & ~3) + off)
            elif i.mnemonic in ('movs', 'mov') and '#' in i.op_str:
                regs[i.op_str.split(',')[0]] = int(i.op_str.split('#')[1], 16)
            elif i.mnemonic == 'bl':
                t = int(i.op_str.lstrip('#'), 16)
                if t == FLAGSET and 'r0' in regs:
                    flags.append(regs['r0'])
                elif t == VARSET:
                    vars_.append([regs['r0'], regs['r1']])
                regs = {}
        if not got:
            cur += 2
    return flags, vars_


def main():
    rom = open(sys.argv[1], 'rb').read()
    kf, kv = ops(rom, 0x082B4224, 0x082B4356)
    jf, jv = ops(rom, 0x082B4442, 0x082B45F0)
    out = {'_note': 'set once on a region\'s first arrival (guard flag set last); every arrival also sets "arrival"',
           'johto': {'guard': 0x1422, 'flags': jf, 'vars': jv, 'arrival': 0x11BA},
           'kanto': {'guard': 0x2269, 'flags': kf, 'vars': kv, 'arrival': 0x2245}}
    json.dump(out, open(os.path.join(REPO, 'quetzal_rip/data/region_init.json'), 'w'), indent=1)
    print('JOHTO %d flags %s, KANTO %d flags %s' % (len(jf), jv, len(kf), kv))


if __name__ == '__main__':
    main()
