#!/usr/bin/env python3
# A tiny Sega Genesis VGM made here (no game's music): each of the chips' parts alone, one after another, for
# the conversion's levels and drums to be checked against the emulator's.
#   0.0-1.0 s  FM channel 1: a sine (algorithm 7, only operator 4 heard, total level 0), middle C
#   1.0-2.0 s  the DAC: a full-scale square, 220 Hz, written from the data bank at about 11 kHz
#   2.0-3.0 s  PSG tone 1 at full volume, 440 Hz
#   3.0-4.0 s  PSG noise (white, clock / 512) at full volume
#   4.0-6.0 s  eight DAC hits a quarter second apart; only the first is started by a seek, the others go on in
#              the bank where the one before ended, after the DAC was switched off and on again
#   6.0-6.5 s  silence
import struct, sys, math

FM_CLOCK, PSG_CLOCK, RATE = 7670453, 3579545, 44100
cmds = bytearray()

def wait(n):
    while n > 0:
        k = min(n, 65535)
        cmds.extend(b'\x61' + struct.pack('<H', k))
        n -= k

def ym(port, a, d):
    cmds.extend(bytes([0x52 + port, a, d]))

def psg(d):
    cmds.extend(bytes([0x50, d]))

# the data bank: the 220 Hz square (1 s at 11025 writes a second), then eight hits (a decaying noise burst each)
bank = bytearray()
SQ_RATE = 11025
for i in range(SQ_RATE):
    bank.append(0xFF if (i * 220 * 2 // SQ_RATE) % 2 == 0 else 0x00)
hit_start = len(bank)
HIT_LEN = 1200
seed = 12345
for h in range(8):
    for i in range(HIT_LEN):
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF
        amp = 120 * (1 - i / HIT_LEN) ** 2
        bank.append(max(0, min(255, 0x80 + int(amp * ((seed >> 16) / 32768.0 * 2 - 1)))))
cmds.extend(b'\x67\x66\x00' + struct.pack('<I', len(bank)) + bank)

# silence everything
for v in range(4):
    psg(0x9F | v << 5)
ym(0, 0x22, 0)
ym(0, 0x27, 0)
ym(0, 0x2B, 0)
for ch in range(3):
    ym(0, 0x28, ch)
    ym(0, 0x28, 4 | ch)

# FM channel 1: algorithm 7, operators 1-3 silent, operator 4 a sine at total level 0
ym(0, 0xB0, 0x07)
ym(0, 0xB4, 0xC0)
for op_ofs, tl in ((0, 127), (8, 127), (4, 127), (12, 0)):
    ym(0, 0x30 + op_ofs, 0x01)       # DT 0, MUL 1
    ym(0, 0x40 + op_ofs, tl)
    ym(0, 0x50 + op_ofs, 0x1F)       # AR 31
    ym(0, 0x60 + op_ofs, 0x00)
    ym(0, 0x70 + op_ofs, 0x00)
    ym(0, 0x80 + op_ofs, 0x0F)       # SL 0, RR 15
    ym(0, 0x90 + op_ofs, 0x00)
ym(0, 0xA4, 4 << 3 | 644 >> 8)
ym(0, 0xA0, 644 & 0xFF)
ym(0, 0x28, 0xF0)
wait(int(RATE * 0.9))
ym(0, 0x28, 0x00)
wait(int(RATE * 0.1))

# the DAC square
ym(1, 0xB6, 0xC0)
ym(0, 0x2B, 0x80)
cmds.extend(b'\xE0' + struct.pack('<I', 0))
acc = 0.0
for i in range(SQ_RATE):
    acc += RATE / SQ_RATE
    n = int(acc)
    acc -= n
    cmds.append(0x80 | min(n, 15))
    if n > 15:
        wait(n - 15)
ym(0, 0x2B, 0x00)

# PSG tone 1, 440 Hz
n = round(PSG_CLOCK / (32 * 440))
psg(0x80 | (n & 15))
psg((n >> 4) & 0x3F)
psg(0x90)
wait(RATE)
psg(0x9F)

# PSG noise, white, clock / 512
psg(0xE4)
psg(0xF0)
wait(RATE)
psg(0xFF)

# eight DAC hits; the first by a seek, the rest go on where the bank was
cmds.extend(b'\xE0' + struct.pack('<I', hit_start))
HIT_WRITES_RATE = 8000
for h in range(8):
    ym(0, 0x2B, 0x80)
    acc = 0.0
    t = 0
    for i in range(HIT_LEN):
        acc += RATE / HIT_WRITES_RATE
        k = int(acc)
        acc -= k
        cmds.append(0x80 | min(k, 15))
        if k > 15:
            wait(k - 15)
        t += k
    ym(0, 0x2B, 0x00)
    wait(int(RATE * 0.25) - t)
wait(int(RATE * 0.5))
cmds.append(0x66)

total = 0
p = 0
# the total sample count (for the header)
def count(c):
    t = 0
    i = 0
    while i < len(c):
        b = c[i]
        if b == 0x61: t += c[i + 1] | c[i + 2] << 8; i += 3
        elif 0x80 <= b <= 0x8F: t += b & 15; i += 1
        elif b == 0x67: i += 7 + struct.unpack_from('<I', c, i + 3)[0]
        elif b in (0x52, 0x53): i += 3
        elif b == 0x50: i += 2
        elif b == 0xE0: i += 5
        elif b == 0x66: break
        else: i += 1
    return t

hdr = bytearray(0x40)
hdr[0:4] = b'Vgm '
struct.pack_into('<I', hdr, 0x08, 0x150)
struct.pack_into('<I', hdr, 0x0C, PSG_CLOCK)
struct.pack_into('<I', hdr, 0x18, count(cmds))
struct.pack_into('<I', hdr, 0x24, 60)
struct.pack_into('<H', hdr, 0x28, 0x0009)
hdr[0x2A] = 16
struct.pack_into('<I', hdr, 0x2C, FM_CLOCK)
struct.pack_into('<I', hdr, 0x34, 0x0C)
data = hdr + cmds
struct.pack_into('<I', data, 0x04, len(data) - 4)
open(sys.argv[1], 'wb').write(data)
