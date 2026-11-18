#!/usr/bin/env python3
"""cards_theme_kanto: KANTO's card players bring some of its 8 theme decks (and never another region's)"""
import re, sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
sets = re.findall(r'tcgvariety kind \d+: theme deck .* \(([^)]*)\)', log)
print('theme decks met:', len(sets), sorted(set(sets)))
if not sets or any(s not in ('Gym Heroes', 'Gym Challenge') for s in sets):
    sys.exit(1)
