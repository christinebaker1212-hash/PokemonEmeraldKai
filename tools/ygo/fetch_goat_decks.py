#!/usr/bin/env python3
"""goatformat.com's deck guides (https://www.goatformat.com/decks.html) as decks for the skilled and named duelists:
each guide's deck list and its variants, the cards mapped to the catalog (tools/ygo/catalog.json) the way
scrape_npc_decks.py maps the Game Boy decks. Appended to tools/ygo/npc_decks.json as game "GOAT" (earlier GOAT entries
replaced), then tools/ygo/rate_npc_decks.sh rates them by AI duels like the rest.

How strong a guide says its deck is, from the page's own words ("the best deck", "tier 1", "competitive", "rogue",
"casual", "meme"...), is kept as "strength" 1-4: the game leans skilled duelists toward the stronger ones.

    python3 -I tools/ygo/fetch_goat_decks.py [CACHE_DIR]

The pages are cached outside the repository (default ~/.cache/goatformat); nothing downloaded is committed.
"""
import html
import json
import os
import re
import sys
import time
import urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, 'tools', 'ygo'))
import scrape_npc_decks as npc   # noqa: E402

SITE = 'https://www.goatformat.com'
UA = 'MoemonEmeraldKai deck importer (fan game)'
# pages linked from the deck index that aren't deck guides
NOT_DECKS = {'decks', 'basics', 'cardpool', 'gettingintogoat', 'gettingstarted', 'goatformatchampionships',
             'goatgrandprix', 'goatrank', 'how2play', 'indivrulings', 'privacy-policy', 'rules', 'rulingnotices',
             'sjcfreeroll', 'tourcoverage', 'whatisgoat', 'whygoat', 'worlds'}


def fetch(cache, page):
    path = os.path.join(cache, page + '.html')
    if not os.path.exists(path):
        req = urllib.request.Request(SITE + '/' + page + '.html', headers={'User-Agent': UA})
        with urllib.request.urlopen(req, timeout=60) as r:
            data = r.read()
        open(path, 'wb').write(data)
        time.sleep(0.5)
    return open(path, encoding='utf-8', errors='replace').read()


def text_of(page_html):
    t = re.sub(r'<script.*?</script>|<style.*?</style>', '', page_html, flags=re.S)
    t = re.sub(r'<br\s*/?>', '\n', t)
    t = re.sub(r'</(p|div|h\d|li|td|tr|span)>', '\n', t)
    t = html.unescape(re.sub(r'<[^>]+>', '', t)).replace('​', '')
    return '\n'.join(l.strip() for l in t.split('\n') if l.strip())


CARD = re.compile(r'^([123])\s*x\s+(.+)$', re.I)
SECTION = {'monsters', 'spells', 'traps', 'vertical divider', 'extra deck', 'fusion deck', 'side deck', 'fusions'}


def decks_of(lines, title):
    """Each list on the page: (name, [(card, n)], [(extra card, n)]); the first is the guide's own deck, the rest
    its variants, named by the line before their description"""
    out = []
    i = 0
    name = title
    start = None
    for k, l in enumerate(lines):
        if l.lower() == 'deck list':
            start = k
            break
    if start is None:
        return out
    i = start + 1
    cur, extra, in_extra, in_side = [], [], False, False
    pending_name = title
    last_text = []
    while i < len(lines):
        l = lines[i]
        m = CARD.match(l)
        low = l.lower().rstrip(':')
        if m:
            if in_side:
                pass
            elif in_extra:
                extra.append((m.group(2).strip(), int(m.group(1))))
            else:
                cur.append((m.group(2).strip(), int(m.group(1))))
        elif low in SECTION:
            in_extra = low in ('extra deck', 'fusion deck', 'fusions')
            in_side = low == 'side deck'
        else:
            if cur:
                out.append((name, cur, extra))
                cur, extra, in_extra, in_side = [], [], False, False
                name = None
            if l.lower() in ('variants', 'variant'):
                pass
            elif name is None and not cur:
                # a variant: its name is a short line, its description follows
                if len(l) <= 40 and not l.endswith('.'):
                    name = l
            last_text.append(l)
        i += 1
    if cur:
        out.append((name or title, cur, extra))
    return out


# What a guide says about its deck, strongest first
STRENGTH = [
    (4, r'best deck in the format|tier ?1\b|top tier|dominant|the deck to beat|won (the )?world|world champion'),
    (3, r'tier ?2\b|competitive|strong|top ?8|top cut|consistent|powerful'),
    (2, r'tier ?3\b|rogue|niche|fun|underrated|surprise'),
    (1, r'casual|meme|gimmick|janky|unplayable|weak|not competitive'),
]


def strength(text):
    t = text.lower()
    score, hits = 0, 0
    for s, pat in STRENGTH:
        n = len(re.findall(pat, t))
        score += s * n
        hits += n
    return max(1, min(4, round(score / hits))) if hits else 2


def main():
    cache = sys.argv[1] if len(sys.argv) > 1 else os.path.expanduser('~/.cache/goatformat')
    os.makedirs(cache, exist_ok=True)
    index = fetch(cache, 'decks')
    pages = sorted({p for p in re.findall(r'href="/([a-z0-9\-]+)\.html"', index)} - NOT_DECKS)
    pages = [p for p in pages if not p.startswith('rulings') and not p.startswith('worlds')]
    cat = json.load(open(os.path.join(ROOT, 'tools', 'ygo', 'catalog.json')))['cards']
    byname = npc.names(cat)
    out, unknown = [], {}
    for p in pages:
        try:
            lines = text_of(fetch(cache, p)).split('\n')
        except Exception as e:   # (a page that won't come: skipped, said)
            print('skipped %s: %s' % (p, e))
            continue
        title = next((l for l in lines if l.endswith('- GoatFormat.com')), p).replace(' - GoatFormat.com', '').strip()
        # the guide's own words: its Overview, up to its deck list (the site's menu and the contents left out)
        low = [l.lower() for l in lines]
        a = low.index('overview') if 'overview' in low else 0
        b = low.index('deck list', a) if 'deck list' in low[a:] else len(lines)
        st = strength('\n'.join(lines[a:b]))
        for n, (name, main, extra) in enumerate(decks_of(lines, title)):
            cards, missing = [], []
            for card, k in main + extra:
                c = byname.get(npc.norm(card))
                if not c:
                    missing.append(card)
                    unknown[card] = unknown.get(card, 0) + 1
                    continue
                cards += [c['id']] * k
            main_n = sum(k for _, k in main)
            # (a list missing more than a few cards isn't that deck any more)
            if main_n < 40 or len(missing) > 3:
                continue
            if not name or name.upper() in ('N/A', 'NA') or not n:
                name = title if not n else '%s %d' % (title, n + 1)
            out.append({'game': 'GOAT', 'name': name, 'page': p, 'deck': n, 'cards': cards,
                        'missing': missing, 'core': len(set(cards)), 'strength': st})
    path = os.path.join(ROOT, 'tools', 'ygo', 'npc_decks.json')
    decks = [d for d in json.load(open(path)) if d.get('game') != 'GOAT'] + out
    open(path, 'w').write(json.dumps(decks, indent=1) + '\n')
    print('%d goatformat.com decks from %d pages -> %s (%d decks in all)' % (len(out), len(pages), path, len(decks)))
    if unknown:
        print('not in the catalog:', ', '.join(sorted(unknown, key=lambda k: -unknown[k])[:30]))


if __name__ == '__main__':
    main()
