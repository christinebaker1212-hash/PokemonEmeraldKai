#!/usr/bin/env python3
"""cards_variety: every kind of card player brings at least 6 different decks in 20 (COLORLESS has the fewest), GB2's among them; each special
trainer at least 2 in 5"""
import re, sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
kinds = re.findall(r'tcgvariety kind (\d+): (\d+) different decks of 20 \((\d+) GB2\)', log)
specials = re.findall(r'tcgvariety special (\d+): (\d+) different decks of 5', log)
bad = [k for k, n, g in kinds if int(n) < 6 or int(g) < 1] + ['special ' + k for k, n in specials if int(n) < 2]
print('kinds:', ' '.join('%s:%s/%s' % k for k in kinds))
print('specials:', ' '.join('%s:%s' % s for s in specials))
if len(kinds) != 7 or bad:
    print('too little variety:', bad)
    sys.exit(1)
