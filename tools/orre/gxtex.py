#!/usr/bin/env python3
"""GameCube GX textures (I4, I8, IA4, IA8, RGB565, RGB5A3, RGBA8, C4, C8, C14X2, CMPR) -> RGBA numpy arrays."""
import numpy as np

# format: (bits per pixel, block width, block height)
FORMATS = {0: (4, 8, 8), 1: (8, 8, 4), 2: (8, 8, 4), 3: (16, 4, 4), 4: (16, 4, 4), 5: (16, 4, 4),
           6: (32, 4, 4), 8: (4, 8, 8), 9: (8, 8, 4), 10: (16, 4, 4), 14: (4, 8, 8)}


def size(fmt, w, h):
    bpp, bw, bh = FORMATS[fmt]
    bx, by = (w + bw - 1) // bw, (h + bh - 1) // bh
    return bx * by * bw * bh * bpp // 8


def c565(v):
    v = v.astype(np.uint32)
    r = ((v >> 11) & 31) * 255 // 31
    g = ((v >> 5) & 63) * 255 // 63
    b = (v & 31) * 255 // 31
    return r, g, b


def c5a3(v):
    v = v.astype(np.uint32)
    opaque = (v & 0x8000) != 0
    r = np.where(opaque, ((v >> 10) & 31) * 255 // 31, ((v >> 8) & 15) * 17)
    g = np.where(opaque, ((v >> 5) & 31) * 255 // 31, ((v >> 4) & 15) * 17)
    b = np.where(opaque, (v & 31) * 255 // 31, (v & 15) * 17)
    a = np.where(opaque, 255, ((v >> 12) & 7) * 255 // 7)
    return r, g, b, a


def palette(raw, tfmt, n):
    """TLUT: 0 IA8, 1 RGB565, 2 RGB5A3 -> (n,4) uint8."""
    v = np.frombuffer(raw[:n * 2], dtype=">u2")
    out = np.zeros((n, 4), dtype=np.uint8)
    if tfmt == 0:
        out[:, 0] = out[:, 1] = out[:, 2] = v & 0xFF
        out[:, 3] = v >> 8
    elif tfmt == 1:
        r, g, b = c565(v)
        out[:, 0], out[:, 1], out[:, 2], out[:, 3] = r, g, b, 255
    else:
        r, g, b, a = c5a3(v)
        out[:, 0], out[:, 1], out[:, 2], out[:, 3] = r, g, b, a
    return out


def deblock(vals, w, h, bw, bh):
    """Values in block order (each block row-major) -> an image of (h, w, ...)."""
    bx, by = (w + bw - 1) // bw, (h + bh - 1) // bh
    rest = vals.shape[1:]
    a = vals[:bx * by * bw * bh].reshape((by, bx, bh, bw) + rest)
    a = a.swapaxes(1, 2).reshape((by * bh, bx * bw) + rest)
    return a[:h, :w]


def cmpr(raw, w, h):
    """CMPR (S3TC/DXT1 in 8x8 tiles of four 4x4 blocks, big endian), every block at once."""
    bx, by = (w + 7) // 8, (h + 7) // 8
    n = bx * by * 4
    d = np.frombuffer(raw[:n * 8], dtype=np.uint8).reshape(n, 8).astype(np.int32)
    c0 = (d[:, 0] << 8) | d[:, 1]
    c1 = (d[:, 2] << 8) | d[:, 3]
    def rgb(c):
        return np.stack([((c >> 11) & 31) * 255 // 31, ((c >> 5) & 63) * 255 // 63, (c & 31) * 255 // 31], 1)
    p0, p1 = rgb(c0), rgb(c1)
    gt = (c0 > c1)[:, None]
    p2 = np.where(gt, (2 * p0 + p1) // 3, (p0 + p1) // 2)
    p3 = np.where(gt, (p0 + 2 * p1) // 3, 0)
    pal = np.zeros((n, 4, 4), dtype=np.uint8)
    pal[:, 0, :3], pal[:, 1, :3], pal[:, 2, :3], pal[:, 3, :3] = p0, p1, p2, p3
    pal[:, :3, 3] = 255
    pal[:, 3, 3] = np.where(gt[:, 0], 255, 0)
    bits = d[:, 4:8]                                    # 4 rows of 4 two-bit indices, high bits first
    idx = np.stack([(bits >> (6 - 2 * c)) & 3 for c in range(4)], 2)   # (n, 4 rows, 4 cols)
    px = pal[np.arange(n)[:, None, None], idx]          # (n, 4, 4, 4)
    px = px.reshape(by, bx, 2, 2, 4, 4, 4)              # tile row, tile col, sub row, sub col, y, x, rgba
    px = px.transpose(0, 2, 4, 1, 3, 5, 6).reshape(by * 8, bx * 8, 4)
    return px[:h, :w]


def decode(raw, fmt, w, h, pal=None):
    if fmt == 14:
        return cmpr(raw, w, h)
    bpp, bw, bh = FORMATS[fmt]
    n = ((w + bw - 1) // bw) * ((h + bh - 1) // bh) * bw * bh
    d = np.frombuffer(raw, dtype=np.uint8)
    rgba = np.zeros((n, 4), dtype=np.uint8)
    if bpp == 4:
        v = np.empty(n, dtype=np.uint8)
        v[0::2] = d[:n // 2] >> 4
        v[1::2] = d[:n // 2] & 15
    elif bpp == 8:
        v = d[:n]
    if fmt == 0:
        rgba[:] = (v * 17)[:, None]
    elif fmt == 1:
        rgba[:] = v[:, None]
    elif fmt == 2:
        rgba[:, :3] = ((v & 15) * 17)[:, None]
        rgba[:, 3] = (v >> 4) * 17
    elif fmt == 3:
        p = d[:n * 2].reshape(-1, 2)
        rgba[:, 3], rgba[:, 0] = p[:, 0], p[:, 1]
        rgba[:, 1] = rgba[:, 2] = p[:, 1]
    elif fmt == 4:
        r, g, b = c565(np.frombuffer(raw[:n * 2], dtype=">u2"))
        rgba[:, 0], rgba[:, 1], rgba[:, 2], rgba[:, 3] = r, g, b, 255
    elif fmt == 5:
        r, g, b, a = c5a3(np.frombuffer(raw[:n * 2], dtype=">u2"))
        rgba[:, 0], rgba[:, 1], rgba[:, 2], rgba[:, 3] = r, g, b, a
    elif fmt == 6:
        blocks = d[:n * 4].reshape(-1, 2, 16, 2)   # per 4x4 block: 16 AR pairs, then 16 GB pairs
        rgba_b = np.zeros((blocks.shape[0], 16, 4), dtype=np.uint8)
        rgba_b[:, :, 3] = blocks[:, 0, :, 0]
        rgba_b[:, :, 0] = blocks[:, 0, :, 1]
        rgba_b[:, :, 1] = blocks[:, 1, :, 0]
        rgba_b[:, :, 2] = blocks[:, 1, :, 1]
        rgba = rgba_b.reshape(-1, 4)
    elif fmt in (8, 9, 10):
        if fmt == 10:
            v = np.frombuffer(raw[:n * 2], dtype=">u2") & 0x3FFF
        if pal is None:
            pal = np.stack([np.arange(256) % 256] * 3 + [np.full(256, 255)], 1).astype(np.uint8)
        v = np.minimum(v.astype(np.int64), len(pal) - 1)
        rgba = pal[v]
    return deblock(rgba, w, h, bw, bh)
