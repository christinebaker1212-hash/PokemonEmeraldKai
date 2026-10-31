#!/usr/bin/env python3
"""HOENN's wild POKéMON and trainers' teams by GENERATION: src/data/hoenn_modern.h (mp_region.c).

    python3 tools/moemon_gen59/gen_hoenn_modern.py

JOHTO and KANTO take Quetzal's own tables as GENERATION rises (wild_qz_modern.h...); HOENN has none, so this makes them:
- Wild: each table gives one of its rarer slots to each generation 4-9 (land 5% 5% 4% 4% 1% 1%; water and ROCK SMASH
  5% 4% 1% twice over; fishing one slot per rod and two more), the common slots stay. The slot's newer POKéMON shares
  a type with the one it stands in for (its first type first) and is at the same evolution stage; never a legendary,
  mythical, ULTRA BEAST or paradox POKéMON, and no species twice in a table. The paradox POKéMON have their own
  places at GENERATION 9 (PARADOX: one each, the 1% land slot).
- Trainers: a team member has an even chance of a generation 4-9 of its own (about half of them in all): at that
  GENERATION and above it is a newer POKéMON chosen the same way (no two alike in a team). Gym Leaders', the Elite
  Four's, the Champion's, the teams' leaders' and the rival's last POKéMON (their ace) stay.
Deterministic (crc32 of the place), so a table never changes between runs.
"""
import json
import os
import re
import zlib

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
OUT = os.path.join(REPO, 'src', 'data', 'hoenn_modern.h')
WILD_SLOTS = {   # field -> {generation: slot}
    'land_mons': {4: 6, 5: 7, 6: 8, 7: 9, 8: 10, 9: 11},
    'water_mons': {4: 2, 5: 3, 6: 4, 7: 2, 8: 3, 9: 4},
    'rock_smash_mons': {4: 2, 5: 3, 6: 4, 7: 2, 8: 3, 9: 4},
    'fishing_mons': {4: 1, 5: 4, 6: 9, 7: 3, 8: 8, 9: 7},
}
FIELD_INFO = {'land_mons': 'LandMonsInfo', 'water_mons': 'WaterMonsInfo', 'rock_smash_mons': 'RockSmashMonsInfo',
              'fishing_mons': 'FishingMonsInfo'}
NOT_WILD = {  # evolve or are babies, but are legendary / mythical / Ultra Beasts / one of a kind
    'COSMOG', 'COSMOEM', 'SOLGALEO', 'LUNALA', 'NECROZMA', 'TYPE_NULL', 'SILVALLY', 'POIPOLE', 'NAGANADEL', 'KUBFU',
    'URSHIFU', 'MELTAN', 'MELMETAL', 'ZACIAN', 'ZAMAZENTA', 'ETERNATUS', 'CALYREX', 'GLASTRIER', 'SPECTRIER',
    'ENAMORUS', 'TERAPAGOS', 'OGERPON', 'OKIDOGI', 'MUNKIDORI', 'FEZANDIPITI', 'PECHARUNT', 'GIMMIGHOUL',
    'GHOLDENGO', 'MANAPHY', 'PHIONE', 'KORAIDON', 'MIRAIDON',
}
PARADOX = {  # the paradox POKéMON, wild nowhere else: the GENERATION 9 land slot (1%) of a fitting place
    'gRoute111': 'GREAT_TUSK', 'gDesertUnderpass': 'SANDY_SHOCKS', 'gRoute120': 'SCREAM_TAIL', 'gRoute119': 'BRUTE_BONNET',
    'gMtPyre_Summit': 'FLUTTER_MANE', 'gSafariZone_Northeast': 'SLITHER_WING', 'gSkyPillar_5F': 'ROARING_MOON',
    'gVictoryRoad_B1F': 'IRON_TREADS', 'gShoalCave_LowTideIceRoom': 'IRON_BUNDLE', 'gNewMauville_Inside': 'IRON_HANDS',
    'gMeteorFalls_B1F_1R': 'IRON_JUGULIS', 'gMagmaHideout_4F': 'IRON_MOTH', 'gNewMauville_Entrance': 'IRON_THORNS',
    'gArtisanCave_1F': 'IRON_VALIANT',
}
PSEUDO = {  # strong lines: wild only where the slot's POKéMON are Lv. 30 or more
    'GIBLE', 'GABITE', 'GARCHOMP', 'DEINO', 'ZWEILOUS', 'HYDREIGON', 'GOOMY', 'SLIGGOO', 'GOODRA', 'JANGMO_O',
    'HAKAMO_O', 'KOMMO_O', 'DREEPY', 'DRAKLOAK', 'DRAGAPULT', 'FRIGIBAX', 'ARCTIBAX', 'BAXCALIBUR', 'RIOLU', 'LUCARIO',
}
BOSS_CLASSES = {'TRAINER_CLASS_LEADER', 'TRAINER_CLASS_ELITE_FOUR', 'TRAINER_CLASS_CHAMPION',
                'TRAINER_CLASS_AQUA_LEADER', 'TRAINER_CLASS_MAGMA_LEADER', 'TRAINER_CLASS_RIVAL'}


