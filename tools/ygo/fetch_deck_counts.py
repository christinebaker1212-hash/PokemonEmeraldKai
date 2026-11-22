#!/usr/bin/env python3
"""tools/ygo/deck_counts.json: the English Starter and Structure Decks after World Championship 2006's, card for card
with how many copies of each (the card database only lists the printings; fetch_wc2006.py has the earlier ones), from
Yugipedia's set lists (cached in tools/ygo/.cache/yugipedia). tools/ygo/build_catalog.py gives the catalog's decks
these counts, so the GAME CENTER clerk can sell them as they came (the user, 2026-11-22: "more structure decks for
purchase with money").

usage: tools/ygo/fetch_deck_counts.py   then tools/ygo/build_catalog.py && tools/ygo/gen_catalog.py
"""
import json
import re
from pathlib import Path

import scrape_npc_decks as yp   # (the cached, gentle Yugipedia reader)

ROOT = Path(__file__).resolve().parents[2]
DB = ROOT / 'tools/ygo/metadata/cardinfo.json'
CATALOG = ROOT / 'tools/ygo/catalog.json'
WC6 = ROOT / 'tools/ygo/wc2006.json'
OUT = ROOT / 'tools/ygo/deck_counts.json'


def norm(s):
    s = s.lower().replace('\u03b1', 'alpha').replace('\u03b2', 'beta').replace('\u03b3', 'gamma')
    return re.sub('[^a-z0-9]', '', s)


def titles_of(code, name):
    """The pages the set's list may be under: its code's redirect, its name both ways round"""
    out = []
    r = yp.get({'action': 'query', 'titles': code, 'redirects': '1'})
    page = list(r['query']['pages'].values())[0]
    if 'missing' not in page:
        out.append(page['title'])
    bare = re.sub(r'^Structure Deck:\s*|\s*Structure Deck$', '', name)
    out += [name, 'Structure Deck: ' + bare, bare + ' Structure Deck']
    return list(dict.fromkeys(out))


def card_list(title, code):
    """[(name, copies)] from the page's {{Set list}} blocks (its printings numbered code-EN..., code-...)"""
    r = yp.get({'action': 'parse', 'page': 'Set Card Lists:%s (TCG-EN)' % title, 'prop': 'wikitext'})
    if 'parse' not in r:
        return None
    text = r['parse']['wikitext']['*']
    out = []
    for block in re.findall(r'\{\{Set list\|(.*?)\}\}', text, re.S):
        head, _, body = block.partition('\n')
        qty = re.search(r'qty=(\d+)', head)
        default = int(qty.group(1)) if qty else 1
        for line in body.splitlines():
            parts = [p.strip() for p in line.split('//')[0].split(';')]
            if len(parts) < 2 or not re.match(r'^%s-' % re.escape(code.split('_')[0]), parts[0]):
                continue
            n = int(parts[3]) if len(parts) > 3 and parts[3].isdigit() else default
            out.append((re.sub(r'\s*\(.*?\)\s*$', '', parts[1]), n))
    return out


def main():
    db = json.load(open(DB))['data']
    byname = {}
    for c in db:
        byname.setdefault(norm(c['name']), c)
    cat = json.load(open(CATALOG))
    ours = {c['id']: c for c in cat['cards']}
    anyname = {}
    for c in cat['cards']:
        anyname.setdefault(norm(c['name']), c['password'])
    done = json.load(open(WC6))['decks']
    out = {}
    for s in cat['sets']:
        if s['kind'] not in ('starter', 'structure') or s['code'] in done or not s['cards']:
            continue
        rows = None
        for title in titles_of(s['code'].split('_')[0], s['name']):
            rows = card_list(title, s['code'])
            if rows:
                break
        if not rows:
            print('  %s (%s): no set list' % (s['code'], s['name']))
            continue
        printed = {norm(ours[e[0]]['name']): ours[e[0]]['password'] for e in s['cards']}
        deck, lost = {}, []
        for name, n in rows:
            if 'token' in name.lower() or (byname.get(norm(name)) or {}).get('type', 'Skill Card') == 'Skill Card':
                continue   # (tokens, Speed Duel Skills and what isn't a card at all (Field Centers): not the deck's)
            # its printing in the set, else the catalog's card of that name (a reprint the card database missed)
            pw = printed.get(norm(name)) or anyname.get(norm(name))
            if pw is None:
                lost.append(name)
                continue
            deck[pw] = deck.get(pw, 0) + n
        if lost:
            print('  %s: left as listed (%d of its cards not matched: %s)' % (s['code'], len(lost), ', '.join(lost[:4])))
            continue
        out[s['code']] = [[pw, n] for pw, n in deck.items()]
        print('  %s: %d cards (%d different)' % (s['code'], sum(deck.values()), len(deck)))
    json.dump({'source': 'Yugipedia: Set Card Lists (TCG-EN)', 'decks': out}, open(OUT, 'w'), indent=0)
    print('%d decks -> %s' % (len(out), OUT))


if __name__ == '__main__':
    main()
