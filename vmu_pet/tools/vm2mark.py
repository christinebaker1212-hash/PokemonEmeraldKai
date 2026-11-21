#!/usr/bin/env python3
"""Moemon VMU pet: mark the pet in a VMU image as a VM2's (then its MENU > LINK is there: pet/link.inc vm2_check).

usage: vm2mark.py IMAGE [OUT]      (OUT defaults to IMAGE)

The mark is in the pet's SAVE page (the file's $BB80): $78-$7b "VM2M", $7c-$7f the image's own root block bytes
$33-$36 (when it was formatted), so a copy of the program on another VMU doesn't count as a VM2. Kai leaves the
same mark when a VM2 is connected to it (PC > VM2).
"""
import sys

SAVE_PAGE = 0xBB80


def mark(raw):
    raw = bytearray(raw)
    root = 255 * 512
    d = 253 * 512
    first = None
    for blk in range(253, 240, -1):
        for e in range(16):
            ent = raw[blk * 512 + e * 32: blk * 512 + e * 32 + 32]
            if ent[0] == 0xCC:
                first = ent[2] | ent[3] << 8
    if first is None:
        sys.exit('no GAME on this image')
    at = first * 512 + SAVE_PAGE
    raw[at + 0x78:at + 0x7c] = b'VM2M'
    raw[at + 0x7c:at + 0x80] = raw[root + 0x33:root + 0x37]
    return bytes(raw)


if __name__ == '__main__':
    src = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else src
    data = open(src, 'rb').read()
    open(out, 'wb').write(mark(data))
