#!/usr/bin/env python3
"""Platinum's BATTLE FRONTIER trainers and their POKéMON sets (res/trainers/frontier: 315 trainers, 951 sets), the
BATTLE TOWER's (src/sinnoh/s3d_tower.c) and the other facilities' opponents, as the game's tables:
src/data/sinnoh_frontier.h. Their names and lines are Platinum's words: the text pack's frontier bank
(tools/sinnoh/build_text.py frontier_bank()), never here.

    python3 -I tools/sinnoh/gen_frontier.py POKEPLATINUM_DIR     (run from the repository)

Sets: species, moves, held item, nature, the stats given EVs (bit 0 HP, 1 ATTACK, 2 DEFENSE, 3 SPEED, 4 SP. ATK,
5 SP. DEF, Platinum's order). Trainers, in Platinum's FRONTIER_TRAINER_* order: the game's class and picture for it
(gen_trainers.py CLASSES, else FRONTIER_CLASSES below), Platinum's battle picture, female, and its sets (a slice of
sSinnohFrontierSetLists).
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_trainers  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EV_ORDER = ['hp', 'attack', 'defense', 'speed', 'special_attack', 'special_defense']
# the frontier's own classes (the brains, the BATTLE SALON's partners, the rest gen_trainers.py lacks): (the game's
# class, picture, female); the picture shown is Platinum's own when the graphics pack has it (SINNOH_PIC_*)
FRONTIER_CLASSES = {
    'TOWER_TYCOON': ('S:TOWER_TYCOON', 'QZ_100', 0), 'FACTORY_HEAD': ('S:FACTORY_HEAD', 'QZ_100', 0),
    'HALL_MATRON': ('S:HALL_MATRON', 'QZ_216', 1), 'CASTLE_VALET': ('S:CASTLE_VALET', 'QZ_216', 1),
    'ARCADE_STAR': ('S:ARCADE_STAR', 'QZ_216', 1),
    'TRAINER_BUCK': ('PKMN_TRAINER_1', 'QZ_100', 0), 'TRAINER_CHERYL': ('PKMN_TRAINER_1', 'COOLTRAINER_F', 1),
    'TRAINER_MARLEY': ('PKMN_TRAINER_1', 'COOLTRAINER_F', 1), 'TRAINER_MIRA': ('PKMN_TRAINER_1', 'PSYCHIC_F', 1),
    'TRAINER_RILEY': ('PKMN_TRAINER_1', 'BLACK_BELT', 0),
    'REPORTER': ('INTERVIEWER', 'INTERVIEWER', 1), 'IDOL': ('S:IDOL', 'LASS', 1), 'CLOWN': ('S:CLOWN', 'QZ_220', 0),
    'WAITER': ('S:WAITER', 'QZ_220', 0),
}


def names(path, prefix):
    out = []
    for line in open(path, encoding='utf-8'):
        n = line.strip()
        if n.startswith(prefix):
            out.append(n[len(prefix):].lower())
    return out


def main():
    root = sys.argv[1]
    fdir = os.path.join(root, 'res/trainers/frontier')
    set_names = [n.strip() for n in open(os.path.join(fdir, 'frontier_pokemon.order'), encoding='utf-8') if n.strip()]
    known = set()
    for h in ('include/constants/species.h', 'include/constants/moves.h', 'include/constants/items.h',
              'include/constants/trainers.h'):
        known |= set(re.findall(r'#define (\w+)\s', open(os.path.join(ROOT, h)).read()))
    bad = set()

    # (the frontier's spellings of names Platinum's trainers don't use)
    rename = dict(gen_trainers.RENAME, ITEM_BLACKGLASSES='ITEM_BLACK_GLASSES', ITEM_BRIGHTPOWDER='ITEM_BRIGHT_POWDER',
                  ITEM_DEEPSEASCALE='ITEM_DEEP_SEA_SCALE', ITEM_NEVERMELTICE='ITEM_NEVER_MELT_ICE',
                  ITEM_SILVERPOWDER='ITEM_SILVER_POWDER', ITEM_TWISTEDSPOON='ITEM_TWISTED_SPOON',
                  MOVE_SELFDESTRUCT='MOVE_SELF_DESTRUCT')

    def c(name):
        name = rename.get(name, name)
        if name not in known:
            bad.add(name)
        return name

    sets = []
    for n in set_names:
        p = os.path.join(fdir, 'pokemon', n + '.json')
        if not os.path.exists(p):
            sets.append('    {SPECIES_NONE},   // %s' % n)
            continue
        d = json.load(open(p, encoding='utf-8'))
        mv = [c(m) for m in d['moves']][:4] + ['MOVE_NONE'] * (4 - len(d['moves']))
        ev = sum(1 << i for i, k in enumerate(EV_ORDER) if d['evFlags'].get(k))
        sets.append('    {%s, {%s}, %s, %s, 0x%02X},   // %s' % (c(d['species']), ', '.join(mv),
                                                              c(d.get('item') or 'ITEM_NONE'), d['nature'], ev, n))
    index = {n: i for i, n in enumerate(set_names)}
    have_pics = set(re.findall(r'#define SINNOH_PIC_(\w+)\s+\d', open(os.path.join(ROOT, 'include/constants/sinnoh_gfx.h')).read()))
    trainers, lists = [], []
    for name in names(os.path.join(root, 'generated/frontier_trainers.txt'), 'FRONTIER_TRAINER_'):
        p = os.path.join(fdir, 'data', name + '.json')
        if not os.path.exists(p):
            trainers.append('    {0},   // %s (none)' % name)
            continue
        d = json.load(open(p, encoding='utf-8'))
        pclass = d['class'][len('TRAINER_CLASS_'):]
        if pclass in gen_trainers.CLASSES:
            ours, pic, _music, female = gen_trainers.CLASSES[pclass]
        else:
            ours, pic, female = FRONTIER_CLASSES.get(pclass, ('PKMN_TRAINER_1', 'BRENDAN', 0))
        cls = 'TRAINER_CLASS_SINNOH_' + ours[2:] if ours.startswith('S:') else c('TRAINER_CLASS_' + ours)
        ids = [index[s] for s in d.get('availableSets', []) if s in index]
        trainers.append('    {%s, %s, %s, %d, %d, %d},   // %s' % (
            cls if cls in known or cls.startswith('TRAINER_CLASS_SINNOH_') else 'TRAINER_CLASS_PKMN_TRAINER_1',
            c('TRAINER_PIC_' + pic), 'SINNOH_PIC_' + pclass if pclass in have_pics else 'SINNOH_PIC_NONE', female,
            len(lists), len(ids), name))
        lists += ids
    bad = {b for b in bad if not b.startswith('TRAINER_CLASS_SINNOH_')}
    if bad:
        sys.exit('gen_frontier: not in this game: %s' % ' '.join(sorted(bad)))
    out = ('// Generated by tools/sinnoh/gen_frontier.py from pret/pokeplatinum res/trainers/frontier. Do not edit.\n'
           '// Platinum\'s BATTLE FRONTIER trainers and sets (src/sinnoh/s3d_tower.c); names and lines in the text pack\n'
           '#define SINNOH_FRONTIER_SETS %d\n#define SINNOH_FRONTIER_TRAINERS %d\n\n' % (len(sets), len(trainers)) +
           'static const struct SinnohFrontierSet sSinnohFrontierSets[SINNOH_FRONTIER_SETS] = {\n' + '\n'.join(sets) +
           '\n};\n\nstatic const struct SinnohFrontierTrainer sSinnohFrontierTrainers[SINNOH_FRONTIER_TRAINERS] = {\n' +
           '\n'.join(trainers) + '\n};\n\nstatic const u16 sSinnohFrontierSetLists[] = {\n' +
           ''.join('    %s,\n' % ', '.join(str(x) for x in lists[i:i + 16]) for i in range(0, len(lists), 16)) + '};\n')
    open(os.path.join(ROOT, 'src/data/sinnoh_frontier.h'), 'w').write(out)
    print('src/data/sinnoh_frontier.h: %d sets, %d trainers, %d set entries' % (len(sets), len(trainers), len(lists)))


if __name__ == '__main__':
    main()
