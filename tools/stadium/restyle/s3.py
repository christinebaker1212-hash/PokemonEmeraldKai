# More of STADIUM 2's materials, measured from the user's three screenshots (entry, pre-battle, result):
#   plate / window / hpbar   the silver plates with pictures set into them, the HP bars under them
#   neon                     the glowing WIN / LOSE / VS
#   night / ribbon / sky     the black backdrop with its dotted lines, the titles' ribbon, the entry screen's sky
#   tag / key / listpanel / cell / brackets   the entry screen: the 1P tag, the tab keys, the blue list, the cursor
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s2 import *
from PIL import ImageFilter

YELLOW = 0xd8dc48
YELLOW_INK = 0x2c2c00
TYPE_RGB = {'NORMAL': 0xa8a878, 'FIRE': 0xf08030, 'WATER': 0x6890f0, 'GRASS': 0x78c850, 'ELECTRIC': 0xf8d030, 'ICE': 0x98d8d8,
            'FIGHT': 0xc03028, 'POISON': 0xa040a0, 'GROUND': 0xe0c068, 'FLYING': 0xa890f0, 'PSYCHIC': 0xf85888, 'BUG': 0xa8b820,
            'ROCK': 0xb8a038, 'GHOST': 0x705898, 'DRAGON': 0x7038f8, 'DARK': 0x705848, 'STEEL': 0xb8b8d0}
_rng = np.random.default_rng(11)


def pil_blur(a, r):
    im = Image.fromarray((np.clip(a, 0, 1) * 255).astype(np.uint8), 'L').filter(ImageFilter.GaussianBlur(r))
    return np.array(im).astype(np.float32) / 255


def fbm(w, h, cells, stretch=2.0):
    """Soft noise 0..1 (the sky's clouds), wider than tall."""
    out = np.zeros((h, w), np.float32)
    amp, tot = 1.0, 0.0
    for cell in cells:
        small = _rng.random((max(2, int(h / cell) + 2), max(2, int(w / (cell * stretch)) + 2))).astype(np.float32)
        big = np.array(Image.fromarray(small, 'F').resize((w, h), Image.BICUBIC))
        out += big * amp
        tot += amp
        amp *= 0.5
    return out / tot


