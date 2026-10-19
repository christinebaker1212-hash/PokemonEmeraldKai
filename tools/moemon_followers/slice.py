#!/usr/bin/env python3
"""Moemon Co-op: cuts the follower Pokemon overworld sheets into frames.

sheets/kanto.png, johto.png, hoenn.png are the HGSS-style overworld rips
(national dex order). Each Pokemon has a cell: normally 64x128 with 32x32
frames (column 1: up, up, down, down; column 2: left, left, right, right).
Lugia and Ho-Oh use four cells with 64x64 frames (columns: up, down, left,
right). Kyogre, Groudon, Rayquaza and Wailord use two cells where the rows
are down, up, left, right and the columns are the two walking frames.
A blue dot marks a male variant, pink the female one, orange an alternate
form (skipped). Johto has Unown's 28 letters.

Output (default ../../graphics/moemon/followers):
  followers.bin   frames (4bpp GBA tiles) + palettes, see FORMAT below
  index.json      what's in it (also used to make src/data/mp_followers.h)
  check_*.png     contact sheets for checking the mapping by eye

FORMAT: per entry, 8 frames in the order down1 down2 up1 up2 left1 left2
right1 right2, each size*size pixels as 4bpp tiles (row-major 8x8 tiles),
then 16 RGB555 palette entries (index 0 = transparent).
"""
import json, os, sys
from collections import Counter
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
LINE = {'kanto.png': (0, 64, 128), 'johto.png': (85, 85, 85), 'hoenn.png': (255, 255, 255)}
MARK_MALE, MARK_FEMALE, MARK_FORM = (0, 128, 255), (255, 0, 255), (255, 128, 64)
MARKS = {MARK_MALE, MARK_FEMALE, MARK_FORM}

def lines(im, lc, axis):
    W, H = im.size
    px = im.load()
    if axis == 'x':
        found = [x for x in range(W) if sum(px[x, y] == lc for y in range(H)) > H * 0.3]
    else:
        found = [y for y in range(H) if sum(px[x, y] == lc for x in range(W)) > W * 0.3]
    # collapse runs; keep only thin lines (a thick run is the legend)
    out, run = [], []
    for v in found + [None]:
        if run and (v is None or v != run[-1] + 1):
            if len(run) <= 2:
                out.append(run[-1])
            run = []
        if v is not None:
            run.append(v)
    return out

def bands(edges, total, pitch=None):
    starts = [0] + [e + 1 for e in edges]
    ends = edges + [total]
    out = []
    for s, e in zip(starts, ends):
        if e - s < 40:
            continue
        # A band with no line inside it (the bottom rows run into the legend):
        # cut it into rows of the normal pitch
        while pitch and e - s > pitch + 8:
            out.append((s, s + pitch - 1))
            s += pitch
        out.append((s, e))
    return out

def line_present(px, lc, x, y0, y1):
    n = sum(px[x, y] == lc for y in range(y0, y1))
    return n >= (y1 - y0) * 0.7

def cell_bg(im, box):
    x0, y0, x1, y1 = box
    px = im.load()
    c = Counter(px[x, y] for x in range(x0, x1) for y in range(y0, y1))
    return c.most_common(1)[0][0]

def marker(im, box):
    x0, y0, x1, y1 = box
    px = im.load()
    c = Counter(px[x, y] for x in range(x0, x1) for y in range(y0, y1))
    for m in (MARK_FEMALE, MARK_FORM, MARK_MALE):
        if c.get(m, 0) >= 6:
            return m
    return None

def is_empty(im, box, bg):
    x0, y0, x1, y1 = box
    px = im.load()
    return sum(px[x, y] != bg and px[x, y] not in MARKS for x in range(x0, x1) for y in range(y0, y1)) < 20

