#!/usr/bin/env python3
"""Moemon Co-op: highlight sheets for hand-mapping sprite colors.

Usage: colorsheets.py DUMP_DIR OUT_DIR
DUMP_DIR comes from the test harness command "mp dumpcolors DIR". Each sheet
shows a few sprites: the sprite itself, then one copy per palette color with
that color in magenta and the rest grayed out, labeled with the color number.
"""
import os
import sys

import numpy as np
from PIL import Image, ImageDraw

PER_SHEET = {'ow': 3, 'front': 2, 'back': 2}


def dec(tiles, w, h):
    t = np.frombuffer(tiles, np.uint8).reshape(h // 8, w // 8, 8, 4)
    px = np.stack([t & 15, t >> 4], -1).reshape(h // 8, w // 8, 8, 8)
    return px.transpose(0, 2, 1, 3).reshape(h, w)


def load(dump, line):
    kind, cfg, w, h, n, fn = line.split()
    w, h, n = int(w), int(h), int(n)
    data = open(os.path.join(dump, fn), 'rb').read()
    fb = w * h // 2
    frames = [dec(data[i * fb:(i + 1) * fb], w, h) for i in range(n)]
    pal = np.frombuffer(data[n * fb:n * fb + 32], '<u2')
    cols = [((v & 31) << 3, ((v >> 5) & 31) << 3, ((v >> 10) & 31) << 3) for v in pal]
    return kind, cfg, frames, cols


def render(idx, cols, sel=None, scale=3):
    h, w = idx.shape
    im = np.full((h, w, 3), 255, np.uint8)
    for c in range(1, 16):
        m = idx == c
        if not m.any():
            continue
        if sel is None or c == sel:
            im[m] = (255, 0, 255) if sel is not None else cols[c]
        else:
            r, g, b = cols[c]
            y = int(0.3 * r + 0.59 * g + 0.11 * b)
            y = 150 + y * 90 // 255
            im[m] = (y, y, y)
    return Image.fromarray(im).resize((w * scale, h * scale), Image.NEAREST)


def main():
    dump, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    lines = [l for l in open(os.path.join(dump, 'index.txt')).read().split('\n') if l.strip()]
    lines = [l for l in lines if ' sd_' not in l]   # Showdown pictures keep their colors
    groups = {}
    for l in lines:
        groups.setdefault(l.split()[0], []).append(l)
    listing = []
    for kind, ls in groups.items():
        per = PER_SHEET[kind]
        for s0 in range(0, len(ls), per):
            chunk = [load(dump, l) for l in ls[s0:s0 + per]]
            rows = []
            for k, cfg, frames, cols in chunk:
                scale = 4 if k == 'ow' else 2
                base = Image.new('RGB', (1, 1))
                strip = [render(f, cols, None, scale) for f in frames]
                used = [c for c in range(1, 16) if any((f == c).any() for f in frames)]
                cells = [(None, s) for s in strip]
                for c in used:
                    hl = [render(f, cols, c, scale) for f in frames]
                    wsum = sum(i.width for i in hl) + 2 * (len(hl) - 1)
                    cell = Image.new('RGB', (wsum, hl[0].height), (255, 255, 255))
                    x = 0
                    for i in hl:
                        cell.paste(i, (x, 0))
                        x += i.width + 2
                    cells.append((c, cell))
                cw = sum(c[1].width + 10 for c in cells)
                ch = max(c[1].height for c in cells) + 34
                row = Image.new('RGB', (cw + 10, ch), (235, 235, 245))
                d = ImageDraw.Draw(row)
                d.text((4, 2), '%s %s' % (k, cfg), fill=(0, 0, 0))
                x = 6
                for c, cell in cells:
                    row.paste(cell, (x, 16))
                    if c is not None:
                        d.rectangle((x, 16 + cell.height + 2, x + 14, 16 + cell.height + 14), fill=cols[c], outline=(0, 0, 0))
                        d.text((x + 18, 16 + cell.height + 3), str(c), fill=(0, 0, 0))
                    x += cell.width + 10
                rows.append(row)
                listing.append((kind, cfg, used))
            W = max(r.width for r in rows)
            H = sum(r.height + 4 for r in rows)
            sheet = Image.new('RGB', (W, H), (120, 120, 130))
            y = 0
            for r in rows:
                sheet.paste(r, (0, y))
                y += r.height + 4
            name = '%s_%03d.png' % (kind, s0 // per)
            sheet.save(os.path.join(out, name))
    with open(os.path.join(out, 'assets.txt'), 'w') as f:
        for kind, cfg, used in listing:
            f.write('%s %s %s\n' % (kind, cfg, ','.join(map(str, used))))
    print('%d assets' % len(listing))


if __name__ == '__main__':
    main()
