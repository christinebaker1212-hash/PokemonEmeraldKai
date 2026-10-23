#!/usr/bin/env python3
# items_d1: EVIOLITE cuts both kinds of damage taken by a Moemon that can evolve, and nothing else
import re
import sys

log = open(sys.argv[1] + '/a.log', errors='replace').read()
rows = {int(m[0]): (int(m[1]), [int(v) for v in m[2:]]) for m in re.findall(
    r'holdtest eviolite species (\d+) canevolve (\d): tackle (\d+) -> (\d+), water gun (\d+) -> (\d+)', log)}
bad = []
if set(rows) != {1, 3, 132}:
    bad.append('eviolite rows: %s' % sorted(rows))
else:
    can, (t0, t1, w0, w1) = rows[1]
    if not (can == 1 and t1 < t0 and w1 < w0 and abs(t1 * 3 - t0 * 2) <= 3 and abs(w1 * 3 - w0 * 2) <= 3):
        bad.append('BULBASAUR with EVIOLITE: %s' % (rows[1],))
    for sp in (3, 132):
        can, (t0, t1, w0, w1) = rows[sp]
        if can != 0 or t0 != t1 or w0 != w1:
            bad.append('species %d: %s' % (sp, rows[sp]))
print('\n'.join(bad) if bad else 'items_d1 ok')
sys.exit(1 if bad else 0)
