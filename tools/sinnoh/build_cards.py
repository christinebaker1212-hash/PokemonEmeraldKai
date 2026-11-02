#!/usr/bin/env python3
"""The TRAINER CARD's JOHTO and SINNOH cards (src/mp/mp_regioncard.c): HeartGold's card (pret/pokeheartgold's
files/a/0/4/9, fetched by fetch_hgss_card.sh) and Platinum's (pret/pokeplatinum's res/graphics/trainer_case, fetched
with the rest of SINNOH by fetch_decomp.sh), front and back, each in its six colors by stars. Nothing of it is
committed: tools/sinnoh/make_pack.sh builds sinnoh/cards.pak and the release ships it.

  python3 -I tools/sinnoh/build_cards.py POKEPLATINUM_DIR HGSS_CARD_DIR OUT.pak

cards.pak: "MCRD", u8 version 1, u8 kinds 2 (0: HeartGold's, 1: Platinum's), then per kind: u16 w, h, u8 palettes;
the front's w*h color indices, the back's, then each palette's 256 colors as u32 0xAARRGGBB (index 0 clear)."""
import os, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from png8 import Image  # noqa: E402   (no Pillow on the release's runner)


# ---- the DS's own formats (NARC archives, NCLR palettes, NCGR tiles, NSCR screens)
def narc(data):
    assert data[:4] == b'NARC'
    off = struct.unpack_from('<H', data, 12)[0]
    sz, n = struct.unpack_from('<IH', data, off + 4)
    ents = [struct.unpack_from('<II', data, off + 12 + 8 * i) for i in range(n)]
    off += sz
    off += struct.unpack_from('<I', data, off + 4)[0]
    base = off + 8
    return [data[base + a: base + b] for a, b in ents]


def sections(f):
    hs, n = struct.unpack_from('<HH', f, 12)
    off, out = hs, {}
    for _ in range(n):
        if off + 8 > len(f):
            break
        out[f[off:off + 4]] = f[off:off + struct.unpack_from('<I', f, off + 4)[0]]
        off += struct.unpack_from('<I', f, off + 4)[0]
    return out


def nclr(f):
    s = sections(f)[b'TTLP']
    _, _, sz, poff = struct.unpack_from('<IIII', s, 8)
    raw = s[8 + poff: 8 + poff + sz]
    return [struct.unpack_from('<H', raw, i)[0] for i in range(0, len(raw) - 1, 2)]


def ncgr8(f):
    s = sections(f)[b'RAHC']
    _, _, _, _, _, _, dsz, doff = struct.unpack_from('<HHIHHIII', s, 8)
    return s[8 + doff: 8 + doff + dsz]


def nscr(f):
    s = sections(f)[b'NRCS']
    w, h, _, sz = struct.unpack_from('<HHII', s, 8)
    return w, h, s[20:20 + sz]


def argb(c555, clear):
    if clear:
        return 0
    r, g, b = c555 & 31, (c555 >> 5) & 31, (c555 >> 10) & 31
    return 0xFF000000 | (r * 255 // 31) << 16 | (g * 255 // 31) << 8 | (b * 255 // 31)


def crop(maps):
    """the part of the screens the card covers (index 0 is the backdrop around it)"""
    x0 = y0 = 9999
    x1 = y1 = 0
    for m, w, h in maps:
        for y in range(h):
            for x in range(w):
                if m[y * w + x]:
                    x0, y0, x1, y1 = min(x0, x), min(y0, y), max(x1, x), max(y1, y)
    return x0, y0, x1 + 1, y1 + 1


def cut(m, w, box):
    x0, y0, x1, y1 = box
    return bytes(m[y * w + x] for y in range(y0, y1) for x in range(x0, x1))


# ---- HeartGold's card: 8-bit tiles (member 41), screens 47 (front) and 48 (back), palettes 0-5 (0-5 stars)
def hgss(path):
    fs = narc(open(path, 'rb').read())
    tiles = ncgr8(fs[41])
    maps = []
    for k in (47, 48):
        w, h, ents = nscr(fs[k])
        ents = [struct.unpack_from('<H', ents, 2 * i)[0] for i in range(len(ents) // 2)]
        h = 192
        m = bytearray(w * h)
        for i, e in enumerate(ents):
            tx, ty = (i % (w // 8)) * 8, (i // (w // 8)) * 8
            if ty >= h:
                break
            t, hf, vf = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1
            tb = tiles[t * 64:(t + 1) * 64]
            for y in range(8):
                for x in range(8):
                    xx, yy = (7 - x if hf else x), (7 - y if vf else y)
                    m[(ty + y) * w + tx + x] = tb[yy * 8 + xx] if len(tb) == 64 else 0
        maps.append((m, w, h))
    box = crop(maps)
    pals = [[argb(c, i == 0) for i, c in enumerate(nclr(fs[p])[:256])] for p in range(6)]
    return box, [cut(m, w, box) for m, w, h in maps], pals


# ---- Platinum's card: 8-bit tiles on an affine layer (a byte a tile), its six colors' palettes
def platinum(root):
    d = os.path.join(root, 'res', 'graphics', 'trainer_case')
    t = Image(os.path.join(d, 'trainer_card_tiles.png'))
    tw = t.w // 8
    maps = []
    for name in ('trainer_card_front', 'trainer_card_back'):
        w, h, ents = nscr(open(os.path.join(d, name + '.NSCR'), 'rb').read())
        h = 192
        m = bytearray(w * h)
        for i, tn in enumerate(ents):
            tx, ty = (i % (w // 8)) * 8, (i // (w // 8)) * 8
            if ty >= h:
                break
            sx, sy = (tn % tw) * 8, (tn // tw) * 8
            for y in range(8):
                for x in range(8):
                    m[(ty + y) * w + tx + x] = t.px(sx + x, sy + y) if sy + y < t.h else 0
        maps.append((m, w, h))
    box = crop(maps)
    pals = []
    for color in ('normal', 'cobalt', 'bronze', 'silver', 'gold', 'black'):
        lines = open(os.path.join(d, 'trainer_card_%s.pal' % color)).read().split('\n')[3:]
        cols = [tuple(map(int, l.split())) for l in lines if l.strip()][:256]
        cols += [(0, 0, 0)] * (256 - len(cols))
        pals.append([0 if i == 0 else 0xFF000000 | r << 16 | g << 8 | b for i, (r, g, b) in enumerate(cols)])
    return box, [cut(m, w, box) for m, w, h in maps], pals


def main():
    plat, hg, out = sys.argv[1:4]
    pak = bytearray(b'MCRD' + bytes([1, 2]))
    for box, sides, pals in (hgss(os.path.join(hg, 'a049.narc')), platinum(plat)):
        w, h = box[2] - box[0], box[3] - box[1]
        pak += struct.pack('<HHB', w, h, len(pals))
        for s in sides:
            pak += s
        for p in pals:
            pak += struct.pack('<256I', *p)
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    with open(out, 'wb') as f:
        f.write(pak)
    print('%s: the JOHTO and SINNOH cards, %d bytes' % (out, len(pak)))


if __name__ == '__main__':
    main()
