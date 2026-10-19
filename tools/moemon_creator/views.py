"""Turns kit images (drawn at 2x) into the game's 1x frames for each view."""
import numpy as np
from PIL import Image

_cache = {}


def rgba(path):
    if path not in _cache:
        _cache[path] = np.array(Image.open(path).convert('RGBA'))
    return _cache[path]


def half(a, oy=0, ox=0):
    return a[oy::2, ox::2]


# Back sheets: 5 frames in 175-pixel cells, 2x with each frame shifted by 0 or 1 pixel
BACK_CELL = 175
BACK_ORDER = (1, 2, 3, 0)   # Emerald slots (ready, wind-up, throw, standing) <- kit frames


def back_parity(base):
    """x offset (0/1) of the 2x grid for each of the 5 frames, from the base body."""
    out = []
    for i in range(5):
        cell = base[:, i * BACK_CELL:(i + 1) * BACK_CELL]
        best = 0
        for ox in (0, 1):
            b = cell[:, ox::2]
            up = np.repeat(b, 2, 1)[:, :cell.shape[1] - ox]
            if (up == cell[:, ox:ox + up.shape[1]]).all():
                best = ox
                break
        out.append(best)
    return out


def back_frames(a, parity):
    frames = []
    for slot in range(4):
        i = BACK_ORDER[slot]
        cell = a[:, i * BACK_CELL:(i + 1) * BACK_CELL]
        frames.append(half(cell, 0, parity[i])[:, :87])
    return frames


def front_frame(a):
    return half(a)          # 80x80


# Overworld sheets: 4x4 frames of 32x32 (rows: down, left, right, up; cols: stand, step, stand, step)
OW_FRAMES = [(0, 0), (3, 0), (1, 0), (0, 1), (0, 3), (3, 1), (3, 3), (1, 1), (1, 3)]   # Emerald's 9


def ow_frames(a):
    h = half(a)
    return [h[r * 32:r * 32 + 32, c * 32:c * 32 + 32] for r, c in OW_FRAMES]


def over(dst, src):
    m = src[:, :, 3] > 0
    dst[m] = src[m]
    return dst
