#!/usr/bin/env python3
"""The particle textures of Platinum's and Black's move animations (and Platinum's ball seals), read from the user's
ROMs: every SPA file's SPT textures, decoded to RGBA. Nothing read from a ROM is ever committed: the ball effects'
DS pack is built from this by build_ds_pack.py at release time (like UI STYLE's PLATINUM / BLACK packs).

    python3 -I tools/moemon_ballfx/ds_particles.py platinum ROM OUTDIR    # every texture, deduplicated, as PNGs + sheets
    python3 -I tools/moemon_ballfx/ds_particles.py black ROM OUTDIR

A texture's key: GAME:ARCHIVE:FILE:TEX (e.g. "pl:waza:52:1"), the same in build_ds_pack.py's list.
"""
import hashlib
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'sinnoh'))
import nds  # noqa: E402

ARCHIVES = {
    'platinum': [('waza', 'wazaeffect/effectdata/waza_particle.narc'), ('ball', 'wazaeffect/effectdata/ball_particle.narc'),
                 ('data', 'particledata/particledata.narc'), ('etc', 'particledata/pl_etc/pl_etc_particle.narc')],
    'black': [('waza', 'a/0/0/6'), ('etc', 'a/2/1/6'), ('etc2', 'a/1/1/5')],
}
PREFIX = {'platinum': 'pl', 'black': 'bw'}


