#!/usr/bin/env python3
"""KANTO's and JOHTO's trainers on the original games' teams (audit case 57): FireRed's for KANTO
(pokefirered src/data/trainers.h + trainer_parties.h), Crystal's for JOHTO (pokecrystal data/trainers/parties.asm).

    python3 -I tools/quetzal_port/gen_original_teams.py POKECRYSTAL_DIR     (writes src/data/qz_original_teams.h)

Pairing:
- KANTO's maps are FireRed's: a trainer object here is FireRed's trainer standing on the same cell of the same map;
  a trainer fought only from a script (the ELITE FOUR, GIOVANNI, grunts...) is FireRed's of the same name fought in the
  same map's scripts, in order.
- JOHTO: a trainer is Crystal's of the same name fought on the same map (`trainer` / `loadtrainer`), in order;
  SILVER is Crystal's RIVAL with TOTODILE (mp_region.c swaps the line for the player's starter).
- Not paired (the rival BLUE, already FireRed's; Quetzal's own trainers) keep Quetzal's team.
mp_region.c TrainerDataFor gives the paired trainers these teams unless the GENERATION's modern teams are on (5+).
FireRed's VS SEEKER rematch tiers (sFrRematches) are written too: the trainer's FireRed _2.._4 parties.
"""
import glob
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fix_move_tutors import fetch   # noqa: E402  (pokefirered, cached)

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
OUT = os.path.join(REPO, 'src/data/qz_original_teams.h')
SKIP_NAMES = {'BLUE', 'MAXIE', 'ARCHIE', 'JESSIE&JAMES'}
GLOBAL_SKIP = SKIP_NAMES | {'RED', 'GRUNT', 'GIOVANNI', 'SILVER'}
CR_SPECIES = {'MR__MIME': 'MR_MIME', 'FARFETCH_D': 'FARFETCHD'}
CR_MOVES = {'PSYCHIC_M': 'PSYCHIC', 'HIGH_JUMP_KICK': 'HI_JUMP_KICK', 'BUBBLEBEAM': 'BUBBLE_BEAM',
            'CONVERSION2': 'CONVERSION_2', 'DOUBLESLAP': 'DOUBLE_SLAP', 'POISONPOWDER': 'POISON_POWDER',
            'SELFDESTRUCT': 'SELF_DESTRUCT', 'SOFTBOILED': 'SOFT_BOILED', 'SOLARBEAM': 'SOLAR_BEAM',
            'SONICBOOM': 'SONIC_BOOM', 'THUNDERPUNCH': 'THUNDER_PUNCH', 'THUNDERSHOCK': 'THUNDER_SHOCK',
            'ANCIENTPOWER': 'ANCIENT_POWER', 'DRAGONBREATH': 'DRAGON_BREATH', 'DYNAMICPUNCH': 'DYNAMIC_PUNCH',
            'EXTREMESPEED': 'EXTREME_SPEED', 'FAINT_ATTACK': 'FAINT_ATTACK', 'SMOKESCREEN': 'SMOKESCREEN',
            'SAND_ATTACK': 'SAND_ATTACK', 'VICEGRIP': 'VICE_GRIP', 'DOUBLE_EDGE': 'DOUBLE_EDGE'}
CR_ITEMS = {'BERRY': 'ORAN_BERRY', 'GOLD_BERRY': 'SITRUS_BERRY', 'MIRACLEBERRY': 'LUM_BERRY',
            'PRZCUREBERRY': 'CHERI_BERRY', 'PSNCUREBERRY': 'PECHA_BERRY', 'MINT_BERRY': 'CHESTO_BERRY',
            'ICE_BERRY': 'RAWST_BERRY', 'BURNT_BERRY': 'ASPEAR_BERRY', 'BITTER_BERRY': 'PERSIM_BERRY',
            'MYSTERYBERRY': 'LEPPA_BERRY', 'PINK_BOW': 'SILK_SCARF', 'POLKADOT_BOW': 'SILK_SCARF',
            'NEVERMELTICE': 'NEVER_MELT_ICE', 'BLACKGLASSES': 'BLACK_GLASSES', 'TWISTEDSPOON': 'TWISTED_SPOON',
            'SILVERPOWDER': 'SILVER_POWDER', 'BRIGHTPOWDER': 'BRIGHT_POWDER', 'DRAGON_FANG': 'DRAGON_FANG'}


