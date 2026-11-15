#!/usr/bin/env python3
"""CANALAVE LIBRARY's TV (Platinum's src/library_tv/library_tv.c), drawn from the user's ROM: the decomp doesn't hold
its archives. Platinum's main screen, three layers at one priority (the lower BG number on top):

    BG0  demo/intro/intro_tv.narc  chars 1, screen 4   the TV's frame (color 0: the backdrop, black: Bg_MaskPalette)
    BG1  demo/intro/intro_tv.narc  chars 2, screen 5   the scan lines, blended 4/16 over BG3 (G2_SetBlendAlpha BG1 ->
                                                       BG2|BG3, 4, 12), scrolled up a line every 4 frames
    BG3  graphic/library_tv.narc   chars 2, screen 4   the news picture (256 colors)

all in library_tv.narc's palette 3 (Graphics_LoadPalette at 0). Written as rendered layers for src/sinnoh/s3d_libtv.c:

    "SLTV" u16 width 256, u16 height 192, then three layers of 256 x 256 u16 (BGR555, bit 15 = drawn): the frame, the
    scan lines (the whole 256-line map: it scrolls round), the picture

    python3 -I tools/sinnoh/build_library_tv.py ROM OUT     (OUT: sinnoh/platinum_libtv.bin; never committed)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nds  # noqa: E402


def lz10(d):
    """the NitroSDK's LZ77 (type 0x10), when a member is packed"""
    size = d[1] | d[2] << 8 | d[3] << 16
    out, o = bytearray(), 4
    while len(out) < size:
        flags = d[o]
        o += 1
        for bit in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> bit):
                n = (d[o] >> 4) + 3
                disp = ((d[o] & 15) << 8 | d[o + 1]) + 1
                o += 2
                for _ in range(n):
                    out.append(out[-disp])
            else:
                out.append(d[o])
                o += 1
    return bytes(out)


def unpack(d):
    return lz10(d) if d[:1] == b'\x10' and d[:4] not in (b'RGCN', b'RCSN', b'RLCN') else d


def section(d, magic):
    """a Nitro file's block (NCGR's CHAR, NSCR's SCRN, NCLR's PLTT; stored reversed as in the files)"""
    tag = magic[::-1]
    o = struct.unpack_from('<H', d, 12)[0]
    while o + 8 <= len(d):
        if d[o:o + 4] == tag:
            return d[o:o + struct.unpack_from('<I', d, o + 4)[0]]
        o += struct.unpack_from('<I', d, o + 4)[0]
    raise ValueError('no %s' % magic)


def tiles(ncgr):
    c = section(ncgr, b'CHAR')
    depth = struct.unpack_from('<I', c, 12)[0]   # (3: 4 bits, 4: 8 bits)
    size, off = struct.unpack_from('<II', c, 24)
    raw = c[off + 8:off + 8 + size] if off else c[32:32 + size]
    if depth == 3:
        px = [v for b in raw for v in (b & 15, b >> 4)]
    else:
        px = list(raw)
    return [px[t * 64:t * 64 + 64] for t in range(len(px) // 64)], depth


def screen(nscr):
    s = section(nscr, b'SCRN')
    w, h, _, size = struct.unpack_from('<HHII', s, 8)
    return w, h, struct.unpack_from('<%dH' % (size // 2), s, 20)


def palette(nclr):
    p = section(nclr, b'PLTT')
    size, off = struct.unpack_from('<II', p, 16)
    raw = p[8 + off:8 + off + size] if off != 0x10 else p[24:24 + size]
    return list(struct.unpack_from('<%dH' % (len(raw) // 2), raw))


def layer(ncgr, nscr, pal, full_rows=192):
    """a BG layer as 256 x 256 BGR555 values, bit 15 where drawn (color 0 of a tile clear)"""
    ts, depth = tiles(ncgr)
    w, h, ents = screen(nscr)
    out = [0] * (256 * 256)
    for k, e in enumerate(ents):
        if w > 256:   # (a wider map is kept as 256 x 256 screen blocks side by side, as the DS reads it)
            blk, r = divmod(k, 1024)
            tx, ty = blk * 256 + (r % 32) * 8, (r // 32) * 8
        else:
            tx, ty = (k % (w // 8)) * 8, (k // (w // 8)) * 8
        if tx >= 256 or ty >= 256:
            continue
        t, hf, vf, pn = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1, e >> 12
        if t >= len(ts):
            continue
        for i, v in enumerate(ts[t]):
            if not v:
                continue
            x, y = i % 8, i // 8
            if hf:
                x = 7 - x
            if vf:
                y = 7 - y
            c = pal[(pn * 16 + v) if depth == 3 else v] if ((pn * 16 + v) if depth == 3 else v) < len(pal) else 0
            out[(ty + y) * 256 + tx + x] = (c & 0x7FFF) | 0x8000
    return out


def main():
    rom, out = sys.argv[1], sys.argv[2]
    r = nds.Rom(rom)
    tv = r.narc('demo/intro/intro_tv.narc')
    lib = r.narc('graphic/library_tv.narc')
    pal = palette(unpack(lib[3]))
    frame = layer(unpack(tv[1]), unpack(tv[4]), pal)
    lines = layer(unpack(tv[2]), unpack(tv[5]), pal)
    picture = layer(unpack(lib[2]), unpack(lib[4]), pal)
    data = b'SLTV' + struct.pack('<HH', 256, 192)
    for lay in (frame, lines, picture):
        data += struct.pack('<%dH' % len(lay), *lay)
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    open(out, 'wb').write(data)
    print('%s: the library TV (%d bytes)' % (out, len(data)))


if __name__ == '__main__':
    main()
