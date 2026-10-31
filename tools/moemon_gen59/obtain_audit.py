#!/usr/bin/env python3
"""Which POKéMON can be obtained at each GENERATION (3-9), and how.

    python3 tools/moemon_gen59/obtain_audit.py [--gen N] [--why SPECIES] [--out docs/audit_obtain.txt]

Reads the game's own tables (no ROM): wild tables of HOENN, JOHTO and KANTO as mp_region.c picks them by GENERATION
(HOENN's base tables + hoenn_modern.h's slots; JOHTO / KANTO's Gen 2 tables at GEN 3, the Gen 4 cap's at 4, Quetzal's
own from 5), HEADBUTT trees, statics (qz_static_encounters.h) and the scripts' gifts / wild battles / eggs, starters,
in-game trades, the Bug-Catching Contest, MoeGatcha's fixed prizes and the extra sources listed in EXTRA below;
then evolutions (any method; items are the item audit's business) and breeding (an egg hatches the family's first
stage the GENERATION allows; incense babies need their incense).

The rules it models (mp_region.c): a region shows POKéMON up to MP_Gen_Cap (HOENN: the GENERATION; JOHTO / KANTO:
Gen 2 at GEN 3, else the GENERATION); scripted wild battles and HEADBUTT stand in an earlier form; and a POKéMON past
the GENERATION is SEALED (it can't battle), so it doesn't count. The NATIONAL DEX at GEN g is 1..LAST[g].

Writes the report (missing species per GENERATION, and each one's sources) and exits 1 if anything is missing.
"""
import json
import os
import re
import sys
from collections import defaultdict

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
LAST = {1: 151, 2: 251, 3: 386, 4: 493, 5: 649, 6: 721, 7: 809, 8: 905, 9: 1025}
HOENN, JOHTO, KANTO = 0, 1, 2
RNAME = ['HOENN', 'JOHTO', 'KANTO']


def rd(p):
    return open(os.path.join(REPO, p), encoding='utf-8', errors='replace').read()


# ------------------------------------------------------------------------------------------------ species and numbers
SPECIES = {}   # NAME -> id
for m in re.finditer(r'#define SPECIES_(\w+)\s+(\d+)\b', rd('include/constants/species.h')):
    SPECIES.setdefault(m.group(1), int(m.group(2)))
NAT = {}       # NAME -> national number (the first enum of pokedex.h counts from NATIONAL_DEX_NONE = 0)
for i, n in enumerate(re.findall(r'^\s*NATIONAL_DEX_(\w+),', rd('include/constants/pokedex.h').split('};')[0], re.M)):
    if n in SPECIES:
        NAT[n] = i
BY_NAT = {}
for n, v in NAT.items():
    BY_NAT.setdefault(v, n)


def gen_of(name):
    n = NAT.get(name, 0)
    for g in range(1, 10):
        if n and n <= LAST[g]:
            return g
    return 0


def cap(region, g):
    return 2 if region != HOENN and g <= 3 else g


def species_of(token):
    """SPECIES_X or a number -> NAME (None for vars)"""
    token = token.strip()
    if token.startswith('SPECIES_'):
        return token[8:] if token[8:] in SPECIES else None
    try:
        v = int(token, 0)
    except ValueError:
        return None
    for n, i in SPECIES.items():
        if i == v:
            return n
    return None


# ------------------------------------------------------------------------------------------------ evolutions
EVOS = defaultdict(list)   # NAME -> [(method, param, target)]
for f in ('evolution.h', 'evolution_gen4.h', 'evolution_gen59.h'):
    text = rd('src/data/pokemon/' + f)
    parts = re.split(r'\[SPECIES_(\w+)\]\s*=', text)
    for i in range(1, len(parts), 2):
        for m in re.finditer(r'\{(EVO_\w+),\s*([^,]+),\s*SPECIES_(\w+)\}', parts[i + 1].split('[SPECIES_')[0]):
            EVOS[parts[i]].append((m.group(1), m.group(2).strip(), m.group(3)))
