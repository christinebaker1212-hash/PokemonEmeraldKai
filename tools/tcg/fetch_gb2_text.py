#!/usr/bin/env python3
"""English text for Pokemon Card GB2's cards, from Bulbapedia.

usage: tools/tcg/fetch_gb2_text.py   -> tools/tcg/gb2_text.json (pages cached in tools/tcg/metadata/bulbapedia)

The GB2 disassembly (pret/poketcg2) is of the Japanese game, so its card text is Japanese. Bulbapedia's
"List of cards by index number in Pokemon Trading Card Game 2" names every card's page in the game's own index
order (the disassembly's CardPointers order), and each card page carries the official English text, or for
cards never printed in English (the Vending series, the Game Boy originals, Japanese promos) Bulbapedia's
translation: name, attacks and Pokemon Powers with their rules text, the Pokedex category and entry.
Written in ASCII ("Pokemon"), with the wiki markup taken out.
"""
import hashlib, json, re, sys, time, unicodedata, urllib.parse, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / 'tools/tcg/metadata/bulbapedia'
OUT = ROOT / 'tools/tcg/gb2_text.json'
API = 'https://bulbapedia.bulbagarden.net/w/api.php'
LIST = 'List of cards by index number in Pokémon Trading Card Game 2'
UA = 'MoemonEmeraldKai card importer (fan game)'


def api(params):
    key = urllib.parse.urlencode(sorted(params.items()))
    path = CACHE / (re.sub(r'[^A-Za-z0-9]+', '_', key)[:120] + '_' + hashlib.sha1(key.encode()).hexdigest()[:12] + '.json')
    if path.exists():
        return json.loads(path.read_text())
    url = API + '?' + urllib.parse.urlencode(dict(params, format='json'))
    for tries in range(5):
        try:
            req = urllib.request.Request(url, headers={'User-Agent': UA})
            data = json.loads(urllib.request.urlopen(req, timeout=60).read())
            break
        except Exception as e:
            if tries == 4:
                raise
            time.sleep(2 ** tries)
    CACHE.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data))
    time.sleep(0.5)   # (gentle on the wiki)
    return data


def ascii_text(s):
    s = s.replace('♀', ' F').replace('♂', ' M').replace('×', 'x').replace('’', "'")
    s = s.replace('“', '"').replace('”', '"').replace('—', ' - ').replace('–', '-')
    return unicodedata.normalize('NFKD', s).encode('ascii', 'ignore').decode()


def unwiki(s):
    """Wiki markup to plain text."""
    s = re.sub(r'<ref[^>]*>.*?</ref>|<ref[^>]*/>', '', s, flags=re.S)
    s = re.sub(r'<br\s*/?>', ' ', s)
    s = re.sub(r'<[^>]+>', '', s)
    for _ in range(4):
        s = re.sub(r'\{\{tt\|([^|{}]*)\|[^{}]*\}\}', r'\1', s)          # {{tt|shown|hover}}
        s = re.sub(r'\{\{(?:TCG|ct|e|p|m|t|OBP|TCG ID|GB|rar|jpn|ttt)\|([^{}]*)\}\}',
                   lambda m: m.group(1).split('|')[0 if not m.group(0).startswith('{{TCG ID') else 1], s)
        s = re.sub(r'\{\{[^{}]*\}\}', '', s)
    s = re.sub(r'\[\[(?:[^|\]]*\|)?([^\]]*)\]\]', r'\1', s)
    s = re.sub(r"'{2,}", '', s)
    return ascii_text(re.sub(r'\s+', ' ', s).strip())


def field(block, name):
    m = re.search(r'^\|\s*' + name + r'\s*=(.*?)(?=^\||^\}\}|\Z)', block, re.M | re.S)
    return m.group(1).strip() if m else ''


def templates(text, name):
    """Each {{name ...}} template's body (nesting allowed)."""
    out, i = [], 0
    while True:
        i = text.find('{{' + name, i)
        if i < 0:
            return out
        depth, j = 0, i
        while j < len(text):
            if text.startswith('{{', j):
                depth += 1; j += 2
            elif text.startswith('}}', j):
                depth -= 1; j += 2
                if depth == 0:
                    break
            else:
                j += 1
        out.append(text[i:j])
        i = j


