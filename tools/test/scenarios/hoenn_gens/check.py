#!/usr/bin/env python3
"""HOENN's tables by GENERATION: at GEN 9 ROUTE 101's rolls bring newer POKéMON (ids past 411), at GEN 3 none."""
import re, sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
rolls = [list(map(int, l.split()[2:])) for l in re.findall(r'^\[test\] wildroll:.*$', log, re.M)]
assert len(rolls) == 2 and rolls[0] and rolls[1], 'no rolls'
new9 = sorted({s for s in rolls[0] if s > 411}); new3 = [s for s in rolls[1] if s > 411]
print('GEN 9 newer species:', new9, 'GEN 3:', new3)
assert len(new9) >= 3, 'GEN 9 brought no newer POKéMON to ROUTE 101'
assert not new3, 'GEN 3 has newer POKéMON'
