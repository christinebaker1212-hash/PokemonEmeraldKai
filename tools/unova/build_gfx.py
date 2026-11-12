#!/usr/bin/env python3
"""UNOVA's people as Black draws them: its field characters (billboards: archive 0x31, a/0/4/9, one texture a frame,
4bpp with one 16-colour palette; which one a person shows from the object code table, archive 0x30, a/0/4/8: 0x1C
bytes a code, its resource at +0x10; aletheia's fldmmdl.c) into unova/black_gfx.pak beside the 3D pack (never
committed; the release builds and ships it). src/unova/u3d_gfx.c reads it; without it the game shows stand-ins.

    python3 -I tools/unova/build_gfx.py OUT.pak

Black's walking sheets: up, down, left as a standing frame and a step (6 to 9 frames: most people), or three frames
each (12 and more: the main characters); the game's nine (down, up, left standing, then two steps each; right is
left turned round) are made from them, the second step of up and down the first turned round, as Black walks.
Anything else is still: its first frame. 64x64 sheets are still 64x64.

Battle pictures (version 2): Black's trainers' front pictures (archive 0x48, a/0/7/2: eight files a picture, LZ11;
aletheia's pokegra.c), each its cell 0 (NCER, 1D mapping: a tile number counts 64 bytes) of its 80x80 character sheet
(NCGR) in its palette (NCLR), drawn whole by the game's big-sprite renderer; which picture a trainer class shows is
arm9's TrainerGraIndexTable (0x020A023C, 0x6A classes).

Format (little endian): "U3DG" u32 version 2, u16 codes; each: u8 kind (0 walking: 9 frames 32x32, 1 still 32x32,
  4 still 64x64, 0xFF none), u8 pad, u16 palette[16] (BGR555, 0 transparent), the frames as GBA 4bpp tiles; then
  MIXED's smaller copy (OPTIONS > DISPLAY > SPRITE SCALING > UNOVA PEOPLE): u16 palette[16], the same frames
  then "TRPC", u16 classes, u8 the picture of each (padded to 2), u16 pictures; each: u16 palette[16] (0 transparent),
  s16 dx, dy (its top-left from the sprite's point), u16 w, h, rows of (w + 1) / 2 bytes (4 bits, the low nibble first)
  then "BACK", u16 pictures; each: u16 palette[16], four 64x64 frames as GBA 4bpp tiles (the game's trainer back
  picture layout): Black's backs (archive 0x49, a/0/7/3: the two player characters, JUNIPER, CHEREN...), cell 0
  over their 256x128 bitmap sheet (2D mapping: a tile number is a place 32 tiles a row), half its size, four times
"""
import importlib.util
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(os.path.dirname(HERE), 'sinnoh'))
import narc  # noqa: E402
import nitro  # noqa: E402

_spec = importlib.util.spec_from_file_location('sinnoh_build_gfx', os.path.join(os.path.dirname(HERE), 'sinnoh',
                                                                               'build_gfx.py'))
_sinnoh = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_sinnoh)
shrink, tiles_of = _sinnoh.shrink, _sinnoh.tiles_of   # (the smaller copy's filter: a tool, never the engines)

VERSION = 2
SMALL = 0.84          # MIXED: about Gen 3's size, as SINNOH's
CODES_ARC, BBD_ARC = 'a/0/4/8', 'a/0/4/9'
WALK3 = (3, 0, 6, 4, 5, 1, 2, 7, 8)    # (three a direction: up 0-2, down 3-5, left 6-8)


class Sheet:
    """Frames of colour indices side by side, as the shrink filter reads an image"""

    def __init__(self, frames, size, pal):
        self.frames, self.size_, self.pal = frames, size, pal

    def px(self, x, y):
        f, x = divmod(x, self.size_)
        return self.frames[f][y][x] if 0 <= f < len(self.frames) and 0 <= y < self.size_ else 0

    def getpalette(self):
        out = []
        for c in self.pal:
            out += [(c & 31) << 3, ((c >> 5) & 31) << 3, ((c >> 10) & 31) << 3]
        return out


