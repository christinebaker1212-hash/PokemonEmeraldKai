#!/usr/bin/env python3
"""Gen 5-9 Pokémon in the POKéDEX's sort orders (src/data/pokemon/pokedex_orders.h: alphabetical, weight, height).

    python3 tools/moemon_gen59/gen_dex_orders.py

Runs after tools/moemon_gen4/gen_dex_orders.py: keeps the order of everything already listed and inserts each
Gen 5-9 Pokémon where its name / weight / height places it (ties after the older ones). Names, weights and heights
come from tools/moemon_gen59/species.json and the older entries from src/data/pokemon/pokedex_entries*.h.
Re-running gives the same file.
"""
import json
import os
import re

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
P = os.path.join(REPO, 'src/data/pokemon/pokedex_orders.h')


def main():
    new59 = json.load(open(os.path.join(REPO, 'tools/moemon_gen59/species.json')))['species']
    stats, names = {}, {}
    for f in ('pokedex_entries.h', 'pokedex_entries_gen4.h'):
        t = open(os.path.join(REPO, 'src/data/pokemon', f), encoding='utf-8').read()
        for m in re.finditer(r'\[(NATIONAL_DEX_\w+)\] =\s*\{[^}]*?\.height = (\d+),\s*\.weight = (\d+)', t):
            stats[m.group(1)] = (int(m.group(3)), int(m.group(2)))
    for s in json.load(open(os.path.join(REPO, 'tools/moemon_gen4/gen4_data.json')))['species']:
        names['NATIONAL_DEX_' + s['const'][8:]] = s['name']
    mine = []
    for s in new59:
        c = 'NATIONAL_DEX_' + s['const'][8:]
        names[c] = s['name']
        stats[c] = (s['weight'], s['height'])
        mine.append(c)
    gone = set(mine)
    out = open(P, encoding='utf-8').read()
    for table, key in (('Alphabetical', lambda c: names.get(c, c[13:].replace('_', ' ')).upper()),
                       ('Weight', lambda c: stats.get(c, (0, 0))[0]), ('Height', lambda c: stats.get(c, (0, 0))[1])):
        m = re.search(r'const u16 gPokedexOrder_%s\[\] =\n\{\n(.*?)\n\};' % table, out, re.S)
        new = [c for c in (l.strip().rstrip(',') for l in m.group(1).split('\n') if l.strip()) if c not in gone]
        for c in mine:
            k = key(c)
            i = len(new)
            while i > 0 and key(new[i - 1]) > k:
                i -= 1
            new.insert(i, c)
        out = out[:m.start(1)] + '\n'.join('    %s,' % c for c in new) + out[m.end(1):]
    open(P, 'w', encoding='utf-8').write(out)
    print('dex orders: %d Gen 5-9 Pokémon placed' % len(mine))


if __name__ == '__main__':
    main()
