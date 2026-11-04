#!/usr/bin/env python3
"""Trim the card border out of the GB2 cards' printed illustrations (graphics/tcg/printed.bin).

usage: tools/tcg/trim_printed_rects.py        (after build_printed_art.py; works on printed.bin itself)

The GB2 cards' printings (ids 229-442: Team Rocket, the Japanese scans with their own measured windows, the promos)
sit a little differently on their scans than their layout's rectangle, and the yellow card border showed in the
corners or along an edge of the cut-out picture in the Game Boy frame. An edge whose outermost line (or a corner of
it) is the border's yellow moves in past the yellow, at most MAX_TRIM pixels (none when it is yellow all the way in:
yellow art). A wedge left in a rounded corner is cleaned when the card is drawn (src/tcg/tcg_gfx.c
CleanArtCorners). Only the rectangles change; the pictures are kept as they are.
"""
import io, json, struct, sys
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
FIRST_GB2, LAST_GB2 = 229, 442
MAX_TRIM = 12
CORNER = 8


def yellow(p):
    r, g, b = p
    return r > 140 and g > 100 and b < g * 0.62 and r - b > 70


def lum(p):
    return 0.3 * p[0] + 0.59 * p[1] + 0.11 * p[2]


def line_at(side, l, t, r, b, k):
    """the line K pixels in from SIDE"""
    if side == 'l': return [(l + k, y) for y in range(t, b)]
    if side == 'r': return [(r - 1 - k, y) for y in range(t, b)]
    if side == 't': return [(x, t + k) for x in range(l, r)]
    return [(x, b - 1 - k) for x in range(l, r)]


def border_depth(im, side, l, t, r, b):
    """How far the card border reaches into the rectangle on SIDE (0: it doesn't): when the outermost line (or a
    corner of it) is the border's yellow, the edge moves to the first line inward that isn't, plus one (the bevel's
    soft inner edge), if one comes within MAX_TRIM. Yellow art (EKANS' sand) stays yellow further in: untouched."""
    def shares(k):
        pts = line_at(side, l, t, r, b, k)
        ys = [yellow(im.getpixel(q)) for q in pts]
        return sum(ys) / len(ys), max(sum(ys[:CORNER]), sum(ys[-CORNER:])) / CORNER
    whole, ends = shares(0)
    corner_only = whole < 0.25
    if corner_only and ends < 0.4:
        return 0
    for k in range(1, MAX_TRIM):
        w, e = shares(k)
        if (e if corner_only else w) < 0.15:
            return min(k + 1, MAX_TRIM)
    return 0


def trim(im, x, y, w, h):
    l, t, r, b = x, y, x + w, y + h
    moved = {side: border_depth(im, side, l, t, r, b) for side in 'ltrb'}
    l, t, r, b = l + moved['l'], t + moved['t'], r - moved['r'], b - moved['b']
    return l, t, r - l, b - t, moved


def main():
    path = ROOT / 'graphics/tcg/printed.bin'
    data = bytearray(path.read_bytes())
    count = struct.unpack('<I', data[4:8])[0]
    report_path = ROOT / 'tools/tcg/printed_art.json'
    report = json.loads(report_path.read_text())
    trimmed = 0
    for card in range(FIRST_GB2, min(LAST_GB2, count) + 1):
        at = 8 + 16 * (card - 1)
        off, size, x, y, w, h = struct.unpack('<IIHHHH', data[at:at + 16])
        if not size or 'energy' in report['cards'].get(str(card), {}).get('layout', ''):
            continue
        im = Image.open(io.BytesIO(bytes(data[off:off + size]))).convert('RGB')
        nx, ny, nw, nh, moved = trim(im, x, y, w, h)
        if (nx, ny, nw, nh) != (x, y, w, h):
            data[at:at + 16] = struct.pack('<IIHHHH', off, size, nx, ny, nw, nh)
            report['cards'][str(card)]['illustration'] = [nx, ny, nw, nh]
            report['cards'][str(card)]['trimmed'] = moved
            trimmed += 1
    path.write_bytes(bytes(data))
    report_path.write_text(json.dumps(report, indent=1) + '\n')
    print(f'graphics/tcg/printed.bin: {trimmed} GB2 illustrations trimmed of the card border')


if __name__ == '__main__':
    sys.exit(main())
