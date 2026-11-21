#!/usr/bin/env python3
"""Moemon VMU pet: assemble, and whenever a short branch can't reach (+-127 bytes), swap it for waterbear's long form
(bz -> bzo, bp -> bpo, be -> beo..., br -> jmpf) in the source, until it assembles.

usage: fix_branches.py DIR SOURCE.s OUT.VMS
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
WB = os.path.join(HERE, '..', '.tools', 'waterbear-bin')
LONG = {'bz': 'bzo', 'bnz': 'bnzo', 'bp': 'bpo', 'bn': 'bno', 'be': 'beo', 'bne': 'bneo', 'br': 'jmpf', 'bpc': 'bpco',
        'dbnz': 'dbnzo'}

d, src, out = os.path.abspath(sys.argv[1]), sys.argv[2], os.path.abspath(sys.argv[3])
src = os.path.join(d, src)
for _ in range(400):
    r = subprocess.run([WB, 'assemble', src, '-o', out], cwd=d, capture_output=True, text=True, env=dict(os.environ, PWD=d))
    text = r.stdout + r.stderr
    if '[OK]' in text:
        print(text.strip().splitlines()[-1])
        sys.exit(0)
    m = re.search(r'out of range.*?\n\s*\n?\s*([^\s:]+):(\d+):', text, re.S)
    if not m:
        print(text)
        sys.exit(1)
    path, line = os.path.join(d, m.group(1)), int(m.group(2))
    lines = open(path).read().split('\n')
    l = lines[line - 1]
    mm = re.match(r'^(\s*)(\w+)(\s.*)$', l)
    if not mm or mm.group(2) not in LONG:
        print('can\'t lengthen:', path, line, l)
        sys.exit(1)
    lines[line - 1] = mm.group(1) + LONG[mm.group(2)] + mm.group(3)
    open(path, 'w').write('\n'.join(lines))
    print('lengthened %s:%d %s' % (os.path.basename(path), line, mm.group(2)))
print('gave up')
sys.exit(1)
