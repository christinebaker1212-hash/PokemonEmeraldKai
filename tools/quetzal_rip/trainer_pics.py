"""Trainer battle pictures from Moemon Quetzal (front pics, and the back pic frames).

How the tables were found (ROM addresses; file offset = address - 0x08000000):
- GetTrainerFrontPicId (0x082A16D4) gives the pic id: trainer 0x137F -> 0x11C, Kanto ids
  (0x2000-0x22E8 with a presence byte) -> trainerPic + 0xDF, else trainerPic.
- DecompressTrainerFrontPic (0x08093490) indexes 0x084F4910 by pic id (CompressedSpriteSheet
  {data*, u16 size 0x800, u16 tag = id}) and loads the palette from 0x084F51F8 (CompressedSpritePalette
  {data*, u16 tag, u16 pad}, LZ77 palettes). The two tables sit back to back: 285 front pics (ids 0-0x11C;
  id 131 is a null entry).
- The pic data goes through 0x0803AD10, which is pokeemerald-expansion's DecompressDataWithHeaderWram: a
  header whose low nibble is 0 is LZ77, 1-6 is "smol" (tANS) data. Quetzal's trainer pics are smol mode 6.
  smol_decompress() below is a Python port of expansion's src/decompress.c (SmolDecompressData).
- Back pics: DecompressTrainerBackPic (0x080934D8) indexes 0x084F6CF8 ({data*, u16 size, u16 id} x 113;
  raw 4bpp, 0x2000 = 4 frames or 0x2800 = 5 frames of 64x64) and loads the palette from 0x084F70C8
  ({pal*, u16 id, u16 0}, LZ77). (The 475-entry table at 0x084E284C is the same data cut into single
  frames for the sprite templates at 0x085C4F4C; 0x084F6928 holds the templates' anims, not palettes.)
"""
import os
import struct

from gfx import lz77, gba_colors, write_jasc, sprite_png

FRONT_PIC_TABLE = 0x084F4910      # gTrainerFrontPicTable: {data*, u16 0x800, u16 id} x 285
FRONT_PAL_TABLE = 0x084F51F8      # gTrainerFrontPicPaletteTable: {pal*, u16 id, u16 0} x 285
NUM_FRONT_PICS = 285              # ids 0..0x11C (0x11C = trainer 0x137F's special pic)
BACK_PIC_TABLE = 0x084F6CF8       # gTrainerBackPicTable: {raw 4bpp data*, u16 size, u16 id} x 113
BACK_PAL_TABLE = 0x084F70C8       # gTrainerBackPicPaletteTable: {pal*, u16 id, u16 0} x 113
NUM_BACK_PICS = 113
ROM_BASE = 0x08000000
PIC_SIZE = 0x800                  # 64x64 4bpp


# ---- smol (tANS) decompression, ported from pokeemerald-expansion -------------------
def _yk(x):
    """sYkTemplate[x]: (k, y) for tANS table slot value x (1..127)."""
    k = 6 - (x.bit_length() - 1) if x < 64 else 0
    return k, (x << k) - 64


def _build_table(words):
    freqs = [0] * 16
    for i in range(3):
        w = words[i]
        for j in range(5):
            freqs[i * 5 + j] = (w >> (6 * j)) & 0x3F
    freqs[15] = sum(((words[i] >> 30) & 3) << (2 * i) for i in range(3))
    table = []
    for sym, f in enumerate(freqs):
        for x in range(f, 2 * f):
            k, y = _yk(x)
            table.append((sym, k, y))
    if len(table) != 64:
        raise ValueError('bad tANS frequencies (sum %d)' % len(table))
    return table


class _Bits:
    def __init__(self, d, o):
        self.d, self.o, self.pos = d, o, 0

    def read(self, k):
        v = 0
        for i in range(k):
            byte = self.d[self.o + (self.pos >> 3)]
            v |= ((byte >> (self.pos & 7)) & 1) << i
            self.pos += 1
        return v


def smol_decompress(d, o):
    """Decompress expansion 'smol' data (modes 1-6) at file offset o."""
    w0, w1 = struct.unpack_from('<II', d, o)
    mode = w0 & 15
    sym_size = w0 >> 18
    state = w1 & 0x3F
    bitstream_size = (w1 >> 6) & 0x1FFF
    lo_size = w1 >> 19
    if not 1 <= mode <= 6:
        raise ValueError('not smol data at 0x%X (mode %d)' % (o, mode))
    data = o + 8
    words = lambda p: struct.unpack_from('<3I', d, p)
    lo_enc = mode in (4, 5, 6)
    sym_enc = mode in (2, 3, 5, 6)
    sym_delta = mode in (3, 6)
    lo_freq = sym_freq = None
    if mode == 1:
        bits_at = None
    elif mode == 4:
        lo_freq, bits_at = words(data), data + 12
    elif mode in (2, 3):
        sym_freq, bits_at = words(data), data + 12
    else:
        lo_freq, sym_freq, bits_at = words(data), words(data + 12), data + 24
    bits = _Bits(d, bits_at) if bits_at is not None else None

    def decode(table, n):
        nonlocal state
        out = []
        for _ in range(n):
            s, k, y = table[state]
            state = y + bits.read(k)
            out.append(s)
        return out

    left = data
    lo = syms = None
    if lo_enc:
        nib = decode(_build_table(lo_freq), 2 * lo_size)
        lo = bytes(nib[i] | nib[i + 1] << 4 for i in range(0, len(nib), 2))
        left += 12
    if sym_enc:
        nib = decode(_build_table(sym_freq), 4 * sym_size)
        if sym_delta:
            cur = 0
            for i, v in enumerate(nib):
                cur = (cur + v) & 15
                nib[i] = cur
        syms = [nib[i] | nib[i + 1] << 4 | nib[i + 2] << 8 | nib[i + 3] << 12 for i in range(0, len(nib), 4)]
        left += 12
    if lo_enc or sym_enc:
        left += 4 * bitstream_size
    if not sym_enc:
        syms = list(struct.unpack_from('<%dH' % sym_size, d, left))
        left += 2 * sym_size
    if not lo_enc:
        lo = d[left:left + lo_size]

    # DecodeInstructions
    out = []
    p = si = 0
    while p < lo_size:
        if lo[p] & 0x80:
            length = (lo[p] & 0x7F) | lo[p + 1] << 7
            off = lo[p + 2] & 0x7F
            if lo[p + 2] & 0x80:
                off |= lo[p + 3] << 7
                p += 4
            else:
                p += 3
        else:
            length = lo[p] & 0x7F
            off = lo[p + 1] & 0x7F
            if lo[p + 1] & 0x80:
                off |= lo[p + 2] << 7
                p += 3
            else:
                p += 2
        if length:
            out.append(syms[si])
            si += 1
            if off == 1:
                out.extend([out[-1]] * length)
            else:
                for _ in range(length):
                    out.append(out[-off])
        else:
            out.extend(syms[si:si + off])
            si += off
    return struct.pack('<%dH' % len(out), *out)


