#!/usr/bin/env python3
"""The regional sets' cards (docs/TCG_REGIONS.md) as the catalogue's entries: tools/tcg/regions/cards.json.

Reads PokemonTCG/pokemon-tcg-data's set lists (cards/en/<set>.json, cached in ~/.cache/moemon/tcgdata: fetch them
with --fetch) and gives every card but the sets' basic Energy (today's Energy cards stand for them) an id after
today's 442, in set order, keeping the ids already given (cards.json is read first: ids never move). Each text that
asks something of the engine gets an effect label ("<set>-<number>:a<k>" an attack, ":p" its Poke-Power / Poke-Body /
Pokemon Power, ":t" a Trainer's or a special Energy's rules); tools/tcg/regions/effects.py says what each one does.
generate_catalog.py appends these cards to src/tcg/tcg_catalog.c.
"""
import json, sys, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
CACHE = Path.home() / '.cache/moemon/tcgdata'
OUT = ROOT / 'tools/tcg/regions/cards.json'
DATA_REV = 'master'
# (set id, region, the catalogue's set number: TcgCard.origin)
SETS = [('gym1', 'KANTO', 13), ('gym2', 'KANTO', 14), ('neo1', 'JOHTO', 15), ('neo2', 'JOHTO', 16), ('neo3', 'JOHTO', 17),
        ('neo4', 'JOHTO', 18), ('ecard1', 'ECARD', 19), ('ecard2', 'ECARD', 20), ('ecard3', 'ECARD', 21),
        ('basep', 'PROMO', 22), ('np', 'PROMO', 23)]
FIRST_ID = 443
TYPES = ['Fire', 'Grass', 'Lightning', 'Water', 'Fighting', 'Psychic', 'Colorless', 'Darkness', 'Metal']
FLAGS = {'Stadium': 'STADIUM', 'Pokémon Tool': 'TOOL', 'Supporter': 'SUPPORTER', 'Technical Machine': 'TM',
         'Baby': 'BABY', 'EX': 'EX', "Rocket's Secret Machine": 'RSM', 'Goldenrod Game Corner': 'GAME_CORNER',
         'Special': 'SPECIAL'}


def plain(text):
    """the game's font: e for é, F / M for the Nidoran signs, Greek letters spelled"""
    t = (text or '').replace('Pokémon', 'Pokemon').replace('é', 'e').replace('’', "'").replace('—', '-')
    for a, b in (('♀', ' F'), ('♂', ' M'), ('α', 'Alpha'), ('β', 'Beta'), ('γ', 'Gamma'), ('δ', 'Delta'), ('×', 'x')):
        t = t.replace(a, b)
    return ' '.join(t.split())


def fetch():
    CACHE.mkdir(parents=True, exist_ok=True)
    for sid, _, _ in SETS:
        url = f'https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/{DATA_REV}/cards/en/{sid}.json'
        with urllib.request.urlopen(url) as r:
            (CACHE / f'{sid}.json').write_bytes(r.read())
        print('fetched', sid)


def attack(a, label):
    dmg = a.get('damage', '') or ''
    num = int(''.join(ch for ch in dmg if ch.isdigit()) or 0)
    cat = 'DAMAGE_PLUS' if dmg.endswith('+') else 'DAMAGE_X' if dmg.endswith('×') or dmg.endswith('x') else \
        'DAMAGE_MINUS' if dmg.endswith('-') else 'DAMAGE_NORMAL'
    text = plain(a.get('text', ''))
    if not num and text and cat == 'DAMAGE_NORMAL':
        cat = 'RESIDUAL'
    cost = {}
    for t in a.get('cost', []):
        if t in TYPES:
            cost[t] = cost.get(t, 0) + 1
    return {'name': plain(a['name']), 'cost': cost, 'damage': num, 'category': cat, 'text': text,
            'effect': label if text else 'NONE'}


def main():
    if '--fetch' in sys.argv:
        fetch()
    old = {}
    if OUT.exists():
        old = {c['src']: c['id'] for c in json.loads(OUT.read_text())['cards']}
    nxt = max(old.values(), default=FIRST_ID - 1) + 1
    cards = []
    for sid, region, origin in SETS:
        for x in json.loads((CACHE / f'{sid}.json').read_text()):
            subs = x.get('subtypes', [])
            if x['supertype'] == 'Energy' and 'Basic' in subs:
                continue
            src = x['id']
            if src in old:
                cid = old[src]
            else:
                cid, nxt = nxt, nxt + 1
            c = {'id': cid, 'src': src, 'set': sid, 'region': region, 'origin': origin, 'number': x['number'],
                 'name': plain(x['name']), 'rarity': x.get('rarity', 'Promo'),
                 'flags': sorted({FLAGS[s] for s in subs if s in FLAGS}), 'image': x['images']['large']}
            name = x['name']
            if name.startswith('Shining '):
                c['flags'].append('SHINING')
            if name.startswith('Light '):
                c['flags'].append('LIGHT')
            if name.startswith('Dark '):
                c['flags'].append('DARK')
            if 'Crystal Type' in ' '.join(a.get('name', '') for a in x.get('abilities', [])):
                c['flags'].append('CRYSTAL')
            if "'s " in name and x['supertype'] == 'Pokémon' and sid in ('gym1', 'gym2', 'basep'):
                c['flags'].append('OWNER')
            if x['supertype'] == 'Pokémon':
                c['kind'] = 'pokemon'
                c['type'] = (x.get('types') or ['Colorless'])[0]
                c['hp'] = int(x.get('hp', '0') or 0)
                c['stage'] = 2 if 'Stage 2' in subs else 1 if 'Stage 1' in subs else 0
                c['evolvesFrom'] = plain(x.get('evolvesFrom', ''))
                c['retreat'] = len(x.get('retreatCost', []))
                c['weakness'] = (x.get('weaknesses') or [{}])[0].get('type')
                c['resistance'] = (x.get('resistances') or [{}])[0].get('type')
                c['species'] = (x.get('nationalPokedexNumbers') or [0])[0]
                c['level'] = int(''.join(ch for ch in str(x.get('level', '')) if ch.isdigit()) or 0)
                c['flavor'] = plain(x.get('flavorText', ''))
                c['attacks'] = []
                for k, ab in enumerate(x.get('abilities', [])[:1]):
                    kind = {'Poké-Power': 'POWER', 'Poké-Body': 'BODY', 'Pokémon Power': 'POWER'}.get(ab.get('type'), 'POWER')
                    c['ability'] = {'name': plain(ab['name']), 'kind': kind, 'text': plain(ab.get('text', '')),
                                    'effect': f'{src}:p'}
                for k, a in enumerate(x.get('attacks', [])):
                    c['attacks'].append(attack(a, f'{src}:a{k}'))
            else:
                c['kind'] = 'trainer' if x['supertype'] == 'Trainer' else 'energy'
                c['type'] = 'Colorless'
                c['text'] = plain(' '.join(x.get('rules', [])))
                c['effect'] = f'{src}:t'
                if 'TM' in c['flags']:
                    # (a Technical Machine's own attack: the Pokemon it's attached to may use it)
                    c['attacks'] = [attack(a, f'{src}:a{k}') for k, a in enumerate(x.get('attacks', []))]
            cards.append(c)
    cards.sort(key=lambda c: c['id'])
    OUT.write_text(json.dumps({'source': 'PokemonTCG/pokemon-tcg-data', 'cards': cards}, indent=1, ensure_ascii=True) + '\n')
    print(f'{len(cards)} cards, ids {cards[0]["id"]}-{cards[-1]["id"]}')


if __name__ == '__main__':
    main()
