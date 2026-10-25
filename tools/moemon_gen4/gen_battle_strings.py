#!/usr/bin/env python3
"""Gen 4 battle messages: include/constants/battle_gen4_strings.h (ids from 420, usable in battle scripts) and
src/data/text/battle_gen4_strings.h (the texts, read by battle_message.c BufferStringBattle).

    python3 tools/moemon_gen4/gen_battle_strings.py
"""
import os

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
FIRST = 420
S = [
    ('GRAVITY_START', 'Gravity intensified!'),
    ('GRAVITY_END', 'Gravity returned to normal!'),
    ('GRAVITY_CANT', "{B_ATK_NAME_WITH_PREFIX} can't use\\n{B_CURRENT_MOVE} because of gravity!"),
    ('MIRACLE_EYE', '{B_ATK_NAME_WITH_PREFIX} identified\\n{B_DEF_NAME_WITH_PREFIX}!'),
    ('HEALING_WISH', 'The healing wish came true\\nfor {B_SCR_ACTIVE_NAME_WITH_PREFIX}!'),
    ('LUNAR_DANCE', '{B_SCR_ACTIVE_NAME_WITH_PREFIX} became cloaked\\nin mystical moonlight!'),
    ('FEINT', '{B_DEF_NAME_WITH_PREFIX} fell for\\nthe feint!'),
    ('BUG_BITE', "{B_ATK_NAME_WITH_PREFIX} stole and ate\\nits foe's {B_LAST_ITEM}!"),
    ('TAILWIND', 'A tailwind blew from behind\\n{B_ATK_NAME_WITH_PREFIX}\'s team!'),
    ('TAILWIND_END', "{B_ACTIVE_NAME_WITH_PREFIX}'s team's\\ntailwind petered out!"),
    ('EMBARGO', "{B_DEF_NAME_WITH_PREFIX} can't use\\nitems anymore!"),
    ('EMBARGO_END', '{B_ACTIVE_NAME_WITH_PREFIX} can use\\nitems again!'),
    ('FLING', '{B_ATK_NAME_WITH_PREFIX} flung its\\n{B_LAST_ITEM}!'),
    ('HEAL_BLOCK', '{B_DEF_NAME_WITH_PREFIX} was prevented\\nfrom healing!'),
    ('HEAL_BLOCK_END', "{B_ACTIVE_NAME_WITH_PREFIX}'s HEAL BLOCK\\nwore off!"),
    ('HEAL_BLOCKED', "{B_ATK_NAME_WITH_PREFIX} can't use\\n{B_CURRENT_MOVE} because of HEAL BLOCK!"),
    ('POWER_TRICK', '{B_ATK_NAME_WITH_PREFIX} switched its\\nATTACK and DEFENSE!'),
    ('GASTRO_ACID', "{B_DEF_NAME_WITH_PREFIX}'s ability\\nwas suppressed!"),
    ('LUCKY_CHANT', "The LUCKY CHANT shielded\\n{B_ATK_NAME_WITH_PREFIX}'s team from critical hits!"),
    ('LUCKY_CHANT_END', "{B_ACTIVE_NAME_WITH_PREFIX}'s team's\\nLUCKY CHANT wore off!"),
    ('POWER_SWAP', '{B_ATK_NAME_WITH_PREFIX} switched all changes\\nto ATTACK and SP. ATK with the target!'),
    ('GUARD_SWAP', '{B_ATK_NAME_WITH_PREFIX} switched all changes\\nto DEFENSE and SP. DEF with the target!'),
    ('HEART_SWAP', '{B_ATK_NAME_WITH_PREFIX} switched stat\\nchanges with the target!'),
    ('WORRY_SEED', '{B_DEF_NAME_WITH_PREFIX} acquired\\nINSOMNIA!'),
    ('TOXIC_SPIKES', "Poison spikes were scattered all around\\n{B_DEF_NAME_WITH_PREFIX}'s team's feet!"),
    ('TOXIC_SPIKES_GONE', "The poison spikes disappeared from\\naround {B_SCR_ACTIVE_NAME_WITH_PREFIX}'s team's feet!"),
    ('AQUA_RING', '{B_ATK_NAME_WITH_PREFIX} surrounded itself\\nwith a veil of water!'),
    ('AQUA_RING_HEAL', "A veil of water restored\\n{B_ACTIVE_NAME_WITH_PREFIX}'s HP!"),
    ('MAGNET_RISE', '{B_ATK_NAME_WITH_PREFIX} levitated on\\nelectromagnetism!'),
    ('MAGNET_RISE_END', "{B_ACTIVE_NAME_WITH_PREFIX}'s electromagnetism\\nwore off!"),
    ('DEFOG', 'It blew away the obstacles\\naround {B_DEF_NAME_WITH_PREFIX}!'),
    ('TRICK_ROOM', '{B_ATK_NAME_WITH_PREFIX} twisted\\nthe dimensions!'),
    ('TRICK_ROOM_END', 'The twisted dimensions\\nreturned to normal!'),
    ('STEALTH_ROCK', "Pointed stones float in the air\\naround {B_DEF_NAME_WITH_PREFIX}'s team!"),
    ('STEALTH_ROCK_HIT', 'Pointed stones dug into\\n{B_SCR_ACTIVE_NAME_WITH_PREFIX}!'),
    ('SHADOW_FORCE', '{B_ATK_NAME_WITH_PREFIX} vanished\\ninstantly!'),
    ('LIFE_ORB', '{B_ATK_NAME_WITH_PREFIX} lost some\\nof its HP!'),
    ('ITEM_HURT', '{B_ACTIVE_NAME_WITH_PREFIX} is hurt by\\nits {B_LAST_ITEM}!'),
    ('ITEM_POISON', '{B_ACTIVE_NAME_WITH_PREFIX} was badly\\npoisoned by its {B_LAST_ITEM}!'),
    ('ITEM_BURN', '{B_ACTIVE_NAME_WITH_PREFIX} was burned\\nby its {B_LAST_ITEM}!'),
    ('SLOW_START', "{B_SCR_ACTIVE_NAME_WITH_PREFIX} can't\\nget it going!"),
    ('SLOW_START_END', '{B_ACTIVE_NAME_WITH_PREFIX} finally got\\nits act together!'),
    ('ANTICIPATION', '{B_SCR_ACTIVE_NAME_WITH_PREFIX} shuddered!'),
    ('FOREWARN', "It was alerted to\\n{B_BUFF2}'s {B_BUFF1}!"),
    ('FRISK', '{B_SCR_ACTIVE_NAME_WITH_PREFIX} frisked its foe\\nand found its {B_LAST_ITEM}!'),
    ('ABILITY_HEAL', '{B_ACTIVE_NAME_WITH_PREFIX} restored HP\\nusing its {B_LAST_ABILITY}!'),
    ('ABILITY_HURT', '{B_ACTIVE_NAME_WITH_PREFIX} was hurt\\nby its {B_LAST_ABILITY}!'),
    ('BAD_DREAMS', '{B_ACTIVE_NAME_WITH_PREFIX} is tormented!'),
    ('AFTERMATH', '{B_ATK_NAME_WITH_PREFIX} was caught\\nin the aftermath!'),
    ('ABILITY_CURED', "{B_ACTIVE_NAME_WITH_PREFIX}'s {B_LAST_ABILITY}\\ncured its status problem!"),
    ('ANGER_POINT', '{B_DEF_NAME_WITH_PREFIX} maxed\\nits ATTACK!'),
    ('POWER_HERB', '{B_ATK_NAME_WITH_PREFIX} became fully\\ncharged due to its POWER HERB!'),
    ('ITEM_HEAL', '{B_ACTIVE_NAME_WITH_PREFIX} restored a little\\nHP using its {B_LAST_ITEM}!'),
    ('DOWNLOAD', "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s DOWNLOAD\\nraised its {B_BUFF1}!"),
    ('ABILITY_IMMUNE', "{B_DEF_NAME_WITH_PREFIX}'s {B_DEF_ABILITY}\\nmade {B_CURRENT_MOVE} useless!"),
    ('MOLD_BREAKER', '{B_SCR_ACTIVE_NAME_WITH_PREFIX} breaks the mold!'),
    ('PRESSURE_ABILITY', '{B_SCR_ACTIVE_NAME_WITH_PREFIX} is exerting\\nits {B_SCR_ACTIVE_ABILITY}!'),
    ('ACUPRESSURE', "{B_ATK_NAME_WITH_PREFIX}'s {B_BUFF1}\\nsharply rose!"),
    ('PSYCHO_SHIFT', '{B_ATK_NAME_WITH_PREFIX} shifted its\\nstatus to {B_DEF_NAME_WITH_PREFIX}!'),
    ('STICKY_BARB_MOVED', 'The {B_LAST_ITEM} latched onto\\n{B_ATK_NAME_WITH_PREFIX}!'),
    ('NATURAL_GIFT_FAIL', "But it failed!"),
    ('ITEM_STAT', "The {B_LAST_ITEM} raised\\n{B_ACTIVE_NAME_WITH_PREFIX}'s {B_BUFF1}!"),
    ('CUSTAP', '{B_ACTIVE_NAME_WITH_PREFIX} can act faster\\nthan normal, thanks to its {B_LAST_ITEM}!'),
    ('ITEM_WEAKENED', "The {B_LAST_ITEM} weakened\\nthe damage to {B_DEF_NAME_WITH_PREFIX}!"),
    ('ITEM_RECOIL', "{B_ATK_NAME_WITH_PREFIX} was hurt by\\n{B_DEF_NAME_WITH_PREFIX}'s {B_LAST_ITEM}!"),
    ('SNOW_WARNING', "{B_SCR_ACTIVE_NAME_WITH_PREFIX}'s SNOW WARNING\\nwhipped up a hailstorm!"),
    ('FLOWER_VEIL_NONE', ''),
    ('TARGET_WOKE', "{B_DEF_NAME_WITH_PREFIX} woke up!"),
    ('CHOICE_ITEM', ''),
    ('MICLE', "{B_SCR_ACTIVE_NAME_WITH_PREFIX} boosted the accuracy\\nof its next move with {B_LAST_ITEM}!"),
    ('ITEM_SWAP_GOT', "{B_ATK_NAME_WITH_PREFIX} obtained\\n{B_LAST_ITEM}."),
]


