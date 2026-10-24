#!/usr/bin/env python3
"""Checks MOEMON_FOLLOWTRACE output (mp_follower.c): the local follower never jumps.

Usage: check_followtrace.py RESULT_DIR [MAX_PIXELS]
Reads RESULT_DIR/a.log; fails if the follower's position moves more than MAX_PIXELS (default 12) between two
frames, except across a map change (a warp). Prints the largest step seen."""
import sys, os, re

d = sys.argv[1]
limit = int(sys.argv[2]) if len(sys.argv) > 2 else 12
prev = None
worst = 0
bad = []
n = 0
for line in open(os.path.join(d, 'a.log'), errors='replace'):
    if '[test] warp' in line:
        prev = None
        continue
    m = re.search(r'\[test\] followtrace (-?\d+) (-?\d+) auto (\d) ret (\d)', line)
    if not m:
        continue
    x, y = int(m.group(1)), int(m.group(2))
    n += 1
    if prev is not None:
        step = abs(x - prev[0]) + abs(y - prev[1])
        worst = max(worst, step)
        if step > limit:
            bad.append((n, prev, (x, y), m.group(3), m.group(4)))
    prev = (x, y)
print('followtrace: %d frames, largest step %d px' % (n, worst))
for b in bad[:10]:
    print('  jump at trace frame %d: %s -> %s (auto %s ret %s)' % b)
sys.exit(1 if bad or n == 0 else 0)
