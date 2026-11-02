#!/usr/bin/env python3
"""The BADGE CASE's badge pictures (graphics/moemon/badges/badges.bin, built in by src/mp/mp_badge_data.c): the
official artwork of the 32 badges of HOENN, JOHTO, KANTO and SINNOH, from PokeAPI's sprites repository at a pinned
commit (sprites/badges/1-8 KANTO, 9-16 JOHTO, 17-24 HOENN, 25-32 SINNOH; each region's in its gyms' order, which is
also the order of its badge flags: mp_region.c MP_Region_BadgeFlag).

The artwork's white stroke around each badge's dark outline is taken off (a flood from outside the badge through
clear and near-white pixels, stopped by the outline, so white inside a badge stays). Each badge is then cut to what it
covers, centered in a square with a small margin and resized with Lanczos to the two
sizes the case draws it at (its 480x320 picture: CASE in a tray, BIG on the page about one badge).

  python3 tools/moemon_badges/build_badges.py [SPRITES_DIR]
SPRITES_DIR: a checkout of github.com/PokeAPI/sprites (fetched at the pinned commit into ~/.cache when not given).

badges.bin: "MBDG", u16 version 1, u16 count 32, u16 CASE, u16 BIG, then per badge (region-major: HOENN, JOHTO,
KANTO, SINNOH) CASE*CASE then BIG*BIG pixels, each u32 0xAARRGGBB (SDL_PIXELFORMAT_ARGB8888, little-endian),
straight alpha."""
import os, struct, subprocess, sys
from collections import deque
from PIL import Image

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
URL = 'https://github.com/PokeAPI/sprites'
COMMIT = '1dd55c0436d2a5b6e324c0b780e1b1b416fbfb64'
CASE, BIG = 64, 128
# the game's regions in mp_region.h order -> PokeAPI's first badge number
REGION_FIRST = {'HOENN': 17, 'JOHTO': 9, 'KANTO': 1, 'SINNOH': 25}
REGIONS = ['HOENN', 'JOHTO', 'KANTO', 'SINNOH']


def fetch(dest):
    if os.path.exists(os.path.join(dest, 'sprites', 'badges', '32.png')):
        return
    os.makedirs(dest, exist_ok=True)
    run = lambda *a: subprocess.check_call(['git', '-C', dest] + list(a), stdout=subprocess.DEVNULL)
    if not os.path.isdir(os.path.join(dest, '.git')):
        run('init', '-q')
        run('remote', 'add', 'origin', URL)
    run('config', 'core.sparseCheckout', 'true')
    with open(os.path.join(dest, '.git', 'info', 'sparse-checkout'), 'w') as f:
        f.write('sprites/badges/\n')
    run('fetch', '-q', '--depth', '1', '--filter=blob:none', 'origin', COMMIT)
    run('checkout', '-q', COMMIT)


def without_halo(im, lum=150, spread=60):
    """the badge without the white stroke outside its outline"""
    im = im.convert('RGBA')
    w, h = im.size
    px = im.load()

    def passable(x, y):
        r, g, b, a = px[x, y]
        return a < 60 or (min(r, g, b) > lum and max(r, g, b) - min(r, g, b) < spread)
    outside = bytearray(w * h)
    q = deque([(x, y) for x in range(w) for y in (0, h - 1)] + [(x, y) for y in range(h) for x in (0, w - 1)])
    while q:
        x, y = q.popleft()
        if x < 0 or y < 0 or x >= w or y >= h or outside[y * w + x] or not passable(x, y):
            continue
        outside[y * w + x] = 1
        q.extend(((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)))
    for y in range(h):
        for x in range(w):
            if outside[y * w + x]:
                px[x, y] = (0, 0, 0, 0)
    return im


def pixels(im):
    return list(im.get_flattened_data()) if hasattr(im, 'get_flattened_data') else list(im.getdata())


def fitted(im, size):
    """the badge cut to its pixels, in a square with a 4% margin, at size x size"""
    im = im.convert('RGBA')
    box = im.getchannel('A').point(lambda a: 255 if a > 8 else 0).getbbox()
    if box:
        im = im.crop(box)
    side = max(im.size)
    pad = max(1, round(side * 0.04))
    sq = Image.new('RGBA', (side + 2 * pad, side + 2 * pad), (0, 0, 0, 0))
    sq.paste(im, ((sq.width - im.width) // 2, (sq.height - im.height) // 2))
    # resize premultiplied, so the transparent edge doesn't bleed its hidden color into the outline
    r, g, b, a = sq.split()
    pre = Image.merge('RGBA', [Image.composite(c, Image.new('L', sq.size, 0), a) for c in (r, g, b)] + [a])
    pre = pre.resize((size, size), Image.LANCZOS)
    out = []
    for (pr, pg, pb, pa) in pixels(pre):
        if pa == 0:
            out.append(0)
            continue
        cr, cg, cb = (min(255, round(v * 255 / pa)) for v in (pr, pg, pb))
        out.append((pa << 24) | (cr << 16) | (cg << 8) | cb)
    return out


def main():
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.environ.get('XDG_CACHE_HOME', os.path.expanduser('~/.cache')), 'moemon', 'pokeapi_sprites')
    fetch(src)
    out = bytearray(b'MBDG' + struct.pack('<HHHH', 1, 32, CASE, BIG))
    for region in REGIONS:
        for k in range(8):
            im = without_halo(Image.open(os.path.join(src, 'sprites', 'badges', '%d.png' % (REGION_FIRST[region] + k))))
            for size in (CASE, BIG):
                out += struct.pack('<%dI' % (size * size), *fitted(im, size))
    dst = os.path.join(REPO, 'graphics', 'moemon', 'badges', 'badges.bin')
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    with open(dst, 'wb') as f:
        f.write(out)
    print('%s: 32 badges, %d bytes' % (os.path.relpath(dst, REPO), len(out)))


if __name__ == '__main__':
    main()