def read(p):
    return open(os.path.join(REPO, p), encoding='utf-8').read()


def species_ids():
    return {m.group(1): int(m.group(2)) for m in
            re.finditer(r'#define\s+(SPECIES_\w+)\s+(\d+)\s*$', read('include/constants/species.h'), re.M)}


def generation(sid):
    if sid <= 151:
        return 1
    if sid <= 251:
        return 2
    if sid <= 276:
        return 0          # the old UNOWN letters
    if sid <= 411:
        return 3
    if sid <= 518:
        return 4
    nat = sid - 25
    for g, last in ((5, 649), (6, 721), (7, 809), (8, 905), (9, 1025)):
        if nat <= last:
            return g
    return 0


def species_info(ids):
    info = {}
    for f in ('src/data/pokemon/species_info.h', 'src/data/pokemon/species_info_gen4.h',
              'src/data/pokemon/species_info_gen59.h'):
        s = read(f)
        for m in re.finditer(r'\[(SPECIES_\w+)\]\s*=\s*\{(.*?)\n    \}', s, re.S):
            body = m.group(2)
            t = re.search(r'\.types\s*=\s*\{\s*(\w+),\s*(\w+)\s*\}', body)
            e = re.search(r'\.eggGroups\s*=\s*\{\s*(\w+),\s*(\w+)\s*\}', body)
            c = re.search(r'\.catchRate\s*=\s*(\d+)', body)
            if t and m.group(1) in ids:
                info[m.group(1)] = {'types': (t.group(1), t.group(2)), 'eggs': e.groups() if e else (),
                                    'catch': int(c.group(1)) if c else 255}
    return info


def evolutions():
    parent, children = {}, {}
    for f in ('src/data/pokemon/evolution.h', 'src/data/pokemon/evolution_gen4.h',
              'src/data/pokemon/evolution_gen59.h'):
        for m in re.finditer(r'\[(SPECIES_\w+)\]\s*=\s*\{(.*?)\}\},', read(f), re.S):
            for tgt in re.findall(r'(SPECIES_\w+)\s*\}', m.group(2) + '}'):
                if tgt != m.group(1):
                    parent.setdefault(tgt, m.group(1))
                    children.setdefault(m.group(1), set()).add(tgt)
    return parent, children


