"""The regional special trainers' decks and promos (docs/TCG_REGIONS.md step 5; gen_special_decks.py writes them
after HOENN's into src/tcg/tcg_decks.c, as TCG_SPECIAL_* 24 on: include/tcg/npc.h in this order).

Each deck is built from the finished cards (effects.ready) of the trainer's region's sets: the Pokemon lines of
their types (KANTO's leaders' own Gym cards first: "Brock's Onix"), the region's Trainers that help any deck, and
Energy for every attack's cost; then checked (60 cards, four of a name but basic Energy, every evolution's earlier
Stages, a Basic). Their promo is a finished Black Star promo of their type nobody else hands over (decision 3:
Wizards' for KANTO and JOHTO, Nintendo's for SINNOH and UNOVA), put in their deck.
"""
from collections import Counter
from pathlib import Path
import json
import sys

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent))
import effects as E  # noqa: E402

CARDS = [c for c in json.loads((HERE / 'cards.json').read_text())['cards']]
BY_ID = {c['id']: c for c in CARDS}
READY = {c['id'] for c in CARDS if E.ready(c)}
# The Game Boy games' Pokemon (all playable), for a promo's earlier Stages: as the regional cards' fields
for g in json.loads((HERE.parent / 'catalog.json').read_text())['cards']:
    if g['kind'] == 'pokemon' and g['id'] > 6 and not g['sets'].startswith('PROMOTIONAL'):
        stage = {'BASIC': 0, 'STAGE1': 1, 'STAGE2': 2}.get(g.get('stage'), 0)
        BY_ID.setdefault(g['id'], {'id': g['id'], 'name': g['name'], 'kind': 'pokemon', 'stage': stage,
                                   'evolvesFrom': g.get('evolves_from') or '', 'flags': [], 'attacks': [],
                                   'type': g['type'].replace('TYPE_PKMN_', '').title(), 'set': 'gb', 'src': 'gb'})
        READY.add(g['id'])
GB = [c for c in BY_ID.values() if c['set'] == 'gb']
BASIC_ENERGY = {'Grass': 1, 'Fire': 2, 'Water': 3, 'Lightning': 4, 'Fighting': 5, 'Psychic': 6}
SPECIAL_ENERGY = {'Darkness': 'neo1-104', 'Metal': 'neo1-19'}
SRC = {c['src']: c['id'] for c in CARDS}
REGION_SETS = {'KANTO': ('gym1', 'gym2'), 'JOHTO': ('neo1', 'neo2', 'neo3', 'neo4'),
               'SINNOH': ('ecard1', 'ecard2', 'ecard3'), 'UNOVA': ('ecard1', 'ecard2', 'ecard3')}
PROMO_SET = {'KANTO': 'basep', 'JOHTO': 'basep', 'SINNOH': 'np', 'UNOVA': 'np'}
# Trainers that help any deck, best first (by name; the first finished printing in the region's sets)
STAPLES = ['Professor Elm', 'Professor Oak\'s Research', 'Bill\'s Maintenance', 'Pokemon Fan Club', 'Copycat',
           'Energy Switch', 'Super Potion', 'Hyper Potion', 'Pokemon Nurse', 'Moo-Moo Milk', 'Energy Removal 2',
           'Pokemon Reversal', 'Energy Restore', 'Fisherman', 'Mary', 'Erika', 'Brock', 'Misty\'s Wrath',
           'Good Manners', 'Secret Mission', 'Juggler', 'Town Volunteers', 'Super Rod', 'Energy Charge', 'PokeGear',
           'Arcade Game', 'Sprout Tower', 'Pokemon Park', 'Focus Band', 'Gold Berry', 'Miracle Berry']

