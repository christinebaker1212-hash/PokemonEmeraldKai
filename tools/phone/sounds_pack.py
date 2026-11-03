#!/usr/bin/env python3
"""The app's sounds (the game's MOEMON_EXPORT_APP sounds.bin, src/mp/mp_export_sound.c): list them, or write some as WAV.

    python3 tools/phone/sounds_pack.py sounds.bin                   the list: name, kind, rate, seconds, loop, bytes
    python3 tools/phone/sounds_pack.py sounds.bin --wav DIR [NAME...]   decoded as 16-bit WAV (songs: twice round the loop)
"""
import struct
import sys
import wave

STEPS = [7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97,
         107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
         876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871,
         5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623,
         27086, 29794, 32767]
MOVE = [-1, -1, -1, -1, 2, 4, 6, 8]
KINDS = ['se', 'cry', 'fanfare', 'song']


def read(path):
    b = open(path, 'rb').read()
    assert b[:4] == b'MSND', 'not a sounds pack'
    ver, count = struct.unpack_from('<II', b, 4)
    at, out = 12, []
    for _ in range(count):
        n = b[at]
        name = b[at + 1:at + 1 + n].decode()
        at += 1 + n
        kind, ch, sid, rate, frames, loop, loop_end, off, size = struct.unpack_from('<BBHIIIIII', b, at)
        at += 28
        out.append(dict(name=name, kind=kind, ch=ch, id=sid, rate=rate, frames=frames, loop=loop, loop_end=loop_end,
                        off=off, size=size))
    return b, out


def decode(b, s):
    ch, p = s['ch'], s['off']
    st = []
    for c in range(ch):
        pred, idx = struct.unpack_from('<hB', b, p + c * 4)
        st.append([pred, idx])
    p += 4 * ch
    out = []
    for i in range(s['frames'] * ch):
        code = (b[p + i // 2] >> (4 * (i & 1))) & 15
        a = st[i % ch]
        step = STEPS[a[1]]
        d = step >> 3
        if code & 4:
            d += step
        if code & 2:
            d += step >> 1
        if code & 1:
            d += step >> 2
        a[0] = max(-32768, min(32767, a[0] - d if code & 8 else a[0] + d))
        a[1] = max(0, min(88, a[1] + MOVE[code & 7]))
        out.append(a[0])
    return out


def loop_twice(pcm, s):
    """The song to its loop's end, crossfaded into the loop's start (as the app plays it), then once round again"""
    ch, L, E = s['ch'], s['loop'], s['loop_end']
    fade = s['frames'] - E
    out = pcm[:E * ch]
    for i in range(fade):
        t = (i + 0.5) / fade
        for c in range(ch):
            out.append(int(pcm[(E + i) * ch + c] * (1 - t) + pcm[(L + i) * ch + c] * t))
    return out + pcm[(L + fade) * ch:E * ch]


def main():
    b, sounds = read(sys.argv[1])
    if '--wav' not in sys.argv:
        total, seen = {}, set()
        for s in sounds:
            if s['off'] not in seen:
                total[s['kind']] = total.get(s['kind'], 0) + s['size']
            seen.add(s['off'])
            if s['kind'] != 1 or s['id'] in (1, 25, 150, 1050):
                print('%-36s %-7s %dch %5d Hz %7.2f s %s %7d B' % (s['name'], KINDS[s['kind']], s['ch'], s['rate'],
                      s['frames'] / s['rate'], 'loop %.2f' % (s['loop'] / s['rate']) if s['loop'] != 0xFFFFFFFF else '',
                      s['size']))
        for k, v in sorted(total.items()):
            print('%s: %d KB' % (KINDS[k], v // 1024))
        return
    i = sys.argv.index('--wav')
    d, names = sys.argv[i + 1], sys.argv[i + 2:]
    for s in sounds:
        if names and s['name'] not in names:
            continue
        pcm = decode(b, s)
        if s['loop'] != 0xFFFFFFFF:
            pcm = loop_twice(pcm, s)
        w = wave.open('%s/%s.wav' % (d, s['name'].replace(':', '_')), 'wb')
        w.setnchannels(s['ch'])
        w.setsampwidth(2)
        w.setframerate(s['rate'])
        w.writeframes(struct.pack('<%dh' % len(pcm), *pcm))
        w.close()


if __name__ == '__main__':
    main()
