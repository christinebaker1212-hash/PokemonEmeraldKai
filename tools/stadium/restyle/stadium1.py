# POKESTADIUM THEME: STADIUM's screens after the first POKéMON STADIUM (the user's recording of 2026-11-12): the
# starfield, the blue and green grid plates, the violet entry lists, the olive ladder, WIN in red and yellow.
# 480x320, the game's fonts and pictures; nothing traced from a stretched frame.
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s3 import *
from screens2 import sprite_in, ITALIC, HEAVY
from screens3 import C4

OUT = os.path.dirname(os.path.abspath(__file__))
_r = np.random.default_rng(5)
SHADOW = hx(0x101020)


class S1(C4):
    def t(self, x, y, s, color=0xffffff, align='l', ink=SHADOW, bold=False, font=SMALL):
        return self.text(x, y, s, font, color, ow=2, outline=ink, align=align, bold=bold)

    # ---- backdrops ----
    def stars(self):
        self.a[:] = 0.0
        for _ in range(170):
            x, y, b = int(_r.integers(0, W)), int(_r.integers(0, H)), float(_r.random()) ** 2
            self.a[y, x] = 0.35 + 0.65 * b
            if b > 0.75:
                self.a[max(0, y - 1):y + 2, x] = np.maximum(self.a[max(0, y - 1):y + 2, x], 0.5)
                self.a[y, max(0, x - 1):x + 2] = np.maximum(self.a[y, max(0, x - 1):x + 2], 0.5)

    def cup_backdrop(self, n=0, dim=0.0, zoom=1.0):
        """A cup's own picture filling the screen, as STADIUM puts it behind its menus."""
        p = stadium_pic(n)
        w = int(480 * zoom); h = int(w * 96 / 128)
        big = scale(p, w, h, Image.BICUBIC)[..., :3]
        y0, x0 = (h - 320) // 2, (w - 480) // 2
        self.a = big[y0:y0 + 320, x0:x0 + 480].copy()
        if dim:
            self.a *= (1 - dim)

    def deep(self):
        """The choice screen: deep blue, a great POKé BALL in its own blue behind."""
        yy, xx = np.mgrid[0:H, 0:W].astype(np.float32)
        self.a = ramp(yy / H, [(0, 0x0808b0), (0.5, 0x0404a0), (1, 0x000050)]).astype(np.float32)
        cx, cy, r = 240, 160, 96
        nx, ny = (xx - cx) / r, (yy - cy) / r
        rr = np.hypot(nx, ny)
        nz = np.sqrt(np.clip(1 - rr * rr, 0, 1))
        v = 0.35 + 0.65 * np.clip(-0.3 * nx - 0.55 * ny + 0.75 * nz, 0, 1)
        ball = np.stack([0.20 * v, 0.28 * v, 0.95 * v], -1)
        m = aa((rr - 1) * r)
        self.a = self.a * (1 - m[..., None]) + ball * m[..., None]
        band = m * (np.abs(ny) < 0.05)
        self.a = self.a * (1 - band[..., None]) + hx(0x0808a0) * band[..., None]
        for rad, col in ((0.30, 0x0808a0), (0.24, 0x5070f0), (0.13, 0x90a8ff)):
            k = aa((rr - rad) * r)
            self.a = self.a * (1 - k[..., None]) + hx(col) * k[..., None]

    # ---- pieces ----
    def titlebar(self, word):
        """The title: a POKé BALL and a violet bar with the words in yellow."""
        sl, xx, yy = self.win(30, 6, 446, 30)
        d = sdf_rr(xx, yy, 34, 9, 440, 27, 9)
        self.mix(sl, aa(d - 1.5), hx(0x9a8cf0)); self.mix(sl, aa(d), hx(0x2c28b0))
        self.mix(sl, aa(d) * (yy < 15), hx(0x3c38c8))
        self.ball(40, 18, 12, top=0xe82020)
        self.t(29, 3.5, word, 0xf8f040, bold=True)

    def box(self, x0, y0, x1, y1, fill=0x20209c, edge=0xd8d8ff, r=3):
        """STADIUM's menu box: navy in a pale line."""
        sl, xx, yy = self.win(x0 - 2, y0 - 2, x1 + 4, y1 + 4)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, r)
        self.mix(sl, aa(sdf_rr(xx, yy, x0 + 2, y0 + 2, x1 + 2, y1 + 2, r)) * 0.6, hx(0x000000))
        self.mix(sl, aa(d), hx(0x080818)); self.mix(sl, aa(d + 1), hx(edge)); self.mix(sl, aa(d + 3), hx(fill))

    def veil(self, x0, y0, x1, y1, color=0x3030b0, alpha=0.72, r=6):
        sl, xx, yy = self.win(x0, y0, x1, y1)
        self.mix(sl, aa(sdf_rr(xx, yy, x0, y0, x1, y1, r)) * alpha, hx(color))

    def wbrackets(self, x0, y0, x1, y1):
        self.brackets(x0, y0, x1, y1, arm=8, th=2, color=0xffffff)

    def gridplate(self, x0, y0, x1, y1, cols, rows, green=False):
        """A team's plate: cells of a dark field with a glowing grid, lines between them."""
        base, line, glow = (0x083828, 0x30b048, 0x209848) if green else (0x0c1070, 0x7868e0, 0x3048d8)
        x0, y0, x1, y1 = int(x0), int(y0), int(x1), int(y1)
        yy, xx = np.mgrid[y0:y1, x0:x1].astype(np.float32)
        cw, ch = (x1 - x0) / cols, (y1 - y0) / rows
        u, v = ((xx - x0) % cw) / cw, ((yy - y0) % ch) / ch
        g = np.maximum(np.exp(-((u * 6) % 1 - 0.5) ** 2 / 0.004), np.exp(-((v * 5) % 1 - 0.5) ** 2 / 0.004))
        fall = 0.35 + 0.65 * np.hypot(u - 0.5, v - 0.5) * 1.6
        self.a[y0:y1, x0:x1] = hx(base)[None, None] * (0.8 + 0.5 * (1 - v))[..., None] + hx(glow)[None, None] * (0.38 * g * fall)[..., None]
        for k in range(cols + 1):
            x = int(round(x0 + k * cw)); self.a[y0:y1, max(x0, x - 1):min(x1, x + 1)] = hx(line)
        for k in range(rows + 1):
            y = int(round(y0 + k * ch)); self.a[max(y0, y - 1):min(y1, y + 1), x0:x1] = hx(line)
        return [(int(x0 + c * cw) + 1, int(y0 + r * ch) + 1, int(x0 + (c + 1) * cw) - 1, int(y0 + (r + 1) * ch) - 1) for r in range(rows) for c in range(cols)]

    def greenbar(self, x0, y0, x1, frac):
        self.a[y0:y0 + 5, x0:x1] = hx(0x101850)
        self.a[y0 + 1:y0 + 4, x0 + 1:x0 + 1 + int((x1 - x0 - 2) * frac)] = hx(0x30e838)

    def fat(self, cx, cy, word, px, top, bottom, edge, shadow=0x000000, font=HEAVY, squash=1.0):
        """STADIUM's fat lettering (WIN, LOSE, VS, the ladder's words): a gradient in a light line over a shadow."""
        S = 3
        f = ImageFont.truetype(font, int(px * S))
        l, t, r, b = f.getbbox(word)
        pad = 12 * S
        m = Image.new('L', (r - l + pad * 2, b - t + pad * 2), 0)
        ImageDraw.Draw(m).text((pad - l, pad - t), word, font=f, fill=255)
        fat1 = m.filter(ImageFilter.MaxFilter(2 * S * 2 + 1))
        fat2 = m.filter(ImageFilter.MaxFilter(4 * S * 2 + 1))
        sm = lambda im: np.array(im.resize((im.width // S, im.height // S), Image.LANCZOS)).astype(np.float32) / 255
        M, E, O = sm(m), sm(fat1), sm(fat2)
        tt = np.clip((np.arange(M.shape[0])[:, None] - pad / S) / ((b - t) / S), 0, 1) * np.ones((1, M.shape[1]))
        face = ramp(tt, [(0, top), (1, bottom)])
        rgba = np.zeros(M.shape + (4,), np.float32)
        rgba[..., :3] = hx(shadow); rgba[..., 3] = O
        rgba[..., :3] = rgba[..., :3] * (1 - E[..., None]) + hx(edge) * E[..., None]
        rgba[..., :3] = rgba[..., :3] * (1 - M[..., None]) + face * M[..., None]
        self.blit(rgba, cx - M.shape[1] / 2, cy - M.shape[0] / 2)

    def button(self, cx, cy, kind):
        """The pick buttons beside each POKéMON: 1-3 in red once picked, an empty socket before."""
        if kind:
            self.sphere(cx, cy, 8, 0xe03828)
            self.text(cx / 2 + 0.5, cy / 2 - 6.5, str(kind), SMALL, ow=1, outline=hx(0x400808), align='c')
        else:
            self.sphere(cx, cy, 7, 0x5060a0, flat=True)

    def hints(self, items, y=148.5, color=0xffffff):
        x = 14
        for key, word in items:
            cap = keycap(key)
            self.blit(cap, x * 2, y * 2 + 2)
            x += cap.shape[1] // 2 + 2
            x += self.t(x, y, word, color) + 10


TEAM = [('ursaring', 'URSARING', 50, 1), ('politoed', 'POLITOED', 50, 1), ('kadabra', 'KADABRA', 50, 0),
        ('hitmonlee', 'HITMONLEE', 50, 1), ('graveler', 'GRAVELER', 50, 0), ('dragonair', 'DRAGONAIR', 50, 1)]
FOES = [('starmie', 'STARMIE', 50, 0), ('ursaring', 'URSARING', 50, 1), ('alakazam', 'ALAKAZAM', 50, 0),
        ('umbreon', 'UMBREON', 50, 1), ('muk', 'MUK', 50, 0), ('tyranitar', 'TYRANITAR', 50, 1)]


# ---------------------------------------------------------------------------------------------------------------
def cups_screen():
    """The cups as pictures floating over the island; the one chosen named below."""
    c = S1()
    c.sky()
    yy = np.mgrid[0:H, 0:W][0]
    c.a = c.a * (1 - 0.5 * smooth(200, 320, yy)[..., None]) + hx(0x58a048) * 0.5 * smooth(200, 320, yy)[..., None]
    spots = [(30, 66, 3), (174, 14, 0), (174, 122, 1), (318, 66, 2)]      # PIKA, POKé, PRIME, PETIT as STADIUM lays them
    focus = 1
    for i, (x, y, n) in enumerate(spots):
        w, h = 132, 99
        c.a[y + 4:y + h + 4, x + 4:x + w + 4] *= 0.45
        c.a[y - 3:y + h + 3, x - 3:x + w + 3] = hx(0x181820)
        c.a[y - 2:y + h + 2, x - 2:x + w + 2] = hx(0xf0f0f8) if i != focus else hx(0xf8f040)
        c.blit(scale(stadium_pic(n), w, h, Image.BILINEAR), x, y)
        if i == focus:
            c.wbrackets(x - 7, y - 7, x + w + 7, y + h + 7)
    c.veil(16, 232, 464, 290)
    c.t(16, 118, 'POKé CUP', 0xf8f040, bold=True)
    c.t(224, 118, 'LEVEL: 50-55', 0xf8f040, align='r', bold=True)
    c.t(16, 131, 'Three of six, 155 levels in all.')
    c.hints([('A', 'ENTER'), ('L', ''), ('R', 'ROUND'), ('B', 'EXIT')])
    return c


def class_screen():
    """The cup's own picture behind, its title, the four classes as STADIUM's banners."""
    c = S1()
    c.cup_backdrop(0, dim=0.15)
    c.titlebar('POKé CUP')
    for t in range(4):
        pic = stadium_pic(4 + t)
        if t == 3:
            l = (pic[..., :3] * [0.30, 0.59, 0.11]).sum(-1, keepdims=True) * 0.75 + 0.08
            pic = np.concatenate([np.repeat(l, 3, -1), pic[..., 3:] * 0.7], -1)
        x, y = 22 + (10 if t == 1 else 0), 40 + t * 62
        b = scale(pic, 213, 60, Image.LANCZOS)
        sh = np.zeros_like(b); sh[..., 3] = b[..., 3] * 0.6
        c.blit(sh, x + 3, y + 3); c.blit(b, x, y)
        if t == 1:
            c.wbrackets(x - 6, y - 2, x + 219, y + 62)
    c.box(262, 44, 468, 282)
    c.t(137, 25, 'GREAT BALL', 0xf8f040, bold=True)
    c.t(137, 37, '8 TRAINERS')
    faces = ['bug_catcher', 'lass', 'camper', 'picnicker', 'hiker', 'psychic_m', 'beauty', 'cooltrainer_m']
    for i, name in enumerate(faces):
        x, y = 272 + (i % 4) * 48, 104 + (i // 4) * 54
        c.a[y:y + 50, x:x + 44] = hx(0x5050d0) if i < 7 else hx(0xb030a0)
        c.clip_blit(trainer(name), x - 10, y - 4, (x, y, x + 44, y + 50))
    c.t(137, 109, 'BEST'); 
    for k in range(4):
        c.sphere(332 + k * 22, 232, 7, 0xe8b020) if k < 2 else c.sphere(332 + k * 22, 232, 6.5, 0x5060a0, flat=True)
    c.t(137, 124, '2 of 4 restrictions')
    c.veil(0, 292, 480, 320, 0x101060, 0.75, 0)
    c.hints([('A', 'CHOOSE'), ('B', 'BACK')])
    return c


def entry_top(c, team):
    c.cup_backdrop(0, dim=0.25, zoom=1.6)
    c.titlebar('Register a team')
    c.t(214, 3.5, 'ENTRY %d / 6' % len(team), align='r')
    x0, y0, x1, y1 = 14, 36, 466, 122
    c.a[y0 - 2:y1 + 2, x0 - 2:x1 + 2] = hx(0x101030); c.a[y0 - 1:y1 + 1, x0 - 1:x1 + 1] = hx(0xe8e8ff)
    c.a[y0:y1, x0:x1] = hx(0x4444c4)
    cw, ch = (x1 - x0) / 3, (y1 - y0) / 2
    for i in range(6):
        cx0, cy0 = int(x0 + (i % 3) * cw), int(y0 + (i // 3) * ch)
        c.a[cy0 + 1:int(cy0 + ch) - 1, cx0 + 1:int(cx0 + cw) - 1] = hx(0x5858d8) if (i + i // 3) % 2 == 0 else hx(0x3c3cb8)
        if i < len(team):
            sp, nm, lv, item = team[i]
            c.clip_blit(scale(icon(sp), 64, 64), cx0 - 6, cy0 - 18, (cx0, cy0 - 4, cx0 + cw, cy0 + ch))
            c.t(cx0 / 2 + 24, cy0 / 2 - 1, nm)
            c.t(cx0 / 2 + 24, cy0 / 2 + 9, 'L%d' % lv)
            if item:
                c.itembox(cx0 / 2 + 48, cy0 / 2 + 9)
        elif i == len(team):
            c.wbrackets(cx0 + 4, cy0 + 4, int(cx0 + cw) - 4, int(cy0 + ch) - 4)
            c.t(cx0 / 2 + cw / 4, cy0 / 2 + 4.5, 'Fifth POKéMON', 0xf8f040, align='c')


def tabrow(c, y=127):
    c.a[y:y + 20, 30:450] = hx(0x181018)
    names = ['PARTY', 'BOXES', 'SHARED', 'GLOBAL', 'RENTALS']
    x = 36
    c.a[y + 2:y + 18, x:x + 22] = hx(0xc03048); c.t(x / 2 + 5.5, y / 2 - 2, 'L', align='c'); x += 28
    for nm in names:
        w = measure(nm, SMALL) * 2 + 10
        sel = nm == 'RENTALS'
        if sel:
            c.a[y - 1:y + 21, x - 2:x + w + 2] = hx(0xffffff)
        c.a[y + 1:y + 19, x:x + w] = hx(0xb828a8) if sel else hx(0x8a8030)
        c.a[y + 1:y + 3, x:x + w] = hx(0xe060d0) if sel else hx(0xb8ac50)
        c.t((x + w / 2) / 2, y / 2 - 2, nm, 0xffffff if sel else 0xe8e0a0, align='c')
        x += w + 4
    c.a[y + 2:y + 18, 422:444] = hx(0xc03048); c.t(216.5, y / 2 - 2, 'R', align='c')


def entry_screen():
    c = S1()
    entry_top(c, TEAM[:4])
    tabrow(c)
    x0, y0, x1, y1 = 14, 152, 466, 290
    c.a[y0 - 2:y1 + 2, x0 - 2:x1 + 2] = hx(0x101030); c.a[y0 - 1:y1 + 1, x0 - 1:x1 + 1] = hx(0xe8e8ff)
    c.a[y0:y1, x0:x1] = hx(0x4444c4)
    c.a[y0:y1, x0:x0 + 14] = hx(0x2c2ca0)
    c.thumb(x0 + 7, y0 + 24)
    names = ['EKANS', 'ELECTABUZZ', 'ELECTRODE', 'ELEKID', 'ENTEI', 'ESPEON', 'EXEGGCUTE', 'EXEGGUTOR', "FARFETCH'D"]
    folders = ['ekans', 'electabuzz', 'electrode', 'elekid', 'entei', 'espeon', 'exeggcute', 'exeggutor', 'farfetchd']
    cw, ch = (x1 - x0 - 14) / 3, (y1 - y0) / 3
    for i, (nm, fd) in enumerate(zip(names, folders)):
        cx0, cy0 = int(x0 + 14 + (i % 3) * cw), int(y0 + (i // 3) * ch)
        c.a[cy0 + 1:int(cy0 + ch), cx0 + 1:int(cx0 + cw)] = hx(0x5858d8) if (i + i // 3) % 2 == 0 else hx(0x4848c8)
        c.clip_blit(scale(icon(fd), 64, 64), cx0 - 8, cy0 - 16, (cx0, cy0, cx0 + cw, cy0 + ch))
        c.t(cx0 / 2 + 23, cy0 / 2 + 0.5, nm); c.t(cx0 / 2 + 23, cy0 / 2 + 10.5, 'L50')
        if i == 5:
            c.wbrackets(cx0 + 3, cy0 + 3, int(cx0 + cw) - 2, int(cy0 + ch) - 2)
    c.veil(0, 294, 480, 320, 0x101060, 0.75, 0)
    c.hints([('A', 'LOOK'), ('SELECT', 'SUMMARY'), ('L', ''), ('R', 'FROM'), ('B', 'DONE')], color=0xffffff)
    return c


def info_screen():
    c = S1()
    entry_top(c, TEAM[:4])
    tabrow(c)
    x0, y0, x1, y1 = 14, 152, 466, 291
    c.a[y0 - 2:y1 + 2, x0 - 2:x1 + 2] = hx(0x101030); c.a[y0 - 1:y1 + 1, x0 - 1:x1 + 1] = hx(0xe8e8ff)
    c.a[y0:y1, x0:x1] = hx(0xa89c38)
    c.a[y0:y0 + 21, x0:372] = hx(0xc8a030)
    c.a[y0:y0 + 96, 372:x1] = hx(0x20209c)
    L = [76, 85.4, 94.8, 104.2, 113.6]
    c.t(11, L[0], 'ESPEON'); c.t(52, L[0], 'L50'); c.t(80, L[0], 'No.196')
    c.a[176:240, 22:90] = hx(0x282890)
    c.clip_blit(front('espeon'), 24, 176, (22, 176, 90, 240))
    c.typepill(50, 89, 'PSYCHIC')
    c.t(50, 99, 'HP'); c.t(84, 99, '141', align='r')
    wd = measure('RENTAL', NARROW) + 7
    c.pill(100, 222, 100 + wd * 2, 240, 0xb030a0); c.text(50 + wd / 2 + 0.5, 110, 'RENTAL', NARROW, 0xf8f040, ow=1, outline=SHADOW, align='c')
    for i, (nm, v) in enumerate((('ATTACK', 85), ('DEFENSE', 80), ('SP. ATK', 150), ('SP. DEF', 115), ('SPEED', 130))):
        c.t(130, L[i], nm); c.t(182, L[i], str(v), align='r')
    c.t(210, 77, 'ENTER', align='c'); c.pointer(388, 199); c.t(203, 93, 'Yes', 0xf8f040); c.t(203, 103.5, 'No')
    c.itembox(194, 127); c.t(205, 127, 'NONE')
    for i, (mv, ty) in enumerate((('PSYCHIC', 'PSYCHIC'), ('SHADOW BALL', 'GHOST'), ('MORNING SUN', 'NORMAL'), ('REFLECT', 'PSYCHIC'))):
        x, y = 11 + (i % 2) * 64, 124.5 + (i // 2) * 9.4
        c.sphere(x * 2 + 5, y * 2 + 13, 4.5, TYPE_RGB[ty])
        c.t(x + 6, y, mv, 0xf8f040 if i % 2 == 0 else 0xffffff)
    c.veil(0, 294, 480, 320, 0x101060, 0.75, 0)
    c.hints([('A', 'CHOOSE'), ('SELECT', 'SUMMARY'), ('B', 'BACK')])
    return c


def ladder_screen():
    c = S1()
    c.stars()
    x0, y0, x1, y1 = 76, 14, 404, 306
    c.a[y0:y1, x0:x1] = hx(0x8a8438)
    c.a[y0:y0 + 3, x0:x1] = hx(0xc8c070); c.a[y0:y1, x0:x0 + 3] = hx(0xc8c070)
    c.a[y1 - 3:y1, x0:x1] = hx(0x504c18); c.a[y0:y1, x1 - 3:x1] = hx(0x504c18)
    bx0, by0, bx1, by1 = x0 + 10, y0 + 10, x1 - 10, y0 + 128
    c.a[by0 - 2:by1 + 2, bx0 - 2:bx1 + 2] = hx(0x504c18)
    c.clip_blit(scale(stadium_pic(21), 392, 147, Image.BILINEAR), bx0 - 40, by0 - 14, (bx0, by0, bx1, by1))
    c.fat(240, by0 + 38, 'POKé CUP', 34, 0xfff8a0, 0xd8a818, 0x282008)
    c.fat(240, by0 + 84, 'BATTLE 1', 34, 0xfff8a0, 0xd8a818, 0x282008)
    faces = ['school_kid_m'] + [None] * 7
    for i in range(8):
        tx, ty = x0 + 10 + (i % 4) * 78, y0 + 140 + (i // 4) * 76
        c.a[ty - 2:ty + 70, tx - 2:tx + 74] = hx(0x504c18)
        c.a[ty:ty + 68, tx:tx + 72] = hx(0xf0f0f0)
        if faces[i]:
            c.a[ty:ty + 68, tx:tx + 72] = hx(0x282890)
            c.clip_blit(trainer(faces[i]), tx + 4, ty + 2, (tx, ty, tx + 72, ty + 68))
            c.a[ty - 2:ty, tx - 2:tx + 74] = hx(0xf08020); c.a[ty + 68:ty + 70, tx - 2:tx + 74] = hx(0xf08020)
            c.a[ty - 2:ty + 70, tx - 2:tx] = hx(0xf08020); c.a[ty - 2:ty + 70, tx + 72:tx + 74] = hx(0xf08020)
        else:
            sl, xx, yy = c.win(tx, ty, tx + 72, ty + 68)
            head = aa(np.hypot(xx - (tx + 36), yy - (ty + 24)) - 15)
            body = aa(sdf_rr(xx, yy, tx + 10, ty + 40, tx + 62, ty + 90, 16))
            c.mix(sl, np.maximum(head, body), hx(0x383840))
            c.fat(tx + 36, ty + 34, '?', 40, 0xff5040, 0xa80808, 0x400000)
    return c


def vs_screen():
    c = S1()
    c.stars()
    cells = c.gridplate(22, 18, 458, 146, 4, 2)
    x0, y0, x1, y1 = cells[0]
    c.a[y0:cells[4][3], x0:x1] = hx(0x7868e0)
    c.fat((x0 + x1) / 2, y0 + 12, '1P', 20, 0xfff8a0, 0xe0a010, 0x282060, font=ITALIC)
    c.t((x0 + x1) / 4, y0 / 2 + 11, 'BRENDAN', align='c')
    c.a[y0 + 48:cells[4][3] - 4, x0 + 18:x1 - 18] = hx(0x000000)
    c.clip_blit(trainer('brendan'), (x0 + x1) / 2 - 32, y0 + 50, (x0 + 18, y0 + 48, x1 - 18, cells[4][3] - 4))
    for i, (sp, nm, lv, item) in enumerate(TEAM):
        sprite_in(c, cells[1 + i % 3 + (i // 3) * 4], front(sp))
    cells = c.gridplate(22, 150, 458, 278, 4, 2, green=True)
    for i, (sp, nm, lv, item) in enumerate(FOES):
        sprite_in(c, cells[i % 3 + (i // 3) * 4], front(sp))
    x0, y0, x1, y1 = cells[3]
    c.a[y0:cells[7][3], x0:x1] = hx(0x28a028)
    c.t((x0 + x1) / 4, y0 / 2 + 1, 'COOLTRAINER', align='c'); c.t((x0 + x1) / 4, y0 / 2 + 11, 'MARTY', align='c')
    c.a[y0 + 48:cells[7][3] - 4, x0 + 18:x1 - 18] = hx(0xf0f0f0)
    c.clip_blit(trainer('cooltrainer_m'), (x0 + x1) / 2 - 32, y0 + 50, (x0 + 18, y0 + 48, x1 - 18, cells[7][3] - 4))
    c.fat(240, 148, 'VS', 60, 0xffe040, 0xe83010, 0x301000, font=ITALIC)
    c.hints([('A', 'CHOOSE POKéMON'), ('SELECT', 'RULES'), ('B', 'QUIT')], y=146)
    return c


def choice_screen():
    c = S1()
    c.deep()
    def plate(y0, team, green, picks=None, cursor=None):
        fill, edge = (0x107818, 0xd8d040) if green else (0x000880, 0xd8d040)
        c.box(40, y0, 474, y0 + 90, fill, edge, r=5)
        for i, (sp, nm, lv, item) in enumerate(team):
            x, y = 46 + (i % 3) * 142, y0 + 5 + (i // 3) * 41
            c.clip_blit(scale(icon(sp), 64, 64), x - 6, y - 18, (x, y - 2, x + 50, y + 40))
            if picks is not None:
                c.button(x + 7, y + 9, picks.get(i, 0))
            c.t(x / 2 + 21, y / 2 - 1.5, nm); c.t(x / 2 + 21, y / 2 + 8, 'L%d' % lv)
            if item:
                c.itembox(x / 2 + 40, y / 2 + 8)
            if cursor == i:
                c.wbrackets(x - 3, y - 2, x + 139, y + 40)
    plate(8, TEAM, False, {0: 1, 3: 2, 5: 3}, cursor=1)
    c.box(40, 100, 136, 126, 0x000880, 0xd8d040); c.t(44, 50.5, 'BRENDAN', align='c')
    c.box(6, 8, 34, 96, 0x90a020, 0xd8d040)
    c.fat(20, 20, '1P', 13, 0xffffff, 0xd0d8ff, 0x101060, font=ITALIC)
    for k in range(3):
        c.ball(20, 42 + k * 20, 7, top=0xe82020)
    plate(202, FOES, True)
    c.box(380, 172, 474, 198, 0x107818, 0xd8d040); c.t(213.5, 86.5, 'MARTY', align='c')
    c.box(6, 204, 34, 292, 0x90a020, 0xd8d040)
    for k in range(3):
        c.ball(20, 222 + k * 20, 7, top=0x58c838)
    c.text(10, 138, 'COM', NARROW, ow=1, outline=SHADOW, align='c')
    c.box(40, 130, 374, 198, 0x108018, 0xd8d040)
    c.t(103.5, 72, 'All ready!', align='c', bold=True)
    c.t(103.5, 84, 'Lv 150 / 155', 0xf8f040, align='c')
    c.hints([('A', 'PICK'), ('R', 'SUMMARY'), ('START', 'BATTLE!'), ('B', 'BACK')], y=148)
    return c


def result_screen():
    c = S1()
    c.stars()
    cells = c.gridplate(22, 18, 458, 146, 4, 1)
    x0, y0, x1, y1 = cells[0]
    c.a[y0:y1, x0:x1] = hx(0x7868e0)
    c.fat((x0 + x1) / 2, y0 + 12, '1P', 20, 0xfff8a0, 0xe0a010, 0x282060, font=ITALIC)
    c.t((x0 + x1) / 4, y0 / 2 + 11, 'BRENDAN', align='c')
    c.a[y0 + 50:y1 - 4, x0 + 18:x1 - 18] = hx(0x000000)
    c.clip_blit(trainer('brendan'), (x0 + x1) / 2 - 32, y0 + 54, (x0 + 18, y0 + 50, x1 - 18, y1 - 4))
    for i, (sp, hp) in enumerate((('ursaring', 0.86), ('hitmonlee', 1.0), ('dragonair', 1.0))):
        b = cells[1 + i]
        big = scale(front(sp), 128, 128)
        c.clip_blit(big, (b[0] + b[2]) / 2 - 64, b[1] - 6, b)
        c.greenbar(b[0] + 8, b[3] - 9, b[2] - 8, hp)
    cells = c.gridplate(22, 150, 458, 278, 4, 1, green=True)
    for i, sp in enumerate(('starmie', 'umbreon', 'tyranitar')):
        b = cells[i]
        c.clip_blit(scale(front(sp), 128, 128), (b[0] + b[2]) / 2 - 64, b[1] - 6, b, dim=0.5)
        sl, xx, yy = c.win(*b)
        cx, cy = (b[0] + b[2]) / 2, (b[1] + b[3]) / 2 - 8
        d = np.minimum(np.abs((xx - cx) - (yy - cy)), np.abs((xx - cx) + (yy - cy))) / 1.414 - 5
        c.mix(sl, aa(d) * (np.hypot(xx - cx, yy - cy) < 44) * 0.9, hx(0xe02828))
        c.fat(cx, b[3] - 16, 'FAINTED', 17, 0xf0b0ff, 0x8030c0, 0x200830)
        c.greenbar(b[0] + 8, b[3] - 9, b[2] - 8, 0.0) if False else None
    x0, y0, x1, y1 = cells[3]
    c.a[y0:y1, x0:x1] = hx(0x28a028)
    c.t((x0 + x1) / 4, y0 / 2 + 1, 'COOLTRAINER', align='c'); c.t((x0 + x1) / 4, y0 / 2 + 11, 'MARTY', align='c')
    c.a[y0 + 50:y1 - 4, x0 + 18:x1 - 18] = hx(0xf0f0f0)
    c.clip_blit(trainer('cooltrainer_m'), (x0 + x1) / 2 - 32, y0 + 54, (x0 + 18, y0 + 50, x1 - 18, y1 - 4))
    c.fat(296, 76, 'WIN', 74, 0xf01818, 0xffe838, 0xffffff, shadow=0x500808)
    c.fat(186, 204, 'LOSE', 62, 0x2838c0, 0x101060, 0xb8d0ff, shadow=0x000020)
    c.hints([('A', 'OK')], y=146)
    return c


def badges_screen(box=False):
    c = S1()
    c.stars()
    tiles = [('BOULDER', 0x8c8c8c), ('CASCADE', 0x3878d8), ('THUNDER', 0xe89858), ('RAINBOW', 0x48b868),
             ('SOUL', 0xe078c8), ('MARSH', 0xe8c890), ('VOLCANO', 0xc02848), ('EARTH', 0x389838)]
    for i, (nm, col) in enumerate(tiles):
        x0, y0 = 44 + (i % 4) * 98, 22 + (i // 4) * 132
        w, h = 96, 130
        n = _r.random((h, w, 1)).astype(np.float32)
        tile = hx(col)[None, None] * (0.86 + 0.26 * n)
        if nm == 'RAINBOW':
            yy = np.linspace(0, 1, h)[:, None]
            tile = ramp(yy + 0 * n[..., 0], [(0, 0x9038b0), (0.3, 0x3878d8), (0.5, 0x48c8a0), (0.7, 0xe8e048), (1, 0xe85838)]) * (0.9 + 0.2 * n)
        c.a[y0:y0 + h, x0:x0 + w] = np.clip(tile, 0, 1)
        c.a[y0:y0 + 2, x0:x0 + w] *= 1.3; c.a[y0:y0 + h, x0:x0 + 2] *= 1.3
        c.a[y0 + h - 2:y0 + h, x0:x0 + w] *= 0.6; c.a[y0:y0 + h, x0 + w - 2:x0 + w] *= 0.6
        c.a[y0:y0 + h, x0:x0 + w] = np.clip(c.a[y0:y0 + h, x0:x0 + w], 0, 1)
        lab, wd, pad = text_rgba(nm, SMALL, 0xffffff, hx(col) * 0.45, None, 2, None, True)
        im = Image.fromarray((lab * 255).astype(np.uint8), 'RGBA').rotate(90, expand=True)
        a = np.array(im).astype(np.float32) / 255
        c.blit(a, x0 + w - 30, y0 + h - 6 - a.shape[0])
        if i == 0:
            c.sphere(x0 + 34, y0 + 44, 24, 0x802010); c.sphere(x0 + 34, y0 + 44, 18, 0xf0e8e0); c.sphere(x0 + 34, y0 + 44, 9, 0xe02020)
    if not box:
        return c
    c.box(136, 104, 372, 206)
    c.t(74, 55, 'Keep battling?')
    c.pointer(152, 161); c.t(84, 74, 'Continue', 0xf8f040); c.t(84, 84.5, 'Suspend')
    c.t(180, 90, 'CONTINUES 2', align='r')
    return c


SCENES = (('cups', cups_screen), ('class', class_screen), ('entry', entry_screen), ('info', info_screen), ('ladder', ladder_screen),
          ('vs', vs_screen), ('choice', choice_screen), ('result', result_screen), ('badges', badges_screen), ('continue', lambda: badges_screen(True)))

if __name__ == '__main__':
    ims = []
    for name, fn in SCENES:
        im = fn().image(); im.save(f'{OUT}/p1_{name}.png'); ims.append(im); print(name)
    for part, chunk in (('a', ims[:6]), ('b', ims[6:])):
        rows = (len(chunk) + 1) // 2
        s = Image.new('RGB', (968, rows * 328 - 8), (40, 40, 48))
        for i, im in enumerate(chunk):
            s.paste(im, ((i % 2) * 488, (i // 2) * 328))
        s.resize((s.width * 2, s.height * 2), Image.NEAREST).save(f'{OUT}/p1_all_{part}.png')
