#!/usr/bin/env python3
"""Special trainers' card decks (src/tcg/tcg_decks.c).

Each starts from one of the Game Boy game's decks (the Club Masters and Grand
Masters), adds the Pokemon lines that match the trainer's own team and their
promo card (with what it evolves from and the Energy its attacks need), then is
trimmed back to 60 cards and checked: four of a name at most, a Basic Pokemon,
every evolution's earlier stages present.
"""
from collections import Counter
from pathlib import Path
import json
import sys

ROOT = Path(__file__).resolve().parents[2]
CAT = json.loads((ROOT / 'tools/tcg/catalog.json').read_text())
CARDS = {c['id']: c for c in CAT['cards']}
BY_SYMBOL = {c['symbol']: c['id'] for c in CAT['cards']}
DECKS = {d['name']: d['cards'] for d in CAT['decks']}
TYPES = ['FIRE', 'GRASS', 'LIGHTNING', 'WATER', 'FIGHTING', 'PSYCHIC', 'COLORLESS']
ENERGY = {'GRASS': 1, 'FIRE': 2, 'WATER': 3, 'LIGHTNING': 4, 'FIGHTING': 5, 'PSYCHIC': 6}

# (enum, name, base deck, [(card, copies)], promo)
SPECIAL = [
    ('ROXANNE', 'ROXANNE', 'RockCrusher', [('ONIX', 2), ('KABUTO', 2), ('KABUTOPS', 1)], 'SUPER_ENERGY_RETRIEVAL'),
    ('BRAWLY', 'BRAWLY', 'FirstStrike', [('MACHOP', 3), ('MACHOKE', 2), ('MACHAMP', 1), ('HITMONCHAN', 2)], 'ELECTABUZZ_LV20'),
    ('WATTSON', 'WATTSON', 'ZappingSelfdestruct', [('MAGNEMITE_LV13', 3), ('MAGNETON_LV28', 2), ('VOLTORB', 2)], 'PIKACHU_LV16'),
    ('FLANNERY', 'FLANNERY', 'FireCharge', [('GROWLITHE', 3), ('PONYTA', 2), ('RAPIDASH', 1), ('MAGMAR_LV31', 1)], 'ARCANINE_LV34'),
    ('NORMAN', 'NORMAN', 'Anger', [('SNORLAX', 1), ('KANGASKHAN', 1), ('TAUROS', 2), ('CHANSEY', 1)], 'JIGGLYPUFF_LV12'),
    ('WINONA', 'WINONA', 'FlyinPokemon', [('PIDGEY', 2), ('PIDGEOTTO', 2), ('PIDGEOT_LV40', 1), ('DODUO', 2), ('DODRIO', 1)], 'FLYING_PIKACHU'),
    ('TATE_LIZA', 'TATE & LIZA', 'StrangePsyshock', [('MR_MIME', 2), ('JYNX', 2), ('DROWZEE', 2), ('HYPNO', 1)], 'MEW_LV8'),
    ('JUAN', 'JUAN', 'GoGoRainDance', [('HORSEA', 3), ('SEADRA', 2), ('STARYU', 2), ('STARMIE', 1)], 'SURFING_PIKACHU_LV13'),
    ('SIDNEY', 'SIDNEY', 'StrangePower', [('GASTLY_LV8', 2), ('HAUNTER_LV22', 2), ('GENGAR', 1)], 'MEWTWO_LV60'),
    ('PHOEBE', 'PHOEBE', 'Ghost', [('GASTLY_LV17', 2), ('HAUNTER_LV17', 2), ('GENGAR', 1)], 'MEW_LV15'),
    ('GLACIA', 'GLACIA', 'LegendaryArticuno', [('SEEL', 3), ('DEWGONG', 2), ('LAPRAS', 2)], 'ARTICUNO_LV37'),
    ('DRAKE', 'DRAKE', 'LegendaryDragonite', [('DRATINI', 3), ('DRAGONAIR', 2), ('AERODACTYL', 1)], 'DRAGONITE_LV41'),
    ('WALLACE', 'WALLACE', 'SoundoftheWaves', [('MAGIKARP', 3), ('GYARADOS', 2), ('LAPRAS', 2), ('TENTACOOL', 2), ('TENTACRUEL', 1)], None),
    ('STEVEN', 'STEVEN', 'PowerfulRonald', [('GEODUDE', 3), ('GRAVELER', 2), ('GOLEM', 1), ('ONIX', 2), ('AERODACTYL', 1)], 'MEWTWO_ALT_LV60'),
    ('RIVAL', 'RIVAL', 'BulbasaurAndFriends', [('IVYSAUR', 2)], 'VENUSAUR_LV64'),
    ('WALLY', 'WALLY', 'Pikachu', [('ABRA', 2), ('KADABRA', 1), ('CLEFAIRY', 2)], 'PIKACHU_ALT_LV16'),
    ('MAXIE', 'MAXIE', 'LegendaryMoltres', [('MAGMAR_LV24', 2), ('VULPIX', 2), ('NINETALES_LV35', 1)], None),
    ('ARCHIE', 'ARCHIE', 'WaterfrontPokemon', [('KRABBY', 2), ('KINGLER', 1), ('TENTACOOL', 2), ('TENTACRUEL', 1)], None),
    ('ANABEL', 'ANABEL', 'StrangePower', [('ALAKAZAM', 1), ('KADABRA', 2), ('ABRA', 3)], None),
    ('TUCKER', 'TUCKER', 'LegendaryZapdos', [('ELECTABUZZ_LV35', 2)], None),
    ('SPENSER', 'SPENSER', 'FlowerPower', [('EXEGGCUTE', 2), ('EXEGGUTOR', 1)], None),
    ('GRETA', 'GRETA', 'MusclesforBrains', [('HITMONLEE', 2), ('HITMONCHAN', 2)], None),
    ('NOLAND', 'NOLAND', 'Kaleidoscope', [('PORYGON', 2), ('DITTO', 1)], None),
    ('LUCY', 'LUCY', 'Poison', [('EKANS', 3), ('ARBOK', 2)], None),
]


