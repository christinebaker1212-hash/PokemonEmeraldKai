#!/usr/bin/env python3
"""SINNOH's trainers (docs/SINNOH_HANDOFF.md step 3): every trainer standing on a SINNOH map, with Platinum's party data
(species, levels, held items, moves) from pret/pokeplatinum's res/trainers/data, as the game's own trainers. What they
say is written for this game (tools/sinnoh/trainer_words.py); nothing of Platinum's text is used.

    python3 -I tools/sinnoh/gen_trainers.py POKEPLATINUM_DIR     (run from the repository; then gen_maps.py)

- Ids from FIRST_SINNOH_TRAINER (1664, after JOHTO's / KANTO's), TRAINER_SINNOH_<Platinum name>; their "beaten" flags
  from 0x3780 (TRAINER_FLAG, include/constants/flags.h), in the region side file.
- Classes the game has (YOUNGSTER, HIKER...) are used; the others are new from 78 (TRAINER_CLASS_SINNOH_*), with
  pictures the game already has (Emerald's or JOHTO's / KANTO's) as stand-ins; Platinum's own picture of each
  (src/data/sinnoh_trainer_pics.h) is drawn when sinnoh/platinum_gfx.pak is there (tools/sinnoh/build_gfx.py). SINNOH's encounter themes are music ids 24-36.
- Writes include/constants/trainers_sinnoh.h, src/data/trainers_sinnoh.h, src/data/trainer_parties_sinnoh.h,
  src/data/text/trainer_class_names_sinnoh.h and data/scripts/sinnoh_trainers.inc (a script a trainer:
  Sinnoh_EventScript_<Trainer>, which gen_maps.py gives the trainer's object).
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import trainer_words  # noqa: E402
import build_text  # noqa: E402
from gen_words import letter_widths, wrap  # noqa: E402

FIRST_ID = 1664
FLAGS_START = 0x3780
FLAGS_END = 0x3E00               # the hidden items' (tools/sinnoh/items.py)
FIRST_CLASS = 78                 # after JOHTO's / KANTO's (TRAINER_CLASS_QZ_LAST 77)
MUSIC = ['KID', 'MINISKIRT', 'TWINS', 'BIKER', 'HIKER', 'ACE_TRAINER', 'MARTIAL_ARTIST', 'SAILOR', 'AROMA_LADY',
         'COLLECTOR', 'GAMBLER', 'ARTIST', 'GALACTIC']      # encounter music 24-36 (battle_setup.c)
M = {m: 24 + i for i, m in enumerate(MUSIC)}
# new classes: (name shown, prize money per level)
NEW = {'WORKER': ('WORKER', 10), 'VETERAN': ('VETERAN', 20), 'SCIENTIST': ('SCIENTIST', 12), 'SKIER': ('SKIER', 8),
       'JOGGER': ('JOGGER', 8), 'CYCLIST': ('CYCLIST', 8), 'GALACTIC': ('GALACTIC', 10),
       'POKE_KID': ('POKé KID', 4), 'DOUBLE_TEAM': ('DOUBLE TEAM', 15), 'BELLE_AND_PA': ('BELLE & PA', 20),
       'WAITRESS': ('WAITRESS', 8), 'COWGIRL': ('COWGIRL', 8), 'SOCIALITE': ('SOCIALITE', 50), 'PI': ('P.I.', 20),
       'RANCHER': ('RANCHER', 10), 'ROUGHNECK': ('ROUGHNECK', 10), 'ARTIST': ('ARTIST', 12),
       'COMMANDER': ('COMMANDER', 40), 'GALACTIC_BOSS': ('GALACTIC BOSS', 60), 'MAID': ('MAID', 30),
       'CAMERAMAN': ('CAMERAMAN', 16)}
NEW_ID = {k: FIRST_CLASS + i for i, k in enumerate(NEW)}
# Platinum class -> (the game's class, picture, encounter music, female)
CLASSES = {
    'YOUNGSTER': ('YOUNGSTER', 'YOUNGSTER', 'KID', 0), 'LASS': ('LASS', 'LASS', 'MINISKIRT', 1),
    'SCHOOL_KID_MALE': ('SCHOOL_KID', 'SCHOOL_KID_M', 'KID', 0),
    'SCHOOL_KID_FEMALE': ('SCHOOL_KID', 'SCHOOL_KID_F', 'MINISKIRT', 1),
    'POKE_KID': ('S:POKE_KID', 'QZ_262', 'MINISKIRT', 1), 'TUBER_MALE': ('TUBER_M', 'TUBER_M', 'KID', 0),
    'TUBER_FEMALE': ('TUBER_F', 'TUBER_F', 'MINISKIRT', 1), 'BUG_CATCHER': ('BUG_CATCHER', 'BUG_CATCHER', 'KID', 0),
    'TWINS': ('TWINS', 'TWINS', 'TWINS', 1), 'NINJA_BOY': ('NINJA_BOY', 'NINJA_BOY', 'KID', 0),
    'CAMPER': ('CAMPER', 'CAMPER', 'KID', 0), 'PICNICKER': ('PICNICKER', 'PICNICKER', 'MINISKIRT', 1),
    'HIKER': ('HIKER', 'HIKER', 'HIKER', 0), 'WORKER': ('S:WORKER', 'QZ_243', 'HIKER', 0),
    'RUIN_MANIAC': ('RUIN_MANIAC', 'RUIN_MANIAC', 'HIKER', 0), 'RANCHER': ('S:RANCHER', 'QZ_247', 'HIKER', 0),
    'COWGIRL': ('S:COWGIRL', 'QZ_236', 'MINISKIRT', 1), 'ROUGHNECK': ('S:ROUGHNECK', 'QZ_238', 'BIKER', 0),
    'SWIMMER_MALE': ('SWIMMER_M', 'SWIMMER_M', 'SAILOR', 0), 'SWIMMER_FEMALE': ('SWIMMER_F', 'SWIMMER_F', 'MINISKIRT', 1),
    'FISHERMAN': ('FISHERMAN', 'FISHERMAN', 'SAILOR', 0), 'SAILOR': ('SAILOR', 'SAILOR', 'SAILOR', 0),
    'BLACK_BELT': ('BLACK_BELT', 'BLACK_BELT', 'MARTIAL_ARTIST', 0),
    'BATTLE_GIRL': ('BATTLE_GIRL', 'BATTLE_GIRL', 'MARTIAL_ARTIST', 1),
    'ACE_TRAINER_MALE': ('COOLTRAINER', 'COOLTRAINER_M', 'ACE_TRAINER', 0),
    'ACE_TRAINER_FEMALE': ('COOLTRAINER', 'COOLTRAINER_F', 'ACE_TRAINER', 1),
    'ACE_TRAINER_SNOW_MALE': ('COOLTRAINER', 'COOLTRAINER_M', 'ACE_TRAINER', 0),
    'ACE_TRAINER_SNOW_FEMALE': ('COOLTRAINER', 'COOLTRAINER_F', 'ACE_TRAINER', 1),
    'VETERAN': ('S:VETERAN', 'QZ_246', 'ACE_TRAINER', 0), 'DRAGON_TAMER': ('DRAGON_TAMER', 'DRAGON_TAMER', 'ACE_TRAINER', 0),
    'RANGER_MALE': ('PKMN_RANGER', 'POKEMON_RANGER_M', 'ACE_TRAINER', 0),
    'RANGER_FEMALE': ('PKMN_RANGER', 'POKEMON_RANGER_F', 'ACE_TRAINER', 1),
    'PSYCHIC_MALE': ('PSYCHIC', 'PSYCHIC_M', 'AROMA_LADY', 0), 'PSYCHIC_FEMALE': ('PSYCHIC', 'PSYCHIC_F', 'AROMA_LADY', 1),
    'GALACTIC_GRUNT_MALE': ('S:GALACTIC', 'QZ_270', 'GALACTIC', 0),
    'GALACTIC_GRUNT_FEMALE': ('S:GALACTIC', 'QZ_271', 'GALACTIC', 1),
    'RICH_BOY': ('RICH_BOY', 'RICH_BOY', 'GAMBLER', 0), 'LADY': ('LADY', 'LADY', 'AROMA_LADY', 1),
    'GENTLEMAN': ('GENTLEMAN', 'GENTLEMAN', 'GAMBLER', 0), 'SOCIALITE': ('S:SOCIALITE', 'QZ_249', 'AROMA_LADY', 1),
    'BEAUTY': ('BEAUTY', 'BEAUTY', 'AROMA_LADY', 1), 'PARASOL_LADY': ('PARASOL_LADY', 'PARASOL_LADY', 'AROMA_LADY', 1),
    'AROMA_LADY': ('AROMA_LADY', 'AROMA_LADY', 'AROMA_LADY', 1),
    'BELLE_AND_PA': ('S:BELLE_AND_PA', 'OLD_COUPLE', 'TWINS', 1), 'WAITRESS': ('S:WAITRESS', 'QZ_239', 'MINISKIRT', 1),
    'ARTIST': ('S:ARTIST', 'QZ_210', 'ARTIST', 0), 'MAID': ('S:MAID', 'QZ_239', 'MINISKIRT', 1),
    'CAMERAMAN': ('S:CAMERAMAN', 'QZ_220', 'COLLECTOR', 0),
    'JOGGER': ('S:JOGGER', 'RUNNING_TRIATHLETE_M', 'BIKER', 0),
    'CYCLIST_MALE': ('S:CYCLIST', 'CYCLING_TRIATHLETE_M', 'BIKER', 0),
    'CYCLIST_FEMALE': ('S:CYCLIST', 'CYCLING_TRIATHLETE_F', 'BIKER', 1),
    'SKIER_MALE': ('S:SKIER', 'RUNNING_TRIATHLETE_M', 'BIKER', 0),
    'SKIER_FEMALE': ('S:SKIER', 'RUNNING_TRIATHLETE_F', 'BIKER', 1),
    'COLLECTOR': ('COLLECTOR', 'COLLECTOR', 'COLLECTOR', 0), 'POKEFAN_MALE': ('POKEFAN', 'POKEFAN_M', 'COLLECTOR', 0),
    'POKEFAN_FEMALE': ('POKEFAN', 'POKEFAN_F', 'COLLECTOR', 1),
    'BREEDER_MALE': ('PKMN_BREEDER', 'POKEMON_BREEDER_M', 'COLLECTOR', 0),
    'BREEDER_FEMALE': ('PKMN_BREEDER', 'POKEMON_BREEDER_F', 'COLLECTOR', 1),
    'BIRD_KEEPER': ('BIRD_KEEPER', 'BIRD_KEEPER', 'KID', 0), 'GUITARIST': ('GUITARIST', 'GUITARIST', 'ARTIST', 0),
    'SCIENTIST': ('S:SCIENTIST', 'QZ_274', 'COLLECTOR', 0), 'PI': ('S:PI', 'QZ_220', 'GAMBLER', 0),
    'POLICEMAN': ('QZ_POLICEMAN', 'QZ_219', 'GAMBLER', 0),
    'DOUBLE_TEAM': ('S:DOUBLE_TEAM', 'QZ_281', 'TWINS', 0), 'YOUNG_COUPLE': ('YOUNG_COUPLE', 'YOUNG_COUPLE', 'TWINS', 1),
}
# trainers battled by the region's own scripts (gym leaders, the ELITE FOUR, the CHAMPION): (class, picture, female);
# pictures the game already has, the closest it has
SPECIAL = {
    'TRAINER_LEADER_ROARK': ('LEADER', 'QZ_243', 0), 'TRAINER_LEADER_GARDENIA': ('LEADER', 'QZ_212', 1),
    'TRAINER_LEADER_FANTINA': ('LEADER', 'QZ_206', 1), 'TRAINER_LEADER_MAYLENE': ('LEADER', 'BATTLE_GIRL', 1),
    'TRAINER_LEADER_WAKE': ('LEADER', 'QZ_205', 0), 'TRAINER_LEADER_BYRON': ('LEADER', 'QZ_247', 0),
    'TRAINER_LEADER_CANDICE': ('LEADER', 'QZ_258', 1), 'TRAINER_LEADER_VOLKNER': ('LEADER', 'QZ_256', 0),
    'TRAINER_ELITE_FOUR_AARON': ('ELITE_FOUR', 'BUG_MANIAC', 0), 'TRAINER_ELITE_FOUR_BERTHA': ('ELITE_FOUR', 'QZ_216', 1),
    'TRAINER_ELITE_FOUR_FLINT': ('ELITE_FOUR', 'QZ_116', 0), 'TRAINER_ELITE_FOUR_LUCIAN': ('ELITE_FOUR', 'QZ_100', 0),
    'TRAINER_CHAMPION_CYNTHIA': ('CHAMPION', 'QZ_171', 1),
}
# ... and the story's (tools/sinnoh/legends.py STORY: BARRY in Platinum's three versions, TEAM GALACTIC's commanders
# and boss): (class, picture, female, encounter music)
STORY_TRAINERS = {}
for _place in ('route_201', 'route_203', 'route_209', 'pastoria_city', 'canalave_city', 'pokemon_league',
               'survival_area_1', 'fight_area'):
    for _starter in ('chimchar', 'piplup', 'turtwig'):
        STORY_TRAINERS['TRAINER_RIVAL_%s_%s' % (_place.upper(), _starter.upper())] = ('RIVAL', 'YOUNGSTER', 0, 'ACE_TRAINER')
for _t, _pic, _f in (('COMMANDER_MARS_VALLEY_WINDWORKS', 'AQUA_ADMIN_F', 1),
                     ('COMMANDER_JUPITER_TEAM_GALACTIC_ETERNA_BUILDING', 'AQUA_ADMIN_F', 1),
                     ('COMMANDER_MARS_LAKE_VERITY', 'AQUA_ADMIN_F', 1), ('COMMANDER_SATURN_VALOR_CAVERN', 'AQUA_ADMIN_M', 0),
                     ('COMMANDER_SATURN_GALACTIC_HQ', 'AQUA_ADMIN_M', 0), ('GALACTIC_BOSS_CYRUS_GALACTIC_HQ', 'MAGMA_LEADER_MAXIE', 0),
                     ('COMMANDER_MARS_SPEAR_PILLAR', 'AQUA_ADMIN_F', 1), ('COMMANDER_JUPITER_SPEAR_PILLAR', 'AQUA_ADMIN_F', 1),
                     ('COMMANDER_MARS_STARK_MOUNTAIN', 'AQUA_ADMIN_F', 1), ('COMMANDER_JUPITER_STARK_MOUNTAIN', 'AQUA_ADMIN_F', 1),
                     ('GALACTIC_BOSS_CYRUS_CELESTIC_TOWN_RUINS', 'MAGMA_LEADER_MAXIE', 0)):
    STORY_TRAINERS['TRAINER_' + _t] = ('S:GALACTIC_BOSS' if 'BOSS' in _t else 'S:COMMANDER', _pic, _f, 'GALACTIC')
# ... and the story's added later, after every other (ids and "beaten" flags of those above stay put): CYRUS in the
# DISTORTION WORLD (tools/sinnoh/distortion.py)
LATE_STORY = {'TRAINER_GALACTIC_BOSS_CYRUS_DISTORTION_WORLD': ('S:GALACTIC_BOSS', 'MAGMA_LEADER_MAXIE', 0, 'GALACTIC')}
STORY_TRAINERS.update(LATE_STORY)
RENAME = {'ITEM_X_DEFENSE': 'ITEM_X_DEFEND', 'MOVE_SMOKE_SCREEN': 'MOVE_SMOKESCREEN', 'MOVE_SOFTBOILED': 'MOVE_SOFT_BOILED', 'MOVE_HIGH_JUMP_KICK': 'MOVE_HI_JUMP_KICK',
          'ITEM_PARLYZ_HEAL': 'ITEM_PARALYZE_HEAL'}


# Trainers who battle when talked to (Platinum's scripts start their battles; their objects aren't trainers in its
# events): ETERNA's gym (its clock moves as they're beaten), the TRAINERS' SCHOOL, the POKéMON MANSION's maids, the
# restaurant's diners. Kept after the others (their ids and flags come last); gen_maps.py gives their objects the
# battle (TALK_SCRIPTS: the Platinum script entry that battles each)
TALK = ['TRAINER_LASS_CAROLINE', 'TRAINER_AROMA_LADY_JENNA', 'TRAINER_AROMA_LADY_ANGELA',
        'TRAINER_SCHOOL_KID_CHRISTINE', 'TRAINER_SCHOOL_KID_HARRISON',
        'TRAINER_MAID_BELINDA', 'TRAINER_MAID_CLARE', 'TRAINER_MAID_ELENA', 'TRAINER_MAID_EMILY', 'TRAINER_MAID_SOPHIE',
        'TRAINER_ARTIST_ISMAEL', 'TRAINER_CAMERAMAN_DARRYL', 'TRAINER_COLLECTOR_EUGENE', 'TRAINER_GENTLEMAN_LEONARDO',
        'TRAINER_PI_KENDRICK', 'TRAINER_RICH_BOY_ROMAN', 'TRAINER_SCHOOL_KID_ESTEBAN', 'TRAINER_SCIENTIST_EMILIO',
        'TRAINER_VETERAN_EMANUEL']


def placed(root):
    """{TRAINER_...: Platinum event file name} of every trainer standing on a map"""
    out = {}
    edir = os.path.join(root, 'res/field/events')
    for f in sorted(os.listdir(edir)):
        if f.endswith('.json'):
            for o in json.load(open(os.path.join(edir, f)))['object_events']:
                if isinstance(o['script'], str) and o['script'].startswith('TRAINER_'):
                    out.setdefault(o['script'], f)
    return out


def camel(const):
    return ''.join(p.capitalize() for p in const[len('TRAINER_'):].split('_'))


def label(const):
    """The trainer's script (gen_maps.py gives it the trainer's object)"""
    return 'Sinnoh_EventScript_Tr' + camel(const)


def our_const(const):
    return 'TRAINER_SINNOH_' + const[len('TRAINER_'):]


def main():
    root = sys.argv[1]
    known = set()
    for h in ('include/constants/species.h', 'include/constants/moves.h', 'include/constants/items.h',
              'include/constants/trainers.h'):
        known |= set(re.findall(r'#define (\w+)\s', open(h).read()))
    bad = set()

    def c(name):
        name = RENAME.get(name, name)
        if name not in known:
            bad.add(name)
        return name

    trainers = sorted(placed(root)) + list(SPECIAL) + [t for t in STORY_TRAINERS if t not in LATE_STORY] \
        + [t for t in TALK if t not in placed(root)] + list(LATE_STORY)
    widths = letter_widths()
    consts, entries, parties, script = [], [], [], []
    pics = []   # Platinum's battle picture of each (its class's, include/constants/sinnoh_gfx.h)
    have_pics = set(re.findall(r'#define SINNOH_PIC_(\w+)\s+\d', open('include/constants/sinnoh_gfx.h').read()))
    head = '// Generated by tools/sinnoh/gen_trainers.py. Do not edit.\n'
    for n, t in enumerate(trainers):
        d = json.load(open(os.path.join(root, 'res/trainers/data', t[len('TRAINER_'):].lower() + '.json')))
        pclass = d['class'][len('TRAINER_CLASS_'):]
        pics.append('SINNOH_PIC_' + pclass if pclass in have_pics else 'SINNOH_PIC_NONE')
        if t in SPECIAL:
            ours, pic, female = SPECIAL[t]
            music = 'ACE_TRAINER'      # (their scripts battle with NO_MUSIC)
        elif t in STORY_TRAINERS:
            ours, pic, female, music = STORY_TRAINERS[t]
        else:
            ours, pic, music, female = CLASSES[pclass]
        cls = 'TRAINER_CLASS_SINNOH_' + ours[2:] if ours.startswith('S:') else c('TRAINER_CLASS_' + ours)
        const = our_const(t)
        consts.append('#define %-48s %d\n' % (const, FIRST_ID + n))
        held = any(m.get('item') for m in d['party'])
        moves = any(m.get('moves') for m in d['party'])
        struct = {(False, False): 'NoItemDefaultMoves', (False, True): 'NoItemCustomMoves',
                  (True, False): 'ItemDefaultMoves', (True, True): 'ItemCustomMoves'}[(held, moves)]
        macro = {'NoItemDefaultMoves': 'NO_ITEM_DEFAULT_MOVES', 'NoItemCustomMoves': 'NO_ITEM_CUSTOM_MOVES',
                 'ItemDefaultMoves': 'ITEM_DEFAULT_MOVES', 'ItemCustomMoves': 'ITEM_CUSTOM_MOVES'}[struct]
        pname = 'sParty_Sinnoh' + camel(t)
        mons = ''
        for m in d['party']:
            line = '    {.iv = %d, .lvl = %d, .species = %s' % (m['iv_scale'], m['level'], c(m['species']))
            if held:
                line += ', .heldItem = %s' % c(m.get('item') or 'ITEM_NONE')
            if moves:
                mv = [c(x) for x in (m.get('moves') or [])][:4]
                line += ', .moves = {%s}' % ', '.join(mv + ['MOVE_NONE'] * (4 - len(mv)))
            mons += line + '},\n'
        parties.append('static const struct TrainerMon%s %s[] = {\n%s};\n\n' % (struct, pname, mons))
        items = [c(i) for i in d['items']][:4]
        items += ['ITEM_NONE'] * (4 - len(items))
        ai = '0x7' if any(f != 'AI_FLAG_BASIC' for f in d['ai_flags']) else '0x1'
        name = d['name'].upper().replace('&', '&')[:12]
        entries.append('    [%s] =\n    {\n        .trainerClass = %s,\n        .encounterMusic_gender = %d%s,\n'
                       '        .trainerPic = TRAINER_PIC_%s,\n        .trainerName = _("%s"),\n'
                       '        .items = {%s},\n        .doubleBattle = %s,\n        .aiFlags = %s,\n'
                       '        .party = %s(%s),\n    },\n\n'
                       % (const, cls, M[music], ' | F_TRAINER_FEMALE' if female else '', c('TRAINER_PIC_' + pic)[12:],
                          name, ', '.join(items), 'TRUE' if d['double_battle'] else 'FALSE', ai, macro, pname))
        if t in SPECIAL or t in STORY_TRAINERS:
            continue        # (their words are in tools/sinnoh/words.py / legends.py, with their scripts)
        intro, defeat, after = trainer_words.lines(pclass, t)
        lab = label(t)
        base = 'Sinnoh_Text_Tr' + camel(t)
        if d['double_battle']:
            script.append('%s::\n\ttrainerbattle_double %s, %sIntro, %sDefeat, %sNotEnough\n\tmsgbox %sAfter, '
                          'MSGBOX_AUTOCLOSE\n\tend\n\n' % (lab, const, base, base, base, base))
        else:
            script.append('%s::\n\ttrainerbattle_single %s, %sIntro, %sDefeat\n\tmsgbox %sAfter, MSGBOX_AUTOCLOSE\n'
                          '\tend\n\n' % (lab, const, base, base, base))
        # their words are Platinum's own (the text pack's trainers' bank, tools/sinnoh/build_text.py: only its numbers
        # here); the written ones only where Platinum has no such line
        kinds = {'Intro': ('TRMSG_PRE_DOUBLE_BATTLE_1', 'TRMSG_PRE_BATTLE'), 'Defeat': ('TRMSG_DOUBLE_BATTLE_DEFEAT_1', 'TRMSG_DEFEAT'),
                 'After': ('TRMSG_POST_DOUBLE_BATTLE_1', 'TRMSG_POST_BATTLE'),
                 'NotEnough': ('TRMSG_DOUBLE_BATTLE_NOT_ENOUGH_POKEMON_1',)}
        for suffix, text in (('Intro', intro), ('Defeat', defeat), ('After', after)) + \
                ((('NotEnough', trainer_words.NOT_ENOUGH),) if d['double_battle'] else ()):
            where = None
            for kind in kinds[suffix][0 if d['double_battle'] else -1:] if suffix != 'NotEnough' else kinds[suffix]:
                where = where or build_text.trainer_message(root, t, kind)
            if where:
                script.append('%s%s:\n\t.byte %s\n\n' % (base, suffix,
                              ', '.join('0x%02X' % x for x in build_text.sinnoh_text_bytes(*where))))
            else:
                script.append('%s%s:\n%s\n' % (base, suffix, wrap(text, widths, t)))
    if bad:
        sys.exit('not in the game: %s' % ' '.join(sorted(bad)))
    assert FLAGS_START + len(trainers) <= FLAGS_END, 'too many SINNOH trainers for their flags'
    open('include/constants/trainers_sinnoh.h', 'w').write(
        head + '#ifndef GUARD_CONSTANTS_TRAINERS_SINNOH_H\n#define GUARD_CONSTANTS_TRAINERS_SINNOH_H\n'
        '#define FIRST_SINNOH_TRAINER %d\n#define SINNOH_TRAINERS_COUNT %d\n'
        '#define TRAINER_ENCOUNTER_MUSIC_SINNOH_FIRST 24   // 24-36: SINNOH\'s encounter themes (battle_setup.c)\n'
        % (FIRST_ID, len(trainers)) +
        ''.join('#define TRAINER_CLASS_SINNOH_%s %d\n' % (k, v) for k, v in NEW_ID.items()) +
        '#define TRAINER_CLASS_SINNOH_LAST %d\n' % (FIRST_CLASS + len(NEW) - 1) + ''.join(consts) + '#endif\n')
    open('src/data/trainers_sinnoh.h', 'w').write(head + ''.join(entries))
    open('src/data/trainer_parties_sinnoh.h', 'w').write(head + ''.join(parties))
    open('src/data/text/trainer_class_names_sinnoh.h', 'w').write(
        head + '#pragma once\n#define SINNOH_TRAINER_CLASS_NAMES \\\n' +
        ''.join('    [TRAINER_CLASS_SINNOH_%s] = _("%s"), \\\n' % (k, v[0]) for k, v in NEW.items()) +
        '\n#define SINNOH_TRAINER_MONEY \\\n' +
        ''.join('    {TRAINER_CLASS_SINNOH_%s, %d}, \\\n' % (k, v[1]) for k, v in NEW.items()) + '\n')
    open('src/data/sinnoh_trainer_pics.h', 'w').write(
        head + "// Each SINNOH trainer's Platinum battle picture (src/sinnoh/s3d_gfx.c S3D_Gfx_TrainerPic), by trainer id\n"
        'static const u8 sSinnohTrainerPics[SINNOH_TRAINERS_COUNT] = {\n' +
        ''.join('    %s,\n' % x for x in pics) + '};\n')
    open('data/scripts/sinnoh_trainers.inc', 'w').write(head.replace('//', '@') + ''.join(script))
    print('%d SINNOH trainers (%d classes new)' % (len(trainers), len(NEW)))


if __name__ == '__main__':
    main()
