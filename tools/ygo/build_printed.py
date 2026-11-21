#!/usr/bin/env python3
"""Build graphics/ygo/printed.bin: the printed Yu-Gi-Oh! cards, built into the game (src/ygo/ygo_art_data.c).

usage: tools/ygo/fetch_art.py && tools/ygo/build_printed.py [--keep-cards]

Each card (813x1185 from fetch_art.py) is stored as a JPEG at 160x233: the card screens draw into the 240x160 game
picture, where even the big picture shows a card about 150 pixels tall. With it goes the rectangle its
illustration fills, for CARD ART's PRINTED ART mode (the printed picture in the game's own frame): on today's
cards it sits in the same place on every kind of card, (100, 218)-(713, 831) on the 813x1185 scans, kept 1 pixel
inside here.

The card back (CARD BACKS: PRINTED, tools/ygo/metadata/back.png) is stored the same way, after the cards.
--keep-cards keeps the cards already in printed.bin (when only the back changed and the scans aren't at hand). When
the catalog has grown, the cards printed.bin already has are kept as they are and only the new ones are encoded.

printed.bin: "YGOP", u32 count, then per card id 1..count and then the back: u32 offset, u32 size (0 = no printed
card), u16 x, y, w, h (the illustration, in the stored picture's pixels; the whole picture for the back).
"""
import io
import json
import struct
import sys
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
SCAN_W, SCAN_H = 813, 1185
STORE_W, STORE_H = 160, 233
ART = (100, 218, 713, 831)
ART_PENDULUM = (55, 214, 757, 733)   # (a Pendulum Monster's illustration: wider, above the Pendulum Effect box)

cards = json.load(open(ROOT / 'tools/ygo/catalog.json'))['cards']
# printed.bin is kept as parts under GitHub's 100 MB a file (graphics/ygo/printed.0.bin ...), joined by the assembler's
# .incbin one after another (src/ygo/ygo_art_data.c) into the same bytes: PARTS of at most PART_MAX each
PARTS, PART_MAX = 3, 95 * 1000 * 1000


def read_printed():
    parts = [ROOT / ('graphics/ygo/printed.%d.bin' % i) for i in range(PARTS)]
    if all(p.exists() for p in parts):
        return b''.join(p.read_bytes() for p in parts)
    single = ROOT / 'graphics/ygo/printed.bin'   # (before the parts)
    return single.read_bytes() if single.exists() else None


def write_printed(blob):
    if len(blob) > PARTS * PART_MAX:
        raise SystemExit('printed.bin needs more than %d parts: raise PARTS here and in src/ygo/ygo_art_data.c' % PARTS)
    for i in range(PARTS):
        (ROOT / ('graphics/ygo/printed.%d.bin' % i)).write_bytes(blob[i * PART_MAX:(i + 1) * PART_MAX])
    single = ROOT / 'graphics/ygo/printed.bin'
    if single.exists():
        single.unlink()
count = max(c['id'] for c in cards)
byid = {c['id']: c for c in cards}
blobs, table = [], []
missing = []
sx, sy = STORE_W / SCAN_W, STORE_H / SCAN_H
def art_rect(card):
    a = ART_PENDULUM if card and 'scale' in card else ART
    return round(a[0] * sx) + 1, round(a[1] * sy) + 1, round(a[2] * sx) - 1, round(a[3] * sy) - 1


x0, y0, x1, y1 = art_rect(None)
old = None
if '--keep-cards' in sys.argv[1:]:
    old = read_printed()
    if old[:4] != b'YGOP' or struct.unpack_from('<I', old, 4)[0] != count:
        raise SystemExit('--keep-cards: printed.bin is from another catalog')
# (cards the catalog had before keep their pictures when it grows: only the new ones are encoded)
grown = read_printed() if old is None else None
if grown is not None and (grown[:4] != b'YGOP' or struct.unpack_from('<I', grown, 4)[0] >= count):
    grown = None
for cid in range(1, count + 1):
    if grown is not None and cid <= struct.unpack_from('<I', grown, 4)[0]:
        offset, size = struct.unpack_from('<II', grown, 8 + (cid - 1) * 16)
        if size:
            table.append(grown[offset:offset + size])
            continue
    if old is not None:
        offset, size = struct.unpack_from('<II', old, 8 + (cid - 1) * 16)
        table.append(old[offset:offset + size] if size else None)
        if not size:
            missing.append(byid[cid]['name'])
        continue
    path = ROOT / 'tools/ygo/metadata/images' / ('%d.jpg' % byid[cid]['password'])
    if not path.exists():
        missing.append(byid[cid]['name'])
        table.append(None)
        continue
    im = Image.open(path).convert('RGB')
    if im.size != (SCAN_W, SCAN_H):
        im = im.resize((SCAN_W, SCAN_H), Image.LANCZOS)
    im = im.resize((STORE_W, STORE_H), Image.LANCZOS)
    out = io.BytesIO()
    im.save(out, 'JPEG', quality=80, optimize=True)
    table.append(out.getvalue())
im = Image.open(ROOT / 'tools/ygo/metadata/back.png').convert('RGB').resize((STORE_W, STORE_H), Image.LANCZOS)
out = io.BytesIO()
im.save(out, 'JPEG', quality=85, optimize=True)
back = out.getvalue()
head = 8 + (count + 1) * 16
offset = head
body = bytearray()
entries = bytearray()
for cid, data in enumerate(table, 1):
    if data is None:
        entries += struct.pack('<IIHHHH', 0, 0, 0, 0, 0, 0)
        continue
    x0, y0, x1, y1 = art_rect(byid.get(cid))
    entries += struct.pack('<IIHHHH', offset + len(body), len(data), x0, y0, x1 - x0, y1 - y0)
    body += data
entries += struct.pack('<IIHHHH', offset + len(body), len(back), 0, 0, STORE_W, STORE_H)
body += back
blob = b'YGOP' + struct.pack('<I', count) + bytes(entries) + bytes(body)
(ROOT / 'graphics/ygo').mkdir(parents=True, exist_ok=True)
write_printed(blob)
print('%d cards, %d missing, %.1f MB -> graphics/ygo/printed.bin' % (count, len(missing), len(blob) / 1048576))
if missing:
    print('  missing:', ', '.join(missing[:20]))