def read(path):
    return open(path, encoding='utf-8').read()


def consts(path, prefix):
    return set(re.findall(r'#define (%s\w+)' % prefix, read(os.path.join(REPO, path))))


KAI = consts('include/constants/species.h', 'SPECIES_') | consts('include/constants/moves.h', 'MOVE_') | \
    consts('include/constants/items.h', 'ITEM_')
warn = set()


def kai(c, fallback):
    if c in KAI:
        return c
    warn.add(c)
    return fallback


def labels_trainers(text):
    """global label -> [(battle kind, TRAINER_...)] in its body"""
    out, cur = {}, None
    for line in text.split('\n'):
        m = re.match(r'^(\w+)::?\s*$', line)
        if m:
            cur = m.group(1)
            out.setdefault(cur, [])
            continue
        m = re.search(r'trainerbattle(\w*)\s+(?:(TRAINER_BATTLE_\w+),\s*)?(TRAINER_\w+)', line)
        if m and cur:
            out[cur].append(((m.group(2) or m.group(1)).lower(), m.group(3)))
    return out


def file_trainers(text):
    """[(kind, TRAINER_...)] in file order, rematches left out"""
    return [(m.group(2) or m.group(1)).lower() and m.group(3) for m in
            re.finditer(r'trainerbattle(\w*)\s+(?:(TRAINER_BATTLE_\w+),\s*)?(TRAINER_\w+)', text)
            if 'rematch' not in (m.group(2) or m.group(1)).lower()]


def qz_names():
    src = read(os.path.join(REPO, 'src/data/trainers_qz.h'))
    out = {}
    for m in re.finditer(r'\[(TRAINER_QZ_\d+)\] =\s*\{(.*?)\n    \},', src, re.S):
        n = re.search(r'trainerName = _\("([^"]*)"', m.group(2))
        out[m.group(1)] = n.group(1) if n else ''
    return out


# ------------------------------------------------------------------------------------------------ FireRed
def firered():
    trainers_h = fetch('src/data/trainers.h').replace('\r', '')
    parties_h = fetch('src/data/trainer_parties.h').replace('\r', '')
    fr = {}
    for m in re.finditer(r'\[(TRAINER_\w+)\] = \{(.*?)\n    \},', trainers_h, re.S):
        body = m.group(2)
        name = re.search(r'trainerName = _\("([^"]*)"', body)
        party = re.search(r'\.party = (\w+)\((\w+)\)', body)
        if party:
            fr[m.group(1)] = {'name': name.group(1) if name else '', 'macro': party.group(1), 'party': party.group(2)}
    parties = {}
    for m in re.finditer(r'static const struct (TrainerMon\w+) (sParty_\w+)\[\] = \{((?:(?!\nstatic ).)*?)\};', parties_h, re.S):
        parties[m.group(2)] = (m.group(1), m.group(3))
    return fr, parties


FLAGS = {'TrainerMonNoItemDefaultMoves': '0', 'TrainerMonNoItemCustomMoves': 'F_TRAINER_PARTY_CUSTOM_MOVESET',
         'TrainerMonItemDefaultMoves': 'F_TRAINER_PARTY_HELD_ITEM',
         'TrainerMonItemCustomMoves': 'F_TRAINER_PARTY_CUSTOM_MOVESET | F_TRAINER_PARTY_HELD_ITEM'}


def fr_party_c(struct, body):
    """FireRed's party body with Kai's constant names (unknown ones dropped to NONE)"""
    body = re.sub(r'\b(SPECIES_\w+)', lambda m: kai(m.group(1), 'SPECIES_NONE'), body)
    body = re.sub(r'\b(MOVE_\w+)', lambda m: kai({'MOVE_HIGH_JUMP_KICK': 'MOVE_HI_JUMP_KICK'}.get(m.group(1), m.group(1)),
                                                 'MOVE_NONE'), body)
    body = re.sub(r'\b(ITEM_\w+)', lambda m: kai(m.group(1), 'ITEM_NONE'), body)
    return body, body.count('.species')


