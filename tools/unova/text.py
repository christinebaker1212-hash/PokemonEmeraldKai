#!/usr/bin/env python3
"""UNOVA's words (docs/UNOVA_HANDOFF.md): Black's own message banks, decrypted as the game does (aletheia's
src/game/arm9/msgdata.c: MSGDATA_ReadString / MSGDATA_DecryptUnits). A bank: u16 languages, u16 count, u32 size,
u32 reserved, u32 each language's offset; there u32 the section's size, then count entries of (u32 offset from the
section, u16 length in units, u16 attributes); a string's units XORed with (index + 3) * 0x2983, the key turned left
3 bits a unit.

    python3 -I tools/unova/text.py ARC FILE        (every line of a/0/0/2 or a/0/0/3's file FILE)
    python3 -I tools/unova/text.py find WORDS      (which banks hold WORDS)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402

SYSTEM, STORY = 'a/0/0/2', 'a/0/0/3'


def decompress(units):
    """A 0xF100 line: 9-bit characters packed into the units (low bits first), 0x1FF ending it (msgdata.c)"""
    out, bits, acc = [], 0, 0
    for u in units[1:]:
        acc |= u << bits
        bits += 16
        while bits >= 9:
            c = acc & 0x1FF
            acc >>= 9
            bits -= 9
            if c == 0x1FF:
                return out
            out.append(c)
    return out


def decode(units):
    """Black's units as text: controls (0xF000 + params) as {ctrl:...}, 0xFFFE a new line"""
    if units and units[0] == 0xF100:
        units = decompress(units)
    out, i = [], 0
    while i < len(units):
        u = units[i]
        if u == 0xFFFF:
            break
        if u == 0xFFFE:
            out.append('\n')
        elif u == 0xF000 and i + 2 < len(units):
            kind, n = units[i + 1], units[i + 2]
            out.append('{%04X:%s}' % (kind, ','.join('%X' % v for v in units[i + 3:i + 3 + n])))
            i += 3 + n
            continue
        else:
            out.append(chr(u))
        i += 1
    return ''.join(out)


def bank(data):
    """Every line of a message bank (the first language), decrypted"""
    langs, count = struct.unpack_from('<HH', data, 0)
    sec = struct.unpack_from('<I', data, 12)[0]
    lines = []
    for i in range(count):
        off, length = struct.unpack_from('<IH', data, sec + 4 + 8 * i)
        units = list(struct.unpack_from('<%dH' % length, data, sec + off))
        key = ((i + 3) * 0x2983) & 0xFFFF
        for k in range(length):
            units[k] ^= key
            key = ((key & 0xE000) >> 13 | (key << 3)) & 0xFFFF
        lines.append(decode(units))
    return lines


def main():
    if sys.argv[1] == 'find':
        words = ' '.join(sys.argv[2:])
        for path in (SYSTEM, STORY):
            for n, f in enumerate(narc.arc(path)):
                for k, line in enumerate(bank(f)):
                    if words.lower() in line.lower():
                        print('%s %d #%d: %s' % (path, n, k, line.replace('\n', ' / ')[:120]))
        return
    path = {'system': SYSTEM, 'story': STORY}.get(sys.argv[1], sys.argv[1])
    for k, line in enumerate(bank(narc.arc(path)[int(sys.argv[2])])):
        print('%d: %s' % (k, line.replace('\n', ' / ')))


if __name__ == '__main__':
    main()
