#!/usr/bin/env python3
"""Moemon VMU pet: the text lines of a harness shot (NAME.txt) read back through the 3x5 font.
usage: screen_text.py SHOT.txt..."""
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_font import G  # noqa: E402

def glyphs():
    out = {}
    for ch, rows in G.items():
        out[tuple(tuple(c == '#' for c in r) for r in rows)] = ch
    return out

GL = glyphs()
for p in sys.argv[1:]:
    rows = [r for r in open(p).read().split('\n') if r]
    lines = []
    for l in range(5):
        s = ''
        for cell in range(12):
            g = tuple(tuple(rows[l * 6 + r][cell * 4 + x] == '#' for x in range(3)) for r in range(5))
            s += GL.get(g, '?' if any(any(r) for r in g) else ' ')
        lines.append(s.rstrip())
    print(os.path.basename(p) + ': ' + ' | '.join(lines))
