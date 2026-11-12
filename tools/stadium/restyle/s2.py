# STADIUM 2's menu materials, measured from the user's screenshot (docs/user_images/77.webp), as drawing pieces for a
# 480x320 picture (the STADIUM screens' canvas: 240x160 units, two pixels a unit).
#   tube / glass   rounded chrome frames around black glass (the name and message boxes, the portraits)
#   slab           the team panels: a bevelled plate with a metallic sheen across it, engraved dividers
#   tab            the side banners with pointed ends
#   sphere         the glossy buttons
#   Text           the game's own fonts (src/fonts.c, graphics/moemon/gFont*.bin), with STADIUM 2's dark outline
import os, re
import numpy as np
from PIL import Image, ImageDraw, ImageFont

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))
HERE = os.path.dirname(os.path.abspath(__file__))
W, H = 480, 320


def hx(c):
    return np.array([(c >> 16) & 255, (c >> 8) & 255, c & 255], np.float32) / 255


def smooth(a, b, x):
    t = np.clip((x - a) / (b - a), 0, 1)
    return t * t * (3 - 2 * t)


def aa(d):
    """Coverage of a shape from its signed distance (negative inside)."""
    return np.clip(0.5 - d, 0, 1)


def sdf_rr(xx, yy, x0, y0, x1, y1, r):
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    qx, qy = np.abs(xx - cx) - ((x1 - x0) / 2 - r), np.abs(yy - cy) - ((y1 - y0) / 2 - r)
    return np.hypot(np.maximum(qx, 0), np.maximum(qy, 0)) + np.minimum(np.maximum(qx, qy), 0) - r


def sdf_poly(xx, yy, pts):
    """A convex polygon (clockwise on screen): the largest distance to its edges."""
    d = np.full(xx.shape, -1e9, np.float32)
    n = len(pts)
    for i in range(n):
        (ax, ay), (bx, by) = pts[i], pts[(i + 1) % n]
        ex, ey = bx - ax, by - ay
        l = np.hypot(ex, ey)
        d = np.maximum(d, ((xx - ax) * ey - (yy - ay) * ex) / l)
    return d


def ramp(u, pts):
    xs = [p for p, _ in pts]
    cols = np.array([hx(c) for _, c in pts])
    return np.stack([np.interp(u, xs, cols[:, k]) for k in range(3)], -1)


# ---- measured from the screenshot ----
# a chrome tube's cross-section from its lit side (up / left) to its far side
TUBE = np.array([0x43, 0xa0, 0xb8, 0xe8, 0xfe, 0xd0, 0xb3, 0x82, 0x65, 0x50, 0x44], np.float32) / 255
# the sheen across a team panel, left to right
SHEEN_BLUE = [(0, 0x7e8ba0), (0.28, 0xcddcfd), (0.42, 0xcddcfd), (0.58, 0xb0bfdc), (0.82, 0x6d788b), (0.90, 0x6d788b), (1, 0xccdafb)]
SHEEN_GREEN = [(0, 0x82a17e), (0.28, 0xd2fcca), (0.42, 0xd3fec9), (0.58, 0xb5d9b1), (0.82, 0x718d6f), (0.90, 0x738972), (1, 0xd1fbcb)]
SHEEN_GOLD = [(0, 0xb89038), (0.28, 0xfff4b8), (0.42, 0xfff4b8), (0.58, 0xf0d478), (0.82, 0xb08428), (0.90, 0xb08428), (1, 0xfff0a8)]
SHEEN_GREY = [(0, 0x7c7e86), (0.28, 0xc4c6ce), (0.42, 0xc4c6ce), (0.58, 0xa8aab2), (0.82, 0x6c6e76), (0.90, 0x6c6e76), (1, 0xc2c4cc)]
PAL = {
    'blue': dict(sheen=SHEEN_BLUE, light=0xcedcf5, dark=0x4f5869, line=0x050b20),
    'green': dict(sheen=SHEEN_GREEN, light=0xd7f7d8, dark=0x4c5c4c, line=0x06180a),
    'gold': dict(sheen=SHEEN_GOLD, light=0xfff8d0, dark=0x705018, line=0x201400),
    'grey': dict(sheen=SHEEN_GREY, light=0xd8dae0, dark=0x4c4e56, line=0x101014),
}
# along a banner, from its root to its tip
TAB_BLUE = [(0, 0x4d525e), (0.30, 0x8791a5), (0.36, 0xb3c1de), (0.80, 0x606775), (0.86, 0x7c8289), (1, 0x9aa0b0)]
TAB_GREEN = [(0, 0x4d5b4b), (0.30, 0x7f987b), (0.36, 0xb4d3b0), (0.80, 0x5c705a), (0.86, 0x7b9477), (1, 0xa6c5a2)]
INK = hx(0x282c38)       # the outline round the writing