def card(sym):
    return CARDS[BY_SYMBOL[sym]]


def name_of(i):
    return CARDS[i]['name']


def stage(i):
    return CARDS[i].get('stage', '')


def needed_types(counts):
    need = Counter()
    for i, n in counts.items():
        c = CARDS[i]
        if c['kind'] != 'pokemon':
            continue
        for a in c['attacks']:
            for t, k in a['energy'].items():
                if t in ENERGY:
                    need[t] = max(need[t], k)
    return need


def pre_evolutions(i, counts_by_name):
    """The cards an evolution card needs below it, by name."""
    out = []
    c = CARDS[i]
    while c['kind'] == 'pokemon' and c.get('stage') in ('STAGE1', 'STAGE2'):
        prev = c['evolves_from']
        out.append(prev)
        # (fossil Pokemon evolve from the MYSTERIOUS FOSSIL Trainer card)
        c = [x for x in CARDS.values() if x['name'] == prev][0]
    return out


ENERGY_TYPE = {v: k for k, v in ENERGY.items()}


def EnergyFloor(i, need):
    t = ENERGY_TYPE[i]
    return max(4, need[t] + 2) if t in need else 0


def build(base, adds, promo):
    counts = Counter(DECKS[base])
    protect = set()
    for sym, n in adds + ([(promo, 1)] if promo else []):
        i = BY_SYMBOL[sym]
        counts[i] = max(counts[i], n) if sym != promo else 1
        protect.add(i)
        # Earlier stages: two of each, the first printing in the base deck if any.
        for prev in pre_evolutions(i, None):
            have = [k for k in counts if CARDS[k]['name'] == prev]
            if have:
                k = have[0]
            else:
                k = min(x['id'] for x in CARDS.values() if x['name'] == prev and x['sets'].split(' | ')[0] != 'PROMOTIONAL')
            counts[k] = max(counts[k], 2)
            protect.add(k)
    # Four of a name at most.
    names = Counter()
    for i in sorted(counts, key=lambda k: (k not in protect, k)):
        if i <= 6:
            continue
        allowed = 4 - names[name_of(i)]
        if counts[i] > allowed:
            counts[i] = allowed
        names[name_of(i)] += counts[i]
    counts = +counts
    # Energy for every attack (four of each type an attack asks for, at least).
    need = needed_types(counts)
    for t, k in need.items():
        e = ENERGY[t]
        if counts[e] < max(4, k + 2):
            counts[e] = max(4, k + 2)

    def size():
        return sum(counts.values())

    def removable_pokemon():
        """Pokemon nobody needs below them, highest stage first."""
        names_needed = set()
        for i in counts:
            for prev in pre_evolutions(i, None):
                names_needed.add(prev)
        out = []
        for i in counts:
            c = CARDS[i]
            if (c['kind'] != 'pokemon' and c['name'] not in names_needed) or i in protect or c['kind'] == 'energy':
                continue
            same = sum(n for k, n in counts.items() if name_of(k) == c['name'])
            if c['name'] in names_needed and same <= 2:
                continue
            out.append(i)
        return sorted(out, key=lambda i: ({'STAGE2': 0, 'STAGE1': 1, 'BASIC': 2}.get(CARDS[i].get('stage'), 3), -counts[i], i))

    while size() > 60:
        fossil_needed = any(p == 'Mysterious Fossil' for k in counts for p in pre_evolutions(k, None))
        trainers = sorted((i for i in counts if CARDS[i]['kind'] == 'trainer' and i not in protect and counts[i] > 1
                           and not (fossil_needed and CARDS[i]['name'] == 'Mysterious Fossil' and counts[i] <= 2)),
                          key=lambda i: (-counts[i], i))
        energy = sorted((i for i in counts if i <= 6 and counts[i] > EnergyFloor(i, need)), key=lambda i: -counts[i])
        total_energy = sum(n for i, n in counts.items() if i <= 7)
        if trainers and counts[trainers[0]] >= 3:
            counts[trainers[0]] -= 1
        elif total_energy > 22 and energy:
            counts[energy[0]] -= 1
        elif removable_pokemon():
            counts[removable_pokemon()[0]] -= 1
        elif trainers:
            counts[trainers[0]] -= 1
        elif energy:
            counts[energy[0]] -= 1
        else:
            sys.exit('cannot trim ' + base)
        counts = +counts
    while size() < 60:
        main = max((i for i in counts if i <= 6), key=lambda i: counts[i])
        counts[main] += 1
    return counts


