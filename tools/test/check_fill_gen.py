#!/usr/bin/env python3
"""TEAM PADDING (FILL) respects the GENERATION: every Moemon in a padded trainer's team (a.log's "coop: trainer N party
a -> b: ..." lines, mp_coop.c MP_Coop_FinishTrainerParty / FinishTwoTrainerParty) is of that GENERATION or earlier.

    python3 tools/test/check_fill_gen.py GEN RESULT_DIR
"""
import os
import re
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
gen, d = int(sys.argv[1]), sys.argv[2]
src = open(os.path.join(REPO, 'include/constants/species.h')).read()
num = {m.group(1): int(m.group(2)) for m in re.finditer(r'#define SPECIES_(\w+) (\d+)\b', src)}
# Kai's species numbers: Gen 1-3 up to CHIMECHO, Gen 4 TURTWIG..ARCEUS, Gens 5-9 from VICTINI (mp_region.c MP_Gen_OfSpecies
# has the exact cut points; the gen 5-9 ones come from tools/moemon_gen59)
cut = {3: num['CHIMECHO'], 4: num['ARCEUS']}
limit = cut.get(gen, 10 ** 6)
seen = 0
for line in open(os.path.join(d, 'a.log'), errors='replace'):
    if 'coop: trainer' not in line or '->' not in line:
        continue
    part = line.split('->', 1)[1]
    if ':' in part:
        part = part.split(':', 1)[1]   # (one trainer: "party 2 -> 3: species..."; two: "-> a b c / d e f")
    for s in part.replace('/', ' ').split():
        k = int(s)
        if k == 0:
            continue
        seen += 1
        if k > limit:
            print('species %d is past GEN %d' % (k, gen))
            sys.exit(1)
if not seen:
    print('no padded team in a.log')
    sys.exit(1)
print('%d padded-team Moemon, all GEN %d or earlier' % (seen, gen))
