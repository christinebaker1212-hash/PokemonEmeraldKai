#!/usr/bin/env python3
"""SINNOH's people as Platinum draws them: its overworld sprites and trainers' battle pictures, from pret/pokeplatinum
(res/graphics/field_sprites, res/trainers/classes; tools/sinnoh/fetch_decomp.sh), into sinnoh/platinum_gfx.pak beside
the 3D pack (never committed; the release builds and ships it). src/sinnoh/s3d_gfx.c reads it; without it the game
shows its own stand-ins.

    python3 -I tools/sinnoh/build_gfx.py POKEPLATINUM_DIR OUT.pak
    python3 -I tools/sinnoh/build_gfx.py --names POKEPLATINUM_DIR   (include/constants/sinnoh_gfx.h: the names only;
        src/data/sinnoh_gfx_standins.h: each one's stand-in; then gen_maps.py)

Overworld sprites: Platinum's walking sheets (32x32, 16 frames: up, down, left, right, four each: stand, step, stand,
step) become the game's nine (down, up, left standing, then two steps each; right is left turned round), so the
game's own walking animations run them. Still sprites (32x32 and 32 wide, 16x16, 64 wide: the big legendaries) are
their first frame. Sheets of any other shape (the grunts in a row) aren't taken: their people keep the stand-ins.
Battle pictures: each trainer class's front picture, its first 80x80 frame, drawn whole by the big-sprite renderer
(mp_bigsprite.c) over the stand-in's place.

Format (little endian): "S3DG" u32 version 2 (then "TOWN", the TOWN MAP; "STRT", the briefcase: build_starter.py; "PKCH")
  u16 sprites: each char name[24] (Platinum's, lower case), u8 kind (0 walking: 9 frames 32x32, 1 still 32x32,
               2 still 16x16, 3 walking with a bow: 10 frames, the POKeMON CENTER nurse), u8 pad, u16 palette[16]
               (BGR555; 0 transparent), then the frames as GBA 4bpp tiles (4: still 64x64, the big legendaries); then the smaller copy (OPTIONS >
               DISPLAY > SPRITE SCALING > SINNOH PEOPLE: MIXED; shrink()): u16 palette[16], the same frames
  u16 pictures: each char name[24] (the class folder), u16 palette[16], then an mp_bigsprite frame:
               s16 dx, s16 dy, u16 w, u16 h, h rows of (w + 1) / 2 bytes (low nibble first)
  "BACK" u16 count: the partners' backs (backs()), each u16 picture, u16 palette[16], 4 x 2048 bytes of tiles
"""
import os
import struct
import sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from png8 import Image  # noqa: E402   (no Pillow on the release's runner)

VERSION = 2
SMALL = 0.84   # MIXED's Platinum sprites: about Gen 3's size (Emerald's people are 0.80-0.88 of Platinum's same ones)
SHEETS = ('npc', 'pokemon', 'objects', 'player')   # (player: only DAWN's and LUCAS's own, PLAYER_ONLY; after the rest, so
                                                    # the others keep their numbers)
PLAYER_ONLY = ('player_f', 'player_m')
# the game's own sprites stay for these (their breaking / cutting animations, the item ball's)
KEEP_OWN = {'rock_smash', 'cut_tree', 'strength_boulder', 'pokeball', 'ice_rock', 'moss_rock', 'surf_wake'}
# Platinum's frame for each of the game's nine: down, up, left standing; down x2, up x2, left x2 stepping
WALK = (4, 0, 8, 5, 7, 1, 3, 9, 11)