def frames_of(t):
    """The sheet's frames as rows of colour indices, its palette (BGR555)"""
    pal_name = list(t.palettes)[0]
    pal = list(struct.unpack_from('<16H', t.d, t.pal_data + t.palettes[pal_name]))
    out = []
    for name in t.order:
        tx = t.textures[name]
        if tx.format != 3:
            return None, None
        at = t.tex_data + tx.offset
        rows = []
        for y in range(tx.height):
            row = []
            for x in range(0, tx.width, 2):
                b = t.d[at + (y * tx.width + x) // 2]
                row += [b & 15, b >> 4]
            rows.append(row)
        out.append(rows)
    return out, pal


def flip(rows):
    return [list(reversed(r)) for r in rows]


def sprite(t):
    """(kind, palette, frames) of a sheet, None when it isn't drawn here"""
    fr, pal = frames_of(t)
    if not fr:
        return None
    size = len(fr[0])
    n = len(fr)
    if size == 64:
        return 4, pal, [fr[2] if n >= 6 else fr[0]]
    if size != 32:
        return None
    if n >= 12:
        return 0, pal, [fr[i] for i in WALK3]
    if 6 <= n <= 9:
        up, up_step, down, down_step, left, left_step = fr[:6]
        return 0, pal, [down, up, left, down_step, flip(down_step), up_step, flip(up_step), left_step, left_step]
    return 1, pal, [fr[0]]


TRAINER_ARC = 'a/0/7/2'
GRA_TABLE, GRA_CLASSES, ARM9_BASE = 0x020A023C, 0x6A, 0x02004000
OAM_SIZES = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
             (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}


def _lz11(d):
    """The NitroSDK's LZ11 (type 0x11), as Black's files are packed"""
    if not d or d[0] != 0x11:
        return d
    size = d[1] | d[2] << 8 | d[3] << 16
    o, out = 4, bytearray()
    if size == 0:
        size, o = struct.unpack_from('<I', d, 4)[0], 8
    while len(out) < size and o < len(d):
        flags = d[o]
        o += 1
        for bit in range(8):
            if len(out) >= size or o >= len(d):
                break
            if not flags & (0x80 >> bit):
                out.append(d[o])
                o += 1
                continue
            b = d[o]
            ind = b >> 4
            if ind == 0:
                n = (((b & 0xF) << 4) | (d[o + 1] >> 4)) + 0x11
                disp = (((d[o + 1] & 0xF) << 8) | d[o + 2]) + 1
                o += 3
            elif ind == 1:
                n = (((b & 0xF) << 12) | (d[o + 1] << 4) | (d[o + 2] >> 4)) + 0x111
                disp = (((d[o + 2] & 0xF) << 8) | d[o + 3]) + 1
                o += 4
            else:
                n = ind + 1
                disp = (((b & 0xF) << 8) | d[o + 1]) + 1
                o += 2
            for _ in range(n):
                out.append(out[-disp])
    return bytes(out[:size])


def _chunk(d, magic):
    o = struct.unpack_from('<H', d, 0xC)[0]
    while o + 8 <= len(d):
        size = struct.unpack_from('<I', d, o + 4)[0]
        if d[o:o + 4] == magic:
            return o
        if size == 0:
            break
        o += size
    raise KeyError(magic)


def trainer_picture(files):
    """(palette[16], dx, dy, w, h, pixel rows) of a picture's cell 0"""
    ncgr, ncer, nclr = (_lz11(files[k]) for k in (0, 2, 7))
    o = _chunk(ncgr, b'RAHC')
    _, _, _, _, _, size, off = struct.unpack_from('<HHIIIII', ncgr, o + 8)
    chars = ncgr[o + 8 + off:o + 8 + off + size]
    o = _chunk(nclr, b'TTLP')
    _, _, size, off = struct.unpack_from('<IIII', nclr, o + 8)
    pal = list(struct.unpack_from('<16H', nclr, o + 8 + off))
    o = _chunk(ncer, b'KBEC')
    n, attrs, off, mapping = struct.unpack_from('<HHII', ncer, o + 8)
    base = o + 8 + off
    cnt, _, ooff = struct.unpack_from('<HHI', ncer, base)
    shift = {0: 0, 1: 1, 2: 2, 3: 3}.get(mapping, 0)
    oams = []
    for k in range(cnt):
        a0, a1, a2 = struct.unpack_from('<HHH', ncer, base + n * (16 if attrs & 1 else 8) + ooff + 6 * k)
        y = (a0 & 0xFF) - (256 if a0 & 0x80 else 0)
        x = (a1 & 0x1FF) - (512 if a1 & 0x100 else 0)
        oams.append((x, y, OAM_SIZES[(a0 >> 14, a1 >> 14)], (a2 & 0x3FF) << shift, bool(a1 & 0x1000), bool(a1 & 0x2000)))
    x0, y0 = min(o[0] for o in oams), min(o[1] for o in oams)
    x1, y1 = max(o[0] + o[2][0] for o in oams), max(o[1] + o[2][1] for o in oams)
    w, h = x1 - x0, y1 - y0
    px = [[0] * w for _ in range(h)]
    for x, y, (ow, oh), t0, hf, vf in reversed(oams):   # (the first OAM in front)
        for ty in range(oh // 8):
            for tx in range(ow // 8):
                t = t0 + ty * (ow // 8) + tx
                for py in range(8):
                    for qx in range(8):
                        k = t * 32 + py * 4 + qx // 2
                        if k >= len(chars):
                            continue
                        v = (chars[k] >> 4) if qx & 1 else chars[k] & 0xF
                        if not v:
                            continue
                        X, Y = tx * 8 + qx, ty * 8 + py
                        if hf:
                            X = ow - 1 - X
                        if vf:
                            Y = oh - 1 - Y
                        px[y - y0 + Y][x - x0 + X] = v
    rows = bytearray()
    for r in px:
        for c in range(0, w, 2):
            rows.append(r[c] | ((r[c + 1] if c + 1 < w else 0) << 4))
    pal[0] = 0
    return pal, x0, y0 - 8, w, h, bytes(rows)   # (8 up: the feet where the game's pictures have theirs)


BACK_ARC = 'a/0/7/3'


class BackSheet:
    """Back picture cells side by side (W x H each), as the shrink filter reads an image"""

    def __init__(self, cells, w, h, pal):
        self.cells, self.w, self.h, self.pal = cells, w, h, pal

    def px(self, x, y):
        f, x = divmod(x, self.w)
        return self.cells[f][y][x] if 0 <= f < len(self.cells) and 0 <= y < self.h else 0

    def getpalette(self):
        out = []
        for c in self.pal:
            out += [(c & 31) << 3, ((c >> 5) & 31) << 3, ((c >> 10) & 31) << 3]
        return out


def back_pictures():
    arc = narc.arc(BACK_ARC)
    out = bytearray(b'BACK' + struct.pack('<H', len(arc) // 8))
    for i in range(len(arc) // 8):
        files = arc[8 * i:8 * i + 8]
        ncbr, ncer, nclr = (_lz11(files[k]) for k in (1, 2, 7))
        o = _chunk(ncbr, b'RAHC')
        h, w, _, _, _, size, off = struct.unpack_from('<HHIIIII', ncbr, o + 8)
        sheet, width = ncbr[o + 8 + off:o + 8 + off + size], w * 8
        o = _chunk(nclr, b'TTLP')
        pal = list(struct.unpack_from('<16H', nclr, o + 8 + struct.unpack_from('<IIII', nclr, o + 8)[3]))
        pal[0] = 0
        o = _chunk(ncer, b'KBEC')
        n, attrs, off, _ = struct.unpack_from('<HHII', ncer, o + 8)
        base, esz = o + 8 + off, 16 if attrs & 1 else 8
        cells = []
        for c in range(1):   # (cell 0 is the whole figure; Black's others are its arm's pieces, laid over it)
            cnt, _, ooff = struct.unpack_from('<HHI', ncer, base + esz * c)
            oams = []
            for k in range(cnt):
                a0, a1, a2 = struct.unpack_from('<HHH', ncer, base + n * esz + ooff + 6 * k)
                y = (a0 & 0xFF) - (256 if a0 & 0x80 else 0)
                x = (a1 & 0x1FF) - (512 if a1 & 0x100 else 0)
                oams.append((x, y, OAM_SIZES[(a0 >> 14, a1 >> 14)], a2 & 0x3FF, bool(a1 & 0x1000)))
            cells.append(oams)
        while len(cells) < 4:
            cells.append(cells[0])
        # each cell on a 96 x 128 canvas, its point (0, 0) at (48, 72): the feet at the bottom middle
        cw, ch = 96, 128
        drawn = []
        for oams in cells:
            px = [[0] * cw for _ in range(ch)]
            for x, y, (ow, oh), t, hf in reversed(oams):
                sx, sy = (t % 32) * 8, (t // 32) * 8
                for yy in range(oh):
                    for xx in range(ow):
                        X, Y = sx + (ow - 1 - xx if hf else xx), sy + yy
                        k = Y * width + X
                        if k // 2 >= len(sheet):
                            continue
                        v = (sheet[k // 2] >> 4) if k & 1 else sheet[k // 2] & 0xF
                        cx, cy = 48 + x + xx, 72 + y + yy
                        if v and 0 <= cx < cw and 0 <= cy < ch:
                            px[cy][cx] = v
            drawn.append(px)
        spal, small = shrink(BackSheet(drawn, cw, ch, pal), [(k * cw, 0) for k in range(4)], 64, 64, src=(cw, ch),
                             scale=0.5)
        out += struct.pack('<16H', *[_sinnoh.bgr555(c) if k else 0 for k, c in enumerate(spal)])
        for f in small:
            out += tiles_of(f, 64, 64)
    return bytes(out), len(arc) // 8


def battle_pictures():
    arm = open(os.path.join(os.path.dirname(narc.EXTRACT), 'arm9_dec.bin'), 'rb').read()
    table = arm[GRA_TABLE - ARM9_BASE:GRA_TABLE - ARM9_BASE + GRA_CLASSES]
    arc = narc.arc(TRAINER_ARC)
    out = bytearray(b'TRPC' + struct.pack('<H', len(table)) + table + b'\0' * (len(table) % 2))
    out += struct.pack('<H', len(arc) // 8)
    for i in range(len(arc) // 8):
        pal, dx, dy, w, h, rows = trainer_picture(arc[8 * i:8 * i + 8])
        out += struct.pack('<16HhhHH', *pal, dx, dy, w, h) + rows
    return bytes(out), len(arc) // 8


def main():
    out_path = sys.argv[1]
    codes = narc.arc(CODES_ARC)[0]
    bbd = narc.arc(BBD_ARC)
    count = struct.unpack_from('<I', codes, 0)[0]
    pack = bytearray(b'U3DG' + struct.pack('<IH', VERSION, count))
    made = 0
    cache = {}
    for i in range(count):
        draw_type = codes[4 + 0x1C * i + 2]
        res = struct.unpack_from('<H', codes, 4 + 0x1C * i + 0x10)[0]
        s = None
        if draw_type == 1 and res < len(bbd):
            if res not in cache:
                try:
                    cache[res] = sprite(nitro.btx0(bbd[res]))
                except (AssertionError, KeyError, IndexError, struct.error):
                    cache[res] = None
            s = cache[res]
        if s is None:
            pack += struct.pack('<BB', 0xFF, 0)
            continue
        kind, pal, frames = s
        size = len(frames[0])
        pal0 = [0] + pal[1:]
        pack += struct.pack('<BB16H', kind, 0, *pal0)
        for f in frames:
            pack += tiles_of(f, size, size)
        spal, small = shrink(Sheet(frames, size, pal0), [(k * size, 0) for k in range(len(frames))], size, size,
                             scale=SMALL)
        pack += struct.pack('<16H', *[_sinnoh.bgr555(c) if k else 0 for k, c in enumerate(spal)])
        for f in small:
            pack += tiles_of(f, size, size)
        made += 1
    pics, npics = battle_pictures()
    pack += pics
    backs, nbacks = back_pictures()
    pack += backs
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    open(out_path, 'wb').write(bytes(pack))
    print('%s: %d of %d object codes drawn, %d battle pictures, %d backs, %d bytes'
          % (out_path, made, count, npics, nbacks, len(pack)))


if __name__ == '__main__':
    main()
