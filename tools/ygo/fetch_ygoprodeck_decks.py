#!/usr/bin/env python3
"""YGOPRODeck's deck database, for the generic duelists' decks (the user, 2026-11-21: docs/YGO_DECISIONS.md "NPC decks
from YGOPRODeck"). Pages /api/decks/getDecks.php format by format (each format has its own window of 200,000: the
plain listing stops there), politely (one request about every 0.7 s), resuming where it stopped. Keeps what the game
needs of each deck (its number, name, format, Main / Extra Deck passcodes, tournament placing, views, rating) in
~/.cache/moemon/ygoprodeck_decks/<format>.jsonl: never committed (the decks are YGOPRODeck's users').

  tools/ygo/fetch_ygoprodeck_decks.py [FORMAT...]      (no FORMAT: every format)

A format whose listing goes past 30,000 is cut into slices by staple cards (SPLIT: "has A", "has B but not A", ...,
"has none of them"), each paged from 0, so no request is deep (the server is slow there); slices.done remembers the
finished ones.
"""
import json, os, sys, time, urllib.parse, urllib.request

API = 'https://ygoprodeck.com/api/decks/getDecks.php'
CACHE = os.path.expanduser('~/.cache/moemon/ygoprodeck_decks')
FORMATS = ['Tournament Meta Decks', 'Tournament Meta Decks (Genesys)', 'Tournament Meta Decks (Genesys OCG)',
           'Tournament Meta Decks OCG', 'Tournament Meta Decks OCG (Asian-English)', 'Tournament Meta Decks OCG (China)',
           'Tournament Meta Decks Worlds', 'meta decks', 'world championship decks', 'non-meta decks', 'anime decks',
           'fun/casual decks', 'theorycrafting decks', 'master duel decks', 'common charity decks', 'genesys decks',
           'domain format decks', 'edison format decks', 'goat format decks', 'worlds format decks',
           'trinity format decks', 'speed duel decks']
KEEP = ['deckNum', 'deck_name', 'format', 'main_deck', 'extra_deck', 'side_deck', 'cover_card', 'deck_views', 'rating',
        'ratingq', 'submit_date', 'tournamentName', 'tournamentPlayerCount', 'tournamentPlacement', 'deck_excerpt']
PAGE = 20      # (the API's own page size)
DELAY = 0.7


def get(params):
    url = API + '?' + urllib.parse.urlencode(params)
    for attempt in range(6):
        try:
            req = urllib.request.Request(url, headers={'User-Agent': 'deck-fetcher/1.0'})
            with urllib.request.urlopen(req, timeout=60) as r:
                return json.loads(r.read().decode('utf-8', 'replace'))
        except Exception as e:   # (a hiccup: wait and again)
            wait = 5 * (attempt + 1)
            print('  retry in %ds: %s' % (wait, e), flush=True)
            time.sleep(wait)
    return None


# staples across the eras, to cut a format too big to page shallowly into slices: "has card 1", "has card 2 but not
# card 1", ... "has none of them" (deep offsets are slow on the server: ~5 s a page at 120,000, linear in the offset)
SPLIT = [55144522, 10045474, 83764718, 24224830, 5318639, 14558127, 12580477, 53129443, 19613556, 44095762, 97268402,
         23434538, 14087893, 41420027, 81439173, 53582587, 25311006, 35261759, 24094653, 72302403, 43898403, 8267140,
         70368879, 32807846, 42141493, 27204311]
SHALLOW = 30000


def slice_params(fmt, has, hasnt):
    params = {}
    if fmt:
        params['_sft_category'] = fmt
    if has:
        params['cardcode'] = '%d|' % has
    if hasnt:
        params['notcardcode'] = ''.join('%d|' % x for x in hasnt)
    return params


def page_slice(fmt, has, hasnt, out, done):
    key = '%s/%s/%s' % (fmt, has, ','.join(map(str, hasnt)))
    if key in done:
        return
    base = slice_params(fmt, has, hasnt)
    deep = get(dict(base, offset=SHALLOW, limit=PAGE))
    time.sleep(DELAY)
    if isinstance(deep, list) and deep:   # (too big: its slices, one card further)
        k = len(hasnt) + (1 if has else 0)
        if k < len(SPLIT):
            rest = [c for c in SPLIT if c != has and c not in hasnt]
            nxt = rest[0]
            print('%s: splitting by %d' % (key, nxt), flush=True)
            # ("has A, not B" and "has B": the deck with both in the second; several cardcodes don't mean AND on the
            # server, so a slice of "has B" may hold more than this one's share: the rest come again, kept once)
            page_slice(fmt, nxt, list(hasnt), out, done)
            page_slice(fmt, has, list(hasnt) + [nxt], out, done)
            done.add(key)
            return
    offset, n = 0, 0
    while offset < 200000:
        page = get(dict(base, offset=offset, limit=PAGE))
        time.sleep(DELAY)
        if not isinstance(page, list) or not page:
            break
        for d in page:
            out.write(json.dumps({k: d.get(k) for k in KEEP}) + '\n')
        out.flush()
        offset += len(page)
        n += len(page)
    done.add(key)
    open(os.path.join(CACHE, 'slices.done'), 'a').write(key + '\n')
    print('%s: %d decks' % (key, n), flush=True)


def fetch(fmt):
    path = os.path.join(CACHE, (fmt or 'all').replace('/', '-').replace(' ', '_') + '.jsonl')
    donepath = os.path.join(CACHE, 'slices.done')
    done = set(open(donepath).read().split('\n')) if os.path.exists(donepath) else set()
    with open(path, 'a') as out:
        page_slice(fmt, None, [], out, done)


def main():
    os.makedirs(CACHE, exist_ok=True)
    for fmt in (sys.argv[1:] or FORMATS):
        fetch(fmt)


if __name__ == '__main__':
    main()
