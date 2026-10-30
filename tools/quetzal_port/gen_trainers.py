#!/usr/bin/env python3
"""Johto / Kanto trainers (the Gen 4 cap of the rip: quetzal_rip/gen4/data/trainers.json) as Kai trainers.

    python3 tools/quetzal_port/gen_trainers.py

- Ids from MAX_TRAINERS_COUNT (864) on, TRAINER_QZ_<Quetzal id> in include/constants/qz_port.h. Their "beaten" flags
  live past Emerald's (TRAINER_FLAG in include/constants/flags.h: flags 0x3000+, in the region side file).
- Quetzal's 12 classes past Emerald's (66-77: BURGLAR .. TEAM ROCKET) keep their numbers (TRAINER_CLASS_QZ_*).
- Front pictures 0-92 are Emerald's: Kai's own art is used. The others are imported (TRAINER_PIC_QZ_<id>, from 95).
- Encounter music 14-23 are Johto's encounter themes (battle_setup.c).
- Party kinds: Quetzal's fifth kind (IVs, EVs, nature, ability) becomes Kai's held item + moves (the extras dropped).
Writes src/data/trainers_qz.h, src/data/trainer_parties_qz.h, src/data/graphics/trainers_qz.h,
src/data/trainer_graphics/front_pic_qz.h and the generated blocks in include/constants/trainers.h.
"""
import json
import os
import re
import shutil

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
RIP = os.path.join(REPO, 'quetzal_rip')
FIRST_ID = 864           # MAX_TRAINERS_COUNT
FIRST_PIC = 95           # after TRAINER_PIC_MP_OPPONENT
KAI_PICS = 93            # Quetzal's 0-92 are Emerald's
CLASSES = {66: 'BURGLAR', 67: 'FIREBREATHER', 68: 'JUGGLER', 69: 'KIMONO_GIRL', 70: 'MYSTERY_MAN', 71: 'POLICEMAN',
           72: 'PSYCHIC_2', 73: 'RIVAL', 74: 'ROCKET_ADMIN', 75: 'SAGE', 76: 'SUPER_NERD', 77: 'TEAM_ROCKET'}
CLASS_NAMES = {66: 'BURGLAR', 67: 'FIREBREATHER', 68: 'JUGGLER', 69: 'KIMONO GIRL', 70: 'MYSTERY MAN', 71: 'POLICEMAN',
               72: 'PSYCHIC', 73: 'RIVAL', 74: 'ROCKET ADMIN', 75: 'SAGE', 76: 'SUPER NERD', 77: 'TEAM ROCKET'}
MONEY = {66: 22, 67: 12, 68: 10, 69: 15, 70: 20, 71: 10, 72: 10, 73: 16, 74: 25, 75: 12, 76: 12, 77: 9}
BEGIN, END = '// Moemon Kai: Johto / Kanto (tools/quetzal_port/gen_trainers.py)', '// end of Johto / Kanto trainers'


def consts(path, prefix):
    return set(re.findall(r'#define (%s\w+)' % prefix, open(os.path.join(REPO, path), encoding='utf-8').read()))


def text(s):
    return s.replace('\\', '\\\\').replace('"', '\\"')


def between(path, block):
    s = open(path, encoding='utf-8').read()
    full = '%s\n%s%s\n' % (BEGIN, block, END)
    pat = re.compile(r'%s\n.*?%s\n' % (re.escape(BEGIN), re.escape(END)), re.S)
    return pat.sub(lambda m: full, s) if pat.search(s) else None, full, s


