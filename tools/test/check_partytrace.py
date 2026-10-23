#!/usr/bin/env python3
# usage: check_partytrace.py DIR
# A co-op Triple Battle traced with MOEMON_PARTYTRACE=1: on every game and at every change, the trainer's three
# battlers (1, 3, 5) never hold the same party slot, nothing points past the party (index 6+ outside a pending
# switch), each player's battler 2i only ever holds that player's own slots (2i, 2i+1), and no Pokemon is a BAD EGG.
import re, sys, glob, os
d = sys.argv[1]
bad = 0
lines = 0
for log in sorted(glob.glob(os.path.join(d, '[a-d].log'))):
    for n, line in enumerate(open(log, errors='replace'), 1):
        if 'partytrace:' not in line:
            continue
        lines += 1
        if 'BADEGG' in line:
            print(f'{os.path.basename(log)}:{n}: {line.strip()}')
            bad += 1
        m = re.search(r'battlers ([\d ]+), switching to ([\d ]+), absent ([0-9a-f]+)', line)
        if not m:
            continue
        idx = [int(x) for x in m.group(1).split()]
        absent = int(m.group(3), 16)
        if idx == [0] * 6:
            continue   # not set up yet
        foes = [idx[b] for b in (1, 3, 5) if not absent & (1 << b)]
        problems = []
        if any(i >= 6 for i in idx):
            problems.append('a battler past the party')
        if len(set(foes)) != len(foes):
            problems.append('two foes on the same slot')
        for seat in range(3):
            b = seat * 2
            if not absent & (1 << b) and idx[b] // 2 != seat:
                problems.append(f'battler {b} holds slot {idx[b]} (not its player\'s)')
        if problems:
            print(f'{os.path.basename(log)}:{n}: {", ".join(problems)}: {line.strip()}')
            bad += 1
if lines == 0:
    print('no partytrace lines')
    bad += 1
print('partytrace:', 'ok' if bad == 0 else f'{bad} problems', f'({lines} lines)')
sys.exit(1 if bad else 0)