# ---------------------------------------------------------------------------------------------------------------
# The game's fonts
def _widths(name):
    src = open(f'{REPO}/src/fonts.c').read()
    body = re.search(name + r'\[\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
    return [int(v, 0) for v in re.findall(r'0x[0-9a-fA-F]+|\d+', re.sub(r'//.*', '', body))]


def _glyphs(path):
    d = np.frombuffer(open(path, 'rb').read(), dtype='<u2').astype(np.int32)
    n = len(d) // 32
    rows = d.reshape(n, 4, 8)                              # glyph, tile, row
    px = np.stack([(rows >> (14 - 2 * k)) & 3 for k in range(8)], -1)    # glyph, tile, row, col
    g = np.zeros((n, 16, 16), np.uint8)
    for t in range(4):
        g[:, (t // 2) * 8:(t // 2) * 8 + 8, (t % 2) * 8:(t % 2) * 8 + 8] = px[:, t]
    return g


def _charmap():
    m = {}
    for name, v in re.findall(r'^#define CHAR_(\w+)\s+0x([0-9A-Fa-f]+)', open(f'{REPO}/include/constants/characters.h').read(), re.M):
        m.setdefault(name, int(v, 16))
    return m


_CH = _charmap()
_PUNCT = {' ': 'SPACE', '!': 'EXCL_MARK', '?': 'QUESTION_MARK', '.': 'PERIOD', '-': 'HYPHEN', ',': 'COMMA', '/': 'SLASH',
          ':': 'COLON', "'": 'SGL_QUOTE_RIGHT', '(': 'LEFT_PAREN', ')': 'RIGHT_PAREN', '+': 'PLUS', '%': 'PERCENT',
          '&': 'AMPERSAND', 'é': 'e_ACUTE'}
NORMAL, SMALL, NARROW = 0, 1, 2
_FONTS = [(_glyphs(f'{REPO}/graphics/moemon/gFontNormalLatinGlyphs.bin'), _widths('gFontNormalLatinGlyphWidths'), 15),
          (_glyphs(f'{REPO}/graphics/moemon/gFontSmallLatinGlyphs.bin'), _widths('gFontSmallLatinGlyphWidths'), 12),
          (_glyphs(f'{HERE}/narrow.latfont'), _widths('gFontSmallNarrowLatinGlyphWidths'), 12)]


def _code(ch):
    if 'A' <= ch <= 'Z':
        return _CH['A'] + ord(ch) - 65
    if 'a' <= ch <= 'z':
        return _CH['a'] + ord(ch) - 97
    if '0' <= ch <= '9':
        return _CH['0'] + ord(ch) - 48
    return _CH[_PUNCT.get(ch, 'QUESTION_MARK')]


def measure(s, font):
    return sum(_FONTS[font][1][_code(c)] for c in s)


def text_masks(s, font):
    g, wd, h = _FONTS[font]
    w = measure(s, font)
    face, shadow = np.zeros((h, w + 1), bool), np.zeros((h, w + 1), bool)
    x = 0
    for c in s:
        k = _code(c)
        cw = wd[k]
        face[:, x:x + cw] |= g[k, :h, :cw] == 1
        shadow[:, x:x + cw] |= g[k, :h, :cw] == 2
        x += cw
    return face, shadow


def dilate(m, r):
    out = np.zeros_like(m)
    p = np.pad(m, r)
    for dy in range(-r, r + 1):
        for dx in range(-r, r + 1):
            if dx * dx + dy * dy <= r * r + 1:
                out |= p[r + dy:r + dy + m.shape[0], r + dx:r + dx + m.shape[1]]
    return out


def text_rgba(s, font, color=0xffffff, outline=INK, shadow=None, ow=2, grad=None, bold=False):
    """A line of the game's font as a picture at real pixels (two a unit), with its outline. Returns it, the
    line's width in units and the picture's margin."""
    face, sh = text_masks(s, font)
    face, sh = np.kron(face, np.ones((2, 2), bool)), np.kron(sh, np.ones((2, 2), bool))
    pad = 3
    face, sh = np.pad(face, pad), np.pad(sh, pad)
    if bold:                 # a pixel heavier (drawn twice, one pixel apart)
        face = face | np.roll(face, 1, 1)
    rgba = np.zeros(face.shape + (4,), np.float32)
    if outline is not None:
        o = dilate(face | sh if shadow is not None else face, ow)
        rgba[o] = np.append(np.asarray(outline, np.float32), 0.92)
    if shadow is not None:
        rgba[sh] = np.append(hx(shadow), 1)
    col = np.broadcast_to(hx(color), face.shape + (3,))
    if grad is not None:     # a soft fall of light down the letters
        t = np.linspace(0, 1, face.shape[0])[:, None, None]
        col = np.broadcast_to(hx(color) * (1 - t) + hx(grad) * t, face.shape + (3,))
    rgba[face, :3] = col[face]
    rgba[face, 3] = 1
    return rgba, measure(s, font), pad


# ---------------------------------------------------------------------------------------------------------------
class Canvas:
    def __init__(self):
        self.a = np.zeros((H, W, 3), np.float32)

    def win(self, x0, y0, x1, y1):
        x0, y0, x1, y1 = max(0, int(np.floor(x0))), max(0, int(np.floor(y0))), min(W, int(np.ceil(x1))), min(H, int(np.ceil(y1)))
        yy, xx = np.mgrid[y0:y1, x0:x1].astype(np.float32) + 0.5
        return (slice(y0, y1), slice(x0, x1)), xx, yy

    def mix(self, sl, mask, color):
        mask = np.clip(mask, 0, 1)[..., None]
        self.a[sl] = self.a[sl] * (1 - mask) + np.asarray(color, np.float32) * mask

    def blit(self, rgba, x, y, alpha=1.0):
        """An RGBA picture (floats 0..1) at real pixels."""
        x, y = int(round(x)), int(round(y))
        h, w = rgba.shape[:2]
        x0, y0, x1, y1 = max(0, x), max(0, y), min(W, x + w), min(H, y + h)
        if x1 <= x0 or y1 <= y0:
            return
        src = rgba[y0 - y:y1 - y, x0 - x:x1 - x]
        m = src[..., 3:4] * alpha
        self.a[y0:y1, x0:x1] = self.a[y0:y1, x0:x1] * (1 - m) + src[..., :3] * m

    def image(self):
        return Image.fromarray((np.clip(self.a, 0, 1) * 255 + 0.5).astype(np.uint8))

    # ---- writing: units (240x160), the game's fonts doubled as the screens draw them ----
    def text(self, x, y, s, font=NORMAL, color=0xffffff, outline=INK, shadow=None, align='l', ow=2, grad=None, bold=False):
        rgba, w, pad = text_rgba(s, font, color, outline, shadow, ow, grad, bold)
        px = int(round((x - (w if align == 'r' else w / 2 if align == 'c' else 0)) * 2))
        self.blit(rgba, px - pad, int(round(y * 2)) - pad)
        return w

    def text_rot(self, cx, cy, s, font, color, angle, outline=None, ow=1, lean=0.0, zoom=1.0):
        """Writing turned on its side (the banners): centred on real pixels (cx, cy)."""
        rgba, w, pad = text_rgba(s, font, color, outline, None, ow, None)
        im = Image.fromarray((rgba * 255).astype(np.uint8), 'RGBA')
        if zoom != 1.0:
            im = im.resize((int(im.width * zoom), int(im.height * zoom)), Image.NEAREST)
        if lean:
            im = im.transform((im.width + int(abs(lean) * im.height), im.height), Image.AFFINE,
                              (1, lean, -lean * im.height if lean > 0 else 0, 0, 1, 0), Image.BILINEAR)
        im = im.rotate(angle, expand=True, resample=Image.BILINEAR)
        a = np.array(im).astype(np.float32) / 255
        self.blit(a, cx - a.shape[1] / 2, cy - a.shape[0] / 2)

    def label_rot(self, cx, cy, word, px, color, angle, font='/usr/share/fonts/truetype/crosextra/Carlito-BoldItalic.ttf'):
        S = 4
        f = ImageFont.truetype(font, px * S)
        l, t, r, b = f.getbbox(word)
        m = Image.new('L', (r - l + 8 * S, b - t + 8 * S), 0)
        ImageDraw.Draw(m).text((4 * S - l, 4 * S - t), word, font=f, fill=255)
        m = m.rotate(angle, expand=True, resample=Image.BICUBIC)
        m = np.array(m.resize((m.width // S, m.height // S), Image.LANCZOS)).astype(np.float32) / 255
        rgba = np.concatenate([np.broadcast_to(hx(color), m.shape + (3,)), m[..., None]], -1)
        self.blit(rgba, cx - m.shape[1] / 2, cy - m.shape[0] / 2)

    # ---- chrome ----
    def tube(self, x0, y0, x1, y1, r=6, T=6, shadow=3, tint=None, outline=True):
        """A chrome tube round a rounded box (real pixels). Returns the window and the distance inside the tube."""
        sl, xx, yy = self.win(x0 - 2, y0 - 2, x1 + shadow + 2, y1 + shadow + 2)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, r)
        if shadow:
            self.mix(sl, aa(sdf_rr(xx, yy, x0 + shadow, y0 + shadow, x1 + shadow, y1 + shadow, r)) * 0.93, hx(0x000004))
        o = 1 if outline else 0
        if outline:
            self.mix(sl, aa(d), hx(0x000306))
        t = np.clip((-d - o) / T, 0, 1)
        gy, gx = np.gradient(d)
        lit = smooth(-0.45, 0.45, (-gx - gy) / 1.4142)      # 1 on the edges facing up / left
        s = lit * t + (1 - lit) * (1 - t)
        v = np.interp(s, np.linspace(0, 1, len(TUBE)), TUBE)
        col = np.repeat(v[..., None], 3, -1)
        if tint is not None:
            col = col * np.asarray(tint) + (col ** 6) * (1 - np.asarray(tint))
        self.mix(sl, aa(d + o) * (1 - aa(d + o + T)), col)
        return sl, xx, yy, d + o + T

    def glass(self, x0, y0, x1, y1, r=6, T=6, shadow=3):
        """A black glass box in a chrome tube: black at the top, grey at the foot, a light lip along the bottom."""
        sl, xx, yy, di = self.tube(x0, y0, x1, y1, r, T, shadow)
        self.mix(sl, aa(di), hx(0x010101))
        iy0, iy1 = y0 + T + 2, y1 - T - 2
        t = np.clip((yy - iy0) / max(1, iy1 - iy0), 0, 1)
        v = 0x09 / 255 + (0x63 - 0x09) / 255 * t
        self.mix(sl, aa(di + 1), np.repeat(v[..., None], 3, -1))
        gy, gx = np.gradient(di)
        lip = aa(di + 1) * (1 - aa(di + 2.6)) * smooth(0.75, 0.95, gy)
        self.mix(sl, lip * 0.8, hx(0x9a9a9e))

    def frame(self, x0, y0, x1, y1, T=5, gap=2, T2=3, shadow=3):
        """The portraits' frame: a tube, a black gap, a thinner tube, black inside. Returns the inner box."""
        sl, xx, yy, di = self.tube(x0, y0, x1, y1, 5, T, shadow)
        self.mix(sl, aa(di), hx(0x000000))
        k = 1 + T + gap
        sl, xx, yy, di = self.tube(x0 + k, y0 + k, x1 - k, y1 - k, 2, T2, 0, outline=False)
        self.mix(sl, aa(di), hx(0x000000))
        k += T2 + 1
        return x0 + k, y0 + k, x1 - k, y1 - k

    def ring(self, x0, y0, x1, y1, r=5, T=4, tint=(1.0, 0.84, 0.30)):
        """The cursor: a gold chrome ring with nothing inside."""
        sl, xx, yy = self.win(x0 - 3, y0 - 3, x1 + 3, y1 + 3)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, r)
        band = aa(d) * (1 - aa(d + T + 2))
        self.mix(sl, band, hx(0x100c04))
        t = np.clip((-d - 1) / T, 0, 1)
        gy, gx = np.gradient(d)
        lit = smooth(-0.45, 0.45, (-gx - gy) / 1.4142)
        s = lit * t + (1 - lit) * (1 - t)
        v = np.interp(s, np.linspace(0, 1, len(TUBE)), TUBE)[..., None]
        col = v * np.asarray(tint, np.float32) + (v ** 5) * (1 - np.asarray(tint, np.float32))
        self.mix(sl, aa(d + 1) * (1 - aa(d + 1 + T)), col)

    def strip(self, y0=293):
        """The foot of the screen: a channel of black glass under a chrome rail (the banners rise from behind it)."""
        self.a[y0 - 6, :] = hx(0x000306)
        for k in range(4):
            v = np.interp(k / 3, np.linspace(0, 1, len(TUBE)), TUBE)
            self.a[y0 - 5 + k, :] = v
        self.a[y0 - 1, :] = hx(0x000000)
        t = np.linspace(0, 1, H - y0)[:, None, None]
        self.a[y0:, :] = (0x0b + (0x4a - 0x0b) * t) / 255

    # ---- the team panels ----
    def slab(self, x0, y0, x1, y1, pal='blue', B=3, shadow=3):
        """A bevelled plate (real pixels, whole numbers): black line, light top / left, dark right / bottom, the
        sheen across. Returns the surface's box."""
        p = PAL[pal]
        x0, y0, x1, y1 = int(x0), int(y0), int(x1), int(y1)
        self.a[y0 + shadow:y1 + shadow, x0 + shadow:x1 + shadow] *= 0.04
        self.a[y0:y1, x0:x1] = hx(0x000714)
        yy, xx = np.mgrid[y0 + 1:y1 - 1, x0 + 1:x1 - 1]
        dt, dl, dr, db = yy - (y0 + 1), xx - (x0 + 1), (x1 - 2) - xx, (y1 - 2) - yy
        lightside = np.minimum(dt, dl) <= np.minimum(dr, db)
        depth = np.minimum(np.minimum(dt, dl), np.minimum(dr, db))
        light = hx(p['light'])[None, None] * np.where(depth == 0, 0.86, 1.0)[..., None]
        dark = hx(p['dark'])[None, None] * np.where(depth == B - 1, 1.12, 1.0)[..., None]
        self.a[y0 + 1:y1 - 1, x0 + 1:x1 - 1] = np.where(lightside[..., None], light, dark)
        sx0, sy0, sx1, sy1 = x0 + 1 + B, y0 + 1 + B, x1 - 1 - B, y1 - 1 - B
        self.sheen(sx0, sy0, sx1, sy1, pal, (sx0, sx1))
        return sx0, sy0, sx1, sy1

    def sheen(self, x0, y0, x1, y1, pal, span):
        """The metal's sheen over a box, as part of a plate spanning `span` (left, right)."""
        u = (np.arange(int(x0), int(x1)) + 0.5 - span[0]) / max(1, span[1] - span[0])
        row = ramp(u, PAL[pal]['sheen'])
        h = int(y1) - int(y0)
        v = 1 + 0.05 * (0.5 - np.linspace(0, 1, h))[:, None, None]       # a breath lighter at the top
        self.a[int(y0):int(y1), int(x0):int(x1)] = np.clip(row[None] * v, 0, 1)

    def engrave_v(self, x, y0, y1, pal='blue'):
        self.a[int(y0):int(y1), int(x)] = hx(PAL[pal]['line'])
        self.a[int(y0):int(y1), int(x) + 1] = hx(0xf8fdff)

    def engrave_h(self, y, x0, x1, pal='blue'):
        self.a[int(y), int(x0):int(x1)] = hx(PAL[pal]['line'])
        self.a[int(y) + 1, int(x0):int(x1)] = hx(0xf8fdff)

    # ---- the side banners ----
    def tab(self, x0, x1, root, tip, grad, shadow=3):
        """A banner from `root` (off the screen's edge) to its pointed `tip` (real pixels)."""
        down = tip > root
        k = (x1 - x0) / 2
        sh = tip - k if down else tip + k                         # where the point begins
        pts = [(x0, root), (x1, root), (x1, sh), ((x0 + x1) / 2, tip), (x0, sh)] if down else \
              [(x0, sh), ((x0 + x1) / 2, tip), (x1, sh), (x1, root), (x0, root)]
        sl, xx, yy = self.win(x0 - 2, min(root, tip) - 2, x1 + shadow + 2, max(root, tip) + shadow + 2)
        d = sdf_poly(xx, yy, pts)
        self.mix(sl, aa(sdf_poly(xx - shadow, yy - shadow, pts)) * 0.93, hx(0x000004))
        self.mix(sl, aa(d), hx(0x01050d))
        u = np.clip((yy - root) / (tip - root), 0, 1)
        body = ramp(u, grad)
        point = (yy > sh) if down else (yy < sh)                   # the point is a darker facet
        body = np.where(point[..., None], body * 0.62, body)
        self.mix(sl, aa(d + 1), body)
        inside = aa(d + 1)
        left = inside * (xx < x0 + 3.5) * (~point)                 # a light edge down the left, a dark one down the right
        right = inside * (xx > x1 - 3.5) * (~point)
        self.mix(sl, left, np.clip(body * 1.5 + 0.18, 0, 1))
        self.mix(sl, right, body * 0.55)
        fold = inside * (np.abs(yy - sh) < 1.0)                    # the fold's bright line
        self.mix(sl, fold * 0.8, hx(0xe8eef8))

    # ---- glossy things ----
    def sphere(self, cx, cy, r, color, outline=0x000000, flat=False):
        sl, xx, yy = self.win(cx - r - 2, cy - r - 2, cx + r + 3, cy + r + 3)
        nx, ny = (xx - cx) / r, (yy - cy) / r
        rr = np.hypot(nx, ny)
        nz = np.sqrt(np.clip(1 - rr * rr, 0, 1))
        diff = np.clip(-0.48 * nx - 0.60 * ny + 0.64 * nz, 0, 1)
        col = hx(color) if isinstance(color, int) else np.asarray(color, np.float32)
        base = col[None, None] * (0.42 + 0.72 * diff)[..., None]
        spec = np.exp(-((nx + 0.36) ** 2 + (ny + 0.42) ** 2) / (0.10 if flat else 0.045 + 0.45 / r))
        base = col[None, None] * ((0.50 if flat else 0.62) + 0.62 * diff)[..., None] + (0.22 if flat else 0.8) * spec[..., None]
        base = base * (0.70 + 0.30 * np.sqrt(nz))[..., None]
        if outline is not None:
            self.mix(sl, aa((rr - 1) * r - 1), hx(outline))
        self.mix(sl, aa((rr - 1) * r), np.clip(base, 0, 1))

    def ball(self, cx, cy, r, top=0x58c838, lit=True):
        """A small POKé BALL token."""
        sl, xx, yy = self.win(cx - r - 2, cy - r - 2, cx + r + 3, cy + r + 3)
        nx, ny = (xx - cx) / r, (yy - cy) / r
        rr = np.hypot(nx, ny)
        nz = np.sqrt(np.clip(1 - rr * rr, 0, 1))
        diff = np.clip(-0.48 * nx - 0.60 * ny + 0.64 * nz, 0, 1)
        col = np.where((ny < 0)[..., None], hx(top if lit else 0x8a8e96), hx(0xf0f0f4 if lit else 0xb4b6bc))
        col = col * (0.50 + 0.66 * diff)[..., None] + 0.6 * np.exp(-((nx + 0.36) ** 2 + (ny + 0.45) ** 2) / 0.05)[..., None]
        self.mix(sl, aa((rr - 1) * r - 1), hx(0x000000))
        self.mix(sl, aa((rr - 1) * r), np.clip(col, 0, 1))
        self.mix(sl, aa((rr - 1) * r) * (np.abs(ny) < 0.14), hx(0x101014))
        self.mix(sl, aa((rr - 0.40) * r), hx(0x101014))
        self.mix(sl, aa((rr - 0.24) * r), hx(0xffffff if lit else 0xc8c8cc))

    def pill(self, x0, y0, x1, y1, color):
        """A glossy capsule (the rule chips): a dark line, a silver rim, the colour with a band of light on top."""
        r = (y1 - y0) / 2
        sl, xx, yy = self.win(x0 - 3, y0 - 3, x1 + 5, y1 + 5)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, r)
        self.mix(sl, aa(sdf_rr(xx, yy, x0 + 2, y0 + 2, x1 + 2, y1 + 2, r)) * 0.8, hx(0x000004))
        self.mix(sl, aa(d), hx(0x06080c))
        t = np.clip((yy - y0) / (y1 - y0), 0, 1)
        self.mix(sl, aa(d + 1), np.repeat((0.98 - 0.45 * t)[..., None], 3, -1))        # the rim
        col = hx(color)[None, None] * (1.12 - 0.45 * t)[..., None]
        self.mix(sl, aa(d + 2.5), np.clip(col, 0, 1))
        gloss = aa(sdf_rr(xx, yy, x0 + 5, y0 + 3.5, x1 - 5, y0 + r + 0.5, r / 2 - 1))
        self.mix(sl, gloss * 0.42, hx(0xffffff))


# ---------------------------------------------------------------------------------------------------------------
# Pictures from the game
def indexed(path, box=None):
    im = Image.open(path)
    idx = np.array(im)
    rgba = np.array(im.convert('RGBA')).astype(np.float32) / 255
    rgba[idx == 0, 3] = 0
    if box:
        rgba = rgba[box[1]:box[3], box[0]:box[2]]
    return rgba


def scale(rgba, w, h, how=Image.NEAREST):
    im = Image.fromarray((np.clip(rgba, 0, 1) * 255).astype(np.uint8), 'RGBA')
    if how != Image.NEAREST:          # premultiplied, or the edges pick up the hidden colour
        a = np.array(im).astype(np.float32)
        a[..., :3] *= a[..., 3:4] / 255
        im = Image.fromarray(a.astype(np.uint8), 'RGBA').resize((int(w), int(h)), how)
        a = np.array(im).astype(np.float32)
        a[..., :3] = np.where(a[..., 3:4] > 0, a[..., :3] * 255 / np.maximum(a[..., 3:4], 1), 0)
        return np.clip(a / 255, 0, 1)
    return np.array(im.resize((int(w), int(h)), how)).astype(np.float32) / 255


def icon(sp):
    return indexed(f'{REPO}/graphics/pokemon/{sp}/icon.png', (0, 0, 32, 32))


def front(sp):
    return indexed(f'{REPO}/graphics/pokemon/{sp}/front.png')


def trainer(name):
    return indexed(f'{REPO}/graphics/trainers/front_pics/{name}.png')


def stadium_pic(n):
    return np.array(Image.open(f'{HERE}/pics/pic{n:02d}.png').convert('RGBA')).astype(np.float32) / 255


_KEYS = {'A': (0x00, 8), 'B': (0x01, 8), 'L': (0x02, 16), 'R': (0x04, 16), 'START': (0x06, 24), 'SELECT': (0x09, 24)}


def keycap(name):
    """The game's own keypad icons (graphics/fonts/keypad_icons.png) as silver keys with dark letters."""
    idx = np.array(Image.open(f'{REPO}/graphics/fonts/keypad_icons.png'))
    tile, w = _KEYS[name]
    g = idx[(tile // 16) * 8:(tile // 16) * 8 + 12, (tile % 16) * 8:(tile % 16) * 8 + w]
    rgba = np.zeros(g.shape + (4,), np.float32)
    for v, c in ((1, 0xf4f6fa), (2, 0x20222a), (3, 0x9ca0ac), (4, 0x20222a)):
        rgba[g == v] = np.append(hx(c), 1)
    return np.kron(rgba, np.ones((2, 2, 1), np.float32))


def blur(a, r):
    n = int(r * 3) + 1
    k = np.exp(-np.arange(-n, n + 1) ** 2 / (2.0 * r * r))
    k /= k.sum()
    a = np.pad(a.astype(np.float32), n, mode='edge')
    a = np.apply_along_axis(lambda v: np.convolve(v, k, 'same'), 1, a)
    a = np.apply_along_axis(lambda v: np.convolve(v, k, 'same'), 0, a)
    return a[n:-n, n:-n]


# ---------------------------------------------------------------------------------------------------------------
# The background: a band of light across pale silver, grey at the top and the foot; chrome fittings; a POKéMON's
# picture faded to white with soft lines, as STADIUM 2's menus have behind them
def background(c, art=None, art_at=(250, 20), art_size=300, flip=False):
    yy, xx = np.mgrid[0:H, 0:W].astype(np.float32) + 0.5
    base = np.empty((H, W, 3), np.float32)
    base[:] = hx(0xe6e9f2)
    base += (0.035 * np.exp(-((xx - 300) ** 2 + (yy - 150) ** 2) / 5.0e4))[..., None]

    def rail(x0, y0, x1, y1, r=8):
        d = sdf_rr(xx, yy, x0, y0, x1, y1, r)
        gy, gx = np.gradient(d)
        t = np.clip(-d / 9, 0, 1)                      # across the rail's half width
        lit = smooth(-0.5, 0.5, (-gx - gy) / 1.4142)
        v = 0.80 + 0.14 * np.sin(np.pi * np.clip(t, 0, 1)) * (lit * 2 - 1) - 0.10 * (1 - lit) * (t < 0.25)
        m = aa(d)
        base[:] = base * (1 - m[..., None]) + (np.stack([v, v, v * 1.03], -1)) * m[..., None]
        edge = aa(d) * (1 - aa(d + 1.2))
        base[:] = base * (1 - 0.35 * edge[..., None]) + hx(0x8890a4) * 0.35 * edge[..., None]

    # the fittings: two rails crossing behind, a great POKé BALL where they meet, arrows at the sides
    rail(-20, 34, 500, 52)
    rail(388, -20, 406, 340)
    rail(-20, 268, 500, 286)
    rail(70, -20, 88, 340)

    def emblem(cx, cy, r):
        nx, ny = (xx - cx) / r, (yy - cy) / r
        rr = np.hypot(nx, ny)
        nz = np.sqrt(np.clip(1 - rr * rr, 0, 1))
        diff = np.clip(-0.45 * nx - 0.62 * ny + 0.64 * nz, 0, 1)
        v = 0.56 + 0.40 * diff
        band = np.exp(-(ny / 0.07) ** 2) * (rr > 0.34)
        v = v - 0.16 * band + 0.10 * np.exp(-((ny - 0.11) / 0.04) ** 2) * (rr > 0.34)
        ringd = np.abs(rr - 0.30)
        v = v - 0.16 * np.exp(-(ringd / 0.05) ** 2) + 0.12 * np.exp(-((rr - 0.17) / 0.06) ** 2)
        m = aa((rr - 1) * r)
        base[:] = base * (1 - m[..., None]) + np.stack([v, v, v * 1.04], -1) * m[..., None]
        edge = aa((rr - 1) * r) * (1 - aa((rr - 1) * r + 1.5))
        base[:] = base * (1 - 0.4 * edge[..., None]) + hx(0x8088a0) * 0.4 * edge[..., None]

    emblem(397, 43, 34)
    emblem(79, 277, 26)
    for cx, sgn in ((18, -1), (462, 1)):          # outlined arrows at the sides
        d1 = np.abs((xx - cx) * sgn + np.abs(yy - 160) * 0.8 - 14) - 3.5
        m = aa(d1) * (np.abs(yy - 160) < 34)
        base[:] = base * (1 - 0.55 * m[..., None]) + hx(0xfafcff) * 0.55 * m[..., None]
        e = (aa(d1 - 1.2) - aa(d1)) * (np.abs(yy - 160) < 35)
        base[:] = base * (1 - 0.3 * e[..., None]) + hx(0x8890a8) * 0.3 * e[..., None]

    if art is not None:
        src = art[:, ::-1] if flip else art
        k = art_size
        big = scale(src, k, k, Image.BICUBIC)
        sil = smooth(0.35, 0.65, blur(big[..., 3], k / 64 * 0.9))
        lum = (big[..., :3] * [0.30, 0.59, 0.11]).sum(-1)
        darkline = smooth(0.30, 0.75, blur(np.where((big[..., 3] > 0.5) & (lum < 0.26), 1.0, 0.0), k / 64 * 0.40)) * sil
        gy, gx = np.gradient(sil)
        edge = np.clip(np.hypot(gx, gy) * 5, 0, 1)
        shade = blur(np.where(big[..., 3] > 0.5, lum, 1), k / 64 * 1.2)
        x, y = art_at
        x0, y0, x1, y1 = max(0, x), max(0, y), min(W, x + k), min(H, y + k)
        s = (slice(y0 - y, y1 - y), slice(x0 - x, x1 - x))
        reg = base[y0:y1, x0:x1]
        fill = hx(0xf9fbff)[None, None] * (0.93 + 0.07 * shade[s])[..., None]
        reg[:] = reg * (1 - 0.86 * sil[s][..., None]) + fill * 0.86 * sil[s][..., None]
        ln = np.clip(0.50 * darkline[s] + 0.62 * edge[s], 0, 0.72)[..., None]
        reg[:] = reg * (1 - ln) + hx(0x8590b2) * ln

    # the band of light: grey above and below it
    t = yy / H
    band = smooth(0.015, 0.135, t) * (1 - smooth(0.815, 0.935, t))
    grey = hx(0x83858f)
    dim = base * 0.60 * (grey / grey.mean())
    c.a = base * band[..., None] + dim * (1 - band[..., None])


# ---------------------------------------------------------------------------------------------------------------
# Chrome lettering (the VS): a real font drawn large, then the tube's light across it
def chrome_word(word, px, font='/usr/share/fonts/truetype/crosextra/Carlito-BoldItalic.ttf'):
    S = 4
    f = ImageFont.truetype(font, px * S)
    l, t, r, b = f.getbbox(word)
    pad = 10 * S
    m = Image.new('L', (r - l + pad * 2, b - t + pad * 2), 0)
    ImageDraw.Draw(m).text((pad - l, pad - t), word, font=f, fill=255)
    m = np.array(m).astype(np.float32) / 255
    hgt = blur(m, 2.2 * S)                                     # a rounded height from the letters' shape
    gy, gx = np.gradient(hgt)
    lit = np.clip(0.5 + (-gx - gy) * 9 * S / 4, 0, 1)
    yy = (np.arange(m.shape[0])[:, None] - pad) / (b - t) * np.ones((1, m.shape[1]))
    sky = ramp(np.clip(yy, 0, 1), [(0, 0xffffff), (0.18, 0xdfe8fb), (0.50, 0x8ea0c8), (0.52, 0x2c3448), (0.75, 0x5c6884), (1, 0xd8e0f0)])
    col = np.clip(sky * (0.55 + 0.75 * lit[..., None]), 0, 1)
    out3 = blur(m, 3.0 * S) > 0.08                              # the black line round it
    rgba = np.zeros(m.shape + (4,), np.float32)
    rgba[out3] = [0.0, 0.01, 0.03, 1]
    rgba[..., :3] = rgba[..., :3] * (1 - m[..., None]) + col * m[..., None]
    rgba[..., 3] = np.maximum(rgba[..., 3], m)
    sh = np.zeros_like(rgba)                                   # the hard shadow, down and right
    sh[3 * S:, 3 * S:, 3] = rgba[:-3 * S, :-3 * S, 3] * 0.93
    sh[..., 3] = np.maximum(sh[..., 3] * (1 - rgba[..., 3]), 0)
    sh[..., :3] = 0
    a = rgba[..., 3:4] + sh[..., 3:4] * (1 - rgba[..., 3:4])
    comb = np.concatenate([rgba[..., :3] * rgba[..., 3:4] / np.maximum(a, 1e-6), a], -1)
    return scale(comb, comb.shape[1] // S, comb.shape[0] // S, Image.LANCZOS)
