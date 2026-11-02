#!/usr/bin/env python3
"""KANTO's HM givers as FireRed's (Kai > FireRed > Quetzal; the user, 2026-11-02): Quetzal made them "teach" a field
move (a flag) and hand over an evolution item instead; FireRed gives the HM. Each giver's Quetzal item block becomes
EventScript_GiveHmOnce (data/scripts/mp_region.inc: not twice across regions) and its words FireRed's
(pokefirered data/maps/<map>/text.inc, fetched into FRLG_DIR; POKéMON / POKéDEX as MOéMON / MOéDEX). Run once; its edits are committed.

    python3 -I tools/quetzal_port/fix_hm_givers.py FRLG_DIR
"""
import os
import re
import sys

# (Kai map, Quetzal's item, the HM, FireRed map, {Quetzal text label suffix: FireRed text label})
GIVERS = [
    ('Kanto_SafariZone_SecretHouse', 'ITEM_DRAGON_SCALE', 'ITEM_HM03', 'SafariZone_SecretHouse',
     {'84284C2': 'CongratsYouveWon', '84285C1': 'ExplainSurf', '8428629': 'DontHaveRoomForPrize'}),
    ('Kanto_Route16_House', 'ITEM_RAZOR_FANG', 'ITEM_HM02', 'Route16_House',
     {'844FCAD': 'FoundMySecretRetreat', '844FD44': 'ExplainHM02', '844FD8F': 'DontHaveAnyRoomForThis'}),
    ('Kanto_FuchsiaCity_WardensHouse', 'ITEM_NUGGET', 'ITEM_HM04', 'FuchsiaCity_WardensHouse',
     {'844900F': 'ExplainStrength'}),
    ('Kanto_Route2_EastBuilding', 'ITEM_NUGGET', 'ITEM_HM05', 'Route2_EastBuilding',
     {'844DC8E': 'ExplainHM05'}),
]
BLOCK = re.compile(r'(\tcheckitemspace (ITEM_\w+), 1\n\tgoto_if_eq VAR_RESULT, 0, \w+\n)?'
                   r'\tadditem (ITEM_\w+), 1\n\tloadword 0, \w+\n\tsetorcopyvar VAR_0x8000, ITEM_\w+\n'
                   r'\tsetorcopyvar VAR_0x8001, 1\n\tsetorcopyvar VAR_0x8002, \w+\n\tcall Quetzal_EventScript_845571E\n')


def fr_text(frlg, frmap, label):
    text = open(os.path.join(frlg, '%s_text.inc' % frmap), encoding='utf-8').read()
    m = re.search(r'^%s_Text_%s::\n((?:\s*\.string "[^"]*"\n)+)' % (frmap, label), text, re.M)
    assert m, (frmap, label)
    lines = re.findall(r'\.string "([^"]*)"', m.group(1))
    return ''.join('\t.string "%s"\n' % l.replace('POKéMON', 'MOéMON').replace('POKéDEX', 'MOéDEX') for l in lines)


def main():
    frlg = sys.argv[1]
    for kai, old, hm, frmap, texts in GIVERS:
        path = 'data/maps/%s/scripts.inc' % kai
        s = open(path, encoding='utf-8').read()
        n = 0

        def repl(m):
            nonlocal n
            assert m.group(3) == old, (kai, m.group(3))
            n += 1
            return ('\tsetvar VAR_0x8004, %s   @ Moemon Kai: FireRed\'s HM (tools/quetzal_port/fix_hm_givers.py)\n'
                    '\tcall EventScript_GiveHmOnce\n' % hm)
        s = BLOCK.sub(repl, s)
        assert n == 1, (kai, n)
        for q, f in texts.items():
            lab = '%s_Text_%s::\n' % (kai, q)
            i = s.index(lab) + len(lab)
            j = i
            while s[j:].startswith('\t.string'):
                j = s.index('\n', j) + 1
            s = s[:i] + fr_text(frlg, frmap, f) + s[j:]
        open(path, 'w', encoding='utf-8').write(s)
        print('%s: %s' % (kai, hm))
    # ICEFALL CAVE: FireRed's HM07 ball (Quetzal's DRAGON SCALE)
    path = 'data/maps/Kanto_FourIsland_IcefallCave_1F/scripts.inc'
    s = open(path, encoding='utf-8').read()
    old = ('\tsetorcopyvar VAR_0x8000, ITEM_DRAGON_SCALE\n\tsetorcopyvar VAR_0x8001, 1\n\tcallstd STD_FIND_ITEM\n')
    assert old in s
    s = s.replace(old, '\tsetvar VAR_0x8004, ITEM_HM07   @ Moemon Kai: FireRed\'s HM07 (fix_hm_givers.py)\n'
                       '\tgoto EventScript_FindHmOnce\n')
    open(path, 'w', encoding='utf-8').write(s)
    print('Kanto_FourIsland_IcefallCave_1F: ITEM_HM07')


if __name__ == '__main__':
    main()
