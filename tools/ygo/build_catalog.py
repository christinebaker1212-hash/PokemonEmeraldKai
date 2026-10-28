#!/usr/bin/env python3
"""tools/ygo/catalog.json: every Yu-Gi-Oh! card the game has, and the real packs that hold them.

usage: tools/ygo/fetch_db.py && tools/ygo/build_catalog.py

The pool is Stairway to the Destined Duel's cards (tools/ygo/sdd.json, from tools/ygo/import_sdd.py) plus every card
of the English boosters and Starter Decks released up to the end of 2003 (even those Stairway lacks). Each card
has today's wording and errata (YGOPRODeck's text), Stairway's copy limit (3 for cards Stairway doesn't have), and
the sets that printed it with the rarity. Card ids are kept in tools/ygo/ids.json (by password) so a save's
collection keeps meaning the same cards when the catalog is rebuilt; new cards get new ids at the end.
"""
import json
import re
from collections import Counter, OrderedDict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DB = ROOT / 'tools/ygo/metadata/cardinfo.json'
SDD = ROOT / 'tools/ygo/sdd.json'
IDS = ROOT / 'tools/ygo/ids.json'
OUT = ROOT / 'tools/ygo/catalog.json'

# The English releases up to the end of 2003, in order (North American first editions: set codes without a letter)
SETS = [
    ('LOB', 'Legend of Blue Eyes White Dragon', '2002-03-08', 'booster'),
    ('SDY', 'Starter Deck: Yugi', '2002-03-29', 'starter'),
    ('SDK', 'Starter Deck: Kaiba', '2002-03-29', 'starter'),
    ('MRD', 'Metal Raiders', '2002-06-26', 'booster'),
    ('SRL', 'Spell Ruler', '2002-09-16', 'booster'),
    ('PSV', "Pharaoh's Servant", '2002-10-20', 'booster'),
    ('LON', 'Labyrinth of Nightmare', '2003-03-01', 'booster'),
    ('SDJ', 'Starter Deck: Joey', '2003-03-30', 'starter'),
    ('SDP', 'Starter Deck: Pegasus', '2003-03-30', 'starter'),
    ('LOD', 'Legacy of Darkness', '2003-06-06', 'booster'),
    ('PGD', 'Pharaonic Guardian', '2003-07-18', 'booster'),
    ('MFC', "Magician's Force", '2003-10-10', 'booster'),
    ('DCR', 'Dark Crisis', '2003-12-01', 'booster'),
]
RARITIES = ['Common', 'Short Print', 'Super Short Print', 'Rare', 'Super Rare', 'Ultra Rare', 'Secret Rare']
# Stairway's older names
ALIASES = {'Dark Magic Ritual': 'Black Magic Ritual', 'B. Skull Dragon': 'Black Skull Dragon',
           'Meteor B. Dragon': 'Meteor Black Dragon'}


def norm(s):
    return re.sub('[^a-z0-9]', '', s.lower())


def frame(c):
    t = c['type']
    if t == 'Token':
        return 'token'
    if t == 'Spell Card':
        return 'spell'
    if t == 'Trap Card':
        return 'trap'
    for k in ('Fusion', 'Ritual'):
        if k in t:
            return k.lower()
    return 'normal' if t == 'Normal Monster' else 'effect'


def main():
    db = json.load(open(DB))['data']
    byid, byname = {}, {}
    for c in db:
        byid[c['id']] = c
        byname.setdefault(norm(c['name']), c)
        for im in c.get('card_images', []):
            byid.setdefault(im['id'], c)
    sdd = json.load(open(SDD))
    insdd = OrderedDict()   # YGOPRODeck card id -> first Stairway card
    tokens = []
    for s in sdd['cards']:
        c = byid.get(int(s['password'])) if s['password'] else None
        c = c or byname.get(norm(ALIASES.get(s['name'], s['name'])))
        if not c:
            raise SystemExit('no card for Stairway card %d %s' % (s['id'], s['name']))
        if c['type'] == 'Token':
            tokens.append(c['id'])
        insdd.setdefault(c['id'], s)
    limits = {}
    bynum = {s['number']: s for s in sdd['cards']}
    for l in sdd['limits']:
        s = bynum.get(l['number'])
        if s:
            c = byid.get(int(s['password'])) if s['password'] else byname.get(norm(s['name']))
            limits[c['id']] = l['limit']
    printings = {code: [] for code, *_ in SETS}
    for c in db:
        for p in c.get('card_sets', []):
            code = p['set_code']
            if '-' not in code:
                continue
            pre, num = code.split('-', 1)
            if pre in printings and num[:1].isdigit():
                rarity = p['set_rarity']
                if rarity not in RARITIES:
                    raise SystemExit('unknown rarity %s for %s' % (rarity, code))
                printings[pre].append((num, c['id'], rarity))
    order = []
    for code, *_ in SETS:
        for num, cid, rarity in sorted(set(printings[code])):
            if cid not in order:
                order.append(cid)
    for cid in insdd:
        if cid not in order and cid not in tokens:
            order.append(cid)
    order += [t for t in tokens if t not in order]
    ids = json.load(open(IDS)) if IDS.exists() else {}
    nextid = max(ids.values(), default=0) + 1
    for cid in order:
        if str(cid) not in ids:
            ids[str(cid)] = nextid
            nextid += 1
    json.dump(ids, open(IDS, 'w'), indent=0)
    cards = []
    for cid in sorted(order, key=lambda x: ids[str(x)]):
        c = byid[cid]
        s = insdd.get(cid)
        f = frame(c)
        card = {'id': ids[str(cid)], 'password': cid, 'name': c['name'], 'frame': f,
                'typeline': c['type'], 'text': c['desc'], 'limit': limits.get(cid, 3),
                'sdd': s['id'] if s else None, 'number': s['number'] if s else None,
                'konami_id': (c.get('misc_info') or [{}])[0].get('konami_id')}
        if f in ('spell', 'trap'):
            card['property'] = c['race']
        else:
            card.update(race=c['race'], attribute=c.get('attribute'), level=c.get('level', 0),
                        atk=c.get('atk', 0), defense=c.get('def', 0))
        card['sets'] = []
        cards.append(card)
    byours = {c['password']: c for c in cards}
    sets = []
    for code, name, date, kind in SETS:
        rows = Counter()
        rar = {}
        for num, cid, rarity in sorted(set(printings[code])):
            rows[cid] += 1
            if RARITIES.index(rarity) > RARITIES.index(rar.get(cid, 'Common')):
                rar[cid] = rarity
            else:
                rar.setdefault(cid, rarity)
        entries = []
        for num, cid, rarity in sorted(set(printings[code])):
            if any(e[0] == ids[str(cid)] for e in entries):
                continue
            entries.append([ids[str(cid)], rar[cid], rows[cid]])
            byours[cid]['sets'].append(code)
        sets.append({'code': code, 'name': name, 'date': date, 'kind': kind, 'cards': entries})
    json.dump({'source': 'tools/ygo/sdd.json + YGOPRODeck (db.ygoprodeck.com)', 'sets': sets, 'cards': cards},
              open(OUT, 'w'), indent=1, ensure_ascii=False)
    print('%d cards (%d from Stairway), %d sets -> %s' % (len(cards), sum(1 for c in cards if c['sdd']), len(sets), OUT))
    for s in sets:
        print(' ', s['code'], len(s['cards']), sum(e[2] for e in s['cards']), dict(Counter(e[1] for e in s['cards'])))


if __name__ == '__main__':
    main()
