#!/usr/bin/env python3
"""TM51-TM92 (Diamond, Pearl and Platinum's TMs past Emerald's 50) as items 618-659, beside TM01-TM50 and the HMs:
which POKéMON learn each (a separate table, gTm2Learnsets: bit k = TM(51 + k), so the generated TM/HM learnsets stay
as they are), the items (name, price, description, pocket) and their icons (the TM disc in the move's type colors).

    python3 -I tools/moemon_tms/gen_tm2.py      (run from the repository; writes the files below)

Who learns them: species 1-493 from Platinum's own TM lists (tools/moemon_gen4/gen4_data.json, learnset.tm), the
newer species from their teachable moves (tools/moemon_gen59/species.json, teach). TM70 is FLASH, also HM05: its
bit is the same as HM05's list for the newer species and Platinum's TM70 list for the others. The descriptions are
written for this game.
"""
import json
import os

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FIRST_ITEM = 618

# (move, type for the disc's colors, price, description)
TMS = [
    ('ROOST', 'Flying', 3000, "The user lands and rests, healing half its max HP. It loses its FLYING type until the turn ends."),
    ('FOCUS_BLAST', 'Fighting', 5500, "The user gathers all its focus into one blast. It may lower the foe's SP. DEF, but it often misses."),
    ('ENERGY_BALL', 'Grass', 3000, "The user draws power from nature and fires it. It may lower the target's SP. DEF."),
    ('FALSE_SWIPE', 'Normal', 2000, "A careful attack that always leaves the target with at least 1 HP. Handy for catching."),
    ('BRINE', 'Water', 3000, "A salty spray that hits twice as hard when the target has half its HP or less."),
    ('FLING', 'Dark', 3000, "The user throws its held item at the target. The item decides the power and any effect."),
    ('CHARGE_BEAM', 'Electric', 3000, "A beam of electricity that may raise the user's SP. ATK as it fires."),
    ('ENDURE', 'Normal', 2000, "The user braces itself and holds on with 1 HP through any hit this turn. It may fail if used in a row."),
    ('DRAGON_PULSE', 'Dragon', 3000, "The user opens its mouth and releases a shock wave of draconic power."),
    ('DRAIN_PUNCH', 'Fighting', 3000, "A punch that drains the target's energy. The user heals by half the damage dealt."),
    ('WILL_O_WISP', 'Fire', 3000, "Sinister blue-white flames that leave the target with a burn."),
    ('SILVER_WIND', 'Bug', 3000, "A gust of glittering scales. It may raise all of the user's stats at once."),
    ('EMBARGO', 'Dark', 3000, "Keeps the target from using its held item, and its TRAINER from using items on it, for five turns."),
    ('EXPLOSION', 'Normal', 5500, "The user blows up, hitting everyone around it with tremendous force. The user faints."),
    ('SHADOW_CLAW', 'Ghost', 3000, "The user slashes with a claw made of shadow. Critical hits land more often."),
    ('PAYBACK', 'Dark', 3000, "If the user moves after the target, this attack hits twice as hard."),
    ('RECYCLE', 'Normal', 2000, "The user brings back a held item that was used up in battle."),
    ('GIGA_IMPACT', 'Normal', 7500, "The user charges with all its might. It must rest on the next turn."),
    ('ROCK_POLISH', 'Rock', 3000, "The user polishes its body smooth, sharply raising its SPEED."),
    ('FLASH_TM', 'Normal', 1000, "A blinding flash that lowers the target's accuracy. It also lights up dark caves."),
    ('STONE_EDGE', 'Rock', 5500, "The user stabs the target with sharpened stones. Critical hits land more often."),
    ('AVALANCHE', 'Ice', 3000, "An icy attack that hits twice as hard if the user was hurt by the target this turn."),
    ('THUNDER_WAVE', 'Electric', 2000, "A weak jolt of electricity that leaves the target paralyzed."),
    ('GYRO_BALL', 'Steel', 3000, "The user spins at high speed. The slower the user is next to the target, the harder it hits."),
    ('SWORDS_DANCE', 'Normal', 3000, "A frenetic dance that sharply raises the user's ATTACK."),
    ('STEALTH_ROCK', 'Rock', 3000, "Floating stones hurt any POKéMON the foe sends out. ROCK-weak ones take more."),
    ('PSYCH_UP', 'Normal', 2000, "The user copies all of the target's stat changes, good and bad."),
    ('CAPTIVATE', 'Normal', 2000, "Sharply lowers the SP. ATK of a target of the opposite gender."),
    ('DARK_PULSE', 'Dark', 3000, "The user sends out a dreadful aura of dark thoughts. It may make the target flinch."),
    ('ROCK_SLIDE', 'Rock', 3000, "Large boulders crash down on the foes. They may flinch."),
    ('X_SCISSOR', 'Bug', 3000, "The user slashes with its scythes or claws crossed, as if they were a pair of scissors."),
    ('SLEEP_TALK', 'Normal', 2000, "While asleep, the user uses one of its other moves at random."),
    ('NATURAL_GIFT', 'Normal', 2000, "The user draws power from its held BERRY. The BERRY sets the type and power."),
    ('POISON_JAB', 'Poison', 3000, "A stab with a poison-soaked arm or tentacle. It may poison the target."),
    ('DREAM_EATER', 'Psychic', 3000, "The user eats the dreams of a sleeping target, healing by half the damage dealt."),
    ('GRASS_KNOT', 'Grass', 3000, "A snare of grass trips the target. The heavier the target, the harder it falls."),
    ('SWAGGER', 'Normal', 2000, "The user enrages and confuses the target, but sharply raises its ATTACK too."),
    ('PLUCK', 'Flying', 2000, "The user pecks the target. If it holds a BERRY, the user eats it and gets its effect."),
    ('U_TURN', 'Bug', 3000, "The user hits the target, then rushes back to switch places with a party POKéMON."),
    ('SUBSTITUTE', 'Normal', 3000, "The user spends a quarter of its max HP to make a decoy that takes hits in its place."),
    ('FLASH_CANNON', 'Steel', 3000, "The user gathers its light energy and fires it at once. It may lower the target's SP. DEF."),
    ('TRICK_ROOM', 'Psychic', 5500, "A strange room where slower POKéMON move first, for five turns."),
]
assert len(TMS) == 42

