#!/usr/bin/env python3
"""Build graphics/mtg/printed.bin: the printed Magic cards, built into the game (src/mtg/mtg_art_data.c).

usage: tools/mtg/fetch_art.py && tools/mtg/build_printed.py [CACHE_DIR]   (default ~/.cache/moemon/mtg_art)

Each card in tools/mtg/art.json (its old-frame printing's "normal" scan, 488x680, from Scryfall) is stored as a JPEG
at 160x223, the size the card screens need on the 240x160 game picture. With it goes the rectangle its illustration
fills on the scan, found by matching the printing's art crop against the scan (the old frames put the picture in
slightly different places from set to set), and the artist: Scryfall asks that an illustration shown without its
printed card credit the artist, so the card view names them.

Cards printed.bin already has keep their bytes (only new cards are encoded), so it grows as more cards play.

printed.bin: "MTGP", u32 count, u32 back offset, u32 back size, then per card (sorted by name, bytewise):
u32 name offset, u32 offset, u32 size, u16 x, y, w, h (the illustration, in the stored picture's pixels),
u32 artist offset; names and artists are NUL-terminated UTF-8 after the table.
"""
import io
import json
import os
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'graphics/mtg/printed.bin'
STORE_W, STORE_H = 160, 223
SCAN_W, SCAN_H = 488, 680


def gray(im, w, h):
    return np.asarray(im.convert('L').resize((w, h), Image.BILINEAR), dtype=np.float32)


def match(scan, crop):
    """The illustration's rectangle on the 488x680 scan: the art crop's scale and place with the least difference
    (at a quarter size first, then refined at full size)."""
    best = None
    q = 4
    small = gray(scan, SCAN_W // q, SCAN_H // q)
    for s in np.arange(0.56, 0.76, 0.01):
        w, h = int(round(crop.width * s / q)), int(round(crop.height * s / q))
        if w < 8 or h < 8 or w >= small.shape[1] or h >= small.shape[0]:
            continue
        t = gray(crop, w, h)
        t = t - t.mean()
        for y in range(0, small.shape[0] // 2 - h // 3):
            for x in range(0, small.shape[1] - w):
                win = small[y:y + h, x:x + w]
                d = float(((win - win.mean() - t) ** 2).mean())
                if best is None or d < best[0]:
                    best = (d, s, x * q, y * q)
    _, s, bx, by = best
    full = gray(scan, SCAN_W, SCAN_H)
    best = None
    for s2 in np.arange(s - 0.012, s + 0.0121, 0.003):
        w, h = int(round(crop.width * s2)), int(round(crop.height * s2))
        t = gray(crop, w, h)
        t = t - t.mean()
        for y in range(max(0, by - 6), min(SCAN_H - h, by + 7)):
            for x in range(max(0, bx - 6), min(SCAN_W - w, bx + 7)):
                win = full[y:y + h, x:x + w]
                d = float(((win - win.mean() - t) ** 2).mean())
                if best is None or d < best[0]:
                    best = (d, x, y, w, h)
    return best[1:]


def read_old():
    """name -> (jpeg bytes, rect, artist) from the printed.bin there is"""
    if not OUT.exists():
        return {}
    b = OUT.read_bytes()
    if b[:4] != b'MTGP':
        return {}
    count = struct.unpack_from('<I', b, 4)[0]
    old = {}
    for i in range(count):
        no, off, size, x, y, w, h, ao = struct.unpack_from('<IIIHHHHI', b, 16 + i * 24)
        name = b[no:b.index(b'\0', no)].decode()
        artist = b[ao:b.index(b'\0', ao)].decode()
        old[name] = (b[off:off + size], (x, y, w, h), artist)
    return old


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    cache = Path(args[0] if args else os.path.join(os.path.expanduser('~'), '.cache', 'moemon', 'mtg_art'))
    listing = json.load(open(ROOT / 'tools/mtg/art.json', encoding='utf-8'))
    old = read_old()
    cards = {}
    fresh = 0
    for name in sorted(listing, key=lambda n: n.encode()):
        e = listing[name]
        if name in old and old[name][2] == e['artist']:
            cards[name] = old[name]
            continue
        scan_path = cache / ('%s.normal.jpg' % e['file'])
        crop_path = cache / ('%s.art_crop.jpg' % e['file'])
        if not scan_path.exists() or not crop_path.exists():
            print('missing scan: %s' % name)
            continue
        scan = Image.open(scan_path).convert('RGB')
        if scan.size != (SCAN_W, SCAN_H):
            scan = scan.resize((SCAN_W, SCAN_H), Image.LANCZOS)
        x, y, w, h = match(scan, Image.open(crop_path).convert('RGB'))
        sx, sy = STORE_W / SCAN_W, STORE_H / SCAN_H
        rect = (int(x * sx + 0.999), int(y * sy + 0.999), int(w * sx) - 1, int(h * sy) - 1)
        out = io.BytesIO()
        scan.resize((STORE_W, STORE_H), Image.LANCZOS).save(out, 'JPEG', quality=82, optimize=True)
        cards[name] = (out.getvalue(), rect, e['artist'])
        fresh += 1
        if fresh % 50 == 0:
            print('%d encoded' % fresh)
    back = io.BytesIO()
    Image.open(cache / 'back.jpg').convert('RGB').resize((STORE_W, STORE_H), Image.LANCZOS).save(
        back, 'JPEG', quality=85, optimize=True)
    names = sorted(cards, key=lambda n: n.encode())
    head = 16 + 24 * len(names)
    strings = bytearray()
    offsets = {}
    for n in names:
        for s in (n, cards[n][2]):
            if s not in offsets:
                offsets[s] = head + len(strings)
                strings += s.encode() + b'\0'
    body = bytearray()
    base = head + len(strings)
    base += (-base) % 4
    pad = base - head - len(strings)
    table = bytearray()
    for n in names:
        data, (x, y, w, h), artist = cards[n]
        table += struct.pack('<IIIHHHHI', offsets[n], base + len(body), len(data), x, y, w, h, offsets[artist])
        body += data
    back_off = base + len(body)
    body += back.getvalue()
    blob = b'MTGP' + struct.pack('<III', len(names), back_off, len(back.getvalue())) + table + strings + \
        b'\0' * pad + body
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_bytes(blob)
    print('%d cards (%d new), %d KB -> %s' % (len(names), fresh, len(blob) // 1024, OUT.relative_to(ROOT)))


if __name__ == '__main__':
    main()
