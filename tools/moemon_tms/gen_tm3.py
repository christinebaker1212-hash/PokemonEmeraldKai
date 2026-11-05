#!/usr/bin/env python3
"""TM93 on: the TMs of Red / Blue (Gen 1) and Crystal (Gen 2) whose moves aren't on that number here (the user's rule,
2026-11-05: "a TM number keeps its Gen 3/4 move; a different move on the same number in Gen 1 or Gen 2 gets a new slot
after the last TM, past 100 if need be; a move already on that number gets none"). A move that is already a TM or HM
here at another number gets no slot either: its places give that TM (Crystal's TM35 SLEEP TALK is TM82, its TM08 ROCK
SMASH is HM06). In number order, Gen 1 before Gen 2 on the same number. Items 697 on, after the other regions' HMs
(TM/HM index 103 on); who learns them is gTm3Learnsets (bit k = TM(93 + k)).

    python3 -I tools/moemon_tms/gen_tm3.py POKEAPI_CSV_DIR POKERED_DIR POKECRYSTAL_DIR

Who learns each: Red / Blue's TM lists (species 1-151) and Crystal's (1-251), Emerald's move tutors (the game's own
tutor_learnsets.h), Platinum / HeartGold / SoulSilver's tutors and TMs (tools/moemon_gen4/gen4_data.json), and the
newer species' teachable moves (tools/moemon_gen59/species.json). Descriptions: PokeAPI's Gen 4 text of the move in
the game's style (tools/moemon_items/gen_desc.py), written without line breaks.
"""
import csv
import importlib.util
import json
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FIRST_ITEM = 697
FIRST_NUMBER = 93
FIRST_INDEX = 103   # (TM01-50 0-49, HM01-08 50-57, TM51-92 58-99, ROCK CLIMB / DEFOG / WHIRLPOOL 100-102)

# Red / Blue and Crystal's own spellings
ALIAS = {'PSYCHIC_M': 'PSYCHIC', 'HI_JUMP_KICK': 'HIGH_JUMP_KICK', 'SELFDESTRUCT': 'SELF_DESTRUCT',
         'DOUBLESLAP': 'DOUBLE_SLAP', 'DYNAMICPUNCH': 'DYNAMIC_PUNCH', 'DRAGONBREATH': 'DRAGON_BREATH',
         'SOLARBEAM': 'SOLAR_BEAM', 'THUNDERPUNCH': 'THUNDER_PUNCH', 'BUBBLEBEAM': 'BUBBLE_BEAM',
         'SOFTBOILED': 'SOFT_BOILED', 'FLASH_TM': 'FLASH', 'FAINT_ATTACK': 'FEINT_ATTACK', 'SONICBOOM': 'SONIC_BOOM',
         'POISONPOWDER': 'POISON_POWDER', 'THUNDERSHOCK': 'THUNDER_SHOCK', 'ANCIENTPOWER': 'ANCIENT_POWER',
         'EXTREMESPEED': 'EXTREME_SPEED', 'VICEGRIP': 'VICE_GRIP', 'SMOKESCREEN': 'SMOKESCREEN'}
STRONG = {'MEGA_KICK', 'HORN_DRILL', 'DOUBLE_EDGE', 'FISSURE', 'SELF_DESTRUCT', 'SKULL_BASH', 'SKY_ATTACK',
          'ZAP_CANNON', 'DYNAMIC_PUNCH', 'SUBMISSION', 'EGG_BOMB', 'TRI_ATTACK', 'BODY_SLAM', 'SOFT_BOILED'}
STATUS = {'WHIRLWIND', 'TELEPORT', 'MIMIC', 'BIDE', 'METRONOME', 'SWEET_SCENT', 'DEFENSE_CURL', 'DETECT', 'CURSE',
          'NIGHTMARE', 'RAGE'}
SPECIES_FILE = {'nidoranf': 'NIDORAN_F', 'nidoranm': 'NIDORAN_M', 'nidoran_f': 'NIDORAN_F', 'nidoran_m': 'NIDORAN_M',
                'farfetch_d': 'FARFETCHD', 'farfetchd': 'FARFETCHD', 'mr_mime': 'MR_MIME', 'mrmime': 'MR_MIME',
                'ho_oh': 'HO_OH', 'hooh': 'HO_OH'}


def n(m):
    return ALIAS.get(m, m)


