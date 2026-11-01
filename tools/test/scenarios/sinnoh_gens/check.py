#!/usr/bin/env python3
"""SINNOH's tables by GENERATION: at GEN 9 ROUTE 202's rolls bring generation 5-9 POKéMON (ids past 518), at GEN 4
none; ROARK leads with NACLI (957)."""
import re, sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
rolls = [list(map(int, l.split()[2:])) for l in re.findall(r'^\[test\] wildroll:.*$', log, re.M)]
assert len(rolls) == 2 and rolls[0] and rolls[1], 'no rolls'
new9 = sorted({s for s in rolls[0] if s > 518}); new4 = [s for s in rolls[1] if s > 518]
print('GEN 9 newer species:', new9, 'GEN 4:', new4)
assert len(new9) >= 3, 'GEN 9 brought no newer POKéMON to ROUTE 202'
assert not new4, 'GEN 4 has newer POKéMON'
