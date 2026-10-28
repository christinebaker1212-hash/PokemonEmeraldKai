#!/usr/bin/env python3
"""Build graphics/ygo/printed.bin: the printed Yu-Gi-Oh! cards, built into the game (src/ygo/ygo_art_data.c).

usage: tools/ygo/fetch_art.py && tools/ygo/build_printed.py

Each card (813x1185 from fetch_art.py) is stored as a JPEG at 160x233: the card screens draw into the 240x160 game
picture, where even the big picture shows a card about 150 pixels tall. With it goes the rectangle its
illustration fills, for CARD ART's PRINTED ART mode (the printed picture in the game's own frame): on today's
cards it sits in the same place on every kind of card, (100, 218)-(713, 831) on the 813x1185 scans, kept 1 pixel
inside here.

printed.bin: "YGOP", u32 count, then per card id 1..count: u32 offset, u32 size (0 = no printed card), u16 x, y,
w, h (the illustration, in the stored picture's pixels).
"""
import io
import json
import struct
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
SCAN_W, SCAN_H = 813, 1185
STORE_W, STORE_H = 160, 233
ART = (100, 218, 713, 831)

cards = json.load(open(ROOT / 'tools/ygo/catalog.json'))['cards']
count = max(c['id'] for c in cards)
byid = {c['id']: c for c in cards}
blobs, table = [], []
missing = []
sx, sy = STORE_W / SCAN_W, STORE_H / SCAN_H
x0, y0 = round(ART[0] * sx) + 1, round(ART[1] * sy) + 1
x1, y1 = round(ART[2] * sx) - 1, round(ART[3] * sy) - 1
for cid in range(1, count + 1):
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
head = 8 + count * 16
offset = head
body = bytearray()
entries = bytearray()
for data in table:
    if data is None:
        entries += struct.pack('<IIHHHH', 0, 0, 0, 0, 0, 0)
        continue
    entries += struct.pack('<IIHHHH', offset + len(body), len(data), x0, y0, x1 - x0, y1 - y0)
    body += data
blob = b'YGOP' + struct.pack('<I', count) + bytes(entries) + bytes(body)
(ROOT / 'graphics/ygo').mkdir(parents=True, exist_ok=True)
(ROOT / 'graphics/ygo/printed.bin').write_bytes(blob)
print('%d cards, %d missing, %.1f MB -> graphics/ygo/printed.bin' % (count, len(missing), len(blob) / 1048576))
if missing:
    print('  missing:', ', '.join(missing[:20]))
