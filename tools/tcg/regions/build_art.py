#!/usr/bin/env python3
"""The regional sets' printed cards (docs/TCG_REGIONS.md): graphics/tcg/regional.bin, built into the game beside
graphics/tcg/printed.bin (src/tcg/tcg_art_data.c; tcg_gfx.c reads a card past today's 442 from it).

usage: tools/tcg/regions/build_art.py [--fetch] [--check SHEET.png]

--fetch downloads each card's scan (tools/tcg/regions/cards.json: PokemonTCG/pokemon-tcg-data's image links, 600x825)
and keeps it as a JPEG at the stored size in ~/.cache/moemon/tcgart/regions (never committed). The stored size is
300x412, about MAGIC's (tools/mtg/build_printed.py): the card screens draw into the 240x160 game picture.

The illustration's rectangle (CARD ART's PRINTED ART mode): LAYOUTS below gives one per era and card kind, and each
card's own window frame is found near it (refine: the strongest straight line near each edge). --check draws them
on a sheet of samples.

Aquapolis and Skyridge (the e-Card design, 'aqua') are cut another way: their window is one printed template (rounded
left corners, the picture running to the right border, the stage tab over its top right), measured once over all
their cards (AQUA below, stored pixels) and laid on each scan where its yellow frame matches best (aqua_fit). The
whole window is kept (no inset): tcg_gfx.c masks its corners and tab as transparency (sAquaCorner), so none of the
picture is lost and none of the frame shows.

regional.bin: "TCGP", u32 count (ids 443 on: entry k is card 443 + k), then per card u32 offset, u32 size (0 = no
picture), u16 x, y, w, h (the illustration, in the stored picture's pixels), then an empty entry where printed.bin
has the card back (the layout android/build_assets.py's webp_pack reads; printed.bin's back is used).
"""
import concurrent.futures, io, json, struct, subprocess, sys
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[3]
CACHE = Path.home() / '.cache/moemon/tcgart/regions'
FIRST_ID = 443
SCAN_W, SCAN_H = 600, 825
STORE_W, STORE_H = 300, 412
INSET = 6
# where the illustration's window starts, on the 600x825 scans: left, top, right, bottom by era and card kind (the
# Wizards of the Coast sets: base promos, GYM, NEO; the e-Card sets: Expedition, Aquapolis, Skyridge; the Nintendo
# promos printed in the EX era). Each card's own edges are then found near these (Refine).
ERA = {'basep': 'wotc', 'gym1': 'wotc', 'gym2': 'wotc', 'neo1': 'wotc', 'neo2': 'wotc', 'neo3': 'wotc', 'neo4': 'wotc',
       'ecard1': 'ecard', 'ecard2': 'aqua', 'ecard3': 'aqua', 'np': 'ex'}
# Aquapolis / Skyridge: the template window in stored pixels (300x412): left, top, right, bottom (inclusive), measured
# from the yellow frame over all their cards of that kind (tcg_gfx.c sAquaCorner holds its corners' shape)
AQUA = {'pokemon': (30, 48, 288, 198), 'trainer': (28, 68, 269, 207), 'energy': (32, 40, 290, 266)}
AQUA_SHIFT = 4   # how far (stored pixels) a scan's frame may sit from the template's
LAYOUTS = {
    'pokemon/wotc': (62, 92, 546, 433),
    'trainer/wotc': (54, 186, 554, 459),
    'energy/wotc': (26, 116, 576, 596),
    'pokemon/ecard': (56, 96, 562, 396),
    'trainer/ecard': (60, 132, 540, 414),
    'energy/ecard': (56, 74, 582, 520),
    'pokemon/ex': (42, 74, 562, 382),
    'trainer/ex': (54, 120, 546, 408),
    'energy/ex': (42, 74, 562, 400),
}
SEARCH = 10   # how far (scan pixels) Refine looks from each edge


def cards():
    return json.loads((ROOT / 'tools/tcg/regions/cards.json').read_text())['cards']


def cached(c):
    return CACHE / (c['src'] + '.jpg')


