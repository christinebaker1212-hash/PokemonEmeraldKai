#!/usr/bin/env python3
"""The generic duelists' decks from YGOPRODeck (the user, 2026-11-21: docs/YGO_DECISIONS.md "NPC decks from
YGOPRODeck"): every fetched deck (~/.cache/moemon/ygoprodeck_decks, tools/ygo/fetch_ygoprodeck_decks.py) made into one
the game can play, then kept once.

  - passcodes to the catalog's cards (alternate artworks through EDOPro's aliases, ~/.cache/moemon/BabelCdb);
    cards the catalog doesn't have are left out (the deck refilled, below)
  - Speed Duel decks and anything with fewer than 40 Main Deck cards it can't refill are dropped; at most 60 / 15
  - its era: its newest card's first English release; its list (tools/ygo/banlists.json): the TCG or OCG list in force
    then (OCG for the OCG formats; GOAT format the April 2005 list, Edison the April 2010 list); each card cut to what
    the list allows (and never more than 3), the Main Deck refilled to its size with more copies of its own strongest
    cards (tools/ygo/card_power.json) the list still allows
  - the same Main and Extra Deck twice: kept once (the copy with a tournament placing first, then the most viewed)

Writes ~/.cache/moemon/npc_pool.json (for tools/ygo/rate_pool.sh): [{"num", "name", "format", "era", "list", "main":
[catalog ids], "extra": [...], "placing", "players", "views"}] and prints how many came through each step.

  tools/ygo/build_npc_pool.py
"""
import glob, json, os, re, sqlite3, sys
from collections import Counter

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DECKS = os.path.expanduser('~/.cache/moemon/ygoprodeck_decks')
BABEL = os.path.expanduser('~/.cache/moemon/BabelCdb')
OUT = os.path.expanduser('~/.cache/moemon/npc_pool.json')
EXTRA_FRAMES = ('fusion', 'synchro', 'xyz', 'link')

# words a deck's name may not show in the game (it is someone's upload): the deck is named after its cover card instead
BADWORDS = re.compile(r'(?i)\b(fuck|shit|bitch|cunt|nigg|fag|retard|porn|sex|dick|cock|pussy|rape|nazi|hitler|whore|slut)')


def aliases():
    out = {}
    for p in glob.glob(os.path.join(BABEL, '*.cdb')):
        try:
            for code, alias in sqlite3.connect(p).execute('select id, alias from datas where alias != 0'):
                out.setdefault(code, alias)
        except sqlite3.Error:
            pass
    return out


def clean_name(name, cover):
    """a name the game's font can show: plain letters, no markup, short; a cover card's name if there's nothing fit"""
    n = re.sub(r'<[^>]+>|&[a-z]+;', ' ', name or '')
    n = n.replace('’', "'").replace('‘', "'").replace('“', '"').replace('”', '"').replace('–', '-')
    n = ''.join(ch for ch in n if 32 <= ord(ch) < 127 or ch in 'éÉ')
    n = re.sub(r'\s+', ' ', n).strip(' -_|.')
    if not n or BADWORDS.search(n) or len(re.sub(r'[^A-Za-z]', '', n)) < 2:
        n = (cover + ' Deck') if cover else 'Deck'
    return n[:28].strip()


