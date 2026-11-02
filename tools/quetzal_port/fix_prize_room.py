#!/usr/bin/env python3
"""Prizes Quetzal's scripts named by its own species / item numbers, which are other POKéMON and items here.

    python3 tools/quetzal_port/fix_prize_room.py      (safe to run again)

import.py turned Quetzal's species and items into names where a script command takes them, but a number first put
in a variable (setvar VAR_TEMP_1, 0x3BA) stayed Quetzal's: its "ALOLAN VULPIX" gave a DOLLIV, its "Expert Belt" a
WACAN BERRY, the vending machines MAX ETHERs. Now:
- CELADON's GAME CORNER prize room is FireRed's (KANTO: FireRed before Quetzal; pokefirered
  data/maps/CeladonCity_GameCorner_PrizeRoom/scripts.inc): POKéMON at FireRed's levels, TMs, items, FireRed's prices.
- GOLDENROD's GAME CORNER prize clerk is Crystal's (pokecrystal maps/GoldenrodGameCorner.asm): ABRA, CUBONE,
  WOBBUFFET at Crystal's levels and prices.
- The menus' words are FireRed's / Crystal's (tools/quetzal_port/gen_multichoice.py PRIZE_MENUS), and B leaves them.
- The vending machines (CELADON's roof, JOHTO's) sell FRESH WATER, SODA POP and LEMONADE.
- CINNABAR's chamber gives the RED ORB and the BLUE ORB it speaks of.
"""
import os
import re

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
MARK = '\t@ Moemon Kai: the original game\'s prize (tools/quetzal_port/fix_prize_room.py)\n'

ROOMS = {   # map: (label prefix, rows: (branch, prize, coins[, level]), the give line, after giving)
    'Kanto_CeladonCity_GameCorner_PrizeRoom': [
        [('841CF29', 'SPECIES_ABRA', 180, 9), ('841CF39', 'SPECIES_CLEFAIRY', 500, 8),
         ('841CF49', 'SPECIES_DRATINI', 2800, 18), ('841CF59', 'SPECIES_SCYTHER', 5500, 25),
         ('841CF69', 'SPECIES_PORYGON', 9999, 26),
         ('841D103', 'ITEM_TM13', 4000), ('841D113', 'ITEM_TM23', 3500), ('841D123', 'ITEM_TM24', 4000),
         ('841D133', 'ITEM_TM30', 4500), ('841D143', 'ITEM_TM35', 4000),
         ('841D25F', 'ITEM_SMOKE_BALL', 800), ('841D26F', 'ITEM_MIRACLE_SEED', 1000),
         ('841D27F', 'ITEM_CHARCOAL', 1000), ('841D28F', 'ITEM_MYSTIC_WATER', 1000),
         ('841D29F', 'ITEM_YELLOW_FLUTE', 1600)],
        '\tgivemon VAR_TEMP_1, 0x18, ITEM_NONE\n', '\tgivemon VAR_TEMP_1, %d, ITEM_NONE\n', '841CFB5', '841CF26',
        ['MULTI_QZ_0x61', 'MULTI_QZ_0x62', 'MULTI_QZ_0x5F'], {}],
    'Johto_GoldenrodGameCorner': [
        [('83D5204', 'SPECIES_ABRA', 100, 5), ('83D5213', 'SPECIES_CUBONE', 800, 15),
         ('83D5222', 'SPECIES_WOBBUFFET', 1500, 15)],
        '\tqz_givemon_ex VAR_TEMP_1, 0x14, ITEM_NONE, 2, 0, 0\n', '\tqz_givemon_ex VAR_TEMP_1, %d, ITEM_NONE, 2, 0, 0\n',
        '83D529A', '83D5297', ['MULTI_QZ_0x4C'],
        # Crystal's three: the menu's other rows (Quetzal's six) close it
        {3: '83D5297', 4: '83D5297', 5: '83D5297'}],
}

VALUES = [   # file, (old line, new line)...
    ('data/maps/Kanto_CeladonCity_DepartmentStore_Roof/scripts.inc',
     [('setvar VAR_TEMP_0, 0x23', 'setvar VAR_TEMP_0, ITEM_FRESH_WATER'),
      ('setvar VAR_TEMP_0, 0x24', 'setvar VAR_TEMP_0, ITEM_SODA_POP'),
      ('setvar VAR_TEMP_0, 0x25', 'setvar VAR_TEMP_0, ITEM_LEMONADE')]),
    ('data/scripts/qz/johto_shared.inc',
     [('setvar VAR_TEMP_0, 0x23', 'setvar VAR_TEMP_0, ITEM_FRESH_WATER'),
      ('setvar VAR_TEMP_0, 0x24', 'setvar VAR_TEMP_0, ITEM_SODA_POP'),
      ('setvar VAR_TEMP_0, 0x25', 'setvar VAR_TEMP_0, ITEM_LEMONADE')]),
    ('data/maps/Kanto_CinnabarIsland_PrimalChamber/scripts.inc',
     [('setvar VAR_0x8000, 0x122', 'setvar VAR_0x8000, ITEM_RED_ORB'),
      ('setvar VAR_0x8000, 0x123', 'setvar VAR_0x8000, ITEM_BLUE_ORB')]),
]


def rooms():
    for name, (rows, give, give_lv, after, close, menus, closed_rows) in ROOMS.items():
        path = os.path.join(REPO, 'data/maps/%s/scripts.inc' % name)
        p = '%s_EventScript_' % name
        s = open(path).read()
        for row in rows:
            label, prize, coins = row[:3]
            m = re.search(re.escape(p + label) + r'::\n(?:\t@ Moemon Kai[^\n]*\n)?\tsetvar VAR_TEMP_1, \w+\n'
                          r'\tsetvar VAR_TEMP_2, \w+\n', s)
            if not m:
                raise SystemExit('%s: no %s' % (name, label))
            s = s[:m.start()] + '%s%s::\n%s\tsetvar VAR_TEMP_1, %s\n\tsetvar VAR_TEMP_2, %d\n' % (
                p, label, MARK, prize, coins) + s[m.end():]
        mons = [r for r in rows if r[1].startswith('SPECIES_')]
        if give in s:   # each POKéMON at its own level (the give command takes a fixed one)
            s = s.replace(give, MARK + ''.join('\tgoto_if_eq VAR_TEMP_1, %s, %sGive_%s\n' % (sp, p, sp[8:])
                                               for _, sp, _, _ in mons) + '\tgoto %s%s\n' % (p, close))
            s += ''.join(('\n%sGive_%s::\n' + give_lv + '\tgoto %s%s\n') % (p, sp[8:], lv, p, after)
                         for _, sp, _, lv in mons)
        for menu in menus:   # B leaves the menu, as in the original games
            s = re.sub(r'(\tmultichoice \w+, \w+, %s), 1\n' % menu, r'\1, 0\n', s)
        for row, target in closed_rows.items():
            s = re.sub(r'(\tgoto_if_eq VAR_0x8000, %d, )%s\w+\n' % (row, re.escape(p)), r'\g<1>%s%s\n' % (p, target), s)
        open(path, 'w').write(s)


def values():
    for f, pairs in VALUES:
        path = os.path.join(REPO, f)
        s = open(path).read()
        for old, new in pairs:
            s = s.replace('\t' + old + '\n', MARK + '\t' + new + '\n')
        open(path, 'w').write(s)


if __name__ == '__main__':
    rooms()
    values()
