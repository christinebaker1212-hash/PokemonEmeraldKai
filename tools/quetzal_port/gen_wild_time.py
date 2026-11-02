#!/usr/bin/env python3
"""MORNING / DAY / NIGHT grass encounters for JOHTO, KANTO and HOENN, as SINNOH has them (Platinum's way: grass slots
2 and 3 hold the time's POKeMON). -> src/data/wild_time.h

    python3 tools/quetzal_port/gen_wild_time.py CRYSTAL_DIR

CRYSTAL_DIR holds pokecrystal's data/wild/johto_grass.asm and kanto_grass.asm (raw.githubusercontent.com/pret/
pokecrystal/master/data/wild/). For each JOHTO / KANTO map Crystal has a grass table for: the POKeMON Crystal shows
only in the morning (not by day) fill slots 2 and 3 in the morning, the night-only ones at night, heaviest Crystal slot
first; by day the slots stay as the table has them (so every POKeMON found today is still found). HOENN has no
original times: a species Crystal shows only at night anywhere is nocturnal, only in the morning / day diurnal; a
HOENN table's own nocturnal species fill slots 2 and 3 at night, its diurnal ones in the morning (nothing new, nothing
gone). Only species of the table's own generation run are used (Crystal's are Gen 1-2).
"""
import json
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
WEIGHTS = [30, 30, 20, 10, 5, 4, 1]   # Crystal's 7 grass slots


def read_crystal(path):
    out = {}
    for m in re.finditer(r'def_grass_wildmons (\w+)\n(.*?)end_grass_wildmons', open(path).read(), re.S):
        mons = re.findall(r'db\s+\d+,\s*(\w+)', m.group(2))
        if len(mons) == 21:
            out[m.group(1)] = {'morn': mons[0:7], 'day': mons[7:14], 'nite': mons[14:21]}
    return out


def camel(name):
    return ''.join(p.capitalize() if not p[0].isdigit() else p.upper() for p in name.split('_'))


def species_const(name, known):
    n = 'SPECIES_' + name.replace('__', '_')
    fixes = {'SPECIES_NIDORAN_F': 'SPECIES_NIDORAN_F', 'SPECIES_NIDORAN_M': 'SPECIES_NIDORAN_M',
             'SPECIES_MR_MIME': 'SPECIES_MR_MIME', 'SPECIES_FARFETCH_D': 'SPECIES_FARFETCHD'}
    n = fixes.get(n, n)
    return n if n in known else None


def special(t, when):
    """The species of time WHEN that the day doesn't have, heaviest first."""
    w = {}
    for i, s in enumerate(t[when]):
        if s not in t['day']:
            w[s] = w.get(s, 0) + WEIGHTS[i]
    return [s for s, _ in sorted(w.items(), key=lambda kv: -kv[1])]


def main():
    cdir = sys.argv[1]
    known = set(re.findall(r'#define (SPECIES_\w+)', open(os.path.join(REPO, 'include/constants/species.h')).read()))
    crystal = {}
    for f in ('johto_grass.asm', 'kanto_grass.asm'):
        crystal.update(read_crystal(os.path.join(cdir, f)))
    # nocturnal / diurnal species by Crystal's tables
    seen = {}
    for t in crystal.values():
        for when in ('morn', 'day', 'nite'):
            for s in t[when]:
                seen.setdefault(s, set()).add(when)
    noct = {species_const(s, known) for s, w in seen.items() if w == {'nite'}} - {None}
    diur = {species_const(s, known) for s, w in seen.items() if 'nite' not in w} - {None}

    groups = json.load(open(os.path.join(REPO, 'data/maps/map_groups.json')))
    ids = {}
    for gi, gn in enumerate(groups['group_order']):
        for mi, m in enumerate(groups[gn]):
            ids[m] = (gi, mi)
    wild = json.load(open(os.path.join(REPO, 'src/data/wild_encounters.json')))
    land = {}
    for g in wild['wild_encounter_groups']:
        if g['label'] != 'gWildMonHeaders':
            continue
        for e in g['encounters']:
            if 'land_mons' in e:
                land[e['map']] = [m['species'] for m in e['land_mons']['mons']]

    mapid = {m: json.load(open(os.path.join(REPO, 'data/maps', m, 'map.json')))['id'] for m in ids
             if os.path.exists(os.path.join(REPO, 'data/maps', m, 'map.json'))}
    def norm(n):
        return re.sub(r'[^a-z0-9]', '', n.lower()).replace('mount', 'mt')
    bynorm = {}
    for m in ids:
        for region in ('Johto_', 'Kanto_'):
            if m.startswith(region):
                bynorm.setdefault(norm(m[len(region):]), []).append(m)
    rows, missing = [], []
    for cname, t in sorted(crystal.items()):
        found = None
        key = norm(cname)
        for m in bynorm.get(key, []) + bynorm.get(key + '1f', []):
            if mapid.get(m) in land:
                found = (m, mapid[m])
                break
        if not found:
            missing.append(cname)
            continue
        m, const = found
        base = land[const][2:4]
        morn = [species_const(s, known) for s in special(t, 'morn')]
        nite = [species_const(s, known) for s in special(t, 'nite')]
        morn = [s for s in morn if s] + base
        nite = [s for s in nite if s] + base
        if morn[:2] == base and nite[:2] == base:
            continue
        rows.append((m, ids[m], base, morn[:2], nite[:2]))

    hoenn = 0
    for const, mons in sorted(land.items()):
        if 'JOHTO' in const or 'KANTO' in const or 'SINNOH' in const:
            continue
        name = next((m for m, i in mapid.items() if i == const), None)
        base = mons[2:4]
        n = [s for s in dict.fromkeys(mons) if s in noct]
        d = [s for s in dict.fromkeys(mons) if s in diur]
        if not n and not d:
            continue
        morn = (d + base)[:2] if d else base
        nite = (n + base)[:2] if n else base
        if morn == base and nite == base:
            continue
        rows.append((name, ids[name], base, morn, nite))
        hoenn += 1

    out = ['// Generated by tools/quetzal_port/gen_wild_time.py from pokecrystal\'s grass tables: MORNING / NIGHT grass',
           '// slots 2-3 for JOHTO, KANTO and HOENN (mp_region.c MP_TimeLandSlot; by day the table\'s own)',
           'static const struct MpTimedSlots {u8 group, num; u16 base[2], morn[2], nite[2];} sMpTimedSlots[] = {']
    for m, (gi, mi), base, morn, nite in rows:
        out.append('    {%d, %d, {%s}, {%s}, {%s}},   // %s' % (gi, mi, ', '.join(base), ', '.join(morn), ', '.join(nite), m))
    out.append('};')
    open(os.path.join(REPO, 'src/data/wild_time.h'), 'w').write('\n'.join(out) + '\n')
    print('%d maps (%d HOENN); Crystal tables not matched: %s' % (len(rows), hoenn, ' '.join(missing)))


if __name__ == '__main__':
    main()
