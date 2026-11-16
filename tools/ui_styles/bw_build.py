"""BLACK as the game draws it (uipack.py): bw.py's pieces made into a pack's fonts, pictures and specs, from the user's
Black ROM. Every spec puts things where bw.py's mock-ups put them (approved 2026-11-16); check_specs.py draws both and
compares them. The pack is never committed: the release builds it (tools/unova/make_pack.sh), as UNOVA's own.

Black's battle gauges carry their words in their own tiles and stand differently per spot on the screen (the bar runs
in from the screen's edge, shortened where the name or level would go off it), so each spot has its own spec, each
name-width class (Black moves a name over 48 pixels left) and level-digit count its own pieces."""
from PIL import Image

import bw as BW
import dsfont
import layout as Lo
import uipack as U
from uipack import grey
from bw import Black, _lines, _rebuild, rgb
from gb_build import diff
from pt_build import kai_font, kai_glyph

TYPES = ['NORMAL', 'FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', 'NORMAL', 'FIRE', 'WATER',
         'GRASS', 'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK', 'FAIRY']     # (KAI's TYPE_* order; MYSTERY: NORMAL)
POS = {'foe2': (1, 3), 'mine2': (0, 2)}     # (B_POSITION_*: Lo.DOUBLE_FOES / DOUBLE_MINE's two)


class Blank(int):
    """a number the gauge prints as nothing (its tiles left blank)"""
    def __str__(self):
        return ''


def carve_keep(im, axis, n, lo=0, hi=None):
    """bw.carve, and the lines it kept"""
    lines = _lines(im, axis)
    keep = list(range(len(lines)))
    hi = len(lines) if hi is None else hi
    while n > 0:
        idx = [i for i in keep if lo <= i < hi]
        best = None
        for ia in range(len(idx)):
            for ib in range(ia + 1, min(len(idx), ia + n + 1)):
                a, b = idx[ia], idx[ib]
                if lines[a] == lines[b]:
                    key = (0, 0, -(ib - ia), abs((a + b) / 2 - (lo + hi) / 2))
                    if best is None or key < best[0]:
                        best = (key, ia, ib)
        if best is None:
            for length in range(n, 0, -1):
                for ia in range(len(idx)):
                    ib = ia + length
                    if ib >= len(idx):
                        continue
                    a, b = idx[ia], idx[ib]
                    d = sum(1 for u, v in zip(lines[a], lines[b]) if u != v)
                    key = (1, d, 0, abs((a + b) / 2 - (lo + hi) / 2))
                    if best is None or key < best[0]:
                        best = (key, ia, ib)
                if best is not None:
                    break
        _, ia, ib = best
        cut = set(idx[ia:ib])
        keep = [i for i in keep if i not in cut]
        n -= len(cut)
    return _rebuild(im, axis, keep), keep


def moved(keep, x):
    """where line X of a picture is after it was carved down to KEEP (the next line kept, for one that went)"""
    for k, v in enumerate(keep):
        if v >= x:
            return k
    return len(keep)


