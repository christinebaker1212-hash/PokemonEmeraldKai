#!/usr/bin/env python3
"""life_watch: learned, nothing new, nothing new, then (half an hour later) learned again.

Usage: check_watch.py RESULT_DIR"""
import sys, os
got = [l.split('life interact: watched, ')[1].strip() for l in open(os.path.join(sys.argv[1], 'a.log'), errors='replace')
       if 'life interact: watched, ' in l]
want = ['learned something', 'nothing new', 'nothing new', 'learned something']
print('watched: %s' % got)
sys.exit(0 if got == want else 1)
