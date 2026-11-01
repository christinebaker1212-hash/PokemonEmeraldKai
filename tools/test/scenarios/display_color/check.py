#!/usr/bin/env python3
# display_color: each COLOR's window (3x, nearest) is the game's frame through that profile (display.c BuildColorLut)
import os, sys
import numpy as np
from PIL import Image

PROFILES = [  # rows: R, G, B out from R, G, B in; luminance; the shader's screen darkness
    None,
    ([[0.905, 0.195, -0.10], [0.10, 0.65, 0.25], [0.1575, 0.1425, 0.70]], 0.91, 1.0),
    ([[0.905, 0.195, -0.10], [0.10, 0.65, 0.25], [0.1575, 0.1425, 0.70]], 0.91, 0.0),
    ([[0.96, 0.11, -0.07], [0.0325, 0.89, 0.0775], [0.001, -0.03, 1.029]], 0.935, 0.0),
    ([[0.835, 0.27, -0.105], [0.10, 0.6375, 0.2625], [0.105, 0.175, 0.72]], 0.905, 0.0),
]


def load(path):
    return np.asarray(Image.open(path).convert('RGB')).astype(np.float64)


def main(d):
    bad = 0
    for i, prof in enumerate(PROFILES):
        base = load(os.path.join(d, 'b%d.png' % i))      # the game's frame of that shot (people walk about)
        k = base.shape[1] // 240
        base = base[::k, ::k]
        five = np.round(base * 31 / 255)                # its 5-bit channels
        win = load(os.path.join(d, 'c%d.png' % i))
        win = win[::win.shape[1] // 720, ::win.shape[1] // 720][1::3, 1::3]   # each game pixel's middle at 3x
        if prof is None:
            ref = base
        else:
            m, lum, dark = prof
            lin = (five / 31) ** 2.2
            out = np.clip(np.einsum('ij,yxj->yxi', np.array(m), lin) * lum, 0, 1)
            ref = np.floor(out ** (1 / 2.2 + dark / 4.84) * 255 + 0.5)
        err = np.abs(win - ref)
        ok = err.max() <= 3
        print('color %d: worst %d, mean %.2f %s' % (i, err.max(), err.mean(), 'ok' if ok else 'BAD'))
        bad += not ok
    # the screen grids at 3x (display.c FilterMask): MATCH takes the COLOR's own, and any grid goes with any COLOR
    GRIDS = {'gba': (0.58, 0.82, 0.78), 'nds': (0.76, 0.90, 0.84)}
    for name, color, grid in (('m1', 1, 'gba'), ('m2', 4, 'nds'), ('m3', 1, 'nds')):
        base = load(os.path.join(d, 'g' + name[1] + '.png'))
        k = base.shape[1] // 240
        five = np.round(base[::k, ::k] * 31 / 255)
        m, lum, dark = PROFILES[color]
        out = np.clip(np.einsum('ij,yxj->yxi', np.array(m), (five / 31) ** 2.2) * lum, 0, 1)
        ref = np.floor(out ** (1 / 2.2 + dark / 4.84) * 255 + 0.5).repeat(3, 0).repeat(3, 1)
        g = GRIDS[grid]
        cell = np.ones((3, 3, 3))
        for y in range(3):
            for x in range(3):
                row = min(g[0] if x == 2 else 1.0, g[0] if y == 2 else 1.0)
                for ch in range(3):
                    cell[y, x, ch] = int(np.float32(1.0 if x == ch else g[2]) * np.float32(row) * np.float32(255) + np.float32(0.5))
        ref = np.floor(ref * np.tile(cell, (160, 240, 1)) / 255)
        win = load(os.path.join(d, name + '.png'))
        win = win[::win.shape[1] // 720, ::win.shape[1] // 720]
        err = np.abs(win - ref)
        ok = np.percentile(err, 99.9) <= 2
        print('%s (color %d, %s grid): 99.9%% %.1f, mean %.2f %s' % (name, color, grid, np.percentile(err, 99.9), err.mean(),
                                                                     'ok' if ok else 'BAD'))
        bad += not ok
    # the profiles really differ from the frame and from each other
    shots = [load(os.path.join(d, 'c%d.png' % i)) for i in range(5)]
    for i in range(5):
        for j in range(i + 1, 5):
            if np.abs(shots[i] - shots[j]).mean() < 1:
                print('colors %d and %d look the same' % (i, j))
                bad += 1
    if bad:
        sys.exit('%d problem(s)' % bad)
    print('ok')


if __name__ == '__main__':
    main(sys.argv[1])