def bgr555(rgb):
    r, g, b = rgb
    return (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10


def palette(im):
    pal = im.getpalette() or []
    cols = [tuple(pal[i * 3:i * 3 + 3]) for i in range(min(16, len(pal) // 3))]
    cols += [(0, 0, 0)] * (16 - len(cols))
    return struct.pack('<16H', *(bgr555(c) for c in cols))


def tiles(im, x0, y0, w, h):
    """w x h pixels at (x0, y0) as GBA 4bpp tiles, row by row of tiles"""
    px = im.px
    out = bytearray()
    for ty in range(h // 8):
        for tx in range(w // 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    a = px(x0 + tx * 8 + x, y0 + ty * 8 + y) & 15
                    b = px(x0 + tx * 8 + x + 1, y0 + ty * 8 + y) & 15
                    out.append(a | b << 4)
    return bytes(out)


def rgb_of(im):
    pal = im.getpalette() or []
    return [tuple(pal[i * 3:i * 3 + 3]) if i * 3 + 2 < len(pal) else (0, 0, 0) for i in range(16)]


def catmull_rom(t):
    t = abs(t)
    return 1.5 * t ** 3 - 2.5 * t ** 2 + 1 if t < 1 else -0.5 * t ** 3 + 2.5 * t ** 2 - 4 * t + 2 if t < 2 else 0.0


def shrink(im, frames, w, h, src=None, scale=None):
    """The frames (their top-left corners) drawn SMALL times as big, smoothly (around the feet, the bottom middle): a
    Catmull-Rom filter widened for shrinking, a pixel kept where at least half of its weight is the sprite;
    then 15 colours picked for the result (k-means from the sprite's own), so edges and thin lines blend instead of
    dropping pixels. Returns (palette as 16 RGB, frames of colour indices w x h)"""
    cols = rgb_of(im)
    sw, sh = src or (w, h)                 # (the source frame's size: the same as the result's unless given)
    inv = 1.0 / (scale or SMALL)
    out = []
    for (fx, fy) in frames:
        rgba = []
        for oy in range(h):
            row = []
            for ox in range(w):
                # Catmull-Rom, widened for shrinking: sharp eyes and outlines, no dropped pixels
                cx, cy = (ox + 0.5 - w / 2) * inv + sw / 2, (oy + 0.5 - h) * inv + sh
                r = g = b = a = total = 0.0
                for sy in range(int(cy - 2 * inv) - 1, int(cy + 2 * inv) + 2):
                    wy = catmull_rom((sy + 0.5 - cy) / inv)
                    if not wy:
                        continue
                    for sx in range(int(cx - 2 * inv) - 1, int(cx + 2 * inv) + 2):
                        wx = catmull_rom((sx + 0.5 - cx) / inv)
                        if not wx:
                            continue
                        wgt = wx * wy
                        total += wgt
                        c = im.px(fx + sx, fy + sy) & 15 if 0 <= sx < sw and 0 <= sy < sh else 0
                        if c:
                            r += cols[c][0] * wgt
                            g += cols[c][1] * wgt
                            b += cols[c][2] * wgt
                            a += wgt
                row.append(tuple(max(0.0, min(255.0, v / a)) for v in (r, g, b)) if a >= total * 0.5 else None)
            rgba.append(row)
        out.append(rgba)
    # 15 colours: k-means over every frame's pixels, from the sprite's own colours
    pts = [c for f in out for row in f for c in row if c]
    centers = [cols[i] for i in range(1, 16)]
    for _ in range(8):
        sums = [[0.0, 0.0, 0.0, 0] for _ in centers]
        for c in pts:
            k = min(range(len(centers)), key=lambda j: (c[0] - centers[j][0]) ** 2 + (c[1] - centers[j][1]) ** 2
                    + (c[2] - centers[j][2]) ** 2)
            sm = sums[k]
            sm[0] += c[0]
            sm[1] += c[1]
            sm[2] += c[2]
            sm[3] += 1
        centers = [(sm[0] / sm[3], sm[1] / sm[3], sm[2] / sm[3]) if sm[3] else centers[j] for j, sm in enumerate(sums)]
    centers = [tuple(int(round(v)) for v in c) for c in centers]
    idx = []
    for f in out:
        rows = []
        for row in f:
            rows.append([0 if c is None else 1 + min(range(15), key=lambda j: (c[0] - centers[j][0]) ** 2
                                                     + (c[1] - centers[j][1]) ** 2 + (c[2] - centers[j][2]) ** 2)
                         for c in row])
        idx.append(rows)
    return [cols[0]] + centers, idx


def tiles_of(rows, w, h):
    out = bytearray()
    for ty in range(h // 8):
        for tx in range(w // 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    out.append(rows[ty * 8 + y][tx * 8 + x] | rows[ty * 8 + y][tx * 8 + x + 1] << 4)
    return bytes(out)


def sheets(root):
    """[(name, kind, image)] in a fixed order (by name within npc, pokemon, objects)"""
    out, seen = [], set()
    for d in SHEETS:
        folder = os.path.join(root, 'res/graphics/field_sprites', d)
        if not os.path.isdir(folder):
            continue
        for f in sorted(os.listdir(folder)):
            name = f[:-4]
            if not f.endswith('.png') or name in seen or name in KEEP_OWN or name.endswith('_unused'):
                continue
            if d == 'player' and name not in PLAYER_ONLY:
                continue
            im = Image(os.path.join(folder, f))
            if im.mode != 'P':
                continue
            w, h = im.size
            kind = 3 if w == 32 and h >= 544 else 0 if w == 32 and h >= 512 else 1 if w == 32 and h in (32, 64, 128) \
                else 2 if (w, h) == (16, 16) else 4 if w == 64 and h in (64, 128) else None   # (1, 4: the first frame)
            if kind is None:
                continue
            seen.add(name)
            out.append((name, kind, im))
    return out


def pictures(root):
    folder = os.path.join(root, 'res/trainers/classes')
    out = []
    for name in sorted(os.listdir(folder)):
        p = os.path.join(folder, name, 'front.png')
        if os.path.exists(p):
            im = Image(p)
            if im.mode == 'P' and im.size[0] == 80 and im.size[1] >= 80:
                out.append((name, im))
    return out


# Partners' backs (tag and partner battles beside them): the class folders with a back.png, but the players' (DAWN's and
# LUCAS's backs are Kai's LOOKs': kai_dawn, kai_lucas). Platinum's frames are 80x80, one pixel apart; the game's back
# slot takes four 64x64 (the LOOKs' backs: mp_avatar.c BackChoiceTiles), so each is drawn 0.8 times as big (around the
# feet, as the sprites' smaller copies) and four are kept: standing, the arm back, the throw, after it (five-frame
# backs keep 0, 1, 3, 4 as Red's do; BARRY's eight 0, 1, 3, 5)
BACK_FRAMES = {5: (0, 1, 3, 4), 8: (0, 1, 3, 5)}


def backs(root, pics):
    """[(picture number, palette, four 64x64 frames of colour indices)] for the partners' class folders"""
    out = []
    for n, (name, _) in enumerate(pics):
        p = os.path.join(root, 'res/trainers/classes', name, 'back.png')
        if 'player' in name or not os.path.exists(p):
            continue
        im = Image(p)
        count = (im.size[1] + 1) // 81
        if im.mode != 'P' or im.size[0] != 80 or count not in BACK_FRAMES:
            continue
        pal, frames = shrink(im, [(0, f * 81) for f in BACK_FRAMES[count]], 64, 64, src=(80, 80), scale=0.8)
        out.append((n, pal, frames))
    return out


def names_header(root):
    lines = ['// Generated by tools/sinnoh/build_gfx.py --names. Do not edit.',
             "// SINNOH's people as Platinum draws them (sinnoh/platinum_gfx.pak, src/sinnoh/s3d_gfx.c): overworld sprite",
             '// ids, and battle pictures by trainer class. The pack holds them in this order; the names are all that is here.',
             '#ifndef GUARD_CONSTANTS_SINNOH_GFX_H', '#define GUARD_CONSTANTS_SINNOH_GFX_H', '',
             '#define OBJ_EVENT_GFX_SINNOH_FIRST 0x400   // (object events keep 12 bits of a graphics id: to 0xFFF)']
    sh = sheets(root)
    for i, (name, kind, _) in enumerate(sh):
        lines.append('#define OBJ_EVENT_GFX_SINNOH_%-30s (OBJ_EVENT_GFX_SINNOH_FIRST + %d)' % (name.upper(), i))
    lines += ['#define SINNOH_SPRITE_COUNT %d' % len(sh), '']
    pics = pictures(root)
    for i, (name, _) in enumerate(pics):
        lines.append('#define SINNOH_PIC_%-30s %d' % (name.upper(), i))
    lines += ['#define SINNOH_PIC_COUNT %d' % len(pics), '#define SINNOH_PIC_NONE 0xFF', '', '#endif', '']
    return '\n'.join(lines)


def main():
    if sys.argv[1] == '--names':
        here = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
        open(os.path.join(here, 'include/constants/sinnoh_gfx.h'), 'w').write(names_header(sys.argv[2]))
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        import sprites
        open(os.path.join(here, 'src/data/sinnoh_gfx_standins.h'), 'w').write(
            '// Generated by tools/sinnoh/build_gfx.py --names. Do not edit.\n'
            "// Each Platinum sprite's stand-in (tools/sinnoh/sprites.py standin()), shown without sinnoh/platinum_gfx.pak\n"
            '#include "constants/qz_port.h"   // (JOHTO\'s / KANTO\'s GBA legendaries)\n'
            'static const u16 sSinnohStandIns[SINNOH_SPRITE_COUNT] = {\n' +
            ''.join('    %s,\n' % sprites.standin(name.upper()) for name, _, _ in sheets(sys.argv[2])) + '};\n')
        open(os.path.join(here, 'src/data/sinnoh_gfx_gen3.h'), 'w').write(
            '// Generated by tools/sinnoh/build_gfx.py --names. Do not edit.\n'
            "// Each Platinum sprite's true Gen 3 equivalent (tools/sinnoh/sprites.py GEN3; 0: none), for OPTIONS > DISPLAY >\n"
            '// SINNOH PEOPLE GEN 3 and MIXED\n'
            'static const u16 sSinnohGen3[SINNOH_SPRITE_COUNT] = {\n' +
            ''.join('    %s,\n' % sprites.gen3(name.upper()) for name, _, _ in sheets(sys.argv[2])) + '};\n')
        return
    root, out_path = sys.argv[1], sys.argv[2]
    out = bytearray(b'S3DG' + struct.pack('<I', VERSION))
    sh = sheets(root)
    out += struct.pack('<H', len(sh))
    for name, kind, im in sh:
        out += name.encode()[:23].ljust(24, b'\0') + struct.pack('<BB', kind, 0) + palette(im)
        if kind in (0, 3):
            for f in WALK + ((16,) if kind == 3 else ()):   # (3: the nurse's bow, the game's frame 9)
                out += tiles(im, 0, f * 32, 32, 32)
        elif kind == 1:
            out += tiles(im, 0, 0, 32, 32)
        elif kind == 4:
            out += tiles(im, 0, 0, 64, 64)
        else:
            out += tiles(im, 0, 0, 16, 16)
        # MIXED's smaller copy: its own palette, then the same frames
        if kind in (0, 3):
            where = [(0, f * 32) for f in WALK + ((16,) if kind == 3 else ())]
        else:
            where = [(0, 0)]
        size = 16 if kind == 2 else 64 if kind == 4 else 32
        pal, small = shrink(im, where, size, size)
        out += struct.pack('<16H', *(bgr555(c) for c in pal))
        for rows in small:
            out += tiles_of(rows, size, size)
    pics = pictures(root)
    out += struct.pack('<H', len(pics))
    for name, im in pics:
        out += name.encode()[:23].ljust(24, b'\0') + palette(im)
        px = im.px
        out += struct.pack('<hhHH', -40, -48, 80, 80)   # (feet where the game's 64x64 pictures have theirs)
        for y in range(80):
            for x in range(0, 80, 2):
                out.append((px(x, y) & 15) | (px(x + 1, y) & 15) << 4)
    # the partners' backs: "BACK", u16 count, each u16 picture number (the class's battle picture above), u16
    # palette[16], four 64x64 frames as GBA 4bpp tiles (the game's trainer back picture layout)
    bk = backs(root, pics)
    out += b'BACK' + struct.pack('<H', len(bk))
    for n, pal, frames in bk:
        out += struct.pack('<H', n) + struct.pack('<16H', *(bgr555(c) for c in pal))
        for rows in frames:
            out += tiles_of(rows, 64, 64)
    # the TOWN MAP's two pages (tools/sinnoh/gen_townmap.py picture: Platinum's town map art, scaled)
    import gen_townmap
    out += b'TOWN' + struct.pack('<3H', gen_townmap.TEX_W, gen_townmap.TEX_H, len(gen_townmap.PAGES))
    for k in range(len(gen_townmap.PAGES)):
        for row in gen_townmap.picture(root, k):
            out += struct.pack('<%dH' % len(row), *row)
    # the briefcase's starter screen, drawn ahead of time (tools/sinnoh/build_starter.py; before the POKéTCH's section,
    # which runs to the end)
    import build_starter
    if os.path.exists(os.path.join(root, build_starter.NARC)):
        out += build_starter.section(root)
    # SUPER CONTESTS' accessories and backdrops (tools/sinnoh/build_contest.py; before the POKéTCH's)
    import build_contest
    out += build_contest.section(root)
    # the Underground's digging screen (tools/sinnoh/build_mining.py; before the POKéTCH's)
    import build_mining
    out += build_mining.section(root)
    # Platinum's UNOWN font (tools/sinnoh/build_unown.py; before the POKéTCH's)
    import build_unown
    out += build_unown.section(root)
    # the POKéTCH's app screens and sprites (tools/sinnoh/build_poketch.py)
    import build_poketch
    out += build_poketch.section(root)
    open(out_path, 'wb').write(out)
    print('%s: %d sprites, %d battle pictures, %d bytes' % (out_path, len(sh), len(pics), len(out)))


if __name__ == '__main__':
    main()
