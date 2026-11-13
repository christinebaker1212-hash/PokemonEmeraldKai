#!/usr/bin/env python3
"""The CO-OP menu's row icons: the game's own item pictures (24x24), brought to 16x16 as pixel art.

Each item's picture is cropped to what it draws; one that already fits in 16x16 is only centred. A bigger one is
area-averaged down, every pixel put back to one of the item's own palette colors, and the shape outlined again in its
darkest color, so it reads like a hand-made small icon rather than a blurred one. Output: 4bpp 16x16 tiles (2x2, the
order a 16x16 sprite reads them) and each icon's own 16-color palette, src/data/mp_coopmenu_icons.h.

usage: python3 tools/moemon_coopmenu/gen_icons.py   (from the repository root)
"""
import os
import sys
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
# (row, item picture) in the order mp_menu.c's MP_COOPICON_* names them
ICONS = [('FRIENDS', 'fame_checker'), ('SOCIAL', 'letter'), ('TRADE', 'linking_cord'), ('SHARED PC', 'devon_goods'),
         ('TOURNAMENT', 'battle_flag'), ('MOEGATCHA', 'coin_case'), ('DAILY', 'contest_pass'),
         ('BERRY GAMES', 'berry_pouch'), ('LOOK', 'silk_scarf'), ('MOVE TUTOR', 'heart_scale'), ('ONLINE', 'vs_seeker'),
         # the A-Life pick's (mp_menu.c MP_OpenVerbWheel): drawn ones, then items
         ('PET', '@hand'), ('GROOM', '@brush'), ('PLAY', 'poke_ball'), ('FEED', 'oran_berry'), ('KEEPSAKE', 'soothe_bell')]

# Drawn icons (16x16, a dark outline): a hand for PET, a brush for GROOM
DRAWN = {
    'hand': (["................",
              "......XX.XX.....",
              "..XX..XoXXoX....",
              "..XoX.XoXXoX.XX.",
              "..XoX.XoXXoXXoX.",
              "...XoXXoXXoXXoX.",
              "...XoXXoooooXoX.",
              "....XoooooooooX.",
              ".XX.XooooooooX..",
              ".XoXXooooooooX..",
              "..XoooooooooX...",
              "...XooooooooX...",
              "....XoooooooX...",
              ".....XooooX.....",
              "......XXXX......",
              "................"], {'X': (72, 72, 72), 'o': (248, 208, 168)}),
    'brush': (["................",
               "..........XX....",
               ".........XooX...",
               "........XooX....",
               ".......XooX.....",
               "......XooX......",
               ".....XooX.......",
               "....XXXX........",
               "...XyyyyX.......",
               "..XyyyyyyX......",
               ".XyyyyyyyX......",
               ".XwwwwwwX.......",
               ".XwXwXwX........",
               ".XwXwXX.........",
               "..XXXX..........",
               "................"], {'X': (72, 72, 72), 'o': (184, 120, 64), 'y': (232, 184, 72), 'w': (248, 248, 240)}),
}


def drawn(name):
    rows, cols = DRAWN[name]
    pal = [(0, 0, 0)] + list(cols.values())
    index = {c: i + 1 for i, c in enumerate(cols)}
    pix = [[index.get(ch, 0) for ch in row] for row in rows]
    return pix, (pal + [(0, 0, 0)] * 16)[:16]


def jasc(path):
    lines = open(path).read().split('\n')[3:]
    return [tuple(int(v) for v in ln.split()[:3]) for ln in lines if ln.strip()]


def load(name):
    """The item's picture as (indices image, its 16 colors)"""
    im = Image.open(os.path.join(ROOT, 'graphics/items/icons/%s.png' % name))
    pp = os.path.join(ROOT, 'graphics/items/icon_palettes/%s.pal' % name)
    if os.path.exists(pp):
        pal = jasc(pp)
    else:
        raw = im.getpalette()[:48]
        pal = [tuple(raw[i:i + 3]) for i in range(0, 48, 3)]
    if im.mode != 'P':
        raise SystemExit('%s: not an indexed picture' % name)
    return im, (pal + [(0, 0, 0)] * 16)[:16]


