#!/usr/bin/env python3
"""Older Pokémon (Gen 1-3) learn the Gen 4 moves they learn in Platinum: by level, and as egg moves.

    python3 tools/moemon_gen4/gen_oldmoves.py

Only moves 355-467 are added (Kai's own Gen 1-3 learnsets stay as they are), at Platinum's level, in level order,
each line marked "// Gen 4". Run again: lines already there are kept, not doubled. TMs and tutors keep Kai's lists.
"""
import json
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))


def main():
    g4 = json.load(open(os.path.join(HERE, 'gen4_data.json'), encoding='utf-8'))
    pmove = {m['const']: m['id'] for m in g4['moves']}
    kai_by_id = {int(v): n for n, v in re.findall(r'#define (MOVE_\w+) (\d+)', open(os.path.join(REPO, 'include/constants/moves.h')).read())}
    ptrs = dict(re.findall(r'\[(SPECIES_\w+)\] = (s\w+LevelUpLearnset)', open(os.path.join(REPO, 'src/data/pokemon/level_up_learnset_pointers.h')).read()))
    lp = os.path.join(REPO, 'src/data/pokemon/level_up_learnsets.h')
    ls = open(lp, encoding='utf-8').read()
    ep = os.path.join(REPO, 'src/data/pokemon/egg_moves.h')
    eg = open(ep, encoding='utf-8').read()
    added_lv = added_egg = 0
    for s in g4['species']:
        if s['national'] > 386 or s['const'] not in ptrs:
            continue
        new = [(lv, kai_by_id[pmove[m]]) for lv, m in s['learnset']['level'] if pmove[m] >= 355]
        sym = ptrs[s['const']]
        m = re.search(r'static const u(?:16|32) %s\[\] = \{\n(.*?)    LEVEL_UP_END\n\};' % sym, ls, re.S)
        if new and m:
            lines = m.group(1).splitlines(keepends=True)
            for lv, mv in new:
                if re.search(r'\b%s\)' % mv, m.group(1)):
                    continue
                pos = len(lines)
                for i, l in enumerate(lines):
                    k = re.search(r'LEVEL_UP_MOVE\(\s*(\d+),', l)
                    if k and int(k.group(1)) > lv:
                        pos = i
                        break
                lines.insert(pos, '    LEVEL_UP_MOVE(%2d, %s),  // Gen 4\n' % (lv, mv))
                added_lv += 1
            ls = ls.replace(m.group(0), 'static const u32 %s[] = {\n%s    LEVEL_UP_END\n};' % (sym, ''.join(lines)), 1)
        eggs = [kai_by_id[pmove[x]] for x in s['learnset']['egg'] if pmove[x] >= 355]
        if eggs:
            name = s['const'][8:]
            m = re.search(r'    egg_moves\(%s,\n(.*?)\),\n' % name, eg, re.S)
            if m:
                have = re.findall(r'MOVE_\w+', m.group(1))
                add = [x for x in eggs if x not in have]
                if add:
                    body = m.group(1) + ''.join(',\n              %s' % x for x in add)
                    eg = eg.replace(m.group(0), '    egg_moves(%s,\n%s),\n' % (name, body), 1)
                    added_egg += len(add)
            else:
                i = eg.index('#include "egg_moves_gen4.h"')
                eg = eg[:i] + '    egg_moves(%s,\n%s),  // Gen 4\n\n' % (name, ',\n'.join('              %s' % x for x in eggs)) + eg[i:]
                added_egg += len(eggs)
    open(lp, 'w', encoding='utf-8').write(ls)
    open(ep, 'w', encoding='utf-8').write(eg)
    print('added %d level-up and %d egg moves to Gen 1-3 Pokémon' % (added_lv, added_egg))


if __name__ == '__main__':
    main()
