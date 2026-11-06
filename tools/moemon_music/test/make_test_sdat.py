#!/usr/bin/env python3
"""A tiny Nintendo DS sound archive (SDAT) for the SDAT player's test (src/rip_sdat.c): one sequence, SEQ_TEST, at
tempo 120 (96 ticks a second): a half-second rest, then a loop of C4, E4, G4 (half a second each) and a half-second
rest. Its instrument is a one-cycle sine (a PCM16 wave in a wave archive) whose rate makes C4 play at 261.6 Hz, with an
instant attack and a short release.

    python3 make_test_sdat.py OUT.sdat
"""
import math
import struct
import sys


def block(magic, body, nblocks=1):
    return magic + struct.pack('<HHIHH', 0xFEFF, 0x0100, 16 + len(body), 16, nblocks) + body


def data_block(payload):
    return b'DATA' + struct.pack('<I', 8 + len(payload)) + payload


def sseq():
    loop = []
    for key in (60, 64, 67):
        loop += [key, 127, 48]               # note KEY, velocity 127, 48 ticks
    loop += [0x80, 48]                       # rest
    head = [0xFE, 0x01, 0x00,                # tracks: 0
            0xE1, 120, 0,                    # tempo 120
            0x81, 0,                         # program 0
            0x80, 48]                        # rest (the intro)
    label = len(head)
    body = head + loop + [0x94] + list(struct.pack('<I', label)[:3])
    data = bytes(body)
    payload = struct.pack('<I', 0x1C) + data   # (the sequence's offset in the file)
    return block(b'SSEQ', data_block(payload))


def sbnk():
    inst = struct.pack('<HHBBBBBB', 0, 0, 60, 127, 127, 127, 126, 64)   # wave 0 of archive 0, base C4, ADSR, pan
    recs = struct.pack('<BHB', 1, 0x3C + 4, 0)                         # one PCM instrument
    payload = bytes(32) + struct.pack('<I', 1) + recs + inst
    return block(b'SBNK', data_block(payload))


def swar():
    n = 32
    rate = 261.63 * n
    timer = int(round(16756991 / rate))
    pcm = b''.join(struct.pack('<h', int(20000 * math.sin(2 * math.pi * i / n))) for i in range(n))
    swav = struct.pack('<BBHHHI', 1, 1, int(rate), timer, 0, len(pcm) // 4) + pcm   # PCM16, looping from 0
    payload = bytes(32) + struct.pack('<I', 1) + struct.pack('<I', 0x3C + 4) + swav
    return block(b'SWAR', data_block(payload))


def main():
    files = [sseq(), sbnk(), swar()]
    # SYMB: the sequence's name
    name = b'SEQ_TEST\0'
    # (record 0: a count of 1 and the name's offset; the other records point at an empty list)
    seq_list_off = 8 + 32
    empty_off = seq_list_off + 8
    name_off = empty_off + 4
    symb = struct.pack('<8I', seq_list_off, *([empty_off] * 7)) + struct.pack('<II', 1, name_off) + struct.pack('<I', 0) + name
    symb = b'SYMB' + struct.pack('<I', 8 + len(symb)) + symb
    # INFO: a sequence (file 0, bank 0, volume 127), a bank (file 1, archive 0), a wave archive (file 2)
    base = 8 + 32
    seq_e = struct.pack('<HHHBBBBH', 0, 0, 0, 127, 64, 64, 0, 0)
    bank_e = struct.pack('<HH4H', 1, 0, 0, 0xFFFF, 0xFFFF, 0xFFFF)
    wa_e = struct.pack('<HH', 2, 0)
    body = b''
    offs = []
    for entry in (seq_e, None, bank_e, wa_e, None, None, None, None):
        offs.append(base + len(body))
        if entry is None:
            body += struct.pack('<I', 0)
        else:
            entry_off = base + len(body) + 8
            body += struct.pack('<II', 1, entry_off) + entry
    info = struct.pack('<8I', *offs) + body
    info = b'INFO' + struct.pack('<I', 8 + len(info)) + info
    header = 0x40
    symb_off = header
    info_off = symb_off + len(symb)
    fat_off = info_off + len(info)
    fat_len = 12 + 16 * len(files)
    file_off = fat_off + fat_len
    pos = file_off + 16
    fat = b''
    blob = b''
    for f in files:
        fat += struct.pack('<II8x', pos + len(blob), len(f))
        blob += f
        while len(blob) % 4:
            blob += b'\0'
    fat = b'FAT ' + struct.pack('<II', fat_len, len(files)) + fat
    filebl = b'FILE' + struct.pack('<II4x', 16 + len(blob), len(files)) + blob
    total = file_off + len(filebl)
    head = b'SDAT' + struct.pack('<HHIHH', 0xFEFF, 0x0100, total, header, 4)
    head += struct.pack('<8I', symb_off, len(symb), info_off, len(info), fat_off, len(fat), file_off, len(filebl))
    head += bytes(header - len(head))
    open(sys.argv[1], 'wb').write(head + symb + info + fat + filebl)


if __name__ == '__main__':
    main()
