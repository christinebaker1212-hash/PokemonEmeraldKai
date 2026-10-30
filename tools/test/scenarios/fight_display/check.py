#!/usr/bin/env python3
# Moemon Fight's display test (fight_display): each rendershot (the window) against the fightshot of the same frame
# (the fight's own 480x320 picture), rendered here the way display.c renders the game's 240x160 in that mode:
#   PIXEL PERFECT: the GBA screen's biggest whole multiple, centred; the fight inside it at half that
#   SHARP FIT / STRETCH: 3:2 fitted / the whole window
#   a whole multiple of the fight's picture: nearest, exact; otherwise sharp-bilinear (nearest to the next whole
#   multiple, then linear down to the size)
#   filters: scanlines (the last row of each picture pixel darker, alpha 72) or the LCD grid (last row and column,
#   alpha 60), drawn when there are enough screen pixels per picture pixel (2 rows / 3x3)
# The harness saves screenshots as 2x PNGs; they are read back at their own size.
import os
import sys

import numpy as np
from PIL import Image

GBA_W, GBA_H, SRC_W, SRC_H = 240, 160, 480, 320


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


def overlay(kx, ky, filt):
    def scan(p, k):
        return 72 if k >= 2 and p == k - 1 else 32 if k >= 6 and p == k - 2 else 0

    def grid(p, k):
        return 60 if k >= 3 and p == k - 1 else 24 if k >= 8 and p == k - 2 else 0
    a = np.zeros((SRC_H * ky, SRC_W * kx))
    for y in range(ky):
        for x in range(kx):
            a[y::ky, x::kx] = max(grid(y, ky), grid(x, kx)) if filt == 'grid' else scan(y, ky)
    return a


def expect(src, mode, filt, w, h):
    x, y, dw, dh = dst_rect(mode, w, h)
    kx, ky = -(-dw // SRC_W), -(-dh // SRC_H)
    exact = dw == SRC_W * kx and dh == SRC_H * ky
    big = src.repeat(ky, 0).repeat(kx, 1)
    on = False
    if filt != 'off':
        on = (ky >= 2 if filt == 'scan' else kx >= 3 and ky >= 3) if exact else dw >= SRC_W * 3 and dh >= SRC_H * 3
    if on:
        big = big * (255 - overlay(kx, ky, filt)[:, :, None]) / 255.0
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
