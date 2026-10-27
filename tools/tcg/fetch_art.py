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
SETS = ['base1', 'base2', 'base3', 'basep']
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
        if c['id'] in OVERRIDES:
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
