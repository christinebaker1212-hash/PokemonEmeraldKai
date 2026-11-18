#!/usr/bin/env python3
"""The regional sets' own theme decks, card for card (tools/tcg/regions/theme_decks.json: pokemon-tcg-data's decks/en
for Gym Heroes / Challenge, Neo Genesis / Discovery / Destiny, Expedition, Aquapolis, Skyridge, at import_sets.py's
DATA_REV; `--fetch` refreshes it from ~/.cache/moemon/tcgdecks). Used by tools/tcg/gen_special_decks.py, which writes
them into src/tcg/tcg_decks.c (gTcgThemeDecks): the card players of their region bring them (tcg_field.c) and the
region's CARD CLUB counters sell them (tcg_ui.c).

Each printing is the game's card: the regional sets' by their number, basic Energy as the game's own, Base Set 2's
Trainers as the Game Boy games' card of that name. A card that doesn't play yet (decision 2 in docs/TCG_REGIONS.md)
is played by a stand-in: the first of STAND_INS the deck holds fewer than 4 of."""
import json
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent))
import effects as E  # noqa: E402

CARDS = json.loads((HERE / 'cards.json').read_text())['cards']
SRC = {c['src']: c for c in CARDS}
GB = {g['name']: g['id'] for g in json.loads((HERE.parent / 'catalog.json').read_text())['cards'] if g['id'] > 6}
BASIC_ENERGY = {'Grass Energy': 1, 'Fire Energy': 2, 'Water Energy': 3, 'Lightning Energy': 4, 'Fighting Energy': 5,
                'Psychic Energy': 6}
STAND_INS = ['Potion', 'Bill', 'PlusPower', 'Switch', 'Energy Retrieval', 'Super Potion']
# (set id, the set's name, the regions whose card players and counters have its decks: include/mp_region.h's bits)
HOENN, JOHTO, KANTO, SINNOH, UNOVA = (1 << r for r in range(5))
SETS = [('gym1', 'Gym Heroes', KANTO), ('gym2', 'Gym Challenge', KANTO),
        ('neo1', 'Neo Genesis', JOHTO), ('neo2', 'Neo Discovery', JOHTO), ('neo4', 'Neo Destiny', JOHTO),
        ('ecard1', 'Expedition', HOENN | SINNOH | UNOVA), ('ecard2', 'Aquapolis', HOENN | SINNOH | UNOVA),
        ('ecard3', 'Skyridge', HOENN | SINNOH | UNOVA)]


def card_id(src, name):
    """The game's card for a printing, or None when it doesn't play yet"""
    if name in BASIC_ENERGY:
        return BASIC_ENERGY[name]
    c = SRC.get(src)
    if c:
        return c['id'] if E.ready(c) else None
    plain = name.replace('é', 'e')
    if plain in GB:
        return GB[plain]
    sys.exit('theme_decks.py: no card for %s (%s)' % (src, name))


def decks():
    """[(name, set name, region bits, Counter of card ids, stand-ins)] in SETS order"""
    data = json.loads((HERE / 'theme_decks.json').read_text())
    out = []
    for sid, setname, regions in SETS:
        for d in data.get(sid, []):
            counts, missing = Counter(), []
            for c in d['cards']:
                i = card_id(c['id'], c['name'])
                if i is None:
                    missing += [c['name']] * c['count']
                else:
                    counts[i] += c['count']
            for name in missing:
                sub = next(GB[s] for s in STAND_INS if counts[GB[s]] < 4)
                counts[sub] += 1
            assert sum(counts.values()) == 60, d['name']
            out.append((d['name'], setname, regions, counts, sorted(set(missing))))
    return out


if __name__ == '__main__':
    if '--fetch' in sys.argv:
        cache = Path.home() / '.cache/moemon/tcgdecks'
        data = {}
        for sid, _, _ in SETS:
            data[sid] = [{'name': d['name'], 'cards': [{'id': c['id'], 'name': c['name'], 'count': c['count']}
                                                       for c in d['cards']]}
                         for d in json.loads((cache / ('%s.json' % sid)).read_text())]
        (HERE / 'theme_decks.json').write_text(json.dumps(data, indent=1, ensure_ascii=False) + '\n')
    for name, setname, regions, counts, missing in decks():
        print('%-16s %-14s regions %2d%s' % (name, setname, regions, '  stand-ins for ' + ', '.join(missing) if missing else ''))
