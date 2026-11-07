#!/usr/bin/env python3
"""evo_gender: each slot's species and gender before (first line per slot) and after (last line)"""
import re, sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
seen = {}
for slot, sp, g in re.findall(r'\[test\] mon (\d+): species (\d+) personality \w+ nature \d+ gender (\d+)', log):
    seen.setdefault(int(slot), []).append((int(sp), int(g)))
F, M = 254, 0
want = {   # slot: (species before, species after, gender throughout)
    0: (702, 703, F),   # ESPURR -> MEOWSTIC, female
    1: (940, 941, F),   # LECHONK -> OINKOLOGNE, female
    2: (782, 782, M),   # a male SALANDIT stays (only females evolve)
    3: (927, 927, F),   # a female BASCULEGION is female
    4: (901, 901, F),   # a female INDEEDEE is female
    5: (702, 703, M),   # ESPURR -> MEOWSTIC, male
}
bad = 0
for slot, (a, b, g) in want.items():
    rows = seen.get(slot, [])
    ok = len(rows) >= 2 and rows[0] == (a, g) and rows[-1] == (b, g)
    print('slot %d: %s -> %s %s' % (slot, rows[0] if rows else '?', rows[-1] if rows else '?', 'ok' if ok else 'WRONG'))
    bad |= not ok
sys.exit(bad)
