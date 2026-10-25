#!/usr/bin/env python3
"""Writes a small General MIDI test song (piano, bass, strings with expression swells, a square
lead with a pitch bend and an RPN bend range, drums on channel 10, loopStart / loopEnd markers).
  python3 make_test_midi.py OUT.mid"""
import struct, sys

def vlq(n):
    out = [n & 0x7F]
    n >>= 7
    while n:
        out.insert(0, 0x80 | (n & 0x7F))
        n >>= 7
    return bytes(out)

def track(events):
    events.sort(key=lambda e: e[0])
    data, t = b'', 0
    for time, msg in events:
        data += vlq(time - t) + msg
        t = time
    data += vlq(0) + b'\xff\x2f\x00'
    return b'MTrk' + struct.pack('>I', len(data)) + data

Q = 480
def note(ch, t, key, dur, vel=100):
    return [(t, bytes([0x90 | ch, key, vel])), (t + dur, bytes([0x80 | ch, key, 0]))]
def name(n):
    return (0, b'\xff\x03' + vlq(len(n)) + n.encode())

conductor = [name('Conductor'), (0, b'\xff\x51\x03' + struct.pack('>I', 500000)[1:]),
             (0, b'\xff\x58\x04\x04\x02\x18\x08'),
             (Q * 4, b'\xff\x06\x09loopStart'), (Q * 20, b'\xff\x06\x07loopEnd')]
chords = [(48, 52, 55), (45, 48, 52), (41, 45, 48), (43, 47, 50)]
piano = [name('Piano'), (0, bytes([0xC0, 0]))]
bass = [name('Bass'), (0, bytes([0xC1, 33]))]
strings = [name('Strings'), (0, bytes([0xC2, 48])), (0, bytes([0xB2, 7, 100]))]
lead = [name('Lead'), (0, bytes([0xC3, 80])), (0, bytes([0xB3, 101, 0])), (0, bytes([0xB3, 100, 0])), (0, bytes([0xB3, 6, 12]))]
drums = [name('Drums')]
for bar in range(20):
    c = chords[bar % 4]
    t = bar * Q * 4
    for k in c:
        piano += note(0, t, k + 12, Q * 2 - 20, 90) + note(0, t + Q * 2, k + 12, Q * 2 - 20, 80)
    bass += note(1, t, c[0] - 12, Q - 30) + note(1, t + Q * 2, c[0] - 12, Q - 30) + note(1, t + Q * 3, c[2] - 12, Q - 30)
    for k in c:
        strings += note(2, t, k, Q * 4 - 10, 70)
    for i in range(8):
        strings.append((t + i * Q // 2, bytes([0xB2, 11, 60 + 8 * (i if i < 4 else 8 - i)])))
    if bar >= 4:
        melody = [c[2] + 12, c[1] + 12, c[0] + 12, c[1] + 12]
        for i, k in enumerate(melody):
            lead += note(3, t + i * Q, k, Q - 40, 100)
        lead.append((t + Q * 3 + Q // 2, bytes([0xE3, 0, 0x50])))
        lead.append((t + Q * 4 - 10, bytes([0xE3, 0, 0x40])))
    for i in range(4):
        drums += note(9, t + i * Q, 36 if i % 2 == 0 else 38, Q // 4, 110)
        drums += note(9, t + i * Q + Q // 2, 42, Q // 8, 80)
tracks = [conductor, piano, bass, strings, lead, drums]
out = b'MThd' + struct.pack('>IHHH', 6, 1, len(tracks), Q) + b''.join(track(t) for t in tracks)
open(sys.argv[1], 'wb').write(out)
