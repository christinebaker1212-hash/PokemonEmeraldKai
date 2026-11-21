#!/usr/bin/env python3
"""Moemon VMU pet: VMU flash images (128 KB, the format the VM2 keeps on its SD card as .bin / .vmu).

usage:
  vmufs.py make OUT.bin [--game FILE.VMS [--name NAME]] [--data NAME=FILE ...]   a formatted image with these files
  vmufs.py ls IMAGE                                                            the directory
  vmufs.py get IMAGE NAME OUT                                                  one file's bytes
  vmufs.py vmi FILE.VMS VMUNAME OUT.VMI [DESCRIPTION]                          the .VMI that goes with a .VMS (for
                                                                               VMU Explorer, the VM2's GUI, emulators)

The layout (a standard VMU; libevmu's evmu_fat.h names the same fields):
  256 blocks of 512 bytes. Block 255 the root: 16 x 0x55, a volume label (custom color), a BCD timestamp at 0x30,
  then u16s at 0x40: total size 255, partition 0, root 255, FAT 254, FAT size 1, directory 253, directory size 13,
  icon shape (u8) and sort flag (u8), the extra region 200 / 41, the GAME's place 0 / 128.
  Block 254 the FAT: 256 little-endian u16s, 0xFFFC free, 0xFFFA the last block of a file, else the next block.
  Blocks 253 down to 241 the directory: 32-byte entries (type 0x33 data / 0xCC game, copy protection, first block,
  12-byte name, BCD timestamp, size in blocks, header offset in blocks (1 for a GAME), 4 unused).
  A GAME starts at block 0 and runs on contiguously (the CPU runs it straight from flash); data files are placed from
  block 199 downwards, as the BIOS does, leaving the low blocks to a GAME.
"""
import datetime
import struct
import sys

BLOCK = 512
BLOCKS = 256
ROOT, FAT, DIR, DIR_SIZE = 255, 254, 253, 13
USER_BLOCKS = 200
FREE, LAST = 0xFFFC, 0xFFFA
TYPE_DATA, TYPE_GAME = 0x33, 0xCC