def main():
    ids = species_ids()
    info = species_info(ids)
    parent, children = evolutions()

    def stage(s):
        n = 0
        while s in parent and n < 3:
            s, n = parent[s], n + 1
        return n

    def legendary(s):
        i = info[s]
        name = s[len('SPECIES_'):]
        if name in NOT_WILD:
            return True
        no_eggs = 'EGG_GROUP_NO_EGGS_DISCOVERED' in i['eggs']
        return no_eggs and s not in parent and s not in children or i['catch'] <= 3

    pool = {}   # generation -> species fit for the wild / trainers
    for s, sid in ids.items():
        g = generation(sid)
        if g >= 4 and s in info and not legendary(s):
            pool.setdefault(g, []).append(s)
    for g in pool:
        pool[g].sort(key=lambda s: ids[s])

    def pick(orig, g, salt, avoid, low=False):
        if orig not in info:
            return None
        ot = info[orig]['types']
        st, final = stage(orig), orig not in children
        best, best_score = [], 0
        for c in pool.get(g, []):
            if c in avoid or (low and c[len('SPECIES_'):] in PSEUDO):
                continue
            ct = info[c]['types']
            score = 4 if ct[0] == ot[0] else 3 if ot[0] in ct else 2 if ot[1] in ct else 0
            if score == 0:
                continue
            score = score * 10 + (5 if stage(c) == st else 0) + (2 if (c not in children) == final else 0)
            if score > best_score:
                best, best_score = [c], score
            elif score == best_score:
                best.append(c)
        if not best or best_score % 10 < 5:   # (same stage or nothing)
            return None
        return best[zlib.crc32(salt.encode()) % len(best)]

    # ---- wild
    w = json.loads(read('src/data/wild_encounters.json'))
    wild_rows = []
    for grp in w['wild_encounter_groups']:
        if grp['label'] != 'gWildMonHeaders':
            continue
        for e in grp['encounters']:
            m = e.get('map', '')
            if not m or m.startswith('MAP_JOHTO') or m.startswith('MAP_KANTO'):
                continue
            for field, slots in WILD_SLOTS.items():
                if field not in e:
                    continue
                mons = e[field]['mons']
                used = {x['species'] for x in mons}
                label = '%s_%s' % (e['base_label'][1:], FIELD_INFO[field])
                for g in sorted(slots):
                    slot = slots[g]
                    if slot >= len(mons):
                        continue
                    orig = mons[slot]['species']
                    if generation(ids.get(orig, 0)) >= 4:
                        continue
                    c = pick(orig, g, '%s:%d:%d' % (label, slot, g), used, mons[slot]['min_level'] < 30)
                    if g == 9 and field == 'land_mons' and e['base_label'] in PARADOX:
                        c = 'SPECIES_' + PARADOX[e['base_label']]
                    if c:
                        used.add(c)
                        wild_rows.append((label, len(mons), slot, g, c))

    # ---- trainers
    trainers = read('src/data/trainers.h')
    parties = read('src/data/trainer_parties.h')
    ids_tr = {m.group(1): int(m.group(2)) for m in
              re.finditer(r'#define\s+(TRAINER_\w+)\s+(\d+)\s*$', read('include/constants/opponents.h'), re.M)}
    party_species = {m.group(1): re.findall(r'\.species\s*=\s*(SPECIES_\w+)', m.group(2)) for m in
                     re.finditer(r'static const struct \w+ (sParty_\w+)\[\]\s*=\s*\{(.*?)\n\};', parties, re.S)}
    tr_rows = []
    for m in re.finditer(r'\[(TRAINER_\w+)\]\s*=\s*\{(.*?)\n    \},', trainers, re.S):
        tid = ids_tr.get(m.group(1))
        if tid is None or tid == 0:
            continue
        body = m.group(2)
        cls = re.search(r'\.trainerClass\s*=\s*(\w+)', body)
        pm = re.search(r'\.party\s*=\s*\w+\((sParty_\w+)\)', body)
        if not pm or pm.group(1) not in party_species:
            continue
        team = party_species[pm.group(1)]
        boss = cls and cls.group(1) in BOSS_CLASSES
        used = set(team)
        for j, orig in enumerate(team):
            if boss and j == len(team) - 1:
                continue
            if generation(ids.get(orig, 0)) >= 4:
                continue
            h = zlib.crc32(('%d:%d' % (tid, j)).encode())
            if h % 100 >= 55:
                continue
            g = 4 + (h // 100) % 6
            c = pick(orig, g, 'T%d:%d:%d' % (tid, j, g), used)
            if c:
                used.add(c)
                tr_rows.append((tid, m.group(1), j, g, c))

    out = ['// Generated by tools/moemon_gen59/gen_hoenn_modern.py. Do not edit.',
           '// HOENN by GENERATION: a slot / team member takes its newer POKéMON from its generation on (mp_region.c).', '']
    for lbl in sorted({r[0] for r in wild_rows}):
        out.append('extern const struct WildPokemonInfo g%s;' % lbl)
    out += ['', 'struct HoennModernWild { const struct WildPokemonInfo *info; u8 count, slot, gen; u16 species; };',
            'static const struct HoennModernWild sHoennModernWild[] =', '{']
    for lbl, n, slot, g, c in wild_rows:
        out.append('    {&g%s, %d, %d, %d, %s},' % (lbl, n, slot, g, c))
    out += ['};', '', 'struct HoennModernMon { u16 trainerId; u8 slot, gen; u16 species; };',
            'static const struct HoennModernMon sHoennModernTrainers[] =', '{']
    for tid, name, j, g, c in sorted(tr_rows):
        out.append('    {%s, %d, %d, %s},' % (name, j, g, c))
    out += ['};', '']
    open(OUT, 'w', encoding='utf-8').write('\n'.join(out))
    from collections import Counter
    print('hoenn modern: %d wild slots, %d trainer POKéMON; by generation wild %s trainers %s' % (
        len(wild_rows), len(tr_rows), dict(sorted(Counter(r[3] for r in wild_rows).items())),
        dict(sorted(Counter(r[3] for r in tr_rows).items()))))


if __name__ == '__main__':
    main()
