#!/usr/bin/env python3
"""KANTO's TMs where FireRed has them (docs/REGIONS_TODO.md A4: the other TM givers; the leaders' are
fix_leader_tms.py). Quetzal put held items and stones in their places; FireRed comes first in KANTO.

    python3 tools/quetzal_port/fix_kanto_tms.py      (run from anywhere; safe to run again)

- Item balls: FireRed's TM balls (pokefirered data/scripts/item_ball_scripts.inc, their places from each map's
  map.json) are Quetzal's balls at the same places (two moved: ROCKET HIDEOUT B2F's, the S.S. ANNE's), whose
  STD_FIND_ITEM item becomes the TM.
- People: the CERULEAN CITY grunt (TM28), ROUTE 12's little girl (TM27) and MR. PSYCHIC (TM29) give FireRed's TM
  with FireRed's words (pokefirered data/maps/<map>/text.inc, POKéMON spelt MOéMON, ' as ’).
What Quetzal gave there stays obtainable elsewhere (tools/moemon_gen59/item_audit.py).
"""
import os
import re

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

BALLS = [   # map, x, y (Quetzal's ball), TM
    ('FiveIsland_RocketWarehouse', 17, 3, 'ITEM_TM36'), ('MtMoon_1F', 11, 35, 'ITEM_TM09'),
    ('MtMoon_B2F', 35, 5, 'ITEM_TM46'), ('PokemonMansion_B1F', 6, 21, 'ITEM_TM22'),
    ('PokemonMansion_B1F', 23, 4, 'ITEM_TM14'), ('PowerPlant', 40, 22, 'ITEM_TM17'), ('PowerPlant', 46, 37, 'ITEM_TM25'),
    ('RocketHideout_B2F', 5, 13, 'ITEM_TM12'), ('RocketHideout_B3F', 19, 14, 'ITEM_TM21'),
    ('RocketHideout_B4F', 1, 6, 'ITEM_TM49'), ('Route12', 18, 36, 'ITEM_TM48'), ('Route15', 20, 6, 'ITEM_TM18'),
    ('Route24', 11, 4, 'ITEM_TM45'), ('Route25', 26, 2, 'ITEM_TM43'), ('Route4', 67, 5, 'ITEM_TM05'),
    ('Route9', 12, 17, 'ITEM_TM40'), ('SSAnne_1F_Room2', 2, 7, 'ITEM_TM31'), ('SSAnne_B1F_Room2', 3, 2, 'ITEM_TM44'),
    ('SafariZone_East', 31, 18, 'ITEM_TM11'), ('SafariZone_North', 28, 9, 'ITEM_TM47'),
    ('SafariZone_West', 17, 13, 'ITEM_TM32'), ('SilphCo_4F', 30, 18, 'ITEM_TM41'), ('SilphCo_5F', 1, 18, 'ITEM_TM01'),
    ('SilphCo_7F', 30, 11, 'ITEM_TM08'), ('VictoryRoad_1F', 14, 1, 'ITEM_TM02'), ('VictoryRoad_2F', 40, 7, 'ITEM_TM07'),
    ('VictoryRoad_2F', 14, 13, 'ITEM_TM37'), ('VictoryRoad_3F', 12, 9, 'ITEM_TM50'),
]

