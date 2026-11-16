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


# the OPTIONS rows the mock-ups show chosen, by style (the mock-ups' own choices; UI STYLE is the style's own name)
OPTION_CHOSEN = {
    'red': None, 'crystal': None, 'platinum': None,            # (the last of each row's choices)
    'firered': {'THEME': 'LIGHT', 'FRAME': 'TYPE 1', 'SHADOWS': 'ON', 'BATTLE ANIMS': 'MODERN', 'TYPE ICONS': 'ON'},
    'black': {'THEME': 'LIGHT', 'FRAME': 'TYPE 1', 'SHADOWS': 'ON', 'BATTLE ANIMS': 'MODERN', 'TYPE ICONS': 'ON'},
}


def options_screen(pack, kind):
    """OPTIONS' DISPLAY tab as the game draws it (src/mp/mp_uistyle.c OptionsScreen) with the mock-ups' rows"""
    specs = {s.name: s for s in pack.specs}
    im = Image.new('RGBA', (C.W, C.H), (0, 0, 0, 255))
    d = {U.F_ITEM0 + i: t for i, t in enumerate(C.OPTION_TABS)}
    d[U.F_HINT] = 'L / R: TABS     MOEMON EMERALD KAI V1.0'
    flags = U.C_SEL0 << C.OPTION_TABS.index('DISPLAY')
    U.render(pack, specs['o'], d, flags, im)
    lst = [op for op in specs['o'].ops if op.kind == U.OP_LIST and (flags & op.cond) == op.cond
           and not flags & op.unless][0]
    chosen = OPTION_CHOSEN[kind]
    for i, (label, vals) in enumerate(C.OPTION_ROWS[:lst.n]):
        names = [s.upper() for s in C.STYLES] if vals is None else list(vals)
        if vals is None:
            cur = names.index(kind.upper())
        elif chosen is None:
            cur = len(names) - 1
        else:
            cur = names.index(chosen[label])
        rd = {U.F_NAME: label, U.F_NCHOICE: len(names), U.F_CUR: cur, U.F_CURSTR: names[cur]}
        for k, n in enumerate(names):
            rd[U.F_CHOICE0 + k] = n
        f = (U.C_SEL0 if i == 0 else 0) | (U.O_ALL if 2 <= len(names) <= 3 else 0) | U.O_VALUE
        U.render(pack, specs['o.row'], rd, f, im, (lst.x, lst.y + i * lst.h))
    return im


def bag_screen(pack, kind, kai, export):
    """the BAG's MEDICINE pocket as the game draws it (src/mp/mp_uistyle.c BagScreen) with the mock-ups' rows"""
    specs = {s.name: s for s in pack.specs}
    im = Image.new('RGBA', (C.W, C.H), (0, 0, 0, 255))
    view = C.POCKETS.index(C.BAG_POCKET)
    flags = U.C_SEL0 << view
    lst = [op for op in specs['bag'].ops if op.kind == U.OP_LIST and (flags & op.cond) == op.cond
           and not flags & op.unless][0]
    rows = [(n, q) for n, q, _ in C.BAG_ITEMS] + [('CLOSE BAG', None)]
    if lst.n < len(rows):
        flags |= U.S_DOWN
    # (the description: the mock-ups' own lines where they keep them, else the words to wrap)
    desc = '\n'.join(C.BAG_DESC) if kind in ('platinum', 'black') else ' '.join(C.BAG_DESC)
    d = {U.F_TITLE: C.BAG_POCKET, U.F_CUR: view, U.F_TEXT0: desc, U.SNAP: C.shot(kai, 'f_town')}
    U.render(pack, specs['bag'], d, flags, im)
    if export:
        icon = [op for op in specs['bag'].ops if op.kind == U.OP_ICON and op.a == 1]
        ic = C.item_icon(export, C.BAG_ITEMS[0][2])
        if icon and ic is not None:
            C.paste(im, ic, icon[0].x - 16, icon[0].y - 16)
    for i, (n, q) in enumerate(rows[:lst.n]):
        rd = {U.F_NAME: n}
        f = U.C_SEL0 if i == 0 else 0
        if q is None:
            f |= U.B_CLOSE
        else:
            rd[U.F_QTY], rd[U.F_QTY2] = str(q), '%2d' % q
            f |= U.B_QTY
        U.render(pack, specs['bag.row'], rd, f, im, (lst.x, lst.y + i * lst.h))
    return im


def summary_screen(pack, kind, export):
    """the summary's INFO page as the game draws it (src/mp/mp_uistyle.c SummaryScreen) with the mock-ups' words"""
    specs = {s.name: s for s in pack.specs}
    im = Image.new('RGBA', (C.W, C.H), (0, 0, 0, 255))
    sm = C.SUMMARY
    d = {U.F_NAME: sm['name'], U.F_LEVEL: str(sm['level']), U.F_DEXNO: sm['no'], U.F_SPECIES: sm['species'],
         U.F_OT: sm['ot'], U.F_IDNO: sm['idno'], U.F_HELD: sm['item'], U.F_NATURE: sm['nature'],
         U.F_ABILITY: sm['ability'], U.F_METLV: str(sm['level']), U.F_METPLACE: sm['place'].rstrip('.'),
         U.F_TYPE1: U.TYPE_NAMES.index(sm['type']), U.F_TYPE2: U.TYPE_NAMES.index(sm['type']),
         U.F_PAGETITLE: 'MOéMON INFO', U.F_PROMPT: 0, U.F_TEXT0: sm['ability_text']}
    f = U.C_MALE | U.SM_MEMOPLAIN | U.SM_RENAME
    U.render(pack, specs['sum.0'], d, f, im)
    if export:
        icon = [op for op in specs['sum.0'].ops if op.kind == U.OP_ICON and op.a == 0]
        if icon:
            C.paste(im, C.creature(export, 280), icon[0].x - 32, icon[0].y - 32)
    return im


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
        if any(sp.name == 'o' for sp in pack.specs):
            got['options'] = options_screen(pack, kind)
        if any(sp.name == 'sum.0' for sp in pack.specs) and os.environ.get('MOEMON_EXPORT'):
            got['summary'] = summary_screen(pack, kind, os.environ.get('MOEMON_EXPORT'))
        if any(sp.name == 'bag' for sp in pack.specs):
            got['bag'] = bag_screen(pack, kind, kai, os.environ.get('MOEMON_EXPORT'))
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