class C3(Canvas):
    def clip_blit(self, rgba, x, y, box, alpha=1.0, dim=1.0):
        """A picture cut to a box (a window's inside)."""
        x, y = int(round(x)), int(round(y))
        bx0, by0, bx1, by1 = [int(v) for v in box]
        sx0, sy0 = max(bx0 - x, 0), max(by0 - y, 0)
        sx1, sy1 = min(bx1 - x, rgba.shape[1]), min(by1 - y, rgba.shape[0])
        if sx1 <= sx0 or sy1 <= sy0:
            return
        part = rgba[sy0:sy1, sx0:sx1].copy()
        part[..., :3] *= dim
        self.blit(part, x + sx0, y + sy0, alpha)

    # ---- the silver plates ----
    def plate(self, x0, y0, x1, y1):
        x0, y0, x1, y1 = int(x0), int(y0), int(x1), int(y1)
        h, w = y1 - y0, x1 - x0
        yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
        u, v = xx / w, yy / h
        val = 0.675 + 0.11 * np.exp(-((u - 0.03) / 0.15) ** 2) * (0.25 + 0.75 * v) \
              - 0.17 * np.exp(-((u - 0.30) / 0.13) ** 2) * v ** 1.4 \
              + 0.10 * np.exp(-((u - 0.72) / 0.20) ** 2) * (0.3 + 0.7 * v) \
              - 0.08 * np.exp(-((u - 0.99) / 0.10) ** 2)
        streak = _rng.normal(0, 1, (h, w + 120)).astype(np.float32)         # the brushing: fine lines along it
        k = np.ones(60, np.float32) / 60
        streak = np.apply_along_axis(lambda r: np.convolve(r, k, 'same'), 1, streak)[:, 60:60 + w]
        val = val + 0.020 * streak / max(1e-6, streak.std())
        self.a[y0 - 1:y1 + 1, x0 - 1:x1 + 1] = 0
        self.a[y0:y1, x0:x1] = np.repeat(val[..., None], 3, -1)
        dt, dl, dr, db = yy, xx, (w - 1) - xx, (h - 1) - yy
        lightside = np.minimum(dt, dl) <= np.minimum(dr, db)
        depth = np.minimum(np.minimum(dt, dl), np.minimum(dr, db)).astype(int)
        light = np.array([0.66, 0.98, 0.92], np.float32)
        dark = np.array([0.15, 0.40, 0.58], np.float32)
        for d in range(3):
            m = depth == d
            self.a[y0:y1, x0:x1][m & lightside] = light[d]
            self.a[y0:y1, x0:x1][m & ~lightside] = dark[d]

    def window(self, x0, y0, x1, y1, band=0xa0a2fa, glow=True):
        """A picture window set into a plate: shadowed at the top and left, a white ledge at the bottom and right;
        black inside with the band of light. Returns the inside's box."""
        x0, y0, x1, y1 = int(x0), int(y0), int(x1), int(y1)
        h, w = y1 - y0, x1 - x0
        yy, xx = np.mgrid[0:h, 0:w]
        dt, dl, dr, db = yy, xx, (w - 1) - xx, (h - 1) - yy
        darkside = np.minimum(dt, dl) <= np.minimum(dr, db)
        depth = np.minimum(np.minimum(dt, dl), np.minimum(dr, db))
        reg = self.a[y0:y1, x0:x1]
        reg[:] = 0.004
        dk = np.array([0.45, 0.20, 0.13], np.float32)
        lt = np.array([0.90, 0.99, 0.80], np.float32)
        for d in range(3):
            m = depth == d
            reg[m & darkside] = dk[d]
            reg[m & ~darkside] = lt[d]
        ix0, iy0, ix1, iy1 = x0 + 4, y0 + 4, x1 - 4, y1 - 4
        if glow:
            sl, gx, gy = self.win(ix0, iy0, ix1, iy1)
            t = (gy - iy0) / (iy1 - iy0)
            wv = smooth(0.13, 0.30, t) * (1 - smooth(0.70, 0.87, t))
            wh = smooth(0, 3.5, gx - (ix0 + 4)) * smooth(0, 3.5, (ix1 - 4) - gx)
            col = hx(band)[None, None] * (0.94 + 0.10 * np.cos((t - 0.5) * 6))[..., None]
            self.mix(sl, wv * wh, np.clip(col, 0, 1))
        return ix0, iy0, ix1, iy1

    def hpbar(self, x0, y0, x1, y1, frac):
        x0, y0, x1, y1 = int(x0), int(y0), int(x1), int(y1)
        self.a[y0:y1, x0:x1] = 0.0
        self.a[y0, x0:x1] = 0.30
        self.a[y0:y1, x0] = 0.30
        self.a[y0 + 1, x0 + 1:x1] = 0.11
        self.a[y1 - 1, x0:x1] = 0.95
        self.a[y0:y1, x1 - 1] = 0.90
        self.a[y1 - 2, x0 + 1:x1 - 1] = 0.75
        fx1 = x0 + 2 + int(round((x1 - x0 - 4) * frac))
        rows = y1 - 2 - (y0 + 2)
        if frac > 0 and rows > 0:
            g = ramp(np.linspace(0, 1, rows), [(0, 0xadefef), (0.3, 0xa5f7f9), (0.7, 0x91dee1), (1, 0x6fb0b3)])
            self.a[y0 + 2:y1 - 2, x0 + 2:fx1] = g[:, None, :]

    # ---- the backdrops ----
    def night(self, lines):
        """Black, with STADIUM 2's dotted lines of violet light."""
        self.a[:] = 0.004
        for y, x0, x1 in lines:
            xs = np.arange(max(0, x0), min(W, x1))
            dots = np.where((xs // 2) % 2 == 0, 1.0, 0.42)[:, None]
            fade = np.clip(np.minimum(xs - x0, x1 - xs) / 30.0, 0, 1)[:, None]
            self.a[y, xs] = hx(0x8985ad) * dots * fade
            for dy in (-1, 1):
                self.a[y + dy, xs] = hx(0x3f3a57) * 0.55 * fade

    def sky(self):
        yy, xx = np.mgrid[0:H, 0:W].astype(np.float32)
        t = yy / H
        self.a = ramp(t, [(0, 0x5c84c4), (0.35, 0x6f95cf), (0.75, 0x8fb0de), (1, 0xb4cbe8)]).astype(np.float32)
        n = fbm(W, H, (70, 34, 16, 8))
        cover = smooth(0.50, 0.66, n + 0.10 * np.sin(yy / H * 5.0 + 1.0))
        lit = smooth(0.52, 0.80, fbm(W, H, (40, 18, 9)) * 0.5 + n * 0.5 + (0.5 - t) * 0.10)
        cloud = hx(0xb9c8e2)[None, None] * (1 - lit[..., None]) + hx(0xffffff)[None, None] * lit[..., None]
        self.a = self.a * (1 - 0.92 * cover[..., None]) + cloud * 0.92 * cover[..., None]

    def ribbon(self, y, left=None, right=None):
        """The titles' ribbon: orange between thin violet lines, fading toward the middle; the titles over it."""
        rows = [(0x8761d9, 0.9), (0xc85a7c, 1), (0xfcb018, 1), (0xf3b600, 1), (0xf3b600, 1), (0xf3b502, 1), (0xf5b402, 1),
                (0xf8b302, 1), (0xfab202, 1), (0xfda53c, 1), (0xb14d81, 1), (0x8a4ecd, 0.9)]
        xs = np.arange(W, dtype=np.float32)
        for word, side in ((left, 0), (right, 1)):
            if not word:
                continue
            reach = measure(word, SMALL) * 2 + 70
            d = xs if side == 0 else (W - 1 - xs)
            al = np.clip(1.25 - d / reach, 0, 1) ** 1.5
            for i, (c, a) in enumerate(rows):
                self.a[y + i] = self.a[y + i] * (1 - (al * a)[:, None]) + hx(c) * (al * a)[:, None]
            tx = 5 if side == 0 else 235
            self.text(tx, (y - 7) / 2, word, SMALL, ow=2, outline=hx(0x000000), align='l' if side == 0 else 'r', bold=True)

    # ---- the entry screen ----
    def tag(self, x0, y0, x1, h=11, label='1P'):
        """The 1P tag along a plate's top: a violet bar with a slanted end and a hook down the plate's side."""
        sl, xx, yy = self.win(x0 - 3, y0 - 3, x1 + 4, y0 + h + 22)
        bar = sdf_poly(xx, yy, [(x0, y0), (x1, y0), (x1 - h, y0 + h), (x0, y0 + h)])
        hook = sdf_poly(xx, yy, [(x0, y0 + 2), (x0 + 10, y0 + h - 1), (x0, y0 + h + 14)])
        d = np.minimum(bar, hook)
        self.mix(sl, aa(d - 1), hx(0x1c2c60))
        t = np.clip((yy - y0) / h, 0, 1)
        col = ramp(t, [(0, 0xa4a8fc), (0.25, 0x8d8ff0), (0.8, 0x7878da), (1, 0x5c5cb0)])
        self.mix(sl, aa(d), col)
        f = '/usr/share/fonts/truetype/crosextra/Carlito-BoldItalic.ttf'
        S = 4
        font = ImageFont.truetype(f, 22 * S)
        l, tp, r, b = font.getbbox(label)
        m = Image.new('L', (r - l + 16 * S, b - tp + 16 * S), 0)
        ImageDraw.Draw(m).text((8 * S - l, 8 * S - tp), label, font=font, fill=255)
        face = np.array(m.resize((m.width // S, m.height // S), Image.LANCZOS)).astype(np.float32) / 255
        out = np.clip(pil_blur((face > 0.3).astype(np.float32), 1.3) * 3.0, 0, 1)
        rgba = np.zeros(face.shape + (4,), np.float32)
        rgba[..., :3] = hx(0x101830)
        rgba[..., 3] = out
        rgba[..., :3] = rgba[..., :3] * (1 - face[..., None]) + face[..., None]
        self.blit(rgba, x0 + 4, y0 - 13)

    def key(self, x0, y0, x1, y1, label, selected=False, font=SMALL):
        """A tab key: a bevelled grey key with a white letter; the chosen one larger in a chrome ring."""
        if selected:
            sl, xx, yy, di = self.tube(x0 - 3, y0 - 5, x1 + 3, y1 + 3, r=6, T=4, shadow=2)
            self.mix(sl, aa(di), hx(0x000000))
            t = np.clip((yy - y0) / (y1 - y0), 0, 1)
            self.mix(sl, aa(di + 1), np.repeat((0.36 - 0.20 * t)[..., None], 3, -1))
            cx = (x0 + x1) / 2
            sl, xx, yy = self.win(cx - 8, y1 + 2, cx + 8, y1 + 10)      # the pointer down to the list
            d = sdf_poly(xx, yy, [(cx - 6, y1 + 2), (cx + 6, y1 + 2), (cx, y1 + 8)])
            self.mix(sl, aa(d - 1), hx(0x000306))
            self.mix(sl, aa(d), hx(0xf4f4f4))
            self.text(cx / 2, (y0 + y1) / 4 - 7.5, label, font, ow=1, outline=hx(0x000000), align='c', bold=True)
            return
        c = 4
        pts = [(x0 + c, y0), (x1 - c, y0), (x1, y0 + c), (x1, y1 - c), (x1 - c, y1), (x0 + c, y1), (x0, y1 - c), (x0, y0 + c)]
        sl, xx, yy = self.win(x0 - 2, y0 - 2, x1 + 4, y1 + 4)
        d = sdf_poly(xx, yy, pts)
        self.mix(sl, aa(sdf_poly(xx - 2, yy - 2, pts)) * 0.8, hx(0x000004))
        self.mix(sl, aa(d - 1), hx(0x080808))
        t = np.clip((yy - y0) / (y1 - y0), 0, 1)
        self.mix(sl, aa(d), np.repeat((0.60 - 0.16 * t)[..., None], 3, -1))
        gy, gx = np.gradient(d)
        lit = smooth(-0.3, 0.3, (-gx - gy) / 1.4142)
        edge = aa(d) * (1 - aa(d + 2.2))
        self.mix(sl, edge * lit, hx(0xd8d8d8))
        self.mix(sl, edge * (1 - lit), hx(0x303030))
        self.text((x0 + x1) / 4, (y0 + y1) / 4 - 6.5, label, font, 0xe8e8e8, ow=1, outline=hx(0x0c0c0c), align='c')

    def listpanel(self, x0, y0, x1, y1, track=16):
        """The list: a chrome line round royal blue, the scroll track down its left. Returns the blue's box."""
        sl, xx, yy = self.win(x0 - 2, y0 - 2, x1 + 5, y1 + 5)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, 7)
        self.mix(sl, aa(sdf_rr(xx, yy, x0 + 3, y0 + 3, x1 + 3, y1 + 3, 7)) * 0.9, hx(0x000004))
        self.mix(sl, aa(d), hx(0x10140a))
        gy, gx = np.gradient(d)
        lit = smooth(-0.5, 0.5, (-gx - gy) / 1.4142)
        self.mix(sl, aa(d + 1), np.repeat((0.80 + 0.19 * lit)[..., None], 3, -1))
        self.mix(sl, aa(d + 4), hx(0x3a3a3a))
        inner = aa(d + 5)
        bx0 = x0 + 5 + track
        tr = np.clip((xx - (x0 + 5)) / track, 0, 1)
        tcol = np.repeat((0.80 + 0.18 * np.sin(np.pi * np.clip(tr * 1.25, 0, 1)))[..., None], 3, -1)
        self.mix(sl, inner * (xx < bx0), tcol)
        self.mix(sl, inner * (np.abs(xx - bx0) < 1.0), hx(0x403f64))
        t = np.clip((yy - y0) / (y1 - y0), 0, 1)
        blue = ramp(t, [(0, 0x3b3fee), (0.5, 0x3a3ee8), (1, 0x3035d8)])
        self.mix(sl, inner * (xx >= bx0 + 1), blue)
        return bx0 + 3, y0 + 7, x1 - 7, y1 - 7

    def cell(self, x0, y0, x1, y1, color=0x22269b, alpha=1.0):
        sl, xx, yy = self.win(x0 - 2, y0 - 2, x1 + 2, y1 + 2)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, 5)
        self.mix(sl, np.clip(-d / 2.5 + 0.2, 0, 1) * alpha, hx(color))

    def thumb(self, cx, cy):
        sl, xx, yy = self.win(cx - 9, cy - 12, cx + 9, cy + 12)
        d = sdf_poly(xx, yy, [(cx, cy - 10), (cx + 7, cy - 3), (cx + 7, cy + 3), (cx, cy + 10), (cx - 7, cy + 3), (cx - 7, cy - 3)])
        self.mix(sl, aa(d - 1), hx(YELLOW_INK))
        self.mix(sl, aa(d), hx(0xf2f416))
        self.mix(sl, aa(d) * (np.abs(yy - cy) < 1.6), hx(0x6a6700))
        for s in (-1, 1):
            a = sdf_poly(xx, yy, [(cx, cy + s * 8), (cx + 4 * s, cy + s * 4), (cx - 4 * s, cy + s * 4)][::s])
            self.mix(sl, aa(a), hx(0x726b00))

    def brackets(self, x0, y0, x1, y1, arm=9, th=3, color=YELLOW):
        """The cursor: four yellow corners."""
        for cx, cy, sx, sy in ((x0, y0, 1, 1), (x1, y0, -1, 1), (x0, y1, 1, -1), (x1, y1, -1, -1)):
            for (ax0, ay0, ax1, ay1) in ((cx, cy, cx + sx * arm, cy + sy * th), (cx, cy, cx + sx * th, cy + sy * arm)):
                bx0, bx1 = sorted((ax0, ax1))
                by0, by1 = sorted((ay0, ay1))
                self.a[int(by0) - 1:int(by1) + 2, int(bx0) - 1:int(bx1) + 2] = hx(YELLOW_INK) * 0.5
            for (ax0, ay0, ax1, ay1) in ((cx, cy, cx + sx * arm, cy + sy * th), (cx, cy, cx + sx * th, cy + sy * arm)):
                bx0, bx1 = sorted((ax0, ax1))
                by0, by1 = sorted((ay0, ay1))
                self.a[int(by0):int(by1), int(bx0):int(bx1)] = hx(color)
                self.a[int(by0), int(bx0):int(bx1)] = np.clip(hx(color) * 1.15, 0, 1)

    def typepill(self, x, y, name):
        """A type's name on its colour (units); returns its width."""
        w = measure(name, NARROW) + 6
        self.pill(x * 2, y * 2 + 1, (x + w) * 2, (y + 9) * 2 + 1, TYPE_RGB[name])
        self.text(x + w / 2 + 0.5, y - 1.5, name, NARROW, ow=1, outline=hx(0x181818), align='c')
        return w

    # ---- neon ----
    def neon(self, cx, cy, word, px, color, font, hollow=True, stroke=3.2, glow=5.0, fill=0.40):
        """Glowing lettering (WIN / LOSE hollow with a white line, VS solid white), centred on real pixels."""
        S = 3
        f = ImageFont.truetype(font, int(px * S))
        l, t, r, b = f.getbbox(word)
        pad = int(26 * S)
        m = Image.new('L', (r - l + pad * 2, b - t + pad * 2), 0)
        ImageDraw.Draw(m).text((pad - l, pad - t), word, font=f, fill=255)
        k = int(stroke * S) * 2 + 1
        fat = m.filter(ImageFilter.MaxFilter(int(1.2 * S) * 2 + 1))
        halo = fat.filter(ImageFilter.GaussianBlur(glow * S))
        halo2 = fat.filter(ImageFilter.GaussianBlur(glow * S * 0.30))
        small = lambda im: np.array(im.resize((im.width // S, im.height // S), Image.LANCZOS)).astype(np.float32) / 255
        M = small(m)
        Hl = np.clip(small(halo) * 1.25 + small(halo2) * 0.75, 0, 1.25)
        col = hx(color)
        x, y = int(cx - M.shape[1] / 2), int(cy - M.shape[0] / 2)
        x0, y0, x1, y1 = max(0, x), max(0, y), min(W, x + M.shape[1]), min(H, y + M.shape[0])
        s = (slice(y0 - y, y1 - y), slice(x0 - x, x1 - x))
        reg = self.a[y0:y1, x0:x1]
        if hollow:
            inner = small(m.filter(ImageFilter.MinFilter(k)))
            line = np.clip(M - inner, 0, 1)
            outside = 1 - M[s][..., None]
            reg[:] = np.clip(reg + Hl[s][..., None] * col * 0.85 * (0.35 + 0.65 * outside), 0, 1)     # the glow adds light
            reg[:] = reg * (1 - fill * inner[s][..., None]) + col * fill * inner[s][..., None]         # a wash of colour inside
            reg[:] = reg * (1 - line[s][..., None]) + (0.90 + 0.10 * col) * line[s][..., None]         # the white line
        else:
            reg[:] = np.clip(reg + Hl[s][..., None] * col * 0.95, 0, 1)
            tt = np.linspace(0, 1, M.shape[0])[:, None, None]
            face = 1.0 - (1 - col) * 0.30 * smooth(0.45, 1.0, tt)
            face = np.broadcast_to(face, M.shape + (3,))
            reg[:] = reg * (1 - M[s][..., None]) + face[s] * M[s][..., None]
