#!/usr/bin/env python3
"""Every Forbidden & Limited list, TCG and OCG, since 2002, by the date it took effect (the user, 2026-11-21: each
YGOPRODeck deck trimmed to its era's list; docs/YGO_DECISIONS.md "NPC decks from YGOPRODeck"). From Yugipedia's
list pages ({{Limitation list}}: start_date, medium, format, forbidden / limited / semi-limited), cached in
~/.cache/moemon/banlists/; card names matched to the catalog (tools/ygo/catalog.json) by name. Writes
tools/ygo/banlists.json: [{"date": "YYYY-MM-DD", "medium": "TCG"|"OCG", "format": ..., "title": ..., "cards":
{"password": 0|1|2}}], oldest first (a card not listed: 3 copies).

  tools/ygo/fetch_banlists.py
"""
import json, os, re, sys, time, urllib.parse, urllib.request
from datetime import datetime

API = 'https://yugipedia.com/api.php'
CACHE = os.path.expanduser('~/.cache/moemon/banlists')
CATEGORIES = ['Category:TCG Advanced Format Forbidden & Limited Lists', 'Category:OCG Forbidden & Limited Lists']
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def api(params):
    params = dict(params, format='json')
    url = API + '?' + urllib.parse.urlencode(params)
    for attempt in range(5):
        try:
            req = urllib.request.Request(url, headers={'User-Agent': 'deck-tool/1.0'})
            with urllib.request.urlopen(req, timeout=60) as r:
                return json.loads(r.read().decode('utf-8'))
        except Exception as e:
            print('  retry:', e, flush=True)
            time.sleep(5 * (attempt + 1))
    raise SystemExit('no answer from ' + url)


def members(cat):
    out, cont = [], {}
    while True:
        d = api(dict({'action': 'query', 'list': 'categorymembers', 'cmtitle': cat, 'cmlimit': 500}, **cont))
        out += [m['title'] for m in d['query']['categorymembers'] if m['ns'] == 0]
        if 'continue' not in d:
            return out
        cont = d['continue']


def wikitext(title):
    path = os.path.join(CACHE, re.sub(r'[^A-Za-z0-9]+', '_', title) + '.txt')
    if os.path.exists(path):
        return open(path, encoding='utf-8').read()
    d = api({'action': 'parse', 'page': title, 'prop': 'wikitext'})
    text = d['parse']['wikitext']['*']
    open(path, 'w', encoding='utf-8').write(text)
    time.sleep(0.5)
    return text


def norm(name):
    return re.sub(r'[^a-z0-9]', '', name.lower().replace('&amp;', '&'))


def parse(text):
    """the {{Limitation list}}'s fields: start date, medium, format and the three lists' card names"""
    m = re.search(r'\{\{\s*Limitation list(.*?)\n\}\}', text, re.S)
    if not m:
        return None
    fields, key = {}, None
    for line in m.group(1).split('\n'):
        f = re.match(r'\s*\|\s*([a-z_-]+)\s*=\s*(.*)$', line)
        if f:
            key = f.group(1)
            fields[key] = [f.group(2).strip()] if f.group(2).strip() else []
        elif key:
            fields[key].append(line.strip())
    def names(k):
        out = []
        for l in fields.get(k, []):
            l = re.sub(r'//.*$', '', l).strip()
            l = re.sub(r'<[^>]+>', '', l).strip()
            l = re.sub(r'\|.*$', '', l).strip()   # ("Allure of Darkness|sc")
            if l:
                out.append(l)
        return out
    def one(k):
        return ' '.join(fields.get(k, [])).strip()
    return {'start': one('start_date'), 'medium': one('medium'), 'format': one('format'),
            'forbidden': names('forbidden'), 'limited': names('limited'),
            'semi': names('semi-limited') + names('semi_limited')}


def main():
    os.makedirs(CACHE, exist_ok=True)
    cat = json.load(open(os.path.join(ROOT, 'tools/ygo/catalog.json')))['cards']
    byname = {norm(c['name']): c['password'] for c in cat}
    lists, unknown = [], {}
    for category in CATEGORIES:
        for title in members(category):
            p = parse(wikitext(title))
            if not p or not p['start']:
                print('no list on', title)
                continue
            try:
                date = datetime.strptime(re.sub(r'\[\[|\]\]', '', p['start']).strip(' .'), '%B %d, %Y').strftime('%Y-%m-%d')
            except ValueError:
                print('no date on', title, p['start'])
                continue
            medium = 'OCG' if 'OCG' in p['medium'] else 'TCG'
            cards = {}
            for status, key in ((0, 'forbidden'), (1, 'limited'), (2, 'semi')):
                for n in p[key]:
                    pw = byname.get(norm(re.sub(r'\s*\(.*?\)\s*$', '', n))) or byname.get(norm(n))
                    if pw:
                        cards[str(pw)] = status
                    else:
                        unknown[n] = unknown.get(n, 0) + 1
            lists.append({'date': date, 'medium': medium, 'format': p['format'] or 'Advanced Format', 'title': title,
                          'cards': cards})
    lists.sort(key=lambda l: (l['date'], l['medium']))
    json.dump(lists, open(os.path.join(ROOT, 'tools/ygo/banlists.json'), 'w'), indent=0)
    print('%d lists (%d TCG, %d OCG), %s to %s; %d names not in the catalog' % (
        len(lists), sum(l['medium'] == 'TCG' for l in lists), sum(l['medium'] == 'OCG' for l in lists),
        lists[0]['date'], lists[-1]['date'], len(unknown)))
    for n, k in sorted(unknown.items(), key=lambda x: -x[1])[:15]:
        print('  not matched: %s (%d)' % (n, k))


if __name__ == '__main__':
    main()