def rgb555(v):
    return ((v & 31) * 255 // 31, ((v >> 5) & 31) * 255 // 31, ((v >> 10) & 31) * 255 // 31)


def decode_spt(d, p):
    """one SPT texture block at p: (w, h, rgba list) or None"""
    params = struct.unpack_from('<I', d, p + 4)[0]
    tex_size, pal_off, pal_size, pal_idx_off, _, block = struct.unpack_from('<6I', d, p + 8)
    fmt = params & 15
    w = 8 << ((params >> 4) & 15)
    h = 8 << ((params >> 8) & 15)
    c0t = (params >> 16) & 1
    tex = p + 0x20
    pal = [rgb555(struct.unpack_from('<H', d, p + pal_off + 2 * i)[0]) for i in range(pal_size // 2)]

    def pc(i):
        return pal[i] if i < len(pal) else (255, 0, 255)
    out = [(0, 0, 0, 0)] * (w * h)
    if fmt == 1:      # A3I5
        for i in range(w * h):
            v = d[tex + i]
            a = (v >> 5) * 36
            out[i] = pc(v & 31) + (min(255, a),)
    elif fmt == 6:    # A5I3
        for i in range(w * h):
            v = d[tex + i]
            out[i] = pc(v & 7) + ((v >> 3) * 255 // 31,)
    elif fmt == 2:
        for i in range(w * h):
            k = (d[tex + i // 4] >> (2 * (i % 4))) & 3
            out[i] = (0, 0, 0, 0) if (k == 0 and c0t) else pc(k) + (255,)
    elif fmt == 3:
        for i in range(w * h):
            k = (d[tex + i // 2] >> (4 * (i % 2))) & 15
            out[i] = (0, 0, 0, 0) if (k == 0 and c0t) else pc(k) + (255,)
    elif fmt == 4:
        for i in range(w * h):
            k = d[tex + i]
            out[i] = (0, 0, 0, 0) if (k == 0 and c0t) else pc(k) + (255,)
    elif fmt == 7:
        for i in range(w * h):
            v = struct.unpack_from('<H', d, tex + 2 * i)[0]
            out[i] = rgb555(v) + (255 if v & 0x8000 else 0,)
    elif fmt == 5:    # 4x4 compressed: the index data follows the texels (pal_idx_off)
        idx = p + pal_idx_off if pal_idx_off else tex + tex_size
        bw = w // 4
        for by in range(h // 4):
            for bx in range(bw):
                blk = by * bw + bx
                bits = struct.unpack_from('<I', d, tex + 4 * blk)[0]
                pi = struct.unpack_from('<H', d, idx + 2 * blk)[0]
                mode, base = pi >> 14, (pi & 0x3FFF) * 2
                cols = [pc(base + k) for k in range(4)]
                if mode == 0:
                    cl = [c + (255,) for c in cols[:3]] + [(0, 0, 0, 0)]
                elif mode == 1:
                    mid = tuple((a + b) // 2 for a, b in zip(cols[0], cols[1]))
                    cl = [cols[0] + (255,), cols[1] + (255,), mid + (255,), (0, 0, 0, 0)]
                elif mode == 2:
                    cl = [c + (255,) for c in cols]
                else:
                    m1 = tuple((5 * a + 3 * b) // 8 for a, b in zip(cols[0], cols[1]))
                    m2 = tuple((3 * a + 5 * b) // 8 for a, b in zip(cols[0], cols[1]))
                    cl = [cols[0] + (255,), cols[1] + (255,), m1 + (255,), m2 + (255,)]
                for yy in range(4):
                    for xx in range(4):
                        out[(by * 4 + yy) * w + bx * 4 + xx] = cl[(bits >> (2 * (yy * 4 + xx))) & 3]
    else:
        return None
    return w, h, out, block


def spa_textures(d):
    """[(w, h, rgba)] of an SPA file (' APS12_1' or Black's version)"""
    if d[:4] != b' APS':
        return []
    ntex = struct.unpack_from('<H', d, 10)[0]
    p = struct.unpack_from('<I', d, 24)[0]
    out = []
    for _ in range(ntex):
        if d[p:p + 4] != b' TPS':
            break
        r = decode_spt(d, p)
        if r is None:
            break
        w, h, px, block = r
        out.append((w, h, px))
        p += block
    return out


def textures(game, rom_path):
    """{key: (w, h, rgba)} for every texture of the game's particle archives (the first of identical ones)"""
    rom = nds.Rom(rom_path)
    seen, out = set(), {}
    for arc, path in ARCHIVES[game]:
        narc = rom.narc(path)
        for fi in range(len(narc)):
            for ti, (w, h, px) in enumerate(spa_textures(narc[fi])):
                hsh = hashlib.md5(bytes(v for c in px for v in c)).hexdigest()
                if hsh in seen or all(c[3] == 0 for c in px):
                    continue
                seen.add(hsh)
                out['%s:%s:%d:%d' % (PREFIX[game], arc, fi, ti)] = (w, h, px)
    return out


def main():
    from PIL import Image, ImageDraw
    game, rom, outdir = sys.argv[1:4]
    os.makedirs(outdir, exist_ok=True)
    tex = textures(game, rom)
    print('%s: %d textures' % (game, len(tex)))
    # contact sheets: each texture on a dark and a light square, its key under it
    keys = sorted(tex, key=lambda k: [int(x) if x.isdigit() else x for x in k.split(':')])
    cell, cols, per = 72, 16, 16 * 12
    for s in range(0, len(keys), per):
        sheet = Image.new('RGB', (cols * cell, ((min(per, len(keys) - s) + cols - 1) // cols) * (cell + 10)), (30, 34, 40))
        dr = ImageDraw.Draw(sheet)
        for i, k in enumerate(keys[s:s + per]):
            w, h, px = tex[k]
            im = Image.new('RGBA', (w, h))
            im.putdata(px)
            sc = max(1, min(64 // w, 64 // h))
            im = im.resize((w * sc, h * sc), Image.NEAREST) if sc > 1 else im.resize((min(w, 64), min(h, 64)))
            x, y = (i % cols) * cell, (i // cols) * (cell + 10)
            bg = Image.new('RGBA', im.size, (60, 70, 80, 255))
            bg.alpha_composite(im)
            sheet.paste(bg.convert('RGB'), (x + 4, y + 2))
            dr.text((x + 2, y + cell - 4), k.split(':', 1)[1], fill=(200, 200, 200))
        sheet.save(os.path.join(outdir, '%s_sheet%02d.png' % (game, s // per)))
    import pickle
    pickle.dump(tex, open(os.path.join(outdir, game + '.pickle'), 'wb'))


if __name__ == '__main__':
    main()
