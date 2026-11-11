#!/usr/bin/env python3
import os
"""orre_battle: the foes are WILLIE's two ZIGZAGOON (species 288) at Lv 24, and the prize is Colosseum's: 5 x 24 x 2"""
import re
import sys

log = open(sys.argv[1] + '/a.log', errors='replace').read()
ok = True
m = re.findall(r'\[test\] money (\d+)', log)
# (a day's first battle can finish a daily goal, which pays on top: "daily: goal N done")
daily = 300 * len(re.findall(r'daily: goal \d+ done', log))
if not m or int(m[-1]) not in (240, 240 + daily):
    print('FAIL prize: %s (want 240)' % (m[-1] if m else 'none'))
    ok = False
else:
    print('ok   prize 240')
sys.exit(0 if ok else 1)
