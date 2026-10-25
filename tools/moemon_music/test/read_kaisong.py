#!/usr/bin/env python3
"""Reference reader for .kaisong files (docs/KAISONG_FORMAT.md). Checks a file the way a loader
in the game should, and prints what is in it.

  python3 read_kaisong.py FILE.kaisong [--tracks]     print the header, info, voices
  python3 read_kaisong.py FILE.kaisong --bytecode     the tracks with targets as offsets (like --dump)
"""
import struct, sys

class Bad(Exception):
    pass

def read(path):
    data = open(path, 'rb').read()
    if len(data) < 32 or data[:8] != b'KAISONG\0':
        raise Bad('not a .kaisong file')
    version, hsize, ntracks, priority, reverb, mvl, nchunks = struct.unpack_from('<HHBBBBI', data, 8)
    if version != 1:
        raise Bad('version %d (this reader knows 1)' % version)
    if not 1 <= ntracks <= 10:
        raise Bad('%d tracks (the game plays 1-10)' % ntracks)
    song = dict(version=version, ntracks=ntracks, priority=priority, reverb=reverb, volume=mvl, chunks={})
    pos = hsize
    for _ in range(nchunks):
        if pos + 8 > len(data):
            raise Bad('chunk header past the end')
        cid = data[pos:pos + 4].decode('ascii', 'replace')
        size, = struct.unpack_from('<I', data, pos + 4)
        body = data[pos + 8:pos + 8 + size]
        if len(body) != size:
            raise Bad('chunk %s past the end' % cid)
        song['chunks'][cid] = body
        pos += 8 + size + (-size % 4)
    for need in ('INFO', 'NAME', 'VGRP', 'KSPL', 'TRAK'):
        if need not in song['chunks']:
            raise Bad('no %s chunk' % need)

    info = {}
    for line in song['chunks']['INFO'].decode('utf-8', 'replace').splitlines():
        if '=' in line:
            k, v = line.split('=', 1)
            info[k] = v
    song['info'] = info

    b = song['chunks']['NAME']
    n, = struct.unpack_from('<I', b, 0)
    names, p = [], 4
    for _ in range(n):
        l, = struct.unpack_from('<H', b, p)
        names.append(b[p + 2:p + 2 + l].decode('ascii'))
        p += 2 + l
    song['names'] = names

    b = song['chunks']['VGRP']
    ng, = struct.unpack_from('<I', b, 0)
    if ng < 1 or len(b) != 4 + ng * 128 * 16:
        raise Bad('VGRP holds %d bytes for %d groups' % (len(b), ng))
    groups = []
    for g in range(ng):
        voices = []
        for i in range(128):
            o = 4 + (g * 128 + i) * 16
            t, key, length, ps, a, d, s, r, kind, _, ref, ks, _ = struct.unpack_from('<8BBBHHH', b, o)
            v = dict(type=t, key=key, length=length, pan_sweep=ps, adsr=(a, d, s, r), kind=kind, ref=ref, ks=ks)
            if kind in (1, 2) and ref >= len(names):
                raise Bad('group %d voice %d names string %d of %d' % (g, i, ref, len(names)))
            if kind in (4, 5):
                if g != 0:
                    raise Bad('group %d voice %d is a kit inside a kit' % (g, i))
                if not 1 <= ref < ng:
                    raise Bad('voice %d points at group %d of %d' % (i, ref, ng))
                if (kind == 4) != bool(t & 0x80) or (kind == 5) != bool(t & 0x40):
                    raise Bad('voice %d: kind %d with type 0x%02X' % (i, kind, t))
            voices.append(v)
        groups.append(voices)
    song['groups'] = groups

    b = song['chunks']['KSPL']
    nk, = struct.unpack_from('<I', b, 0)
    if len(b) != 4 + nk * 128:
        raise Bad('KSPL size')
    song['keysplits'] = [b[4 + k * 128:4 + (k + 1) * 128] for k in range(nk)]
    for v in groups[0]:
        if v['kind'] == 5 and v['ks'] >= nk:
            raise Bad('key-split table %d of %d' % (v['ks'], nk))

    b = song['chunks']['TRAK']
    nt, = struct.unpack_from('<I', b, 0)
    if nt != ntracks:
        raise Bad('TRAK has %d tracks, header says %d' % (nt, ntracks))
    tracks, p = [], 4
    for _ in range(nt):
        length, nrel = struct.unpack_from('<II', b, p)
        p += 8
        code = b[p:p + length]
        p += length
        rel = list(struct.unpack_from('<%dI' % nrel, b, p))
        p += 4 * nrel
        p += -p % 4
        for r in rel:
            if r + 4 > length:
                raise Bad('relocation %d past the track' % r)
            target, = struct.unpack_from('<I', code, r)
            if target >= length:
                raise Bad('jump to %d in a %d-byte track' % (target, length))
        tracks.append(dict(code=code, relocs=rel))
    song['tracks'] = tracks
    return song

KINDS = {0: 'none', 1: 'sample', 2: 'wave', 3: 'value', 4: 'kit', 5: 'key-split'}

def describe(song, v):
    k = v['kind']
    ref = song['names'][v['ref']] if k in (1, 2) else v['ref']
    return 'type 0x%02X key %3d len %3d pan/sweep 0x%02X adsr %-17s %s %s' % (
        v['type'], v['key'], v['length'], v['pan_sweep'], '%d,%d,%d,%d' % v['adsr'], KINDS.get(k, '?'), ref)

def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    try:
        song = read(sys.argv[1])
    except Bad as e:
        sys.exit('bad .kaisong: %s' % e)
    if '--bytecode' in sys.argv:
        for i, t in enumerate(song['tracks']):
            print('T%d %s' % (i + 1, ' '.join('%02x' % x for x in t['code'])))
        return
    print('version %(version)d, %(ntracks)d tracks, priority %(priority)d, reverb 0x%(reverb)02X, volume %(volume)d' % song)
    for k, v in song['info'].items():
        print('  %s: %s' % (k, v))
    print('%d sample / wave names, %d voice groups, %d key-split tables' % (len(song['names']), len(song['groups']), len(song['keysplits'])))
    for i, v in enumerate(song['groups'][0]):
        if not (v['type'] == 1 and v['kind'] == 3 and v['adsr'] == (0, 0, 15, 0)):   # skip the empty-slot square
            print('  voice %3d: %s' % (i, describe(song, v)))
    for i, t in enumerate(song['tracks']):
        print('  track %d: %d bytes, %d jump(s)' % (i + 1, len(t['code']), len(t['relocs'])))

if __name__ == '__main__':
    main()
