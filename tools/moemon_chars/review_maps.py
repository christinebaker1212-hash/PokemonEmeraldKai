#!/usr/bin/env python3
"""Moemon Co-op: review sheets for recolor maps.

Usage: review_maps.py DUMP_DIR MAPS_FILE OUT_DIR KIND [PER_SHEET]
  MAPS_FILE  colormaps.txt-style lines (kind cfg MAP) to show
Each panel: the sprite, the same sprite with each color painted by its role (S skin pink, H hair yellow,
O outfit blue, A accent green, unmapped gray), and the palette: color number, swatch, role.
"""
import os
import sys

import numpy as np
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'moemon_frlg'))
from colorsheets import load  # noqa: E402

ROLE = {'S': (255, 120, 200), 'H': (250, 220, 40), 'O': (40, 110, 255), 'A': (30, 200, 60), '.': (120, 120, 120)}


def paint(idx, cols, cmap, scale):
    h, w = idx.shape
    im = np.full((h, w, 3), 215, np.uint8)
    for c in range(1, 16):
        if cmap is None:
            im[idx == c] = cols[c]
        else:
            role = cmap[c - 1]
            base = np.array(ROLE[role], np.float32)
            lum = sum(cols[c]) / 765.0
            im[idx == c] = (base * (0.45 + 0.55 * lum)).astype(np.uint8)
    return Image.fromarray(im).resize((w * scale, h * scale), Image.NEAREST)


