"""Draws each style's specs (uipack.py) with the mock-ups' own words and numbers over the same scenes and compares
them with the approved mock-ups, pixel for pixel: what the game will draw is what was approved.
usage: [MOEMON_EXPORT=DIR] [CHECK_OUT=DIR] check_specs.py KAI_SHOTS STYLE...
    (STYLE: red=PRET crystal=PRET firered=PRET platinum=PRET:ROM black=ROM; MOEMON_EXPORT adds the party screen)"""
import os
import sys

from PIL import Image

import common as C
import layout as Lo
import uipack as U


def hud_data(m):
    d = {U.F_NAME: m['name'], U.F_LEVEL: m['level']}
    frac = m['hp'] / m['maxhp'] if 'maxhp' in m else m['hp']
    d[U.F_HPFRAC] = float(frac)
    if 'maxhp' in m:
        d[U.F_HP3] = '%3d' % m['hp']
        d[U.F_MAXHP3] = '%3d' % m['maxhp']
        d[U.F_HP], d[U.F_MAXHP] = m['hp'], m['maxhp']
    if 'exp' in m:
        d[U.F_EXP] = float(m['exp'])
    flags = (U.C_MALE if m.get('sex') == 'm' else U.C_FEMALE if m.get('sex') == 'f' else 0)
    flags |= U.C_LV3 if m['level'] >= 100 else U.C_LV1 if m['level'] < 10 else 0
    return d, flags


POS = {'foe2': (1, 3), 'mine2': (0, 2)}     # (a box's B_POSITION_*, for a style with one spec per spot)


def huds(pack, specs, im, rects, mons, kind, exp=False):
    for k, (r, m) in enumerate(zip(rects, mons)):
        d, f = hud_data(m)
        if exp and 'exp' in m:
            f |= U.C_EXP
        sp = specs.get('hud.%s.%d' % (kind, POS[kind][k])) if kind in POS else None
        U.render(pack, sp or specs['hud.' + kind], d, f, im, (r[0], r[1]))


def screens(pack, kai, account=None, icons=None):
    specs = {s.name: s for s in pack.specs}
    out = {}
    cmd = {U.F_TEXT0 + 1: U.encode('\n'.join(C.PROMPT))}
    for i, a in enumerate(C.ACTIONS):
        cmd[U.F_ITEM0 + i] = a
    for name, scene, layout in (('battle_single', 's_single', 'single'), ('battle_double', 's_double', 'double'),
                                ('battle_triple', 's_triple', 'triple'), ('battle_moves', 's_single', 'moves')):
        im = C.shot(kai, scene)
        if layout in ('single', 'moves'):
            huds(pack, specs, im, [Lo.SINGLE_FOE], [C.FOE], 'foe1')
            huds(pack, specs, im, [Lo.SINGLE_MINE], [C.MINE], 'mine1', exp=True)
        elif layout == 'double':
            huds(pack, specs, im, Lo.DOUBLE_FOES, C.DOUBLE_FOES, 'foe2')
            huds(pack, specs, im, Lo.DOUBLE_MINE, C.DOUBLE_MINE, 'mine2', exp=True)
        else:
            huds(pack, specs, im, Lo.TRIPLE_FOES, C.TRIPLE_FOES, 'foe3')
            huds(pack, specs, im, Lo.TRIPLE_MINE, C.TRIPLE_MINE, 'mine3')
        if layout == 'moves':
            d = {U.F_TYPE: C.MOVES[0][3], U.F_PP: C.MOVES[0][1], U.F_MAXPP: C.MOVES[0][2]}
            for i, mv in enumerate(C.MOVES):
                d[U.F_ITEM0 + i] = mv[0]
                d[U.F_ITEM0 + 8 + i] = mv[3]
            U.render(pack, specs['b.moves'], d, U.C_SEL0, im)
        else:
            U.render(pack, specs['b.cmd'], cmd, U.C_SEL0 | U.C_STDCMD | U.C_MOEWORD, im)
        out[name] = im
    im = C.shot(kai, 'f_mart')
    U.render(pack, specs['f.msg'], {U.F_TEXT0: C.MESSAGE}, U.C_WAIT, im)
    out['message'] = im
    im = C.shot(kai, 'f_town')
    d = {U.F_ITEM0 + i: s for i, s in enumerate(C.START_MENU)}
    if account:
        d[U.F_TEXT0 + 30], d[U.F_TEXT0 + 31] = account
    for i, k in enumerate(icons or []):
        d[U.F_ICON0 + i] = k
    U.render(pack, specs['start.%d' % len(C.START_MENU)], d, U.C_SEL0, im)
    out['start_menu'] = im
    return out


# names, levels, genders and HP the approved screens don't show: each box kind drawn by the pack and by the mock-up's
# own code, over a plain ground
VARIANTS = [dict(name='MASQUERAIN', sex='f', level=100, hp=13, maxhp=200, exp=0.9),
            dict(name='MEW', sex=None, level=5, hp=1, maxhp=20, exp=0.0),
            dict(name='SHEDINJA', sex='m', level=55, hp=1, maxhp=1, exp=0.5),
            dict(name='PEWTERCRUN', sex='m', level=9, hp=99, maxhp=200, exp=1.0),
            dict(name='WWWWWWWWWW', sex='f', level=99, hp=150, maxhp=200, exp=0.2)]