PEOPLE = {   # map: ([(Quetzal's item, TM)...], {text label: FireRed's words})
    'CeruleanCity': ([('ITEM_SUN_STONE', 'ITEM_TM28')], {
        '842EF62': ['…Okay.\\n', "I'll return the TM I stole.$"],
        '842EF8C': ['{PLAYER} recovered TM28 from\\n', 'the TEAM ROCKET GRUNT.$'],
    }),
    'Route12_NorthEntrance_2F': ([('ITEM_SHINY_STONE', 'ITEM_TM27')], {
        '844F283': ["My POKéMON's ashes are stored in\\n", 'POKéMON TOWER.\\p', 'You can have this TM.\\n',
                    "I don't need it anymore…$"],
        '844F2EA': ["My POKéMON's ashes are stored in\\n", 'POKéMON TOWER.\\p', 'You can have this TM.\\n',
                    "I don't need it anymore…$"],
        '844F351': ['{PLAYER} received TM27\\n', 'from the little girl.$'],
        '844F381': ['TM27 is a move called RETURN…\\p', 'If you treat your POKéMON good,\\n',
                    'it will return your love by working\\l', 'its hardest in battle.$'],
    }),
    'SaffronCity_MrPsychicsHouse': ([('ITEM_MOON_STONE', 'ITEM_TM29')], {
        '844C0C7': ['{PLAYER} received TM29\\n', 'from MR. PSYCHIC.$'],
        '844C0F2': ["You already know, don't you?\\n", 'TM29 is PSYCHIC.\\p', "It's an attack that may also lower\\n",
                    "the target's special stats.$"],
    }),
    # the thirsty girl on the roof: FRESH WATER, SODA POP, LEMONADE
    'CeladonCity_DepartmentStore_Roof': ([('ITEM_SWEET_APPLE', 'ITEM_TM16'), ('ITEM_TART_APPLE', 'ITEM_TM20'),
                                          ('ITEM_SYRUPY_APPLE', 'ITEM_TM33')], {
        '8445F1E': ['TM16 contains LIGHT SCREEN.\\p', "It's a move that weakens the power\\n",
                    'of special attacks by your foe.$'],
        '8445F84': ['TM20 contains SAFEGUARD.\\p', "It's a move that prevents status\\n",
                    'problems among your POKéMON.$'],
        '8445FE9': ['TM33 contains REFLECT.\\p', "It's a move that weakens the power\\n",
                    'of physical attacks by your foe.$'],
    }),
}

MARK = '@ Moemon Kai: FireRed\'s TM (tools/quetzal_port/fix_kanto_tms.py)'


def kai_words(s):
    return s.replace('POKéMON', 'MOéMON').replace("'", '’')


def balls():
    import json
    for name, x, y, tm in BALLS:
        d = os.path.join(REPO, 'data/maps/Kanto_' + name)
        objs = json.load(open(os.path.join(d, 'map.json')))['object_events']
        script = [o['script'] for o in objs if o['x'] == x and o['y'] == y][0]
        path = os.path.join(d, 'scripts.inc')
        s = open(path).read()
        m = re.search(re.escape(script) + r'::\n(\t@ Moemon Kai[^\n]*\n)?\tsetorcopyvar VAR_0x8000, (ITEM_\w+)\n'
                      r'\tsetorcopyvar VAR_0x8001, 1\n\tcallstd STD_FIND_ITEM\n', s)
        if not m:
            raise SystemExit('%s: no item ball at %d,%d' % (name, x, y))
        if m.group(2) == tm:
            continue
        s = s[:m.start()] + ('%s::\n\t%s (Quetzal had %s here)\n\tsetorcopyvar VAR_0x8000, %s\n'
                             '\tsetorcopyvar VAR_0x8001, 1\n\tcallstd STD_FIND_ITEM\n') % (script, MARK, m.group(2), tm) \
            + s[m.end():]
        open(path, 'w').write(s)
        print(name, tm, 'for', m.group(2))


def people():
    for name, (swaps, texts) in PEOPLE.items():
        path = os.path.join(REPO, 'data/maps/Kanto_%s/scripts.inc' % name)
        s = open(path).read()
        if MARK in s:
            continue
        first = min(s.index(old) for old, _ in swaps)
        line = s.rindex('\n', 0, first) + 1
        s = s[:line] + '\t%s (Quetzal gave %s)\n' % (MARK, ', '.join(o.replace('ITEM_', '') for o, _ in swaps)) + s[line:]
        for old, tm in swaps:
            s = s.replace(old, tm)
        for label, lines in texts.items():
            full = 'Kanto_%s_Text_%s' % (name, label)
            t = re.search(re.escape(full) + r'::\n((?:\t\.string .*\n)+)', s)
            if not t:
                raise SystemExit('%s: no %s' % (name, full))
            s = s[:t.start(1)] + ''.join('\t.string "%s"\n' % kai_words(l) for l in lines) + s[t.end(1):]
        open(path, 'w').write(s)
        print(name, ', '.join('%s for %s' % (tm, old) for old, tm in swaps))


if __name__ == '__main__':
    balls()
    people()