def main():
    catalog = json.load(open(os.path.join(ROOT, 'tools/ygo/catalog.json')))
    setdate = {s['code']: s.get('date') or '' for s in catalog['sets']}
    cards = {c['password']: c for c in catalog['cards']}
    first = {pw: min([setdate[s] for s in c.get('sets', []) if setdate.get(s)] or ['9999']) for pw, c in cards.items()}
    power = {int(k): v for k, v in json.load(open(os.path.join(ROOT, 'tools/ygo/card_power.json'))).items()}
    alias = aliases()
    lists = json.load(open(os.path.join(ROOT, 'tools/ygo/banlists.json')))
    by_medium = {m: [l for l in lists if l['medium'] == m] for m in ('TCG', 'OCG')}

    def to_card(code):
        code = int(code)
        for _ in range(3):
            if code in cards:
                return code
            code = alias.get(code, 0)
        return None

    def list_for(fmt, era):
        f = (fmt or '').lower()
        medium = 'OCG' if 'ocg' in f else 'TCG'
        date = era
        if 'goat' in f:
            date = '2005-04-01'
        elif 'edison' in f:
            date = '2010-04-01'
        pick = None
        for l in by_medium[medium]:
            if l['date'] <= date:
                pick = l
        return pick or by_medium[medium][0]

    stats = Counter()
    raw = {}
    for p in sorted(glob.glob(os.path.join(DECKS, '*.jsonl'))):
        for line in open(p):
            try:
                d = json.loads(line)
            except ValueError:
                continue
            raw[d['deckNum']] = d
    stats['fetched (distinct)'] = len(raw)
    pool = {}
    for num, d in raw.items():
        fmt = d.get('format') or ''
        if 'speed duel' in fmt.lower() or 'rush' in fmt.lower():
            stats['dropped: Speed / Rush Duel'] += 1
            continue
        try:
            main = [to_card(x) for x in json.loads(d.get('main_deck') or '[]')]
            extra = [to_card(x) for x in json.loads(d.get('extra_deck') or '[]')]
        except (ValueError, TypeError):
            stats['dropped: unreadable'] += 1
            continue
        missing = sum(1 for x in main + extra if x is None)
        main = [x for x in main if x]
        extra = [x for x in extra if x]
        # (an Extra Deck monster in the Main Deck list, or the other way: where it belongs)
        main, extra = [x for x in main if cards[x]['frame'] not in EXTRA_FRAMES], \
            extra + [x for x in main if cards[x]['frame'] in EXTRA_FRAMES]
        extra = [x for x in extra if cards[x]['frame'] in EXTRA_FRAMES]
        if len(main) < 30:
            stats['dropped: under 30 Main Deck cards'] += 1
            continue
        era = max(first.get(x, '9999') for x in main + extra)
        if era == '9999':
            era = '2026-12-31'
        lst = list_for(fmt, era)
        allow = lambda x: min(3, lst['cards'].get(str(x), 3))
        size = min(60, max(40, len(main) + missing))
        cnt = Counter(main)
        cut = 0
        for x in list(cnt):
            if cnt[x] > allow(x):
                cut += cnt[x] - allow(x)
                cnt[x] = allow(x)
        if cut:
            stats['decks cut to their list'] += 1
        # refill to its size: its own strongest cards the list still allows, one copy at a time round them
        own = sorted([x for x in cnt if cnt[x] > 0], key=lambda x: -power.get(x, 1500))
        while sum(cnt.values()) < size:
            added = False
            for x in own:
                if cnt[x] < allow(x) and sum(cnt.values()) < size:
                    cnt[x] += 1
                    added = True
            if not added:
                break
        while sum(cnt.values()) > 60:
            weakest = min((x for x in cnt if cnt[x] > 0), key=lambda x: power.get(x, 1500))
            cnt[weakest] -= 1
        if sum(cnt.values()) < 40:
            stats['dropped: can\'t make 40 within its list'] += 1
            continue
        ecnt = Counter(extra)
        for x in list(ecnt):
            ecnt[x] = min(ecnt[x], allow(x))
        extra_ids = sorted(ecnt.elements(), key=lambda x: (-power.get(x, 1500), x))[:15]
        main_ids = sorted(cnt.elements())
        key = (tuple(main_ids), tuple(sorted(extra_ids)))
        placing = d.get('tournamentPlacement') or ''
        entry = {'num': num, 'name': clean_name(d.get('deck_name'), cards.get(to_card(d.get('cover_card') or 0) or 0, {}).get('name')),
                 'format': fmt, 'era': era, 'list': lst['title'],
                 'main': [cards[x]['id'] for x in main_ids], 'extra': [cards[x]['id'] for x in sorted(extra_ids)],
                 'placing': placing, 'players': d.get('tournamentPlayerCount') or 0, 'views': d.get('deck_views') or 0}
        old = pool.get(key)
        if old is None or (bool(entry['placing']), entry['views']) > (bool(old['placing']), old['views']):
            if old is not None:
                stats['duplicates kept once'] += 1
            pool[key] = entry
        else:
            stats['duplicates kept once'] += 1
    decks = sorted(pool.values(), key=lambda e: e['num'])
    json.dump(decks, open(OUT, 'w'))
    stats['kept'] = len(decks)
    for k, v in stats.items():
        print('%-40s %d' % (k, v))
    print('formats:', Counter(e['format'] for e in decks).most_common())
    print('eras:', sorted(Counter(e['era'][:4] for e in decks).items()))


if __name__ == '__main__':
    main()