def variants(pack, mock, kind):
    specs = {s.name: s for s in pack.specs}
    out = []
    for kinds, rects in ((('foe', 'foe1', False), Lo.SINGLE_FOE), (('mine', 'mine1', True), Lo.SINGLE_MINE),
                         (('foe2', 'foe2', False), Lo.DOUBLE_FOES[1]), (('mine2', 'mine2', False), Lo.DOUBLE_MINE[1]),
                         (('foe3', 'foe3', False), Lo.TRIPLE_FOES[2]), (('mine3', 'mine3', False), Lo.TRIPLE_MINE[2])):
        mk, sk, exp = kinds
        for m in VARIANTS:
            m = dict(m)
            if m['level'] >= 100:
                m['exp'] = 0.0      # (the game's EXP bar is empty at Lv100: mp_uistyle.c ExpOf)
            exp = exp or mk == 'mine2'
            if mk.startswith('foe'):
                m['hp'] = m['hp'] / m['maxhp']
                del m['maxhp']
            want = Image.new('RGBA', (240, 160), (40, 120, 80, 255))
            got = want.copy()
            try:
                mock.hud(want, rects, m, mk)
            except ValueError as e:
                out.append((sk, m['name'], 'mock-up refuses: %s' % e))
                continue
            d, f = hud_data(m)
            if exp and 'exp' in m:
                f |= U.C_EXP
            pos = {'foe2': 3, 'mine2': 2}.get(sk)
            sp = specs.get('hud.%s.%d' % (sk, pos)) if pos is not None else None
            U.render(pack, sp or specs['hud.' + sk], d, f, got, (rects[0], rects[1]))
            out.append((sk, m['name'], compare(got, want)))
            if compare(got, want) != 'same' and os.environ.get('CHECK_OUT'):
                got.save(os.path.join(os.environ['CHECK_OUT'], 'var_%s_%s_%s.png' % (kind, sk, m['name'])))
                want.save(os.path.join(os.environ['CHECK_OUT'], 'varw_%s_%s_%s.png' % (kind, sk, m['name'])))
    return out


def party_screen(pack, export):
    """the party screen as the game draws it from the pack with the mock-ups' PARTY (each slot's spec, Kai's icons
    where its ICON ops put them)"""
    specs = {s.name: s for s in pack.specs}
    im = Image.new('RGBA', (240, 160), (0, 0, 0, 255))
    U.render(pack, specs['party'], {U.F_TEXT0: 'Choose a MOéMON.'}, 0, im)
    for i in range(6):
        sp = specs.get('party.%d' % i)
        if sp is None:
            continue
        m = C.PARTY[i] if i < len(C.PARTY) else None
        if m is None:
            U.render(pack, sp, {}, U.P_EMPTY, im)
            continue
        d, f = hud_data(m)
        d[U.F_HP], d[U.F_MAXHP] = str(m['hp']), str(m['maxhp'])
        f |= U.C_SEL0 if i == 0 else 0
        U.render(pack, sp, d, f, im)
        for op in sp.ops:
            if op.kind == U.OP_ICON and op.a == 0 and not (op.unless & f):
                ic = C.creature(export, m['species'], 'icon')
                C.paste(im, ic, op.x - 16, op.y - 16)
        U.render(pack, sp, d, f, im, text_only=True)    # (the words over Kai's icons, as the game draws them)
    return im


def compare(a, b):
    a, b = a.convert('RGB'), b.convert('RGB')
    pa, pb = a.load(), b.load()
    bad = [(x, y) for y in range(a.height) for x in range(a.width) if pa[x, y] != pb[x, y]]
    if not bad:
        return 'same'
    xs, ys = [p[0] for p in bad], [p[1] for p in bad]
    return '%d pixels differ in (%d,%d)-(%d,%d)' % (len(bad), min(xs), min(ys), max(xs), max(ys))


def main():
    kai = sys.argv[1]
    for arg in sys.argv[2:]:
        kind, src = arg.split('=')
        if kind in ('red', 'crystal'):
            import gb_build
            b = gb_build.GBPack(kind, src)
            pack, mock = b.make(), gb_build.GB(kind, src, kai, None)
            got = screens(pack, kai, ('MOéMON', 'database') if kind == 'crystal' else None)
        elif kind == 'platinum':
            pret, rom = src.split(':', 1)
            os.environ['POKEPLATINUM'] = pret   # (pt.py reads it as it's imported)
            import pt_build
            b = pt_build.PTPack(pret, rom)
            pack, mock = b.make(), pt_build.Platinum(rom, kai, None)
            got = screens(pack, kai, None, icons=[0, 1, 2, 3, 4, 5, 6])
        elif kind == 'black':
            import bw_build
            pack, mock = bw_build.BWPack(src).make(), bw_build.Black(src, kai, None)
            got = screens(pack, kai, None)
        elif kind == 'firered':
            import fr_build
            pack, mock = fr_build.FRPack(src).make(), fr_build.FireRed(src, kai, None)
            got = screens(pack, kai, ('The MOéDEX keeps your record of', 'every MOéMON seen and caught.'))
        export = os.environ.get('MOEMON_EXPORT')
        if export and any(sp.name == 'party' for sp in pack.specs):
            mock.export = export
            got['party'] = party_screen(pack, export)
        for sk, name, res in variants(pack, mock, kind):
            print(kind, 'variant', sk, name, res)
        for name, im in got.items():
            want = getattr(mock, name)()
            if name == 'battle_triple':   # (KAI's own type icons beside the cards: the game draws those, not a style)
                for x, y in Lo.TRIPLE_FOE_ICONS + Lo.TRIPLE_MINE_ICONS:
                    want.paste(im.crop((x - 1, y - 1, x + 13, y + 13)), (x - 1, y - 1))
            print(kind, name, compare(im, want))
            if os.environ.get('CHECK_OUT'):   # (a folder for the drawn screens, to look at)
                im.save(os.path.join(os.environ['CHECK_OUT'], 'spec_%s_%s.png' % (kind, name)))


if __name__ == '__main__':
    main()
