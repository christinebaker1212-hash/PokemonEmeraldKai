#!/usr/bin/env python3
"""Add Pokemon Card GB2's cards, boosters and decks to tools/tcg/catalog.json.

usage: tools/tcg/fetch_gb2_text.py && tools/tcg/import_poketcg2.py /path/to/poketcg2
       (then tools/tcg/generate_catalog.py and tools/tcg/generate_effects.py)

pret/poketcg2 is the Japanese game's disassembly (pinned below): stats, attacks with their effect commands, the
in-game booster of each card, the Game Boy illustrations and the 120-odd decks. The English text comes from
tools/tcg/gb2_text.json (Bulbapedia: the official wording, or its translation for cards never printed in English),
and where that has none, from the English printing (PokemonTCG/pokemon-tcg-data).

The first game's 228 cards keep their ids; a GB2 card that is one of them (same name, level and HP) is that card.
The rest are added after them in GB2's own index order, and tools/tcg/gb2_ids.json keeps each one's id so a save's
cards keep meaning the same thing when this runs again. Their Game Boy pictures go to graphics/tcg/original/NNN.png.
"""
import argparse, json, re, subprocess, unicodedata
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
REVISION = '212d97bd5195bf5d147a9db4ae8d36f7c85b63b9'
CATALOG = ROOT / 'tools/tcg/catalog.json'
IDS = ROOT / 'tools/tcg/gb2_ids.json'
TEXT = ROOT / 'tools/tcg/gb2_text.json'
META = ROOT / 'tools/tcg/metadata'
ART = ROOT / 'graphics/tcg/original'
# GB2's boosters, in its order (BOOSTER_*): the packs this game sells
PACKS = ['BEGINNING_POKEMON', 'LEGENDARY_POWER', 'ISLAND_OF_FOSSIL', 'PSYCHIC_BATTLE', 'SKY_FLYING_POKEMON',
         'WE_ARE_TEAM_ROCKET', 'TEAM_ROCKETS_AMBITION']
PACK_NAMES = ['Beginning Pokemon', 'Legendary Power', 'Island of Fossil', 'Psychic Battle', 'Sky-Flying Pokemon',
              'We Are Team Rocket', "Team Rocket's Ambition"]
ENGLISH_SETS = ['base1', 'base2', 'base3', 'base4', 'base5', 'basep', 'gym1', 'gym2', 'neo1']


def plain(s):
    return unicodedata.normalize('NFKD', s.replace('♀', ' F').replace('♂', ' M').replace('é', 'e')) \
        .encode('ascii', 'ignore').decode()


def norm(s):
    return re.sub(r'[^a-z0-9]', '', plain(s).lower())


def words(label):
    """A disassembly label's words: 'SlashingStrikeName' -> 'Slashing Strike'."""
    s = re.sub(r'(Alt)?(Name|Card|Deck)$', '', label)
    return re.sub(r'(?<=[a-z0-9])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])', ' ', s)


def blocks(text, suffix):
    parts = re.split(r'^(\w+' + suffix + r'):\s*$', text, flags=re.M)
    return {parts[i]: parts[i + 1] for i in range(1, len(parts), 2)}


def field(body, comment):
    for line in body.splitlines():
        if line.rstrip().endswith('; ' + comment):
            return line.split(';')[0].split(None, 1)[1].strip()
    return None