def fetch_one(c):
    path = cached(c)
    if path.exists():
        return 0
    data = subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error', '--connect-timeout', '15',
                           '--max-time', '120', '--retry', '3', c['image']], check=True, capture_output=True).stdout
    img = Image.open(io.BytesIO(data)).convert('RGB')
    if img.size != (SCAN_W, SCAN_H):
        img = img.resize((SCAN_W, SCAN_H), Image.Resampling.LANCZOS)
    out = io.BytesIO()
    img.resize((STORE_W, STORE_H), Image.Resampling.LANCZOS).save(out, format='JPEG', quality=80, subsampling=2,
                                                                  optimize=True)
    path.write_bytes(out.getvalue())
    return 1


def fetch():
    CACHE.mkdir(parents=True, exist_ok=True)
    todo = [c for c in cards() if not cached(c).exists()]
    done = 0
    with concurrent.futures.ThreadPoolExecutor(8) as pool:
        for f in concurrent.futures.as_completed([pool.submit(fetch_one, c) for c in todo]):
            try:
                done += f.result()
            except Exception as e:   # (a failed one is fetched again next time)
                print('failed:', e, file=sys.stderr)
    print(f'fetched {done} of {len(todo)} missing scans into {CACHE}')


def layout_key(c):
    kind = 'trainer' if c['kind'] == 'trainer' else 'energy' if c['kind'] == 'energy' else 'pokemon'
    return f"{kind}/{ERA.get(c['set'], 'wotc')}"


def border_yellow(a):
    """the printed cards' yellow border (tcg_gfx.c BorderYellow)"""
    import numpy as np
    r, g, b = (a[..., i].astype(np.int32) for i in range(3))
    return (r > 140) & (g > 100) & (b * 100 < g * 62) & (r - b > 70)


_aqua_frames = {}


def aqua_frame(kind):
    """the template's yellow frame for that kind: how often each pixel is border yellow over all Aquapolis /
    Skyridge cards of that kind"""
    import numpy as np
    if kind not in _aqua_frames:
        acc, n = np.zeros((STORE_H, STORE_W)), 0
        for c in cards():
            if ERA.get(c['set']) == 'aqua' and layout_key(c).split('/')[0] == kind and cached(c).exists():
                acc += border_yellow(np.asarray(Image.open(cached(c)).convert('RGB')))
                n += 1
        _aqua_frames[kind] = acc / max(n, 1) > 0.5
    return _aqua_frames[kind]


def aqua_fit(c):
    """an Aquapolis / Skyridge card's window: the template's, moved to where the card's own yellow frame matches the
    template's best (within AQUA_SHIFT), in scan pixels"""
    import numpy as np
    kind = layout_key(c).split('/')[0]
    l, t, r, b = AQUA[kind]
    frame = aqua_frame(kind)
    mine = border_yellow(np.asarray(Image.open(cached(c)).convert('RGB')))
    # (the band round the window: its frame, where the template is sure)
    y0, y1, x0, x1 = t - 12, b + 12, l - 12, r + 2
    best, shift = -1, (0, 0)
    for dy in range(-AQUA_SHIFT, AQUA_SHIFT + 1):
        for dx in range(-AQUA_SHIFT, AQUA_SHIFT + 1):
            a = frame[y0:y1, x0:x1]
            m = mine[y0 + dy:y1 + dy, x0 + dx:x1 + dx]
            score = int((a == m).sum()) - abs(dx) - abs(dy)
            if score > best:
                best, shift = score, (dx, dy)
    dx, dy = shift
    sx, sy = SCAN_W / STORE_W, SCAN_H / STORE_H
    return (l + dx) * sx, (t + dy) * sy, (r + 1 + dx) * sx, (b + 1 + dy) * sy


def window(c):
    """the illustration's rectangle in scan pixels, as built in: the WotC / EX layouts refined and inset, the
    Aquapolis / Skyridge template whole"""
    if ERA.get(c['set']) == 'aqua':
        return aqua_fit(c)
    l, t, r, b = refine(c)
    return l + INSET, t + INSET, r - INSET, b - INSET