def main():
    data = json.load(open(os.path.join(RIP, 'gen4', 'data', 'trainers.json')))
    trainers = sorted(data['trainers'], key=lambda t: t['id'])
    # Quetzal's Hoenn trainer the Johto / Kanto scripts battle (its Hoenn data isn't in the rip): Jessie and James
    rocket_pic = next(t['trainer_pic'] for t in trainers if t['class'] == 77)
    trainers.append({'id': 913, 'const': 'TRAINER_QZ_913', 'name': 'JESSIE&JAMES', 'class': 77, 'encounter_music': 14,
                     'female': True, 'trainer_pic': rocket_pic, 'items': ['ITEM_NONE'] * 4, 'double_battle': False,
                     'ai_flags': '0x7', 'party_type': 1, 'party': [
                         {'iv': 120, 'lvl': 30, 'species': 'SPECIES_ARBOK', 'moves': ['MOVE_POISON_FANG', 'MOVE_BITE', 'MOVE_GLARE', 'MOVE_SCREECH']},
                         {'iv': 120, 'lvl': 30, 'species': 'SPECIES_WEEZING', 'moves': ['MOVE_SLUDGE', 'MOVE_SMOKESCREEN', 'MOVE_TACKLE', 'MOVE_HAZE']},
                         {'iv': 120, 'lvl': 30, 'species': 'SPECIES_MEOWTH', 'moves': ['MOVE_PAY_DAY', 'MOVE_FURY_SWIPES', 'MOVE_BITE', 'MOVE_SCREECH']},
                         {'iv': 120, 'lvl': 30, 'species': 'SPECIES_WOBBUFFET', 'moves': ['MOVE_COUNTER', 'MOVE_MIRROR_COAT', 'MOVE_SAFEGUARD', 'MOVE_DESTINY_BOND']}]})
    kai = consts('include/constants/species.h', 'SPECIES_') | consts('include/constants/moves.h', 'MOVE_') | \
        consts('include/constants/items.h', 'ITEM_')
    bad = set()

    def check(c):
        c = {'MOVE_HIGH_JUMP_KICK': 'MOVE_HI_JUMP_KICK'}.get(c, c)
        if c not in kai:
            bad.add(c)
        return c

    # pictures
    pics, picmap = [], {}
    for t in trainers:
        p = t['trainer_pic']
        if p >= KAI_PICS and p not in picmap:
            picmap[p] = FIRST_PIC + len(pics)
            pics.append(p)
    os.makedirs(os.path.join(REPO, 'graphics/trainers/front_pics/qz'), exist_ok=True)
    gfx, sheets, pals, coords, anims, picconsts = '', '', '', '', '', ''
    for p in pics:
        shutil.copy(os.path.join(RIP, 'trainer_pics/front/pic_%d.png' % p), os.path.join(REPO, 'graphics/trainers/front_pics/qz/pic_%d.png' % p))
        shutil.copy(os.path.join(RIP, 'trainer_pics/front/pic_%d.pal' % p), os.path.join(REPO, 'graphics/trainers/front_pics/qz/pic_%d.pal' % p))
        gfx += ('const u32 gTrainerFrontPic_Qz%d[] = INCGFX_U32("graphics/trainers/front_pics/qz/pic_%d.png", ".4bpp.lz");\n'
                'const u32 gTrainerPalette_Qz%d[] = INCGFX_U32("graphics/trainers/front_pics/qz/pic_%d.pal", ".gbapal.lz");\n') % (p, p, p, p)
        picconsts += '#define TRAINER_PIC_QZ_%d %d\n' % (p, picmap[p])
        sheets += '    TRAINER_SPRITE(QZ_%d, gTrainerFrontPic_Qz%d, TRAINER_PIC_SIZE), \\\n' % (p, p)
        pals += '    TRAINER_PAL(QZ_%d, gTrainerPalette_Qz%d), \\\n' % (p, p)
        coords += '    [TRAINER_PIC_QZ_%d] = {.size = 8, .y_offset = 1}, \\\n' % p
        anims += '    [TRAINER_PIC_QZ_%d] = sAnims_RubySapphireMay, \\\n' % p
    head = '// Generated by tools/quetzal_port/gen_trainers.py. Do not edit.\n'
    open(os.path.join(REPO, 'src/data/graphics/trainers_qz.h'), 'w').write(head + gfx)
    open(os.path.join(REPO, 'src/data/trainer_graphics/front_pic_qz.h'), 'w').write(
        head + '#pragma once\n' + ''.join('extern const u32 gTrainerFrontPic_Qz%d[];\nextern const u32 gTrainerPalette_Qz%d[];\n' % (p, p) for p in pics)
        + '#define QZ_TRAINER_SPRITES \\\n%s\n#define QZ_TRAINER_PALS \\\n%s\n#define QZ_TRAINER_COORDS \\\n%s\n#define QZ_TRAINER_ANIMS \\\n%s\n'
        % (sheets, pals, coords, anims))

    # trainers and parties
    parties, entries, ids = '', '', ''
    for n, t in enumerate(trainers):
        tid = FIRST_ID + n
        ids += '#define %-40s %d\n' % (t['const'], tid)
        kind = t['party_type']
        name = 'sParty_Qz%d' % t['id']
        held = kind in (2, 3, 4)
        moves = kind in (1, 3, 4)
        struct = {(False, False): 'NoItemDefaultMoves', (False, True): 'NoItemCustomMoves',
                  (True, False): 'ItemDefaultMoves', (True, True): 'ItemCustomMoves'}[(held, moves)]
        mons = ''
        for m in t['party']:
            iv = m.get('iv', 0)
            if kind == 4:
                iv = min(255, iv * 255 // 31) if iv <= 31 else iv
            line = '    {.iv = %d, .lvl = %d, .species = %s' % (iv, m['lvl'], check(m['species']))
            if held:
                line += ', .heldItem = %s' % check(m.get('held_item') or m.get('item') or 'ITEM_NONE')
            if moves:
                line += ', .moves = {%s}' % ', '.join(check(x) for x in (m.get('moves') or ['MOVE_NONE'] * 4))
            mons += line + '},\n'
        parties += 'static const struct TrainerMon%s %s[] = {\n%s};\n\n' % (struct, name, mons)
        macro = {'NoItemDefaultMoves': 'NO_ITEM_DEFAULT_MOVES', 'NoItemCustomMoves': 'NO_ITEM_CUSTOM_MOVES',
                 'ItemDefaultMoves': 'ITEM_DEFAULT_MOVES', 'ItemCustomMoves': 'ITEM_CUSTOM_MOVES'}[struct]
        cls = t['class']
        cls_c = 'TRAINER_CLASS_QZ_%s' % CLASSES[cls] if cls in CLASSES else str(cls)
        pic = picmap.get(t['trainer_pic'], t['trainer_pic'])
        entries += ('    [%s] =\n    {\n        .trainerClass = %s,\n        .encounterMusic_gender = %d%s,\n'
                    '        .trainerPic = %d,\n        .trainerName = _("%s"),\n        .items = {%s},\n'
                    '        .doubleBattle = %s,\n        .aiFlags = %s,\n        .party = %s(%s),\n    },\n\n') % (
            t['const'], cls_c, t['encounter_music'], ' | F_TRAINER_FEMALE' if t['female'] else '', pic, text(t['name']),
            ', '.join(check(i) for i in t['items']), 'TRUE' if t['double_battle'] else 'FALSE', t['ai_flags'], macro, name)
    open(os.path.join(REPO, 'src/data/trainer_parties_qz.h'), 'w', encoding='utf-8').write(head + parties)
    open(os.path.join(REPO, 'src/data/trainers_qz.h'), 'w', encoding='utf-8').write(head + entries)

    # constants
    block = '#define FIRST_QZ_TRAINER %d\n#define QZ_TRAINERS_COUNT %d\n' % (FIRST_ID, len(trainers))
    block += ''.join('#define TRAINER_CLASS_QZ_%s %d\n' % (v, k) for k, v in CLASSES.items())
    block += '#define TRAINER_CLASS_QZ_LAST %d\n' % max(CLASSES)
    block += '#define TRAINER_ENCOUNTER_MUSIC_QZ_FIRST 14   // 14-23: Johto\'s encounter themes\n'
    block += picconsts + '#define TRAINER_PIC_QZ_END %d\n' % (FIRST_PIC + len(pics))
    p = os.path.join(REPO, 'include/constants/trainers.h')
    new, full, s = between(p, block)
    if new is None:
        new = s.replace('#define TRAINER_ENCOUNTER_MUSIC_MALE', full + '\n#define TRAINER_ENCOUNTER_MUSIC_MALE', 1)
    open(p, 'w', encoding='utf-8').write(new)
    p = os.path.join(REPO, 'include/constants/qz_port.h')
    s = open(p, encoding='utf-8').read()
    s = re.sub(r'#define (TRAINER_QZ_\d+)\s+\S+\n', '', s)
    s = s.replace('// trainers (placeholders until the trainers are imported)\n',
                  '// trainers (tools/quetzal_port/gen_trainers.py)\n' + ids)
    s = re.sub(r'// trainers \(tools/quetzal_port/gen_trainers.py\)\n(#define TRAINER_QZ_\d+\s+\d+\n)*',
               lambda m: '// trainers (tools/quetzal_port/gen_trainers.py)\n' + ids, s)
    open(p, 'w', encoding='utf-8').write(s)
    # class names and money
    open(os.path.join(REPO, 'src/data/text/trainer_class_names_qz.h'), 'w').write(
        head + '#pragma once\n' + ''.join('    [TRAINER_CLASS_QZ_%s] = _("%s"), \\\n' % (CLASSES[k], CLASS_NAMES[k]) for k in CLASSES).join(
            ['#define QZ_TRAINER_CLASS_NAMES \\\n', '\n']) +
        '#define QZ_TRAINER_MONEY \\\n' + ''.join('    {TRAINER_CLASS_QZ_%s, %d}, \\\n' % (CLASSES[k], MONEY[k]) for k in CLASSES) + '\n')
    print('%d trainers (%d-%d), %d new pictures' % (len(trainers), FIRST_ID, FIRST_ID + len(trainers) - 1, len(pics)))
    for c in sorted(bad):
        print('  not in Kai:', c)


if __name__ == '__main__':
    main()
