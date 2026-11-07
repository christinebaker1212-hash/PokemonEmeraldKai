#!/usr/bin/env python3
"""MAGIC's structure decks: the real preconstructed decks of the Premodern years, from MTGJSON.

    fetch_precons.py              download the deck lists, write tools/mtg/precons.json and src/mtg/mtg_precons_data.c
    fetch_precons.py --offline    only write src/mtg/mtg_precons_data.c from tools/mtg/precons.json

Every theme, starter, intro, advanced / enhanced deck and the World Championship / Pro Tour decks released from Fourth
Edition (1995) to Scourge (2003) whose main deck holds only cards of the pool (tools/mtg/pool.json, not banned) is
kept in tools/mtg/precons.json, with its main deck as printed. Only the CURATED few go into the game (the user,
2026-11-07: "Let's not overwhelm the player with theme decks. Please make a curated selection"), in the shop's order;
the game offers one once every card in it plays (Mtg_CanUse, at run time: src/mtg/mtg_precons.c), so translating more
cards opens more of them by itself. To change the selection, edit CURATED and run with --offline.
"""
import json
import os
import sys
import time
import urllib.parse
import urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
LIST = 'https://mtgjson.com/api/v5/DeckList.json'
DECK = 'https://mtgjson.com/api/v5/decks/%s.json'
FIRST, LAST = '1995-04-01', '2003-06-30'     # Fourth Edition .. Scourge
KINDS = {                                    # MTGJSON's type -> the shop's kind
    'Starter Deck': 'STARTER', 'Intro Pack': 'STARTER', 'Demo Deck': 'STARTER',
    'Theme Deck': 'THEME', 'Enhanced Deck': 'THEME', 'Advanced Deck': 'THEME', 'Box Set': 'THEME',
    'World Championship Deck': 'CHAMPION', 'Pro Tour Deck': 'CHAMPION',
}
KIND_ORDER = ['STARTER', 'THEME', 'CHAMPION']
# The shop's selection: (set, MTGJSON's name, the shop's name, its words). Two first decks, one theme deck of note from
# each era, five World Champions' decks.
CURATED = [
    ('6ED', 'Deck A', 'SIXTH STARTER A', 'Sixth Edition\'s first deck: green and black, with a touch of white.'),
    ('6ED', 'Deck B', 'SIXTH STARTER B', 'Sixth Edition\'s other first deck: blue, red and white.'),
    ('7ED', 'Infestation', 'INFESTATION', 'Red goblins that swarm, led by their king.'),
    ('7ED', 'Armada', 'ARMADA', 'White soldiers and knights that march together.'),
    ('7ED', 'Bomber', 'BOMBER', 'Blue fliers, merfolk and spells that stop spells.'),
    ('7ED', 'Way Wild', 'WAY WILD', 'Green beasts, elves and the mana to cast them.'),
    ('ULG', 'Crusher - Enhanced Deck', 'CRUSHER', 'Green and white creatures that grow and trample.'),
    ('MMQ', "Rebel's Call", 'REBELS CALL', 'White rebels that search out more rebels.'),
    ('UDS', 'Fiendish Nature - Enhanced Deck', 'FIENDISH NATURE', 'Black and green: creatures that come back.'),
    ('LGN', 'Sliver Shivers', 'SLIVER SHIVERS', 'Slivers: each one makes all the others stronger.'),
    ('SCG', 'Goblin Mob', 'GOBLIN MOB', 'Red goblins, as many as fit on the table.'),
    ('WC01', 'Alex Borteh - Merfolk Opposition', 'OPPOSITION', 'A World Championship deck: blue merfolk that lock the table.'),
    ('WC99', 'Matt Linde - Mono-Green Stompy', 'GREEN STOMPY', 'A World Championship deck: fast green creatures.'),
    ('WC01', 'Tom van de Logt - Machine Head', 'MACHINE HEAD', 'The 2001 World Champion\'s black and red deck.'),
    ('WC98', 'Ben Rubin - Sligh', 'SLIGH', 'A World Championship deck: cheap red creatures and burn.'),
    ('WC02', 'Carlos Romão - Psychatog', 'PSYCHATOG', 'The 2002 World Champion\'s blue and black deck.'),
]


