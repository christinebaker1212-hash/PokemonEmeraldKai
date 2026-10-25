#!/usr/bin/env python3
"""Reference data from Pokémon Red, Crystal and FireRed for capping the rip to a generation (gencap.py).

    python3 tools/quetzal_rip/gen_refs.py REF_DIR [OUT]      (default tools/quetzal_rip/gen_refs.json)

REF_DIR holds clones of pret/pokered, pret/pokecrystal and pret/pokefirered (any depth, only data/ and src/data/
are read). The output is committed, so gencap.py runs without the clones.

Everything is keyed by a normalized name: lowercase letters and digits only (ROUTE_29, Route29 and
MAP_JOHTO_ROUTE29 all become "route29"; MR__MIME and SPECIES_MR_MIME become "mrmime").

Output:
  wild:     {game: {map: {land|water|fish|rock|headbutt: [species...]}}}  (Crystal: grass morn/day/nite, water,
            the map's fishing group, its headbutt tree set, rock smash = TreeMonSet_Rock where the map has rocks is
            not recorded, so rock pools are Crystal's one set for every map; Red: grass, water, super rod, good rod;
            FireRed: land, water, rock smash, fishing, LeafGreen too)
  trainers: {game: [{class, name, party: [[level, species], ...]}]}  (Red has no names: class + index)
  trades:   {game: {map: [[requested, offered], ...]}}
"""
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def key(s):
    s = re.sub(r'^(MAP_|SPECIES_|TRAINER_CLASS_)', '', s)
    s = s.replace('♀', 'F').replace('♂', 'M').replace('é', 'e').replace('É', 'E')
    return re.sub(r'[^a-z0-9]', '', s.lower())


def camel_key(s):
    return key(s)


SPECIES_LINE = re.compile(r'^\s*db\s+(\d+)\s*,\s*([A-Z][A-Z0-9_]*)')


def crystal(root):
    wild = {}

    def add(m, kind, sp):
        lst = wild.setdefault(key(m), {}).setdefault(kind, [])
        if key(sp) not in lst:
            lst.append(key(sp))

    for f, kind in (('johto_grass', 'land'), ('kanto_grass', 'land'), ('johto_water', 'water'), ('kanto_water', 'water')):
        cur = None
        for line in open(os.path.join(root, 'data', 'wild', f + '.asm')):
            m = re.match(r'\s*def_(grass|water)_wildmons\s+(\w+)', line)
            if m:
                cur = m.group(2)
                continue
            if 'end_' in line:
                cur = None
                continue
            m = SPECIES_LINE.match(line)
            if cur and m:
                add(cur, kind, m.group(2))

    # fishing: FishGroups -> labels; TimeFishGroups; maps.asm gives each map its group
    fish_src = open(os.path.join(root, 'data', 'wild', 'fish.asm')).read()
    order = re.findall(r'fishgroup [^,]+,\s*\.(\w+)_Old', fish_src)
    time_groups = []
    tg = fish_src.split('TimeFishGroups:')[1] if 'TimeFishGroups:' in fish_src else ''
    for line in tg.split('\n'):
        m = re.match(r'\s*db\s+([A-Z_0-9]+)\s*,\s*\d+\s*,\s*([A-Z_0-9]+)\s*,\s*\d+', line)
        if m:
            time_groups.append([m.group(1), m.group(2)])
    groups = {}
    label = None
    for line in fish_src.split('\n'):
        m = re.match(r'^\.(\w+)_(Old|Good|Super):', line)
        if m:
            label = m.group(1)
            continue
        if line.startswith('TimeFishGroups'):
            label = None
        if label is None:
            continue
        m = re.match(r'\s*db\s+[^,]+,\s*([A-Z][A-Z0-9_]*)\s*,', line)
        if m:
            groups.setdefault(label, []).append(m.group(1))
        m = re.search(r'time_group (\d+)', line)
        if m and int(m.group(1)) < len(time_groups):
            groups.setdefault(label, []).extend(time_groups[int(m.group(1))])
    fish_names = {}
    for i, lab in enumerate(order):
        fish_names['FISHGROUP_' + re.sub(r'(?<!^)(?=[A-Z])', '_', lab).upper()] = lab
    for line in open(os.path.join(root, 'data', 'maps', 'maps.asm')):
        m = re.match(r'\s*map (\w+),.*,\s*(FISHGROUP_\w+)', line)
        if not m or m.group(2) == 'FISHGROUP_NONE':
            continue
        lab = None
        g = m.group(2)[len('FISHGROUP_'):].replace('_', '').lower()
        for l2 in order:
            if l2.replace('_', '').lower() == g:
                lab = l2
        for sp in groups.get(lab, []):
            if sp != 'NO_MON':
                add(m.group(1), 'fish', sp)

    # headbutt trees
    tm = open(os.path.join(root, 'data', 'wild', 'treemons.asm')).read()
    sets = {}
    for part in re.split(r'\nTreeMonSet_', tm)[1:]:
        name = part.split(':')[0]
        sets[name] = [m.group(1) for m in re.finditer(r'db\s+\d+,\s*([A-Z][A-Z0-9_]*)\s*,\s*\d+', part)]
    for line in open(os.path.join(root, 'data', 'wild', 'treemon_maps.asm')):
        m = re.match(r'\s*treemon_map (\w+),\s*TREEMON_SET_(\w+)', line)
        if m:
            nm = m.group(2).capitalize()
            for sp in sets.get(nm, []):
                add(m.group(1), 'headbutt', sp)
    wild['_any'] = {'rock': [key(s) for s in dict.fromkeys(sets.get('Rock', []))]}

    trainers = []
    cls, name = None, None
    for line in open(os.path.join(root, 'data', 'trainers', 'parties.asm')):
        m = re.match(r'^(\w+)Group:', line)
        if m:
            cls = m.group(1)
            continue
        m = re.match(r'\s*db "([^@"]*)@"', line)
        if m:
            name = m.group(1)
            trainers.append({'class': cls, 'name': name, 'party': []})
            continue
        m = SPECIES_LINE.match(line)
        if m and trainers:
            trainers[-1]['party'].append([int(m.group(1)), key(m.group(2))])

    trades = {}
    consts = re.findall(r'const (NPC_TRADE_\w+)', open(os.path.join(root, 'constants', 'npc_trade_constants.asm')).read())
    entries = re.findall(r'npctrade \w+,\s*(\w+),\s*(\w+)', open(os.path.join(root, 'data', 'events', 'npc_trades.asm')).read())
    for f in glob.glob(os.path.join(root, 'maps', '*.asm')):
        for m in re.finditer(r'trade (NPC_TRADE_\w+)', open(f).read()):
            i = consts.index(m.group(1))
            trades.setdefault(key(os.path.basename(f)[:-4]), []).append([key(entries[i][0]), key(entries[i][1])])
    return wild, trainers, trades


