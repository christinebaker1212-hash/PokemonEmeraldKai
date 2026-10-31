#!/usr/bin/env python3
"""Disassemble one of Moemon Quetzal's specials (or any THUMB address) for porting it.

    python3 -I tools/quetzal_rip/dis_special.py QUETZAL_ROM s759        # special 759 (gSpecials at 0x082BEE04)
    python3 -I tools/quetzal_rip/dis_special.py QUETZAL_ROM 82ad754 90  # an address, 90 instructions

Needs capstone (pip). Literal-pool loads show their value (`; =0x...`). Useful addresses: gSaveBlock1Ptr 0x0300559C
(location at +0x474: group, num), gSaveBlock2Ptr 0x030055A0, gPlayerParty 0x020235CC (0x68 a Pokémon), the move
table 0x08542B64 (24 bytes a move), FlagGet 0x080E4800, FlagSet 0x080E4788, FlagClear 0x080E47D8, VarGet 0x080E45D8,
GetMonData 0x0809EAE8. Run with -I so nothing in the current folder is imported.
"""
import struct
import sys

import capstone

SPECIALS = 0x082BEE04


def main():
    rom = open(sys.argv[1], 'rb').read()
    rd32 = lambda a: struct.unpack_from('<I', rom, a - 0x08000000)[0]
    a = sys.argv[2]
    count = int(sys.argv[3]) if len(sys.argv) > 3 else 120
    if a.startswith('s'):
        addr = rd32(SPECIALS + 4 * int(a[1:]))
        print('special %s at %08X' % (a[1:], addr))
    else:
        addr = int(a, 16) | 0x08000000
    addr &= ~1
    md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_THUMB)
    code = rom[addr - 0x08000000:addr - 0x08000000 + count * 2]
    for i in md.disasm(code, addr):
        extra = ''
        if i.mnemonic == 'ldr' and 'pc' in i.op_str:
            off = int(i.op_str.split('#')[-1].rstrip(']'), 16) if '#' in i.op_str else 0
            extra = '  ; =0x%08X' % rd32(((i.address + 4) & ~3) + off)
        print('%08X: %-6s %s%s' % (i.address, i.mnemonic, i.op_str, extra))


if __name__ == '__main__':
    main()
