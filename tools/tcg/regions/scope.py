#!/usr/bin/env python3
"""How much of the regional sets (docs/TCG_REGIONS.md) the engine already plays: docs/TCG_REGIONS_SCOPE.md.

Reads the sets' card lists (PokemonTCG/pokemon-tcg-data, cached in ~/.cache/moemon/tcgdata/<set>.json) and today's
catalogue (src/tcg/tcg_catalog.c, effects from src/tcg/tcg_effect_table.inc). An attack with no text plays as plain
damage; one whose words match a catalogued attack's (the card's own name, numbers and types abstracted) plays as that
effect family, once its numbers are carried over; a Trainer plays when the catalogue has a card of that name. The rest
is the work list, most common wordings first.
"""
import json, re, sys, collections
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
CACHE = Path.home() / '.cache/moemon/tcgdata'
SETS = [('KANTO', 'gym1'), ('KANTO', 'gym2'), ('JOHTO', 'neo1'), ('JOHTO', 'neo2'), ('JOHTO', 'neo3'), ('JOHTO', 'neo4'),
        ('E-CARD', 'ecard1'), ('E-CARD', 'ecard2'), ('E-CARD', 'ecard3'), ('PROMO', 'basep'), ('PROMO', 'np')]
TYPES = 'Grass|Fire|Water|Lightning|Psychic|Fighting|Colorless|Darkness|Metal'


def norm(text, name):
    t = text.replace('Pokémon', 'Pokemon').replace('é', 'e').replace('’', "'")
    if name:
        for n in sorted({name, name.split()[-1]}, key=len, reverse=True):
            t = re.sub(r'\b' + re.escape(n) + r'\b', 'SELF', t)
    t = re.sub(r"\b(this Pokemon|this card)\b", 'SELF', t, flags=re.I)
    t = re.sub(r'\b(' + TYPES + r')\b', 'TYPE', t)
    t = re.sub(r'\d+', 'N', t)
    t = re.sub(r'\([^)]*\)', '', t)          # reminders in brackets
    t = re.sub(r'[^a-z A-Z]', ' ', t).lower()
    return ' '.join(t.split())


def catalogue():
    src = (ROOT / 'src/tcg/tcg_catalog.c').read_text()
    kinds = dict(re.findall(r'\{"(\w+)", (FX_\w+)', (ROOT / 'src/tcg/tcg_effect_table.inc').read_text()))
    known, trainers = {}, set()
    for block in re.split(r'\n\[\d+\] = ', src)[1:]:
        name = re.search(r'\.name="([^"]*)"', block).group(1)
        if '.kind=TCG_TRAINER' in block:
            trainers.add(name.lower())
            continue
        for aname, desc, eff in re.findall(r'\{\.name="([^"]*)",\.description="((?:[^"\\]|\\.)*)",\.effect="(\w*)"', block):
            if desc:
                known.setdefault(norm(desc.replace('\\"', '"'), name), (kinds.get(eff, 'POWER/' + eff), name, aname))
    return known, trainers


def main():
    known, trainers = catalogue()
    rows, unmatched, per_set = [], collections.Counter(), {}
    examples = {}
    for region, sid in SETS:
        cards = json.loads((CACHE / f'{sid}.json').read_text())
        tally = collections.Counter()
        for c in cards:
            ok = True
            texts = []
            if c['supertype'] == 'Trainer':
                texts = [] if c['name'].lower().replace('é', 'e') in trainers else [('TRAINER', ' '.join(c.get('rules', [])))]
            elif c['supertype'] == 'Energy':
                texts = [] if 'Basic' in c.get('subtypes', []) else [('ENERGY', ' '.join(c.get('rules', [])))]
            else:
                for ab in c.get('abilities', []):
                    texts.append((ab.get('type', 'Power'), ab.get('text', '')))
                for a in c.get('attacks', []):
                    if a.get('text'):
                        texts.append(('attack', a['text']))
            for kind, t in texts:
                key = (kind, norm(t, c['name']))
                if kind == 'attack' and key[1] in known:
                    tally['attacks matched'] += 1
                    continue
                ok = False
                unmatched[key] += 1
                examples.setdefault(key, f"{c['name']} ({sid}): {t}")
                tally[f'{kind} to translate'] += 1
            tally['cards'] += 1
            tally['cards that play now' if ok else 'cards waiting'] += 1
        per_set[sid] = (region, tally)
    out = ['# The regional sets: what plays (tools/tcg/regions/scope.py)', '',
           'Generated; rerun after translating. "Plays now": every attack, power and rule on the card matches what the',
           'engine already does (numbers to carry over), or it has none. See docs/TCG_REGIONS.md.', '',
           '| Region | Set | Cards | Play now | Waiting | Attacks matched | Attacks to translate | Powers / Bodies | Trainers / Energy |',
           '|---|---|---|---|---|---|---|---|---|']
    total = collections.Counter()
    for sid, (region, t) in per_set.items():
        powers = sum(v for k, v in t.items() if k.endswith(' to translate') and k.split(' to ')[0] not in ('attack', 'TRAINER', 'ENERGY'))
        te = t['TRAINER to translate'] + t['ENERGY to translate']
        out.append(f"| {region} | {sid} | {t['cards']} | {t['cards that play now']} | {t['cards waiting']} | "
                   f"{t['attacks matched']} | {t['attack to translate']} | {powers} | {te} |")
        total.update(t)
        total['powers'] += powers
        total['te'] += te
    out.append(f"| | **all** | {total['cards']} | {total['cards that play now']} | {total['cards waiting']} | "
               f"{total['attacks matched']} | {total['attack to translate']} | {total['powers']} | {total['te']} |")
    out += ['', f'{len(unmatched)} different wordings to translate. The most common (count, kind, an example):', '']
    for (kind, _), n in unmatched.most_common(80):
        out.append(f"- {n} x {kind}: {examples[(kind, _)][:220]}")
    (ROOT / 'docs/TCG_REGIONS_SCOPE.md').write_text('\n'.join(out) + '\n')
    print('\n'.join(out[5:5 + len(per_set) + 3]))


if __name__ == '__main__':
    main()