# (enum, region, title, types (first the main), owner word for KANTO's Gym cards or None, promo name wished or None)
SPECIALS = [
    # JOHTO: Crystal's leaders, ELITE FOUR, champion, rival and the two who stand in for STEVEN and WALLY
    ('FALKNER', 'JOHTO', 'FALKNER', ['Colorless', 'Lightning'], None, 'Flying Pikachu'),
    ('BUGSY', 'JOHTO', 'BUGSY', ['Grass'], None, 'Scizor'),
    ('WHITNEY', 'JOHTO', 'WHITNEY', ['Colorless'], None, 'Igglybuff'),
    ('MORTY', 'JOHTO', 'MORTY', ['Psychic'], None, 'Misdreavus'),
    ('CHUCK', 'JOHTO', 'CHUCK', ['Fighting'], None, 'Hitmontop'),
    ('JASMINE', 'JOHTO', 'JASMINE', ['Metal', 'Lightning'], None, None),
    ('PRYCE', 'JOHTO', 'PRYCE', ['Water'], None, 'Marill'),
    ('CLAIR', 'JOHTO', 'CLAIR', ['Water', 'Colorless'], None, None),
    ('WILL', 'JOHTO', 'WILL', ['Psychic'], None, 'Mewtwo'),
    ('KOGA_J', 'JOHTO', 'KOGA', ['Grass'], None, None),
    ('BRUNO_J', 'JOHTO', 'BRUNO', ['Fighting'], None, 'Machamp'),
    ('KAREN', 'JOHTO', 'KAREN', ['Darkness', 'Psychic'], None, None),
    ('LANCE_J', 'JOHTO', 'LANCE', ['Colorless', 'Water'], None, 'Ho-oh'),
    ('SILVER', 'JOHTO', 'SILVER', ['Darkness', 'Water'], None, None),
    ('RED', 'JOHTO', 'RED', ['Lightning', 'Fire'], None, 'Pikachu'),
    ('ELM', 'JOHTO', 'PROF. ELM', ['Grass', 'Fire'], None, 'Celebi'),
    # KANTO: the Gym leaders with their own Gym cards
    ('BROCK', 'KANTO', 'BROCK', ['Fighting'], "Brock's", None),
    ('MISTY', 'KANTO', 'MISTY', ['Water'], "Misty's", 'Psyduck'),
    ('SURGE', 'KANTO', 'LT. SURGE', ['Lightning'], "Lt. Surge's", 'Electabuzz'),
    ('ERIKA', 'KANTO', 'ERIKA', ['Grass'], "Erika's", 'Scyther'),
    ('KOGA_K', 'KANTO', 'KOGA', ['Grass'], "Koga's", None),
    ('SABRINA', 'KANTO', 'SABRINA', ['Psychic'], "Sabrina's", "Sabrina's Abra"),
    ('BLAINE', 'KANTO', 'BLAINE', ['Fire'], "Blaine's", 'Arcanine'),
    ('GIOVANNI', 'KANTO', 'GIOVANNI', ['Fighting', 'Grass'], "Giovanni's", 'Dark Persian'),
    ('LORELEI', 'KANTO', 'LORELEI', ['Water'], None, 'Articuno'),
    ('BRUNO_K', 'KANTO', 'BRUNO', ['Fighting'], None, None),
    ('AGATHA', 'KANTO', 'AGATHA', ['Psychic'], None, None),
    ('LANCE_K', 'KANTO', 'LANCE', ['Colorless', 'Lightning'], None, 'Zapdos'),
    ('BLUE', 'KANTO', 'BLUE', ['Fire', 'Water'], None, 'Rapidash'),
    ('OAK', 'KANTO', 'PROF. OAK', ['Colorless', 'Grass'], None, 'Jigglypuff'),
    ('BILL', 'KANTO', 'BILL', ['Colorless', 'Psychic'], None, "Team Rocket's Meowth"),
    # SINNOH: Platinum's leaders, ELITE FOUR, CYNTHIA, BARRY, PROF. ROWAN and JASMINE (visiting)
    ('ROARK', 'SINNOH', 'ROARK', ['Fighting'], None, 'Groudon ex'),
    ('GARDENIA', 'SINNOH', 'GARDENIA', ['Grass'], None, 'Treecko'),
    ('MAYLENE', 'SINNOH', 'MAYLENE', ['Fighting'], None, None),
    ('WAKE', 'SINNOH', 'WAKE', ['Water'], None, 'Mudkip'),
    ('FANTINA', 'SINNOH', 'FANTINA', ['Psychic'], None, 'Latias'),
    ('BYRON', 'SINNOH', 'BYRON', ['Metal', 'Fighting'], None, None),
    ('CANDICE', 'SINNOH', 'CANDICE', ['Water'], None, 'Kyogre ex'),
    ('VOLKNER', 'SINNOH', 'VOLKNER', ['Lightning'], None, 'Pikachu'),
    ('AARON', 'SINNOH', 'AARON', ['Grass'], None, 'Celebi'),
    ('BERTHA', 'SINNOH', 'BERTHA', ['Fighting'], None, 'Flygon'),
    ('FLINT', 'SINNOH', 'FLINT', ['Fire'], None, 'Torchic'),
    ('LUCIAN', 'SINNOH', 'LUCIAN', ['Psychic'], None, 'Latios'),
    ('CYNTHIA', 'SINNOH', 'CYNTHIA', ['Darkness', 'Fighting'], None, 'Jirachi'),
    ('BARRY', 'SINNOH', 'BARRY', ['Fire', 'Colorless'], None, 'Meowth'),
    ('ROWAN', 'SINNOH', 'PROF. ROWAN', ['Colorless', 'Water'], None, 'Whismur'),
    ('JASMINE_S', 'SINNOH', 'JASMINE', ['Metal', 'Water'], None, None),
    # UNOVA: Black's leaders (STRIATON's three), ELITE FOUR, ALDER, N, CHEREN, BIANCA, PROF. JUNIPER and IRIS
    ('CILAN', 'UNOVA', 'CILAN', ['Grass'], None, None),
    ('CHILI', 'UNOVA', 'CHILI', ['Fire'], None, None),
    ('CRESS', 'UNOVA', 'CRESS', ['Water'], None, None),
    ('LENORA', 'UNOVA', 'LENORA', ['Colorless'], None, None),
    ('BURGH', 'UNOVA', 'BURGH', ['Grass'], None, None),
    ('ELESA', 'UNOVA', 'ELESA', ['Lightning'], None, None),
    ('CLAY', 'UNOVA', 'CLAY', ['Fighting', 'Metal'], None, None),
    ('SKYLA', 'UNOVA', 'SKYLA', ['Colorless', 'Water'], None, None),
    ('BRYCEN', 'UNOVA', 'BRYCEN', ['Water'], None, None),
    ('DRAYDEN', 'UNOVA', 'DRAYDEN', ['Colorless', 'Water'], None, None),
    ('SHAUNTAL', 'UNOVA', 'SHAUNTAL', ['Psychic'], None, None),
    ('GRIMSLEY', 'UNOVA', 'GRIMSLEY', ['Darkness', 'Colorless'], None, None),
    ('CAITLIN', 'UNOVA', 'CAITLIN', ['Psychic'], None, None),
    ('MARSHAL', 'UNOVA', 'MARSHAL', ['Fighting'], None, None),
    ('ALDER', 'UNOVA', 'ALDER', ['Fire', 'Grass'], None, None),
    ('N', 'UNOVA', 'N', ['Lightning', 'Psychic'], None, None),
    ('CHEREN', 'UNOVA', 'CHEREN', ['Colorless', 'Fire'], None, None),
    ('BIANCA', 'UNOVA', 'BIANCA', ['Water', 'Grass'], None, None),
    ('JUNIPER', 'UNOVA', 'PROF. JUNIPER', ['Grass', 'Colorless'], None, None),
    ('IRIS', 'UNOVA', 'IRIS', ['Colorless', 'Fighting'], None, None),
]