def refine(c):
    """the card's own window: each edge of its layout's rectangle moved to the strongest straight line within
    SEARCH pixels (the window's frame), measured on the stored scan"""
    import numpy as np
    l, t, r, b = LAYOUTS[layout_key(c)]
    a = np.asarray(Image.open(cached(c)).convert('L'), dtype=np.float32)
    h, w = a.shape
    sx, sy = w / SCAN_W, h / SCAN_H
    gy = np.abs(np.diff(a, axis=0))
    gx = np.abs(np.diff(a, axis=1))
    L, T, R, B = round(l * sx), round(t * sy), round(r * sx), round(b * sy)
    def best_row(y0, x0, x1):
        span = gy[:, x0 + (x1 - x0) // 5: x1 - (x1 - x0) // 5].mean(axis=1)
        lo, hi = max(1, y0 - round(SEARCH * sy)), min(h - 2, y0 + round(SEARCH * sy))
        return lo + int(np.argmax(span[lo:hi + 1]))
    def best_col(x0, y0, y1):
        span = gx[y0 + (y1 - y0) // 5: y1 - (y1 - y0) // 5, :].mean(axis=0)
        lo, hi = max(1, x0 - round(SEARCH * sx)), min(w - 2, x0 + round(SEARCH * sx))
        return lo + int(np.argmax(span[lo:hi + 1]))
    T2, B2 = best_row(T, L, R), best_row(B, L, R)
    L2, R2 = best_col(L, T2, B2), best_col(R, T2, B2)
    if B2 - T2 < (B - T) * 3 // 4 or R2 - L2 < (R - L) * 3 // 4:   # (no clear frame: the layout's own)
        L2, T2, R2, B2 = L, T, R, B
    return L2 / sx, T2 / sy, R2 / sx, B2 / sy


def check(out):
    """a sheet of every layout's cards with their windows drawn (--check FILE)"""
    from PIL import ImageDraw
    groups = {}
    for c in cards():
        if cached(c).exists():
            groups.setdefault(layout_key(c), []).append(c)
    picks = [c for k in sorted(groups) for c in groups[k][::max(1, len(groups[k]) // 5)][:5]]
    W, H = STORE_W, STORE_H
    sheet = Image.new('RGB', (W * 10, H * ((len(picks) + 9) // 10)), 'white')
    for i, c in enumerate(picks):
        im = Image.open(cached(c)).convert('RGB')
        l, t, r, b = window(c)
        f = STORE_W / SCAN_W
        ImageDraw.Draw(im).rectangle((l * f, t * f, r * f - 1, b * f - 1), outline=(255, 0, 255), width=1)
        sheet.paste(im, ((i % 10) * W, (i // 10) * H))
    sheet.save(out)


def build():
    rows = cards()
    count = max(c['id'] for c in rows) - FIRST_ID + 1
    by_id = {c['id']: c for c in rows}
    table, blobs = [], []
    offset = 8 + 16 * (count + 1)
    sx, sy = STORE_W / SCAN_W, STORE_H / SCAN_H
    missing = []
    for k in range(count):
        c = by_id.get(FIRST_ID + k)
        if not c or not cached(c).exists():
            table.append(struct.pack('<IIHHHH', 0, 0, 0, 0, 0, 0))
            if c:
                missing.append(c['src'])
            continue
        data = cached(c).read_bytes()
        l, t, r, b = window(c)
        x, y = round(l * sx), round(t * sy)
        w, h = round(r * sx) - x, round(b * sy) - y
        table.append(struct.pack('<IIHHHH', offset, len(data), x, y, w, h))
        blobs.append(data)
        offset += len(data)
    table.append(struct.pack('<IIHHHH', 0, 0, 0, 0, 0, 0))   # (no back of its own)
    out = b'TCGP' + struct.pack('<I', count) + b''.join(table) + b''.join(blobs)
    (ROOT / 'graphics/tcg/regional.bin').write_bytes(out)
    print(f'graphics/tcg/regional.bin: {len(blobs)} printed cards, {len(out)} bytes'
          + (f'; no scan for {len(missing)}: {" ".join(missing[:10])}' if missing else ''))


if __name__ == '__main__':
    if '--fetch' in sys.argv:
        fetch()
    if '--check' in sys.argv:
        check(sys.argv[sys.argv.index('--check') + 1])
    else:
        build()
