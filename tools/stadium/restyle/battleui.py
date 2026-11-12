# POKESTADIUM / POKESTADIUM 2 THEME on the battle HUD. Unlike the STADIUM menus (their own 480x320 canvas), a battle
# overlay goes over the GBA frame (mp_tricards.c MP_TriCards_Composite), so everything here is drawn at 240x160 in
# the game's own fonts at 1x: what the game could really put on screen. Saved at 4x to look at.
import os, sys, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import numpy as np
from PIL import Image
from s2 import ramp, hx, aa, smooth, sdf_rr, ramp, text_masks, measure, dilate, SMALL, NARROW, NORMAL, scale, front, icon, trainer
from s3 import TYPE_RGB

OUT = os.path.dirname(os.path.abspath(__file__))
REPO = '/home/user/PokemonEmeraldKai'
W, H = 240, 160


def battle_bg(name='building', pal='palette'):
    """The game's own battle platforms (4bpp tiles + tilemap + palette) over a plain arena: map.bin holds only the
    platforms, so sky, crowd and field here are a stand-in, not the game's art."""
    tiles = np.array(Image.open(f'{REPO}/graphics/battle_environment/{name}/tiles.png'))
    cols = tiles.shape[1] // 8
    pl = [l.split() for l in open(f'{REPO}/graphics/battle_environment/{name}/{pal}.pal').read().splitlines()[3:]]
    colors = np.array([[int(v) for v in c] for c in pl if len(c) == 3], np.float32) / 255
    m = np.frombuffer(open(f'{REPO}/graphics/battle_environment/{name}/map.bin', 'rb').read(), '<u2')
    rgb = np.zeros((H, W, 3), np.float32)
    alpha = np.zeros((H, W), np.float32)
    for ty in range(H // 8):
        for tx in range(W // 8):
            e = m[ty * 64 + tx]
            ti, pr = e & 0x3FF, (e >> 12) & 15
            blk = tiles[(ti // cols) * 8:(ti // cols) * 8 + 8, (ti % cols) * 8:(ti % cols) * 8 + 8]
            if e & 0x400:
                blk = blk[:, ::-1]
            if e & 0x800:
                blk = blk[::-1]
            rgb[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8] = colors[pr * 16 + blk]
            alpha[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8] = (blk != 0)
    yy = np.arange(H)[:, None] * np.ones((1, W))
    sky = ramp(np.clip(yy / 70, 0, 1), [(0, 0x5898d8), (1, 0x90c0e8)])
    crowd = ramp(np.clip((yy - 54) / 26, 0, 1), [(0, 0x486890), (1, 0x30506c)])
    field = ramp(np.clip((yy - 80) / 80, 0, 1), [(0, 0x58a048), (1, 0x3c7c34)])
    back = np.where((yy < 54)[..., None], sky, np.where((yy < 80)[..., None], crowd, field))
    speck = ((np.arange(W)[None] * 7 + np.arange(H)[:, None] * 13) % 11 == 0) & (yy >= 54) & (yy < 80)
    back = np.where(speck[..., None], np.clip(back * 1.5, 0, 1), back)
    return back * (1 - alpha[..., None]) + rgb * alpha[..., None]


class B:
    """A 240x160 frame, drawn at 1x with the game's fonts."""
    def __init__(self, bg):
        self.a = bg.copy()

    def win(self, x0, y0, x1, y1):
        x0, y0 = max(0, int(np.floor(x0))), max(0, int(np.floor(y0)))
        x1, y1 = min(W, int(np.ceil(x1))), min(H, int(np.ceil(y1)))
        yy, xx = np.mgrid[y0:y1, x0:x1].astype(np.float32) + 0.5
        return (slice(y0, y1), slice(x0, x1)), xx, yy

    def mix(self, sl, m, col):
        m = np.clip(m, 0, 1)[..., None]
        self.a[sl] = self.a[sl] * (1 - m) + np.asarray(col, np.float32) * m

    def blit(self, rgba, x, y, alpha=1.0):
        x, y = int(round(x)), int(round(y))
        h, w = rgba.shape[:2]
        x0, y0, x1, y1 = max(0, x), max(0, y), min(W, x + w), min(H, y + h)
        if x1 <= x0 or y1 <= y0:
            return
        s = rgba[y0 - y:y1 - y, x0 - x:x1 - x]
        m = s[..., 3:4] * alpha
        self.a[y0:y1, x0:x1] = self.a[y0:y1, x0:x1] * (1 - m) + s[..., :3] * m

    def text(self, x, y, s, font=SMALL, col=0xffffff, ink=0x101018, align='l', outline=True):
        face, sh = text_masks(s, font)
        w = measure(s, font)
        px = int(round(x - (w if align == 'r' else w / 2 if align == 'c' else 0)))
        pad = 2
        f = np.pad(face, pad)
        rgba = np.zeros(f.shape + (4,), np.float32)
        if outline:
            rgba[dilate(f, 1)] = np.append(hx(ink), 1)
        rgba[f] = np.append(hx(col), 1)
        self.blit(rgba, px - pad, int(round(y)) - pad)
        return w

    # ---- POKESTADIUM 2: a chrome tube round black glass ----
    def s2_panel(self, x0, y0, x1, y1, tint=(1.0, 1.0, 1.0), glass=(0x0a0a0c, 0x4a4a50)):
        sl, xx, yy = self.win(x0 - 1, y0 - 1, x1 + 2, y1 + 2)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, 3)
        self.mix(sl, aa(d), hx(0x000306))
        gy, gx = np.gradient(d)
        lit = smooth(-0.5, 0.5, (-gx - gy) / 1.4142)
        band = aa(d + 1) * (1 - aa(d + 3))
        v = (0.95 * lit + 0.30 * (1 - lit))[..., None] * np.asarray(tint, np.float32)
        self.mix(sl, band, np.clip(v, 0, 1))
        t = np.clip((yy - y0) / max(1, y1 - y0), 0, 1)
        self.mix(sl, aa(d + 3), ramp(t, [(0, glass[0]), (1, glass[1])]))

    # ---- POKESTADIUM: a navy box in a pale line ----
    def p1_panel(self, x0, y0, x1, y1, fill=0x20209c, edge=0xd8d8ff):
        sl, xx, yy = self.win(x0 - 1, y0 - 1, x1 + 2, y1 + 2)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, 2)
        self.mix(sl, aa(d), hx(0x080818))
        self.mix(sl, aa(d + 1), hx(edge))
        self.mix(sl, aa(d + 2), hx(fill))

    def hpbar(self, x0, y, w, frac, theme):
        h = 5 if theme == 's2' else 4
        self.a[y:y + h, x0:x0 + w] = hx(0x101014)
        self.a[y + 1:y + h - 1, x0 + 1:x0 + w - 1] = hx(0x2c2c34)
        col = 0x58d048 if frac > 0.5 else 0xf8c030 if frac > 0.2 else 0xf05038
        n = int(round((w - 2) * frac))
        if n > 0:
            self.a[y + 1:y + h - 1, x0 + 1:x0 + 1 + n] = hx(col)
            self.a[y + 1, x0 + 1:x0 + 1 + n] = np.clip(hx(col) * 1.35, 0, 1)

    def dots(self, x, y, left, total=3, theme='s2'):
        for i in range(total):
            cx = x + i * 7
            sl, xx, yy = self.win(cx - 3, y - 3, cx + 4, y + 4)
            r = np.hypot(xx - cx, yy - cy_(y))
            self.mix(sl, aa(r - 3), hx(0x101018))
            self.mix(sl, aa(r - 2.2), hx(0xf04030 if i < left else 0x585c68))
            self.mix(sl, aa(r - 1.0) * (yy < y), hx(0xff9080 if i < left else 0x80848c))

    def portrait(self, x, y, sp, theme, size=30):
        if theme == 's2':
            self.s2_panel(x, y, x + size, y + size, glass=(0x050505, 0x151518))
        else:
            self.p1_panel(x, y, x + size, y + size, fill=0x282890)
        pic = scale(front(sp), 64, 64)
        rows = np.nonzero(pic[..., 3].max(1) > 0.5)[0]
        cols = np.nonzero(pic[..., 3].max(0) > 0.5)[0]
        cut = pic[rows[0]:rows[0] + 48, cols[0]:cols[-1] + 1]
        k = min((size - 6) / cut.shape[1], (size - 6) / cut.shape[0])
        sm = scale(cut, max(1, int(cut.shape[1] * k)), max(1, int(cut.shape[0] * k)), Image.BILINEAR)
        self.blit(sm, x + (size - sm.shape[1]) / 2, y + (size - sm.shape[0]) / 2)


def cy_(y):
    return y


# ---------------------------------------------------------------------------------------------------------------
FOE = dict(name='MANKEY', lv=5, hp=7, max=20, cond='Nm', gender='♀', sp='mankey')
MINE = dict(name='SQUIRTLE', lv=5, hp=19, max=19, cond='Nm', gender='♂', sp='squirtle')
MOVES = [('FLAMETHROWER', 'FIRE', 15, 15), ('RAGE', 'NORMAL', 20, 20), ('GROWL', 'NORMAL', 40, 40), ('SUNNY DAY', 'FIRE', 5, 5)]


def gender_mark(b, x, y, male, col=0xffffff):
    """The game has no ♂ / ♀ in this font: drawn as pixels, as the healthbox does."""
    c = hx(0x60a8f0 if male else 0xf07098)
    sl, xx, yy = b.win(x - 1, y - 1, x + 7, y + 9)
    r = np.hypot(xx - (x + 3), yy - (y + 3 if male else y + 2))
    b.mix(sl, aa(r - 2.6) * (1 - aa(r - 1.6)), c)
    if male:
        b.a[y:y + 1, x + 4:x + 7] = c; b.a[y:y + 3, x + 6:x + 7] = c
    else:
        b.a[y + 5:y + 8, x + 3:x + 4] = c; b.a[y + 6:y + 7, x + 2:x + 5] = c


def box(b, x0, y0, m, theme, numbers=True):
    w, h = 88, 44
    if theme == 's2':
        b.s2_panel(x0, y0, x0 + w, y0 + h, glass=(0x0e1412, 0x47534f))
        name, lab, val = 0xffffff, 0xb8c4c0, 0xffffff
    else:
        b.p1_panel(x0, y0, x0 + w, y0 + h)
        name, lab, val = 0xf8f040, 0xc8d0ff, 0xffffff
    b.text(x0 + w / 2, y0 + 3, m['name'], SMALL, name, align='c')
    b.text(x0 + 5, y0 + 15, 'L%d' % m['lv'], NARROW, val)
    b.text(x0 + 30, y0 + 15, m['cond'], NARROW, lab)
    gender_mark(b, x0 + w - 12, y0 + 14, m['gender'] == '♂')
    b.text(x0 + 5, y0 + 25, 'HP:', NARROW, lab)
    b.hpbar(x0 + 22, y0 + 26, w - 28, m['hp'] / m['max'], theme)
    b.text(x0 + w - 5, y0 + 31, '%d/%d' % (m['hp'], m['max']), NARROW, val, align='r')
    return h


def capsule(b, x0, y0, x1, y1, label, theme, on=False, key=None):
    if theme == 's2':
        b.s2_panel(x0, y0, x1, y1, tint=(1.0, 0.86, 0.45) if on else (1, 1, 1),
                   glass=(0x35350e, 0x6c6c22) if on else (0x101014, 0x3a3a42))
    else:
        b.p1_panel(x0, y0, x1, y1, fill=0x3838c0 if on else 0x181868, edge=0xf8f040 if on else 0xa0a0d0)
    x = x0 + 5
    if key:
        sl, xx, yy = b.win(x0 + 3, y0 + 3, x0 + 15, y1 - 3)
        cx, cy = x0 + 8, (y0 + y1) / 2
        r = np.hypot(xx - cx, yy - cy)
        b.mix(sl, aa(r - 5), hx(0x101018))
        b.mix(sl, aa(r - 4.2), hx(0xc8ccd4))
        b.mix(sl, aa(r - 2.6) * (yy < cy), hx(0xffffff))
        b.text(cx, cy - 5, key, NARROW, 0x202028, ink=0xc8ccd4, align='c')
        x = x0 + 15
    elif on:
        sl, xx, yy = b.win(x0 + 3, y0 + 4, x0 + 11, y1 - 4)
        cy = (y0 + y1) / 2
        b.mix(sl, aa(np.maximum(np.abs(yy - cy) - 2.6 + (xx - (x0 + 4)) * 0.9, -(xx - (x0 + 4)) - 0.5)), hx(0xf8f040))
        x = x0 + 11
    b.text((x + x1) / 2, (y0 + y1) / 2 - 5, label, SMALL, 0xffffff if theme == 's2' or on else 0xd8d8ff, align='c')


def movebar(b, x0, y0, x1, y1, mv, theme, sel):
    name, ty, pp, mx = mv
    if theme == 's2':
        b.s2_panel(x0, y0, x1, y1, tint=(1.0, 0.86, 0.55) if sel else (1.0, 1.0, 1.0),
                   glass=((TYPE_RGB[ty] & 0xfefefe) >> 1 | 0x201008, 0x140a04) if sel else (0x101014, 0x3c3c44))
    else:
        b.p1_panel(x0, y0, x1, y1, fill=TYPE_RGB[ty] if sel else 0x3c3cb8, edge=0xf8f040 if sel else 0xd8d8ff)
    b.text(x0 + 6, y0 + 2, name, SMALL)
    b.text(x0 + 6, y0 + 11, ty, NARROW, 0xf8e8a0 if sel else 0xc8ccd8)
    b.text(x1 - 6, y0 + 11, '%d/%d' % (pp, mx), NARROW, 0xffffff, align='r')
    if sel:
        sl, xx, yy = b.win(x1 - 12, y0 + 2, x1 - 4, y0 + 11)
        cy = y0 + 6.5
        b.mix(sl, aa(np.maximum(np.abs(yy - cy) - 2.4 + (xx - (x1 - 11)) * 0.9, -(xx - (x1 - 11)) - 0.5)), hx(0xf8f040))


def dpad(b, cx, cy, theme):
    for dx, dy in ((0, -1), (0, 1), (-1, 0), (1, 0)):
        x, y = cx + dx * 14, cy + dy * 13
        sl, xx, yy = b.win(x - 7, y - 7, x + 8, y + 8)
        r = np.hypot(xx - x, yy - y)
        b.mix(sl, aa(r - 6), hx(0x201c00))
        b.mix(sl, aa(r - 5), hx(0xe8d820 if theme == 's2' else 0xd8d040))
        b.mix(sl, aa(r - 2.8) * (yy < y - 0.5), hx(0xfff4b0))
        across = np.abs((xx - x) * (0 if dx else 1) + (yy - y) * (0 if dy else 1))
        along = (xx - x) * dx + (yy - y) * dy
        b.mix(sl, aa(np.maximum(across + along * 0.9 - 2.4, -along - 2.2)), hx(0x282400))


def scene(theme, state):
    b = B(battle_bg())
    b.blit(scale(front(FOE['sp']), 64, 64), 150, 14)
    bk = np.array(Image.open(f'{REPO}/graphics/pokemon/{MINE["sp"]}/back.png').convert('RGBA')).astype(np.float32) / 255
    bk[np.array(Image.open(f'{REPO}/graphics/pokemon/{MINE["sp"]}/back.png')) == 0, 3] = 0
    b.blit(bk, 26, 56)
    panel = b.s2_panel if theme == 's2' else b.p1_panel
    # the foe, where the game puts its box: top left, its portrait and team below
    box(b, 6, 4, FOE, theme)
    b.portrait(6, 51, FOE['sp'], theme, 26)
    b.dots(40, 56, 2, 3, theme)
    if theme == 's2':
        b.s2_panel(36, 63, 94, 75)
    else:
        b.p1_panel(36, 63, 94, 75, fill=0x107818)
    b.text(65, 65, 'BERNIE', SMALL, align='c')
    # yours: bottom right, the 1P tab over it
    if theme == 's2':
        b.s2_panel(146, 46, 176, 57)
    else:
        b.p1_panel(146, 46, 176, 57, fill=0x5050d8)
    b.text(161, 47, '1P', SMALL, 0xf8f040 if theme == 'p1' else 0xffffff, align='c')
    box(b, 146, 58, MINE, theme)
    b.portrait(118, 58, MINE['sp'], theme, 26)
    b.dots(122, 90, 3, 3, theme)
    if state == 'action':
        panel(4, 118, 132, 156)
        b.text(10, 125, 'What will', SMALL)
        b.text(10, 138, 'SQUIRTLE do?', SMALL)
        for i, lab in enumerate(('FIGHT', 'BAG', 'POKéMON', 'RUN')):
            x0, y0 = 138 + (i % 2) * 50, 118 + (i // 2) * 20
            capsule(b, x0, y0, x0 + 48, y0 + 17, lab, theme, on=(i == 0))
    else:
        for i, mv in enumerate(MOVES):
            x0, y0 = 4 + (i % 2) * 118, 106 + (i // 2) * 25
            movebar(b, x0, y0, x0 + 114, y0 + 22, mv, theme, sel=(i == 0))
    return b


if __name__ == '__main__':
    ims = []
    for theme in ('s2', 'p1'):
        for state in ('action', 'move'):
            im = Image.fromarray((np.clip(scene(theme, state).a, 0, 1) * 255 + .5).astype(np.uint8))
            im.save(f'{OUT}/b_{theme}_{state}.png')
            ims.append(im.resize((W * 4, H * 4), Image.NEAREST))
            print(theme, state)
    s = Image.new('RGB', (W * 8 + 8, H * 8 + 8), (40, 40, 48))
    for i, im in enumerate(ims):
        s.paste(im, ((i % 2) * (W * 4 + 8), (i // 2) * (H * 4 + 8)))
    s.save(f'{OUT}/b_all.png')