def check(name, counts):
    assert sum(counts.values()) == 60, name
    names = Counter()
    for i, n in counts.items():
        if i > 6:
            names[name_of(i)] += n
    assert max(names.values()) <= 4, (name, names.most_common(3))
    assert any(CARDS[i]['kind'] == 'pokemon' and CARDS[i]['stage'] == 'BASIC' for i in counts), name
    present = {name_of(i) for i in counts}
    for i in counts:
        for prev in pre_evolutions(i, None):
            assert prev in present, (name, name_of(i), prev)
    for t, k in needed_types(counts).items():
        assert counts[ENERGY[t]] >= k, (name, t)


def main():
    out = ['/* Generated by tools/tcg/gen_special_decks.py: the special trainers\' decks. */',
           '#include "tcg/tcg.h"',
           'const struct TcgDeckDefinition gTcgSpecialDecks[] = {']
    for enum, title, base, adds, promo in SPECIAL:
        counts = build(base, adds, promo)
        check(enum, counts)
        cards = []
        for i in sorted(counts):
            cards += [i] * counts[i]
        out.append('    {"%s", {%s}}, /* %s from %s */' % (enum, ','.join(map(str, cards)), title, base))
        if promo:
            assert counts[BY_SYMBOL[promo]] == 1
    out += ['};', 'const unsigned gTcgSpecialDeckCount = sizeof(gTcgSpecialDecks) / sizeof(gTcgSpecialDecks[0]);',
            '/* The promo card a first win over each hands over (0: none). */',
            'const uint16_t gTcgSpecialPromos[] = {' + ', '.join(str(BY_SYMBOL[p]) if p else '0' for _, _, _, _, p in SPECIAL) + '};', '']
    (ROOT / 'src/tcg/tcg_decks.c').write_text('\n'.join(out))
    print('Generated %d special decks' % len(SPECIAL))


main()
