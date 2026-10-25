#!/usr/bin/env python3
"""garden_places: every type (TNEED_* 1-17) found one of its own places (a zone of src/mp/mp_garden_map.h open at
the group's badges) and walked all the way to it, with no badges and then with eight.

Usage: check_garden_places.py RESULT_DIR"""
import os, re, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
zones = []
for m in re.finditer(r'\{TNEED_(\w+), (\d+), (\d+), (\d+), (\d+), (\d+), (-?\d+), (-?\d+)\}',
                     open(os.path.join(ROOT, 'src/mp/mp_garden_map.h')).read()):
    zones.append((m.group(1),) + tuple(int(v) for v in m.groups()[1:]))
names = ['NONE', 'COZY', 'SPAR', 'SKY', 'DAMP', 'DIG', 'STONE', 'FOLIAGE', 'SHADE', 'SHINE', 'BASK', 'SOAK', 'SUN',
         'STORM', 'STILL', 'COOL', 'HEIGHT', 'NIGHT']

log = open(os.path.join(sys.argv[1], 'a.log'), errors='replace').read().split('\n')
badges, ok, rounds = 0, True, {0: set(), 8: set()}
pending = None
for line in log:
    m = re.search(r'garden here 1 growth (\d+) badges', line)
    if m:
        badges = int(m.group(1))
    m = re.search(r'garden place need (\d+) -> (-?\d+),(-?\d+) zone (-?\d+)', line)
    if m:
        need, x, y, z = (int(v) for v in m.groups())
        if z < 0 or z >= len(zones):
            print('need %s: no place of its own (zone %d)' % (names[need], z))
            ok = False
            continue
        zn, x0, y0, x1, y1, zb, fx, fy = zones[z]
        if zn != names[need] or zb > badges or not (x0 <= x <= x1 and y0 <= y <= y1):
            print('need %s at %d badges: zone %d (%s, %d badges) does not fit %d,%d' % (names[need], badges, z, zn, zb, x, y))
            ok = False
        pending = (need, x, y)
        continue
    m = re.search(r'garden: \d+ at its place for (\d+): (\d+),(\d+)(.*)', line)
    if m and pending:
        need, x, y = int(m.group(1)), int(m.group(2)), int(m.group(3))
        if (need, x, y) != pending or m.group(4).strip():
            print('need %s: arrived %d,%d%s, planned %s' % (names[need], x, y, m.group(4), pending))
            ok = False
        else:
            rounds.setdefault(badges, set()).add(need)
        pending = None
for b in (0, 8):
    missing = [names[n] for n in range(1, 18) if n not in rounds.get(b, set())]
    print('%d badges: %d of 17 places reached%s' % (b, 17 - len(missing), (' (missing %s)' % ', '.join(missing)) if missing else ''))
    ok = ok and not missing
sys.exit(0 if ok else 1)
