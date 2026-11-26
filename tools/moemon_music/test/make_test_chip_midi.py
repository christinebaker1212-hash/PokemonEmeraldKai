#!/usr/bin/env python3
"""A small General MIDI song for the CHIPTUNE test (run_tests.sh): more parts than a chip has channels.
  lead     square lead (80): C5 D5 E5 F5 G5 ... as quarter notes
  pad      strings (48): the chord C4 E4 G4 held, three notes at once
  flute    flute (73): A4 held
  bass     bass (33): C2 as half notes
  drums    channel 10: kick, snare, closed hi-hat
120 BPM, 4 bars (8 s), loopStart at bar 2 (2 s), loopEnd at the end.
  python3 make_test_chip_midi.py OUT.mid"""
import struct, sys

def vlq(n):
    out = [n & 0x7F]
    n >>= 7
    while n:
        out.insert(0, 0x80 | (n & 0x7F))
        n >>= 7
    return bytes(out)

def track(events):
    events.sort(key=lambda e: (e[0], e[1][0] & 0xF0 == 0x90))
    data, t = b'', 0
    for time, msg in events:
        data += vlq(time - t) + msg
        t = time
    data += vlq(0) + b'\xff\x2f\x00'
    return b'MTrk' + struct.pack('>I', len(data)) + data

Q = 480
BAR = Q * 4
def note(ch, t, key, dur, vel=100):
    return [(t, bytes([0x90 | ch, key, vel])), (t + dur, bytes([0x80 | ch, key, 0]))]

conductor = [(0, b'\xff\x51\x03' + struct.pack('>I', 500000)[1:]), (BAR, b'\xff\x06\x09loopStart'), (BAR * 4, b'\xff\x06\x07loopEnd')]
lead = [(0, bytes([0xC0, 80]))]
melody = [72, 74, 76, 77, 79, 77, 76, 74]
for i in range(16):
    lead += note(0, i * Q, melody[i % 8], Q - 40, 110)
pad = [(0, bytes([0xC1, 48]))]
for bar in range(4):
    for k in (60, 64, 67):
        pad += note(1, bar * BAR, k, BAR - 10, 90)
flute = [(0, bytes([0xC2, 73]))]
for bar in range(4):
    flute += note(2, bar * BAR, 69, BAR - 10, 90)
bass = [(0, bytes([0xC3, 33]))]
for i in range(8):
    bass += note(3, i * Q * 2, 36, Q * 2 - 30, 110)
drums = []
for i in range(16):
    drums += note(9, i * Q, 36 if i % 2 == 0 else 38, Q // 4, 110)
    drums += note(9, i * Q + Q // 2, 42, Q // 4, 90)

tracks = [conductor, lead, pad, flute, bass, drums]
with open(sys.argv[1], 'wb') as f:
    f.write(b'MThd' + struct.pack('>IHHH', 6, 1, len(tracks), Q))
    for t in tracks:
        f.write(track(t))
