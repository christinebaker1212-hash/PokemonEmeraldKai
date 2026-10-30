#!/usr/bin/env python3
"""Default (non-PMD) follower sprites for the Gen 4-9 Pokemon, from Moemon Quetzal's overworld sprites.

    python3 tools/moemon_gen4/gen_followers.py QUETZAL_ROM [--check DIR]

Kai's followers for national dex 1-386 are HGSS-style overworld sprites cut by tools/moemon_followers/slice.py.
Quetzal has the same kind for every Pokemon (its "FFP2" blobs, tools/quetzal_rip/ffp2.py): this takes 387-1025
(Gen 5-9 names from tools/moemon_gen59/species.json; one Quetzal lacks comes from graphics/moemon/followers/extra)
(matched by name in tools/quetzal_rip/pokemon_ow_gfx.json, the normal colors) and appends them to
graphics/moemon/followers/followers.bin in slice.py's format (8 frames down1 down2 up1 up2 left1 left2 right1 right2,
4bpp tiles, then 16 colors; right = the left frames mirrored), with "sheet": "quetzal" in index.json, and rewrites
src/data/mp_followers.h. Running it again replaces its own entries. The ROM is only read as data.
"""
import json
import os
import sys
import unicodedata

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(REPO, 'tools', 'quetzal_rip'))
from qrom import Rom  # noqa: E402
from sprites import graphics_info, frames, palette_tags, POKEMON_OW  # noqa: E402

OUT = os.path.join(REPO, 'graphics', 'moemon', 'followers')
FIRST, LAST = 387, 1025


def norm(s):
    s = unicodedata.normalize('NFKD', s.replace('?', 'e'))   # Quetzal's table writes é as ?
    return ''.join(c for c in s.lower() if c.isascii() and c.isalnum())