def main():
    page = api({'action': 'parse', 'page': LIST, 'prop': 'wikitext'})['parse']['wikitext']['*']
    rows = re.findall(r'\{\{Setlist/nmentry\|([0-9A-F]{2}) ([0-9A-F]{2})\|(\{\{(?:TCG ID|OBP|TCG)\|[^{}]*\}\}|\[\[[^\]]*\]\])\|', page)
    if len(rows) != 445:
        sys.exit(f'expected 445 cards in the list, found {len(rows)}')
    # each row's page title (expanding the link templates, 40 at a time)
    titles = {}
    for k in range(0, len(rows), 40):
        chunk = rows[k:k + 40]
        text = '\n'.join(r[2] for r in chunk)
        exp = api({'action': 'expandtemplates', 'text': text, 'prop': 'wikitext'})['expandtemplates']['wikitext']
        links = re.findall(r'\[\[([^|\]]+)', exp)
        if len(links) != len(chunk):
            sys.exit('a link template did not expand to one link')
        for r, t in zip(chunk, links):
            titles[int(r[0], 16) + 256 * int(r[1], 16)] = t
    # the pages, 40 at a time (redirects followed)
    pages = {}
    names = sorted(set(titles.values()))
    for k in range(0, len(names), 40):
        q = api({'action': 'query', 'prop': 'revisions', 'rvprop': 'content', 'rvslots': 'main', 'redirects': '1',
                 'titles': '|'.join(names[k:k + 40])})['query']
        redir = {r['from']: r['to'] for r in q.get('redirects', [])}
        norm = {r['from']: r['to'] for r in q.get('normalized', [])}
        got = {p['title']: p['revisions'][0]['slots']['main']['*'] for p in q['pages'].values() if 'revisions' in p}
        for n in names[k:k + 40]:
            t = redir.get(norm.get(n, n), norm.get(n, n))
            if t not in got:
                sys.exit(f'no page for {n}')
            pages[n] = got[t]
    cards = {}
    for index, title in sorted(titles.items()):
        text = pages[title]
        box = (templates(text, 'PokémoncardInfobox') or templates(text, 'Trainercardinfobox')
               or templates(text, 'TrainerInfobox') or templates(text, 'Energycardinfobox') or [''])[0]
        card = {'page': title, 'name': unwiki(field(box, 'cardname') or title.split(' (')[0]), 'attacks': [],
                'text': '', 'flavor': '', 'category': '', 'level': unwiki(field(box, 'level')), 'hp': unwiki(field(box, 'hp')),
                'image': field(box, 'image').strip()}
        if not card['image']:
            m = re.search(r'^\|image=([^\n|]+)', text, re.M)   # (a Trainer's infobox template)
            card['image'] = m.group(1).strip() if m else ''
        # The rules: the card text section; where the English printing differs, the Japanese release's (the game
        # plays the Japanese cards: Team Rocket's Grimer poisons with Poison Gas there)
        start = text.find('==Card text==')
        rules = text[start:] if start >= 0 else text
        nxt = re.search(r'\n==[^=]', rules[5:])
        end = min([i for i in (rules.find('===Pok\u00e9dex data==='), nxt.start() + 5 if nxt else -1) if i > 0] or [len(rules)])
        rules = rules[:end]
        # several printings, each under its own heading: one of them, a Japanese one when there is one
        parts = re.split(r'\n===([^=\n]+)===\s*\n', rules)
        if len(parts) > 1:
            heads = [h for h in zip(parts[1::2], parts[2::2]) if re.search(r'\{\{(Cardtext/|TCGTrainerText|TCGEnergyText)', h[1])]
            heads = heads or [('', parts[0])]
            pick = next((h for h in heads if 'japan' in h[0].lower()), heads[0])
            rules = pick[1]
            card['from'] = unwiki(pick[0])
        powers = templates(rules, 'Cardtext/Pok\u00e9mon Power') + templates(rules, 'Cardtext/Poke-Power') + \
                 templates(rules, 'Cardtext/Pok\u00e9-Power') + templates(rules, 'Cardtext/Pok\u00e9-Body')
        for t in templates(rules, 'Cardtext/Attack') + powers:
            card['attacks'].append({'name': unwiki(field(t, 'name')), 'text': unwiki(field(t, 'effect')),
                                    'damage': unwiki(field(t, 'damage')),
                                    'power': not t.startswith('{{Cardtext/Attack')})
        # attacks in the order the page lists them (powers come first on the cards)
        order = {a['name']: rules.find('|name=' + a['name']) for a in card['attacks']}
        card['attacks'].sort(key=lambda a: order[a['name']] if order[a['name']] >= 0 else 1 << 30)
        eff = templates(rules, 'TCGTrainerText') + templates(rules, 'TCGEnergyText') + templates(rules, 'Cardtext/Trainer') + \
              templates(rules, 'Cardtext/Energy') + templates(rules, 'Cardtext/Effect')
        if eff:
            card['text'] = unwiki(re.split(r'\|\s*\w+\s*=', field(eff[0], 'effect') or field(eff[0], 'text'))[0])
        if not card['text']:
            m = re.search(r'\{\{Cardtext/Header\|[^}]*\}\}(.*?)\{\{Cardtext/Footer', text, re.S)
            if m and not card['attacks']:
                card['text'] = unwiki(m.group(1))
        dex = templates(text, 'Carddex')
        if dex:
            card['category'] = unwiki(field(dex[0], 'species'))
            card['flavor'] = unwiki(field(dex[0], 'transdex') or field(dex[0], 'dex'))
        cards[str(index)] = card
    # The card pictures' URLs (the scans on each card's page: the Japanese printing for cards never printed in
    # English), 40 at a time
    files = sorted({c['image'] for c in cards.values() if c['image']})
    urls = {}
    for k in range(0, len(files), 40):
        q = api({'action': 'query', 'prop': 'imageinfo', 'iiprop': 'url|size',
                 'titles': '|'.join('File:' + f for f in files[k:k + 40])})['query']
        norm = {r['to']: r['from'] for r in q.get('normalized', [])}
        for pg in q['pages'].values():
            if pg.get('imageinfo'):
                name = norm.get(pg['title'], pg['title']).split(':', 1)[1]
                urls[name] = pg['imageinfo'][0]
    for c in cards.values():
        info = urls.get(c['image'])
        c['image_url'] = info['url'] if info else ''
        c['image_size'] = [info.get('width', 0), info.get('height', 0)] if info else []
    OUT.write_text(json.dumps({'source': 'https://bulbapedia.bulbagarden.net (CC BY-NC-SA)', 'list': LIST,
                               'cards': cards}, indent=1, ensure_ascii=True) + '\n')
    print(f'{OUT.relative_to(ROOT)}: {len(cards)} cards, '
          f'{sum(1 for c in cards.values() if c["attacks"] or c["text"])} with rules text')


if __name__ == '__main__':
    main()
