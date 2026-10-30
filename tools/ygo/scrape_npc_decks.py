#!/usr/bin/env python3
"""The opponents' decks of the 2002-2006 Game Boy Advance Yu-Gi-Oh! games, from Yugipedia's character pages: casual
decks of every strength (a starting rival's to a final boss's), never tournament decklists. Kept when the game's card
pool (tools/ygo/catalog.json) has nearly all of the deck: cards it hasn't got are left out, and a deck short of 40
after that is dropped.

  tools/ygo/scrape_npc_decks.py            -> tools/ygo/npc_decks.json (cached pages in tools/ygo/.cache/yugipedia)
Then tools/ygo/gen_npc_decks.py turns it into src/ygo/ygo_npc_decks.c (with the ratings from rate_decks)."""
import json, os, re, sys, time, urllib.parse, urllib.request

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
CACHE = os.path.join(ROOT, 'tools', 'ygo', '.cache', 'yugipedia')
API = 'https://yugipedia.com/api.php'
# (game, its characters' template, the heading of its part on pages that hold decks of several games). 7 Trials to
# Glory (World Championship 2005) has no opponents' decks on Yugipedia.
GAMES = [
    ('SDD', 'Template:Yu-Gi-Oh! Worldwide Edition: Stairway to the Destined Duel characters', None),
    ('EDS', 'Template:Yu-Gi-Oh! The Eternal Duelist Soul characters', None),
    ('WC4', 'Template:Yu-Gi-Oh! World Championship Tournament 2004 characters', None),
    ('ROD', 'Template:Yu-Gi-Oh! Reshef of Destruction characters', None),
    ('TSC', 'Template:Yu-Gi-Oh! The Sacred Cards characters', None),
    ('WC6', 'Template:Yu-Gi-Oh! Ultimate Masters: World Championship Tournament 2006 characters', 'World Championship 2006'),
    ('GXDA', 'Template:Yu-Gi-Oh! GX Duel Academy characters', None),
]

SKIP = {'Mimicry'}   # (a deck idea on the GX Duel Academy page, with decks from every later game)


def section(text, heading):
    """The part of a page under the heading that names the game (when the page has one), else the whole page"""
    if not heading:
        return text
    heads = list(re.finditer(r'^(=+)\s*([^=\n]+?)\s*\1\s*$', text, re.M))
    parts = []
    for i, m in enumerate(heads):
        if heading.lower() in m.group(2).lower():
            level = len(m.group(1))
            end = next((h.start() for h in heads[i + 1:] if len(h.group(1)) <= level), len(text))
            parts.append(text[m.end():end])
    if parts:
        return '\n'.join(parts)
    # (no part named after a game at all: the page is this game's)
    games = ('World Championship', 'Tag Force', 'Duel Links', 'Legacy of the Duelist', 'Spirit Caller')
    if any(any(g in m.group(2) for g in games) for m in heads):
        return ''
    return text


def get(params):
    key = urllib.parse.urlencode(sorted(params.items()))
    path = os.path.join(CACHE, re.sub(r'[^A-Za-z0-9]+', '_', key)[:180] + '.json')
    if os.path.exists(path):
        return json.load(open(path))
    url = API + '?' + urllib.parse.urlencode(dict(params, format='json'))
    req = urllib.request.Request(url, headers={'User-Agent': 'MoemonEmeraldKai deck importer (fan game)'})
    for tries in range(4):
        try:
            data = json.load(urllib.request.urlopen(req, timeout=30))
            break
        except Exception as e:  # (a slow mirror: try again)
            print('  retry', e, file=sys.stderr)
            time.sleep(2 * (tries + 1))
    else:
        raise SystemExit('Yugipedia unreachable')
    os.makedirs(CACHE, exist_ok=True)
    json.dump(data, open(path, 'w'))
    time.sleep(0.4)   # (gently)
    return data


def norm(name):
    name = name.split('|')[0]   # ("Collapse (World Championship 2006)|Shrink|...": a link's page)
    name = re.sub(r'\s*\([^)]*\)\s*$', '', name).strip().lower()
    name = re.sub(r'\bb\.\s*', 'black ', name)   # ("B. Skull Dragon", "Red-Eyes B. Dragon": the old short names)
    return re.sub(r'[^a-z0-9]+', '', name.replace('&', 'and'))


ALIASES = {'darkmagicritual': 'blackmagicritual', 'blackskulldragon': 'blackskulldragon'}


def names(cat):
    """Every name a catalog card goes by: the catalog's, and EDOPro's database's for it and its other printings."""
    byname = {norm(c['name']): c for c in cat}
    cdb = os.environ.get('BABELCDB', '/home/user/edopro/BabelCdb') + '/cards.cdb'
    if os.path.exists(cdb):
        import sqlite3
        db = sqlite3.connect(cdb)
        alias = dict(db.execute('select id, alias from datas'))
        canon = lambda c: alias.get(c) if alias.get(c) and abs(alias[c] - c) < 20 else c
        bycode = {canon(c['password']): c for c in cat}
        for code, name in db.execute('select id, name from texts'):
            c = bycode.get(canon(code))
            if c and norm(name) not in byname:
                byname[norm(name)] = c
    for a, b in ALIASES.items():
        if b in byname:
            byname.setdefault(a, byname[b])
    # World Championship 2006's own names (its opponents' lists use them)
    wc6 = os.path.join(ROOT, 'tools', 'ygo', 'wc2006.json')
    if os.path.exists(wc6):
        bypw = {c['password']: c for c in cat}
        for name, pw in json.load(open(wc6)).get('names', {}).items():
            if pw in bypw:
                byname.setdefault(norm(name), bypw[pw])
    return byname