PREV = defaultdict(set)
for a, lst in EVOS.items():
    for _, _, b in lst:
        PREV[b].add(a)
INCENSE = {}   # baby -> (item, next stage)
for m in re.finditer(r'\{SPECIES_(\w+), (ITEM_\w+), SPECIES_(\w+)\}', rd('src/daycare.c')):
    INCENSE[m.group(1)] = (m.group(2), m.group(3))
INCENSE.setdefault('WYNAUT', ('ITEM_LAX_INCENSE', 'WOBBUFFET'))
INCENSE.setdefault('AZURILL', ('ITEM_SEA_INCENSE', 'MARILL'))
NO_EGGS = set()   # (legendaries and the like: Undiscovered egg group)
for f in ('species_info.h', 'species_info_gen4.h', 'species_info_gen59.h'):
    for m in re.finditer(r'\[SPECIES_(\w+)\]\s*=\s*\{(.*?)\n\s*\},', rd('src/data/pokemon/' + f), re.S):
        if 'EGG_GROUP_UNDISCOVERED' in m.group(2) or 'EGG_GROUP_NO_EGGS' in m.group(2):
            NO_EGGS.add(m.group(1))
NO_EGGS.discard('DITTO')


def stand_in(name, c):
    """MP_Gen_StandIn: the nearest earlier form within cap c"""
    s = name
    for _ in range(4):
        if gen_of(s) <= c:
            return s
        prev = sorted(PREV.get(s, ()))
        if not prev:
            return name
        s = prev[0]
    return s if gen_of(s) <= c else name


# ------------------------------------------------------------------------------------------------ the sources
# sources[(region, how)] -> set of species, per GENERATION: built by base(g)
MAP_REGION = {}   # map folder name / MAP_ constant -> region
groups = json.load(open(os.path.join(REPO, 'data/maps/map_groups.json')))
for gi, gname in enumerate(groups['group_order']):
    for name in groups[gname]:
        r = HOENN if gi < 34 else (JOHTO if gi < 36 else KANTO)
        MAP_REGION[name] = r
        try:
            MAP_REGION[json.load(open(os.path.join(REPO, 'data/maps', name, 'map.json')))['id']] = r
        except (OSError, KeyError):
            pass

WILD = []   # (region, label, field, [species])
wj = json.load(open(os.path.join(REPO, 'src/data/wild_encounters.json')))
for grp in wj['wild_encounter_groups']:
    if grp['label'] != 'gWildMonHeaders':
        continue   # (the BATTLE PYRAMID / PIKE: nothing to keep)
    for e in grp['encounters']:
        lab = e['base_label']
        m = re.match(r'gQuetzal_(\d+)_', lab)
        region = HOENN if not m else (JOHTO if int(m.group(1)) < 36 else KANTO)
        for field in ('land_mons', 'water_mons', 'rock_smash_mons', 'fishing_mons', 'hidden_mons'):
            if field in e:
                sp = [species_of(x['species']) for x in e[field]['mons']]
                WILD.append((region, lab, field, sp))

FIELD_SUFFIX = {'land_mons': 'LandMons', 'water_mons': 'WaterMons', 'rock_smash_mons': 'RockSmashMons',
                'fishing_mons': 'FishingMons', 'hidden_mons': 'HiddenMons'}
GEN2 = {}     # label_Field -> [species]
for m in re.finditer(r'static const struct WildPokemon sQz2_(\w+)\[\] = \{(.*?)\};', rd('src/data/wild_qz_gen2.h')):
    GEN2[m.group(1)] = [species_of(x) for x in re.findall(r'SPECIES_\w+', m.group(2))]
MODERN = {}   # label_Field -> [species per slot]
for m in re.finditer(r'static const u16 sQzM_(\w+)\[\] = \{(.*?)\};', rd('src/data/wild_qz_modern.h')):
    MODERN[m.group(1)] = [species_of(x) for x in re.findall(r'SPECIES_\w+', m.group(2))]
