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
    return bytes(out) + stages(root)


# Platinum's stages (res/prebuilt/contest/graphic/contest_bg.narc, as overlay017 loads them): (screen, tiles, palette)
# members of the main screen's picture, in the order src/sinnoh/s3d_contest_ui.c names them (STAGE_*)
STAGE_NARC = 'res/prebuilt/contest/graphic/contest_bg.narc'
STAGES = [
    (2, 1, 30),    # the VISUAL round's stage, the audience above (ov17_0223B140.c: BG3)
    (14, 13, 32),  # the DANCE round's floor (ov17_0223DAD0.c ov17_0223E67C: Unk_ov17_02252FC4[0])
    (15, 13, 32),  # ... its other floor (Unk_ov17_02252FC4[1])
    (22, 23, 35),  # the ACTING round's stage (ov17_0223CB1C.c: BG3)
    (25, 27, 39),  # the results' bars (ov17_02250744.c: BG3)
    (20, 19, 34),  # the audience (ov17_0223CB1C.c: the lower screen)
]


def lz10(d):
    """Nintendo's LZ77 (type 0x10), the NARC's members' compression"""
    if not d or d[0] != 0x10:
        return d
    size = d[1] | d[2] << 8 | d[3] << 16
    out, i = bytearray(), 4
    while len(out) < size:
        flags = d[i]
        i += 1
        for b in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> b):
                v = d[i] << 8 | d[i + 1]
                i += 2
                for _ in range((v >> 12) + 3):
                    out.append(out[-((v & 0xFFF) + 1)])
            else:
                out.append(d[i])
                i += 1
    return bytes(out)


def stage_picture(m, scr, chr_, pal):
    """A stage's 256-wide picture (the screen's left 32 x 32 block for the wider ones), RGB555 | 0x8000"""
    import build_journal as bj
    s = bj.section_of(m[scr], b'SCRN')
    w, h = struct.unpack_from('<HH', s, 8)
    size = struct.unpack_from('<I', s, 16)[0]
    ents = struct.unpack_from('<%dH' % (size // 2), s, 20)
    ts = bj.tiles(m[chr_])
    p = bj.palettes(m[pal])
    out = bytearray()
    for y in range(min(h, 256)):
        for x in range(256):
            e = ents[(y // 8) * 32 + x // 8] if w > 256 else ents[(y // 8) * (w // 8) + x // 8]
            n, hf, vf, bank = e & 0x3FF, e >> 10 & 1, e >> 11 & 1, e >> 12
            c = 0
            if n < len(ts):
                c = ts[n][(7 - y % 8 if vf else y % 8) * 8 + (7 - x % 8 if hf else x % 8)]
            out += struct.pack('<H', 0x8000 | p[(bank * 16 + c) % 256 if c else 0])
    return 256, min(h, 256), out


def stages(root):
    """Section "CSTG": u16 count, pictures (u16 w, h, RGB555)"""
    path = os.path.join(root, STAGE_NARC)
    if not os.path.exists(path):
        return b''
    from tv_data import narc_members
    m = [lz10(x) for x in narc_members(path)]
    out = bytearray(b'CSTG') + struct.pack('<H', len(STAGES))
    for scr, chr_, pal in STAGES:
        w, h, px = stage_picture(m, scr, chr_, pal)
        out += struct.pack('<HH', w, h) + px
    return bytes(out)


if __name__ == '__main__':
    s = section(sys.argv[1])
    print('contest art: %d bytes' % len(s))
