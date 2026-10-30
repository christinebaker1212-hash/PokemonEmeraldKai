#!/usr/bin/env python3
"""tools/ygo/catalog.json: every Yu-Gi-Oh! card the game has, and the real packs that hold them.

usage: tools/ygo/fetch_db.py && tools/ygo/build_catalog.py

The pool is Stairway to the Destined Duel's cards (tools/ygo/sdd.json, from tools/ygo/import_sdd.py) plus every card
of the English boosters, Starter Decks and Structure Decks up to World Championship 2006's (Shadow of Infinity,
Structure Deck: Lord of the Storm) and every card of World Championship 2006 itself (tools/ygo/wc2006.json, from
tools/ygo/fetch_wc2006.py). Each card has today's wording and errata (YGOPRODeck's text), Stairway's copy limit
(World Championship 2006's March 2006 list for cards Stairway doesn't have, else 3), and the sets that printed it
with the rarity (an Ultimate Rare is a second printing of a card the set also has at its own rarity). Card ids are kept in tools/ygo/ids.json (by password) so a save's
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

WC6 = ROOT / 'tools/ygo/wc2006.json'

# The English releases up to World Championship 2006, in order (North American first editions)
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
    ('IOC', 'Invasion of Chaos', '2004-03-01', 'booster'),
    ('SKE', 'Starter Deck: Kaiba Evolution', '2004-03-01', 'starter'),
    ('SYE', 'Starter Deck: Yugi Evolution', '2004-03-01', 'starter'),
    ('AST', 'Ancient Sanctuary', '2004-06-01', 'booster'),
    ('SOD', 'Soul of the Duelist', '2004-10-01', 'booster'),
    ('RDS', 'Rise of Destiny', '2004-11-20', 'booster'),
    ('SD1', "Structure Deck: Dragon's Roar", '2005-01-01', 'structure'),
    ('SD2', 'Structure Deck: Zombie Madness', '2005-01-01', 'structure'),
    ('FET', 'Flaming Eternity', '2005-03-01', 'booster'),
    ('SD3', 'Structure Deck: Blaze of Destruction', '2005-05-09', 'structure'),
    ('SD4', 'Structure Deck: Fury from the Deep', '2005-05-09', 'structure'),
    ('TLM', 'The Lost Millennium', '2005-06-01', 'booster'),
    ('CRV', 'Cybernetic Revolution', '2005-08-17', 'booster'),
    ('SD5', "Structure Deck: Warrior's Triumph", '2005-10-28', 'structure'),
    ('EEN', 'Elemental Energy', '2005-11-16', 'booster'),
    ('SD6', "Structure Deck: Spellcaster's Judgment", '2006-01-18', 'structure'),
    ('SOI', 'Shadow of Infinity', '2006-02-18', 'booster'),
    ('YSD', 'Starter Deck 2006', '2006-03-23', 'starter'),
    ('SD7', 'Structure Deck: Invincible Fortress', '2006-05-15', 'structure'),
    ('SD8', 'Structure Deck: Lord of the Storm', '2006-07-12', 'structure'),
]
OLD_SETS = ['LOB', 'SDY', 'SDK', 'MRD', 'SRL', 'PSV', 'LON', 'SDJ', 'SDP', 'LOD', 'PGD', 'MFC', 'DCR']   # (the pool before 1.56.0)
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
    wc6 = json.load(open(WC6))
    # World Championship 2006's list for the cards new since 1.56.0 (the 2003 pool keeps the limits it had: 3 when
    # Stairway hasn't got the card), so no saved deck changes
    old2003 = {c['id'] for c in db for p in c.get('card_sets', []) if re.match(r'^(%s)-\d+$' % '|'.join(OLD_SETS), p['set_code'])}
    for k, v in wc6['limits'].items():
        c = byid[int(k)]
        if c['id'] not in insdd and c['id'] not in old2003:
            limits[c['id']] = v
    # (but never below what a Starter or Structure Deck sold here holds: Structure Deck: Blaze of Destruction's two
    # Level Limit - Area B)
    for deck in wc6['decks'].values():
        for p, n in deck:
            cid = byid[p]['id']
            if cid in limits and limits[cid] < n and cid not in insdd and cid not in old2003:
                limits[cid] = n
    # North American first editions: "LOB-001" up to 2004, then "SOD-EN001" (the "-EN" codes of the early sets are
    # Europe's printings)
    printings = {code: [] for code, *_ in SETS}
    plain = {m.group(1) for c in db for p in c.get('card_sets', []) for m in [re.match(r'^([A-Z0-9]+)-\d+$', p['set_code'])] if m}
    for c in db:
        for p in c.get('card_sets', []):
            m = re.match(r'^([A-Z0-9]+)-(EN)?(\d+)$', p['set_code'])
            if m and m.group(1) in printings and (m.group(2) is None) == (m.group(1) in plain):
                pre, num = m.group(1), m.group(3)
                rarity = p['set_rarity']
                if rarity == 'Ultimate Rare':
                    if any(q['set_code'] == p['set_code'] and q['set_rarity'] != rarity for q in c['card_sets']):
                        continue   # (the same card's second printing)
                    rarity = 'Ultra Rare'
                if rarity not in RARITIES:
                    raise SystemExit('unknown rarity %s for %s' % (rarity, p['set_code']))
                printings[pre].append((num, c['id'], rarity))
    order = []
    for code, *_ in SETS:
        for num, cid, rarity in sorted(set(printings[code])):
            if cid not in order:
                order.append(cid)
    for cid in insdd:
        if cid not in order and cid not in tokens:
            order.append(cid)
    for cid in wc6['cards']:
        cid = byid[cid]['id']
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
        copies = {byid[p]['id']: n for p, n in wc6['decks'].get(code, [])}   # (a deck's copies of each card)
        if kind != 'booster' and copies:
            missing = set(copies) - set(rows)
            if missing:
                raise SystemExit('%s: the deck list has %s, the printings not' % (code, missing))
            rows = Counter(copies)
        entries = []
        for num, cid, rarity in sorted(set(printings[code])):
            if any(e[0] == ids[str(cid)] for e in entries):
                continue
            entries.append([ids[str(cid)], rar[cid], rows[cid]])
            byours[cid]['sets'].append(code)
        sets.append({'code': code, 'name': name, 'date': date, 'kind': kind, 'cards': entries})
    json.dump({'source': 'tools/ygo/sdd.json + tools/ygo/wc2006.json + YGOPRODeck (db.ygoprodeck.com)', 'sets': sets, 'cards': cards},
              open(OUT, 'w'), indent=1, ensure_ascii=False)
    print('%d cards (%d from Stairway), %d sets -> %s' % (len(cards), sum(1 for c in cards if c['sdd']), len(sets), OUT))
    for s in sets:
        print(' ', s['code'], len(s['cards']), sum(e[2] for e in s['cards']), dict(Counter(e[1] for e in s['cards'])))


if __name__ == '__main__':
    main()
