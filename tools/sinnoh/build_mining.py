"""The Underground's digging art for sinnoh/platinum_gfx.pak (build_gfx.py appends it): Platinum's own mining screen
from pret/pokeplatinum's res/graphics/mining (never committed), as RGB555 pictures (src/sinnoh/s3d_mining.c).

Section "MINE": u16 count, then count pictures, each u16 w, h, then w*h RGB555 (0x8000 set: a drawn pixel; 0: clear),
in this order:
  0      the screen (interface.NSCR over interface_tiles.png, 256x192: Platinum's BG0, the bottom layer)
  1      the crack strip (the screen's top four tile rows, 208x32, every column cracked: Mining_DrawWallCrack's tiles;
         the game shows its right-hand columns as the wall weakens)
  2-5    the HAMMER button pressed, unpressed, the PICKAXE pressed, unpressed (48x64, Mining_DrawButton's tiles)
  6-12   the dirt, layers 0-6 (16x16 each, Mining_DrawDirt's tiles; layer 0 clear)
  13-17  the hit sprites (32x32: the hammer's two frames, the pickaxe's two, the spark: animations.png)
  18-    the buried objects in gSinnohMiningObjects' order (contest_data.mining_objects), w*16 x h*16
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import png8  # noqa: E402

MINING = 'res/graphics/mining'


def bgr555(rgb):
    r, g, b = rgb[:3]
    return 0x8000 | (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10


def pic(w, h, colors):
    return struct.pack('<HH', w, h) + struct.pack('<%dH' % (w * h), *colors)


def jasc(path):
    lines = open(path).read().split('\n')
    n = int(lines[2])
    return [tuple(int(v) for v in lines[3 + i].split()) for i in range(n)]


def tile_px(t, n, x, y, hf=0, vf=0):
    tw = t.w // 8
    return t.px((n % tw) * 8 + (7 - x if hf else x), (n // tw) * 8 + (7 - y if vf else y))


def nscr(path):
    b = open(path, 'rb').read()
    w, h = struct.unpack('<HH', b[0x18:0x1C])
    return w // 8, h // 8, list(struct.unpack('<%dH' % ((w // 8) * (h // 8)), b[0x24:0x24 + (w // 8) * (h // 8) * 2]))


def render_map(t, pal, entries, mw, x0, y0, tw, th, opaque):
    out = []
    for y in range(th * 8):
        for x in range(tw * 8):
            v = entries[((y0 * 8 + y) // 8) * mw + (x0 * 8 + x) // 8]
            n, hf, vf, bank = v & 0x3FF, v >> 10 & 1, v >> 11 & 1, v >> 12
            c = tile_px(t, n, x % 8, y % 8, hf, vf)
            if c == 0:
                out.append(bgr555(pal[0]) if opaque else 0)
            else:
                out.append(bgr555(pal[(bank * 16 + c) % len(pal)]))
    return out


def rotate(rows, turns):
    # (turns of 90 degrees clockwise)
    for _ in range(turns % 4):
        rows = [list(r) for r in zip(*rows[::-1])]
    return rows


def occupancy(rows, w, h):
    return [1 if any(rows[y * 16 + j][x * 16 + i] for j in range(16) for i in range(16)) else 0
            for y in range(h) for x in range(w)]


def section(root):
    base = os.path.join(root, MINING)
    if not os.path.isdir(base):
        return b''
    import contest_data
    pics = []
    # the screen and its crack
    t = png8.Image(os.path.join(base, 'interface/interface_tiles.png'))
    pal = t.palette
    mw, mh, e = nscr(os.path.join(base, 'interface/interface.NSCR'))
    pics.append(pic(256, 192, render_map(t, pal, e, mw, 0, 0, 32, 24, True)))
    crack = list(e)
    for r, start, tile in ((0, 25, 11), (1, 57, 65), (2, 89, 119), (3, 121, 173)):
        for i in range(26):
            crack[start - i] = (crack[start - i] & 0xFC00) + tile - (i % 3)
    pics.append(pic(208, 32, render_map(t, pal, crack, mw, 0, 0, 26, 4, True)))
    # the buttons (BG1 over the screen: palette 0, 54 tiles a row)
    for first in (30, 18, 48, 36):
        m = [0] * (6 * 8)
        for row in range(8):
            for col in range(6):
                m[row * 6 + col] = first + row * 54 + col
        pics.append(pic(48, 64, render_map(t, pal, m, 6, 0, 0, 6, 8, False)))
    # the dirt
    d = png8.Image(os.path.join(base, 'objects/dirt_tiles.png'))
    for tiles in ((14, 15, 30, 31), (10, 11, 26, 27), (8, 9, 24, 25), (6, 7, 22, 23), (4, 5, 20, 21), (2, 3, 18, 19),
                  (0, 1, 16, 17)):
        out = []
        for y in range(16):
            for x in range(16):
                c = tile_px(d, tiles[(y // 8) * 2 + x // 8], x % 8, y % 8)
                out.append(bgr555(d.palette[c]) if c % 16 else 0)
        pics.append(pic(16, 16, out))
    # the hit sprites
    a = png8.Image(os.path.join(base, 'animations/animations.png'))
    for k in range(5):
        out = []
        for y in range(32):
            for x in range(32):
                c = a.px(x, k * 32 + y)
                out.append(bgr555(a.palette[c]) if c % 16 else 0)
        pics.append(pic(32, 32, out))
    # the objects
    objs = contest_data.mining_objects(root)
    for o in objs:
        name = o['sprite'][:-len('_NCGR')]
        turns = 0
        for suffix, k in (('_90deg', 1), ('_180deg', 2), ('_270deg', 3)):
            if name.endswith(suffix) and not os.path.exists(os.path.join(base, 'objects', name + '.png')):
                name, turns = name[:-len(suffix)], k
        im = png8.Image(os.path.join(base, 'objects', name + '.png'))
        rows = [[im.px(x, y) for x in range(im.w)] for y in range(im.h)]
        want = [o['grid'][y * 6 + x] for y in range(o['h']) for x in range(o['w'])]
        if turns:
            # (ncgr_gen's turn direction: the one whose cells match the shape; a square-shaped one either way)
            best = None
            for k in (turns, -turns):
                r = rotate(rows, k)
                if len(r[0]) != o['w'] * 16 or len(r) != o['h'] * 16:
                    continue
                score = sum(1 for p, q in zip(occupancy(r, o['w'], o['h']), want) if p == q)
                if best is None or score > best[0]:
                    best = (score, r)
            rows = best[1]
        pname = o['palette'][:-len('_NCLR')]
        pp = os.path.join(base, 'objects', pname + '.pal')
        opal = jasc(pp) if os.path.exists(pp) else png8.Image(os.path.join(base, 'objects', pname + '.png')).palette
        w, h = o['w'] * 16, o['h'] * 16
        out = []
        for y in range(h):
            for x in range(w):
                c = rows[y][x] % 16 if y < len(rows) and x < len(rows[0]) else 0
                out.append(bgr555(opal[c]) if c else 0)
        pics.append(pic(w, h, out))
    return b'MINE' + struct.pack('<H', len(pics)) + b''.join(pics)


if __name__ == '__main__':
    s = section(sys.argv[1])
    print('mining art: %d bytes' % len(s))
    if len(sys.argv) > 2:
        # (a look: every picture side by side in a PNG, tools only)
        from PIL import Image
        p, n = 6, struct.unpack('<H', s[4:6])[0]
        ims = []
        for _ in range(n):
            w, h = struct.unpack('<HH', s[p:p + 4])
            px = struct.unpack('<%dH' % (w * h), s[p + 4:p + 4 + w * h * 2])
            p += 4 + w * h * 2
            im = Image.new('RGB', (w, h), (255, 0, 255))
            for i, c in enumerate(px):
                if c & 0x8000:
                    im.putpixel((i % w, i // w), ((c & 31) * 8, (c >> 5 & 31) * 8, (c >> 10 & 31) * 8))
            ims.append(im)
        W = 1200
        x = y = rowh = 0
        sheet = Image.new('RGB', (W, 1400), (40, 40, 40))
        for im in ims:
            if x + im.width > W:
                x, y, rowh = 0, y + rowh + 4, 0
            sheet.paste(im, (x, y))
            x += im.width + 4
            rowh = max(rowh, im.height)
        sheet.crop((0, 0, W, y + rowh)).save(sys.argv[2])
