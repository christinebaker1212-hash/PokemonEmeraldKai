#!/usr/bin/env python3
"""Match every card to its English printing and cache the scans.

usage: tools/tcg/fetch_art.py   (then tools/tcg/build_printed_art.py)

A card matches a printing by name, HP, level and attack names. The Game Boy game
changed a few levels, so a card with no exact match takes the printing whose name,
HP and attacks agree. Jungle and Fossil printed every rare twice (holo and plain,
the same illustration): the holo is used. A card the game invented has no printing
and keeps its Game Boy picture. Scans come from PokemonTCG/pokemon-tcg-data's
image links, the card back from the Pokemon TCG site; tools/tcg/art_manifest.json
records each one with its hash.
"""
import concurrent.futures, hashlib, json, re, subprocess, unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CACHE = ROOT / 'tools/tcg/metadata'
SETS = ['base1', 'base2', 'base3', 'basep', 'base5', 'neo1', 'gym1', 'gym2']   # (GB2's: Team Rocket, Neo, Gym)
# Cards two printings fit equally well, by hand.
OVERRIDES = {
    158: 'basep-3',    # the two Mewtwo promos, in the order the Game Boy game lists them
    159: 'basep-14',
    211: 'base1-85',   # POKEMON CENTER: the Base Set card, not its promo reprint
    196: 'base1-73',   # Imposter (Impostor) Professor Oak
}
# The card back (the English back hasn't changed since 1999), from the Pokemon TCG site.
BACK_URL = 'https://tcg.pokemon.com/assets/img/global/tcg-card-back-2x.jpg'
ALIASES = {'ninetails': 'ninetales', 'imposterprofessoroak': 'impostorprofessoroak',
           'neutralizingshield': 'neutralshield'}


def fetch(url, path):
    if not path.exists():
        tmp = Path(str(path) + '.tmp')
        subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error', '--connect-timeout', '15',
                        '--max-time', '120', '--retry', '3', url, '-o', str(tmp)], check=True)
        tmp.replace(path)
    return path


# Where a Japanese scan's illustration window is: the scans come cropped a little differently, so each is measured
# (the strongest straight edges near the usual place, as build_printed_art.py's layouts were found)
JP_GUESS = {'pokemon': (64, 100, 548, 440), 'trainer': (56, 185, 556, 458), 'energy': (72, 163, 560, 480)}


def measure_window(path, kind):
    import numpy as np
    from PIL import Image
    a = np.asarray(Image.open(path).convert('L').resize((600, 825)), dtype=np.float32)
    gy, gx = np.abs(np.diff(a, axis=0)), np.abs(np.diff(a, axis=1))
    l, t, r, b = JP_GUESS[kind]
    cols, rows = gx[t + 20:b - 20, :].mean(axis=0), gy[:, l + 20:r - 20].mean(axis=1)
    def best(profile, c, span=28):
        lo = max(0, c - span)
        return lo + int(profile[lo:c + span].argmax()) + 1
    return [best(cols, l), best(rows, t), best(cols, r), best(rows, b)]


def norm(s):
    s = unicodedata.normalize('NFKD', s.replace('♀', ' F').replace('♂', ' M')).encode('ascii', 'ignore').decode()
    s = re.sub('[^a-z0-9]', '', s.lower())
    return ALIASES.get(s, s)


def moves(card):
    """Attack and Pokemon Power names, the same way for both sources."""
    if 'abilities' in card or 'supertype' in card:
        return [norm(a['name']) for a in card.get('attacks', [])], [norm(a['name']) for a in card.get('abilities', [])]
    attacks = [a for a in card.get('attacks', []) if a['name']]
    return ([norm(a['name']) for a in attacks if a.get('category') != 'POKEMON_POWER'],
            [norm(a['name']) for a in attacks if a.get('category') == 'POKEMON_POWER'])


