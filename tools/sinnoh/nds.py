#!/usr/bin/env python3
"""Pokémon Platinum's files: the DS cartridge's file system (FNT / FAT) and the NARC archives inside it.

    python3 -I tools/sinnoh/nds.py ROM                       # every file: id, size, path
    python3 -I tools/sinnoh/nds.py ROM fielddata/land_data/land_data.narc   # a NARC's members and sizes

Reads the user's ROM; nothing read from it is ever committed (docs/SINNOH_HANDOFF.md).
"""
import struct
import sys


class Rom:
    def __init__(self, path):
        self.data = open(path, 'rb').read()
        d = self.data
        self.code = d[12:16].decode('ascii', 'replace')
        fnt_off, fnt_size, fat_off, fat_size = struct.unpack_from('<IIII', d, 0x40)
        self.fat = [struct.unpack_from('<II', d, fat_off + 8 * i) for i in range(fat_size // 8)]
        self.paths = {}   # 'dir/sub/name' -> file id
        self._walk(fnt_off, 0xF000, '')

    def _walk(self, fnt, dir_id, prefix):
        d = self.data
        sub_off, first_id, _ = struct.unpack_from('<IHH', d, fnt + 8 * (dir_id & 0xFFF))
        p = fnt + sub_off
        fid = first_id
        while True:
            t = d[p]
            p += 1
            if t == 0:
                break
            n = t & 0x7F
            name = d[p:p + n].decode('latin-1')
            p += n
            if t & 0x80:
                sub = struct.unpack_from('<H', d, p)[0]
                p += 2
                self._walk(fnt, sub, prefix + name + '/')
            else:
                self.paths[prefix + name] = fid
                fid += 1

    def file(self, path_or_id):
        fid = self.paths[path_or_id] if isinstance(path_or_id, str) else path_or_id
        a, b = self.fat[fid]
        return self.data[a:b]

    def narc(self, path):
        return Narc(self.file(path))


class Narc:
    """A NARC: BTAF (offsets), BTNF (names, unused here), GMIF (the data)."""

    def __init__(self, data):
        assert data[:4] == b'NARC', data[:4]
        p = struct.unpack_from('<H', data, 0x0C)[0]   # header size
        assert data[p:p + 4] == b'BTAF'
        btaf_size, count = struct.unpack_from('<IH', data, p + 4)
        offs = [struct.unpack_from('<II', data, p + 12 + 8 * i) for i in range(count)]
        p += btaf_size
        assert data[p:p + 4] == b'BTNF'
        p += struct.unpack_from('<I', data, p + 4)[0]
        assert data[p:p + 4] == b'GMIF'
        base = p + 8
        self.members = [data[base + a:base + b] for a, b in offs]

    def __len__(self):
        return len(self.members)

    def __getitem__(self, i):
        return self.members[i]


def main():
    rom = Rom(sys.argv[1])
    if len(sys.argv) > 2:
        n = rom.narc(sys.argv[2])
        print('%s: %d members' % (sys.argv[2], len(n)))
        for i, m in enumerate(n.members):
            print('%5d %8d %s' % (i, len(m), m[:4]))
        return
    print('game code', rom.code)
    for path, fid in sorted(rom.paths.items(), key=lambda x: x[1]):
        a, b = rom.fat[fid]
        print('%5d %9d %s' % (fid, b - a, path))


if __name__ == '__main__':
    main()
