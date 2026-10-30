#!/usr/bin/env python3
"""Gen 4 Pokémon in the POKéDEX's sort orders (src/data/pokemon/pokedex_orders.h: alphabetical, weight, height).

    python3 tools/moemon_gen4/gen_dex_orders.py

Keeps the Gen 1-3 order as it is and inserts each Gen 4 Pokémon where its name / weight / height places it (ties
after the older ones). Names come from tools/moemon_gen4/gen4_data.json, weight and height from
src/data/pokemon/pokedex_entries.h and pokedex_entries_gen4.h. Re-running gives the same file.
"""
import json
import os
import re

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
P = os.path.join(REPO, 'src/data/pokemon/pokedex_orders.h')


def main():
    g4 = [s for s in json.load(open(os.path.join(REPO, 'tools/moemon_gen4/gen4_data.json')))['species'] if s['gen'] == 4]
    stats = {}
    for f in ('pokedex_entries.h', 'pokedex_entries_gen4.h'):
        t = open(os.path.join(REPO, 'src/data/pokemon', f), encoding='utf-8').read()
        for m in re.finditer(r'\[(NATIONAL_DEX_\w+)\] =\s*\{[^}]*?\.height = (\d+),\s*\.weight = (\d+)', t):
            stats[m.group(1)] = (int(m.group(3)), int(m.group(2)))
    names = {}
    for s in json.load(open(os.path.join(REPO, 'tools/moemon_gen4/gen4_data.json')))['species']:
        names['NATIONAL_DEX_' + s['const'][8:]] = s['name']
    src = open(P, encoding='utf-8').read()
    out = src
    for table, key in (('Alphabetical', lambda c: names.get(c, c[13:].replace('_', ' '))),
                       ('Weight', lambda c: stats.get(c, (0, 0))[0]), ('Height', lambda c: stats.get(c, (0, 0))[1])):
        m = re.search(r'const u16 gPokedexOrder_%s\[\] =\n\{\n(.*?)\n\};' % table, out, re.S)
        old = [l.strip().rstrip(',') for l in m.group(1).split('\n') if l.strip()]
        old = [c for c in old if c not in {'NATIONAL_DEX_' + s['const'][8:] for s in g4}]
        new = list(old)
        for s in g4:
            c = 'NATIONAL_DEX_' + s['const'][8:]
            k = key(c)
            i = len(new)
            while i > 0 and key(new[i - 1]) > k:
                i -= 1
            new.insert(i, c)
        out = out[:m.start(1)] + '\n'.join('    %s,' % c for c in new) + out[m.end(1):]
    open(P, 'w', encoding='utf-8').write(out)
    print('dex orders: %d Gen 4 Pokémon placed' % len(g4))


if __name__ == '__main__':
    main()