def red(root):
    wild = {}
    labels = {}
    for line in open(os.path.join(root, 'data', 'wild', 'grass_water.asm')):
        m = re.match(r'\s*dw (\w+)\s*;\s*(\w+)', line)
        if m and m.group(1) != 'NothingWildMons':
            labels[m.group(1)] = m.group(2)
    for f in glob.glob(os.path.join(root, 'data', 'wild', 'maps', '*.asm')):
        src = open(f).read()
        lab = re.search(r'^(\w+):', src, re.M).group(1)
        if lab not in labels:
            continue
        kind = None
        for line in src.split('\n'):
            if 'def_grass_wildmons' in line:
                kind = 'land'
            elif 'def_water_wildmons' in line:
                kind = 'water'
            m = SPECIES_LINE.match(line)
            if kind and m:
                lst = wild.setdefault(key(labels[lab]), {}).setdefault(kind, [])
                if key(m.group(2)) not in lst:
                    lst.append(key(m.group(2)))
    src = open(os.path.join(root, 'data', 'wild', 'super_rod.asm')).read()
    groups = {}
    parts = re.split(r'\n\.(Group\d+):', src)
    for i in range(1, len(parts), 2):
        groups[parts[i]] = [m.group(1) for m in re.finditer(r'db\s+\d+,\s*([A-Z][A-Z0-9_]*)', parts[i + 1])]
    good = [m.group(1) for m in re.finditer(r'db\s+\d+,\s*([A-Z][A-Z0-9_]*)',
                                            open(os.path.join(root, 'data', 'wild', 'good_rod.asm')).read())]
    for m in re.finditer(r'dbw (\w+),\s*\.(Group\d+)', parts[0]):
        lst = wild.setdefault(key(m.group(1)), {}).setdefault('fish', [])
        for sp in ['MAGIKARP'] + good + groups.get(m.group(2), []):
            if key(sp) not in lst:
                lst.append(key(sp))

    trainers = []
    cls = None
    for line in open(os.path.join(root, 'data', 'trainers', 'parties.asm')):
        m = re.match(r'^(\w+)Data:', line)
        if m:
            cls = m.group(1)
            n = 0
            continue
        m = re.match(r'\s*db\s+(\$FF|\d+)\s*,(.*?)(;.*)?$', line)
        if not (m and cls):
            continue
        vals = [v.strip() for v in m.group(2).split(',') if v.strip()]
        n += 1
        party = []
        if m.group(1) == '$FF':
            for i in range(0, len(vals) - 1, 2):
                party.append([int(vals[i]), key(vals[i + 1])])
        else:
            lvl = int(m.group(1))
            party = [[lvl, key(v)] for v in vals if v != '0']
        trainers.append({'class': cls, 'name': '%s %d' % (cls, n), 'party': party})
    return wild, trainers


