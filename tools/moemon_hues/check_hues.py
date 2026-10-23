#!/usr/bin/env python3
"""Test check (hue_math): the game's [hue] lines against the Stadium hue page's JavaScript (reference.js), and the
Gen 1-2 limits the game uses against stadium_hues.json. usage: check_hues.py RESULT_DIR"""
import json, os, re, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
d = sys.argv[1]
log = open(os.path.join(d, 'a.log'), errors='replace').read()
r = subprocess.run(['node', os.path.join(HERE, 'reference.js')], input=log, capture_output=True, text=True)
print(r.stdout.strip())
if r.returncode != 0:
    sys.exit('the page answers differently')
# Gen 1-2 species (1-251 are the national dex numbers in the game's order) use Stadium's limits as they are
stad = {v['dex']: (v['min'], v['max']) for v in json.load(open(os.path.join(HERE, 'stadium_hues.json'))).values()}
bad = 0
for sp, mn, mx in set(re.findall(r'\[hue\] deg (\d+) \d+ (\d+) (\d+)', log)):
    sp = int(sp)
    if sp <= 251 and stad.get(sp) != (int(mn), int(mx)):
        bad += 1
        print('species %d limits %s/%s, Stadium %s' % (sp, mn, mx, stad.get(sp)))
if bad or 'nv' not in log:
    sys.exit('limits differ')
print('limits ok')