def kanto_pairs(names):
    tinc = fetch('data/scripts/trainers.inc')
    fr_lt = labels_trainers(tinc)
    fr, _ = firered()
    pairs = {}
    for d in sorted(glob.glob(os.path.join(REPO, 'data/maps/Kanto_*'))):
        mp = os.path.basename(d)
        frmap = mp[len('Kanto_'):]
        try:
            ours = json.load(open(os.path.join(d, 'map.json')))
            ours_src = read(os.path.join(d, 'scripts.inc'))
            fj = json.loads(fetch('data/maps/%s/map.json' % frmap))
        except Exception:
            continue
        try:
            fr_src = fetch('data/maps/%s/scripts.inc' % frmap)
        except Exception:
            fr_src = ''
        lt = labels_trainers(ours_src)
        flt = dict(fr_lt)
        flt.update(labels_trainers(fr_src))
        on_objects = set()
        for o in ours['object_events']:
            tr = [t for k, t in lt.get(o['script'], []) if 'rematch' not in k]
            if not tr:
                continue
            on_objects.add(tr[0])
            for fo in fj['object_events']:
                if fo['x'] == o['x'] and fo['y'] == o['y']:
                    ft = [t for k, t in flt.get(fo.get('script', ''), []) if 'rematch' not in k]
                    if ft and ft[0] in fr:
                        pairs[tr[0]] = (mp, ft[0])
        # fought from scripts only: the same name in FireRed's map scripts, in order
        fr_script = [t for t in file_trainers(fr_src) if t in fr]
        used = set(v[1] for v in pairs.values())
        for t in file_trainers(ours_src):
            if t in pairs or t in on_objects or not t.startswith('TRAINER_QZ_') or names.get(t, '') in SKIP_NAMES:
                continue
            for ft in fr_script:
                if ft not in used and fr[ft]['name'].upper() == names.get(t, '').upper():
                    pairs[t] = (mp, ft)
                    used.add(ft)
                    break
    return pairs


# ------------------------------------------------------------------------------------------------ Crystal
def crystal(pc):
    tconst = read(os.path.join(pc, 'constants/trainer_constants.asm'))
    classes, class_ids, cur = [], {}, None
    for m in re.finditer(r'^\s+(trainerclass|const)\s+(\w+)', tconst, re.M):
        if m.group(1) == 'trainerclass':
            cur = m.group(2)
            classes.append(cur)
            class_ids[cur] = []
        elif cur:
            class_ids[cur].append(m.group(2))
    src = read(os.path.join(pc, 'data/trainers/parties.asm'))
    groups = re.split(r'^(\w+)Group:\s*$', src, flags=re.M)
    entries = {}
    for k in range(1, len(groups), 2):
        es = re.split(r'^\s*; [^\n]*\(\d+\)\s*$', groups[k + 1], flags=re.M)
        entries[groups[k]] = [e for e in es if 'db "' in e]
    gnames = re.findall(r'^\s+dw (\w+)Group', read(os.path.join(pc, 'data/trainers/party_pointers.asm')), re.M)
    class_group = dict(zip(classes[1:], gnames))

    def party(cls, tid):
        e = entries[class_group[cls]][class_ids[cls].index(tid)]
        head = re.search(r'db "([^@"]*)@?"?,\s*(\w+)', e)
        kind = head.group(2)
        mons = []
        for lvl, rest in re.findall(r'^\s+db\s+(-?\d+),\s*(.+)$', e, re.M):
            if int(lvl) < 0:
                break
            parts = [p.split(';')[0].strip() for p in rest.split(',')]
            item, moves = None, []
            if kind == 'TRAINERTYPE_ITEM':
                item = parts[1]
            elif kind == 'TRAINERTYPE_MOVES':
                moves = parts[1:5]
            elif kind == 'TRAINERTYPE_ITEM_MOVES':
                item, moves = parts[1], parts[2:6]
            mons.append((int(lvl), parts[0], item, moves))
        return head.group(1).replace('#', 'POKé'), kind, mons
    return party


def class_ivs(pc):
    """Crystal's DVs per class (data/trainers/dvs.asm) as the game's 0-255 trainer IV: the four DVs' mean, doubled
    to Gen 3's 0-31 scale"""
    out = {}
    for a, b, c, d, cls in re.findall(r'^\s+dn\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\s*;\s*(\w+)',
                                     read(os.path.join(pc, 'data/trainers/dvs.asm')), re.M):
        mean = (int(a) + int(b) + int(c) + int(d)) / 4.0
        out[cls] = min(255, int(round(mean * 2 * 255 / 31)))
    return out