def firered(root):
    wild = {}
    d = json.load(open(os.path.join(root, 'src', 'data', 'wild_encounters.json')))
    kinds = {'land_mons': 'land', 'water_mons': 'water', 'rock_smash_mons': 'rock', 'fishing_mons': 'fish'}
    for g in d['wild_encounter_groups']:
        for e in g['encounters']:
            for k, kind in kinds.items():
                for mon in e.get(k, {}).get('mons', []):
                    lst = wild.setdefault(key(e['map']), {}).setdefault(kind, [])
                    if key(mon['species']) not in lst:
                        lst.append(key(mon['species']))

    parties = {}
    src = open(os.path.join(root, 'src', 'data', 'trainer_parties.h')).read()
    for m in re.finditer(r'(sParty_\w+)\[\] = \{(.*?)\n\};', src, re.S):
        parties[m.group(1)] = [[int(a), key(b)] for a, b in
                               re.findall(r'\.lvl = (\d+),\s*\.species = (SPECIES_\w+)', m.group(2))]
    trainers = []
    src = open(os.path.join(root, 'src', 'data', 'trainers.h')).read()
    for m in re.finditer(r'\.trainerClass = (TRAINER_CLASS_\w+),.*?\.trainerName = _\("([^"]*)"\),.*?\((sParty_\w+)\)',
                         src, re.S):
        trainers.append({'class': m.group(1)[len('TRAINER_CLASS_'):], 'name': m.group(2),
                         'party': parties.get(m.group(3), [])})

    offered = {}
    src = open(os.path.join(root, 'src', 'data', 'ingame_trades.h')).read()
    for m in re.finditer(r'\[(INGAME_TRADE_\w+)\] =\s*\{(.*?)\n    \}', src, re.S):
        sps = re.findall(r'\.species = (SPECIES_\w+)', m.group(2))         # FireRed and LeafGreen versions
        rqs = re.findall(r'\.requestedSpecies = (SPECIES_\w+)', m.group(2))
        offered[m.group(1)] = [[key(r), key(sp)] for r, sp in zip(rqs, sps)]
    trades = {}
    for f in glob.glob(os.path.join(root, 'data', 'maps', '*', 'scripts.inc')):
        for m in re.finditer(r'setvar VAR_0x8008, (INGAME_TRADE_\w+)', open(f).read()):
            for pair in offered.get(m.group(1), []):
                lst = trades.setdefault(key(os.path.basename(os.path.dirname(f))), [])
                if pair not in lst:
                    lst.append(pair)
    return wild, trainers, trades


def main():
    ref = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(HERE, 'gen_refs.json')
    cw, ct, ctr = crystal(os.path.join(ref, 'pokecrystal'))
    rw, rt = red(os.path.join(ref, 'pokered'))
    fw, ft, ftr = firered(os.path.join(ref, 'pokefirered'))
    data = {'_note': 'built by tools/quetzal_rip/gen_refs.py from pret/pokered, pokecrystal, pokefirered',
            'wild': {'crystal': cw, 'red': rw, 'firered': fw},
            'trainers': {'crystal': ct, 'red': rt, 'firered': ft},
            'trades': {'crystal': ctr, 'firered': ftr}}
    with open(out, 'w', encoding='utf-8') as fh:
        fh.write('{\n')
        for i, (k, v) in enumerate(data.items()):
            fh.write(' %s: ' % json.dumps(k))
            if isinstance(v, dict):
                fh.write('{\n' + ',\n'.join('  %s: %s' % (json.dumps(g), json.dumps(x, ensure_ascii=False))
                                            for g, x in v.items()) + '\n }')
            else:
                fh.write(json.dumps(v))
            fh.write(',\n' if i < len(data) - 1 else '\n')
        fh.write('}\n')
    print('wild maps: crystal %d, red %d, firered %d; trainers: crystal %d, red %d, firered %d; trades %d + %d' % (
        len(cw), len(rw), len(fw), len(ct), len(rt), len(ft), len(ctr), len(ftr)))


if __name__ == '__main__':
    main()
