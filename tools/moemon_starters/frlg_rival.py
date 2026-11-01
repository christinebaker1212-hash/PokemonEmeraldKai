#!/usr/bin/env python3
"""BLUE's teams in KANTO as FireRed's (the user, 2026-11-01: "reference FRLG for team compositions").

    python3 -I tools/moemon_starters/frlg_rival.py POKEFIRERED_TRAINER_PARTIES_H [--check]

Moemon Quetzal keeps FireRed's three versions of each of BLUE's battles (by the starter BLUE took: SQUIRTLE,
BULBASAUR, CHARMANDER), but rewrote some of them (most of the SQUIRTLE ones), and its scripts only ever call the
SQUIRTLE one (any of 36 starters could be picked). The game now picks the version from the player's starter
(battle_main.c, qz_specials.c QzKantoRivalVersion), so each version must be FireRed's: every party whose POKéMON
and levels differ from FireRed's is replaced by FireRed's whole party (moves and held items too), in
src/data/trainer_parties_qz.h, with its trainer's party kind in src/data/trainers_qz.h. Re-run it after re-importing
Quetzal's trainers. The file is pret/pokefirered's src/data/trainer_parties.h (raw.githubusercontent.com).
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PARTIES = os.path.join(ROOT, 'src', 'data', 'trainer_parties_qz.h')
TRAINERS = os.path.join(ROOT, 'src', 'data', 'trainers_qz.h')

# Quetzal's trainers per battle (BLUE with SQUIRTLE, BULBASAUR, CHARMANDER) and FireRed's party names
BATTLES = [
    ((8518, 8519, 8520), 'RivalOaksLab'),
    ((8521, 8522, 8523), 'RivalRoute22Early'),
    ((8524, 8525, 8526), 'RivalCerulean'),
    ((8618, 8619, 8620), 'RivalSsAnne'),
    ((8621, 8622, 8623), 'RivalPokemonTower'),
    ((8624, 8625, 8626), 'RivalSilph'),
    ((8627, 8628, 8629), 'RivalRoute22Late'),
    ((8630, 8631, 8632), 'ChampionFirst'),
]
VERSIONS = ('Squirtle', 'Bulbasaur', 'Charmander')
KIND = {
    'TrainerMonNoItemDefaultMoves': 'NO_ITEM_DEFAULT_MOVES',
    'TrainerMonNoItemCustomMoves': 'NO_ITEM_CUSTOM_MOVES',
    'TrainerMonItemDefaultMoves': 'ITEM_DEFAULT_MOVES',
    'TrainerMonItemCustomMoves': 'ITEM_CUSTOM_MOVES',
}


def parse_party(text, name):
    m = re.search(r'static const struct (\w+) %s\[\] = \{(.*?)\n\};' % re.escape(name), text, re.S)
    if not m:
        raise SystemExit('no party ' + name)
    mons = []
    for body in re.findall(r'\{([^{}]*(?:\{[^{}]*\}[^{}]*)?)\}', m.group(2)):
        f = dict(re.findall(r'\.(\w+)\s*=\s*(\{[^}]*\}|[^,\n]+)', body))
        if 'species' in f:
            mons.append({k: v.strip() for k, v in f.items()})
    return m.group(1), mons, m


def one_line(mon):
    keys = [k for k in ('iv', 'lvl', 'species', 'heldItem', 'moves') if k in mon]
    return '    {' + ', '.join('.%s = %s' % (k, re.sub(r'\s+', ' ', mon[k])) for k in keys) + '},'


def main():
    frlg = open(sys.argv[1]).read()
    parties = open(PARTIES).read()
    trainers = open(TRAINERS).read()
    changed = []
    for ids, base in BATTLES:
        for tid, ver in zip(ids, VERSIONS):
            ftype, fmons, _ = parse_party(frlg, 'sParty_%s%s' % (base, ver))
            qtype, qmons, qm = parse_party(parties, 'sParty_Qz%d' % tid)
            if [(m['species'], m['lvl']) for m in fmons] == [(m['species'], m['lvl']) for m in qmons]:
                continue
            changed.append('%d %s%s: %s -> %s' % (tid, base, ver,
                           ' '.join(m['species'][8:] + m['lvl'] for m in qmons),
                           ' '.join(m['species'][8:] + m['lvl'] for m in fmons)))
            new = 'static const struct %s sParty_Qz%d[] = {\n%s\n};' % (ftype, tid, '\n'.join(one_line(m) for m in fmons))
            parties = parties[:qm.start()] + new + parties[qm.end():]
            trainers, n = re.subn(r'(\.party = )\w+\(sParty_Qz%d\)' % tid, r'\g<1>%s(sParty_Qz%d)' % (KIND[ftype], tid), trainers)
            assert n == 1, tid
    print('\n'.join(changed) or 'all of BLUE\'s parties are FireRed\'s')
    if '--check' in sys.argv:
        sys.exit(1 if changed else 0)
    open(PARTIES, 'w').write(parties)
    open(TRAINERS, 'w').write(trainers)


if __name__ == '__main__':
    main()
