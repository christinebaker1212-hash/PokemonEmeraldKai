#!/usr/bin/env python3
"""catch_yesno_frame: BG palette 1 (the battle's YES / NO frame) as displayed equals its loaded colors"""
import os
import struct
import sys

d = open(os.path.join(sys.argv[1], 'yesno_pal.bin'), 'rb').read()
size = (len(d) - 0x60) // 3   # (palette RAM, the unfaded buffer, the faded buffer, then the display registers)
shown = struct.unpack_from('<16H', d, 32)
loaded = struct.unpack_from('<16H', d, size + 32)
print('BG palette 1 shown %s, loaded %s' % (' '.join('%04x' % c for c in shown[:6]), ' '.join('%04x' % c for c in loaded[:6])))
sys.exit(0 if shown == loaded else 1)