# The other regions' own HMs (items ITEM_HM_ROCK_CLIMB on, TM/HM index 100 on, gTm2Learnsets bits 42 on), for parity
# with their games: (move, disc type, description, who learns it: Platinum's HM number for the species to 493, or None)
HMS2 = [
    ('ROCK_CLIMB', 'Normal', "A charging attack that may confuse the target. Out of battle, it climbs rocky walls.", 'HM08'),
    ('DEFOG', 'Flying', "A wind that blows away the foe's barriers and lowers its evasion. Out of battle, it clears deep fog.", 'HM05'),
    ('WHIRLPOOL', 'Water', "Traps the target in a raging whirlpool for several turns. Out of battle, it crosses whirlpools.", None),
]
# WHIRLPOOL's learners to 493 (national numbers): Crystal's HM06 and HeartGold / SoulSilver's HM05 (PokeAPI's
# pokemon_moves.csv: move 250, method 4 (machine), version groups 4 and 10)
WHIRLPOOL_LEARNERS = {7, 8, 9, 31, 34, 54, 55, 60, 61, 62, 72, 73, 79, 80, 86, 87, 90, 91, 98, 99, 108, 112, 115, 116, 117,
                      118, 119, 120, 121, 128, 130, 131, 134, 138, 139, 140, 141, 143, 147, 148, 149, 151, 158, 159, 160,
                      161, 162, 170, 171, 183, 184, 186, 194, 195, 199, 211, 215, 222, 223, 224, 226, 230, 241, 245, 248,
                      249, 258, 259, 260, 263, 264, 270, 271, 272, 279, 295, 296, 297, 298, 306, 318, 319, 320, 321, 339,
                      340, 341, 342, 349, 350, 363, 364, 365, 366, 367, 368, 369, 370, 380, 381, 382, 384, 393, 394, 395,
                      400, 409, 418, 419, 422, 423, 445, 446, 456, 457, 458, 461, 463, 464, 484, 489, 490, 493}


def camel(name):
    return ''.join(p.capitalize() for p in name.split('_'))


def write(path, text):
    open(os.path.join(REPO, path), 'w', encoding='utf-8').write(text)