def fairy_icon(psychic, write):
    """FAIRY's icon for a game that has none (Gen 4 / 5): its PSYCHIC icon's shape, its colours turned toward Gen 6's
    FAIRY pink, the word written again in the icon's own letters (WRITE(im, centre x, y, word, ink, shadow))"""
    im = psychic.copy()
    q = im.load()
    letters = set()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = q[x, y]
            if a and r >= 232 and g >= 232 and b >= 232:
                letters.add((x, y))
    ground = {}
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = q[x, y]
            if a and (x, y) not in letters:
                ground[(x, y)] = ((r + 255) // 2, (g * 3 + 120) // 4, (b * 3 + 255) // 4, a)
    # (the old word's pixels and their shadow take the ground's own colour from the row)
    for (x, y) in list(letters):
        row = [ground[(xx, y)] for xx in range(im.width) if (xx, y) in ground]
        q[x, y] = max(set(row), key=row.count) if row else (232, 136, 200, 255)
    for (x, y), c in ground.items():
        q[x, y] = c
    shade = sorted(ground.values(), key=lambda c: sum(c[:3]))
    t = Image.new('RGBA', im.size, (0, 0, 0, 0))
    write(t, im.width // 2, 0, 'FAIRY', (248, 248, 248), shade[len(shade) // 4][:3])
    bb = t.getbbox()
    if bb:
        lb = [p for p in letters]
        ly = min(p[1] for p in lb) if lb else (im.height - (bb[3] - bb[1])) // 2
        im.alpha_composite(t.crop(bb), ((im.width - (bb[2] - bb[0])) // 2, ly))
    return im


def recolour_icon(icon, col):
    """ICON's colours turned to COL's by their lightness (a type icon in another type's colours)"""
    im = icon.copy()
    q = im.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = q[x, y]
            if not a:
                continue
            l = (r + g + b) / (3 * 255.0)
            q[x, y] = tuple(min(255, int(c * (0.35 + 0.9 * l)) if l < 0.9 else 248) for c in col) + (a,)
    return im


class BWPack:
    def __init__(self, rom):
        self.st = Black(rom, None, None)
        self.p = U.Pack('BLACK')

    # ---------------------------------------------------------------------------------------------------- fonts
    def nftr(self, name, font):
        f = U.Font(font.height)
        sheet, widths = kai_font('normal')
        for code, ch in sorted(U.kai_codes().items()):
            if code in (U.FIELD, U.EOS, U.NEWLINE):
                continue
            if ord(ch) in font.map:
                rows, adv = font.glyph(ch)
                pix = [[v if v in (1, 2) else 0 for v in r] for r in rows]
            else:
                g = kai_glyph(sheet, widths, code)
                if g is None:
                    continue
                pix, adv = g
            f.add(code, adv, 0, 0, pix)
        return self.p.font(name, f)

    def fonts(self):
        st = self.st
        self.fm = self.nftr('bw.main', st.fmain)
        self.fs = self.nftr('bw.sub', st.fsub)
        P = st.gauge_parts()['pal']
        self.gink, self.gsh = rgb(P[1]), rgb(P[4])
        # the gauge's HP digits (a/0/1/1 0xA4's digit tiles: colours 1, 13, 14), as printed on the player's gauge
        f = U.Font(10)
        for d in range(10):
            g = self.gauge(0, 0, hp=d, maxhp=Blank(10 ** 9))     # (the bar left empty: only the digit changes)
            z = self.gauge(0, 0)
            a, b = g.load(), z.load()
            pix = []
            for y in range(18, 28):
                row = []
                for x in range(72, 80):
                    c = a[x, y]
                    row.append(0 if c == b[x, y] else 1 if c[:3] == rgb(P[1]) else 2 if c[:3] == rgb(P[13]) else 3)
                pix.append(row)
            f.add(0xA1 + d, 8, 0, 0, pix)
        f.add(0x00, 8, 0, 0, [])
        self.fdig = self.p.font('bw.digits', f)
        self.dig = (rgb(P[1]), rgb(P[13]), rgb(P[14]))

    # ---------------------------------------------------------------------------------------------------- helpers
    def img(self, sp, name, im, x=0, y=0, **k):
        sp.add(U.OP_IMG, x=x, y=y, a=self.p.image(name, im), **k)

    def text(self, sp, s, x, y, font, ink, shadow=None, outline=None, **k):
        sp.add(U.OP_TEXT, x=x, y=y, a=self.p.string(s), font=font, ink=ink, shadow=shadow, outline=outline, **k)

    def gauge(self, side, layout, **m):
        """bw.py's gauge with M's things (and the rest blank)"""
        base = dict(name='', sex=None, level=Blank(1), hp=Blank(0), maxhp=Blank(1), exp=0.0)
        base.update(m)
        return self.st.gauge(side, layout, base, numbers=(side == 0 and layout == 0), name=True)

    def tier_gauges(self, side, layout, **m):
        """the gauge full in each colour (Black's green / yellow / red parts in the green's place)"""
        g = self.st.gauge_parts()
        parts = g['parts']
        saved = list(parts)
        out = []
        for c in range(3):
            for k in range(9):
                parts[k] = saved[(c * 0x120) // 32 + k]
            out.append(self.gauge(side, layout, hp=Blank(1), maxhp=Blank(1), **m))
        parts[:] = saved
        return out

    # ---------------------------------------------------------------------------------------------------- the gauges
    def huds(self):
        for kind, rects in (('foe1', [Lo.SINGLE_FOE]), ('mine1', [Lo.SINGLE_MINE]), ('foe2', Lo.DOUBLE_FOES),
                            ('mine2', Lo.DOUBLE_MINE)):
            for k, r in enumerate(rects):
                name = 'hud.' + kind + ('.%d' % POS[kind][k] if kind in POS else '')
                self.p.spec(self.hud(name, kind, r))
        for kind, r in (('foe3', Lo.TRIPLE_FOES[0]), ('mine3', Lo.TRIPLE_MINE[0])):
            self.p.spec(self.card('hud.' + kind, kind == 'foe3', r))

    def hud(self, name, kind, r):
        """bw.py hud(): the gauge put where KAI's box is, shortened as the name and level need"""
        st, p = self.st, self.p
        x0, y0, x1, y1 = r
        sp = U.Spec(name, x1 - x0, y1 - y0)
        foe = kind.startswith('foe')
        side = 1 if foe else 0
        G0 = self.gauge(side, 0)
        FULL = self.tier_gauges(side, 0)
        E1 = self.gauge(side, 0, exp=1.0, level=Blank(1)) if not foe else None    # (Black's doubles show it too)
        GEN = {s: self.gauge(side, 0, sex=s) for s in ('m', 'f')}
        # (where the gauge stands goes by its own extent with its level printed, and its gender mark, which reaches
        # higher, as bw.py measures it; a double's box too tall for KAI's loses rows under its bar)
        Y = {}
        for sx in (None, 'm', 'f'):
            lvl = self.gauge(side, 0, level=1, sex=sx)
            bb = lvl.getbbox()
            ycut = None
            if kind == 'mine2' and bb[3] - bb[1] > y1 - y0:
                _, ycut = carve_keep(lvl, 'y', bb[3] - bb[1] - (y1 - y0), 22, 31)
                bb = _rebuild(lvl, 'y', ycut).getbbox()

            def ycarve(im, ycut=ycut):
                return _rebuild(im, 'y', ycut) if ycut else im
            Y[sx] = dict(fy=((y1 - y0) - (bb[3] - bb[1])) // 2 - bb[1], g0=ycarve(G0),
                         full=[ycarve(f) for f in FULL], e1=ycarve(E1) if E1 is not None else None,
                         mark=ycarve(GEN[sx]) if sx else None, ny=moved(ycut, 5) if ycut else 5)
        digits_w = {1: dsfont.width(st.fsub, '0'), 2: max(dsfont.width(st.fsub, '%d' % (10 * a + 8)) for a in
                                                        range(1, 10)),
                    3: dsfont.width(st.fsub, '100')}
        lvc = {1: (U.C_LV1, 0), 2: (0, U.C_LV1 | U.C_LV3), 3: (U.C_LV3, 0)}
        sp.add(U.OP_LONG, a=U.F_NAME, font=self.fs, w=48)
        gcs = {None: (0, U.C_MALE | U.C_FEMALE), 'm': (U.C_MALE, 0), 'f': (U.C_FEMALE, 0)}
        # (a wild foe's caught ball, BTLV_GAUGE_DrawCaughtIcon's: left of the name, its own variants of the box)
        ball = None
        if foe:
            plain = self.gauge(side, 0)
            ball = diff(self.st.gauge(side, 0, dict(name='', sex=None, level=Blank(1), hp=Blank(0), maxhp=Blank(1),
                                                    exp=0.0, caught=True), numbers=False, name=True), plain)
        for (long, nd, sx, caught) in [(lg, n, g, c) for lg in (False, True) for n in (1, 2, 3)
                                       for g in (None, 'm', 'f') for c in ((False, True) if foe else (False,))]:
            nxw = 2 if long else 8
            if True:
                V = Y[sx]
                fy, g0, g0u, full, e1, ny = V['fy'], V['g0'], V['g0'], V['full'], V['e1'], V['ny']
                cond = lvc[nd][0] | (U.C_LONG if long else 0) | gcs[sx][0] | (U.C_CAUGHT if caught else 0)
                unless = lvc[nd][1] | (0 if long else U.C_LONG) | gcs[sx][1] | (U.C_CAUGHT if foe and not caught
                                                                                  else 0)
                # (bw.py: where the frame stands, and what's carved, for a name this wide and a level this long)
                keep = None
                if foe:
                    fx = x1 - 128
                    short = 2 - (fx + 16 + nxw)
                    if short > 0:
                        n = min(short, 7)
                        _, keep = carve_keep(g0, 'x', n, 112, 120)
                        fx = x1 - (128 - n) + max(0, short - n)
                else:
                    fx = x0
                    over = fx + 88 + digits_w[nd] - (240 - 2)
                    if over > 0:
                        free = 10 if long else 16
                        n = min(over, free - 6)
                        _, keep = carve_keep(g0, 'x', n, 6, free)
                        fx -= max(0, over - n)
                if caught:
                    # (the ball at the gauge's x 8; where that's off the screen the box moves in to show it)
                    bx = fx + (moved(keep, 8) if keep else 8)
                    fx += max(0, 1 - bx)
                ox, oy = fx - x0, fy

                def X(x):
                    return ox + (moved(keep, x) if keep else x)

                def cut(im):
                    return _rebuild(im, 'x', keep) if keep else im
                tag = '%s.%d%s%s%s' % (name, nd, 'L' if long else '', sx or '', 'c' if caught else '')
                frame = cut(g0)
                self.img(sp, tag + '.frame', frame, ox, oy, cond=cond, unless=unless)
                empty = cut(g0u)
                fills = [p.image('%s.fill.%s' % (tag, t), diff(cut(f), empty)) for t, f in zip('gyr', full)]
                sp.add(U.OP_BAR, x=ox, y=oy, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2], cond=cond,
                       unless=unless)
                if e1 is not None:
                    e = p.image(tag + '.exp', diff(cut(e1), empty))
                    sp.add(U.OP_BAR, x=ox, y=oy, a=U.F_EXP, b=e, c=e, d=e, cond=cond | U.C_EXP, unless=unless)
                if sx:
                    self.img(sp, tag + '.mark', diff(cut(V['mark']), empty), ox, oy, cond=cond, unless=unless)
                if caught:
                    bb = ball.getbbox()
                    self.img(sp, name + '.ball', ball.crop(bb), X(bb[0]), oy + bb[1], cond=cond,
                             unless=unless)
                nx = (16 if foe else 8) + nxw
                self.text(sp, '{%d}' % U.F_NAME, X(nx), oy + ny, self.fs, self.gink, self.gsh, cond=cond,
                          unless=unless)
                lx = 96 if foe else 88
                self.text(sp, '{%d}' % U.F_LEVEL, X(lx), oy + ny, self.fs, self.gink, self.gsh, cond=cond,
                          unless=unless)
                if kind == 'mine1':
                    d1, d2, d3 = self.dig
                    self.text(sp, '{%d}' % U.F_HP3, X(56), oy + 18, self.fdig, d1, d2, d3, cond=cond, unless=unless)
                    self.text(sp, '{%d}' % U.F_MAXHP, X(88), oy + 18, self.fdig, d1, d2, d3, cond=cond,
                              unless=unless)
        return sp

    def card(self, name, foe, r):
        """bw.py triple_card(): the card's bar (its pieces and the gauge shortened to the room the card has) filled
        as far as the HP goes, the name row floating, the Lv icon and digits before the bar"""
        st, p = self.st, self.p
        x0, y0, x1, y1 = r
        w, h = x1 - x0, y1 - y0
        sp = U.Spec(name, w, h)
        blank = dict(name='MEW', sex=None, level=1, hp=0, maxhp=1)
        saved, saved_log = st.draw, st.log
        st.draw = lambda im, font, x, y, s, *a, **k: x + dsfont.width(font, s)
        st.log = lambda *a, **k: st.inks.append(((0, 0, 0, 0), ''))
        try:
            pic, _, _ = st.triple_card(foe, w, h, blank)
            fills = []
            P = st.gauge_parts()['parts']
            keep = list(P)
            for c in range(3):
                for k in range(9):
                    P[k] = keep[(c * 0x120) // 32 + k]
                full, _, _ = st.triple_card(foe, w, h, dict(blank, hp=1))
                fills.append(p.image('%s.fill.%s' % (name, 'gyr'[c]), diff(full, pic)))
            P[:] = keep
            genders = {s: diff(st.triple_card(foe, w, h, dict(blank, sex=s))[0], pic) for s in ('m', 'f')}
        finally:
            st.draw, st.log = saved, saved_log
        self.img(sp, name + '.card', pic)
        sp.add(U.OP_BAR, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
        # (bw.py: the name row from (1, by - 12), the level after the Lv icon on the bar's white rows)
        bar_h = 11
        by = h - bar_h + 1
        if foe:
            # (a wild foe's caught ball before the name, as Black's gauge has it; the name after it)
            plain = st.gauge(1, 2, dict(blank, name=''), numbers=False, name=True)
            ball = diff(st.gauge(1, 2, dict(blank, name='', caught=True), numbers=False, name=True), plain)
            bb = ball.getbbox()
            ball = ball.crop(bb)
            self.img(sp, name + '.ball', ball, 1, by - 12 + 4, cond=U.C_CAUGHT)
            self.text(sp, '{%d}' % U.F_NAME, 1 + ball.width + 2, by - 12, self.fs, self.gink, self.gsh, reg=1,
                      cond=U.C_CAUGHT)
        self.text(sp, '{%d}' % U.F_NAME, 1, by - 12, self.fs, self.gink, self.gsh, reg=1,
                  unless=U.C_CAUGHT if foe else 0)
        mew_end = 1 + dsfont.width(st.fsub, 'MEW')
        for s, gc in (('m', U.C_MALE), ('f', U.C_FEMALE)):
            gpic = genders[s]
            gb = gpic.getbbox()
            sp.add(U.OP_IMG, x=gb[0] - mew_end, y=gb[1], a=p.image('%s.%s' % (name, s), gpic.crop(gb)), rel=True,
                   reg=1, cond=gc)
        lv_box = (86, 6, 98, 16) if foe else (79, 6, 91, 16)
        g = st.gauge(1 if foe else 0, 2, dict(blank), numbers=False, name=False)
        lv = g.crop(lv_box)
        lv = lv.crop(lv.getbbox())
        ly = by + 3
        self.text(sp, '{%d}' % U.F_LEVEL, 1 + lv.width + 1, ly - 3, self.fs, self.gink, self.gsh, h=h - (ly - 3))
        return sp

    # ---------------------------------------------------------------------------------------------------- panels
    def msg_layer(self, r):
        x0, y0, x1, y1 = r
        return self.st.msg_window(x1 - x0, y1 - y0)

    def panels(self):
        st, p = self.st, self.p
        ink, sh = st.MSG_INK
        sp = U.Spec('b.msg')
        sp.add(U.OP_BLEND, x=0, y=112, a=p.image('bw.msgwin', self.msg_layer((0, 112, 240, 160))), b=7)
        sp.add(U.OP_MSG, x=8, y=120, a=0, h=16, n=2, w=224, font=self.fm, ink=ink, shadow=sh, evb=7)
        p.spec(sp)
        sp = U.Spec('b.cmd')
        sp.add(U.OP_BLEND, x=0, y=112, a=p.image('bw.prompt', self.msg_layer(Lo.BATTLE_MSG)), b=7)
        sp.add(U.OP_MSG, x=8, y=120, a=1, h=16, n=2, w=104, e=self.fs + 1, font=self.fm, ink=ink, shadow=sh, evb=7)
        self.command(sp)
        p.spec(sp)
        self.moves()
        self.field_msg()
        self.start_menus()

    def command(self, sp):
        """bw.py battle_menu(): the POKé BALL's column, four mesh buttons, each command's own lettering (KAI's words
        in Black's letters where the commands aren't the usual four), the pink arcs on the chosen"""
        st, p = self.st, self.p
        x0, y0, x1, y1 = Lo.BATTLE_MENU
        w, h = x1 - x0, y1 - y0
        cv = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        saved = st.command_words, st.fight_word, st.arcs
        st.command_words = lambda: {k: Image.new('RGBA', (1, 1), (0, 0, 0, 0)) for k in ('BAG', 'RUN', 'MOéMON')}
        st.fight_word = lambda: Image.new('RGBA', (1, 1), (0, 0, 0, 0))
        st.arcs = lambda *a, **k: None
        try:
            st.battle_menu(cv)
        finally:
            st.command_words, st.fight_word, st.arcs = saved
        self.img(sp, 'bw.cmd', cv.crop((x0, y0, x1, y1)), x0, y0)
        words = dict(st.command_words(), FIGHT=st.fight_word())
        # (POKéMON as Black letters it, for OPTIONS > CREATURES POKéMON)
        t = st.touch()
        pts = [(x, y) for y in range(150, 192) for x in range(170, 256) if max(t.getpixel((x, y))[:3]) -
               min(t.getpixel((x, y))[:3]) >= 48]
        X0, X1 = min(q[0] for q in pts) - 1, max(q[0] for q in pts) + 2
        Y0, Y1 = min(q[1] for q in pts) - 1, max(q[1] for q in pts) + 2
        pk = Image.new('RGBA', (X1 - X0, Y1 - Y0), (0, 0, 0, 0))
        for y in range(Y1 - Y0):
            for x in range(X1 - X0):
                c = t.getpixel((X0 + x, Y0 + y))
                if c[:3] in BW.OUTLINE or max(c[:3]) - min(c[:3]) >= 48:
                    pk.putpixel((x, y), c)
        bh = 22
        cols = [(1, 74), (76, 43)]
        place = {'FIGHT': (0, 0, 0), 'BAG': (1, 0, 1), 'MOéMON': (0, 1, 2), 'RUN': (1, 1, 3)}
        ink, sh = st.MOVE_INK
        for key, (c, rr, field) in place.items():
            bx, bw_ = cols[c]
            by = 0 if rr == 0 else h - bh
            choices = [(words[key], U.C_STDCMD | (U.C_MOEWORD if key == 'MOéMON' else 0), 0)]
            if key == 'MOéMON':
                choices.append((pk, U.C_STDCMD, U.C_MOEWORD))
            for wd, cond, unless in choices:
                self.img(sp, 'bw.word.%s.%d' % (key, unless), wd, x0 + bx + (bw_ - wd.width) // 2,
                         y0 + by + (bh - wd.height + 1) // 2, cond=cond, unless=unless)
            self.text(sp, '{%d}' % (U.F_ITEM0 + field), x0 + bx + bw_ // 2, y0 + by + 5, self.fm, ink, sh, align=2,
                      unless=U.C_STDCMD)
        # (the chosen command: a pink ring round its button's own six-sided shape: bw.py command_rings)
        rings = st.command_rings(w, h)
        for key, (c, rr, field) in place.items():
            ring = rings[key]
            rb = ring.getbbox()
            self.img(sp, 'bw.arc.%s' % key, ring.crop(rb), x0 + rb[0], y0 + rb[1], cond=U.C_SEL0 << field)

    def type_button(self, kind, w, h):
        st = self.st
        if kind == 'FAIRY':
            im = st.move_button('PSYCHIC', w, h)
            q = im.load()
            for y in range(im.height):
                for x in range(im.width):
                    r, g, b, a = q[x, y]
                    if a and not (r == g == b):
                        q[x, y] = ((r + 255) // 2, (g * 3 + 255) // 4, (b * 3 + 255) // 4, a)
            return im
        return st.move_button(kind, w, h)

    def type_icon(self, kind):
        if kind == 'FAIRY':
            return fairy_icon(self.st.type_icon('PSYCHIC'), lambda im, x, y, t, ink, sh: self.st.draw(
                im, self.st.fsmall, x, y, t, ink, sh, 'c'))
        return self.st.type_icon(kind)

    def moves(self):
        """bw.py move_panel(): the ground's stripes, four type-coloured buttons with their names, the arcs on the
        chosen; beside them that move's button with its type's icon and PP"""
        st, p = self.st, self.p
        sp = U.Spec('b.moves')
        x0, y0, x1, y1 = Lo.MOVES
        P = st.pal(st.GAUGE, 0x5C)
        ground = Image.new('RGBA', (x1 - x0, y1 - y0))
        gp = ground.load()
        for y in range(ground.height):
            for x in range(ground.width):
                gp[x, y] = rgb(P[16 + (0 if (y // 2) % 2 else 11)]) + (255,)
        self.img(sp, 'bw.ground', ground, x0, y0)
        bw_, bh_ = 80, 24
        base = [p.image('bw.move.%d' % t, self.type_button(k, bw_, bh_)) for t, k in enumerate(TYPES)]
        assert base == list(range(base[0], base[0] + len(TYPES)))
        ink, sh = st.MOVE_INK
        for i in range(4):
            bx, by = x0 + (i % 2) * 80, y0 + (i // 2) * 24
            sp.add(U.OP_IMGSEL, x=bx, y=by, a=base[0], b=U.F_ITEM0 + 8 + i)
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), bx + bw_ // 2, by + 5, self.fs, ink, sh, align=2)
        # (the chosen move: the pink ring round its oval: bw.py move_rings)
        rings = st.move_rings(x1 - x0, y1 - y0, bw_, bh_)
        for i in range(4):
            ring = rings[i]
            rb = ring.getbbox()
            self.img(sp, 'bw.marc.%d' % i, ring.crop(rb), x0 + rb[0], y0 + rb[1], cond=U.C_SEL0 << i)
            # (the move held to swap with SELECT: its ring grey)
            self.img(sp, 'bw.marc.held.%d' % i, grey(ring.crop(rb)), x0 + rb[0], y0 + rb[1], cond=U.C_HELD0 << i,
                     unless=U.C_SEL0 << i)
        mx0, my0, mx1, my1 = Lo.MOVE_INFO
        self.img(sp, 'bw.ground.info', ground.crop((0, 0, mx1 - mx0, my1 - my0)), mx0, my0)
        info = [p.image('bw.info.%d' % t, self.type_button(k, mx1 - mx0, my1 - my0)) for t, k in enumerate(TYPES)]
        sp.add(U.OP_IMGSEL, x=mx0, y=my0, a=info[0], b=U.F_TYPE)
        icons = [self.type_icon(k) for k in TYPES]
        iw = icons[0].width
        ic = [p.image('bw.type.%d' % t, im) for t, im in enumerate(icons)]
        sp.add(U.OP_IMGSEL, x=mx0 + (mx1 - mx0 - iw) // 2, y=my0 + 7, a=ic[0], b=U.F_TYPE, unless=U.C_ALT)
        self.text(sp, 'PP', mx0 + 14, my0 + 25, self.fm, ink, sh, unless=U.C_ALT)
        self.text(sp, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), mx1 - 14, my0 + 25, self.fm, ink, sh, align=1,
                  unless=U.C_ALT)
        sp.add(U.OP_MSG, x=mx0 + 10, y=my0 + 8, a=10, h=14, n=2, font=self.fs, ink=ink, shadow=sh, cond=U.C_ALT)
        p.spec(sp)

    def field_msg(self):
        st, p = self.st, self.p
        sp = U.Spec('f.msg')
        x0, y0, x1, y1 = Lo.FIELD_MSG
        self.img(sp, 'bw.balloon', st.balloon(x1 - x0, y1 - y0), x0, y0)
        ink, sh = st.INK
        sp.add(U.OP_MSG, x=x0 + 10, y=y0 + 10, a=0, h=16, n=2, w=x1 - x0 - 20, e=self.fs + 1, font=self.fm, ink=ink,
               shadow=sh, reg=2)
        arrow = Image.new('RGBA', (8, 8), (0, 0, 0, 0))
        st.next_arrow(arrow, 0, 0)
        self.img(sp, 'bw.next', arrow, 2, 2, rel=True, rely=True, reg=2, cond=U.C_WAIT)
        p.spec(sp)

    def start_menus(self):
        """bw.py start_menu(): Black's X menu in KAI's column, a tab for each entry but the last, which (EXIT) is the
        bottom bar's return button; the column runs to the screen's foot for eight or more entries"""
        st, p = self.st, self.p
        A = st.XMENU
        x0, y0, x1, _ = Lo.START
        W_ = x1 - x0
        T = 'a/0/8/3'
        ret, _, _ = st.cell(T, 21, st.tiles(T, 20), st.pal(T, 19), 1)
        ret = ret.crop(ret.getbbox())
        ink, sh = st.XINK
        bg_full = st.screen(A, 2, 0x0E, 0).crop((0, 0, 256, 192))
        tab_src = st.screen(A, 2, 0x10, 0).crop((3, 24, 126, 67))
        lit_src, _, _ = st.cell(A, 28, st.tiles(A, 13), st.pal(A, 1))
        lit_src = lit_src.crop(lit_src.getbbox())
        for n in range(1, 11):
            sp = U.Spec('start.%d' % n)
            tabs = n - 1
            y1 = Lo.START[3] if tabs <= 6 else 160
            H_ = y1 - y0
            room = H_ - 4 - ret.height - 2
            th = min(20, room // max(1, tabs))
            bg = BW.carve(BW.carve(bg_full, 'x', bg_full.width - W_, 8, 248), 'y', bg_full.height - H_, 24, 160)
            self.img(sp, 'bw.xbg.%d' % H_, bg, x0, y0)
            tw = W_ - 4

            def cut(pic, under, th=th):
                pic = BW.widen(pic, 'x', 16, 10)
                keep = list(range(th - 3 - under)) + list(range(37, 40 + under))
                pic = _rebuild(pic, 'y', keep)
                return BW.carve(pic, 'x', pic.width - tw, 33 + 16 + th - 1, pic.width - 3)
            tab = p.image('bw.tab.%d' % th, cut(tab_src, 1))
            lit = p.image('bw.lit.%d' % th, cut(lit_src, 0))
            for i in range(tabs):
                bx, by = x0 + 2, y0 + 2 + i * th
                sp.add(U.OP_IMG, x=bx, y=by, a=tab, unless=U.C_SEL0 << i)
                sp.add(U.OP_IMG, x=bx, y=by, a=lit, cond=U.C_SEL0 << i)
                self.text(sp, '{%d}' % (U.F_ITEM0 + i), bx + 7, by + 3 - max(0, (20 - th) // 2), self.fm, ink, sh)
            ex = x1 - 3 - ret.width
            ry = y1 - 2 - ret.height
            self.img(sp, 'bw.ret', ret, ex, ry)
            self.text(sp, '{%d}' % (U.F_ITEM0 + tabs), ex - 3, y1 - 15, self.fm, ink, sh, align=1)
            arc = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
            st.arcs(arc, ex - 1, ry - 1, ex + ret.width + 1, ry + ret.height + 1, out=4)
            ab = arc.getbbox()
            if ab:
                self.img(sp, 'bw.retarc.%d' % ry, arc.crop(ab), ab[0], ab[1], cond=U.C_SEL0 << tabs)
            p.spec(sp)

    # ---------------------------------------------------------------------------------------------------- party
    def party(self):
        """bw.py party(): Black's party screen, its ground and six plates two by two (the chosen lit as Black's lead
        plate, an empty slot's own), each with Kai's icon, the name and gender, the HP bar filled in its colour, Lv
        and the HP (the status in the level's place, as Black shows it); the balloon with the message and the return
        button for CANCEL"""
        st, p = self.st, self.p
        ground = Image.new('RGBA', (240, 160), (0, 0, 0, 255))
        bg = st.screen(st.PARTY, 15, 20, 6).crop((0, 0, 256, 192))
        bg = BW.carve(BW.carve(bg, 'x', 16, 0, 256), 'y', 32, 0, 192)
        ground.alpha_composite(bg.convert('RGBA'), (0, 0))
        ground.alpha_composite(st.balloon(168, 24), (0, 136))
        T = 'a/0/8/3'
        ret, _, _ = st.cell(T, 21, st.tiles(T, 20), st.pal(T, 19), 1)
        ret = ret.crop(ret.getbbox())
        ground.alpha_composite(ret, (240 - 6 - ret.width, 140))
        sp = U.Spec('party')
        self.img(sp, 'bw.party.ground', ground)
        ink, sh = st.INK
        sp.add(U.OP_MSG, x=10, y=140, a=0, h=16, n=1, w=154, e=self.fs + 1, font=self.fm, ink=ink, shadow=sh)
        pi, ps = st.PLATE_INK
        self.text(sp, 'CANCEL', 240 - 10 - ret.width, 141, self.fm, pi, ps, align=1)
        arc = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
        rx = 240 - 6 - ret.width
        st.arcs(arc, rx - 1, 139, rx + ret.width + 1, 141 + ret.height, out=4)
        ab = arc.getbbox()
        self.img(sp, 'bw.party.cancel', arc.crop(ab), ab[0], ab[1], cond=U.C_SEL0)
        p.spec(sp)
        pw, ph = 120, 44
        empty_bar = st.party_hp(0.0)
        P = st.pal('a/0/8/3', 0)
        fills = []
        for t, colour in zip('gyr', ((5, 6), (9, 10), (7, 8))):
            im = Image.new('RGBA', empty_bar.size, (0, 0, 0, 0))
            for i in range(48):
                im.putpixel((16 + i, 3), BW.rgb(P[colour[0]]) + (255,))
                im.putpixel((16 + i, 4), BW.rgb(P[colour[1]]) + (255,))
            fills.append(p.image('bw.partybar.' + t, im))
        bar_id = p.image('bw.partybar', empty_bar)
        plates = {k: p.image('bw.plate.' + k, st.plate(k, pw, ph)) for k in ('lead', 'other', 'empty')}
        mi, fi = st.MALE_INK, st.FEMALE_INK
        for i in range(6):
            sp = U.Spec('party.%d' % i)
            x, y = (i % 2) * pw, (i // 2) * (ph + 1)
            off, no = U.P_EMPTY, U.P_EMPTY | U.P_EGG
            no_hp = no | U.P_DESC
            sp.add(U.OP_IMG, x=x, y=y, a=plates['empty'], cond=U.P_EMPTY)
            sp.add(U.OP_IMG, x=x, y=y, a=plates['other'], unless=U.P_EMPTY | U.C_SEL0 | U.P_SWITCH)
            sp.add(U.OP_IMG, x=x, y=y, a=plates['lead'], cond=U.C_SEL0, unless=U.P_EMPTY)
            sp.add(U.OP_IMG, x=x, y=y, a=plates['lead'], cond=U.P_SWITCH, unless=U.P_EMPTY | U.C_SEL0)
            sp.add(U.OP_ICON, x=x + 20, y=y + 20, a=0, unless=off)
            self.text(sp, '{%d}' % U.F_NAME, x + 38, y + 3, self.fm, pi, ps, unless=off)
            self.text(sp, '♂', x + 104, y + 3, self.fm, mi[0], mi[1], cond=U.C_MALE, unless=no)
            self.text(sp, '♀', x + 104, y + 3, self.fm, fi[0], fi[1], cond=U.C_FEMALE, unless=no)
            sp.add(U.OP_IMG, x=x + 42, y=y + 17, a=bar_id, unless=no_hp)
            sp.add(U.OP_BAR, x=x + 42, y=y + 17, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2], unless=no_hp)
            self.text(sp, 'Lv{%d}' % U.F_LEVEL, x + 8, y + 26, self.fs, pi, ps, unless=no | U.C_STATUS)
            sp.add(U.OP_ICON, x=x + 8 + 16, y=y + 26 + 6, a=2, cond=U.C_STATUS, unless=off)
            self.text(sp, '{%d}' % U.F_HP, x + 76, y + 26, self.fs, pi, ps, align=1, unless=no_hp)
            self.text(sp, '/', x + 77, y + 26, self.fs, pi, ps, unless=no_hp)
            self.text(sp, '{%d}' % U.F_MAXHP, x + 84, y + 26, self.fs, pi, ps, unless=no_hp)
            self.text(sp, '{%d}' % (U.F_TEXT0 + 1), x + 42, y + 24, self.fm, pi, ps, cond=U.P_DESC, unless=off)
            p.spec(sp)

    # ---------------------------------------------------------------------------------------------------- OPTIONS
    def options(self):
        """bw.py options_page(): Black's OPTIONS, KAI's tabs (or a page's title) on the title bar, a row on each of
        its six bars, the cursor's bar lit as Black lights it (palette 7), the name and the choices (the one on green)
        on the bar; a line about the row on the title bar's look turned over at the foot, five rows then"""
        from common import OPTION_TABS
        st, p = self.st, self.p
        green, gsh = (96, 248, 144), (8, 64, 48)
        white, wsh = (248, 248, 248), (48, 48, 64)
        dim = (152, 136, 216)
        ground = st.options_ground(None)
        sp = U.Spec('o')
        self.img(sp, 'bw.o.ground', ground)
        x = 4
        for i, t in enumerate(OPTION_TABS):
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, 11, self.fs, green, gsh, cond=U.C_SEL0 << i, unless=U.S_PAGE)
            self.text(sp, '{%d}' % (U.F_ITEM0 + i), x, 11, self.fs, (160, 160, 160), (40, 40, 40),
                      unless=U.S_PAGE | (U.C_SEL0 << i))
            x += st.measure(t, 's') + 7
        self.text(sp, '{%d}' % U.F_TITLE, 4, 11, self.fs, green, gsh, cond=U.S_PAGE)
        foot = Image.new('RGBA', (240, 26), (0, 0, 0, 255))
        foot.paste(ground.crop((0, 0, 240, 24)).transpose(Image.FLIP_TOP_BOTTOM), (0, 2))
        self.img(sp, 'bw.o.foot', foot, 0, 134, cond=U.S_HINT)
        self.text(sp, '{%d}' % U.F_HINT, 8, 139, self.fs, white, wsh, cond=U.S_HINT)
        # (more rows: small white wedges, at the title bar's end and under the last bar)
        wedge = Image.new('RGBA', (7, 4), (0, 0, 0, 0))
        for k in range(4):
            wedge.paste(white + (255,), (k, k, 7 - k, k + 1))
        self.img(sp, 'bw.o.down', wedge, 229, 156, cond=U.S_DOWN)
        self.img(sp, 'bw.o.up', wedge.transpose(Image.FLIP_TOP_BOTTOM), 229, 15, cond=U.S_UP)
        sp.add(U.OP_LIST, x=0, y=27, h=22, n=6, unless=U.S_HINT)
        sp.add(U.OP_LIST, x=0, y=27, h=22, n=5, cond=U.S_HINT)
        p.spec(sp)
        # the lit bars: each bar's own pixels lit, drawn under the row's words
        row = U.Spec('o.row')
        lit = st.options_ground(0)
        band = diff(lit.crop((0, 24, 240, 46)), ground.crop((0, 24, 240, 46)))
        self.img(row, 'bw.o.lit', band, 0, -3, cond=U.C_SEL0)
        self.text(row, '{%d}' % U.F_NAME, 8, 0, self.fs, white, wsh)
        self.text(row, '{%d}' % U.F_CURSTR, 232, 0, self.fs, green, gsh, align=1, cond=U.O_VALUE, unless=U.O_ALL)
        row.add(U.OP_CHOICES, x=112, y=0, w=120, n=8, a=U.F_CHOICE0, align=1, font=self.fs, ink=green, shadow=gsh,
                c=U.col(dim), d=U.col(wsh), cond=U.O_ALL)
        p.spec(row)

    # ---------------------------------------------------------------------------------------------------- BAG
    def bag(self):
        """bw.py bag_page(): Black's BAG ground, the pocket's name and its own tab, the eight pockets as marks
        between << and >>, four rows on Black's row plates (the count at the right, a registered item's button in
        its place, the touch bar's arrow at the cursor's), START: SORT, small wedges for more rows, the balloon with
        the item's icon and its words (closer when they take a fourth line)"""
        from bw import carve
        from common import POCKETS
        st, p = self.st, self.p
        A = st.BAG
        lab, lsh = (248, 248, 248), (64, 64, 96)
        ground = Image.new('RGBA', (240, 160), (0, 0, 0, 255))
        bg = st.screen(A, 3, 5, 4).crop((0, 0, 256, 168))
        bg = carve(carve(bg, 'x', 16, 0, 240), 'y', 8, 20, 150)
        BW.paste(ground, bg, 0, 0)
        T = 'a/0/8/3'
        for k, frame in ((0, 4), (1, 5)):
            ar, _, _ = st.cell(T, 21, st.tiles(T, 20), st.pal(T, 19), frame)
            ar = ar.crop(ar.getbbox())
            BW.paste(ground, ar, 4 if k == 0 else 100 - ar.width, st.BAG_PIC_Y - ar.height // 2)
        px = ground.load()
        n = len(POCKETS)
        for i in range(n):
            x = 52 - (n * 7) // 2 + i * 7
            for dy in range(4):
                for dx in range(4):
                    if dx in (0, 3) or dy in (0, 3):
                        px[x + dx, st.BAG_MARKS_Y + dy] = lab + (255,)
        st.text(ground, 232, 92, 'START: SORT', lab, lsh, 's', 'r')
        BW.paste(ground, st.balloon(240, 56), 0, 104)
        sp = U.Spec('bag')
        self.img(sp, 'bw.bag.ground', ground)
        self.text(sp, '{%d}' % U.F_TITLE, 52, 4, self.fm, lab, lsh, align=2)
        # (the pocket's picture, whole: HILBERT's bag, or HILDA's tote for a girl)
        for sex in (0, 1):
            first = None
            for i, name in enumerate(POCKETS):
                ic = st.pocket_icon(name, True, female=bool(sex))
                c = Image.new('RGBA', (64, 84), (0, 0, 0, 0))
                c.alpha_composite(ic, (32 - ic.width // 2, 42 - ic.height // 2))
                k = p.image('bw.bag.pocket%d.%d' % (sex, i), c)
                first = k if first is None else first
                assert k == first + i
            g = dict(cond=U.C_FEMALE) if sex else dict(unless=U.C_FEMALE)
            sp.add(U.OP_IMGSEL, x=52 - 32, y=st.BAG_PIC_Y - 42, a=first, b=U.F_CUR, **g)
        dot = Image.new('RGBA', (2, 2), lab + (255,))
        for i in range(n):
            self.img(sp, 'bw.bag.mark', dot, 52 - (n * 7) // 2 + i * 7 + 1, st.BAG_MARKS_Y + 1, cond=U.C_SEL0 << i)
        self.img(sp, 'bw.bag.down', st.wedge(), 112, 95, cond=U.S_DOWN)
        self.img(sp, 'bw.bag.up', st.wedge(True), 112, 1, cond=U.S_UP)
        sp.add(U.OP_ICON, x=22, y=132, a=1)
        sp.add(U.OP_MSG, x=38, y=109, a=0, h=15, n=3, w=196, b=11, c=4, d=108, font=self.fm, ink=st.INK[0],
               shadow=st.INK[1])
        sp.add(U.OP_LIST, x=0, y=6, h=21, n=4)
        p.spec(sp)
        plate = st.screen(A, 0x21, 0x23, 0x22)
        plate = plate.crop(plate.getbbox())
        plate = carve(carve(plate, 'y', plate.height - 20, 6, plate.height - 6), 'x', plate.width - 128, 20, 120)
        arrow, _, _ = st.cell(T, 21, st.tiles(T, 20), st.pal(T, 19), 5)
        arrow = arrow.crop(arrow.getbbox())
        row = U.Spec('bag.row')
        self.img(row, 'bw.bag.plate', plate, 108, 0)
        self.text(row, '{%d}' % U.F_NAME, 118, 2, self.fm, lab, (64, 64, 72), w=84, b=self.fs + 1)
        self.text(row, 'x{%d}' % U.F_QTY, 232, 2, self.fm, lab, (64, 64, 72), align=1, cond=U.B_QTY)
        self.text(row, '{%d}' % U.F_TAG, 232, 2, self.fs, lab, (64, 64, 72), align=1, cond=U.B_TAG)
        ay = (18 - arrow.height) // 2
        self.img(row, 'bw.bag.cursor', arrow, 108 - arrow.width + 2, ay, cond=U.C_SEL0)
        self.img(row, 'bw.bag.cursor', arrow, 108 - arrow.width + 2, ay, cond=U.B_SWAP, unless=U.C_SEL0)
        p.spec(row)

    # ---------------------------------------------------------------------------------------------------- pop-ups
    def popups(self):
        """KAI's own pop-ups (menus, YES / NO, the save's box) in Black's balloon and font (mp_uistyle.c PopUps):
        the red ones red, the pale ones grey"""
        st, p = self.st, self.p
        sp = U.Spec('win')
        self.img(sp, 'bw.win', st.balloon(24, 24))
        sp.add(U.OP_FILL, w=0, h=0, ink=(248, 248, 248), a=255)
        p.spec(sp)
        t = U.Spec('win.text')
        self.text(t, '{%d}' % U.F_NAME, 0, 0, self.fm, *st.INK, unless=U.C_SEL0 | U.C_ALT)
        self.text(t, '{%d}' % U.F_NAME, 0, 0, self.fm, (232, 64, 48), (248, 168, 168), cond=U.C_SEL0)
        self.text(t, '{%d}' % U.F_NAME, 0, 0, self.fm, (168, 168, 168), (216, 216, 216), cond=U.C_ALT)
        p.spec(t)

    # ---------------------------------------------------------------------------------------------------- summary
    # (letters of Black's type icons, 4 or 5 pixels and their shadow column: the type, the first and last column)
    ICON_LETTERS = {'C': ('ROCK', 17, 21), 'O': ('ROCK', 11, 15), 'L': ('STEEL', 23, 27), 'B': ('BUG', 7, 11),
                    'E': ('STEEL', 13, 17), 'A': ('WATER', 10, 14), 'U': ('BUG', 14, 18), 'T': ('STEEL', 9, 12),
                    'Y': ('PSYCHIC', 11, 16), 'S': ('STEEL', 4, 8), 'M': ('NORMAL', 16, 21), 'R': ('ROCK', 5, 9),
                    'G': ('BUG', 21, 25), 'H': ('GHOST', 9, 13)}

    def contest_icon(self, word, col):
        """a contest kind's icon, which Black has none of: a type icon's frame (its border, fill and the word's
        white letters with their shadow) in the kind's colour, the word spelt from Black's type icons' own
        letters"""
        st = self.st
        dark = tuple(int(c * 0.45) for c in col)
        im = Image.new('RGBA', (32, 16), (0, 0, 0, 0))
        q = im.load()
        for y in range(12):
            for x in range(32):
                edge = y in (0, 11) or x in (0, 31)
                q[x, 2 + y] = (dark if edge else col) + (255,)
        masks = []
        for ch in word:
            t, x0, x1 = self.ICON_LETTERS[ch]
            src = st.type_icon(t)
            src = src.crop(src.getbbox())
            sq = src.load()
            fill = sq[1, 1][:3]
            m = [[('#' if sq[x, y][:3] == (248, 248, 248) else 's' if sq[x, y][:3] != fill else '.')
                  for x in range(x0, x1 + 1)] for y in range(2, 10)]
            masks.append(m)
        total = sum(len(m[0]) for m in masks)
        gap = 1 if total + len(masks) - 1 <= 30 else 0
        total += gap * (len(masks) - 1)
        assert total <= 30, word
        x = 1 + (30 - total) // 2
        for m in masks:
            for yy, row in enumerate(m):
                for xx, v in enumerate(row):
                    if v != '.':
                        q[x + xx, 2 + 2 + yy] = ((248, 248, 248) if v == '#' else dark) + (255,)
            x += len(m[0]) + gap
        return im

    def summary(self):
        """bw.py summary_page(): Black's summary (a/0/7/8) for KAI's six pages. Each: the touch screen's title bar
        with MOéMON INFO (or POKéMON INFO, as OPTIONS > CREATURES says) and the page as Black names its pages, six
        dots. INFO as approved (Black's INFO page, screen 63); SKILLS on Black's STATS page (screen 73: HP and its
        bar, the five stats on its bullet rows, IV stars or EVs, nature's red and blue), EXP. POINTS and NEXT LV.
        under it; the moves on Black's lined page (screen 64): a row each, the type (or contest) icon, the name and
        its PP, the cursor's row lit as Black's lists light it; the picked move's POWER / ACCURACY (or APPEAL / JAM)
        on the navy and its words on more lines; the POKé BALL and the JOURNAL on the lined page"""
        st, p = self.st, self.p
        ink, sh = st.INK
        lab, lsh = (248, 248, 248), (128, 128, 128)
        red, blue = ((232, 56, 40), (248, 168, 160)), ((48, 96, 232), (160, 184, 248))
        navy = (56, 56, 72)
        mi, fi = st.MALE_INK, st.FEMALE_INK
        moe = p.image('bw.sum.title.moe', st.summary_title(True))
        poke = p.image('bw.sum.title.poke', st.summary_title(False))
        assert poke == moe + 1
        tfirst = None
        for i, t in enumerate(U.TYPE_NAMES):
            k = p.image('bw.type%d' % i, self.type_icon(t if t != '???' else 'NORMAL'))
            tfirst = k if tfirst is None else tfirst
            assert k == tfirst + i
        # (the contest kinds, which Black has none of: its NORMAL icon in each kind's colour, the word in the icon's
        # own letters)
        cfirst = None
        for i, (word, col) in enumerate((('COOL', (232, 80, 56)), ('BEAUTY', (72, 128, 232)),
                                         ('CUTE', (240, 120, 176)), ('SMART', (64, 176, 96)),
                                         ('TOUGH', (224, 176, 40)))):
            k = p.image('bw.contest%d' % i, self.contest_icon(word, col))
            cfirst = k if cfirst is None else cfirst
            assert k == cfirst + i
        star = (0x10, 0x10, 0x38, 0xFE, 0x7C, 0x38, 0x6C, 0x44)
        heart = (0x00, 0xD8, 0xF8, 0xF8, 0x70, 0x20, 0x00, 0x00)

        def shapes(tag, shape, counts, total, on, off, perfect=None, pitch=8):
            first = None
            for i, n in enumerate(counts):
                im = Image.new('RGBA', (pitch * total, 8), (0, 0, 0, 0))
                for j in range(total):
                    c = (perfect if perfect and n > total else on) if j < min(n, total) else off
                    for yy in range(8):
                        for xx in range(8):
                            if shape[yy] & (0x80 >> xx):
                                im.putpixel((j * pitch + xx, yy), c + (255,))
                k = p.image('bw.%s%d' % (tag, i), im)
                first = k if first is None else first
                assert k == first + i
            return first
        stars = shapes('stars', star, [0, 1, 2, 3, 4], 3, ink, (200, 200, 200), perfect=red[0])
        hearts = shapes('hearts', heart, range(9), 8, (248, 96, 120), (112, 112, 128), pitch=6)
        prompts = ['', 'A: IV RATING', 'A: EVs', 'A: STATS', 'A: INFO', 'A: SWITCH', 'A: MORE', '']
        pfirst = None
        for i, t in enumerate(prompts):
            im = Image.new('RGBA', (120, 16), (0, 0, 0, 0))
            if t:
                st.text(im, 119, 0, t, lab, lsh, 's', 'r')
            k = p.image('bw.sum.prompt%d' % i, im)
            pfirst = k if pfirst is None else pfirst
            assert k == pfirst + i
        lfirst = None
        for i, t in enumerate(prompts):
            im = Image.new('RGBA', (120, 16), (0, 0, 0, 0))
            if t:
                st.text(im, 0, 0, t, lab, lsh, 's')
            k = p.image('bw.sum.lprompt%d' % i, im)
            lfirst = k if lfirst is None else lfirst
            assert k == lfirst + i

        def header(sp, k, ground):
            ground.alpha_composite(st.summary_bar(k), (0, 0))
            self.img(sp, 'bw.sum%d' % k, ground)
            sp.add(U.OP_IMGSEL, x=0, y=0, a=moe, b=U.F_CREATURES)

        def rows(top, pick, y0, ground):
            for i, r in enumerate(pick):
                ground.paste(top.crop((0, r, 240, r + 1)), (0, y0 + i))

        # INFO (as approved: bw.py summary_page)
        top = st.summary_top(63)
        g = Image.new('RGBA', (240, 160), (0, 0, 0, 255))
        g.paste(top.crop((0, 6, 240, 150)), (0, 16))
        labels = ['Dex No.', 'Name', 'Type', 'OT', 'ID No.', 'Ability']
        for i, t in enumerate(labels):
            st.text(g, 16, 16 * (i + 1) + 3, t, lab, lsh)
        # (a memo of more than the nature, the place and the level: the box under it navy, its lines on the navy)
        gn = g.copy()
        q = gn.load()
        for y in range(146, 160):
            for x in range(0, 152):
                r_, g_, b_ = q[x, y][:3]
                if r_ == g_ == b_ and r_ >= 100:
                    q[x, y] = navy + (255,)
        sp = U.Spec('sum.0')
        g0 = g.copy()
        g0.alpha_composite(st.summary_bar(0), (0, 0))
        gn.alpha_composite(st.summary_bar(0), (0, 0))
        self.img(sp, 'bw.sum0', g0, cond=U.SM_MEMOPLAIN)
        self.img(sp, 'bw.sum0n', gn, unless=U.SM_MEMOPLAIN)
        sp.add(U.OP_IMGSEL, x=0, y=0, a=moe, b=U.F_CREATURES)
        for i, f in ((0, U.F_DEXNO), (1, U.F_SPECIES), (3, U.F_OT), (4, U.F_IDNO)):
            self.text(sp, '{%d}' % f, 83, 16 * (i + 1) + 3, self.fm, ink, sh)
        self.text(sp, '{%d}' % U.F_ABILITY, 83, 99, self.fm, lab, lsh)
        sp.add(U.OP_IMGSEL, x=83, y=53, a=tfirst, b=U.F_TYPE1, unless=U.SM_EGG)
        sp.add(U.OP_IMGSEL, x=83 + 34, y=53, a=tfirst, b=U.F_TYPE2, cond=U.SM_TWOTYPES, unless=U.SM_EGG)
        self.text(sp, '{%d}' % U.F_TEXT0, 110, 115, self.fs, ink, sh, align=2)
        self.text(sp, '{%d} nature.' % U.F_NATURE, 16, 131, self.fm, lab, lsh, cond=U.SM_MEMOPLAIN)
        self.text(sp, 'Lv{%d}, {%d}.' % (U.F_METLV, U.F_METPLACE), 28, 147, self.fs, ink, sh, cond=U.SM_MEMOPLAIN)
        sp.add(U.OP_MSG, x=16, y=128, a=2, h=11, n=3, w=200, font=self.fs, ink=lab, shadow=lsh,
               unless=U.SM_MEMOPLAIN)
        self.text(sp, 'SELECT: RENAME', 200, 149, self.fs, ink, sh, align=2, cond=U.SM_RENAME)
        self.text(sp, '{%d}' % U.F_NAME, 163, 16, self.fs, ink, sh, reg=1)
        self.text(sp, '♂', 2, 16, self.fs, mi[0], mi[1], rel=True, reg=1, cond=U.C_MALE)
        self.text(sp, '♀', 2, 16, self.fs, fi[0], fi[1], rel=True, reg=1, cond=U.C_FEMALE)
        self.text(sp, 'Lv{%d}' % U.F_LEVEL, 236, 27, self.fs, lab, lsh, align=1, unless=U.SM_EGG)
        sp.add(U.OP_ICON, x=196, y=78, a=0)
        sp.add(U.OP_ICON, x=176, y=33, a=2)
        p.spec(sp)

        # SKILLS on Black's STATS page: its rows 6-8, then 24-143 (the HP row from 32), navy under them
        top = st.summary_top(73, 10)
        g = Image.new('RGBA', (240, 160), navy + (255,))
        rows(top, [6, 7, 8] + list(range(24, 144)), 16, g)
        st.text(g, 88, 28, 'HP', lab, lsh)
        for i, t in enumerate(['Attack', 'Defense', 'Sp. Atk', 'Sp. Def', 'Speed']):
            st.text(g, 64, 52 + 16 * i, t, lab, lsh)
        st.text(g, 16, 139, 'Exp. Points', lab, lsh, 's')
        st.text(g, 124, 139, 'Next Lv.', lab, lsh, 's')
        sp = U.Spec('sum.1')
        header(sp, 1, g)
        no = U.SM_IV | U.SM_EV
        self.text(sp, '{%d}/{%d}' % (U.F_HP, U.F_MAXHP), 196, 28, self.fm, ink, sh, align=1, unless=no)
        sp.add(U.OP_IMGSEL, x=172, y=32, a=stars, b=U.F_IV0, cond=U.SM_IV)
        self.text(sp, '{%d}' % U.F_EV0, 196, 28, self.fm, ink, sh, align=1, cond=U.SM_EV)
        bar = p.image('bw.partybar', st.party_hp(0.0))
        P = st.pal('a/0/8/3', 0)
        fills = []
        for t, colour in zip('gyr', ((5, 6), (9, 10), (7, 8))):
            im = Image.new('RGBA', st.party_hp(0.0).size, (0, 0, 0, 0))
            for j in range(48):
                im.putpixel((16 + j, 3), BW.rgb(P[colour[0]]) + (255,))
                im.putpixel((16 + j, 4), BW.rgb(P[colour[1]]) + (255,))
            fills.append(p.image('bw.partybar.' + t, im))
        sp.add(U.OP_IMG, x=132, y=43, a=bar)
        sp.add(U.OP_BAR, x=132, y=43, a=U.F_HPFRAC, b=fills[0], c=fills[1], d=fills[2])
        for i in range(5):
            y = 52 + 16 * i
            self.text(sp, '{%d}' % (U.F_STAT0 + i), 196, y, self.fm, *red, align=1, cond=U.C_SEL0 << i, unless=no)
            self.text(sp, '{%d}' % (U.F_STAT0 + i), 196, y, self.fm, *blue, align=1, cond=U.C_SEL0 << (5 + i),
                      unless=no)
            self.text(sp, '{%d}' % (U.F_STAT0 + i), 196, y, self.fm, ink, sh, align=1,
                      unless=no | (U.C_SEL0 << i) | (U.C_SEL0 << (5 + i)))
            sp.add(U.OP_IMGSEL, x=172, y=y + 4, a=stars, b=U.F_IV0 + 1 + i, cond=U.SM_IV)
            self.text(sp, '{%d}' % (U.F_EV0 + 1 + i), 196, y, self.fm, ink, sh, align=1, cond=U.SM_EV)
        self.text(sp, '{%d}' % U.F_EXPPTS, 110, 139, self.fs, lab, lsh, align=1)
        self.text(sp, '{%d}' % U.F_NEXTLV, 232, 139, self.fs, lab, lsh, align=1)
        self.text(sp, 'START: CLEAR', 16, 150, self.fs, lab, lsh, cond=U.SM_CLEAREV)
        sp.add(U.OP_IMGSEL, x=116, y=150, a=pfirst, b=U.F_PROMPT)
        p.spec(sp)

        # the moves on Black's lined page: its rows 6-8, then its lined rows 40-120 (five), navy under them; picked,
        # the lined rows again under the navy row for its words
        lined = st.summary_top(64, 10)
        for k, contest in ((2, False), (3, True)):
            g = Image.new('RGBA', (240, 160), navy + (255,))
            rows(lined, [6, 7, 8] + list(range(40, 121)), 16, g)
            rows(lined, list(range(184, 192)), 152, g)
            gc = g.copy()
            rows(lined, list(range(40, 84)), 116, gc)
            sp = U.Spec('sum.%d' % k)
            gc.alpha_composite(st.summary_bar(k), (0, 0))
            self.img(sp, 'bw.sum%dc' % k, gc, cond=U.SM_CHOOSING)
            g.alpha_composite(st.summary_bar(k), (0, 0))
            self.img(sp, 'bw.sum%d' % k, g, unless=U.SM_CHOOSING)
            sp.add(U.OP_IMGSEL, x=0, y=0, a=moe, b=U.F_CREATURES)
            # (not picked: the POKéMON, its name and level on the navy)
            self.text(sp, '{%d}' % U.F_NAME, 16, 104, self.fm, lab, lsh, reg=1, unless=U.SM_CHOOSING)
            self.text(sp, '♂', 2, 104, self.fm, mi[0], mi[1], rel=True, reg=1, cond=U.C_MALE, unless=U.SM_CHOOSING)
            self.text(sp, '♀', 2, 104, self.fm, fi[0], fi[1], rel=True, reg=1, cond=U.C_FEMALE,
                      unless=U.SM_CHOOSING)
            self.text(sp, 'Lv{%d}' % U.F_LEVEL, 16, 120, self.fs, lab, lsh, unless=U.SM_CHOOSING | U.SM_EGG)
            sp.add(U.OP_ICON, x=196, y=130, a=0, unless=U.SM_CHOOSING)
            sp.add(U.OP_IMGSEL, x=16, y=138, a=lfirst, b=U.F_PROMPT, unless=U.SM_CHOOSING)
            # (picked: its numbers on the navy row, its words on the lines)
            if contest:
                self.text(sp, 'Appeal', 8, 101, self.fs, lab, lsh, cond=U.SM_CHOOSING)
                sp.add(U.OP_IMGSEL, x=44, y=104, a=hearts, b=U.F_APPEAL, cond=U.SM_CHOOSING)
                self.text(sp, 'Jam', 100, 101, self.fs, lab, lsh, cond=U.SM_CHOOSING)
                sp.add(U.OP_IMGSEL, x=120, y=104, a=hearts, b=U.F_JAM, cond=U.SM_CHOOSING)
            else:
                self.text(sp, 'Power', 8, 101, self.fs, lab, lsh, cond=U.SM_CHOOSING)
                self.text(sp, '{%d}' % U.F_POWER, 68, 101, self.fs, lab, lsh, align=1, cond=U.SM_CHOOSING)
                self.text(sp, 'Accuracy', 80, 101, self.fs, lab, lsh, cond=U.SM_CHOOSING)
                self.text(sp, '{%d}' % U.F_ACC, 150, 101, self.fs, lab, lsh, align=1, cond=U.SM_CHOOSING)
            sp.add(U.OP_IMGSEL, x=116, y=101, a=pfirst, b=U.F_PROMPT, cond=U.SM_CHOOSING)
            sp.add(U.OP_MSG, x=8, y=117, a=1, h=16, n=3, w=224, e=self.fs + 1, font=self.fm, ink=ink, shadow=sh,
                   cond=U.SM_CHOOSING)
            sp.add(U.OP_LIST, x=0, y=20, h=16, n=5)
            p.spec(sp)
            row = U.Spec('sum.%d.row' % k)
            # (the cursor's row lit as Black lights a list's row: its own lines in the BAG's blue)
            lit = Image.new('RGBA', (232, 15), (168, 216, 248, 255))
            self.img(row, 'bw.sum.lit', lit, 4, 0, cond=U.C_SEL0)
            self.img(row, 'bw.sum.swap', Image.new('RGBA', (232, 15), (248, 200, 168, 255)), 4, 0, cond=U.B_SWAP,
                     unless=U.C_SEL0)
            if contest:
                row.add(U.OP_IMGSEL, x=6, y=0, a=cfirst, b=U.F_CTYPE, unless=U.R_EMPTY | U.R_CANCEL)
            else:
                row.add(U.OP_IMGSEL, x=6, y=0, a=tfirst, b=U.F_TYPE, unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '{%d}' % U.F_NAME, 42, 0, self.fm, ink, sh, w=110, b=self.fs + 1,
                      unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, 'PP', 156, 1, self.fs, ink, sh, unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '{%d}/{%d}' % (U.F_PP, U.F_MAXPP), 204, 0, self.fm, ink, sh, align=1,
                      unless=U.R_EMPTY | U.R_CANCEL)
            self.text(row, '-', 42, 0, self.fm, ink, sh, cond=U.R_EMPTY)
            self.text(row, 'CANCEL', 42, 0, self.fm, ink, sh, cond=U.R_CANCEL)
            p.spec(row)

        # the POKé BALL and the JOURNAL on the lined page
        for k in (4, 5):
            g = Image.new('RGBA', (240, 160), navy + (255,))
            rows(lined, [6, 7, 8] + list(range(40, 181)), 16, g)
            sp = U.Spec('sum.%d' % k)
            header(sp, k, g)
            if k == 4:
                # (the lines on the lined page's own rows, 16 apart, from the margin the other pages' words keep;
                # the ball at the right, between its first two rows)
                sp.add(U.OP_LIST, x=16, y=19, h=0, n=16)
                sp.add(U.OP_ICON, x=214, y=39, a=4)
            else:
                sp.add(U.OP_MSG, x=8, y=20, a=3, h=16, n=8, w=224, e=self.fs + 1, font=self.fm, ink=ink, shadow=sh)
                self.text(sp, '{%d}' % U.F_JPAGE, 232, 148, self.fs, ink, sh, align=1, cond=U.S_DOWN)
            p.spec(sp)
            row = U.Spec('sum.%d.row' % k)
            self.text(row, '{%d}' % U.F_NAME, 0, 0, self.fm, ink, sh)
            p.spec(row)

    def make(self):
        self.fonts()
        self.huds()
        self.panels()
        self.party()
        self.options()
        self.bag()
        self.popups()
        self.summary()
        return self.p