def nearest(c, pal):
    best, bi = None, 1
    for i in range(1, 16):
        q = pal[i]
        d = (q[0] - c[0]) ** 2 * 3 + (q[1] - c[1]) ** 2 * 4 + (q[2] - c[2]) ** 2 * 2
        if best is None or d < best:
            best, bi = d, i
    return bi


def small(name):
    im, pal = load(name)
    w0, h0 = im.size
    px = im.load()
    solid = [(x, y) for y in range(h0) for x in range(w0) if px[x, y] != 0]
    x0, y0 = min(p[0] for p in solid), min(p[1] for p in solid)
    x1, y1 = max(p[0] for p in solid) + 1, max(p[1] for p in solid) + 1
    w, h = x1 - x0, y1 - y0
    out = [[0] * 16 for _ in range(16)]
    if w <= 16 and h <= 16:
        ox, oy = (16 - w) // 2, (16 - h) // 2
        for y in range(h):
            for x in range(w):
                out[oy + y][ox + x] = px[x0 + x, y0 + y]
        return out, pal
    used = set(px[x, y] for (x, y) in solid)
    outline = min(used, key=lambda i: sum(pal[i]))
    k = 16.0 / max(w, h)
    nw, nh = max(1, round(w * k)), max(1, round(h * k))
    rgb = Image.new('RGB', (w, h))
    alpha = Image.new('L', (w, h))
    for y in range(h):
        for x in range(w):
            i = px[x0 + x, y0 + y]
            if i:
                rgb.putpixel((x, y), pal[i])
                alpha.putpixel((x, y), 255)
    srgb, sa = rgb.resize((nw, nh), Image.BOX), alpha.resize((nw, nh), Image.BOX)
    ox, oy = (16 - nw) // 2, (16 - nh) // 2
    filled = set()
    for y in range(nh):
        for x in range(nw):
            a = sa.getpixel((x, y))
            if a >= 110:
                c = tuple(min(255, int(v * 255 / a)) for v in srgb.getpixel((x, y)))
                out[oy + y][ox + x] = nearest(c, pal)
                filled.add((x, y))
    for (x, y) in filled:
        if any((x + dx, y + dy) not in filled for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            out[oy + y][ox + x] = outline
    return out, pal


def tiles(pix):
    data = []
    for ty in range(2):
        for tx in range(2):
            for y in range(8):
                for x in range(0, 8, 2):
                    a = pix[ty * 8 + y][tx * 8 + x]
                    b = pix[ty * 8 + y][tx * 8 + x + 1]
                    data.append(a | (b << 4))
    return data


def rgb555(c):
    return (c[0] >> 3) | ((c[1] >> 3) << 5) | ((c[2] >> 3) << 10)


def main():
    out = ['// Generated by tools/moemon_coopmenu/gen_icons.py - do not edit',
           '// The CO-OP menu\'s row icons: the game\'s own item pictures as 16x16 pixel art (4bpp, 2x2 tiles), each with',
           '// its item\'s own palette.',
           '#define MP_COOPICON_COUNT %d' % len(ICONS), '']
    out.append('static const u8 sCoopIconTiles[MP_COOPICON_COUNT][128] = {')
    pals = []
    for row, item in ICONS:
        pix, pal = drawn(item[1:]) if item.startswith('@') else small(item)
        pals.append(pal)
        t = tiles(pix)
        out.append('    {   // %s: %s' % (row, item.upper().lstrip('@')))
        for i in range(0, 128, 16):
            out.append('        ' + ', '.join('0x%02X' % v for v in t[i:i + 16]) + ',')
        out.append('    },')
    out.append('};')
    out.append('')
    out.append('static const u16 sCoopIconPals[MP_COOPICON_COUNT][16] = {')
    for (row, item), pal in zip(ICONS, pals):
        out.append('    {' + ', '.join('0x%04X' % rgb555(c) for c in pal) + '},   // %s' % row)
    out.append('};')
    path = os.path.join(ROOT, 'src/data/mp_coopmenu_icons.h')
    open(path, 'w').write('\n'.join(out) + '\n')
    print('wrote', path)


if __name__ == '__main__':
    main()
