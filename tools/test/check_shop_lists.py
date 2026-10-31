#!/usr/bin/env python3
"""Checks `mp shopdump` (tools/test/scenarios/shop_lists_gen*): JOHTO's GOLDENROD and town marts on Crystal's lists,
KANTO's on FireRed / LeafGreen's, nothing from before GEN 4 that those games didn't sell, and the newer items only
with GENERATION and badges.

    python3 tools/test/check_shop_lists.py GEN RESULT_DIR
"""
import re
import sys

gen, d = int(sys.argv[1]), sys.argv[2]
names = {}
for line in open('include/constants/items.h'):
    m = re.match(r'#define ITEM_(\w+) (\d+)$', line.strip())
    if m and int(m.group(2)) not in names:
        names[int(m.group(2))] = m.group(1)
maps = {}
for line in open('include/constants/map_groups.h'):
    m = re.match(r'\s*MAP_(\w+)\s*=\s*\((\d+) \| \((\d+) << 8\)\)', line)
    if m:
        maps[int(m.group(2)) | int(m.group(3)) << 8] = m.group(1)
lists = {}
for line in open(d + '/a.log', errors='replace'):
    m = re.match(r'\[test\] shop ([0-9a-f]+) (\d+):(.*)', line)
    if m:
        first = int(m.group(2))
        key = (maps[int(m.group(1), 16)], 'REGION' if first == 65534 else 'ANY' if first == 65535 else names[first])
        lists.setdefault(key, [names.get(int(x), x) for x in m.group(3).split()])
bad = []


def want(key, ok, what):
    if not ok:
        bad.append('%s %s: %s (%s)' % (key[0], key[1], what, ' '.join(map(str, lists.get(key, ['missing'])))))


CRYSTAL_X = 'X_SPEED X_SPECIAL X_DEFEND X_ATTACK DIRE_HIT GUARD_SPEC X_ACCURACY'.split()
CRYSTAL_MEDS = 'POTION SUPER_POTION ANTIDOTE PARALYZE_HEAL AWAKENING BURN_HEAL ICE_HEAL'.split()
k = ('JOHTO_GOLDENROD_DEPT_STORE2F', 'X_SPEED')
want(k, lists[k][:7] == CRYSTAL_X, "Crystal's X items first")
k = ('JOHTO_GOLDENROD_DEPT_STORE2F', 'REGION')
want(k, lists[k] == CRYSTAL_MEDS, "Crystal's 2F medicine, nothing else")
k = ('JOHTO_GOLDENROD_DEPT_STORE3F', 'HP_UP')
want(k, lists[k] == 'PROTEIN IRON CARBOS CALCIUM HP_UP'.split(), "Crystal's 4F vitamins")
k = ('KANTO_VIRIDIAN_CITY_MART', 'REGION')
want(k, lists[k] == 'POKE_BALL POTION ANTIDOTE PARALYZE_HEAL'.split(), "FRLG's Viridian")
k = ('KANTO_CELADON_CITY_DEPARTMENT_STORE_2F', 'BLUE_FLUTE')
want(k, lists[k][:6] == 'TM05 TM15 TM28 TM31 TM43 TM45'.split(), "FRLG's Celadon 2F TMs")
older = ('QUICK_CLAW', 'KINGS_ROCK', 'SHELL_BELL', 'MENTAL_HERB', 'WHITE_HERB', 'MACHO_BRACE', 'RARE_CANDY')
for key, items in lists.items():
    if key[0].startswith(('JOHTO_GOLDENROD', 'KANTO_CELADON')):
        want(key, not set(items) & set(older), 'no older item those games didn\'t sell')
if gen == 3:
    for key in [('JOHTO_GOLDENROD_DEPT_STORE3F', 'MACHO_BRACE'), ('JOHTO_GOLDENROD_DEPT_STORE4F', 'WATER_STONE'),
                ('JOHTO_GOLDENROD_DEPT_STORE5F', 'NONE')]:
        want(key, lists[key] == [], "nothing at GEN 3 (Quetzal's own clerk)")
    k = ('JOHTO_GOLDENROD_DEPT_STORE2F', 'X_SPEED')
    want(k, lists[k] == CRYSTAL_X, "only Crystal's at GEN 3")
else:
    k = ('JOHTO_GOLDENROD_DEPT_STORE3F', 'MACHO_BRACE')
    want(k, 'POWER_BRACER' in lists[k] and 'LIFE_ORB' in lists[k], 'training and battle items at GEN 9, 8 badges')
    k = ('JOHTO_GOLDENROD_DEPT_STORE5F', 'NONE')
    want(k, 'VENUSAURITE' in lists[k], 'MEGA STONES at GEN 9, 8 badges')
    k = ('JOHTO_MAHOGANY_MART1F', 'PEWTER_CRUNCHIES')
    want(k, lists[k][:2] == ['GREAT_BALL', 'SUPER_POTION'] and 'ICY_ROCK' in lists[k], "Crystal's later MAHOGANY list")
for x in bad:
    print('FAIL', x)
print('%d shop lists checked at GEN %d: %s' % (len(lists), gen, 'ok' if not bad else '%d problems' % len(bad)))
sys.exit(1 if bad else 0)
