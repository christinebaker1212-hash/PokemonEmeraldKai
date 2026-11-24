#!/usr/bin/env python3
"""tools/ygo/ai_tags.json: what each card does, as Yugipedia tags it (the user, 2026-11-24: "scour Yugioh wikis to check
out card effects ... the AI should consider its card texts and use its cards intelligently"). Yugipedia's card pages
carry search tags that say whose cards an effect touches ("Destroys your opponent's Monster Cards" against "Destroys
Monster Cards"), what it costs ("You discard for cost", "Pays Life Points for cost"), what it gains ("You draw cards",
"Adds from Deck to hand"). Read tag by tag (each tag's cards, by password, kept for the catalog's) (cached in
tools/ygo/.cache/yugipedia/ai_tags); tools/ygo/gen_ai_tags.py turns them into the AI's table.

usage: tools/ygo/fetch_ai_tags.py   then tools/ygo/gen_ai_tags.py
"""
import hashlib
import json
import os
import sys
import time
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CATALOG = ROOT / 'tools/ygo/catalog.json'
CACHE = ROOT / 'tools/ygo/.cache/yugipedia/ai_tags'
OUT = ROOT / 'tools/ygo/ai_tags.json'
API = 'https://yugipedia.com/api.php'
PROPS = ['MonsterSpellTrap', 'Stats', 'LP', 'Banishing', 'Actions', 'Summoning', 'Password']   # (what gen_ai_tags.py reads)
# (of Summoning, only what an effect Summons: "Can be Special Summoned" and its like are every monster's)
ONLY = {'Summoning': ('Special Summons from', 'Special Summons to')}


def get(params):
    key = urllib.parse.urlencode(sorted(params.items()))
    path = CACHE / (hashlib.sha1(key.encode()).hexdigest() + '.json')
    if path.exists():
        return json.load(open(path))
    url = API + '?' + urllib.parse.urlencode(dict(params, format='json'))
    req = urllib.request.Request(url, headers={'User-Agent': 'MoemonEmeraldKai card reader (fan game)'})
    for tries in range(5):
        try:
            data = json.load(urllib.request.urlopen(req, timeout=60))
            if 'query' in data:
                break
            print('  odd answer', str(data)[:200], file=sys.stderr)
        except Exception as e:  # (a slow mirror: try again)
            print('  retry', e, file=sys.stderr)
        time.sleep(3 * (tries + 1))
    else:
        raise SystemExit('Yugipedia unreachable')
    CACHE.mkdir(parents=True, exist_ok=True)
    json.dump(data, open(path, 'w'))
    time.sleep(0.5)   # (gently)
    return data


def values(prop):
    """Every value the wiki has for a property"""
    out, off = [], 0
    while True:
        r = get({'action': 'smwbrowse', 'browse': 'pvalue',
                 'params': json.dumps({'property': prop, 'limit': 500, 'offset': off, 'search': ''})})
        out += r['query']
        if len(r['query']) < 500:
            return out
        off += 500


def main():
    cards = json.load(open(CATALOG))['cards']
    wanted = {'%08d' % c['password'] for c in cards if c.get('password')}
    out = {}
    for prop in PROPS[:-1]:
        vals = values(prop)
        print('  %s: %d values' % (prop, len(vals)), flush=True)
        for v in vals:
            if prop in ONLY and not v.startswith(ONLY[prop]):
                continue
            off = 0
            while True:
                r = get({'action': 'ask', 'query': '[[%s::%s]]|?Password|limit=500|offset=%d' % (prop, v, off)})
                res = r['query']['results'] or {}   # (none: an empty list)
                for x in res.values():
                    for one in x['printouts'].get('Password', []):
                        one = str(one).zfill(8)
                        if one in wanted:
                            out.setdefault(one, set()).add(v)
                if len(res) < 500 or off >= 9500:
                    break
                off += 500
    missing = sorted(wanted - set(out))
    json.dump({'source': 'Yugipedia: card search tags',
               'cards': {k: sorted(v) for k, v in sorted(out.items())}}, open(OUT, 'w'), indent=0)
    print('%d cards tagged, %d with no tags -> %s' % (len(out), len(missing), OUT))


if __name__ == '__main__':
    main()