def crop_frame(im, box, bg, size):
    """Frame from box, background -> transparent, placed bottom-centered on a size x size canvas."""
    fr = im.crop(box).convert('RGBA')
    px = fr.load()
    for y in range(fr.size[1]):
        for x in range(fr.size[0]):
            p = px[x, y][:3]
            if p == bg or p in MARKS:
                px[x, y] = (0, 0, 0, 0)
    bb = fr.getbbox()
    out = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    if not bb:
        return out
    spr = fr.crop(bb)
    w, h = spr.size
    if w > size or h > size:
        spr.thumbnail((size, size), Image.NEAREST)
        w, h = spr.size
    # keep the sprite's own position when it fits the canvas as cut (32x32 cells);
    # otherwise bottom-center it
    if fr.size == (size, size):
        out.paste(spr, bb[:2], spr)
    else:
        out.paste(spr, ((size - w) // 2, size - h - 2), spr)
    return out

def slice_sheet(name):
    im = Image.open(os.path.join(HERE, 'sheets', name)).convert('RGB')
    lc = LINE[name]
    px = im.load()
    W, H = im.size
    cols = bands(lines(im, lc, 'x'), W)
    rows = bands(lines(im, lc, 'y'), H, None if name == 'hoenn.png' else 129)
    cells = []
    for (y0, y1) in rows:
        c = 0
        while c < len(cols):
            x0, x1 = cols[c]
            span = 1
            # a missing line on the right = a wide (big Pokemon) cell. Only the
            # Hoenn sheet has those; elsewhere same-colored neighbors just lack a line.
            while name == 'hoenn.png' and c + span < len(cols) and not line_present(px, lc, cols[c + span - 1][1], y0, y1):
                span += 1
            x1 = cols[c + span - 1][1]
            # The last band also holds the sheet's legend: only real cells, 128 tall
            top = (x0, y0, x1, min(y1, y0 + 130)) if span == 1 else (x0, y0, x1, y1)
            cells.append({'box': top, 'span': span, 'band': (y0, y1), 'col': c})
            c += span
    return im, cells

def frames_for(im, cell, bg, kind):
    x0, y0, x1, y1 = cell['box']
    if kind == 'normal':
        f = lambda c, r: crop_frame(im, (x0 + 32 * c, y0 + 32 * r, x0 + 32 * c + 32, y0 + 32 * r + 32), bg, 32)
        # down1 down2 up1 up2 left1 left2 right1 right2
        return 32, [f(0, 2), f(0, 3), f(0, 0), f(0, 1), f(1, 0), f(1, 1), f(1, 2), f(1, 3)]
    if kind == 'quad':  # Steelix / Lugia / Ho-Oh: 4 cells (up, down, left, right), 64x64 frames
        boxes = cell['quad']
        def f(c, r):
            bx0, by0, bx1 = boxes[c][0], boxes[c][1], boxes[c][2]
            return crop_frame(im, (bx0, by0 + 64 * r, min(bx1, bx0 + 64), by0 + 64 * r + 64), bg, 64)
        return 64, [f(1, 0), f(1, 1), f(0, 0), f(0, 1), f(2, 0), f(2, 1), f(3, 0), f(3, 1)]
    # 'rows': rows down, up, left, right; columns frame 1, 2. The rows aren't
    # evenly spaced, so find them from the empty lines between sprites.
    px = im.load()
    def busy(x, y):
        p = px[x, y]
        return p != bg and p not in MARKS and p != (255, 255, 255)
    occ = [sum(busy(x, y) for x in range(x0, x1)) for y in range(y0, y1)]
    runs, start = [], None
    for i, n in enumerate(occ + [0]):
        if n and start is None:
            start = i
        elif not n and start is not None:
            if i - start >= 6:
                runs.append((y0 + start, y0 + i))
            start = None
    if len(runs) != 4:
        h = (y1 - y0) / 4.0
        runs = [(int(y0 + h * r), int(y0 + h * (r + 1))) for r in range(4)]
    frames = []
    for (ry0, ry1) in runs:
        cocc = [sum(busy(x, y) for y in range(ry0, ry1)) for x in range(x0, x1)]
        # split at the emptiest column near the middle
        mid = (x1 - x0) // 2
        best = min(range(mid - 12, mid + 13), key=lambda k: (cocc[k], abs(k - mid)))
        frames.append(crop_frame(im, (x0, ry0, x0 + best, ry1), bg, 64))
        frames.append(crop_frame(im, (x0 + best, ry0, x1, ry1), bg, 64))
    return 64, frames

def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, '..', '..', 'graphics', 'moemon', 'followers')
    os.makedirs(outdir, exist_ok=True)
    os.makedirs(os.path.join(HERE, 'check'), exist_ok=True)
    entries = []
    dex = 0
    checks = []
    for name in ('kanto.png', 'johto.png', 'hoenn.png'):
        im, cells = slice_sheet(name)
        unown_left = 0
        quad_skip = 0
        last_band = max(c['band'] for c in cells)
        legend_cols = {'kanto.png': 3, 'johto.png': 1, 'hoenn.png': 99}[name]
        for i, cell in enumerate(cells):
            if cell['band'] == last_band and cell['col'] >= legend_cols:
                continue  # legend text
            if quad_skip:
                quad_skip -= 1
                continue
            bg = cell_bg(im, cell['box'])
            if is_empty(im, cell['box'], bg):
                continue
            m = marker(im, cell['box'])
            kind = 'normal'
            if cell['span'] >= 2:
                kind = 'rows'
            variant = 0
            if m == MARK_FEMALE:
                variant = 1
            elif m == MARK_FORM or unown_left > 0 or (dex == 386 and name == 'hoenn.png'):
                variant = 2   # alternate form (skipped in game for now)
                if unown_left > 0:
                    unown_left -= 1
            else:
                dex += 1
                if dex == 201:
                    unown_left = 27
            if dex in (208, 249, 250) and name == 'johto.png':
                # Steelix / Lugia / Ho-Oh: the next four cells in reading order
                quad = [c['box'] for c in cells[i:i + 4]]
                cell = dict(cell, quad=quad)
                kind = 'quad'
                quad_skip = 3
            size, frames = frames_for(im, cell, bg, kind)
            entries.append({'dex': dex, 'variant': variant, 'size': size, 'frames': frames, 'sheet': name, 'cell': i})
    print('species reached', dex, 'entries', len(entries))
    # contact sheet for checking
    per_row = 12
    for start in range(0, len(entries), 120):
        chunk = entries[start:start + 120]
        sheet = Image.new('RGB', (per_row * 72, ((len(chunk) + per_row - 1) // per_row) * 84), (60, 90, 70))
        d = ImageDraw.Draw(sheet)
        for k, e in enumerate(chunk):
            x, y = (k % per_row) * 72, (k // per_row) * 84
            fr = e['frames'][0]
            if e['size'] == 64:
                fr = fr.resize((32, 32), Image.NEAREST)
            big = fr.resize((64, 64), Image.NEAREST)
            sheet.paste(big, (x + 4, y + 14), big)
            d.text((x + 2, y + 1), '%d%s' % (e['dex'], {0: '', 1: 'f', 2: '*'}[e['variant']]), fill=(255, 255, 255))
        sheet.save(os.path.join(HERE, 'check', 'check_%03d.png' % (start // 120)))
    # binary
    blob = bytearray()
    index = []
    for e in entries:
        frames = e['frames']
        # one palette for all 8 frames, at most 15 colors
        strip = Image.new('RGBA', (e['size'] * 8, e['size']), (0, 0, 0, 0))
        for k, fr in enumerate(frames):
            strip.paste(fr, (k * e['size'], 0), fr)
        opaque = [p[:3] for p in (strip.get_flattened_data() if hasattr(strip, 'get_flattened_data') else strip.getdata()) if p[3] >= 128]
        colors = [c for c, _ in Counter(opaque).most_common()]
        if len(colors) > 15:
            rgb = Image.new('RGB', strip.size, (0, 0, 0))
            rgb.paste(strip, (0, 0), strip)
            q = rgb.quantize(colors=15, method=Image.MEDIANCUT)
            pal = q.getpalette()[:45]
            colors = [tuple(pal[i * 3:i * 3 + 3]) for i in range(15)]
        lut = {}
        def nearest(c):
            if c not in lut:
                lut[c] = 1 + min(range(len(colors)), key=lambda i: sum((a - b) ** 2 for a, b in zip(colors[i], c)))
            return lut[c]
        off = len(blob)
        sz = e['size']
        for fr in frames:
            data = fr.load()
            for ty in range(sz // 8):
                for tx in range(sz // 8):
                    for y in range(8):
                        for x in range(0, 8, 2):
                            def idx(xx):
                                p = data[tx * 8 + xx, ty * 8 + y]
                                return 0 if p[3] < 128 else nearest(p[:3])
                            blob.append(idx(x) | (idx(x + 1) << 4))
        palette = [(0, 0, 0)] + colors + [(0, 0, 0)] * (15 - len(colors))
        for (r, g, b) in palette[:16]:
            v = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)
            blob += bytes((v & 255, v >> 8))
        index.append({'dex': e['dex'], 'variant': e['variant'], 'size': sz, 'offset': off, 'sheet': e['sheet'], 'cell': e['cell']})
    with open(os.path.join(outdir, 'followers.bin'), 'wb') as f:
        f.write(blob)
    with open(os.path.join(outdir, 'index.json'), 'w') as f:
        json.dump(index, f, indent=0)
    # C table for the game (src/data/mp_followers.h)
    header = os.path.join(HERE, '..', '..', 'src', 'data', 'mp_followers.h')
    with open(header, 'w') as f:
        f.write('// Generated by tools/moemon_followers/slice.py from the overworld sheets. Do not edit.\n')
        f.write('// dex = national dex number, variant 0 = normal, 1 = female, 2 = alternate form\n')
        f.write('// size = frame width/height (32 or 64), offset = into graphics/moemon/followers/followers.bin\n')
        f.write('static const struct MpFollowerGfx sMpFollowerGfx[] = {\n')
        for e in index:
            f.write('    {%d, %d, %d, %d},\n' % (e['dex'], e['variant'], e['size'], e['offset']))
        f.write('};\n')
    print('wrote %d bytes' % len(blob))

if __name__ == '__main__':
    main()