def to_pixels(raw, size):
    t = size // 8
    px = [[0] * size for _ in range(size)]
    for ty in range(t):
        for tx in range(t):
            base = (ty * t + tx) * 32
            for y in range(8):
                for x in range(8):
                    b = raw[base + y * 4 + x // 2]
                    px[ty * 8 + y][tx * 8 + x] = (b >> 4) if x & 1 else (b & 15)
    return px


def to_tiles(px, size):
    t = size // 8
    out = bytearray()
    for ty in range(t):
        for tx in range(t):
            for y in range(8):
                for x in range(0, 8, 2):
                    out.append(px[ty * 8 + y][tx * 8 + x] | (px[ty * 8 + y][tx * 8 + x + 1] << 4))
    return bytes(out)


def mirror(raw, size):
    return to_tiles([row[::-1] for row in to_pixels(raw, size)], size)


def main():
    rom_path = next((a for a in sys.argv[1:] if a.endswith('.gba')), os.environ.get('QUETZAL_ROM'))
    if not rom_path:
        raise SystemExit(__doc__)
    rom = Rom(rom_path)
    names = json.load(open(os.path.join(HERE, 'gen4_data.json'), encoding='utf-8'))
    dex_names = {s['national']: s['name'] for s in names['species'] if FIRST <= s.get('national', 0) <= LAST}
    for s in json.load(open(os.path.join(REPO, 'tools', 'moemon_gen59', 'species.json'), encoding='utf-8'))['species']:
        dex_names[s['national']] = s['name']
    by_name = {}
    for gid, (sp, nm, shiny) in POKEMON_OW.items():
        if not shiny:
            by_name.setdefault(norm(nm), int(gid))
    pals = palette_tags(rom)

    index = [e for e in json.load(open(os.path.join(OUT, 'index.json'))) if e.get('sheet') != 'quetzal']
    blob = bytearray(open(os.path.join(OUT, 'followers.bin'), 'rb').read())
    end = max(e['offset'] + 8 * e['size'] * e['size'] // 2 + 32 for e in index)
    del blob[end:]
    added, missing = 0, []
    for dex in range(FIRST, LAST + 1):
        gid = by_name.get(norm(dex_names.get(dex, '')))
        info = graphics_info(rom, gid) if gid else None
        fr = frames(rom, info) if info else []
        if len(fr) < 6 or info['width'] != info['height'] or info['width'] not in (32, 64):
            missing.append(dex)
            continue
        size = info['width']
        down1, down2, left1, left2, up1, up2 = [bytes(f) for f in fr[:6]]
        off = len(blob)
        for f in (down1, down2, up1, up2, left1, left2, mirror(left1, size), mirror(left2, size)):
            blob += f
        blob += bytes(rom.d[pals[info['palette_tag']]:pals[info['palette_tag']] + 32])
        index.append({'dex': dex, 'variant': 0, 'size': size, 'offset': off, 'sheet': 'quetzal', 'cell': gid})
        added += 1
    # the ones Quetzal lacks: an overworld sheet of 6 frames (down1 down2 up1 up2 left1 left2, indexed colors) in
    # graphics/moemon/followers/extra/<dex>_<name>.png, from pokeemerald-expansion (OGERPON)
    from PIL import Image
    extra_dir = os.path.join(REPO, 'graphics', 'moemon', 'followers', 'extra')
    for fn in sorted(os.listdir(extra_dir)) if os.path.isdir(extra_dir) else []:
        dex = int(fn.split('_')[0])
        if dex not in missing:
            continue
        im = Image.open(os.path.join(extra_dir, fn))
        size = im.height
        pix = im.load()
        fr = [to_tiles([[pix[k * size + x, y] & 15 for x in range(size)] for y in range(size)], size) for k in range(6)]
        pal = im.getpalette()[:48] + [0] * max(0, 48 - len(im.getpalette()[:48]))
        off = len(blob)
        for f in (fr[0], fr[1], fr[2], fr[3], fr[4], fr[5], mirror(fr[4], size), mirror(fr[5], size)):
            blob += f
        for c in range(16):
            r, g, b = pal[3 * c] >> 3, pal[3 * c + 1] >> 3, pal[3 * c + 2] >> 3
            v = r | g << 5 | b << 10
            blob += bytes([v & 0xFF, v >> 8])
        index.append({'dex': dex, 'variant': 0, 'size': size, 'offset': off, 'sheet': 'quetzal', 'cell': 0})
        missing.remove(dex)
        added += 1
    open(os.path.join(OUT, 'followers.bin'), 'wb').write(blob)
    json.dump(index, open(os.path.join(OUT, 'index.json'), 'w'), indent=0)
    with open(os.path.join(REPO, 'src', 'data', 'mp_followers.h'), 'w') as f:
        f.write('// Generated by tools/moemon_followers/slice.py from the overworld sheets (387-1025: '
                'tools/moemon_gen4/gen_followers.py). Do not edit.\n')
        f.write('// dex = national dex number, variant 0 = normal, 1 = female, 2 = alternate form\n')
        f.write('// size = frame width/height (32 or 64), offset = into graphics/moemon/followers/followers.bin\n')
        f.write('static const struct MpFollowerGfx sMpFollowerGfx[] = {\n')
        for e in index:
            f.write('    {%d, %d, %d, %d},\n' % (e['dex'], e['variant'], e['size'], e['offset']))
        f.write('};\n')
    if '--check' in sys.argv:
        from PIL import Image
        d = sys.argv[sys.argv.index('--check') + 1]
        es = [e for e in index if e['sheet'] == 'quetzal']
        sheet = Image.new('RGB', (64 * 8, 64 * ((len(es) + 7) // 8)), (40, 120, 40))
        for i, e in enumerate(es):
            sz, o = e['size'], e['offset']
            pal = blob[o + 8 * sz * sz // 2:o + 8 * sz * sz // 2 + 32]
            cols = [((v & 31) << 3, ((v >> 5) & 31) << 3, ((v >> 10) & 31) << 3)
                    for v in (pal[k] | pal[k + 1] << 8 for k in range(0, 32, 2))]
            px = to_pixels(blob[o:o + sz * sz // 2], sz)
            im = Image.new('RGB', (sz, sz), (40, 120, 40))
            for y in range(sz):
                for x in range(sz):
                    if px[y][x]:
                        im.putpixel((x, y), cols[px[y][x]])
            sheet.paste(im.resize((64, 64)) if sz == 32 else im, ((i % 8) * 64, (i // 8) * 64))
        sheet.save(os.path.join(d, 'gen4_followers.png'))
    print('%d Gen 4-9 followers added, missing: %s; followers.bin %d bytes' % (added, missing, len(blob)))


if __name__ == '__main__':
    main()