# Their teams in their own games: lines with these Pokemon come first (any Owner's / Dark / Light printing)
TEAMS = {
    'FALKNER': 'Pidgey Pidgeotto Hoothoot Noctowl', 'BUGSY': 'Scyther Metapod Kakuna Ledyba Spinarak',
    'WHITNEY': 'Clefairy Miltank Snubbull Granbull', 'MORTY': 'Gastly Haunter Gengar Misdreavus',
    'CHUCK': 'Primeape Poliwrath Mankey Poliwag Machop', 'JASMINE': 'Magnemite Magneton Steelix Onix Skarmory',
    'PRYCE': 'Seel Dewgong Swinub Piloswine Delibird', 'CLAIR': 'Dragonair Dratini Kingdra Seadra Horsea',
    'WILL': 'Xatu Natu Jynx Exeggutor Slowbro Smoochum', 'KOGA_J': 'Ariados Spinarak Venomoth Forretress Muk Crobat Zubat',
    'BRUNO_J': 'Hitmontop Hitmonlee Hitmonchan Onix Machamp Tyrogue', 'KAREN': 'Umbreon Vileplume Gengar Murkrow Houndoom Houndour',
    'LANCE_J': 'Gyarados Dragonite Dragonair Aerodactyl Charizard', 'SILVER': 'Sneasel Golbat Crobat Magneton Gengar Feraligatr Totodile',
    'RED': 'Pikachu Espeon Snorlax Venusaur Charizard Blastoise', 'ELM': 'Chikorita Bayleef Meganium Togepi Togetic',
    'LORELEI': 'Dewgong Seel Cloyster Shellder Slowbro Jynx Lapras', 'BRUNO_K': 'Onix Hitmonchan Hitmonlee Machamp Machop',
    'AGATHA': 'Gengar Haunter Gastly Golbat Arbok', 'LANCE_K': 'Gyarados Dragonair Dragonite Aerodactyl',
    'BLUE': 'Pidgeot Alakazam Rhydon Arcanine Gyarados Exeggutor', 'OAK': 'Tauros Exeggutor Arcanine Blastoise Gyarados',
    'BILL': 'Eevee Vaporeon Jolteon Flareon', 'ROARK': 'Geodude Onix Cranidos Graveler Golem',
    'GARDENIA': 'Cherubi Turtwig Roserade Bulbasaur Oddish', 'MAYLENE': 'Meditite Machoke Machop Lucario',
    'WAKE': 'Gyarados Quagsire Wooper Floatzel Magikarp', 'FANTINA': 'Duskull Haunter Gastly Mismagius Misdreavus',
    'BYRON': 'Bronzor Steelix Onix Bastiodon Magnemite', 'CANDICE': 'Sneasel Piloswine Swinub Abomasnow Froslass',
    'VOLKNER': 'Jolteon Raichu Pikachu Luxray Electabuzz', 'AARON': 'Dustox Beautifly Heracross Scyther Venomoth',
    'BERTHA': 'Whiscash Gligar Golem Rhyhorn Rhydon Sandslash', 'FLINT': 'Houndoom Flareon Rapidash Magmar Charmander',
    'LUCIAN': 'Mr. Mime Girafarig Medicham Alakazam Abra', 'CYNTHIA': 'Tyranitar Larvitar Togetic Milotic Dragonite Gible',
    'BARRY': 'Pidgeot Heracross Snorlax Torchic Charmander', 'ROWAN': 'Pidgey Chansey Eevee Dratini',
    'JASMINE_S': 'Steelix Onix Magnemite Magneton Skarmory',
    # (UNOVA's own POKeMON aren't in these sets: the nearest of the older ones)
    'CILAN': 'Oddish Gloom Bellossom Sunkern Bellsprout', 'CHILI': 'Growlithe Arcanine Vulpix Magmar Slugma',
    'CRESS': 'Poliwag Poliwhirl Psyduck Golduck Totodile', 'LENORA': 'Sentret Furret Meowth Persian Kangaskhan Miltank',
    'BURGH': 'Scyther Pinsir Heracross Kakuna Beedrill Yanma', 'ELESA': 'Pikachu Raichu Mareep Flaaffy Ampharos Elekid',
    'CLAY': 'Sandshrew Sandslash Diglett Dugtrio Onix Steelix', 'SKYLA': 'Pidgey Pidgeotto Hoothoot Skarmory Swablu Altaria',
    'BRYCEN': 'Swinub Piloswine Seel Dewgong Jynx Lapras', 'DRAYDEN': 'Dratini Dragonair Dragonite Horsea Seadra Kingdra',
    'SHAUNTAL': 'Gastly Haunter Gengar Misdreavus Duskull', 'GRIMSLEY': 'Sneasel Houndour Houndoom Murkrow Umbreon',
    'CAITLIN': 'Natu Xatu Girafarig Abra Kadabra Ralts', 'MARSHAL': 'Machop Machoke Machamp Hitmonlee Hitmonchan',
    'ALDER': 'Ninetales Arcanine Heracross Scyther Magcargo', 'N': 'Electabuzz Jolteon Espeon Natu Xatu',
    'CHEREN': 'Eevee Snorlax Sentret Furret Cyndaquil', 'BIANCA': 'Totodile Croconaw Chikorita Marill Azumarill',
    'JUNIPER': 'Chikorita Bayleef Meganium Eevee Togepi', 'IRIS': 'Dratini Dragonair Dragonite Tyrogue Hitmontop',
}


