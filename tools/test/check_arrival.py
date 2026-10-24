#!/usr/bin/env python3
"""life_arrival: the follower reacts to arriving only in a different area (PETALBURG: a curious look; back on
Route 104, known by then: nothing), never for MR. BRINEY's house (the same area) or at the start.

Usage: check_arrival.py RESULT_DIR"""
import sys, os, re

lines = [l.strip() for l in open(os.path.join(sys.argv[1], 'a.log'), errors='replace') if 'life arrival:' in l]
got = [re.sub(r'^.*life arrival: ', '', l) for l in lines]
want = ['new area 7 (emote 4)', 'new area 19 (emote 255)']
print('arrivals: %s' % got)
sys.exit(0 if got == want else 1)