def main():
    dump, mapsf, out, kind = sys.argv[1:5]
    per = int(sys.argv[5]) if len(sys.argv) > 5 else 8
    os.makedirs(out, exist_ok=True)
    maps = []
    for line in open(mapsf):
        p = line.split()
        if len(p) == 3 and p[0] == kind:
            maps.append((p[1], p[2]))
    index = {}
    for line in open(os.path.join(dump, 'index.txt')):
        if line.strip():
            k, cfg = line.split()[:2]
            index[(k, cfg)] = line
    panels = []
    for cfg, cmap in maps:
        _, _, frames, cols = load(dump, index[(kind, cfg)])
        scale = 4 if kind == 'ow' else 2
        shown = frames[:2] if kind == 'ow' else frames[:1]
        imgs = [paint(f, cols, None, scale) for f in shown] + [paint(f, cols, cmap, scale) for f in shown]
        used = set(int(v) for f in frames for v in np.unique(f))
        leg_w = 15 * 22
        w = sum(i.width + 4 for i in imgs) + leg_w + 8
        hgt = max(i.height for i in imgs) + 14
        panel = Image.new('RGB', (w, hgt), (250, 250, 250))
        d = ImageDraw.Draw(panel)
        d.text((2, 0), cfg, fill=(0, 0, 0))
        x = 0
        for i in imgs:
            panel.paste(i, (x, 13))
            x += i.width + 4
        for c in range(1, 16):
            lx = x + (c - 1) * 22
            d.rectangle([lx, 16, lx + 19, 34], fill=tuple(cols[c]), outline=(0, 0, 0) if c in used else (220, 0, 0))
            d.text((lx + 2, 37), '%d' % c, fill=(0, 0, 0))
            d.text((lx + 6, 50), cmap[c - 1], fill=(0, 0, 0))
            d.rectangle([lx, 64, lx + 19, 70], fill=ROLE[cmap[c - 1]])
        panels.append(panel)
    for s in range(0, len(panels), per):
        chunk = panels[s:s + per]
        W = max(p.width for p in chunk)
        H = sum(p.height + 3 for p in chunk)
        sheet = Image.new('RGB', (W, H), (90, 90, 100))
        y = 0
        for p in chunk:
            sheet.paste(p, (0, y))
            y += p.height + 3
        sheet.save(os.path.join(out, '%s_%02d.png' % (kind, s // per)))
    print('%d panels' % len(panels))


if __name__ == '__main__' and (len(sys.argv) < 2 or sys.argv[1] not in ('check', 'strips')):
    main()


def check_main():
    """check DUMP_DIR MAPS_FILE OUT_DIR KIND [PER_SHEET]: the sprite and the three test looks of colorcheck.py"""
    from colorcheck import LOOKS, presets, recolor, rgb5, to_img
    dump, mapsf, out, kind = sys.argv[2:6]
    per = int(sys.argv[6]) if len(sys.argv) > 6 else 20
    os.makedirs(out, exist_ok=True)
    P = presets()
    index = {}
    for line in open(os.path.join(dump, 'index.txt')):
        if line.strip():
            k, cfg = line.split()[:2]
            index[(k, cfg)] = line
    cells = []
    for line in open(mapsf):
        p = line.split()
        if len(p) != 3 or p[0] != kind:
            continue
        _, _, frames, cols = load(dump, index[(kind, p[1])])
        pal = [rgb5(int(r) >> 3, int(g) >> 3, int(b) >> 3) for r, g, b in cols]
        scale = int(os.environ.get('SCALE', '3' if kind == 'ow' else '1'))
        imgs = [to_img(frames[0], pal, scale)] + [to_img(frames[0], recolor(pal, p[2], lk, P), scale) for lk in LOOKS]
        if kind == 'ow':
            imgs += [to_img(frames[1], recolor(pal, p[2], LOOKS[0], P), scale)]
        w = sum(i.width + 2 for i in imgs)
        cell = Image.new('RGB', (max(w, 120), imgs[0].height + 11), (255, 255, 255))
        d = ImageDraw.Draw(cell)
        d.text((1, 0), p[1][4:24], fill=(0, 0, 0))
        x = 0
        for i in imgs:
            cell.paste(i, (x, 11))
            x += i.width + 2
        cells.append(cell)
    cw = max(c.width for c in cells) + 4
    chh = max(c.height for c in cells) + 4
    cols_n = int(os.environ.get('COLS', '4' if kind == 'ow' else '3'))
    for s in range(0, len(cells), per):
        chunk = cells[s:s + per]
        rows = (len(chunk) + cols_n - 1) // cols_n
        sheet = Image.new('RGB', (cols_n * cw, rows * chh), (120, 120, 130))
        for k, c in enumerate(chunk):
            sheet.paste(c, ((k % cols_n) * cw, (k // cols_n) * chh))
        sheet.save(os.path.join(out, 'check_%s_%02d.png' % (kind, s // per)))
    print('%d cells' % len(cells))


if __name__ == '__main__' and len(sys.argv) > 1 and sys.argv[1] == 'check':
    check_main()


def strips_main():
    """strips DUMP_DIR MAPS_FILE OUT_DIR KIND [PER_SHEET]: per sprite, the sprite and one copy per color
    with that color in magenta (the rest gray), the color's number and its role in the map under it"""
    dump, mapsf, out, kind = sys.argv[2:6]
    per = int(sys.argv[6]) if len(sys.argv) > 6 else 6
    os.makedirs(out, exist_ok=True)
    index = {}
    for line in open(os.path.join(dump, 'index.txt')):
        if line.strip():
            k, cfg = line.split()[:2]
            index[(k, cfg)] = line
    panels = []
    for line in open(mapsf):
        p = line.split()
        if len(p) != 3 or p[0] != kind:
            continue
        _, _, frames, cols = load(dump, index[(kind, p[1])])
        f = frames[0]
        scale = 3 if kind == 'ow' else 1
        if kind == 'ow':
            # S and N side by side: the back of the head shows the hair
            f = np.concatenate([frames[0], frames[1]], axis=1)
        h, w = f.shape
        used = sorted(set(int(v) for v in np.unique(f)) - {0})
        imgs = [paint(f, cols, None, scale)]
        for c in range(1, 16):
            im = np.full((h, w, 3), 230, np.uint8)
            for k in range(1, 16):
                g = sum(cols[k]) // 3
                im[f == k] = (g // 2 + 100,) * 3
            im[f == c] = (255, 0, 255)
            imgs.append(Image.fromarray(im).resize((w * scale, h * scale), Image.NEAREST))
        cw = imgs[0].width + 3
        panel = Image.new('RGB', (cw * 16, imgs[0].height + 26), (250, 250, 250))
        d = ImageDraw.Draw(panel)
        for k, im in enumerate(imgs):
            panel.paste(im, (k * cw, 0))
            if k:
                d.rectangle([k * cw, im.height + 2, k * cw + 9, im.height + 11], fill=tuple(cols[k]), outline=(0, 0, 0))
                d.text((k * cw + 12, im.height + 1), '%d%s' % (k, '' if k in used else '-'), fill=(0, 0, 0))
                d.text((k * cw + 12, im.height + 12), p[2][k - 1], fill=(200, 0, 0))
        d.text((2, imgs[0].height + 12), p[1][4:22], fill=(0, 0, 160))
        panels.append(panel)
    for s in range(0, len(panels), per):
        chunk = panels[s:s + per]
        W = max(pp.width for pp in chunk)
        H = sum(pp.height + 4 for pp in chunk)
        sheet = Image.new('RGB', (W, H), (60, 60, 70))
        y = 0
        for pp in chunk:
            sheet.paste(pp, (0, y))
            y += pp.height + 4
        sheet.save(os.path.join(out, 'strip_%s_%02d.png' % (kind, s // per)))
    print('%d panels' % len(panels))


if __name__ == '__main__' and len(sys.argv) > 1 and sys.argv[1] == 'strips':
    strips_main()
