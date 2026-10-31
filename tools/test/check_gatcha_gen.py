#!/usr/bin/env python3
"""MoeGatcha banners by GENERATION (mp_gatcha.c GatchaGen): `mp gatchadump` lists every banner's prizes. At GEN 3 the
banners are the MoeGatcha as it always was, with nothing from a later generation; at a later GENERATION its Moemon and
items join (STANDARD's daily picks, PICKUPs, held items, MEGA STONES from 6, Z-CRYSTALS from 7).

    python3 tools/test/check_gatcha_gen.py GEN RESULT_DIR
"""
import os
import re
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
gen, d = int(sys.argv[1]), sys.argv[2]
src = open(os.path.join(REPO, 'include/constants/species.h')).read()
num = {m.group(1): int(m.group(2)) for m in re.finditer(r'#define SPECIES_(\w+) (\d+)\b', src)}
log = open(os.path.join(d, 'a.log'), errors='replace').read()
lines = [l for l in log.split('\n') if l.startswith('[test] gatchadump ')]
if not lines:
    sys.exit('no gatchadump lines')
head = lines[0]
m = re.search(r'gen (\d+) day \d+ pickup \d+ of (\d+) allowed', head)
if not m or int(m.group(1)) != gen:
    sys.exit('dumped at the wrong GENERATION: ' + head)
pools = {}
for l in lines[1:]:
    name, items = l[len('[test] gatchadump '):].split(':', 1)
    pools[name] = items.split()
species = [int(x[1:].rstrip('*')) for v in pools.values() for x in v if x.startswith('m')]
items = [int(x[1:]) for v in pools.values() for x in v if x.startswith('i')]
bad = []
if gen == 3:
    if m.group(2) != '19':
        bad.append('GEN 3 has %s PICKUPs, not the 19 it always had' % m.group(2))
    bad += ['later Moemon %d' % s for s in species if s > num['CHIMECHO']]
    bad += ['later item %d' % i for i in items if 389 <= i <= 396 or i >= 400]
    if len(pools['STANDARD UR']) != 6:
        bad.append("GEN 3's STANDARD UR has %d legends, not 6" % len(pools['STANDARD UR']))
else:
    later = [s for s in pools['STANDARD UR'] if int(s[1:].rstrip('*')) > num['CHIMECHO']]
    if len(later) != gen - 3:
        bad.append('STANDARD UR has %d later legends at GEN %d (one per later generation)' % (len(later), gen))
    want = {418: 4, 544: 6, 553: 6, 600: 7, 539: 9, 441: 4}   # LIFE ORB, ASSAULT VEST, a MEGA STONE, a Z-CRYSTAL...
    for i, g in want.items():
        if (g <= gen) != (i in items):
            bad.append('item %d (GEN %d) %s at GEN %d' % (i, g, 'missing' if g <= gen else 'there', gen))
    if int(m.group(2)) != 19 + 2 * (gen - 3):
        bad.append('%s PICKUPs at GEN %d, not %d' % (m.group(2), gen, 19 + 2 * (gen - 3)))
for b in bad:
    print('FAIL', b)
print('%d prizes checked at GEN %d: %s' % (len(species) + len(items), gen, 'ok' if not bad else '%d problems' % len(bad)))
sys.exit(1 if bad else 0)
