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
        for (long, nd, sx) in [(lg, n, g) for lg in (False, True) for n in (1, 2, 3) for g in (None, 'm', 'f')]:
            nxw = 2 if long else 8
            if True:
                V = Y[sx]
                fy, g0, g0u, full, e1, ny = V['fy'], V['g0'], V['g0'], V['full'], V['e1'], V['ny']
                cond = lvc[nd][0] | (U.C_LONG if long else 0) | gcs[sx][0]
                unless = lvc[nd][1] | (0 if long else U.C_LONG) | gcs[sx][1]
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
                ox, oy = fx - x0, fy

                def X(x):
                    return ox + (moved(keep, x) if keep else x)

                def cut(im):
                    return _rebuild(im, 'x', keep) if keep else im
                tag = '%s.%d%s%s' % (name, nd, 'L' if long else '', sx or '')
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
        self.text(sp, '{%d}' % U.F_NAME, 1, by - 12, self.fs, self.gink, self.gsh, reg=1)
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
        for key, (c, rr, field) in place.items():
            bx, bw_ = cols[c]
            by = 0 if rr == 0 else h - bh
            arc = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
            st.arcs(arc, x0 + bx, y0 + by, x0 + bx + bw_, y0 + by + bh, out=4)
            ab = arc.getbbox()
            self.img(sp, 'bw.arc.%s' % key, arc.crop(ab), ab[0], ab[1], cond=U.C_SEL0 << field)

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
        for i in range(4):
            bx, by = x0 + (i % 2) * 80, y0 + (i // 2) * 24
            arc = Image.new('RGBA', (240, 160), (0, 0, 0, 0))
            st.arcs(arc, bx, by, bx + bw_, by + bh_, out=4)
            ab = arc.getbbox()
            self.img(sp, 'bw.marc.%d' % i, arc.crop(ab), ab[0], ab[1], cond=U.C_SEL0 << i)
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

    def make(self):
        self.fonts()
        self.huds()
        self.panels()
        self.party()
        self.options()
        return self.p
