#!/usr/bin/env python3
"""Gives a .kaisong without a loop (INFO loop=no: its tracks end with FINE, so the song stops once played) one: every
track is padded with waits to the song's full length (the longest track's, rounded up to a beat), then jumps back to
its start (GOTO 0, with its relocation), so all of them start over together. INFO says loop=yes then.
(1.63.8: the SINNOH and JOHTO songs made from rips without a loop point found stopped after one play.)

    python3 -I tools/kaisong/add_loop.py music/FILE.kaisong...     (a file that already loops is left as it is)
    python3 -I tools/kaisong/add_loop.py --check [DIR]              every song in music/ loops, but the game's
                                                                    fanfares' (they must end); exit 1 if not

docs/KAISONG_FORMAT.md: the tracks' code has no running status and no patterns, so its length is the sum of its waits.
"""
import os
import struct
import sys

CLOCKS = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 28, 30, 32, 36,
          40, 42, 44, 48, 52, 54, 56, 60, 64, 66, 68, 72, 76, 78, 80, 84, 88, 90, 92, 96]   # gClockTable (waits 0x80-0xB0)
ONE_ARG = set(range(0xBA, 0xC6)) | {0xC8}
BEAT = 24


def chunks(data):
    hsize, = struct.unpack_from('<H', data, 10)
    n, = struct.unpack_from('<I', data, 16)
    out, pos = [], hsize
    for _ in range(n):
        cid = data[pos:pos + 4]
        size, = struct.unpack_from('<I', data, pos + 4)
        out.append([cid, data[pos + 8:pos + 8 + size]])
        pos += 8 + size + (-size % 4)
    return data[:hsize], out


def walk(code):
    """(clocks, has a jump, where the final FINE is (None: none))"""
    i, clocks, jump, fine = 0, 0, False, None
    while i < len(code):
        c = code[i]
        if 0x80 <= c <= 0xB0:
            clocks += CLOCKS[c - 0x80]
            i += 1
        elif c == 0xB1:
            fine = i
            break
        elif c == 0xB2:
            jump = True
            i += 5
        elif c == 0xB9:   # MEMACC op addr data [t32 for ops 6-17]
            op = code[i + 1]
            i += 4 + (4 if 6 <= op <= 17 else 0)
            if 6 <= op <= 17:
                jump = True
        elif c in ONE_ARG:
            i += 2
        elif c == 0xCD:
            i += 3
        elif c == 0xCE:
            i += 2 if i + 1 < len(code) and code[i + 1] < 0x80 else 1
        elif c >= 0xCF:   # TIE / notes: key, velocity, [gate] (operands below 0x80)
            i += 1
            k = 0
            while k < 3 and i < len(code) and code[i] < 0x80:
                i += 1
                k += 1
        else:
            raise ValueError('unknown command %02X at %d' % (c, i))
    return clocks, jump, fine


def waits(n):
    out = bytearray()
    while n > 0:
        k = max(j for j, v in enumerate(CLOCKS) if v <= n)
        out.append(0x80 + k)
        n -= CLOCKS[k]
    return out


def add_loop(path):
    data = open(path, 'rb').read()
    head, chs = chunks(data)
    trak = next(c for c in chs if c[0] == b'TRAK')
    body = trak[1]
    count, = struct.unpack_from('<I', body, 0)
    tracks, p = [], 4
    for _ in range(count):
        length, nrel = struct.unpack_from('<II', body, p)
        p += 8
        code = bytearray(body[p:p + length])
        p += length
        rels = list(struct.unpack_from('<%dI' % nrel, body, p))
        p += 4 * nrel
        p += -(8 + length + 4 * nrel) % 4
        tracks.append((code, rels))
    walked = [walk(code) for code, _ in tracks]
    if any(j for _, j, _ in walked):
        return 'loops already'
    total = max(c for c, _, _ in walked)
    total = (total + BEAT - 1) // BEAT * BEAT
    out = bytearray(struct.pack('<I', count))
    for (code, rels), (clocks, _, fine) in zip(tracks, walked):
        code = code[:fine] if fine is not None else code
        code += waits(total - clocks)
        rels = rels + [len(code) + 1]
        code += b'\xB2' + struct.pack('<I', 0) + b'\xB1'
        rec = struct.pack('<II', len(code), len(rels)) + bytes(code) + struct.pack('<%dI' % len(rels), *rels)
        out += rec + b'\0' * (-len(rec) % 4)
    trak[1] = bytes(out)
    info = next(c for c in chs if c[0] == b'INFO')
    info[1] = info[1].replace(b'loop=no', b'loop=yes')
    new = bytearray(head)
    for cid, b in chs:
        new += cid + struct.pack('<I', len(b)) + b + b'\0' * (-len(b) % 4)
    open(path, 'wb').write(new)
    return 'loops after %d clocks' % total


def fanfares(root):
    """The songs the game plays as fanfares (src/sound.c sFanfares, PlayFanfare / playfanfare)"""
    import glob
    import re
    names = set()
    text = open(os.path.join(root, 'src/sound.c')).read()
    table = text[text.index('sFanfares[]'):]
    names |= set(re.findall(r'MUS_\w+', table[:table.index('};')]))
    for p in glob.glob(os.path.join(root, 'src/**/*.c'), recursive=True) + glob.glob(os.path.join(root, 'data/**/*.inc'),
                                                                                       recursive=True):
        names |= set(re.findall(r'(?:PlayFanfare|PlayFanfareByFanfareNum|playfanfare)[ (]+(MUS_\w+)', open(p, errors='replace').read()))
    return {n.lower() for n in names}


def check(root):
    import glob
    fan = fanfares(root)
    bad = []
    for path in sorted(glob.glob(os.path.join(root, 'music', '*.kaisong'))):
        song = os.path.basename(path)[:-len('.kaisong')].split('@')[0]
        head, chs = chunks(open(path, 'rb').read())
        info = next(c for c in chs if c[0] == b'INFO')[1]
        loops = b'loop=yes' in info
        if song in fan and loops:
            bad.append('%s: a fanfare that loops (the game would wait for it forever)' % path)
        elif song not in fan and not loops:
            bad.append('%s: stops after one play (add_loop.py gives it a loop)' % path)
    for b in bad:
        print(b)
    return 1 if bad else 0


def main():
    if sys.argv[1:2] == ['--check']:
        sys.exit(check(sys.argv[2] if len(sys.argv) > 2 else
                       os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))))
    for path in sys.argv[1:]:
        print('%s: %s' % (path, add_loop(path)))


if __name__ == '__main__':
    main()