def species(name):
    for prefix in ('Dark ', 'Light ', 'Shining '):
        if name.startswith(prefix):
            name = name[len(prefix):]
    if "'s " in name:
        name = name.split("'s ", 1)[1]
    return name.replace(' ex', '')


def ready(card_id):
    return card_id in READY


def pool(region):
    sets = REGION_SETS[region]
    return [c for c in CARDS if c['set'] in sets and ready(c['id'])]


def first_named(cards, name):
    for c in cards:
        if c['name'] == name:
            return c
    return None


def cost_types(c):
    need = Counter()
    for a in c.get('attacks', []):
        if a.get('category') == 'POKEMON_POWER':
            continue
        for t, k in a.get('cost', {}).items():
            if t != 'Colorless':
                need[t] = max(need[t], k)
    return need


def lines(cards, types, owner, team=(), taken=()):
    """Evolution lines of those types, best first: (score, [basic, stage1?, stage2?])"""
    mons = [c for c in cards if c['kind'] == 'pokemon']
    out = []
    for b in mons:
        if b['stage'] != 0 or b['type'] not in types or 'BABY' in b['flags']:
            continue
        if owner and not b['name'].startswith(owner) and any(m['name'].startswith(owner) for m in mons if m['type'] in types):
            continue
        line = [b]
        s1 = [m for m in mons if m['stage'] == 1 and m['evolvesFrom'] == b['name']]
        if s1:
            s1.sort(key=lambda m: (-(m['rarity'].startswith('Rare')), -m['hp'], m['id']))
            line.append(s1[0])
            s2 = [m for m in mons if m['stage'] == 2 and m['evolvesFrom'] == s1[0]['name']]
            if s2:
                s2.sort(key=lambda m: (-(m['rarity'].startswith('Rare')), -m['hp'], m['id']))
                line.append(s2[0])
        if any(c['type'] not in types + ['Colorless'] for c in line):
            continue
        score = sum(c['hp'] for c in line) + 40 * len(line) + 30 * any(c['rarity'].startswith('Rare') for c in line)
        score += 60 * (types.index(b['type']) == 0) + (100 if owner and b['name'].startswith(owner) else 0)
        score += 220 * any(species(c['name']) in team for c in line) - 180 * (b['name'] in taken)
        out.append((score, line))
    out.sort(key=lambda x: (-x[0], x[1][0]['id']))
    seen, unique = set(), []
    for score, line in out:
        key = tuple(c['name'] for c in line)
        if key[0] in seen:
            continue
        seen.add(key[0])
        unique.append(line)
    return unique


