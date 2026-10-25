#!/usr/bin/env python3
"""The CANDY JAR screen from the user's mock-up (jar_mockup.png, 240x160).

Writes src/mp/mp_candyjar_gfx.h:
- the background (BG3): the mock-up with the jar, its lid, the list's text and the message box's text taken out
  (the jar's place gets the panel's own speckled fill, the list's and message box's insides their plain color);
- the jar's glass (BG1, so it can shake on its own): the mock-up's jar below the lid, its three sample candies
  painted out with the glass around them (the rows above the floor from the clear glass higher up, the floor's rows
  from its middle, between the candies);
- the lid (a 64x32 sprite, so it can fly off): its rows 25-40;
- the glass's map for the candies' physics (0 outside, 1 outline, 2 glass), its place and size, the neck.
The background and the glass are full-color pictures (one table of the mock-up's own colors, in 15-bit color):
mp_candy.c draws them under the game's layers (MP_Candy_Composite), not bound by the GBA's 16-color palettes.
The lid keeps a sprite palette: more than 15 colors merge the closest.

usage: tools/moemon_candyjar/gen.py [--preview OUT.png]  (from the repo root)
"""
import os, sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, '..', '..'))
OUT = os.path.join(REPO, 'src', 'mp', 'mp_candyjar_gfx.h')

W, H = 240, 160
JX0, JY0, JW, JH = 16, 24, 80, 96          # the jar's box on the screen
LID_TOP, LID_BOTTOM = 25, 40               # the lid's rows (40: its bottom edge, inside the neck)
GLASS_TOP = 41                             # the mouth's first row of glass
NECK_L, NECK_R = 34, 76                    # the neck's inside (screen x)
LID_SX, LID_SY = 24, 24                    # the lid sprite's top left (64x32)
PANEL_FILL_X = range(17, 23)               # the left panel's inside, left of the jar: its speckled fill
LIST_IN = (119, 8, 232, 134)               # the list's white inside (inclusive)
MSG_IN = (4, 146, 235, 159)                # the message box's cream inside


def is_dark(p):
    return max(p) < 90


