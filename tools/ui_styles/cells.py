"""Nitro sprite cells drawn whole (their OAMs may sit left of or above the sprite point): the mock-ups' reader."""
import sys, struct
sys.path.insert(0, __import__('os').path.join(__import__('os').path.dirname(__import__('os').path.abspath(__file__)), '..', 'sinnoh'))
import build_library_tv as L
from PIL import Image
OAM = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
       (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}
def cells(ncgr, ncer, pal, force=None, tile_base=0):
    ts, depth = L.tiles(ncgr)
    c = L.section(ncer, b'CEBK')
    n, attrs, off, mapping = struct.unpack_from('<HHII', c, 8)
    shift = mapping & 3; base = 8 + off; step = 16 if attrs & 1 else 8
    out = []
    for i in range(n):
        cnt, _, ooff = struct.unpack_from('<HHI', c, base + i * step)
        oams = []
        for k in range(cnt):
            a0, a1, a2 = struct.unpack_from('<HHH', c, base + n * step + ooff + 6 * k)
            y = (a0 & 0xFF) - (256 if a0 & 0x80 else 0)
            x = (a1 & 0x1FF) - (512 if a1 & 0x100 else 0)
            oams.append((x, y, OAM[(a0 >> 14, a1 >> 14)], ((a2 & 0x3FF) << shift) + tile_base, a2 >> 12, bool(a1 & 0x1000), bool(a1 & 0x2000)))
        if not oams:
            out.append((Image.new('RGBA', (1, 1)), 0, 0)); continue
        x0 = min(o[0] for o in oams); y0 = min(o[1] for o in oams)
        x1 = max(o[0] + o[2][0] for o in oams); y1 = max(o[1] + o[2][1] for o in oams)
        im = Image.new('RGBA', (x1 - x0, y1 - y0), (0, 0, 0, 0)); px = im.load()
        for x, y, (ow, oh), t0, pn, hf, vf in reversed(oams):
            if force is not None: pn = force
            for ty in range(oh // 8):
                for tx in range(ow // 8):
                    t = t0 + ty * (ow // 8) + tx
                    if t >= len(ts): continue
                    for j, v in enumerate(ts[t]):
                        if not v: continue
                        X, Y = tx * 8 + j % 8, ty * 8 + j // 8
                        if hf: X = ow - 1 - X
                        if vf: Y = oh - 1 - Y
                        kk = (pn * 16 + v) if depth == 3 else v
                        cc = pal[kk] if kk < len(pal) else 0
                        px[x - x0 + X, y - y0 + Y] = ((cc & 31) << 3, (cc >> 5 & 31) << 3, (cc >> 10 & 31) << 3, 255)
        out.append((im, x0, y0))
    return out