TAKEN = {}   # the Basics each region's trainers already use


def build(enum, region, types, owner, promo):
    cards = pool(region)
    taken = TAKEN.setdefault(region, set())
    counts = Counter()
    names = Counter()

    def add(c, n):
        n = min(n, 4 - names[c['name']])
        if 'SHINING' in c['flags']:
            n = min(n, 1 - sum(k for i, k in counts.items() if 'SHINING' in BY_ID[i]['flags']))
        if n > 0:
            counts[c['id']] += n
            names[c['name']] += n

    # the promo and what it evolves from
    if promo:
        add(promo, 1)
        prev = promo.get('evolvesFrom')
        while prev:
            p = first_named([c for c in cards if c['kind'] == 'pokemon'], prev) or \
                first_named([c for c in CARDS if c['kind'] == 'pokemon' and ready(c['id'])], prev) or first_named(GB, prev)
            if not p:
                return None
            add(p, 2)
            prev = p.get('evolvesFrom')
    # Pokemon lines: 4-3-2 for the best, 3-2-1 / 4 Basics for the rest, about 20 Pokemon
    for k, line in enumerate(lines(cards, types, owner, TEAMS.get(enum, '').split(), taken)):
        if sum(n for i, n in counts.items() if BY_ID[i]['kind'] == 'pokemon') >= 19:
            break
        copies = [4, 3, 2] if k == 0 else [3, 2, 1] if len(line) > 1 else [3]
        for c, n in zip(line, copies):
            add(c, n)
    if not any(BY_ID[i]['kind'] == 'pokemon' and BY_ID[i]['stage'] == 0 for i in counts):
        return None
    taken.update(BY_ID[i]['name'] for i in counts if BY_ID[i]['kind'] == 'pokemon' and BY_ID[i]['stage'] == 0)
    # Trainers: about 12 staples
    for name in STAPLES:
        if sum(n for i, n in counts.items() if BY_ID[i]['kind'] == 'trainer') >= 12:
            break
        t = first_named([c for c in cards if c['kind'] == 'trainer'], name)
        if t and not set(t['flags']) & {'STADIUM'}:
            add(t, 2 if 'SUPPORTER' not in t['flags'] else 3)
    # Energy to 60: every attack's cost covered, split by need (Darkness / Metal: their special Energy, 4 at most)
    need = Counter()
    for i in counts:
        for t, k in cost_types(BY_ID[i]).items():
            need[t] += k * counts[i]
    room = 60 - sum(counts.values())
    if room < 12:
        return None
    energy = Counter()
    for t in list(need):
        if t in SPECIAL_ENERGY:
            special = SRC[SPECIAL_ENERGY[t]]
            if not ready(special):
                return None
            energy[special] = 4
            room -= 4
            del need[t]
    total = sum(need.values()) or 1
    basics = [t for t in need if t in BASIC_ENERGY]
    if not basics:
        basics = [t for t in types if t in BASIC_ENERGY][:1] or ['Fighting']
        need = Counter({basics[0]: 1})
        total = 1
    left = room
    for t in sorted(basics, key=lambda t: -need[t]):
        n = max(4, round(room * need[t] / total))
        n = min(n, left)
        energy[BASIC_ENERGY[t]] += n
        left -= n
    main = max(basics, key=lambda t: need[t])
    energy[BASIC_ENERGY[main]] += left
    counts.update(energy)
    return +counts