def cr_party_c(kind, mons, iv=0):
    struct = {'TRAINERTYPE_NORMAL': 'TrainerMonNoItemDefaultMoves', 'TRAINERTYPE_MOVES': 'TrainerMonNoItemCustomMoves',
              'TRAINERTYPE_ITEM': 'TrainerMonItemDefaultMoves',
              'TRAINERTYPE_ITEM_MOVES': 'TrainerMonItemCustomMoves'}[kind]
    rows = []
    for lvl, sp, item, moves in mons:
        r = '.iv = %d, .lvl = %d, .species = %s' % (iv, lvl, kai('SPECIES_' + CR_SPECIES.get(sp, sp), 'SPECIES_NONE'))
        if item:
            r += ', .heldItem = %s' % ('ITEM_NONE' if item == 'NO_ITEM' else
                                      kai('ITEM_' + CR_ITEMS.get(item, item), 'ITEM_NONE'))
        if moves:
            r += ', .moves = {%s}' % ', '.join('MOVE_NONE' if mv == 'NO_MOVE' else
                                               kai('MOVE_' + CR_MOVES.get(mv, mv), 'MOVE_NONE') for mv in moves)
        rows.append('    {%s},' % r)
    return struct, '\n'.join(rows), len(mons)


ALIAS = {}
# Quetzal's maps that are several of Crystal's, or named otherwise
MAP_ALIAS = {'Johto_OlivineLighthouse': ['OlivineLighthouse%dF' % n for n in range(1, 7)],
             'Johto_BlackthornGym1F': ['BlackthornGym1F', 'BlackthornGym2F'], 'Johto_Route26South': ['Route26'],
             'Johto_VictoryRoad3F': ['VictoryRoad'], 'Johto_DragonsDenB1F': ['DragonsDenB1F', 'DragonsDen1F']}


def norm(n):
    return n.upper().replace(' ', '')
# TEAM ROCKET's executives (HGSS's names in Quetzal; Crystal's are all "EXECUTIVE"): which one each is
EXECUTIVES = {('Johto_RadioTower5F', 'ARIANA'): 'EXECUTIVEF_1', ('Johto_RadioTower5F', 'PETREL'): 'EXECUTIVEM_3',
              ('Johto_RadioTower5F', 'ARCHER'): 'EXECUTIVEM_1', ('Johto_RadioTower4F', 'PROTON'): 'EXECUTIVEM_2',
              ('Johto_TeamRocketBaseB2F', 'ARIANA'): 'EXECUTIVEF_2', ('Johto_TeamRocketBaseB3F', 'PETREL'): 'EXECUTIVEM_4'}


def johto_pairs(pc, names, party):
    pairs, everywhere = {}, []
    for d in sorted(glob.glob(os.path.join(REPO, 'data/maps/Johto_*'))):
        mp = os.path.basename(d)
        crfs = [os.path.join(pc, 'maps/%s.asm' % m) for m in MAP_ALIAS.get(mp, [mp[len('Johto_'):]])]
        crfs = [f for f in crfs if os.path.exists(f)]
        if not crfs:
            continue
        cr_src = '\n'.join(read(f) for f in crfs)
        cr = []   # (class, id, name) in file order
        for m in re.finditer(r'^\s+(?:trainer|loadtrainer)\s+(\w+),\s*(\w+)', cr_src, re.M):
            cls, tid = m.group(1), m.group(2)
            try:
                nm = party(cls, tid)[0]
            except (KeyError, ValueError, IndexError):
                continue
            if cls.startswith('RIVAL'):
                if not tid.endswith('TOTODILE'):
                    continue
                nm = 'SILVER'
            if (cls, tid, nm) not in cr:
                cr.append((cls, tid, nm))
        used = set()
        for t in file_trainers(read(os.path.join(d, 'scripts.inc'))):
            if not t.startswith('TRAINER_QZ_') or t in pairs:
                continue
            nm = norm(names.get(t, ''))
            same = [c for c in cr if norm(c[2]) == nm]
            fresh = [c for c in same if c not in used]
            if fresh or same:   # (a second copy of a team, Quetzal's QzUseAltGymLeaderTeam: the same one)
                c = (fresh or same)[0]
                pairs[t] = (mp, c[0], c[1])
                used.add(c)
        everywhere.extend(cr)
    for d in sorted(glob.glob(os.path.join(REPO, 'data/maps/Johto_*'))):
        mp = os.path.basename(d)
        for t in file_trainers(read(os.path.join(d, 'scripts.inc'))):
            tid = EXECUTIVES.get((mp, names.get(t, '').upper()))
            if tid and t not in pairs:
                pairs[t] = (mp, tid.rsplit('_', 1)[0], tid)
    # Quetzal's maps that join Crystal's (the LIGHTHOUSE's floors...): a name only one Crystal trainer has
    by_name = {}
    for c in everywhere:
        by_name.setdefault(norm(c[2]), set()).add(c)
    for d in sorted(glob.glob(os.path.join(REPO, 'data/maps/Johto_*'))):
        mp = os.path.basename(d)
        for t in file_trainers(read(os.path.join(d, 'scripts.inc'))):
            nm = norm(names.get(t, ''))
            if t.startswith('TRAINER_QZ_') and t not in pairs and len(by_name.get(nm, ())) == 1 and nm not in GLOBAL_SKIP:
                c = next(iter(by_name[nm]))
                pairs[t] = (mp, c[0], c[1])
    return pairs


