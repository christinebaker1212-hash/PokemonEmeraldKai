"""SINNOH's JOURNAL art for sinnoh/platinum_gfx.pak (build_gfx.py appends it): Platinum's own notebook,
res/prebuilt/graphic/f_note_gra.narc of pret/pokeplatinum (never committed), as journal_controller.c loads it: the
boy's (members 0 tilemap, 2 tiles, 4 palettes) and the girl's (1, 3, 5) 256x192 page, each page of the JOURNAL in its
own palette (JournalController_LoadNewPagePalette: page N's tiles take palette N).

Section "JRNL": u16 w, h; then for the boy and the girl w*h u8 color numbers (0-15, the tile's own) and 16 x 16 u16
palettes (RGB555 | 0x8000). src/sinnoh/s3d_journal.c draws a page as palette[page][color].
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tv_data import narc_members  # noqa: E402

NARC = 'res/prebuilt/graphic/f_note_gra.narc'


def section_of(data, magic):
    """A Nitro file's section MAGIC (its 4 bytes reversed in the file), from its header on"""
    o = struct.unpack_from('<H', data, 12)[0]
    while o + 8 <= len(data):
        tag, size = data[o:o + 4], struct.unpack_from('<I', data, o + 4)[0]
        if tag == magic[::-1]:
            return data[o:o + size]
        o += size
    raise ValueError('no %s section' % magic)


def tiles(ncgr):
    c = section_of(ncgr, b'CHAR')
    depth = struct.unpack_from('<I', c, 12)[0]   # (3: 4 bits a pixel)
    size, off = struct.unpack_from('<II', c, 24)
    raw = c[8 + off:8 + off + size] if off != 0x18 else c[0x20:0x20 + size]
    if depth != 3:
        raise ValueError('f_note_gra: 8-bit tiles')
    out = []
    for t in range(len(raw) // 32):
        px = []
        for b in raw[t * 32:t * 32 + 32]:
            px += [b & 15, b >> 4]
        out.append(px)
    return out


def palettes(nclr):
    p = section_of(nclr, b'PLTT')
    size, off = struct.unpack_from('<II', p, 16)
    raw = p[8 + off:8 + off + size]
    cols = [struct.unpack_from('<H', raw, 2 * i)[0] & 0x7FFF for i in range(len(raw) // 2)]
    cols += [0] * (256 - len(cols))
    return cols[:256]


def picture(nscr, ncgr):
    s = section_of(nscr, b'SCRN')
    w, h = struct.unpack_from('<HH', s, 8)
    size = struct.unpack_from('<I', s, 16)[0]
    ents = struct.unpack_from('<%dH' % (size // 2), s, 20)
    ts = tiles(ncgr)
    mw = w // 8
    out = bytearray(w * h)
    for ty in range(h // 8):
        for tx in range(mw):
            e = ents[ty * mw + tx]
            n, hf, vf = e & 0x3FF, e >> 10 & 1, e >> 11 & 1
            if n >= len(ts):
                continue
            t = ts[n]
            for y in range(8):
                for x in range(8):
                    sx, sy = 7 - x if hf else x, 7 - y if vf else y
                    out[(ty * 8 + y) * w + tx * 8 + x] = t[sy * 8 + sx]
    return w, h, out


def section(root):
    path = os.path.join(root, NARC)
    if not os.path.exists(path):
        return b''
    m = narc_members(path)
    out = bytearray(b'JRNL')
    for k, (scr, chr_, pal) in enumerate(((0, 2, 4), (1, 3, 5))):
        w, h, px = picture(m[scr], m[chr_])
        if k == 0:
            out += struct.pack('<HH', w, h)
        out += px
        out += struct.pack('<256H', *(0x8000 | c for c in palettes(m[pal])))
    return bytes(out)


if __name__ == '__main__':
    s = section(sys.argv[1])
    print('JRNL: %d bytes' % len(s))
    if len(sys.argv) > 2 and s:   # (a look at the boy's first page)
        import zlib
        w, h = struct.unpack_from('<HH', s, 4)
        px = s[8:8 + w * h]
        pal = struct.unpack_from('<256H', s, 8 + w * h)
        raw = b''
        for y in range(h):
            raw += b'\0'
            for x in range(w):
                c = pal[px[y * w + x]]
                raw += bytes(((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3))

        def chunk(t, d):
            return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d))
        open(sys.argv[2], 'wb').write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)) +
                                      chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))