def energy(spec):
    """'GRASS, 2, COLORLESS, 1' -> {'GRASS': 2, 'COLORLESS': 1}"""
    parts = [p.strip() for p in spec.split(',') if p.strip()]
    if parts == ['0']:
        return {}
    return {parts[i]: int(parts[i + 1]) for i in range(0, len(parts) - 1, 2)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('source', type=Path)
    src = ap.parse_args().source
    revision = subprocess.check_output(['git', '-C', str(src), 'rev-parse', 'HEAD'], text=True).strip()
    if revision != REVISION:
        raise SystemExit(f'Expected poketcg2 {REVISION}; found {revision}')
    s = src / 'src'
    data = (s / 'data/cards1.asm').read_text() + '\n' + (s / 'data/cards2.asm').read_text()
    bodies = blocks(data, 'Card')
    pointers = re.findall(r'card_ptr (\w+Card)\s*; (\w+)', (s / 'data/card_pointers.asm').read_text())
    assert len(pointers) == 445, len(pointers)
    gfx = {m.group(1): m.group(2) for m in re.finditer(r'^(\w+CardGfx)::\s*\n\s*INCBIN "gfx/cards/(\w+)/', (s / 'gfx.asm').read_text(), re.M)}
    text = json.loads(TEXT.read_text())['cards']
    english = {}
    for st in ENGLISH_SETS:
        path = META / f'{st}.json'
        if not path.exists():
            subprocess.check_call(['curl', '-sfL', '-o', str(path),
                                   f'https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/{st}.json'])
        for e in json.loads(path.read_text()):
            english[f'{st}-{e["number"]}'] = e
    catalog = json.loads(CATALOG.read_text())
    gb1 = [c for c in catalog['cards'] if c['id'] <= 228]
    ids = json.loads(IDS.read_text()) if IDS.exists() else {}

    # Each GB2 card read from the disassembly
    cards = []
    for index, (label, const) in enumerate(pointers, 1):
        b = bodies[label]
        c = {'label': label, 'const': const, 'gb2_index': index, 'type': field(b, 'type'), 'name_label': field(b, 'name'),
             'rarity': field(b, 'rarity'), 'real_set': field(b, 'real set'), 'gb2_set': field(b, 'in-game set'),
             'gfx': gfx.get(field(b, 'gfx'))}
        if c['type'].startswith('TYPE_PKMN'):
            c.update(kind='pokemon', hp=int(field(b, 'hp')), stage=field(b, 'stage'), pre=field(b, 'pre-evo name'),
                     retreat=int(field(b, 'retreat cost')), weakness=field(b, 'weakness'),
                     resistance=field(b, 'resistance'), level=int(field(b, 'level')), dark=field(b, 'is Dark') == 'TRUE',
                     species=field(b, 'Pokedex number'))
            atks = []
            for part in re.split(r';\s*attack \d', b)[1:]:
                m = re.search(r'energy ([^;\n]*?)\s*; energies.*?tx (\w+) ; name.*?db (\d+) ; damage\s*\n\s*db (\w+) ; '
                              r'category\s*\n\s*dw (\w+) ; effect commands\s*\n\s*db ([\w| ]+) ; flags 1\s*\n\s*db ([\w| ]+) ; '
                              r'flags 2\s*\n\s*db ([\w| ]+) ; flags 3\s*\n\s*db (\w+) ; effect param\s*\n\s*db (\w+) ; animation',
                              part, re.S)
                if m and m.group(2) != 'NONE':
                    atks.append({'energy': energy(m.group(1)), 'label': m.group(2), 'damage': int(m.group(3)),
                                 'category': m.group(4), 'effect': m.group(5),
                                 'flags': [m.group(6).strip(), m.group(7).strip(), m.group(8).strip()],
                                 'parameter': m.group(9), 'animation': m.group(10)})
            c['attacks'] = atks
        else:
            c.update(kind='energy' if c['type'].startswith('TYPE_ENERGY') else 'trainer')
            m = re.search(r'dw (\w+) ; effect commands', b)
            c['effect'] = m.group(1) if m else 'NONE'
        cards.append(c)
    by_name_label = {}
    for c in cards:
        by_name_label.setdefault(c['name_label'], c)
    # Each card's Bulbapedia page: the one at its index, unless a same-named card's page there fits it better (the
    # list and the disassembly order a few same-named cards differently): name, level and HP decide
    by_index = dict(text)
    text = {}
    for c in cards:
        here = by_index[str(c['gb2_index'])]
        if c['kind'] == 'pokemon':
            near = [by_index[str(i)] for i in range(max(1, c['gb2_index'] - 4), min(445, c['gb2_index'] + 4) + 1)]
            fit = [p for p in near if norm(p['name']) == norm(here['name']) and p.get('level') == str(c['level'])
                   and p.get('hp') == str(c['hp'])]
            if len(fit) == 1:
                here = fit[0]
        text[str(c['gb2_index'])] = here
        c['text_index'] = next(int(k) for k, v in by_index.items() if v is here)

    # Which of them are the first game's cards
    def gb1_match(c):
        if c['kind'] == 'pokemon':
            for o in gb1:
                if o['kind'] == 'pokemon' and norm(o['name'].replace('Ninetails', 'Ninetales')) == norm(text[str(c['gb2_index'])]['name']) \
                        and o['level'] == c['level'] and o['hp'] == c['hp']:
                    return o['id']
            return None
        for o in gb1:
            if o['kind'] == c['kind'] and o['label'] == c['label']:
                return o['id']
        return None

    next_id = max([229] + [v + 1 for v in ids.values()])
    for c in cards:
        old = gb1_match(c)
        if old:
            c['id'] = old
            continue
        if c['label'] not in ids:
            ids[c['label']] = next_id
            next_id += 1
        c['id'] = ids[c['label']]
    IDS.write_text(json.dumps(ids, indent=1, sort_keys=True) + '\n')

    # The added cards, in the catalog's own format
    def printing(c):
        """The English printing with the same name, HP and attack names (or Trainer name), if any"""
        t = text[str(c['gb2_index'])]
        for key, e in english.items():
            if norm(e['name']) != norm(t['name']):
                continue
            if c['kind'] != 'pokemon':
                return key
            if str(e.get('hp')) != str(c['hp']):
                continue
            names = sorted(norm(a['name']) for a in e.get('attacks', []) + e.get('abilities', []))
            if names == sorted(norm(a['name']) for a in t['attacks']):
                return key
        return None

    added = []
    for c in cards:
        if c['id'] <= 228:
            continue
        t = text[str(c['gb2_index'])]
        eng = printing(c)
        out = {'id': c['id'], 'symbol': re.sub(r'[^A-Z0-9_]', '_', c['const']), 'label': c['label'],
               'name': plain(t['name']), 'kind': c['kind'], 'type': c['type'], 'rarity': c['rarity'],
               'sets': f'{c["gb2_set"]} | {c["real_set"]}', 'gb2_index': c['gb2_index'], 'text_index': c['text_index'],
               'gb2_set': c['gb2_set'],
               'real_set': c['real_set'], 'english_printing': eng, 'original_art': f'graphics/tcg/original/{c["id"]:03d}.png'}
        if c['kind'] == 'pokemon':
            pre = by_name_label.get(c['pre'])
            out.update(hp=c['hp'], stage=c['stage'], dark=c['dark'], retreat=c['retreat'], weakness=c['weakness'],
                       resistance=c['resistance'], level=c['level'],
                       evolves_from=plain(text[str(pre['gb2_index'])]['name']) if pre else '',
                       species=int(c['species'].replace('DEX_', '')) if c['species'] and c['species'].isdigit() else c['species'],
                       flavor=plain(t['flavor']), category=plain(t['category']))
            # Bulbapedia's attacks by name (a Pokemon Power isn't always listed first there)
            pool = list(t['attacks'])
            attacks = []
            for a in c['attacks']:
                want = norm(words(a['label']))
                pick = next((x for x in pool if norm(x['name']) == want), None)
                if pick is None:
                    pick = next((x for x in pool if want in norm(x['name']) or norm(x['name']) in want), None)
                if pick is None and pool:
                    pick = pool[0]
                if pick:
                    pool.remove(pick)
                attacks.append({'name': plain(pick['name']) if pick else words(a['label']),
                                'description': plain(pick['text']) if pick else '', 'energy': a['energy'],
                                'damage': a['damage'], 'category': a['category'], 'effect': a['effect'],
                                'flags': a['flags'], 'parameter': a['parameter'], 'animation': a['animation']})
            out['attacks'] = attacks
        else:
            desc = t['text']
            if not desc and eng:
                desc = ' '.join(english[eng].get('rules', []))
            out.update(effect=c['effect'], description=plain(desc))
        added.append(out)
    # species: GB2 writes DEX_<NAME> (constants/pokedex_constants.asm: const_def entries with their value in the
    # comment, DEF ... EQU for the later ones)
    dex = {}
    for m in re.finditer(r'(?:const (DEX_\w+)\s*; \$([0-9a-f]+))|(?:DEF (DEX_\w+)\s+EQU\s+(\d+))', (s / 'constants/pokedex_constants.asm').read_text()):
        dex[m.group(1) or m.group(3)] = int(m.group(2), 16) if m.group(1) else int(m.group(4))
    species = {c['id']: c.get('species') for c in cards}
    for a in added:
        if a['kind'] == 'pokemon':
            a['species'] = dex[species[a['id']]]

    catalog['cards'] = [x for x in catalog['cards'] if x['id'] <= 228] + sorted(added, key=lambda x: x['id'])
    # The first game's cards that GB2 sells: their booster in that game
    gb2_set = {c['id']: c['gb2_set'] for c in cards}
    gb2_index = {c['id']: c['gb2_index'] for c in cards}
    text_index = {c['id']: c['text_index'] for c in cards}
    for x in catalog['cards']:
        if x['id'] <= 228:
            x['gb2_set'] = gb2_set.get(x['id'])
            x['gb2_index'] = gb2_index.get(x['id'])
            x['text_index'] = text_index.get(x['id'])

    # GB2's boosters: every card each one can hold
    catalog['gb2_packs'] = [{'name': PACK_NAMES[i], 'set': p,
                             'cards': sorted(c['id'] for c in cards if c['gb2_set'] == p)} for i, p in enumerate(PACKS)]
    # GB2's decks (60 cards each), by their label's words
    decks_src = (s / 'data/decks.asm').read_text()
    const_id = {c['const']: c['id'] for c in cards}
    gb2_decks = []
    for label, body in blocks(decks_src, 'Deck').items():
        items = re.findall(r'card_item (\w+),\s*(\d+)', body)
        deck = [const_id[k] for k, n in items for _ in range(int(n))]
        if len(deck) == 60:
            gb2_decks.append({'label': label, 'name': words(label), 'cards': sorted(deck)})
    catalog['gb2_decks'] = gb2_decks
    catalog['gb2_revision'] = REVISION
    CATALOG.write_text(json.dumps(catalog, indent=1) + '\n')

    # The added cards' Game Boy pictures (8-bit palette PNGs, like the first game's)
    from PIL import Image
    ART.mkdir(parents=True, exist_ok=True)
    for c in cards:
        if c['id'] > 228 and c['gfx']:
            im = Image.open(s / 'gfx/cards' / c['gfx'] / 'regular.png').convert('RGB')
            im.quantize(colors=16, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE).save(ART / f'{c["id"]:03d}.png', optimize=True)
    print(f'tools/tcg/catalog.json: {len(added)} GB2 cards added (ids 229-{max(a["id"] for a in added)}), '
          f'{len(gb2_decks)} decks, {len(PACKS)} boosters; '
          f'{sum(1 for a in added if a["english_printing"])} printed in English')


if __name__ == '__main__':
    main()
