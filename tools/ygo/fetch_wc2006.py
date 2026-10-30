#!/usr/bin/env python3
"""tools/ygo/wc2006.json: what tools/ygo/build_catalog.py takes from Yugipedia (cached in tools/ygo/.cache/yugipedia).

usage: tools/ygo/fetch_wc2006.py

- cards: every card of Yu-Gi-Oh! Ultimate Masters: World Championship Tournament 2006 (the GBA game), by password
  (its card pages name each card's password; tokens left out).
- limits: that game's latest Forbidden & Limited list (March 2006), by password: 0 Forbidden, 1 Limited, 2
  Semi-Limited. The catalog uses it for the cards Stairway to the Destined Duel doesn't have.
- names: the game's own names for its cards ("Collapse" is Shrink), for its opponents' deck lists.
- decks: the English Starter and Structure Decks of the era card for card, with how many copies of each (the card
  database only lists the printings): code -> [[password, copies], ...].
"""
import json
import re
from pathlib import Path

import scrape_npc_decks as yp   # (the cached, gentle Yugipedia reader)

ROOT = Path(__file__).resolve().parents[2]
DB = ROOT / 'tools/ygo/metadata/cardinfo.json'
GAME = 'Yu-Gi-Oh! Ultimate Masters: World Championship Tournament 2006'
LIST = 'March 2006 Lists (World Championship 2006)'
DECKS = {
    'SKE': 'Starter Deck: Kaiba Evolution', 'SYE': 'Starter Deck: Yugi Evolution',
    'SD1': "Structure Deck: Dragon's Roar", 'SD2': 'Structure Deck: Zombie Madness',
    'SD3': 'Structure Deck: Blaze of Destruction', 'SD4': 'Structure Deck: Fury from the Deep',
    'SD5': "Structure Deck: Warrior's Triumph", 'SD6': "Structure Deck: Spellcaster's Judgment",
    'SD7': 'Structure Deck: Invincible Fortress', 'SD8': 'Structure Deck: Lord of the Storm',
    'YSD': 'Starter Deck 2006',
}


def norm(s):
    return re.sub('[^a-z0-9]', '', s.lower())


def members(category):
    out, cont = [], {}
    while True:
        r = yp.get(dict({'action': 'query', 'list': 'categorymembers', 'cmtitle': category, 'cmlimit': '500'}, **cont))
        out += [m['title'] for m in r['query']['categorymembers'] if m['ns'] == 0]
        if 'continue' not in r:
            return out
        cont = {'cmcontinue': r['continue']['cmcontinue']}


def fields(titles):
    """Each page's {{...}} infobox fields, 50 pages a request"""
    out = {}
    for i in range(0, len(titles), 50):
        r = yp.get({'action': 'query', 'prop': 'revisions', 'rvprop': 'content', 'rvslots': 'main',
                    'titles': '|'.join(titles[i:i + 50])})
        for page in r['query']['pages'].values():
            text = page['revisions'][0]['slots']['main']['*']
            out[page['title']] = dict(re.findall(r'^\|\s*(\w+)\s*=\s*(.*?)\s*$', text, re.M))
    return out


def main():
    db = json.load(open(DB))['data']
    byid, byname = {}, {}
    for c in db:
        byid[c['id']] = c
        byname.setdefault(norm(c['name']), c)
        for im in c.get('card_images', []):
            byid.setdefault(im['id'], c)

    def card(password, name):
        c = byid.get(int(password)) if password and password.isdigit() else None
        return c or (byname.get(norm(name)) if name and norm(name) else None)

    pages = fields(members('Category:%s cards' % GAME))
    cards, wc6 = set(), {}
    for title, f in sorted(pages.items()):
        c = card(f.get('password', '').strip(), f.get('main'))
        if not c:
            if 'Token' not in title:
                print('  no card for', title)
            continue
        if c['type'] != 'Token':
            cards.add(c['id'])
            wc6[re.sub(r'\s*\(World Championship 2006[^)]*\)$', '', title)] = c['id']

    text = yp.get({'action': 'parse', 'page': LIST, 'prop': 'wikitext'})['parse']['wikitext']['*']
    limits = {}
    for name, status in re.findall(r'^([^;\n|{}]+);\s*(Forbidden|Limited|Semi-Limited)\s*$', text, re.M):
        name = re.sub(r'\s*\(World Championship 2006[^)]*\)$', '', name.strip())
        cid = wc6.get(name) or (card('', name) or {}).get('id')
        if not cid:
            print('  no card for list entry', name)
            continue
        limits[str(cid)] = {'Forbidden': 0, 'Limited': 1, 'Semi-Limited': 2}[status]

    decks = {}
    for code, name in DECKS.items():
        text = yp.get({'action': 'parse', 'page': 'Set Card Lists:%s (TCG-EN)' % name, 'prop': 'wikitext'})
        text = text['parse']['wikitext']['*']
        deck = []
        for line in re.findall(r'^%s-(?:EN)?\d+;[^\n]*$' % code, text, re.M):
            parts = [p.strip() for p in line.split(';')]
            name = parts[1].split('//')[0].strip()
            qty = int(parts[4]) if len(parts) > 4 and parts[4].isdigit() else 1
            c = card('', name)
            if not c:
                raise SystemExit('no card for %s in %s' % (name, code))
            deck.append([c['id'], qty])
        decks[code] = deck
        print('  %s: %d cards (%d different)' % (code, sum(q for _, q in deck), len(deck)))

    out = ROOT / 'tools/ygo/wc2006.json'
    json.dump({'source': 'Yugipedia: %s, %s, Set Card Lists' % (GAME, LIST), 'cards': sorted(cards),
               'limits': limits, 'decks': decks, 'names': dict(sorted(wc6.items()))}, open(out, 'w'), indent=0)
    print('%d cards, %d on the list -> %s' % (len(cards), len(limits), out))


if __name__ == '__main__':
    main()
