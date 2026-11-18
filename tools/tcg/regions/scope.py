#!/usr/bin/env python3
"""How much of the regional sets (docs/TCG_REGIONS.md) plays: docs/TCG_REGIONS_SCOPE.md.

A card is finished (TCG_CF_READY) when tools/tcg/regions/effects.py has every text on it (an attack, its power, a
Trainer's or special Energy's rules) and the engine plays every rule it needs (Stadium, Tool, Supporter, Baby...;
Darkness / Metal costs). The report counts them per set and lists what's left: the rules missing and the wordings
to translate, most cards first.
"""
import json, sys, collections, re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'tools/tcg/regions'))
sys.path.insert(0, str(ROOT / 'tools/tcg'))
import effects as E


def wording(text, name):
    return E.template(text, name)[0]


def main():
    cards = json.loads((ROOT / 'tools/tcg/regions/cards.json').read_text())['cards']
    per_set = collections.OrderedDict()
    rules, words, example = collections.Counter(), collections.Counter(), {}
    for c in cards:
        t = per_set.setdefault(c['set'], collections.Counter())
        t['cards'] += 1
        if E.ready(c):
            t['finished'] += 1
            continue
        for f in c['flags']:
            if f not in E.RULES and f not in ('DARK', 'LIGHT'):
                rules[f] += 1
        texts = []
        if c['kind'] == 'pokemon':
            for a in c['attacks']:
                if a['text'] and a['effect'] not in E.ATTACKS:
                    texts.append(('attack', a['text']))
                for ty, rule in (('Darkness', 'DARK_ENERGY'), ('Metal', 'METAL_ENERGY')):
                    if a['cost'].get(ty) and rule not in E.RULES:
                        rules[rule] += 1
            if c.get('ability') and c['ability']['effect'] not in E.POWERS:
                texts.append((c['ability']['kind'], c['ability']['text']))
        elif c['effect'] not in E.TRAINERS:
            texts.append((c['kind'].upper(), c['text']))
        for kind, text in texts:
            key = (kind, wording(text, c['name']))
            words[key] += 1
            example.setdefault(key, f"{c['name']} ({c['src']}): {text}")
    out = ['# The regional sets: what plays (tools/tcg/regions/scope.py)', '',
           'Generated; rerun after translating (docs/TCG_REGIONS.md). A card is finished when every text and rule on it',
           'plays (tools/tcg/regions/effects.py); only finished cards appear in the game.', '',
           '| Set | Cards | Finished |', '|---|---|---|']
    total = collections.Counter()
    for sid, t in per_set.items():
        out.append(f"| {sid} | {t['cards']} | {t['finished']} |")
        total.update(t)
    out.append(f"| **all** | {total['cards']} | {total['finished']} |")
    out += ['', '## Rules the engine doesn\'t play yet (cards needing each)', '']
    out += [f'- {r}: {n}' for r, n in rules.most_common()]
    out += ['', f'## Wordings to translate ({len(words)}; the most cards first)', '']
    for (kind, _), n in words.most_common():
        out.append(f"- {n} x {kind}: {example[(kind, _)][:240]}")
    (ROOT / 'docs/TCG_REGIONS_SCOPE.md').write_text('\n'.join(out) + '\n')
    print(f"{total['finished']} of {total['cards']} finished; {len(words)} wordings, rules: {dict(rules.most_common())}")


if __name__ == '__main__':
    main()
