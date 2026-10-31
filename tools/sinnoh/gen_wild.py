#!/usr/bin/env python3
"""SINNOH's wild POKéMON (docs/SINNOH_HANDOFF.md step 3): each kept place's encounter table from pret/pokeplatinum
(res/field/encounters/<wildEncountersArchiveID>.json: species and levels) as the game's own wild tables, entries of
src/data/wild_encounters.json's gWildMonHeaders, as JOHTO's and KANTO's are.

    python3 -I tools/sinnoh/gen_wild.py POKEPLATINUM_DIR     (run from the repository; safe to run again)

Platinum's slots line up with Emerald's: grass has the same 12 slots and rates, surfing the same 5. Fishing: Emerald
has 10 slots (OLD ROD 2, GOOD ROD 3, SUPER ROD 5), Platinum 5 a rod: the first 2 / 3 / 5 of each. Platinum's extra
grass tables (by day and night, swarms, the POKé RADAR, a GBA game in the slot) aren't used yet: the base table is
what plays.
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from source import open_source  # noqa: E402
from places import places  # noqa: E402
from gen_maps import ident  # noqa: E402

LABEL = 'gSinnoh_'


def table(rate, mons, take=None):
    mons = [m for m in mons if m['species'] != 'SPECIES_NONE']
    if not rate or not mons:
        return None
    return {'encounter_rate': rate, 'mons': mons}


def land(d):
    mons = [{'min_level': e['level'], 'max_level': e['level'], 'species': e['species']} for e in d['land_encounters']]
    if len(mons) != 12 or any(m['species'] == 'SPECIES_NONE' for m in mons):
        return None
    return table(d['land_rate'], mons)


def ranged(e):
    return {'min_level': min(e['level_min'], e['level_max']), 'max_level': max(e['level_min'], e['level_max']),
            'species': e['species']}


def water(d):
    mons = [ranged(e) for e in d['surf_encounters']]
    if len(mons) != 5 or any(m['species'] == 'SPECIES_NONE' for m in mons):
        return None
    return table(d['surf_rate'], mons)


def fishing(d):
    old, good, sup = d['old_rod_encounters'][:2], d['good_rod_encounters'][:3], d['super_rod_encounters'][:5]
    mons = [ranged(e) for e in old + good + sup]
    if len(mons) != 10 or any(m['species'] == 'SPECIES_NONE' for m in mons):
        return None
    return table(max(d['old_rod_rate'], d['good_rod_rate'], d['super_rod_rate']), mons)


def main():
    root = sys.argv[1]
    src = open_source(root)
    path = 'src/data/wild_encounters.json'
    data = json.load(open(path))
    headers = data['wild_encounter_groups'][0]
    headers['encounters'] = [e for e in headers['encounters'] if not e.get('base_label', '').startswith(LABEL)]
    added = 0
    for p in sorted(places(src, root), key=lambda p: p['header']):
        name = p['fields'].get('wildEncountersArchiveID')
        file = os.path.join(root, 'res/field/encounters', (name or '') + '.json')
        if not name or not os.path.exists(file):
            continue
        d = json.load(open(file))
        const, camel = ident(p['header'])
        entry = {'map': 'MAP_SINNOH_' + const, 'base_label': LABEL + camel}
        for key, t in (('land_mons', land(d)), ('water_mons', water(d)), ('fishing_mons', fishing(d))):
            if t:
                entry[key] = t
        if len(entry) > 2:
            headers['encounters'].append(entry)
            added += 1
    json.dump(data, open(path, 'w'), indent=2)
    open(path, 'a').write('\n')
    print('%d SINNOH places with wild POKéMON' % added)


if __name__ == '__main__':
    main()
