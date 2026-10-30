#!/usr/bin/env python3
"""Build graphics/tcg/printed.bin: the printed cards, built into the game (src/tcg/tcg_art_data.c).

usage: tools/tcg/fetch_art.py && tools/tcg/build_printed_art.py

Each scan (600x825, from fetch_art.py) is stored as a JPEG at 360x495: the card screen
draws into the 240x160 game picture, where even the zoomed big picture shows a card about
280 pixels tall. With it goes the rectangle its illustration fills, for CARD ART's PRINTED
ART mode (the printed picture in the Game Boy frame).

The rectangles were measured on the scans: every card of one layout (Base Set, Jungle,
Fossil and promo Pokemon; Base / Jungle / Fossil Trainers; basic Energy and DOUBLE
COLORLESS) sits within a few pixels of the same place, so each layout has one rectangle,
the inner edge of the window's bevel, found as the strongest straight line in the
average of all its cards. A 3-pixel inset keeps the bevel out when a scan sits off by a
pixel or two. Energy cards have no window: the rectangle frames the symbol.

The card back (CARD BACKS: PRINTED) is stored the same way, after the cards.

printed.bin: "TCGP", u32 count, then per card id 1..count and then the back: u32 offset,
u32 size (0 = no printed card), u16 x, y, w, h (the illustration, in the stored picture's
pixels; the whole picture for the back).
"""
import io, json, struct
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
SCAN_W, SCAN_H = 600, 825
STORE_W, STORE_H = 360, 495
INSET = 3
# left, top, right, bottom on the 600x825 scans (right / bottom exclusive)
LAYOUTS = {
    'pokemon/base1': (64, 98, 537, 427),
    'pokemon/base2': (64, 95, 537, 423),
    'pokemon/base3': (62, 93, 539, 423),
    'pokemon/basep': (65, 96, 536, 423),
    'trainer/base1': (62, 187, 553, 457),
    'trainer/base2': (54, 186, 555, 459),
    'trainer/base3': (54, 190, 557, 461),
    'energy/basic': (80 - INSET, 405 - INSET, 520 + INSET, 735 + INSET),     # the symbol and its glow, 4:3
    'energy/double': (90 - INSET, 250 - INSET, 510 + INSET, 565 + INSET),
    # GB2's printings (measured the same way): Team Rocket, Neo Genesis and Gym, the renders made for cards never
    # printed in English (tools/tcg/card_renders.json, Base Set's layout), and the special Energy cards' pictures
    'pokemon/base5': (65, 94, 549, 435),
    'pokemon/render': (66, 98, 529, 434),
    'trainer/base5': (55, 187, 557, 460),
    'trainer/neo1': (58, 188, 554, 460),
    'trainer/gym1': (56, 187, 556, 460),
    'trainer/gym2': (57, 189, 554, 461),
    'trainer/render': (62, 192, 540, 459),
    'trainer/basep': (55, 193, 548, 461),
    'energy/base5': (72, 163, 560, 480),
    'energy/neo1': (72, 164, 578, 433),
}


def layout(row):
    source = row['printing'].split('-')[0]
    if row['kind'] == 'energy':
        if source in ('base5', 'neo1'):
            return 'energy/' + source   # (the special Energy cards: a picture, not a symbol)
        return 'energy/double' if row['card_id'] == 7 else 'energy/basic'
    if source == 'render':
        return row['kind'] + '/render'
    return row['kind'] + '/' + source


def jpeg(path):
    stream = io.BytesIO()
    Image.open(path).convert('RGB').resize((STORE_W, STORE_H), Image.Resampling.LANCZOS).save(
        stream, format='JPEG', quality=80, subsampling=2, optimize=True)   # about 9 MB for all of them
    return stream.getvalue()


def main():
    manifest = json.loads((ROOT / 'tools/tcg/art_manifest.json').read_text())
    count = max(r['card_id'] for r in manifest['cards'])
    rows = {r['card_id']: r for r in manifest['cards'] if r['status'] == 'matched'}
    table, blobs, report = [], [], {}
    offset = 8 + 16 * (count + 1)
    sx, sy = STORE_W / SCAN_W, STORE_H / SCAN_H
    for card in range(1, count + 1):
        row = rows.get(card)
        if not row:
            table.append(struct.pack('<IIHHHH', 0, 0, 0, 0, 0, 0))
            continue
        if Image.open(ROOT / row['path']).size != (SCAN_W, SCAN_H):
            raise ValueError(f'{row["path"]}: expected {SCAN_W}x{SCAN_H}')
        data = jpeg(ROOT / row['path'])
        # (a Japanese scan: its own measured window, tools/tcg/fetch_art.py)
        l, t, r, b = row['art_rect'] if row.get('art_rect') else LAYOUTS[layout(row)]
        l, t, r, b = l + INSET, t + INSET, r - INSET, b - INSET
        x, y = round(l * sx), round(t * sy)
        w, h = round(r * sx) - x, round(b * sy) - y
        table.append(struct.pack('<IIHHHH', offset, len(data), x, y, w, h))
        blobs.append(data)
        offset += len(data)
        report[card] = {'name': row['name'], 'printing': row['printing'], 'layout': layout(row),
                        'illustration': [x, y, w, h], 'bytes': len(data)}
    data = jpeg(ROOT / manifest['back']['path'])
    table.append(struct.pack('<IIHHHH', offset, len(data), 0, 0, STORE_W, STORE_H))
    blobs.append(data)
    out =b'TCGP' + struct.pack('<I', count) + b''.join(table) + b''.join(blobs)
    (ROOT / 'graphics/tcg/printed.bin').write_bytes(out)
    (ROOT / 'tools/tcg/printed_art.json').write_text(json.dumps(
        {'stored': [STORE_W, STORE_H], 'scan': [SCAN_W, SCAN_H], 'inset': INSET, 'layouts': LAYOUTS,
         'cards': report}, indent=1) + '\n')
    print(f'graphics/tcg/printed.bin: {len(blobs) - 1} printed cards and the back, {len(out)} bytes')


if __name__ == '__main__':
    main()
