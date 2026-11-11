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

Format (little endian): "U3DG" u32 version 1, u16 codes; each: u8 kind (0 walking: 9 frames 32x32, 1 still 32x32,
  4 still 64x64, 0xFF none), u8 pad, u16 palette[16] (BGR555, 0 transparent), the frames as GBA 4bpp tiles; then
  MIXED's smaller copy (OPTIONS > DISPLAY > SPRITE SCALING > UNOVA PEOPLE): u16 palette[16], the same frames
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

VERSION = 1
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
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    open(out_path, 'wb').write(bytes(pack))
    print('%s: %d of %d object codes drawn, %d bytes' % (out_path, made, count, len(pack)))


if __name__ == '__main__':
    main()
