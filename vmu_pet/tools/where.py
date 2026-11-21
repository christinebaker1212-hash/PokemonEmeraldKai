#!/usr/bin/env python3
"""Moemon VMU pet: which source line an address is (waterbear's --debug json).

usage: where.py FILE.VMS.debug.json ADDRESS(hex)...
"""
import json
import sys

d = json.load(open(sys.argv[1]))
srcs = d['sources']
ins = d['instructions']
for a in sys.argv[2:]:
    addr = int(a, 16)
    best = None
    for i in ins:
        off = i.get('offset', -1)
        if off <= addr and (best is None or off > best.get('offset', -1)):
            best = i
    lab = max((l for l in d['labels'] if l['offset'] <= addr), key=lambda l: l['offset'], default=None)
    if best:
        sp = best['span']['start']
        src = srcs[sp['source']]
        name = src.get('path', src) if isinstance(src, dict) else src
        print('%04X: %s line %d (after label %s)' % (addr, name, sp['line'] + 1, lab['name'] if lab else '?'))
