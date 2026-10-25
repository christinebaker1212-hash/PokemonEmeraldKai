#!/usr/bin/env python3
"""Gen 1-4 game data from pret/pokeplatinum, as one JSON file for the Gen 4 work.

    python3 tools/moemon_gen4/platinum_extract.py POKEPLATINUM_DIR [OUT]   (default tools/moemon_gen4/gen4_data.json)

Needs a pokeplatinum checkout with res/pokemon, res/moves, res/items and generated/ (a sparse clone is enough).
Platinum is the reference for Gen 4 numbers: stats, types and learnsets as they were in Gen 4 (no Fairy type, no
later stat changes). Moves keep Platinum's own effect names (BATTLE_EFFECT_*); Kai's battle engine names its own.

Output: {"species": [...493 entries, national order...], "moves": [...467...], "items": [...], "abilities": [...],
         "tm_moves": {"TM01": "MOVE_FOCUS_PUNCH", ...}, "tutor_moves": [...]}
"""
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def lines(path):
    return [l.strip() for l in open(path) if l.strip()]


def load(path):
    with open(path, encoding='utf-8') as fh:
        return json.load(fh)


def text(v):
    return ''.join(v).replace('\n', ' ') if isinstance(v, list) else v


def main():
    src = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(HERE, 'gen4_data.json')
    gen = os.path.join(src, 'generated')

    species = []
    for i, const in enumerate(lines(os.path.join(gen, 'species.txt'))[1:494], 1):
        d = load(os.path.join(src, 'res', 'pokemon', const[8:].lower(), 'data.json'))
        dex = d['pokedex_data']
        bs = d['base_stats']
        stats = [bs['hp'], bs['attack'], bs['defense'], bs['speed'], bs['special_attack'], bs['special_defense']]
        ls = d['learnset']
        species.append({
            'national': i, 'const': const, 'name': dex['en']['name'],
            'gen': 1 if i <= 151 else 2 if i <= 251 else 3 if i <= 386 else 4,
            'types': d['types'], 'stats': stats, 'bst': sum(stats),
            'catch_rate': d['catch_rate'], 'exp_yield': d['base_exp_reward'],
            'ev_yields': [d['ev_yields'][k] for k in ('hp', 'attack', 'defense', 'speed', 'special_attack', 'special_defense')],
            'held_items': [d['held_items']['common'], d['held_items']['rare']],
            'gender_ratio': d['gender_ratio'], 'hatch_cycles': d['hatch_cycles'],
            'friendship': d['base_friendship'], 'growth': d['exp_rate'], 'egg_groups': d['egg_groups'],
            'abilities': d['abilities'], 'safari_flee': d['safari_flee_rate'], 'color': d['body_color'],
            'flip': d['flip_sprite'], 'icon_palette': d['icon_palette'],
            'evolutions': d['evolutions'], 'offspring': d['offspring'],
            'footprint': d['footprint'],
            'height_in': dex['height_inches'], 'weight_lb': dex['weight_pounds'],
            'category': dex['en'].get('category'), 'dex_text': text(dex['en'].get('entry_text')),
            'learnset': {'level': ls.get('by_level', []), 'tm': ls.get('by_tm', []),
                         'tutor': ls.get('by_tutor', []), 'egg': ls.get('egg_moves', [])},
        })

    moves = []
    for i, const in enumerate(lines(os.path.join(gen, 'moves.txt'))):
        if const in ('MOVE_NONE', 'MAX_MOVES'):
            continue
        p = os.path.join(src, 'res', 'moves', const[5:].lower(), 'data.json')
        d = load(p)
        moves.append({'id': i, 'const': const, 'name': d['name'], 'gen': 1 if i <= 165 else 2 if i <= 251 else 3 if i <= 354 else 4,
                      'description': text(d['description']), 'class': d['class'], 'type': d['type'],
                      'power': d['power'], 'accuracy': d['accuracy'], 'pp': d['pp'],
                      'effect': d['effect']['type'], 'effect_chance': d['effect']['chance'],
                      'range': d['range'], 'priority': d['priority'], 'flags': d['flags'],
                      'contest': d.get('contest')})

    items, tms = [], {}
    idir = os.path.join(src, 'res', 'items', 'data')
    for i, const in enumerate(lines(os.path.join(gen, 'items.txt'))):
        p = os.path.join(idir, const[5:].lower() + '.json')
        if const == 'ITEM_NONE' or not os.path.exists(p):
            items.append({'id': i, 'const': const, 'missing': const != 'ITEM_NONE'})
            continue
        d = load(p)
        e = {'id': i, 'const': const, 'name': d['name'], 'description': text(d['description']),
             'price': d['price'], 'hold_effect': d['holdEffect'], 'effect_param': d['effectParam'],
             'fling_power': d['flingPower'], 'natural_gift': [d['naturalGiftPower'], d['naturalGiftType']],
             'pocket': d['fieldPocket'], 'field_use': d['fieldUseFunc'],
             'battle_use': d['battleUseCategory'], 'gba_id': d['gbaID'], 'icon': d['icon']}
        if d.get('teachesMove'):
            e['teaches'] = d['teachesMove']
            tms[d['name'].upper()] = d['teachesMove']
        items.append(e)

    tutors = load(os.path.join(src, 'res', 'pokemon', 'move_tutors.json'))
    abilities = lines(os.path.join(gen, 'abilities.txt'))
    data = {'_source': 'pret/pokeplatinum (tools/moemon_gen4/platinum_extract.py)',
            'species': species, 'moves': moves, 'items': items, 'abilities': abilities,
            'tm_moves': tms, 'tutor_moves': tutors}
    with open(out, 'w', encoding='utf-8') as fh:
        fh.write('{\n')
        keys = list(data)
        for n, k in enumerate(keys):
            v = data[k]
            fh.write(' %s: ' % json.dumps(k))
            if isinstance(v, list) and v and isinstance(v[0], dict):
                fh.write('[\n' + ',\n'.join('  ' + json.dumps(x, ensure_ascii=False) for x in v) + '\n ]')
            else:
                fh.write(json.dumps(v, ensure_ascii=False))
            fh.write(',\n' if n < len(keys) - 1 else '\n')
        fh.write('}\n')
    print('%d species, %d moves, %d items (%d missing), %d abilities, %d TM/HMs' % (
        len(species), len(moves), len(items), sum(1 for x in items if x.get('missing')), len(abilities), len(tms)))


if __name__ == '__main__':
    main()