def tm_list(path):
    return [n(m) for m in re.findall(r'^\s*add_tm (\w+)', open(path).read(), re.M)]


def write(path, text):
    open(os.path.join(REPO, path), 'w', encoding='utf-8').write(text)


def the_new_tms(red, crystal):
    """(move, generation, number) for each new slot, and (generation, number, move) for the moves reused"""
    ours = re.findall(r'F\((\w+)\)', open(os.path.join(REPO, 'include/constants/tms_hms.h')).read().split('FOREACH_HM')[0])
    hms = re.findall(r'F\((\w+)\)', open(os.path.join(REPO, 'include/constants/tms_hms.h')).read().split('FOREACH_HM')[1])
    tm2 = re.findall(r'F\((\w+)\)', open(os.path.join(REPO, 'include/constants/tms_hms_tm2.h')).read())
    have = {n(m) for m in ours + hms + tm2}
    new, reused = [], []
    for num in range(1, 51):
        for gen, lst in ((1, red), (2, crystal)):
            m = lst[num - 1]
            if m == n(ours[num - 1]):
                continue
            if m in have:
                reused.append((gen, num, m))
            elif m not in [x[0] for x in new]:
                new.append((m, gen, num))
    return new, reused


def base_stats_tms(d):
    out = {}
    for f in sorted(os.listdir(d)):
        if not f.endswith('.asm'):
            continue
        name = f[:-4]
        const = 'SPECIES_' + SPECIES_FILE.get(name, name.upper())
        text = open(os.path.join(d, f)).read()
        m = re.search(r'tmhm (.*?)\n\s*; end', text, re.S)
        if m:
            out[const] = {n(x) for x in re.findall(r'[A-Z_0-9]+', m.group(1))}
    return out


def emerald_tutors():
    out = {}
    text = open(os.path.join(REPO, 'src/data/pokemon/tutor_learnsets.h')).read()
    for m in re.finditer(r'\[(SPECIES_\w+)\]\s*=\s*\((.*?)\)\s*,', text, re.S):
        out[m.group(1)] = {x for x in re.findall(r'TUTOR\(MOVE_(\w+)\)', m.group(2))}
    return out


def descriptions(csv_dir, moves):
    spec = importlib.util.spec_from_file_location('gen_desc', os.path.join(REPO, 'tools/moemon_items/gen_desc.py'))
    gd = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(gd)
    ids = {r['id']: r['identifier'] for r in csv.DictReader(open(os.path.join(csv_dir, 'moves.csv'), encoding='utf-8'))}
    texts = {}
    for r in csv.DictReader(open(os.path.join(csv_dir, 'move_flavor_text.csv'), encoding='utf-8')):
        if r['language_id'] == gd.ENGLISH and r['version_group_id'] in gd.VERSION_ORDER:
            texts.setdefault(ids[r['move_id']], {})[r['version_group_id']] = r['flavor_text']
    chosen = {}
    vocab = set()
    for m in moves:
        t = gd.best_text(texts.get(m.lower().replace('_', '-'), {}))
        assert t, 'no Gen 4 text for ' + m
        chosen[m] = gd.flatten(t)
        gd.names_in(chosen[m], vocab)
    vocab -= {'I'}
    return {m: gd.game_style(t, vocab).replace('"', '\\"') for m, t in chosen.items()}


