"""Draws each style's specs (uipack.py) with the mock-ups' own words and numbers over the same scenes and compares
them with the approved mock-ups, pixel for pixel: what the game will draw is what was approved.
usage: check_specs.py KAI_SHOTS STYLE...   (STYLE: red=PRET crystal=PRET firered=PRET platinum=ROM black=ROM)"""
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
    flags = (U.C_MALE if m['sex'] == 'm' else U.C_FEMALE if m['sex'] == 'f' else 0)
    flags |= U.C_LV3 if m['level'] >= 100 else U.C_LV1 if m['level'] < 10 else 0
    return d, flags


def huds(pack, specs, im, rects, mons, kind, exp=False):
    for r, m in zip(rects, mons):
        d, f = hud_data(m)
        if exp and 'exp' in m:
            f |= U.C_EXP
        U.render(pack, specs['hud.' + kind], d, f, im, (r[0], r[1]))


def screens(pack, kai, account=None):
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
            huds(pack, specs, im, Lo.DOUBLE_MINE, C.DOUBLE_MINE, 'mine2')
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
            U.render(pack, specs['b.cmd'], cmd, U.C_SEL0, im)
        out[name] = im
    im = C.shot(kai, 'f_mart')
    U.render(pack, specs['f.msg'], {U.F_TEXT0: C.MESSAGE}, U.C_WAIT, im)
    out['message'] = im
    im = C.shot(kai, 'f_town')
    d = {U.F_ITEM0 + i: s for i, s in enumerate(C.START_MENU)}
    if account:
        d[U.F_TEXT0 + 30], d[U.F_TEXT0 + 31] = account
    U.render(pack, specs['start.%d' % len(C.START_MENU)], d, U.C_SEL0, im)
    out['start_menu'] = im
    return out


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
        for name, im in got.items():
            want = getattr(mock, name)()
            if name == 'battle_triple':   # (KAI's own type icons beside the cards: the game draws those, not a style)
                for x, y in Lo.TRIPLE_FOE_ICONS + Lo.TRIPLE_MINE_ICONS:
                    want.paste(im.crop((x - 1, y - 1, x + 13, y + 13)), (x - 1, y - 1))
            print(kind, name, compare(im, want))
            im.save('/tmp/claude-0/-home-user/a6424581-b487-50fe-8bae-5e1098371be0/scratchpad/spec_%s_%s.png'
                    % (kind, name))


if __name__ == '__main__':
    main()