def decompress(d, o):
    """DecompressDataWithHeaderWram: LZ77 or smol, by the header's low nibble."""
    if d[o] & 15 == 0:
        return lz77(d, o)[0]
    return smol_decompress(d, o)


# ---- tables -----------------------------------------------------------------------
def _off(rom, addr):
    return addr - ROM_BASE


def front_pic_entry(rom, pic_id):
    """(pic offset, palette offset) for a front pic id, or None for an empty entry."""
    if not 0 <= pic_id < NUM_FRONT_PICS:
        return None
    pic = rom.ptr(_off(rom, FRONT_PIC_TABLE) + 8 * pic_id)
    pal = rom.ptr(_off(rom, FRONT_PAL_TABLE) + 8 * pic_id)
    if pic is None or pal is None:
        return None
    return pic, pal


def front_pic(rom, pic_id):
    """(0x800 bytes of 4bpp tiles, 16 RGB colors, pic offset) or None."""
    e = front_pic_entry(rom, pic_id)
    if not e:
        return None
    gfx = decompress(rom.d, e[0])[:PIC_SIZE]
    colors = gba_colors(decompress(rom.d, e[1]), 0, 16)
    return gfx, colors, e[0]


def back_pic(rom, back_id):
    """(list of 0x800-byte 64x64 frames, 16 RGB colors, data offset) or None."""
    if not 0 <= back_id < NUM_BACK_PICS:
        return None
    o = _off(rom, BACK_PIC_TABLE) + 8 * back_id
    p = rom.ptr(o)
    pal = rom.ptr(_off(rom, BACK_PAL_TABLE) + 8 * back_id)
    if p is None or pal is None:
        return None
    size = rom.u16(o + 4)
    frames = [rom.d[p + i:p + i + PIC_SIZE] for i in range(0, size, PIC_SIZE)]
    colors = gba_colors(decompress(rom.d, pal), 0, 16)
    return frames, colors, p


# ---- output -----------------------------------------------------------------------
def rip_trainer_pics(rom, pic_ids, out_dir):
    """Write out_dir/front/pic_<id>.png (64x64 indexed) + .pal (JASC) for each front pic id."""
    fdir = os.path.join(out_dir, 'front')
    os.makedirs(fdir, exist_ok=True)
    res = []
    for pid in pic_ids:
        r = front_pic(rom, pid)
        if not r:
            continue
        gfx, colors, addr = r
        png = os.path.join(fdir, 'pic_%d.png' % pid)
        pal = os.path.join(fdir, 'pic_%d.pal' % pid)
        sprite_png(png, gfx, colors, 64, 64)
        write_jasc(pal, colors)
        res.append({'id': pid, 'png': png, 'palette': pal, 'size': (64, 64), 'addr': addr + ROM_BASE})
    return res


def rip_trainer_back_pics(rom, back_ids, out_dir):
    """Write out_dir/back/back_<id>.png (64 wide, one 64x64 frame per row) + .pal for each back pic."""
    bdir = os.path.join(out_dir, 'back')
    os.makedirs(bdir, exist_ok=True)
    res = []
    for bid in back_ids:
        r = back_pic(rom, bid)
        if not r:
            continue
        frames, colors, addr = r
        png = os.path.join(bdir, 'back_%d.png' % bid)
        pal = os.path.join(bdir, 'back_%d.pal' % bid)
        sprite_png(png, b''.join(frames), colors, 64, 64 * len(frames))
        write_jasc(pal, colors)
        res.append({'id': bid, 'png': png, 'palette': pal, 'size': (64, 64), 'frames': len(frames),
                    'addr': addr + ROM_BASE})
    return res


if __name__ == '__main__':
    import sys
    from qrom import Rom, find_rom
    rom = Rom(find_rom())
    out = sys.argv[2] if len(sys.argv) > 2 else 'trainer_pics_out'
    r = rip_trainer_pics(rom, range(NUM_FRONT_PICS), out)
    b = rip_trainer_back_pics(rom, range(NUM_BACK_PICS), out)
    print('%d front pics, %d back pics -> %s' % (len(r), len(b), out))
