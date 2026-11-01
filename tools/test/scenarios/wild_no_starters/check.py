#!/usr/bin/env python3
"""No starter line in 4000 wild rolls on JOHTO's ROUTE 36 (GEN 3, GEN 9)."""
import re, sys
ids = {m.group(1): int(m.group(2)) for m in
       re.finditer(r'#define\s+SPECIES_(\w+)\s+(\d+)\s*$', open('include/constants/species.h').read(), re.M)}
body = open('src/mp/mp_region.c').read()
lines = re.search(r'sStarterLines\[\] = \{(.*?)\};', body, re.S).group(1)
starters = {ids[n] for n in re.findall(r'SPECIES_(\w+)', lines)}
log = open(sys.argv[1] + '/a.log', errors='replace').read()
rolls = [list(map(int, l.split()[2:])) for l in re.findall(r'^\[test\] wildroll:.*$', log, re.M)]
assert len(rolls) == 2 and len(rolls[0]) > 1500 and len(rolls[1]) > 1500, 'no rolls'
bad = [s for r in rolls for s in r if s in starters]
print('species seen:', sorted(set(rolls[0] + rolls[1])), 'starters:', bad[:10])
assert not bad, 'starters in the wild'