def main():
    src = Image.open(os.path.join(HERE, 'jar_mockup.png')).convert('RGB')
    assert src.size == (W, H)
    px = [[src.getpixel((x, y)) for x in range(W)] for y in range(H)]

    def is_panel(p):         # the left panel's speckled green (the glass is bluish, its outline dark)
        return p[1] > p[0] + 25 and p[1] >= p[2] and max(p) > 130

    # --- the jar's outline per row: its first and last pixel that isn't the panel's (rows 40-118; a lighter pixel in
    # the outline, as at row 49's left, still counts as the outline)
    walls = {}
    for y in range(LID_BOTTOM, 119):
        xs = [x for x in range(20, 93) if not is_panel(px[y][x])]
        walls[y] = (xs[0], xs[-1])

    # --- the sample candies (measured: their outline included); nothing of the glass's walls is that close
    candies = [(34.5, 110.0, 6.5), (47.0, 111.3, 6.5), (73.3, 110.3, 6.5)]

    def in_candy(x, y):
        return any((x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2 <= r * r for cx, cy, r in candies)

    # the glass's inside per row: between the wall runs
    def wall_run(y):
        l, r = walls[y]
        while l + 1 < r and is_dark(px[y][l + 1]):
            l += 1
        while r - 1 > l and is_dark(px[y][r - 1]):
            r -= 1
        return l, r          # the last wall pixels; the inside is (l, r) exclusive

    jar = {}                 # (x, y) -> color of the clean jar (glass layer)
    role = {}
    for y in range(LID_BOTTOM, 119):
        L, R = walls[y]
        if y == LID_BOTTOM:  # under the lid only the neck's sides; the mouth is open
            for x in (L, R):
                jar[(x, y)] = px[y][x]
                role[(x, y)] = 1
            continue
        wl, wr = wall_run(y) if y < 118 else (R, R)
        for x in range(L, R + 1):
            p = px[y][x]
            inside = wl < x < wr
            if inside and in_candy(x, y):
                p = px[102][x] if y <= 108 else px[y][60]
            jar[(x, y)] = p
            role[(x, y)] = 2 if inside and not is_dark(p) else 1

    # --- the lid: rows 25-39 between their outline, row 40 inside the neck
    lid = {}
    for y in range(LID_TOP, LID_BOTTOM + 1):
        if y < LID_BOTTOM:   # from its first to its last pixel that isn't the panel's (its soft edge included)
            xs = [x for x in range(24, 88) if not is_panel(px[y][x])]
            l, r = xs[0], xs[-1]
        else:
            l, r = walls[y][0] + 1, walls[y][1] - 1
        for x in range(l, r + 1):
            lid[(x, y)] = px[y][x]

    # --- the background: the jar's place filled, the list's and message's insides plain
    bg = [row[:] for row in px]
    for y in range(LID_TOP, 119):
        for x in range(JX0, JX0 + JW):
            if (x, y) in jar or (x, y) in lid or (y >= LID_BOTTOM and walls[y][0] <= x <= walls[y][1]):
                bg[y][x] = px[y][PANEL_FILL_X[(x * 5 + y * 3) % len(PANEL_FILL_X)]]
    for (x0, y0, x1, y1), fill in ((LIST_IN, px[70][230]), (MSG_IN, px[152][230])):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                bg[y][x] = fill

    def c15(p):
        return (p[0] >> 3, p[1] >> 3, p[2] >> 3)

    def dist(a, b):
        return sum((a[k] - b[k]) ** 2 for k in range(3))

    # --- one color table for both pictures (15-bit colors; entry 0 = nothing, for the glass's outside)
    colors = [None]
    cindex = {}

    def ci(p):
        c = c15(p)
        if c not in cindex:
            cindex[c] = len(colors)
            colors.append(c)
        return cindex[c]

    bg_idx = [[ci(bg[y][x]) for x in range(W)] for y in range(H)]
    jar_idx = [[ci(jar[(x, y)]) if (x, y) in jar else 0 for x in range(JX0, JX0 + JW)] for y in range(JY0, JY0 + JH)]
    assert len(colors) <= 256, '%d colors' % len(colors)

    # --- the lid sprite (64x32, one palette)
    lid_cols = {}
    for p in lid.values():
        lid_cols[c15(p)] = lid_cols.get(c15(p), 0) + 1
    while len(lid_cols) > 15:
        cs = list(lid_cols)
        a, b = min(((a, b) for i, a in enumerate(cs) for b in cs[i + 1:]), key=lambda ab: dist(*ab))
        if lid_cols[a] > lid_cols[b]:
            a, b = b, a
        lid_cols[b] += lid_cols.pop(a)
        for k, p in lid.items():
            if c15(p) == a:
                lid[k] = tuple(v << 3 for v in b)
    lid_pal = sorted(lid_cols)
    lid_idx = {c: i + 1 for i, c in enumerate(lid_pal)}
    lid_bytes = bytearray()
    for ty in range(4):
        for tx in range(8):
            for j in range(8):
                for i in range(0, 8, 2):
                    v = [0, 0]
                    for k in range(2):
                        q = (LID_SX + tx * 8 + i + k, LID_SY + ty * 8 + j)
                        v[k] = lid_idx[c15(lid[q])] if q in lid else 0
                    lid_bytes.append(v[0] | (v[1] << 4))

    glass_area = sum(1 for (x, y), r in role.items() if r == 2 and y >= GLASS_TOP)

    def rgb(c):
        return c[0] | (c[1] << 5) | (c[2] << 10)

    def u32s(b):
        return ['0x%08X' % int.from_bytes(b[i:i + 4], 'little') for i in range(0, len(b), 4)]

    def block(items, per):
        return '\n'.join('    ' + ', '.join(items[i:i + per]) + ',' for i in range(0, len(items), per))

    out = ['// Generated by tools/moemon_candyjar/gen.py from tools/moemon_candyjar/jar_mockup.png: do not edit.',
           '// The CANDY JAR screen: background (BG3) and glass (BG1) tiles, maps and palettes, the lid sprite,',
           '// the glass for the candies\' physics (0 outside, 1 outline, 2 glass).',
           '#define CJ_JAR_X %d' % JX0, '#define CJ_JAR_Y %d' % JY0, '#define CJ_JAR_W %d' % JW, '#define CJ_JAR_H %d' % JH,
           '#define CJ_LID_ROWS %d   // the box\'s rows above the glass (the lid\'s)' % (GLASS_TOP - JY0),
           '#define CJ_NECK_L %d' % (NECK_L - JX0), '#define CJ_NECK_R %d' % (NECK_R - JX0),
           '#define CJ_GLASS_AREA %d' % glass_area,
           '#define CJ_LID_X %d   // the lid sprite\'s center' % (LID_SX + 32), '#define CJ_LID_Y %d' % (LID_SY + 16),
           '#define CJ_LID_BOTTOM %d' % LID_BOTTOM,
           '#define CJ_COLOR_COUNT %d' % len(colors), '']
    out.append('static const u16 sCJ_Colors[CJ_COLOR_COUNT] = {\n%s\n};' % block(
        ['0x0000'] + ['0x%04X' % rgb(c) for c in colors[1:]], 12))
    out.append('static const u8 sCJ_Bg[160][240] = {\n%s\n};' % '\n'.join(
        '    {' + ','.join(str(v) for v in row) + '},' for row in bg_idx))
    out.append('static const u8 sCJ_Jar[CJ_JAR_H][CJ_JAR_W] = {\n%s\n};' % '\n'.join(
        '    {' + ','.join(str(v) for v in row) + '},' for row in jar_idx))
    out.append('static const u16 sCJ_LidPal[16] = {%s};' % ', '.join(
        ['0x0000'] + ['0x%04X' % rgb(c) for c in lid_pal] + ['0x0000'] * (15 - len(lid_pal))))
    out.append('static const u32 sCJ_LidTiles[64 * 32 / 8] = {\n%s\n};' % block(u32s(bytes(lid_bytes)), 8))
    rows = []
    for y in range(JY0, JY0 + JH):
        rows.append('    {' + ','.join(str(role.get((x, y), 0)) for x in range(JX0, JX0 + JW)) + '},')
    out.append('static const u8 sCJ_JarRole[CJ_JAR_H][CJ_JAR_W] = {\n%s\n};' % '\n'.join(rows))
    open(OUT, 'w').write('\n'.join(out) + '\n')
    print('%s: %d colors, lid %d colors, candies %s, glass %d px' % (
        os.path.relpath(OUT, REPO), len(colors) - 1, len(lid_pal),
        ', '.join('(%.1f,%.1f r%.1f)' % c for c in candies), glass_area))

    if len(sys.argv) > 2 and sys.argv[1] == '--preview':
        # what the game should show with the lid on and no candies: background, glass, lid
        im = Image.new('RGB', (W, H))
        for y in range(H):
            for x in range(W):
                j = (x - JX0, y - JY0)
                k = jar_idx[j[1]][j[0]] if 0 <= j[0] < JW and 0 <= j[1] < JH else 0
                c = colors[k] if k else colors[bg_idx[y][x]]
                im.putpixel((x, y), tuple(v << 3 for v in c))
        nolid = im.copy()
        for (x, y), p in lid.items():
            im.putpixel((x, y), tuple((v >> 3) << 3 for v in p))
        im.save(sys.argv[2])
        nolid.save(sys.argv[2].replace('.png', '_nolid.png'))


if __name__ == '__main__':
    main()