def move_types():
    text = open(os.path.join(REPO, 'src/data/battle_moves.h')).read()
    return {m: t for m, t in re.findall(r'\[MOVE_(\w+)\]\s*=\s*\{[^}]*?\.type = TYPE_(\w+)', text, re.S)}


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    csv_dir, red_dir, crystal_dir = sys.argv[1:4]
    red = tm_list(os.path.join(red_dir, 'constants/item_constants.asm'))
    crystal = tm_list(os.path.join(crystal_dir, 'constants/item_constants.asm'))
    assert len(red) == 50 and len(crystal) == 50
    new, reused = the_new_tms(red, crystal)
    moves = [m for m, _, _ in new]
    head = '// Generated by tools/moemon_tms/gen_tm3.py. Do not edit.\n'

    # who learns them
    red_tm, crys_tm = base_stats_tms(os.path.join(red_dir, 'data/pokemon/base_stats')), \
        base_stats_tms(os.path.join(crystal_dir, 'data/pokemon/base_stats'))
    tut = emerald_tutors()
    bits = {}
    g4 = json.load(open(os.path.join(REPO, 'tools/moemon_gen4/gen4_data.json')))['species']
    g4_moves = {}
    for e in g4:
        learn = {x[5:] for x in e['learnset'].get('tutor', [])}
        g4_moves[e['const']] = learn
    for const in set(red_tm) | set(crys_tm) | set(tut) | set(g4_moves):
        have = red_tm.get(const, set()) | crys_tm.get(const, set()) | tut.get(const, set()) | g4_moves.get(const, set())
        b = sum(1 << k for k, m in enumerate(moves) if m in have)
        if b:
            bits[const] = b
    g59 = json.load(open(os.path.join(REPO, 'tools/moemon_gen59/species.json')))
    for e in list(g59.values())[0]:
        teach = {x[5:] for x in e['teach']}
        b = sum(1 << k for k, m in enumerate(moves) if m in teach)
        if b:
            bits[e['const']] = b
    known = set(l.split()[1] for l in open(os.path.join(REPO, 'include/constants/species.h')) if l.startswith('#define SPECIES_'))
    rows = ''.join('    [%s] = 0x%012XULL,\n' % (c, b) for c, b in sorted(bits.items()) if c in known)
    write('src/data/pokemon/tm3_learnsets.h', head +
          '// Which species learn TM93 on (bit k = TM(93 + k), TM/HM index 103 on): Red / Blue and Crystal\'s TMs whose moves\n'
          '// aren\'t TMs here (pokemon.c CanSpeciesLearnTMHM)\n'
          'const u64 gTm3Learnsets[NUM_SPECIES] =\n{\n' + rows + '};\n')

    # the items
    descs = descriptions(csv_dir, moves)
    types = move_types()
    write('include/constants/tms_hms_tm3.h', head +
          '// TM93 on: Red / Blue and Crystal\'s TMs whose moves aren\'t on that number here (TM/HM index 103 on)\n'
          '#define FOREACH_TM3(F) \\\n' + ' \\\n'.join('    F(%s)' % m for m in moves) + '\n'
          + '#define NUM_TECHNICAL_MACHINES_3 %d\n' % len(moves)
          + ''.join('#define ITEM_TM%d %d   // %s (%s TM%02d)\n' % (FIRST_NUMBER + k, FIRST_ITEM + k, m,
                    'Red / Blue' if g == 1 else 'Crystal', num) for k, (m, g, num) in enumerate(new)))
    items, dtext, icons = [head], [head], [head + '#define ITEM_ICONS_TM3 \\\n']
    for k, (m, gen, num) in enumerate(new):
        t = FIRST_NUMBER + k
        price = 5500 if m in STRONG else 2000 if m in STATUS else 3000
        typ = types[m].capitalize()
        if typ == 'Mystery':
            typ = 'Ghost'   # (CURSE: ??? in Gen 3, GHOST from Gen 5)
        items.append('    [ITEM_TM%d] =\n    {\n        .name = _("TM%d"),\n        .itemId = ITEM_TM%d,\n'
                     '        .price = %d,\n        .description = sTM%dDesc,\n        .pocket = POCKET_TM_HM,\n'
                     '        .type = ITEM_USE_PARTY_MENU,\n        .fieldUseFunc = ItemUseOutOfBattle_TMHM,\n    },\n\n'
                     % (t, t, t, price, t))
        dtext.append('static const u8 sTM%dDesc[] = _(\n    "%s");\n\n' % (t, descs[m]))
        icons.append('    [ITEM_TM%d] = {gItemIcon_TM, gItemIconPalette_%sTMHM}, \\\n' % (t, typ))
    write('src/data/items_tm3.h', ''.join(items))
    write('src/data/text/item_descriptions_tm3.h', ''.join(dtext))
    write('src/data/item_icons_tm3.h', ''.join(icons).rstrip(' \\\n') + '\n')
    print('%d new TMs (TM%d-TM%d), %d moves reused, %d species learn at least one'
          % (len(new), FIRST_NUMBER, FIRST_NUMBER + len(new) - 1, len(reused), sum(1 for c in bits if c in known)))
    for g, num, m in reused:
        print('  reused: %s TM%02d %s' % ('Red / Blue' if g == 1 else 'Crystal', num, m))


if __name__ == '__main__':
    main()
