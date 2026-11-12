# The rest of the set after the user's recording of STADIUM 2: the entry screen with the team's names / levels / items,
# its blue info panel and its confirm menu; the cup's ladder and the trainer's line; the result with crosses and the
# loser's line; the castle's badge board; the continue box. 480x320.
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s3 import *
from screens import footer, chip
from screens2 import sprite_in, label, LINES, ITALIC, HEAVY

OUT = os.path.dirname(os.path.abspath(__file__))
NAVY = hx(0x0c0c40)


class C4(C3):
    def bluepanel(self, x0, y0, x1, y1, color=(0x3b3fee, 0x3035d8)):
        sl, xx, yy = self.win(x0 - 2, y0 - 2, x1 + 5, y1 + 5)
        d = sdf_rr(xx, yy, x0, y0, x1, y1, 7)
        self.mix(sl, aa(sdf_rr(xx, yy, x0 + 3, y0 + 3, x1 + 3, y1 + 3, 7)) * 0.9, hx(0x000004))
        self.mix(sl, aa(d), hx(0x10140a))
        gy, gx = np.gradient(d)
        lit = smooth(-0.5, 0.5, (-gx - gy) / 1.4142)
        self.mix(sl, aa(d + 1), np.repeat((0.80 + 0.19 * lit)[..., None], 3, -1))
        self.mix(sl, aa(d + 4), hx(0x3a3a3a))
        t = np.clip((yy - y0) / (y1 - y0), 0, 1)
        self.mix(sl, aa(d + 5), ramp(t, [(0, color[0]), (1, color[1])]))

    def cyanbox(self, x0, y0, x1, y1):
        """The continue box: a bevelled plate of pale cyan with a sheen across it."""
        x0, y0, x1, y1 = int(x0), int(y0), int(x1), int(y1)
        u = np.linspace(0, 1, x1 - x0)
        row = ramp(u, [(0, 0xc8f4f0), (0.18, 0xd8faf6), (0.36, 0x7fe0d8), (0.6, 0xc4f4f0), (0.82, 0xb0eee8), (1, 0x60d4cc)])
        self.a[y0:y1, x0:x1] = row[None]
        for d, (lt, dk) in enumerate(((0xf4fffe, 0x2cb8b0), (0xe0fbf8, 0x40c8c0), (0xd0f6f2, 0x58d0c8))):
            self.a[y0 + d, x0 + d:x1 - d] = hx(lt); self.a[y0 + d:y1 - d, x0 + d] = hx(lt)
            self.a[y1 - 1 - d, x0 + d:x1 - d] = hx(dk); self.a[y0 + d:y1 - d, x1 - 1 - d] = hx(dk)

    def ital(self, cx, cy, word, px, fill=0xffffff, ink=0x202020, emboss=False):
        """Bold italic lettering (the ladder's labels): white in a dark line, or pressed into the metal."""
        S = 4
        f = ImageFont.truetype(ITALIC, int(px * S))
        l, t, r, b = f.getbbox(word)
        m = Image.new('L', (r - l + 16 * S, b - t + 16 * S), 0)
        ImageDraw.Draw(m).text((8 * S - l, 8 * S - t), word, font=f, fill=255)
        face = np.array(m.resize((m.width // S, m.height // S), Image.LANCZOS)).astype(np.float32) / 255
        rgba = np.zeros(face.shape + (4,), np.float32)
        if emboss:      # dark letters with a light edge below and right
            hi = np.roll(np.roll(face, 1, 0), 1, 1)
            rgba[..., :3] = 0.95; rgba[..., 3] = hi * 0.9
            rgba[..., :3] = rgba[..., :3] * (1 - face[..., None]) + hx(ink) * face[..., None]
            rgba[..., 3] = np.maximum(rgba[..., 3], face)
        else:
            out = np.clip(pil_blur((face > 0.3).astype(np.float32), 1.2) * 3.0, 0, 1)
            rgba[..., :3] = hx(ink); rgba[..., 3] = out
            rgba[..., :3] = rgba[..., :3] * (1 - face[..., None]) + hx(fill) * face[..., None]
        self.blit(rgba, cx - face.shape[1] / 2, cy - face.shape[0] / 2)

    def qtile(self, x0, y0, x1, y1, word):
        """A battle not reached yet: ribbed metal with a ? pressed into it and its name."""
        ix0, iy0, ix1, iy1 = self.window(x0, y0, x1, y1, glow=False)
        h = iy1 - iy0
        ribs = 0.78 - 0.30 * (((np.arange(h) % 8) >= 5).astype(np.float32)) + 0.10 * ((np.arange(h) % 8) == 0)
        u = np.linspace(0, 1, ix1 - ix0)
        sh = 1 + 0.10 * np.cos((u - 0.3) * 5)
        self.a[iy0:iy1, ix0:ix1] = (ribs[:, None] * sh[None])[..., None]
        S = 3
        f = ImageFont.truetype(HEAVY, 46 * S)
        l, t, r, b = f.getbbox('?')
        m = Image.new('L', ((ix1 - ix0) * S, h * S), 0)
        ImageDraw.Draw(m).text((((ix1 - ix0) * S - (r - l)) / 2 - l, (h * S - (b - t)) / 2 - t - 6 * S), '?', font=f, fill=255)
        q = np.array(m.resize((ix1 - ix0, h), Image.LANCZOS)).astype(np.float32) / 255
        reg = self.a[iy0:iy1, ix0:ix1]
        reg[:] = reg * (1 - 0.8 * q[..., None]) + 0.97 * 0.8 * q[..., None]
        self.ital((x0 + x1) / 2, y1 - 14, word, 15)

    def cross(self, box):
        """A fainted POKéMON: a bevelled cross over its picture."""
        x0, y0, x1, y1 = box
        sl, xx, yy = self.win(x0, y0, x1, y1)
        cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
        k = min(x1 - x0, y1 - y0) * 0.36
        a, b = (xx - cx + yy - cy) / 1.4142, (xx - cx - (yy - cy)) / 1.4142
        d = np.minimum(np.maximum(np.abs(a) - k, np.abs(b) - 6), np.maximum(np.abs(b) - k, np.abs(a) - 6))
        self.mix(sl, aa(d - 1), hx(0x3a2a34))
        self.mix(sl, aa(d), hx(0xa89aa2))
        gy, gx = np.gradient(d)
        lit = smooth(-0.3, 0.3, (-gx - gy) / 1.4142)
        e = aa(d) * (1 - aa(d + 2))
        self.mix(sl, e * lit, hx(0xe8dce2)); self.mix(sl, e * (1 - lit), hx(0x6a5a62))

    def pointer(self, x, y):
        """The menus' cursor (the game's own arrow, in white)."""
        sl, xx, yy = self.win(x - 2, y - 8, x + 14, y + 8)
        d = sdf_poly(xx, yy, [(x, y - 6), (x + 10, y), (x, y + 6)])
        self.mix(sl, aa(d - 1.5), hx(0x101010)); self.mix(sl, aa(d), hx(0xffffff))

    def itembox(self, x, y):
        """The little held-item mark (units)."""
        px, py = int(x * 2), int(y * 2)
        self.a[py + 3:py + 17, px:px + 18] = hx(0x402800)
        self.a[py + 5:py + 15, px + 2:px + 16] = hx(0xf0a818)
        self.a[py + 8:py + 10, px + 2:px + 16] = hx(0x805000)
        self.a[py + 8:py + 12, px + 7:px + 11] = hx(0xfff0a0)


def pink_sky(c):
    """The LITTLE CUP's backdrop in the recording is the cup's own pink; ours takes each cup's colour. Here the
    POKé CUP's sky, as before."""
    c.sky()


def entry_top(c, team, ask=None):
    c.sky()
    c.ribbon(8, 'REGISTER A TEAM', 'POKé CUP')
    sx0, sy0, sx1, sy1 = c.slab(12, 42, 468, 176, 'blue')
    c.tag(8, 31, 250)
    c.text(40, 11, 'BRENDAN', SMALL, ow=2, outline=hx(0x101830), bold=True)
    c.text(232, 11, '%d / 6' % len(team), SMALL, ow=2, outline=hx(0x101830), align='r')
    cw, ch = (sx1 - sx0) / 3, (sy1 - sy0) / 2
    cells = [(int(sx0 + (i % 3) * cw), int(sy0 + (i // 3) * ch), int(sx0 + (i % 3 + 1) * cw), int(sy0 + (i // 3 + 1) * ch)) for i in range(6)]
    for k in (1, 2):
        c.engrave_v(cells[k][0], sy0 + 4, sy1 - 4)
    c.engrave_h(cells[3][1], sx0 + 4, sx1 - 4)
    for i, (sp, nm, lv, item) in enumerate(team):
        x0, y0, x1, y1 = cells[i]
        c.clip_blit(scale(icon(sp), 64, 64), x0 + 2, y0 - 14, (x0 + 2, y0 - 6, x1, y1))
        c.text(x0 / 2 + 31, y0 / 2 - 0.5, nm, SMALL, ow=2, grad=0xdfe4f0)
        c.text(x0 / 2 + 31, y0 / 2 + 10, 'L%d' % lv, SMALL, ow=2, grad=0xdfe4f0)
        if item:
            c.itembox(x0 / 2 + 4, y0 / 2 + 20.5)
            c.text(x0 / 2 + 15, y0 / 2 + 20.5, item, SMALL, ow=2, grad=0xdfe4f0)
    if ask and len(team) < 6:
        x0, y0, x1, y1 = cells[len(team)]
        c.brackets(x0 + 8, y0 + 7, x1 - 6, y1 - 6, arm=10, th=4)
        c.text((x0 + x1) / 4 + 1, (y0 + y1) / 4 - 6.5, ask, SMALL, YELLOW, ow=2, outline=hx(YELLOW_INK), align='c', bold=True)
    return cells


TEAM4 = [('ursaring', 'URSARING', 50, 'LEFTOVERS'), ('politoed', 'POLITOED', 50, 'MYSTIC WATER'), ('kadabra', 'KADABRA', 50, None),
         ('hitmonlee', 'HITMONLEE', 50, 'BLACK BELT')]
TEAM6 = TEAM4 + [('espeon', 'ESPEON', 50, None), ('dragonair', 'DRAGONAIR', 50, 'DRAGON FANG')]
NAMES = ['EKANS', 'ELECTABUZZ', 'ELECTRODE', 'ELEKID', 'ENTEI', 'ESPEON', 'EXEGGCUTE', 'EXEGGUTOR', "FARFETCH'D"]
FOLDERS = ['ekans', 'electabuzz', 'electrode', 'elekid', 'entei', 'espeon', 'exeggcute', 'exeggutor', 'farfetchd']


def tabs(c, y=181):
    c.key(10, y, 34, y + 20, 'L')
    x = 40
    for name in ('PARTY', 'BOXES', 'SHARED', 'GLOBAL', 'RENTALS'):
        wpx = (measure(name, SMALL) + 9) * 2
        c.key(x, y, x + wpx, y + 20, name, selected=name == 'RENTALS')
        x += wpx + (8 if name in ('GLOBAL', 'RENTALS') else 4)
    c.key(446, y, 470, y + 20, 'R')


def entry_list(c, cursor=5):
    bx0, by0, bx1, by1 = c.listpanel(8, 206, 472, 288)
    c.thumb(21, 228)
    cols, rows = 3, 3
    cw, ch = (bx1 - bx0) / cols, (by1 - by0) / rows
    for i in range(9):
        x0, y0 = bx0 + (i % cols) * cw, by0 + (i // cols) * ch
        c.cell(x0 + 1, y0 + 1, x0 + cw - 2, y0 + ch - 1)
    for i, (nm, fd) in enumerate(zip(NAMES, FOLDERS)):
        x0, y0 = bx0 + (i % cols) * cw, by0 + (i // cols) * ch
        c.blit(icon(fd), x0 + 5, y0 + ch / 2 - 19)
        c.text((x0 + 38) / 2, (y0 + ch / 2) / 2 - 6.5, nm, SMALL, ow=2, outline=NAVY)
    if cursor is not None:
        x0, y0 = bx0 + (cursor % cols) * cw, by0 + (cursor // cols) * ch
        c.brackets(x0 + 1, y0 + 1, x0 + cw - 2, y0 + ch - 1, arm=8)


def entry_screen():
    c = C4()
    entry_top(c, TEAM4, 'Fifth POKéMON')
    tabs(c)
    entry_list(c)
    footer(c, [(('A',), 'LOOK'), (('SELECT',), 'SUMMARY'), (('L', 'R'), 'FROM'), (('START',), 'TEAM'), (('B',), 'DONE')])
    return c


def entry_info_screen():
    """A on a POKéMON: the list gives way to its panel (what today's detail card shows) and ENTER Yes / No."""
    c = C4()
    entry_top(c, TEAM4, 'Fifth POKéMON')
    c.bluepanel(8, 180, 386, 288)
    t = lambda x, y, s, col=0xffffff, **k: c.text(x, y, s, SMALL, col, ow=2, outline=NAVY, **k)
    L = [90.5, 100.3, 110.1, 119.9, 129.7]
    t(9, L[0], 'ESPEON'); t(50, L[0], 'L50'); t(76, L[0], 'No.196')
    w = c.window(18, 208, 80, 250, band=0x9ab0ff)
    sprite_in(c, w, front('espeon'))
    t(46, L[1], 'HP'); t(84, L[1], '141', align='r')
    wd = c.typepill(46, L[2] + 2, 'PSYCHIC'); chip(c, 48 + wd, L[2] + 2, 'RENTAL', 0x4878c0)
    for i, (nm, v) in enumerate((('ATTACK', 85), ('DEFENSE', 80), ('SP. ATK', 150), ('SP. DEF', 115), ('SPEED', 130))):
        t(134, L[i], nm, 0xc8d0ff); t(188, L[i], str(v), align='r')
    for i, (mv, ty) in enumerate((('PSYCHIC', 'PSYCHIC'), ('SHADOW BALL', 'GHOST'), ('MORNING SUN', 'NORMAL'), ('REFLECT', 'PSYCHIC'))):
        x, y = 9 + (i % 2) * 62, L[3 + i // 2]
        c.sphere(x * 2 + 5, y * 2 + 13, 4.5, TYPE_RGB[ty])
        t(x + 6, y, mv)
    # ENTER, and what it holds
    c.bluepanel(392, 180, 472, 250)
    t(216, 92, 'ENTER', align='c')
    c.pointer(402, 221)
    t(209, 104, 'Yes', YELLOW); t(209, 114.5, 'No')
    c.itembox(198, 129.5); t(209, 129.5, 'NONE', 0xffffff)
    footer(c, [(('A',), 'CHOOSE'), (('SELECT',), 'SUMMARY'), (('B',), 'BACK')])
    return c


def entry_menu_screen():
    """Six in: the confirm menu over the list."""
    c = C4()
    entry_top(c, TEAM6)
    tabs(c)
    entry_list(c, cursor=None)
    c.bluepanel(104, 190, 376, 282, color=(0x7c98f4, 0x6c88ec))
    rows = ('OK to Register', 'Reselect some POKéMON', 'Reselect all POKéMON')
    for i, s in enumerate(rows):
        c.text(76, 100 + i * 12.5, s, SMALL, YELLOW if i == 0 else 0xffffff, ow=2, outline=NAVY, bold=True)
    c.pointer(130, 215)
    footer(c, [(('A',), 'CHOOSE'), (('B',), 'BACK')])
    return c


# ---------------------------------------------------------------------------------------------------------------
def ladder(c, y0, tiles_top=True, bars=True, tiles_bottom=True, face='school_kid_m'):
    names = ['BATTLE 2', 'BATTLE 3', 'BATTLE 4', 'BATTLE 5', 'BATTLE 6', 'SEMIFINAL', 'FINAL']
    y = y0
    if tiles_top:
        c.plate(30, y, 450, y + 82)
        w = c.window(38, y + 8, 136, y + 74, glow=False)
        sprite_in(c, w, trainer(face))
        if bars:
            c.a[w[1] - 1:w[3] + 1, w[0] - 1:w[0] + 1] = hx(0x30e0e0); c.a[w[1] - 1:w[3] + 1, w[2] - 1:w[2] + 1] = hx(0x30e0e0)
            c.a[w[1] - 1:w[1] + 1, w[0] - 1:w[2] + 1] = hx(0x30e0e0); c.a[w[3] - 1:w[3] + 1, w[0] - 1:w[2] + 1] = hx(0x30e0e0)
        for i in range(3):
            c.qtile(140 + i * 102, y + 8, 140 + i * 102 + 98, y + 74, names[i])
        y += 84
    if bars:
        for word in ('POKé CUP', 'BATTLE 1'):
            c.plate(30, y, 450, y + 26)
            c.ital(240, y + 13, word, 22)
            y += 28
    if tiles_bottom:
        c.plate(30, y, 450, y + 82)
        for i in range(4):
            c.qtile(38 + i * 102, y + 8, 38 + i * 102 + 98, y + 74, names[3 + i])


def ladder_screen():
    c = C4()
    c.night(((20, -40, 60), (22, 440, 520), (300, 430, 520), (302, -40, 40)))
    ladder(c, 34)
    return c


def line_screen():
    """The trainer speaks before the battle."""
    c = C4()
    c.night(((4, 300, 520), (148, -40, 40), (270, 0, 520), (300, -40, 150)))
    ladder(c, 92, bars=False, tiles_bottom=False)
    c.plate(104, 182, 376, 226)
    c.text(120, 96, "I won't hold back!", SMALL, ow=2, outline=hx(0x202020), align='c', bold=True)
    return c


def result_screen2():
    """After WIN / LOSE: the fainted crossed out, the loser's line under their plate."""
    c = C4()
    c.night(LINES)
    c.ribbon(8, 'POKé CUP', 'BATTLE 3')
    c.plate(14, 24, 466, 146)
    w = c.window(22, 32, 104, 114, glow=False)
    sprite_in(c, w, trainer('brendan'))
    label(c, 11, 58, 'BRENDAN')
    for i, (sp, hp) in enumerate((('ursaring', 0.86), ('hitmonlee', 1.0), ('dragonair', 1.0))):
        x0 = 112 + i * 118
        w = c.window(x0, 32, x0 + 112, 124)
        sprite_in(c, w, front(sp))
        c.hpbar(x0, 129, x0 + 112, 139, hp)
    c.plate(14, 150, 466, 268)
    for i, sp in enumerate(('starmie', 'umbreon', 'tyranitar')):
        x0 = 22 + i * 110
        w = c.window(x0, 158, x0 + 105, 248)
        sprite_in(c, w, front(sp), dim=0.8)
        c.cross(w)
        c.hpbar(x0, 252, x0 + 105, 262, 0.0)
    label(c, 176, 77, 'COOLTRAINER')
    label(c, 176, 87.5, 'MARTY')
    w = c.window(356, 200, 458, 262, glow=False)
    sprite_in(c, w, trainer('cooltrainer_m'), dy=3)
    c.plate(150, 274, 330, 304)
    c.text(120, 138, 'You got me...', SMALL, ow=2, outline=hx(0x202020), align='c', bold=True)
    return c


def badge_screen():
    """The GYM LEADER CASTLE's badge board: the badge just won set into its place."""
    c = C4()
    c.night(((3, 150, 520), (150, -40, 30), (312, -40, 60)))
    badges = [('BOULDER', 0xb8a898), ('CASCADE', 0x60a8f0), ('THUNDER', 0xf0c030), ('RAINBOW', 0x70c868),
              ('SOUL', 0xe878b8), ('MARSH', 0xe0c040), ('VOLCANO', 0xe85038), ('EARTH', 0x48a870)]
    for r in range(2):
        y = 22 + r * 138
        c.plate(30, y, 450, y + 134)
        for k in range(4):
            i = r * 4 + k
            name, col = badges[i]
            cx, cy = 86 + k * 104, y + 50
            sl, xx, yy = c.win(cx - 36, cy - 36, cx + 36, cy + 36)
            e = np.hypot((xx - cx) / 30.0, (yy - cy) / 30.0)
            gy, gx = np.gradient(e)
            lit = smooth(-0.01, 0.01, -gx - gy)
            ring = aa((e - 1) * 30) * (1 - aa((e - 0.84) * 30))
            c.mix(sl, aa((e - 1) * 30 - 1), hx(0x505050))
            c.mix(sl, ring, np.repeat((0.55 + 0.42 * (1 - lit))[..., None], 3, -1))
            c.mix(sl, aa((e - 0.84) * 30), np.repeat((0.42 + 0.20 * lit)[..., None], 3, -1))
            if i == 0:      # won: the badge sits in its socket
                c.sphere(cx, cy, 22, col)
                c.sphere(cx, cy, 12, 0xe8e0d0)
            for sx, sy in ((-1, -1), (1, -1), (-1, 1), (1, 1)):     # the corner marks
                bx, by = int(cx + sx * 36), int(cy + sy * 36)
                c.a[min(by, by - sy * 8):max(by, by - sy * 8) + 1, bx - 1:bx + 1] = 0.94
                c.a[by - 1:by + 1, min(bx, bx - sx * 8):max(bx, bx - sx * 8) + 1] = 0.94
            x0, x1 = 38 + k * 104, 38 + k * 104 + 96
            c.a[y + 100:y + 124, x0:x1] = 0.02
            u = np.linspace(0, 1, x1 - x0 - 4)
            g = hx(col)[None] * (0.75 + 0.5 * np.exp(-((u - 0.5) / 0.3) ** 2))[:, None] * 0.6 + 0.42
            c.a[y + 102:y + 122, x0 + 2:x1 - 2] = np.clip(g, 0, 1)[None]
            c.text((x0 + x1) / 4, (y + 100) / 2 - 0.5, name, SMALL, int(col) , ow=2, outline=hx(0x181818), align='c')
    return c


def continue_screen():
    c = C4()
    c.night(((60, -40, 520), (166, -40, 130), (250, -40, 520)))
    c.cyanbox(176, 74, 304, 130)
    c.text(120, 39, 'POKé CUP', SMALL, ow=2, outline=hx(0x204040), align='c', bold=True)
    c.text(120, 50, 'BATTLE 3', SMALL, ow=2, outline=hx(0x204040), align='c', bold=True)
    c.cyanbox(120, 140, 360, 244)
    c.text(120, 72, 'Keep battling?', SMALL, ow=2, outline=hx(0x204040), align='c', bold=True)
    c.pointer(186, 185)
    c.text(102, 86, 'Continue', SMALL, 0xf8f020, ow=2, outline=hx(0x204040), bold=True)
    c.text(102, 97, 'Suspend', SMALL, ow=2, outline=hx(0x204040), bold=True)
    c.text(176, 108.5, 'CONTINUES 2', SMALL, ow=2, outline=hx(0x204040), align='r', bold=True)
    return c


if __name__ == '__main__':
    made = []
    for name, fn in (('entry', entry_screen), ('entry_info', entry_info_screen), ('entry_menu', entry_menu_screen), ('ladder', ladder_screen),
                     ('line', line_screen), ('result2', result_screen2), ('badges', badge_screen), ('continue', continue_screen)):
        im = fn().image()
        im.save(f'{OUT}/s4_{name}.png')
        made.append(im)
        print(name)
    sheet = Image.new('RGB', (480 * 2 + 8, 320 * 4 + 24), (40, 40, 48))
    for i, im in enumerate(made):
        sheet.paste(im, ((i % 2) * 488, (i // 2) * 328))
    sheet.save(f'{OUT}/s4_sheet.png')