def main():
    head = '// Generated by tools/moemon_tms/gen_tm2.py. Do not edit.\n'
    moves = ['MOVE_FLASH' if m == 'FLASH_TM' else 'MOVE_' + m for m, _, _, _ in TMS]
    bits = {}
    g4 = json.load(open(os.path.join(REPO, 'tools/moemon_gen4/gen4_data.json')))['species']
    for e in g4:
        b = 0
        for tm in e['learnset']['tm']:
            if tm.startswith('TM') and 51 <= int(tm[2:]) <= 92:
                b |= 1 << (int(tm[2:]) - 51)
        for k, (m, _, _, hm) in enumerate(HMS2):
            if (hm and hm in e['learnset']['tm']) or (m == 'WHIRLPOOL' and e['national'] in WHIRLPOOL_LEARNERS):
                b |= 1 << (len(TMS) + k)
        bits[e['const']] = b
    g59 = json.load(open(os.path.join(REPO, 'tools/moemon_gen59/species.json')))
    for e in list(g59.values())[0]:
        teach = set(e['teach'])
        bits[e['const']] = sum(1 << k for k, mv in enumerate(moves + ['MOVE_' + m for m, _, _, _ in HMS2]) if mv in teach)
    known = set(l.split()[1] for l in open(os.path.join(REPO, 'include/constants/species.h')) if l.startswith('#define SPECIES_'))
    rows = ''.join('    [%s] = 0x%011XULL,\n' % (c, b) for c, b in bits.items() if b and c in known)
    write('src/data/pokemon/tm2_learnsets.h', head +
          '// Which species learn TM51-TM92 (bit k = TM(51 + k), TM/HM index 58 on) and the other regions\' HMs (bits 42 on: ROCK\n'
          '// CLIMB, DEFOG, WHIRLPOOL; TM/HM index 100 on) (pokemon.c CanSpeciesLearnTMHM)\n'
          'const u64 gTm2Learnsets[NUM_SPECIES] =\n{\n' + rows + '};\n')
    # the TM/HM index of each (include/constants/tms_hms.h FOREACH_TM2) and the items
    write('include/constants/tms_hms_tm2.h', head +
          '#define FOREACH_TM2(F) \\\n' + ' \\\n'.join('    F(%s)' % m for m, _, _, _ in TMS) + '\n'
          + '// the other regions\' own HMs (ITEM_HM_ROCK_CLIMB on): TM/HM index 100 on\n'
          + '#define FOREACH_HM2(F) \\\n' + ' \\\n'.join('    F(%s)' % m for m, _, _, _ in HMS2) + '\n')
    items, descs, icons = [head], [head], [head + '#define ITEM_ICONS_TM2 \\\n']
    for k, (m, typ, price, desc) in enumerate(TMS):
        n = 51 + k
        items.append('    [ITEM_TM%d] =\n    {\n        .name = _("TM%d"),\n        .itemId = ITEM_TM%d,\n'
                     '        .price = %d,\n        .description = sTM%dDesc,\n        .pocket = POCKET_TM_HM,\n'
                     '        .type = ITEM_USE_PARTY_MENU,\n        .fieldUseFunc = ItemUseOutOfBattle_TMHM,\n    },\n\n'
                     % (n, n, n, price, n))
        descs.append('static const u8 sTM%dDesc[] = _(\n    "%s");\n\n' % (n, desc))
        icons.append('    [ITEM_TM%d] = {gItemIcon_TM, gItemIconPalette_%sTMHM}, \\\n' % (n, typ))
    for m, typ, desc, _ in HMS2:
        name = 'HM ' + m.replace('_', ' ')
        assert len(name) <= 13, name
        items.append('    [ITEM_HM_%s] =\n    {\n        .name = _("%s"),\n        .itemId = ITEM_HM_%s,\n'
                     '        .price = 0,\n        .description = sHm%sDesc,\n        .importance = 1,\n'
                     '        .pocket = POCKET_TM_HM,\n        .type = ITEM_USE_PARTY_MENU,\n'
                     '        .fieldUseFunc = ItemUseOutOfBattle_TMHM,\n    },\n\n' % (m, name, m, camel(m)))
        descs.append('static const u8 sHm%sDesc[] = _(\n    "%s");\n\n' % (camel(m), desc))
        icons.append('    [ITEM_HM_%s] = {gItemIcon_HM, gItemIconPalette_%sTMHM}, \\\n' % (m, typ))
    write('src/data/items_tm2.h', ''.join(items))
    write('src/data/text/item_descriptions_tm2.h', ''.join(descs))
    write('src/data/item_icons_tm2.h', ''.join(icons).rstrip(' \\\n') + '\n')
    print('%d TMs, %d species learn at least one' % (len(TMS), sum(1 for c, b in bits.items() if b and c in known)))


if __name__ == '__main__':
    main()