def get(url):
    for attempt in range(4):
        try:
            req = urllib.request.Request(url, headers={'User-Agent': 'moemon-precons'})
            with urllib.request.urlopen(req, timeout=60) as r:
                return json.loads(r.read().decode('utf-8'))
        except Exception as e:  # network hiccups: try again
            if attempt == 3:
                raise
            time.sleep(2 << attempt)


def pool_names():
    pool = json.load(open(os.path.join(HERE, 'pool.json')))
    names = {}
    for c in pool:
        if not c.get('banned'):
            names[c['name'].lower()] = c['name']
            if ' // ' in c['name']:
                names[c['name'].split(' // ')[0].lower()] = c['name']
    return names


def fetch():
    names = pool_names()
    out, skipped = [], 0
    for e in get(LIST)['data']:
        kind = KINDS.get(e['type'])
        if not kind or not e.get('releaseDate') or not FIRST <= e['releaseDate'] <= LAST:
            continue
        deck = get(DECK % urllib.parse.quote(e['fileName']))['data']
        cards, ok = [], True
        for c in deck.get('mainBoard', []):
            name = names.get(c['name'].lower())
            if name is None:
                ok = False
                break
            cards.append([c['count'], name])
        total = sum(n for n, _ in cards)
        if not ok or total < 40 or total > 100:
            skipped += 1
            continue
        merged = {}
        for n, name in cards:
            merged[name] = merged.get(name, 0) + n
        out.append({'name': e['name'], 'set': e['code'], 'date': e['releaseDate'], 'kind': kind,
                    'cards': sorted([[n, name] for name, n in merged.items()], key=lambda x: x[1])})
        time.sleep(0.05)
    out.sort(key=lambda d: (KIND_ORDER.index(d['kind']), d['date'], d['name']))
    json.dump(out, open(os.path.join(HERE, 'precons.json'), 'w'), indent=1)
    print('fetch_precons: %d decks kept, %d with cards outside the pool' % (len(out), skipped))
    return out


def c_str(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'


def write_c(decks):
    by = {(d['set'], d['name']): d for d in decks}
    lines = ['// Generated by tools/mtg/fetch_precons.py from MTGJSON (tools/mtg/precons.json): the CARD SHOP\'s structure',
             '// decks, real preconstructed decks of the Premodern years, main decks as printed. Don\'t edit: change the',
             '// tool\'s CURATED and rerun it (--offline). The shop offers one once every card plays (src/mtg/mtg_precons.c).',
             '#include <stddef.h>', '#include "mtg/collection.h"', '']
    rows = []
    for i, (code, name, shop, about) in enumerate(CURATED):
        d = by.get((code, name))
        if d is None:
            raise SystemExit('fetch_precons: no %s %s in precons.json' % (code, name))
        lines.append('static const char *const sPrecon%d[] = {' % i)
        cards = [c_str('%d %s' % (n, card)) for n, card in d['cards']]
        for k in range(0, len(cards), 5):
            lines.append('    ' + ', '.join(cards[k:k + 5]) + ',')
        lines.append('    NULL};')
        rows.append('    {%s, %s, %s, %d, MTG_PRECON_%s, sPrecon%d},' % (c_str(shop), c_str(about), c_str(code),
                                                                       int(d['date'][:4]), d['kind'], i))
    lines += ['', 'const MtgPrecon gMtgPrecons[MTG_PRECONS] = {'] + rows + ['};']
    path = os.path.join(ROOT, 'src', 'mtg', 'mtg_precons_data.c')
    open(path, 'w').write('\n'.join(lines) + '\n')
    print('fetch_precons: %d of %d decks into %s' % (len(CURATED), len(decks), os.path.relpath(path, ROOT)))


def main():
    if '--offline' in sys.argv:
        decks = json.load(open(os.path.join(HERE, 'precons.json')))
    else:
        decks = fetch()
    write_c(decks)


if __name__ == '__main__':
    main()
