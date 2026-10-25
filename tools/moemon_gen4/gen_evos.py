#!/usr/bin/env python3
"""Gen 4 evolutions for Kai: the Gen 4 species' own, and the new ones of older species (Magnezone, Togekiss,
Leafeon...).

    python3 tools/moemon_gen4/gen_evos.py

From Platinum (gen4_data.json), in Kai's terms:
  - trade with a held item (Rhyperior, Electivire...): the trade, or use the item like a stone (Kai lets every
    trade evolution happen another way; see its Kadabra / Porygon rows);
  - Mt. Coronet's magnetic field (Magnezone, Probopass): a THUNDER STONE; the Moss Rock (Leafeon): a LEAF STONE;
    the Ice Rock (Glaceon): level up holding NEVERMELTICE (Kai has no ICE STONE);
  - the rest has its own EVO_* (constants/pokemon.h).
Rows of species that already had one in evolution.h are rewritten in place (marked "Moemon Kai: Gen 4"); the
others go in src/data/pokemon/evolution_gen4.h.
"""
import json
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))


def kai_items():
    out = {}
    for name in re.findall(r'#define (ITEM_\w+) \d+', open(os.path.join(REPO, 'include/constants/items.h')).read()):
        out.setdefault(re.sub(r'[^a-z0-9]', '', name[5:].lower()), name)
    return out


def main():
    g4 = json.load(open(os.path.join(HERE, 'gen4_data.json'), encoding='utf-8'))
    items = kai_items()
    moves_by_id = {int(v): n for n, v in re.findall(r'#define (MOVE_\w+) (\d+)', open(os.path.join(REPO, 'include/constants/moves.h')).read())}
    pmove = {m['const']: m['id'] for m in g4['moves']}

    def item(c):
        return items[re.sub(r'[^a-z0-9]', '', c[5:].lower())]

    def conv(e):
        m, tgt = e[0], e[-1]
        p = e[1] if len(e) == 3 else 0
        if m == 'EVO_LEVEL':
            return [('EVO_LEVEL', p, tgt)]
        if m == 'EVO_USE_ITEM':
            return [('EVO_ITEM', item(p), tgt)]
        if m == 'EVO_USE_ITEM_MALE':
            return [('EVO_ITEM_MALE', item(p), tgt)]
        if m == 'EVO_USE_ITEM_FEMALE':
            return [('EVO_ITEM_FEMALE', item(p), tgt)]
        if m == 'EVO_TRADE_WITH_HELD_ITEM':
            return [('EVO_TRADE_ITEM', item(p), tgt), ('EVO_ITEM', item(p), tgt)]
        if m == 'EVO_TRADE':
            return [('EVO_TRADE', 0, tgt), ('EVO_LEVEL', 40, tgt)]
        if m == 'EVO_LEVEL_HAPPINESS':
            return [('EVO_FRIENDSHIP', 0, tgt)]
        if m == 'EVO_LEVEL_HAPPINESS_DAY':
            return [('EVO_FRIENDSHIP_DAY', 0, tgt)]
        if m == 'EVO_LEVEL_HAPPINESS_NIGHT':
            return [('EVO_FRIENDSHIP_NIGHT', 0, tgt)]
        if m == 'EVO_LEVEL_KNOW_MOVE':
            return [('EVO_MOVE', moves_by_id[pmove[p]], tgt)]
        if m == 'EVO_LEVEL_MAGNETIC_FIELD':
            return [('EVO_ITEM', 'ITEM_THUNDER_STONE', tgt)]
        if m == 'EVO_LEVEL_MOSS_ROCK':
            return [('EVO_ITEM', 'ITEM_LEAF_STONE', tgt)]
        if m == 'EVO_LEVEL_ICE_ROCK':
            return [('EVO_ITEM_HOLD', 'ITEM_NEVER_MELT_ICE', tgt)]
        if m == 'EVO_LEVEL_WITH_HELD_ITEM_DAY':
            return [('EVO_ITEM_HOLD_DAY', item(p), tgt)]
        if m == 'EVO_LEVEL_WITH_HELD_ITEM_NIGHT':
            return [('EVO_ITEM_HOLD_NIGHT', item(p), tgt)]
        if m == 'EVO_LEVEL_MALE':
            return [('EVO_LEVEL_MALE', p, tgt)]
        if m == 'EVO_LEVEL_FEMALE':
            return [('EVO_LEVEL_FEMALE', p, tgt)]
        if m == 'EVO_LEVEL_SPECIES_IN_PARTY':
            return [('EVO_SPECIES_IN_PARTY', p, tgt)]
        if m in ('EVO_LEVEL_ATK_GT_DEF', 'EVO_LEVEL_ATK_LT_DEF', 'EVO_LEVEL_ATK_EQ_DEF', 'EVO_LEVEL_NINJASK',
                 'EVO_LEVEL_SHEDINJA', 'EVO_BEAUTY'):
            return [(m, p, tgt)]
        if m == 'EVO_LEVEL_BEAUTY':
            return [('EVO_BEAUTY', p, tgt)]
        if m == 'EVO_LEVEL_PID_LOW':
            return [('EVO_LEVEL_SILCOON', p, tgt)]
        if m == 'EVO_LEVEL_PID_HIGH':
            return [('EVO_LEVEL_CASCOON', p, tgt)]
        raise SystemExit('unknown method %s' % m)

    path = os.path.join(REPO, 'src/data/pokemon/evolution.h')
    src = open(path, encoding='utf-8').read()
    rows = {}
    for m in re.finditer(r'^    \[(SPECIES_\w+)\]\s*= \{(.*?)\},?(  // [^\n]*)?\n(?=    \[|\};|\n|#)', src, re.M | re.S):
        rows[m.group(1)] = m
    gen4_rows = []
    for s in g4['species']:
        new = [x for e in s['evolutions'] for x in conv(e)]
        if s['national'] >= 387:
            if new:
                gen4_rows.append((s['const'], new))
            continue
        gen4_targets = [x for x in new if any(t['const'] == x[2] and t['national'] >= 387 for t in g4['species'])]
        if not gen4_targets:
            continue
        if s['const'] in rows:
            m = rows[s['const']]
            if 'Moemon Kai: Gen 4' in (m.group(3) or ''):
                continue
            old = re.findall(r'\{(EVO_\w+), (\w+), (SPECIES_\w+)\}', m.group(2))
            allr = [tuple(o) for o in old] + [x for x in gen4_targets if (x[0], str(x[1]), x[2]) not in [tuple(o) for o in old]]
            body = ',\n'.join(' ' * 28 + '{%s, %s, %s}' % r for r in allr).lstrip()
            line = '    [%s] = {%s},  // Moemon Kai: Gen 4\n' % (s['const'], body)
            src = src.replace(m.group(0), line, 1)
        else:
            gen4_rows.append((s['const'], gen4_targets))
    open(path, 'w', encoding='utf-8').write(src)
    out = ['// Generated by tools/moemon_gen4/gen_evos.py. Do not edit.\n']
    for c, new in gen4_rows:
        out.append('    [%s] = {%s},\n' % (c, ',\n                            '.join('{%s, %s, %s}' % r for r in new)))
    open(os.path.join(REPO, 'src/data/pokemon/evolution_gen4.h'), 'w', encoding='utf-8').write(''.join(out))
    print('%d rows in evolution_gen4.h' % len(gen4_rows))


if __name__ == '__main__':
    main()
