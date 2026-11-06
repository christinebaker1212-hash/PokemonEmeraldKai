#!/usr/bin/env python3
"""Which moves play the same Classic battle animation (MODERN's audit, docs/MODERN_ANIMS.md).

Reads the pointer table gBattleAnims_Moves (data/battle_anim_scripts.s and its Gen 4 / Gen 5-8 includes) and follows
each move's script through scripts that are only `goto OTHER`. Prints, per Classic script, the moves that land on it.

    python3 tools/moemon_modernanim/anim_shares.py [--json OUT]
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
FILES = [ROOT / 'data/battle_anim_scripts.s', ROOT / 'data/battle_anim_scripts_gen4.inc',
         ROOT / 'data/battle_anim_scripts_gen58.inc']
PTRS = [ROOT / 'data/battle_anim_ptrs_gen4.inc', ROOT / 'data/battle_anim_ptrs_gen58.inc']


def table():
    """move id -> script label, in the table's order"""
    text = FILES[0].read_text().split('gBattleAnims_Moves::', 1)[1]
    labels = []
    for line in text.splitlines():
        line = line.split('@')[0].strip()
        m = re.match(r'ptrvalue\s+(\w+)', line)
        if m:
            labels.append(m.group(1))
            continue
        m = re.match(r'\.include\s+"([^"]+)"', line)
        if m:
            for l2 in (ROOT / m.group(1)).read_text().splitlines():
                m2 = re.match(r'\s*ptrvalue\s+(\w+)', l2.split('@')[0])
                if m2:
                    labels.append(m2.group(1))
            continue
        if line.startswith('.align') or line.startswith('gBattleAnims_StatusConditions'):
            break
    return labels


def bodies():
    """label -> its lines up to the next label"""
    out = {}
    for f in FILES:
        cur = None
        for line in f.read_text().splitlines():
            m = re.match(r'^(\w+):{1,2}\s*$', line)
            if m:
                cur = m.group(1)
                out[cur] = []
                continue
            if cur is not None:
                s = line.split('@')[0].strip()
                if s:
                    out[cur].append(s)
    return out


def resolve(label, b, seen=None):
    seen = seen or set()
    while label in b and label not in seen:
        seen.add(label)
        body = b[label]
        if body and body[0].startswith('goto '):
            label = body[0].split()[1]
            continue
        break
    return label


def main():
    labels = table()
    b = bodies()
    names = {}
    for line in (ROOT / 'include/constants/moves.h').read_text().splitlines():
        m = re.match(r'#define MOVE_(\w+)\s+(\d+)\b', line)
        if m and m.group(1) not in ('UNAVAILABLE',):
            names[int(m.group(2))] = m.group(1)
    groups = {}
    for i, lab in enumerate(labels):
        if i == 0 or lab == 'Move_COUNT':
            continue
        groups.setdefault(resolve(lab, b), []).append(i)
    shared = {k: v for k, v in groups.items() if len(v) > 1}
    if '--json' in sys.argv:
        out = {names.get(i, str(i)): resolve(lab, b) for i, lab in enumerate(labels) if 0 < i < len(labels) and lab != 'Move_COUNT'}
        Path(sys.argv[sys.argv.index('--json') + 1]).write_text(json.dumps(out, indent=1))
    print('%d moves, %d Classic scripts, %d of them played by more than one move' % (len(labels) - 1, len(groups), len(shared)))
    for k, v in sorted(shared.items(), key=lambda kv: -len(kv[1])):
        print('%-28s %2d: %s' % (k, len(v), ', '.join(names.get(i, str(i)) for i in v)))


if __name__ == '__main__':
    main()