def blocks(text):
    """Each deck on a page: a list of (name, copies). Three layouts: {{Card probabilities}} (Stairway), {{Video game
    Deck list}} (Eternal Duelist Soul), {{Decklist}} (World Championship 2004)."""
    out = []
    for kind, block in re.findall(r'\{\{(Card probabilities|Video game Deck list|Decklist)(.*?)\n\}\}', text, re.S):
        if kind == 'Card probabilities' and not re.search(r'\|\s*type\s*=\s*Deck', block):
            continue
        cards = []
        for line in block.splitlines():
            line = line.strip()
            if kind == 'Decklist':
                m = re.match(r'\*\s*\[\[([^\]|]+)(?:\|([^\]]+))?\]\]\s*(?:x\s*(\d+))?', line)
                if m:
                    cards.append((m.group(2) or m.group(1), int(m.group(3) or 1)))
                continue
            if not line or line.startswith('|') or line.startswith('{{'):
                continue
            if ';' in line:
                name, count = line.rsplit(';', 1)
                count = count.strip()
                if '%' in count:
                    continue   # (a random pick, not a fixed card)
                try:
                    n = int(count)
                except ValueError:
                    continue
            elif kind == 'Video game Deck list':
                name, n = line, 1
            else:
                continue
            if 1 <= n <= 3:
                cards.append((name.strip(), n))
        if cards:
            out.append(cards)
    return out


def fill(cards, cat, byid, seed):
    """Up to 40 the way the games did it, with Normal Monsters, but of the deck's own Attribute or Type (Level 4 or
    lower, among the strongest), the same every time for the same deck"""
    import random
    rng = random.Random(seed)
    mons = [byid[i] for i in cards if byid[i]['frame'] in ('normal', 'effect', 'ritual')]
    def most(key):
        counts = {}
        for c in mons:
            counts[c.get(key)] = counts.get(c.get(key), 0) + 1
        return max(counts, key=counts.get) if counts else None
    attr, race = most('attribute'), most('race')
    pool = [c for c in cat if c['frame'] == 'normal' and (c.get('level') or 0) <= 4 and
            (c.get('attribute') == attr or c.get('race') == race)]
    pool.sort(key=lambda c: -(c.get('atk') or 0))
    pool = pool[:24] or [c for c in cat if c['frame'] == 'normal' and (c.get('level') or 0) <= 4]
    while len(cards) < 40:
        c = rng.choice(pool)
        if cards.count(c['id']) < 3:
            cards.append(c['id'])
    return cards


def main():
    cat = json.load(open(os.path.join(ROOT, 'tools', 'ygo', 'catalog.json')))['cards']
    byid = {c['id']: c for c in cat}
    byname = names(cat)
    out, seen = [], set()
    for game, template, heading in GAMES:
        links = get({'action': 'parse', 'page': template, 'prop': 'links'})
        pages = [l['*'] for l in links.get('parse', {}).get('links', []) if l.get('ns') == 0 and 'exists' in l]
        for page in pages:
            if page in SKIP:
                continue
            key = page if heading is None else (game, page)   # (a page under several games: read once)
            if key in seen:
                continue
            seen.add(key)
            data = get({'action': 'parse', 'page': page, 'prop': 'wikitext'})
            text = section(data.get('parse', {}).get('wikitext', {}).get('*', ''), heading)
            for k, block in enumerate(blocks(text)):
                cards, missing, total = [], [], 0
                for name, n in block:
                    total += n
                    c = byname.get(norm(name))
                    if c:
                        cards += [c['id']] * n
                    else:
                        missing.append(name)
                who = re.sub(r'\s*\(.*\)$', '', page)
                core = len(cards)
                ok = core >= 15 and total <= 60 and len(missing) <= max(3, total // 6) and \
                    len(set(cards)) >= 8
                if ok and len(cards) < 40:
                    cards = fill(cards, cat, byid, page + str(k))
                print('%s %-28s %2d/%2d%s %s%s' % (game, who[:28], core, total, '+fill' if len(cards) > core else '     ',
                                                   'ok' if ok else 'skip',
                                                   '' if not missing else '  missing: ' + ', '.join(missing[:4])))
                # (at most two decks a character: some have a deck for every stage of their game)
                if ok and sum(1 for d in out if d['game'] == game and d['name'] == who) >= 2:
                    ok = False
                if ok:
                    out.append({'game': game, 'name': who, 'page': page, 'deck': k, 'cards': cards,
                                'missing': missing, 'core': core})
    path = os.path.join(ROOT, 'tools', 'ygo', 'npc_decks.json')
    json.dump(out, open(path, 'w'), indent=1)
    print('%d decks -> %s' % (len(out), path))


if __name__ == '__main__':
    main()
