#!/usr/bin/env python3
"""The BATTLE FRONTIER's Scratch-Off Cards (Platinum's src/applications/scratch_off_cards), drawn from the user's ROM
(resource/eng/scratch/scratch.narc: the decomp doesn't hold its art) for src/sinnoh/s3d_scratch.c and for the mock-ups
(tools/sinnoh/mockups_scratch.py). Every picture is rendered as Platinum's screens show it:

    0       the top screen's picture while choosing (35 / 37, palette 34)
    1-4     the prize card (38 / 40) in each card's colours (UpdateBackgroundPalette: palette card + 1)
    5-8     the card on the touch screen (4 / 6) in each card's colours (palette card + 4)
    9       the film over the nine cells (7 / 9)
    10-13   the four cards to pick (sprites 12 / 11 / 13, animations 0-3)
    14-18   the POKéMON icons (32 / 31 / 33): the four prizes' and DITTO's (WILDCARD_CELL_VALUE 4)
    19-26   the boxes (16 / 15 / 17): CANCEL pressed 0-2, NEXT CARD pressed 3-5, the matching cell's box 6, the
            centre message box 7
    27-34   the matching cell's box in the eight flashing palettes (22-29, the box sprite's palette 3:
            HighlightMatchingPair / ScratchOffCards_UpdateBoxPalettes)
    35-43   the winning pop-up's nine frames (20 / 19 / 21)

The backgrounds are 256 x 192 (the first screen of a 512-wide map), the sprites their cells' size (their OAMs start at
the sprite's point). File: "SSCR" u16 pictures, then each: u16 width, u16 height, width * height u16 (BGR555, bit 15
where drawn).

    python3 -I tools/sinnoh/build_scratch.py ROM OUT     (OUT: sinnoh/platinum_scratch.bin; never committed)
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import nds  # noqa: E402
import build_library_tv as L  # noqa: E402

OAM_SIZES = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
             (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}


class Pic:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [0] * (w * h)


def background(ncgr, nscr, pal, force=None):
    """the map's first 256 x 192, every tile in palette FORCE when given (Bg_ChangeTilemapRectPalette)"""
    ts, depth = L.tiles(ncgr)
    w, _, ents = L.screen(nscr)
    p = Pic(256, 192)
    for k, e in enumerate(ents):
        if w > 256:
            blk, r = divmod(k, 1024)
            tx, ty = blk * 256 + (r % 32) * 8, (r // 32) * 8
        else:
            tx, ty = (k % (w // 8)) * 8, (k // (w // 8)) * 8
        if tx >= 256 or ty >= 192:
            continue
        t, hf, vf = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1
        pn = force if force is not None else e >> 12
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
            k2 = (pn * 16 + v) if depth == 3 else v
            p.px[(ty + y) * 256 + tx + x] = (pal[k2] & 0x7FFF if k2 < len(pal) else 0) | 0x8000
    return p


def cells(ncgr, ncer, pal, force=None):
    """every cell of a sprite bank as a picture (the OAMs' palettes, or FORCE: Sprite_SetExplicitPaletteWithOffset)"""
    ts, _ = L.tiles(ncgr)
    c = L.section(ncer, b'CEBK')
    n, attrs, off, mapping = struct.unpack_from('<HHII', c, 8)
    shift = mapping & 3
    base = 8 + off
    step = 16 if attrs & 1 else 8
    out = []
    for i in range(n):
        cnt, _, ooff = struct.unpack_from('<HHI', c, base + i * step)
        oams = []
        for k in range(cnt):
            a0, a1, a2 = struct.unpack_from('<HHH', c, base + n * step + ooff + 6 * k)
            y = (a0 & 0xFF) - (256 if a0 & 0x80 else 0)
            x = (a1 & 0x1FF) - (512 if a1 & 0x100 else 0)
            oams.append((x, y, OAM_SIZES[(a0 >> 14, a1 >> 14)], (a2 & 0x3FF) << shift, a2 >> 12,
                         bool(a1 & 0x1000), bool(a1 & 0x2000)))
        w = max([o[0] + o[2][0] for o in oams] or [1])
        h = max([o[1] + o[2][1] for o in oams] or [1])
        p = Pic(w, h)
        for x, y, (ow, oh), t0, pn, hf, vf in reversed(oams):   # (the first OAM in front)
            if force is not None:
                pn = force
            for ty in range(oh // 8):
                for tx in range(ow // 8):
                    t = t0 + ty * (ow // 8) + tx
                    if t >= len(ts):
                        continue
                    for i2, v in enumerate(ts[t]):
                        if not v:
                            continue
                        X, Y = tx * 8 + i2 % 8, ty * 8 + i2 // 8
                        if hf:
                            X = ow - 1 - X
                        if vf:
                            Y = oh - 1 - Y
                        k = pn * 16 + v
                        p.px[(y + Y) * w + x + X] = (pal[k] & 0x7FFF if k < len(pal) else 0) | 0x8000
        out.append(p)
    return out


def pictures(rom):
    n = nds.Rom(rom).narc('resource/eng/scratch/scratch.narc')
    U = lambda i: L.unpack(n[i])  # noqa: E731
    top, bottom = L.palette(U(34)), L.palette(U(0))
    pics = [background(U(35), U(37), top)]
    pics += [background(U(38), U(40), top, card + 1) for card in range(4)]
    pics += [background(U(4), U(6), bottom, card + 4) for card in range(4)]
    pics.append(background(U(7), U(9), bottom))
    pics += cells(U(12), U(11), L.palette(U(13)))[:4]
    pics += cells(U(32), U(31), L.palette(U(33)))[:5]
    pics += cells(U(16), U(15), L.palette(U(17)))[:8]
    pics += [cells(U(16), U(15), L.palette(U(m)), 3)[6] for m in range(22, 30)]
    pics += cells(U(20), U(19), L.palette(U(21)))[:9]
    assert len(pics) == 44, len(pics)
    return pics


def main():
    rom, out = sys.argv[1], sys.argv[2]
    pics = pictures(rom)
    data = bytearray(b'SSCR' + struct.pack('<H', len(pics)))
    for p in pics:
        data += struct.pack('<HH', p.w, p.h) + struct.pack('<%dH' % len(p.px), *p.px)
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    open(out, 'wb').write(data)
    print('%s: the Scratch-Off Cards (%d pictures, %d bytes)' % (out, len(pics), len(data)))


if __name__ == '__main__':
    main()