HMOD = defaultdict(list)   # label_Field -> [(slot, gen, species)]
for m in re.finditer(r'\{&(\w+)Info, (\d+), (\d+), (\d+), SPECIES_(\w+)\}', rd('src/data/hoenn_modern.h')):
    HMOD[m.group(1)].append((int(m.group(3)), int(m.group(4)), m.group(5)))

HEADBUTT = []   # (region, [species])
for m in re.finditer(r'\{(MAP_\w+), \d+, \{(.*?)\}\},', rd('src/data/wild_headbutt_qz.h')):
    HEADBUTT.append((MAP_REGION.get(m.group(1), JOHTO), [species_of(x) for x in re.findall(r'SPECIES_\w+', m.group(2))]))

STATICS = []   # (region, species)
for m in re.finditer(r'\{(\d), \d+, \d+, SPECIES_(\w+),.*?, (\d+)\},', rd('src/data/qz_static_encounters.h')):
    STATICS.append((int(m.group(1)), m.group(2), int(m.group(3))))

SCRIPTED = []   # (region, species, how) from the scripts
for root in ('data/maps', 'data/scripts'):
    for dp, _, fs in os.walk(os.path.join(REPO, root)):
        for f in fs:
            if not f.endswith('.inc'):
                continue
            p = os.path.join(dp, f)
            text = open(p, encoding='utf-8', errors='replace').read()
            folder = os.path.basename(dp)
            if folder in MAP_REGION:
                region = MAP_REGION[folder]
            elif 'johto' in f.lower():
                region = JOHTO
            elif 'kanto' in f.lower():
                region = KANTO
            else:
                region = HOENN
            for m in re.finditer(r'^\s*(givemon|setwildbattle|giveegg|qz_givemon_ex|qz_setstaticwildmon|qz_giveoddegg)\s+(\w+)',
                                 text, re.M):
                how = 'wild' if 'wild' in m.group(1) else ('egg' if 'egg' in m.group(1) else 'gift')
                arg = m.group(2)
                if arg.startswith('VAR_'):
                    for v in re.findall(r'setvar %s, (\w+)' % arg, text):
                        s = species_of(v)
                        if s:
                            SCRIPTED.append((region, s, how + ' (' + f + ')'))
                else:
                    s = species_of(arg)
                    if s:
                        SCRIPTED.append((region, s, how))

STARTERS = []   # (region, species)
for s in re.findall(r'SPECIES_(\w+)', re.search(r'sStarterMon\[STARTER_MON_COUNT\] =\s*\{(.*?)\}', rd('src/starter_choose.c'), re.S).group(1)):
    STARTERS.append((HOENN, s))
qzs = re.search(r'static const u16 sQzStarters\[\] = \{(.*?)\};', rd('src/qz_specials.c'), re.S).group(1)
for s in re.findall(r'SPECIES_(\w+)', qzs):
    STARTERS.append((JOHTO, s))
    STARTERS.append((KANTO, s))

TRADES = []
for m in re.finditer(r'\.species = SPECIES_(\w+)', rd('src/data/trade.h')):
    TRADES.append((HOENN, m.group(1)))
for m in re.finditer(r'\.species = SPECIES_(\w+)', rd('src/data/qz_ingame_trades.h')):
    TRADES.append((JOHTO, m.group(1)))   # (JOHTO's and KANTO's: the same cap)

CONTEST = [(JOHTO, s) for s in re.findall(r'SPECIES_(\w+)', re.search(r'sBugs\[\] = \{(.*?)\};', rd('src/qz_bugcontest.c'), re.S).group(1))]

GATCHA = []   # region-free: the fixed prizes (EV_RANDOM* aside)
for m in re.finditer(r'\[EV_\w+\]\s*=\s*\{[^,]+, SPECIES_(\w+),', rd('src/mp/mp_gatcha.c')):
    GATCHA.append(m.group(1))

