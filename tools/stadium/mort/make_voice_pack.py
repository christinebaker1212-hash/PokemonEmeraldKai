#!/usr/bin/env python3
"""POKeMON STADIUM's announcer pack: the ROM's speech archive copied as it is (6.5 MB of MORT streams).

    make_voice_pack.py ROM_OR_ZIP [OUT]     (OUT: stadium/announcer.s1)

ROM_OR_ZIP is Pokemon Stadium (USA) 1.0 (.z64 / .v64 / .n64, any byte order), or a zip holding it (zips inside
zips too, as the "ROM and Music" download has them). The game decodes the clips itself (mort/, Mort_Decode), so
the pack is the archive and nothing else: an S1 container of three S1 containers of MORT streams (165 + 1 + 657
clips). The ROM and the pack are never committed: stadium/ is ignored.
"""
import io
import os
import struct
import sys
import zipfile

ARCHIVE = 0x197C1E0


def normalize(rom):
    """The ROM in big-endian (.z64) order."""
    head = rom[:4]
    if head == b'\x80\x37\x12\x40':
        return rom
    if head == b'\x37\x80\x40\x12':   # .v64: bytes swapped in pairs
        b = bytearray(rom)
        b[0::2], b[1::2] = rom[1::2], rom[0::2]
        return bytes(b)
    if head == b'\x40\x12\x37\x80':   # .n64: little-endian words
        b = bytearray(len(rom))
        for i in range(0, len(rom) - 3, 4):
            b[i:i + 4] = rom[i:i + 4][::-1]
        return bytes(b)
    return None


def find_rom(data, depth=0):
    """A Pokemon Stadium (USA) 1.0 image in data (a ROM, or a zip of them)."""
    rom = normalize(data)
    if rom is not None:
        if rom[0x20:0x34].rstrip(b' \0') == b'POKEMON STADIUM' and has_archive(rom):
            return rom
        return None
    if depth > 2 or data[:2] != b'PK':
        return None
    with zipfile.ZipFile(io.BytesIO(data)) as z:
        names = sorted(z.namelist(), key=lambda n: ('Rev' in n, n))   # (1.0 before Rev 1 / Rev 2)
        for name in names:
            if name.lower().endswith(('.z64', '.v64', '.n64', '.zip')):
                rom = find_rom(z.read(name), depth + 1)
                if rom is not None:
                    return rom
    return None


def has_archive(rom):
    return rom[ARCHIVE:ARCHIVE + 2] == b'S1' and struct.unpack('>H', rom[ARCHIVE + 2:ARCHIVE + 4])[0] == 3


def archive(rom):
    u32 = lambda o: struct.unpack('>I', rom[o:o + 4])[0]
    u16 = lambda o: struct.unpack('>H', rom[o:o + 2])[0]
    end = 0
    clips = 0
    for c in range(3):
        off, size = u32(ARCHIVE + 4 + c * 8), u32(ARCHIVE + 8 + c * 8)
        child = ARCHIVE + off
        if rom[child:child + 2] != b'S1':
            raise SystemExit('make_voice_pack: the speech archive looks wrong')
        n = u16(child + 2)
        for i in range(n):
            o = child + u32(child + 4 + i * 8)
            if rom[o:o + 4] != b'MORT':
                raise SystemExit('make_voice_pack: clip %d is not a MORT stream' % (clips + i))
        clips += n
        end = max(end, off + size)
    if clips != 823:
        raise SystemExit('make_voice_pack: %d clips, 823 expected (Pokemon Stadium (USA) 1.0 is needed)' % clips)
    return rom[ARCHIVE:ARCHIVE + end]


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    out = sys.argv[2] if len(sys.argv) > 2 else 'stadium/announcer.s1'
    with open(sys.argv[1], 'rb') as f:
        rom = find_rom(f.read())
    if rom is None:
        raise SystemExit('make_voice_pack: no Pokemon Stadium (USA) 1.0 ROM in ' + sys.argv[1])
    data = archive(rom)
    os.makedirs(os.path.dirname(out) or '.', exist_ok=True)
    with open(out, 'wb') as f:
        f.write(data)
    print('make_voice_pack: 823 clips, %d bytes to %s' % (len(data), out))


if __name__ == '__main__':
    main()