def bcd(n):
    return ((n // 10) << 4) | (n % 10)


def timestamp(t=None):
    t = t or datetime.datetime.now()
    return bytes([bcd(t.year // 100), bcd(t.year % 100), bcd(t.month), bcd(t.day), bcd(t.hour), bcd(t.minute),
                  bcd(t.second), t.weekday()])


def crc16(data):
    """The VMS header's CRC (data files; GAME files leave it 0): CRC-16/XMODEM."""
    n = 0
    for b in data:
        n ^= b << 8
        for _ in range(8):
            n = ((n << 1) ^ 0x1021) if n & 0x8000 else (n << 1)
            n &= 0xFFFF
    return n


class Image:
    def __init__(self, raw=None):
        if raw is None:
            self.raw = bytearray(BLOCK * BLOCKS)
            self.format()
        else:
            if len(raw) != BLOCK * BLOCKS:
                raise ValueError('a VMU image is %d bytes, this is %d' % (BLOCK * BLOCKS, len(raw)))
            self.raw = bytearray(raw)

    def block(self, n):
        return memoryview(self.raw)[n * BLOCK:(n + 1) * BLOCK]

    def format(self):
        r = self.block(ROOT)
        r[0:16] = b'\x55' * 16
        r[0x30:0x38] = timestamp()
        struct.pack_into('<7H', r, 0x40, 255, 0, ROOT, FAT, 1, DIR, DIR_SIZE)
        struct.pack_into('<4H', r, 0x50, USER_BLOCKS, 41, 0, 128)
        for b in range(BLOCKS):
            self.set_fat(b, FREE)
        self.set_fat(ROOT, LAST)
        self.set_fat(FAT, LAST)
        for b in range(DIR, DIR - DIR_SIZE, -1):
            self.set_fat(b, b - 1 if b > DIR - DIR_SIZE + 1 else LAST)

    def fat(self, b):
        return struct.unpack_from('<H', self.raw, FAT * BLOCK + b * 2)[0]

    def set_fat(self, b, v):
        struct.pack_into('<H', self.raw, FAT * BLOCK + b * 2, v)

    def entries(self):
        for d in range(DIR, DIR - DIR_SIZE, -1):
            for i in range(BLOCK // 32):
                off = d * BLOCK + i * 32
                yield off, self.raw[off:off + 32]

    def files(self):
        out = []
        for off, e in self.entries():
            if e[0] in (TYPE_DATA, TYPE_GAME):
                first, = struct.unpack_from('<H', e, 2)
                size, hdr = struct.unpack_from('<HH', e, 0x18)
                out.append({'type': 'GAME' if e[0] == TYPE_GAME else 'DATA', 'name': bytes(e[4:16]).decode('ascii', 'replace').rstrip(),
                            'first': first, 'blocks': size, 'header': hdr, 'entry': off})
        return out

    def chain(self, first):
        b, out = first, []
        while b != LAST:
            out.append(b)
            if len(out) > BLOCKS:
                raise ValueError('a FAT chain loops')
            b = self.fat(b)
        return out

    def read(self, name):
        for f in self.files():
            if f['name'] == name:
                return b''.join(bytes(self.block(b)) for b in self.chain(f['first']))
        raise KeyError(name)

    def _entry_slot(self):
        for off, e in self.entries():
            if e[0] == 0:
                return off
        raise ValueError('the directory is full')

    def _write_entry(self, kind, name, first, nblocks, header):
        off = self._entry_slot()
        e = bytearray(32)
        e[0] = kind
        struct.pack_into('<H', e, 2, first)
        e[4:16] = name.encode('ascii')[:12].ljust(12, b' ')
        e[0x10:0x18] = timestamp()
        struct.pack_into('<HH', e, 0x18, nblocks, header)
        self.raw[off:off + 32] = e

    def add_game(self, data, name):
        if any(f['type'] == 'GAME' for f in self.files()):
            raise ValueError('a VMU holds one GAME')
        n = (len(data) + BLOCK - 1) // BLOCK
        if n > USER_BLOCKS or any(self.fat(b) != FREE for b in range(n)):
            raise ValueError('blocks 0-%d are not free for a %d-block GAME' % (n - 1, n))
        data = bytes(data).ljust(n * BLOCK, b'\0')
        for b in range(n):
            self.block(b)[:] = data[b * BLOCK:(b + 1) * BLOCK]
            self.set_fat(b, b + 1 if b + 1 < n else LAST)
        self._write_entry(TYPE_GAME, name, 0, n, 1)

    def add_data(self, data, name):
        n = (len(data) + BLOCK - 1) // BLOCK
        free = [b for b in range(USER_BLOCKS - 1, -1, -1) if self.fat(b) == FREE][:n]
        if len(free) < n:
            raise ValueError('not enough free blocks')
        data = bytes(data).ljust(n * BLOCK, b'\0')
        for i, b in enumerate(free):
            self.block(b)[:] = data[i * BLOCK:(i + 1) * BLOCK]
            self.set_fat(b, free[i + 1] if i + 1 < n else LAST)
        self._write_entry(TYPE_DATA, name, free[0], n, 0)


def make_vmi(vms_path, vmu_name, desc=''):
    """The 108-byte .VMI: checksum (resource name AND 'SEGA'), description, copyright, date, version 0, file number 1,
    the .VMS's resource name (8), the name on the VMU (12), mode (bit 1 GAME, bit 0 copy protected), size."""
    import os
    size = os.path.getsize(vms_path)
    res = os.path.splitext(os.path.basename(vms_path))[0].upper().encode('ascii')[:8].ljust(8, b'\0')
    data = open(vms_path, 'rb').read()
    is_game = len(data) > 0x200 + 0x40 and data[0x200:0x210].strip() != b''
    v = bytearray(108)
    v[0:4] = bytes(res[i] & b'SEGA'[i] for i in range(4))
    v[4:36] = desc.encode('ascii')[:32].ljust(32, b' ')
    v[36:68] = b'Moemon Emerald Kai'.ljust(32, b' ')
    t = datetime.datetime.now()
    struct.pack_into('<H6B', v, 0x44, t.year, t.month, t.day, t.hour, t.minute, t.second, (t.weekday() + 1) % 7)
    struct.pack_into('<HH', v, 0x4C, 0, 1)
    v[0x50:0x58] = res
    v[0x58:0x64] = vmu_name.encode('ascii')[:12].ljust(12, b' ')
    struct.pack_into('<HHI', v, 0x64, 2 if is_game else 0, 0, size)
    return bytes(v)


def main(argv):
    if len(argv) < 3:
        sys.exit(__doc__)
    cmd = argv[1]
    if cmd == 'make':
        img = Image()
        args = argv[3:]
        name = None
        i = 0
        game = None
        datas = []
        while i < len(args):
            if args[i] == '--game':
                game = args[i + 1]; i += 2
            elif args[i] == '--name':
                name = args[i + 1]; i += 2
            elif args[i] == '--data':
                n, p = args[i + 1].split('=', 1); datas.append((n, p)); i += 2
            else:
                sys.exit('unknown argument ' + args[i])
        if game:
            img.add_game(open(game, 'rb').read(), name or 'MOEMON')
        for n, p in datas:
            img.add_data(open(p, 'rb').read(), n)
        open(argv[2], 'wb').write(img.raw)
    elif cmd == 'ls':
        img = Image(open(argv[2], 'rb').read())
        for f in img.files():
            print('%-4s %-12s first %3d  %3d blocks' % (f['type'], f['name'], f['first'], f['blocks']))
    elif cmd == 'get':
        img = Image(open(argv[2], 'rb').read())
        open(argv[4], 'wb').write(img.read(argv[3]))
    elif cmd == 'vmi':
        open(argv[4], 'wb').write(make_vmi(argv[2], argv[3], argv[5] if len(argv) > 5 else ''))
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main(sys.argv)
