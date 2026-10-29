#!/usr/bin/env python3
"""A tiny Game Boy song (GBS) for the tests: square 1 plays a note every 32 frames, the pitch walking up a scale.
usage: make_test_gbs.py OUT.gbs"""
import struct, sys

code = bytes([
    # init (0x0400): the sound on, both sides, full volume
    0x3E, 0x80, 0xE0, 0x26,        # ld a,$80 / ldh (NR52),a
    0x3E, 0x77, 0xE0, 0x24,        # ld a,$77 / ldh (NR50),a
    0x3E, 0xFF, 0xE0, 0x25,        # ld a,$FF / ldh (NR51),a
    0xAF, 0xEA, 0x00, 0xC0,        # xor a / ld ($C000),a
    0xEA, 0x01, 0xC0,              # ld ($C001),a
    0xC9,                          # ret
    # play (0x0414): every 32 calls a new note
    0x21, 0x00, 0xC0,              # ld hl,$C000
    0x34,                          # inc (hl)
    0x7E,                          # ld a,(hl)
    0xE6, 0x1F,                    # and $1F
    0xC0,                          # ret nz
    0x3E, 0xF3, 0xE0, 0x12,        # ld a,$F3 / ldh (NR12),a   volume 15, fading
    0x3E, 0x80, 0xE0, 0x11,        # ld a,$80 / ldh (NR11),a   duty 50%
    0x21, 0x01, 0xC0,              # ld hl,$C001
    0x7E,                          # ld a,(hl)
    0xC6, 0x40,                    # add $40                   the next note up
    0x77,                          # ld (hl),a
    0xE0, 0x13,                    # ldh (NR13),a
    0x3E, 0x86, 0xE0, 0x14,        # ld a,$86 / ldh (NR14),a   trigger, high bits 6
    0xC9,                          # ret
])
head = b'GBS' + bytes([1, 1, 1]) + struct.pack('<HHHH', 0x0400, 0x0400, 0x0414, 0xFFFE) + bytes([0, 0])
head += b'Studio test'.ljust(32, b'\0') + b'Moemon'.ljust(32, b'\0') + b'2026'.ljust(32, b'\0')
open(sys.argv[1], 'wb').write(head + code)
