#!/usr/bin/env python3
"""UNOVA's items (docs/UNOVA_DECISIONS.md: an item the game already has is the game's; a new one is Black's): Black's
item numbers as the game's, matched by name (Black's item names, bank 54, against src/data/items.h: letters only, case
and spaces aside; the game's names are cut to 13 letters, so a Black name may match a game name it starts with), into
src/data/unova_items.h (ITEM_NONE where the game has no such item yet), with each item's pocket in Black's bag
(a/0/2/4's records: 0 ITEMS, 1 MEDICINE, 2 TMs & HMs, 3 BERRIES, 4 KEY ITEMS; the names are bank 55).

    python3 -I tools/unova/gen_items.py          (writes the table, prints Black's items the game lacks)
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import narc  # noqa: E402
import text as tx  # noqa: E402

BANK_ITEM_NAMES = 54
ITEM_DATA_ARC = 'a/0/2/4'


def pockets(count):
    """Black's pocket of each item (ItemRecord: u16 at 8, bits 7-10)"""
    import struct
    recs = narc.arc(ITEM_DATA_ARC)
    return [(struct.unpack_from('<H', recs[i], 8)[0] >> 7) & 0xF if i < len(recs) and len(recs[i]) >= 10 else 0
            for i in range(count)]
# names the two games write differently (Black's -> the game's)
ALIASES = {'THUNDERSTONE': 'THUNDERSTONE', 'PARLYZHEAL': 'PARALYZEHEAL', 'XSPDEF': 'XSPDEF', 'XSPECIAL': 'XSPECIAL',
           'XDEFEND': 'XDEFEND', 'XDEFENSE': 'XDEFEND', 'BICYCLE': 'BICYCLE',
           'BLACKGLASSES': 'BLACKGLASSES', 'DEEPSEATOOTH': 'DEEPSEATOOTH', 'DEEPSEASCALE': 'DEEPSEASCALE',
           'SILKSCARF': 'SILKSCARF', 'NEVERMELTICE': 'NEVERMELTICE', 'TWISTEDSPOON': 'TWISTEDSPOON',
           'BRIGHTPOWDER': 'BRIGHTPOWDER', 'SILVERPOWDER': 'SILVERPOWDER', 'ENERGYPOWDER': 'ENERGYPOWDER',
           'EXPSHARE': 'EXPSHARE', 'POKEBALL': 'POKEBALL', 'TOWNMAP': 'TOWNMAP',
           'DOWSINGMCHN': 'ITEMFINDER'}


def norm(name):
    name = name.upper().replace('É', 'E').replace('é', 'E')
    return re.sub(r'[^A-Z0-9]', '', name)


def game_items():
    """{normalized name: ITEM_ constant}: the game's constants (include/constants/items.h: ITEM_DUSK_STONE ...) and
    its names (src/data/items.h)"""
    out = {}
    consts = open(os.path.join(ROOT, 'include/constants/items.h'), encoding='utf-8').read()
    for c in re.findall(r'#define (ITEM_[A-Z0-9_]+)\s+\d+', consts):
        out.setdefault(norm(c[len('ITEM_'):]), c)
    src = open(os.path.join(ROOT, 'src/data/items.h'), encoding='utf-8').read()
    for m in re.finditer(r'\[(ITEM_[A-Z0-9_]+)\]\s*=\s*\{\s*\.name\s*=\s*_\("([^"]*)"\)', src):
        out.setdefault(norm(m.group(2)), m.group(1))
    return out


BANK_MOVE_NAMES = 203
MACHINE_MOVES = bytes(struct.pack('<4H', 468, 337, 473, 347))   # (Black's TM01-04: HONE CLAWS... where its table starts)
MOVE_ALIASES = {'VICEGRIP': 'VISEGRIP', 'HIJUMPKICK': 'HIGHJUMPKICK'}


def machine_moves():
    """Black's TM01-92, HM01-06, TM93-95 (items 328-425, 618-620): the move each teaches (arm9's table, as
    FILE_UTIL_GetMachineMove reads it)"""
    arm = open(os.path.join(os.path.dirname(narc.EXTRACT), 'arm9_dec.bin'), 'rb').read()
    at = arm.find(MACHINE_MOVES)
    return [struct.unpack_from('<H', arm, at + 2 * k)[0] for k in range(101)]


def game_machines():
    """{normalized move name: the game's TM / HM item}: TM01-50, HM01-08, TM51-92, ROCK CLIMB / DEFOG / WHIRLPOOL,
    TM93 on (include/constants/tms_hms*.h, items.h's order)"""
    out = {}
    lists = [('tms_hms.h', 'FOREACH_TM', 'ITEM_TM01'), ('tms_hms.h', 'FOREACH_HM', 'ITEM_HM01'),
             ('tms_hms_tm2.h', 'FOREACH_TM2', 'ITEM_TM51'), ('tms_hms_tm2.h', 'FOREACH_HM2', 'ITEM_HM_ROCK_CLIMB'),
             ('tms_hms_tm3.h', 'FOREACH_TM3', 'ITEM_TM93'), ('tms_hms_tm4.h', 'FOREACH_TM4', 'ITEM_TM140')]
    for f, macro, first in lists:
        text = open(os.path.join(ROOT, 'include/constants', f), encoding='utf-8').read()
        m = re.search(r'#define %s\(F\)(.*?)(?:\n\n|\n#|$)' % macro, text, re.S)
        for k, move in enumerate(re.findall(r'F\((\w+)\)', m.group(1))):
            out.setdefault(norm(move), '%s + %d' % (first, k) if k else first)
    return out


def main():
    names = tx.bank(narc.arc(tx.SYSTEM)[BANK_ITEM_NAMES])
    game = game_items()
    move_names = tx.bank(narc.arc(tx.SYSTEM)[BANK_MOVE_NAMES])
    machines, teaches = game_machines(), machine_moves()
    tm_items = list(range(328, 426)) + [618, 619, 620]
    rows, missing, labels = [], [], []
    for i, n in enumerate(names):
        key = ALIASES.get(norm(n), norm(n))
        item = game.get(key)
        if i in tm_items:   # (a TM is the move it teaches: the game's TM of that move, or none yet)
            move = norm(move_names[teaches[tm_items.index(i)]])
            item = machines.get(MOVE_ALIASES.get(move, move))
            n = '%s %s' % (n, move_names[teaches[tm_items.index(i)]])
        if item is None and len(key) > 13:
            item = next((v for k, v in game.items() if len(k) >= 10 and key.startswith(k)), None)
        if item is None and i and n not in ('???', 'None', ''):
            missing.append((i, n))
        rows.append(item or 'ITEM_NONE')
        labels.append(n)
    with open(os.path.join(ROOT, 'src/data/unova_items.h'), 'w') as f:
        f.write('// Generated by tools/unova/gen_items.py: Black\'s item numbers as the game\'s (by name; ITEM_NONE: not in the\n'
                '// game yet)\nstatic const u16 sUnovaItems[] = {\n')
        for i, r in enumerate(rows):
            f.write('    %s,   // %d %s\n' % (r, i, labels[i].replace('*/', '')))
        f.write('};\n')
        f.write('static const u8 sUnovaItemPockets[] = {   // (Black\'s bag pocket of each)\n')
        ps = pockets(len(rows))
        for k in range(0, len(ps), 32):
            f.write('    %s,\n' % ', '.join(str(x) for x in ps[k:k + 32]))
        f.write('};\n')
    print('%d of Black\'s items, %d not in the game yet' % (len(names), len(missing)))
    for i, n in missing:
        print('  %d %s' % (i, n))


if __name__ == '__main__':
    main()
