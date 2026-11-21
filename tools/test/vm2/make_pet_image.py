#!/usr/bin/env python3
"""Moemon Kai: a VMU image with the pet (vmu_pet/build/pet.bin) holding a given MOeMON in its SAVE page, for the PC's
VM2 test (tools/test/vm2/kai_vm2_test.sh).

usage: make_pet_image.py PET.BIN OUT.BIN STAGE SPECIES_INDEX LEVEL
"""
import struct
import sys

src, out, stage, sp, lv = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5])
d = bytearray(open(src, 'rb').read())
page = 0xBB80                          # the GAME starts at block 0
rec = bytearray(39)
rec[0] = ord('P')
rec[1] = stage
rec[2] = sp
rec[3] = lv
rec[9] = 0                              # no care mistakes
rec[14] = 40                            # trainings ever
struct.pack_into('<I', rec, 16, 0x12345678)
rec[20:26] = bytes([31, 20, 25, 10, 15, 30])
struct.pack_into('<I', rec, 27, 0xC0FFEE01)
rec[31] = 1
d[page:page + 16] = b'MOEMONVMUPET' + bytes([1, 1, 0, 0])
d[page + 16:page + 16 + 39] = rec
open(out, 'wb').write(d)