def main():
    pc = sys.argv[1] if len(sys.argv) > 1 else os.environ.get('POKECRYSTAL_DIR', '/tmp/claude-0/frlg/pokecrystal')
    names = qz_names()
    fr, fr_parties = firered()
    kp = kanto_pairs(names)
    party = crystal(pc)
    jp = johto_pairs(pc, names, party)
    ivs = class_ivs(pc)
    out = ['// Generated by tools/quetzal_port/gen_original_teams.py from pret/pokefirered and pret/pokecrystal. Do not',
           '// edit. KANTO\'s trainers on FireRed\'s teams, JOHTO\'s on Crystal\'s (mp_region.c TrainerDataFor; audit case 57).',
           '']
    table = []
    done_fr = {}
    for qz, (mp, ft) in sorted(kp.items(), key=lambda kv: int(kv[0].split('_')[-1])):
        pname = fr[ft]['party']
        if pname not in done_fr:
            struct, body = fr_parties[pname]
            body, n = fr_party_c(struct, body)
            cname = 'sOrig_Fr' + pname[len('sParty_'):]
            out.append('static const struct %s %s[] = {%s\n};' % (struct, cname, body))
            done_fr[pname] = (struct, cname, n)
        struct, cname, n = done_fr[pname]
        table.append((qz, FLAGS[struct], n, cname, '%s: FireRed %s' % (mp, ft)))
    done_cr = {}
    for qz, (mp, cls, tid) in sorted(jp.items(), key=lambda kv: int(kv[0].split('_')[-1])):
        if (cls, tid) not in done_cr:
            nm, kind, mons = party(cls, tid)
            struct, body, n = cr_party_c(kind, mons, ivs.get(cls, 0))
            cname = 'sOrig_Cr_%s_%s' % (cls, tid)
            out.append('static const struct %s %s[] = {\n%s\n};' % (struct, cname, body))
            done_cr[(cls, tid)] = (struct, cname, n)
        struct, cname, n = done_cr[(cls, tid)]
        table.append((qz, FLAGS[struct], n, cname, '%s: Crystal %s %s' % (mp, cls, tid)))
    table.sort(key=lambda r: int(r[0].split('_')[-1]))
    out.append('')
    out.append('static const struct QzOriginalTeam { u16 trainerNum; u8 partyFlags; u8 partySize; const void *party; } '
               'sQzOriginalTeams[] = {   // (by trainer number)')
    for qz, flags, n, cname, note in table:
        out.append('    {%s, %s, %d, %s},   // %s' % (qz, flags, n, cname, note))
    out.append('};')
    open(OUT, 'w').write('\n'.join(out) + '\n')
    print('KANTO %d, JOHTO %d paired' % (len(kp), len(jp)))
    if warn:
        print('unknown, dropped:', ' '.join(sorted(warn)))


if __name__ == '__main__':
    main()
