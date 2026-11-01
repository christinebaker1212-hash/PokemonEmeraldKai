#!/usr/bin/env python3
# Moemon Fight's display test (fight_display): each rendershot (the window) against the fightshot of the same frame
# (the fight's own 480x320 picture), rendered here the way display.c renders the game's 240x160 in that mode:
#   PIXEL PERFECT: the GBA screen's biggest whole multiple, centred; the fight inside it at half that
#   SHARP FIT / STRETCH: 3:2 fitted / the whole window, STRETCH with only the game's own 4:3 picture (1.63.14: the
#   MUGEN cabinet's 426 columns in the middle, MF_GAME_W)
#   a whole multiple of the fight's picture: nearest, exact; otherwise sharp-bilinear (nearest to the next whole
#   multiple, then linear down to the size)
#   filters: display.c FilterMask's multiply mask (mask() below: a CRT beam's rows and grille, or the LCD's cells and
#   sub-pixel stripes), drawn when there are enough screen pixels per picture pixel (2 rows / 3x3)
# The harness saves screenshots as 2x PNGs; they are read back at their own size.
import os
import sys

import numpy as np
from PIL import Image

GBA_W, GBA_H, SRC_W, SRC_H = 240, 160, 480, 320
GAME_W = 426   # MUGEN's own picture (include/mf_engine.h MF_GAME_W), centred in the 480x320


def load(path):
    im = np.asarray(Image.open(path).convert('RGB')).astype(np.float64)
    return im[::2, ::2]


def dst_rect(mode, w, h):
    if mode == 'pp':
        k = min(w // GBA_W, h // GBA_H)
        if k >= 1:
            return (w - GBA_W * k) // 2, (h - GBA_H * k) // 2, GBA_W * k, GBA_H * k
        mode = 'fit'
    if mode == 'stretch':
        return 0, 0, w, h
    if w * GBA_H >= h * GBA_W:
        dh = h
        dw = (h * GBA_W + GBA_H // 2) // GBA_H
    else:
        dw = w
        dh = (w * GBA_H + GBA_W // 2) // GBA_W
    return (w - dw) // 2, (h - dh) // 2, dw, dh


def linear(src, dw, dh):
    """Linear filtering to dw x dh, texel centres at (i + 0.5) / size (as the renderer samples)."""
    sh, sw = src.shape[:2]
    xs = (np.arange(dw) + 0.5) * sw / dw - 0.5
    ys = (np.arange(dh) + 0.5) * sh / dh - 0.5
    x0 = np.clip(np.floor(xs).astype(int), 0, sw - 1)
    y0 = np.clip(np.floor(ys).astype(int), 0, sh - 1)
    x1 = np.clip(x0 + 1, 0, sw - 1)
    y1 = np.clip(y0 + 1, 0, sh - 1)
    fx = np.clip(xs - np.floor(xs), 0, 1)[None, :, None]
    fy = np.clip(ys - np.floor(ys), 0, 1)[:, None, None]
    top = src[y0][:, x0] * (1 - fx) + src[y0][:, x1] * fx
    bot = src[y1][:, x0] * (1 - fx) + src[y1][:, x1] * fx
    return top * (1 - fy) + bot * fy


def mask(filt, px, kx, py, ky):
    """display.c FilterMask (1.63.14): how much of each channel a filter keeps at screen pixel (px, py) of a picture
    pixel's kx x ky, as the byte it uploads"""
    import math
    m, row = [1.0, 1.0, 1.0], 1.0
    f32 = np.float32
    if filt == 'scan':
        if ky >= 2:
            t = f32((py + 0.5) / ky) - f32(0.45)
            row = f32(0.38) + f32(0.62) * f32(math.exp(-t * t / (2 * f32(0.21) * f32(0.21))))
        if kx >= 3:
            m = [1.0 if (px * 3 // kx) % 3 == c else 0.86 for c in range(3)]
    elif filt == 'grid':
        gx = gy = 1.0
        if kx >= 3:
            gx = 0.70 if px == kx - 1 else 0.88 if kx >= 8 and px == kx - 2 else 1.0
            m = [1.0 if (px * 3 // kx) % 3 == c else 0.80 for c in range(3)]
        if ky >= 3:
            gy = 0.70 if py == ky - 1 else 0.88 if ky >= 8 and py == ky - 2 else 1.0
        row = min(gx, gy)
    return [int(f32(m[c]) * f32(row) * f32(255.0) + f32(0.5)) for c in range(3)]


def overlay(kx, ky, filt, sw=SRC_W, sh=SRC_H):
    """The multiply mask over the whole kx x ky enlargement (0-255 per channel)"""
    cell = np.zeros((ky, kx, 3))
    for y in range(ky):
        for x in range(kx):
            cell[y, x] = mask(filt, x, kx, y, ky)
    return np.tile(cell, (sh, sw, 1))


def expect(src, mode, filt, w, h):
    x, y, dw, dh = dst_rect(mode, w, h)
    if mode == 'stretch':
        left = (SRC_W - GAME_W) // 2
        src = src[:, left:left + GAME_W]
    sh, sw = src.shape[:2]
    kx, ky = -(-dw // sw), -(-dh // sh)
    exact = dw == sw * kx and dh == sh * ky
    big = src.repeat(ky, 0).repeat(kx, 1)
    on = False
    if filt != 'off':
        on = (ky >= 2 if filt == 'scan' else kx >= 3 and ky >= 3) if exact else dw >= sw * 3 and dh >= sh * 3
    if on:
        big = big * overlay(kx, ky, filt, sw, sh) / 255.0   # (SDL_BLENDMODE_MOD)
    out = big if exact else linear(big, dw, dh)
    return (x, y, dw, dh), exact, out


CASES = [  # name, mode, filter
    ('pp4', 'pp', 'off'), ('pp3', 'pp', 'off'), ('pp1', 'pp', 'off'), ('fit', 'fit', 'off'),
    ('stretch', 'stretch', 'off'), ('grid3', 'pp', 'grid'), ('scan2', 'pp', 'scan'),
]


def main(results):
    bad = 0
    for name, mode, filt in CASES:
        src = load(os.path.join(results, name + '_src.png'))
        win = load(os.path.join(results, name + '.png'))
        h, w = win.shape[:2]
        (x, y, dw, dh), exact, ref = expect(src, mode, filt, w, h)
        got = win[y:y + dh, x:x + dw]
        err = np.abs(got - ref)
        mean, worst = err.mean(), np.percentile(err, 99.5)
        # outside the picture: the bars stay black
        bars = win.copy()
        bars[y:y + dh, x:x + dw] = 0
        ok = (worst <= 3 if exact else mean <= 2.5 and worst <= 40) and bars.max() <= 0
        print('%-8s %4dx%-4d picture %dx%d at %d,%d %s: mean %.2f, 99.5%% %.1f %s' % (
            name, w, h, dw, dh, x, y, 'exact' if exact else 'sharp-bilinear', mean, worst, 'ok' if ok else 'BAD'))
        bad += not ok
    if bad:
        print('%d case(s) differ from the game\'s scaling' % bad)
        sys.exit(1)
    print('ok')


if __name__ == '__main__':
    main(sys.argv[1])