def main():
    h = ['// Generated by tools/moemon_gen4/gen_battle_strings.py. Do not edit.',
         '#ifndef GUARD_BATTLE_GEN4_STRINGS_H', '#define GUARD_BATTLE_GEN4_STRINGS_H',
         '#define STRINGID_G4_FIRST %d' % FIRST]
    h += ['#define STRINGID_G4_%-22s %d' % (n, FIRST + i) for i, (n, t) in enumerate(S)]
    h += ['#define STRINGID_G4_COUNT %d' % len(S), '#endif', '']
    open(os.path.join(REPO, 'include/constants/battle_gen4_strings.h'), 'w').write('\n'.join(h))
    t = ['// Generated by tools/moemon_gen4/gen_battle_strings.py. Do not edit.']
    for n, s in S:
        t.append('static const u8 sText_G4_%s[] = _("%s");' % (n, s))
    t.append('static const u8 *const sGen4BattleTexts[] =\n{')
    t += ['    [STRINGID_G4_%s - STRINGID_G4_FIRST] = sText_G4_%s,' % (n, n) for n, s in S]
    t.append('};\n')
    open(os.path.join(REPO, 'src/data/text/battle_gen4_strings.h'), 'w', encoding='utf-8').write('\n'.join(t))
    print('%d strings' % len(S))


if __name__ == '__main__':
    main()
