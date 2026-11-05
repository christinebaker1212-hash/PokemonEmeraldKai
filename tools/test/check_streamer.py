#!/usr/bin/env python3
"""streamer_mode: with STREAMER MODE on, KIRA and Spud are replaced, RALTS and the other words stay, and each style
gives a name from its own list"""
import re
import sys

log = open(sys.argv[1] + '/a.log', errors='replace').read()
lines = re.findall(r'namefilter <(.*)>', log)
if len(lines) != 4:
    sys.exit('expected 4 filtered lines, got %d' % len(lines))
off, game, anime, original = lines
ok = True
for style, line in (('game', game), ('anime', anime), ('original', original)):
    m = re.match(r'(\S+) met (.+) and RALTS\.$', line)
    if not m or m.group(1) == 'KIRA' or m.group(2) == 'Spud':
        print('%s: not replaced: %s' % (style, line))
        ok = False
    else:
        print('%s: %s / %s' % (style, m.group(1), m.group(2)))
names = {l.split()[0] for l in (game, anime, original)}
if len(names) < 2:
    print('the styles gave the same name')
    ok = False
print('spud gone' if ok else 'FAILED')
sys.exit(0 if ok else 1)