def check(enum, counts):
    assert sum(counts.values()) == 60, (enum, sum(counts.values()))
    names = Counter()
    for i, n in counts.items():
        if i > 6:
            names[BY_ID[i]['name']] += n
    assert max(names.values()) <= 4, (enum, names.most_common(2))
    assert sum(n for i, n in counts.items() if i > 6 and 'SHINING' in BY_ID[i]['flags']) <= 1, enum
    present = {BY_ID[i]['name'] for i in counts if i > 6}
    assert any(BY_ID[i]['kind'] == 'pokemon' and BY_ID[i]['stage'] == 0 for i in counts if i > 6), enum
    for i in counts:
        if i > 6 and BY_ID[i]['kind'] == 'pokemon' and BY_ID[i]['evolvesFrom']:
            assert BY_ID[i]['evolvesFrom'] in present, (enum, BY_ID[i]['name'])
    for i in counts:
        assert i <= 6 or ready(i), (enum, BY_ID[i]['name'])


def promos():
    """Each special's promo (a card dict or None), one trainer each: first every wish that is finished, then a
    finished one of the main type nobody has, then (leaders and the like, not the rivals) any free Basic of their
    types"""
    used, out = set(), {}

    def free(region):
        return [c for c in CARDS if c['set'] == PROMO_SET[region] and ready(c['id']) and c['kind'] == 'pokemon'
                and c['id'] not in used]

    def take(enum, pick):
        if pick:
            used.add(pick['id'])
            out[enum] = pick

    for enum, region, title, types, owner, wish in SPECIALS:
        if wish:
            cands = free(region)
            take(enum, next((c for c in cands if c['name'] == wish and c['type'] in types + ['Colorless']), None)
                 or next((c for c in cands if c['name'] == wish), None))
    for enum, region, title, types, owner, wish in SPECIALS:
        if enum not in out:
            take(enum, next((c for c in free(region) if c['type'] == types[0]), None))
    for enum, region, title, types, owner, wish in SPECIALS:
        if enum not in out and title not in ('SILVER', 'BLUE', 'BARRY', 'CHEREN', 'BIANCA'):
            take(enum, next((c for c in free(region) if c['type'] in types + ['Colorless'] and c['stage'] == 0), None))
    return {enum: out.get(enum) for enum, *_ in SPECIALS}


def decks():
    """[(enum, title, counts or None, promo id or 0)] in SPECIALS' order"""
    out, picks = [], promos()
    for enum, region, title, types, owner, wish in SPECIALS:
        promo = picks[enum]
        counts = build(enum, region, types, owner, promo)
        if counts is None and promo:
            promo = None
            counts = build(enum, region, types, owner, None)
        if counts is not None:
            check(enum, counts)
        out.append((enum, title, counts, promo['id'] if promo else 0))
    return out


if __name__ == '__main__':
    for enum, title, counts, promo in decks():
        if counts is None:
            print('%-10s NO DECK' % enum)
            continue
        mons = sorted({BY_ID[i]['name'] for i in counts if i > 6 and BY_ID[i]['kind'] == 'pokemon'})
        print('%-10s promo %-22s %s' % (enum, BY_ID[promo]['name'] if promo else '-', ', '.join(mons)))
