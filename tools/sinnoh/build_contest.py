"""SUPER CONTESTS' art for sinnoh/platinum_gfx.pak (build_gfx.py appends it): Platinum's own accessories and backdrops
from pret/pokeplatinum's res/graphics/fashion_case (never committed), as RGB555 pictures (src/sinnoh/s3d_contest.c).

Section "CNTS": u16 accessories (Platinum's ACCESSORY_* order: include/constants/accessories.h), each u16 w, h, then
w*h RGB555 (0x8000 set: a drawn pixel; 0: clear); u16 backdrops (generated/backdrops.txt's order), each u16 w, h, then
w*h RGB555 (frame0's tilemap over its tiles).
"""
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import png8  # noqa: E402

FASHION = 'res/graphics/fashion_case'


def bgr555(rgb):
    r, g, b = rgb[:3]
    return 0x8000 | (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10


def accessory_names(root):
    t = open(os.path.join(root, 'include/constants/accessories.h')).read()
    names, n = {}, 0
    for enum in re.findall(r'enum\s*\w*\s*\{([^}]*)\}', t):
        for item in enum.split(','):
            item = re.sub(r'//.*', '', item).strip()
            if not item:
                continue
            name, _, val = item.partition('=')
            n = int(val.strip(), 0) if val.strip() else n
            if name.strip().startswith('ACCESSORY_') and name.strip() != 'ACCESSORY_COUNT':
                names[n] = name.strip()[len('ACCESSORY_'):].lower()
            n += 1
    return [names[i] for i in sorted(names)]


def picture(im):
    pal = im.getpalette() if hasattr(im, 'getpalette') else im.palette
    if pal and isinstance(pal[0], int):
        pal = [tuple(pal[i:i + 3]) for i in range(0, len(pal), 3)]
    out = bytearray()
    for y in range(im.h):
        for x in range(im.w):
            c = im.px(x, y)
            out += struct.pack('<H', 0 if c == 0 else bgr555(pal[c]))
    return out


def backdrop(d):
    t = png8.Image(os.path.join(d, 'tiles.png'))
    pal = t.getpalette()
    if pal and isinstance(pal[0], int):
        pal = [tuple(pal[i:i + 3]) for i in range(0, len(pal), 3)]
    b = open(os.path.join(d, 'frame0.NSCR'), 'rb').read()
    w, h = struct.unpack('<HH', b[0x18:0x1C])
    mw = w // 8
    e = struct.unpack('<%dH' % ((w // 8) * (h // 8)), b[0x24:0x24 + (w // 8) * (h // 8) * 2])
    tw, count = t.w // 8, (t.w // 8) * (t.h // 8)
    out = bytearray()
    for y in range(h):
        for x in range(w):
            v = e[(y // 8) * mw + x // 8]
            n, hf, vf, bank = v & 0x3FF, v >> 10 & 1, v >> 11 & 1, v >> 12
            c = 0
            if n < count:
                px, py = x % 8, y % 8
                c = t.px((n % tw) * 8 + (7 - px if hf else px), (n // tw) * 8 + (7 - py if vf else py))
            idx = bank * 16 + c
            out += struct.pack('<H', bgr555(pal[idx] if idx < len(pal) else pal[c]))
    return w, h, out


def section(root):
    base = os.path.join(root, FASHION)
    if not os.path.isdir(base):
        return b''
    out = bytearray(b'CNTS')
    names = accessory_names(root)
    out += struct.pack('<H', len(names))
    for name in names:
        p = os.path.join(base, 'accessories', 'accessory_%s.png' % name)
        if os.path.exists(p):
            im = png8.Image(p)
            out += struct.pack('<HH', im.w, im.h) + picture(im)
        else:
            out += struct.pack('<HH', 0, 0)
    bd = [x.strip()[len('BACKDROP_'):].lower() for x in open(os.path.join(root, 'generated/backdrops.txt')) if x.strip()]
    bd = [x for x in bd if x != 'count']
    out += struct.pack('<H', len(bd))
    for name in bd:
        d = os.path.join(base, 'backdrops', name)
        if os.path.isdir(d):
            w, h, px = backdrop(d)
            out += struct.pack('<HH', w, h) + px
        else:
            out += struct.pack('<HH', 0, 0)
    return bytes(out)


if __name__ == '__main__':
    s = section(sys.argv[1])
    print('contest art: %d bytes' % len(s))