def main():
    CACHE.mkdir(exist_ok=True)
    for s in SETS:
        fetch(f'https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/{s}.json', CACHE / f'{s}.json')
    physical = [c for s in SETS for c in json.loads((CACHE / f'{s}.json').read_text())]
    by_id = {p['id']: p for p in physical}
    rows, tasks = [], []
    for c in json.loads((ROOT / 'tools/tcg/catalog.json').read_text())['cards']:
        named = [p for p in physical if norm(p['name']) == norm(c['name'])]
        if c['id'] > 228:
            # GB2's cards: the English printing tools/tcg/import_poketcg2.py matched (name, HP, attacks), or none
            candidates = [by_id[c['english_printing']]] if c.get('english_printing') in by_id else []
        elif c['id'] in OVERRIDES:
            candidates = [by_id[OVERRIDES[c['id']]]]
        elif c['kind'] == 'pokemon':
            same = [p for p in named if str(p.get('hp')) == str(c['hp']) and moves(p) == moves(c)]
            exact = [p for p in same if str(p.get('level')) == str(c['level'])]
            candidates = exact or same
        else:
            candidates = named
        # The source's set flags tell Jungle, Fossil and promo printings apart.
        subset = 'base2' if '| JUNGLE' in c['sets'] else 'base3' if '| FOSSIL' in c['sets'] else 'basep' if '| PRO' in c['sets'] else None
        if subset and len(candidates) > 1:
            candidates = [p for p in candidates if p['id'].startswith(subset + '-')] or candidates
        if len(candidates) > 1:
            # Holo and plain printings of one card: the holo has the lower number.
            if len({p['id'].split('-')[0] for p in candidates}) == 1 and len({p.get('rarity') for p in candidates}) > 1:
                candidates = sorted(candidates, key=lambda p: int(p['number']))[:1]
            else:
                candidates = [p for p in candidates if p['id'].startswith('base1-')][:1] or candidates
        row = {'card_id': c['id'], 'name': c['name'], 'kind': c['kind'], 'source_candidates': [p['id'] for p in candidates]}
        if len(candidates) == 1:
            p = candidates[0]
            row.update(status='matched', printing=p['id'], url=p['images']['large'],
                       path=f'graphics/tcg/printed/{c["id"]:03d}.png')
            tasks.append(row)
        else:
            row['status'] = 'game_boy_only' if not candidates else 'ambiguous'
        rows.append(row)
    (ROOT / 'graphics/tcg/printed').mkdir(parents=True, exist_ok=True)

    # Cards never printed in English: the renders made for them (tools/tcg/card_renders.json), art only
    renders = json.loads((ROOT / 'tools/tcg/card_renders.json').read_text())
    cards = {c['id']: c for c in json.loads((ROOT / 'tools/tcg/catalog.json').read_text())['cards']}
    for r in renders['renders']:
        ids = r.get('ids') or [c['id'] for c in cards.values() if norm(c['name']) == norm(r['name']) and (
            c['kind'] == 'trainer' if r.get('trainer') else c['kind'] == 'pokemon' and c['hp'] == r['hp'] and
            sorted(norm(a['name']) for a in c['attacks'] if a['name']) == sorted(norm(m) for m in r['moves']))]
        for cid in ids:
            row = next(x for x in rows if x['card_id'] == cid)
            if row['status'] == 'matched':
                continue   # (a real printing wins)
            src = ROOT / renders['dir'] / r['file']
            out = ROOT / f'graphics/tcg/printed/{cid:03d}.png'
            from PIL import Image
            Image.open(src).convert('RGB').resize((600, 825), Image.Resampling.LANCZOS).save(out, optimize=True)
            raw = out.read_bytes()
            row.update(status='matched', printing='render-' + ('trainer' if r.get('trainer') else 'pokemon'),
                       render=r['file'], path=str(out.relative_to(ROOT)), sha256=hashlib.sha256(raw).hexdigest(),
                       bytes=len(raw))

    # Still none: the Japanese printing's scan (Bulbapedia, through tools/tcg/gb2_text.json), for the cards only
    # printed in Japan (the Vending series, Japanese promos and starter decks); the Game Boy picture only for cards
    # that were never printed at all
    texts = json.loads((ROOT / 'tools/tcg/gb2_text.json').read_text())['cards']
    (CACHE / 'japanese').mkdir(exist_ok=True)
    for row in rows:
        c = cards[row['card_id']]
        t = texts.get(str(c.get('text_index'))) if c.get('text_index') else None   # (its own page: import_poketcg2.py)
        if row['status'] == 'matched' or not t or not t.get('image_url'):
            continue
        name = t['image']
        if re.search(r'\((Base Set|Jungle|Fossil|Team Rocket|Wizards Promo) \d+\)$', t['page']):
            continue   # (an English printing's page: the card itself didn't match it)
        raw_path = fetch(t['image_url'], CACHE / 'japanese' / name)
        out = ROOT / f'graphics/tcg/printed/{row["card_id"]:03d}.png'
        from PIL import Image
        Image.open(raw_path).convert('RGB').resize((600, 825), Image.Resampling.LANCZOS).save(out, optimize=True)
        raw = out.read_bytes()
        row.update(status='matched', printing='japan-' + (c.get('real_set') or 'GB').lower(), page=t['page'],
                   url=t['image_url'], path=str(out.relative_to(ROOT)), sha256=hashlib.sha256(raw).hexdigest(),
                   bytes=len(raw))
        row['art_rect'] = measure_window(out, c['kind'])

    def image(row):
        raw = fetch(row['url'], ROOT / row['path']).read_bytes()
        if not raw.startswith(b'\x89PNG\r\n\x1a\n'):
            raise ValueError(f'Not a PNG: {row["path"]}')
        row['sha256'] = hashlib.sha256(raw).hexdigest()
        row['bytes'] = len(raw)

    with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:
        list(pool.map(image, tasks))
    back = fetch(BACK_URL, ROOT / 'graphics/tcg/printed/back.jpg').read_bytes()
    if not back.startswith(b'\xff\xd8'):
        raise ValueError('The card back is not a JPEG')
    (ROOT / 'tools/tcg/art_manifest.json').write_text(json.dumps(
        {'source': 'https://github.com/PokemonTCG/pokemon-tcg-data', 'cards': rows,
         'back': {'url': BACK_URL, 'path': 'graphics/tcg/printed/back.jpg', 'sha256': hashlib.sha256(back).hexdigest(),
                  'bytes': len(back)}}, indent=2) + '\n')
    count = lambda s: sum(r['status'] == s for r in rows)
    print(f'{count("matched")} printed cards, {count("game_boy_only")} Game Boy only, {count("ambiguous")} ambiguous')
    for r in rows:
        if r['status'] != 'matched':
            print('  ', r['card_id'], r['name'], r['status'], r['source_candidates'])


if __name__ == '__main__':
    main()
