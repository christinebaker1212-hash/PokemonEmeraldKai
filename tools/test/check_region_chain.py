#!/usr/bin/env python3
"""Checks a region chain run (tools/test/gen_region_chain.py) step by step from its `mp regioninfo` lines:
start, region 1's badges, its League, the arrival in region 2, its badges, its League, the arrival in region 3, its
badges, its League. Levels: the Lv 70 POKeMON is Lv 5 on each first arrival and has its best level after each League;
the others stayed behind in the GLOBAL PC; the GLOBAL PC is linked only where the League is beaten.

    python3 tools/test/check_region_chain.py A B C BEST RESULT_DIR
"""
import os
import re
import sys

a, b, c, best = (int(x) for x in sys.argv[1:5])
d = sys.argv[5]
LEAD = 280
info = []
for line in open(os.path.join(d, 'a.log'), errors='replace'):
    m = re.match(r'\[test\] region (\d+) cleared (\d+) started (\d+) global free (\d+) party:(.*) box1 .* badges (\d+) '
                 r'linked (\d+)', line)
    if m:
        party = [tuple(int(v) for v in p.split(':L')) for p in m.group(5).split()]
        info.append({'region': int(m.group(1)), 'cleared': int(m.group(2)), 'free': int(m.group(4)), 'party': party,
                     'badges': int(m.group(6)), 'linked': int(m.group(7))})
names = ['start', 'badges 1', 'League 1', 'arrival 2', 'badges 2', 'League 2', 'arrival 3', 'badges 3', 'League 3']
if len(info) != len(names):
    sys.exit('FAIL: %d regioninfo lines, not %d (the run stopped after "%s")' % (len(info), len(names),
             names[len(info) - 1] if info else 'nothing'))
s = dict(zip(names, info))
bad = []


def want(step, ok, what):
    if not ok:
        bad.append('%s: %s (%s)' % (step, what, s[step]))


def lead(step):
    return [lv for sp, lv in s[step]['party'] if sp == LEAD]


want('start', s['start']['region'] == a and s['start']['linked'] == 0 and lead('start') == [70], 'region %d, Lv 70, not linked' % a)
want('badges 1', s['badges 1']['badges'] == 8 and s['badges 1']['linked'] == 0, '8 badges, the GLOBAL PC not linked yet')
# (HOENN's League is FLAG_SYS_GAME_CLEAR, not a bit in "cleared": "linked" says it for every region)
want('League 1', s['League 1']['linked'] == 1 and lead('League 1') == [70], 'cleared (linked), still Lv 70')
want('arrival 2', s['arrival 2']['region'] == b and s['arrival 2']['party'] == [(LEAD, 5)], 'only the Lv 70 one, at Lv 5')
want('arrival 2', s['arrival 2']['free'] == s['League 1']['free'] - 2, 'the other two in the GLOBAL PC')
want('arrival 2', s['arrival 2']['linked'] == 0, 'the GLOBAL PC not linked before this League')
want('badges 2', s['badges 2']['badges'] == 8, '8 badges')
want('League 2', s['League 2']['linked'] == 1, 'cleared (linked)')
want('League 2', lead('League 2') == [best], 'its best level, %d' % best)
want('arrival 3', s['arrival 3']['region'] == c and lead('arrival 3') == [5], 'Lv 5 in the third region')
want('badges 3', s['badges 3']['badges'] == 8, '8 badges')
want('League 3', (s['League 3']['cleared'] & 6) == 6 and s['League 3']['linked'] == 1, 'JOHTO and KANTO cleared, linked')
want('League 3', lead('League 3') == [best], 'its best level, %d' % best)
for x in bad:
    print('FAIL', x)
print('%d steps checked: %s' % (len(names), 'ok' if not bad else '%d problems' % len(bad)))
sys.exit(1 if bad else 0)
