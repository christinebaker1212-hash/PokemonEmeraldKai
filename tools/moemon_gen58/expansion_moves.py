#!/usr/bin/env python3
"""Reads the Gen 5-8 moves (Hone Claws .. Eerie Spell, ids 468-754: up to Sword / Shield and its DLC) out of
pokeemerald-expansion's src/data/moves_info.h into tools/moemon_gen58/moves.json, so the other scripts don't need
the expansion checkout.

    python3 tools/moemon_gen58/expansion_moves.py EXPANSION_DIR

Values written for several generations (`B_UPDATED_MOVE_DATA >= GEN_6 ? a : b`) keep the newest one up to Gen 8
(expansion's config macros are read as Gen 8: anything gated on GEN_9 takes its other branch).
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
FIRST, LAST = 468, 754
GENS = {'GEN_%d' % g: g for g in range(1, 10)}
GENS.update({'GEN_LATEST': 9})


def pick(expr):
    """Resolve `X >= GEN_n ? a : b` chains as of Gen 8."""
    expr = expr.strip()
    m = re.match(r'^\(?\s*(\w+)\s*(>=|<=|==|<|>)\s*(GEN_\w+)\s*\)?\s*\?\s*(.*?)\s*:\s*(.*)$', expr, re.S)
    if not m:
        return expr.strip('() ')
    g = GENS.get(m.group(3), 9)
    ok = {'>=': 8 >= g, '<=': 8 <= g, '==': 8 == g, '<': 8 < g, '>': 8 > g}[m.group(2)]
    return pick(m.group(4) if ok else m.group(5))


def enum_ids(enum_text):
    ids, n = {}, 0
    for m in re.finditer(r'^\s+(MOVES?_\w+)(?:\s*=\s*(\w+))?,', enum_text, re.M):
        name, val = m.group(1), m.group(2)
        if val:
            if not val.isdigit() and val not in ids:
                break   # past the regular moves (Z / Max moves)
            n = int(val) if val.isdigit() else ids[val]
        ids[name] = n
        n += 1
    return ids


def fields(body):
    out = {}
    depth, key, start = 0, None, None
    i = 0
    # top-level ".field = value," pairs (values may hold braces / parentheses)
    for m in re.finditer(r'^        \.(\w+) = ', body, re.M):
        pass
    parts = re.split(r'^        \.([\w.]+) = ', body, flags=re.M)
    for k, v in zip(parts[1::2], parts[2::2]):
        out[k] = v.strip().rstrip(',').strip()
    return out


def strings(v):
    return ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', v))


def additional(v):
    effs = []
    for blk in re.findall(r'\{([^{}]*)\}', v):
        e = {}
        for k, val in re.findall(r'\.(\w+)\s*=\s*([^,]+)', blk):
            e[k] = pick(val.strip())
        if e:
            effs.append(e)
    return effs


def main():
    exp = sys.argv[1]
    ids = enum_ids(open(os.path.join(exp, 'include/constants/moves.h'), encoding='utf-8').read())
    ids['MOVES_COUNT_GEN4'] = 468
    src = open(os.path.join(exp, 'src/data/moves_info.h'), encoding='utf-8').read()
    moves = []
    for const, n in sorted(ids.items(), key=lambda x: x[1]):
        if not (FIRST <= n <= LAST) or const.startswith('MOVES_'):
            continue
        m = re.search(r'\n    \[%s\] =\n    \{(.*?)\n    \},' % const, src, re.S)
        f = fields(m.group(1))
        mv = {'const': const, 'id': n, 'name': strings(f['name']), 'description': strings(f.get('description', ''))}
        for k in ('effect', 'power', 'type', 'accuracy', 'pp', 'target', 'priority', 'category', 'strikeCount',
                  'criticalHitStage', 'argument', 'recoil', 'contestEffect', 'contestCategory', 'contestComboStarterId',
                  'battleAnimScript', 'multiHit', 'alwaysCriticalHit'):
            if k in f:
                mv[k] = pick(f[k])
        mv['flags'] = sorted(k for k, v in f.items() if pick(v) == 'TRUE')
        mv['additionalEffects'] = additional(f.get('additionalEffects', ''))
        cm = f.get('contestComboMoves', '{0}')
        mv['contestComboMoves'] = [x.strip() for x in cm.strip('{}').split(',') if x.strip() not in ('', '0')]
        for k in ('power', 'accuracy', 'pp', 'priority'):
            if k in mv:
                mv[k] = int(eval(mv[k]))
        moves.append(mv)
    json.dump(moves, open(os.path.join(HERE, 'moves.json'), 'w', encoding='utf-8'), indent=1, ensure_ascii=False)
    print('%d moves (%d-%d)' % (len(moves), moves[0]['id'], moves[-1]['id']))


if __name__ == '__main__':
    main()