# Sources the parse above can't see (specials that give a POKéMON). Keep in step with the code.
EXTRA = [
    # (region, species, how)
]


def obtainable(g):
    """species -> set of how-strings (only species with gen <= g)"""
    got = defaultdict(set)

    def add(region, s, how):
        if s and gen_of(s) and gen_of(s) <= cap(region, g) and gen_of(s) <= g:
            got[s].add('%s %s' % (RNAME[region], how) if region is not None else how)

    for region, lab, field, sp in WILD:
        key = lab + '_' + FIELD_SUFFIX[field]
        if region == HOENN:
            sp = list(sp)
            for slot, sg, s in HMOD.get(key, ()):
                if sg <= g and gen_of(s) <= g and slot < len(sp):
                    sp[slot] = s
        elif g <= 3:
            sp = GEN2.get(key, sp)
        elif g >= 5 and key in MODERN:
            mod = MODERN[key]
            sp = [mod[i] if i < len(mod) and mod[i] and gen_of(mod[i]) <= g else sp[i] for i in range(len(sp))]
        for s in sp:
            add(region, s, 'wild ' + field.split('_')[0])
    for region, sp in HEADBUTT:
        for s in sp:
            add(region, stand_in(s, cap(region, g)), 'headbutt')
    for region, s, fg in STATICS:
        if fg <= cap(region, g):
            add(region, s, 'static')
    for region, s, how in SCRIPTED:
        add(region, stand_in(s, cap(region, g)) if how.startswith('wild') else s, how)
    for region, s in STARTERS:
        add(region, s, 'starter')
    for region, s in TRADES:
        add(region, s, 'in-game trade')
    for region, s in CONTEST:
        add(region, s, 'bug contest')
    for s in GATCHA:
        add(HOENN, s, 'MoeGatcha prize')
    for region, s, how in EXTRA:
        add(region, s, how)

    # evolutions and breeding until nothing changes (evolving happens in any region that shows the target: HOENN's
    # cap is the GENERATION)
    changed = True
    while changed:
        changed = False
        for s in list(got):
            for meth, param, t in EVOS.get(s, ()):
                if gen_of(t) <= g and t not in got:
                    got[t].add('evolves from %s (%s %s)' % (s, meth, param))
                    changed = True
            if s in NO_EGGS:
                continue
            # the egg: the family's first stage within the GENERATION
            base = s
            while PREV.get(base):
                p = sorted(PREV[base])[0]
                if gen_of(p) > g:
                    break
                base = p
            if base == 'MANAPHY':
                base = 'PHIONE'   # (daycare.c)
            if base not in got:
                got[base].add('egg from %s' % s)
                changed = True
            if base in INCENSE:
                pass   # (the baby itself needs its incense: flagged by the item audit)
    return got


def main():
    gens = [int(sys.argv[sys.argv.index('--gen') + 1])] if '--gen' in sys.argv else list(range(3, 10))
    out = sys.argv[sys.argv.index('--out') + 1] if '--out' in sys.argv else None
    why = sys.argv[sys.argv.index('--why') + 1].upper() if '--why' in sys.argv else None
    lines, missing_any = [], False
    for g in gens:
        got = obtainable(g)
        if why:
            print('GEN %d %s: %s' % (g, why, '; '.join(sorted(got.get(why, ()))) or 'NOT OBTAINABLE'))
            continue
        want = [BY_NAT[n] for n in range(1, LAST[g] + 1) if n in BY_NAT]
        miss = [s for s in want if s not in got]
        missing_any |= bool(miss)
        lines.append('GEN %d: %d of %d obtainable, %d missing' % (g, len(want) - len(miss), len(want), len(miss)))
        for s in miss:
            lines.append('    #%04d %s' % (NAT[s], s))
    if why:
        return 0
    text = '\n'.join(lines) + '\n'
    if out:
        open(os.path.join(REPO, out), 'w').write(text)
    print(text, end='')
    return 1 if missing_any else 0


if __name__ == '__main__':
    sys.exit(main())
