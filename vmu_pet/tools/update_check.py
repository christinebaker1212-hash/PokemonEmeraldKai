#!/usr/bin/env python3
"""Moemon VMU pet: after an UPDATE, the VMU's program is the VM2's (pages 0 to $BB7F the same as PET.VMS) and its own
SAVE page is still there (the magic, and the MOeMON byte given).

usage: update_check.py IMAGE PET.VMS [SPECIES_HEX]
"""
import sys

img, vms = open(sys.argv[1], 'rb').read(), open(sys.argv[2], 'rb').read()
# (the header's icon pages, $200-$47f, are the VMU's own again once it starts: icon.inc draws its MOeMON there)
ICON = range(0x200 // 128, 0x480 // 128)
bad = [p for p in range(0xBB80 // 128) if p not in ICON and img[p * 128:(p + 1) * 128] != vms[p * 128:(p + 1) * 128]]
save = img[0xBB80:0xBC00]
msg = []
if bad:
    msg.append('%d pages differ (first %d)' % (len(bad), bad[0]))
if save[:12] != b'MOEMONVMUPET':
    msg.append('no SAVE page')
elif len(sys.argv) > 3 and save[16 + 2] != int(sys.argv[3], 16):
    msg.append('the SAVE page lost its MOeMON (%02x)' % save[18])
if img[0xBC00:0xC000] != vms[0xBC00:0xC000]:
    msg.append('the receiver changed')
print('the update is whole: 375 pages as the VM2\'s (the icon its own), the SAVE page kept' if not msg else 'UPDATE: ' + '; '.join(msg))
sys.exit(1 if msg else 0)
