#!/usr/bin/env python3
"""JOHTO's BATTLE TOWER as Crystal's (audit case 37): Crystal's 70 BATTLE TOWER trainers, its ten level groups of 21
POKéMON and its trainers' words (pret/pokecrystal data/battle_tower/, data/trainers/genders.asm,
engine/events/battle_tower/trainer_text.asm) as src/data/johto_battle_tower.h, read by src/mp/mp_johto_tower.c.

    POKECRYSTAL_DIR=/tmp/claude-0/frlg/pokecrystal python3 -I tools/quetzal_port/gen_battle_tower_gsc.py

Crystal's trainer classes become the game's through a JOHTO trainer of the same Crystal class (gen_original_teams.py's
pairs): its trainer class, picture, encounter music and the overworld sprite it stands as. The POKéMON keep Crystal's
moves, items (as gen_original_teams.py names them), DVs (doubled, plus one, as Gen 3 IVs; HP's from the other four, as
Crystal works it out) and stat experience (each stat's EV: its square root, rounded up, at most 255, as Crystal's
stat bonus reads it); their nicknames are their species' names, as Crystal shows them.
"""
import glob
import json
import math
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_original_teams as orig   # noqa: E402
import crystal_words                # noqa: E402

REPO = orig.REPO
PC = os.environ.get('POKECRYSTAL_DIR', '/tmp/claude-0/frlg/pokecrystal')
OUT = os.path.join(REPO, 'src/data/johto_battle_tower.h')


def c_text(lines):
    """crystal_words' .string lines as one C string"""
    out = ''
    for ln in lines.strip().split('\n'):
        m = re.match(r'\s*\.string "(.*)"$', ln)
        if m:
            out += m.group(1)
    if out.endswith('$'):
        out = out[:-1]
    return '_("%s")' % out


def main():
    rd = lambda p: orig.read(os.path.join(PC, p))
    classes_src = rd('data/battle_tower/classes.asm')
    trainers = re.findall(r'^\s+bt_trainer (\w+),\s*"([^"]*)"', classes_src, re.M)
    assert len(trainers) == 70, len(trainers)
    # the Crystal trainer classes in order (their constants count from 1)
    tconst = rd('constants/trainer_constants.asm')
    cls_order = re.findall(r'^\s+trainerclass\s+(\w+)', tconst, re.M)
    genders = re.findall(r'^\s+db (MALE|FEMALE)\s*;\s*(\w+)', rd('data/trainers/genders.asm'), re.M)
    gender = {c: g for g, c in genders}
    # Crystal class -> a JOHTO trainer of that class
    party = orig.crystal(PC)
    names = orig.qz_names()
    pairs = orig.johto_pairs(PC, names, party)
    by_class = {}
    for qz, (mp, cls, tid) in sorted(pairs.items(), key=lambda kv: orig.num_of(kv[0])):
        by_class.setdefault(cls, []).append(qz)
    qz_src = orig.read(os.path.join(REPO, 'src/data/trainers_qz.h'))
    qz = {}
    for m in re.finditer(r'\[(TRAINER_QZ_\d+)\] =\s*\{(.*?)\n    \},', qz_src, re.S):
        b = m.group(2)
        qz[m.group(1)] = (re.search(r'trainerClass = (\w+)', b).group(1), re.search(r'trainerPic = (\w+)', b).group(1),
                          re.search(r'encounterMusic_gender = ([^,]+),', b).group(1))
    gfx = {}
    for d in glob.glob(os.path.join(REPO, 'data/maps/Johto_*/map.json')):
        lt = orig.labels_trainers(orig.read(os.path.join(os.path.dirname(d), 'scripts.inc')))
        for o in json.load(open(d))['object_events']:
            tr = [t for k, t in lt.get(o['script'], []) if 'rematch' not in k]
            if tr:
                gfx.setdefault(tr[0], []).append(o['graphics_id'])
    # Crystal's classes the tower uses that JOHTO's maps don't have: KANTO's (FireRed's class, gen_trainer_tower.py's way:
    # GUITARIST as FireRed's ROCKER under the game's own GUITARIST class), TEACHER as a BEAUTY (no such class here)
    STAND_IN = {'TEACHER': ['BEAUTY']}
    KANTO = {'BIKER': ('TRAINER_CLASS_BIKER', None), 'GUITARIST': ('TRAINER_CLASS_ROCKER', 'TRAINER_CLASS_GUITARIST')}
    fr, _ = orig.firered()
    kp = orig.kanto_pairs(names)
    by_fr = {ft: q for q, (mp_, ft) in kp.items()}
    from fix_move_tutors import fetch
    trainers_h = fetch('src/data/trainers.h').replace('\r', '')
    fr_class = dict(re.findall(r'\[(TRAINER_\w+)\] = \{\s*\.trainerClass = (TRAINER_CLASS_\w+)', trainers_h))
    for d in glob.glob(os.path.join(REPO, 'data/maps/Kanto_*/map.json')):
        lt = orig.labels_trainers(orig.read(os.path.join(os.path.dirname(d), 'scripts.inc')))
        for o in json.load(open(d))['object_events']:
            tr = [t for k, t in lt.get(o['script'], []) if 'rematch' not in k]
            if tr:
                gfx.setdefault(tr[0], []).append(o['graphics_id'])
    used = []
    for cls, _ in trainers:
        if cls not in used:
            used.append(cls)
    class_rows = []
    for cls in used:
        cands = []
        for c in STAND_IN.get(cls, [cls]):
            cands = [q for q in by_class.get(c, []) if q in gfx and q in qz]
            if cands:
                break
        override = None
        if not cands and cls in KANTO:
            frc, override = KANTO[cls]
            cands = [by_fr[t] for t, k in fr_class.items() if k == frc and t in by_fr and by_fr[t] in gfx and by_fr[t] in qz]
        if not cands:
            raise SystemExit('no JOHTO trainer of Crystal class %s' % cls)
        q = cands[0]
        tc, pic, music = qz[q]
        tc = override or tc
        class_rows.append((cls, tc, pic, music, gfx[q][0], 1 if gender.get(cls) == 'FEMALE' else 0, q))
    out = ['// Generated by tools/quetzal_port/gen_battle_tower_gsc.py from pret/pokecrystal (data/battle_tower/,',
           '// data/trainers/genders.asm, engine/events/battle_tower/trainer_text.asm). Do not edit. JOHTO\'s BATTLE TOWER',
           '// (src/mp/mp_johto_tower.c; audit case 37).', '']
    out.append('enum {%s, JBT_CLASS_COUNT};' % ', '.join('JBT_CLASS_' + c for c in used))
    out.append('// Crystal\'s class: the game\'s trainer class, picture, encounter music, overworld sprite, Crystal\'s gender')
    out.append('static const struct JbtClass { u8 trainerClass; u16 trainerPic; u8 music; u16 objGfx; u8 female; } sJbtClasses[] = {')
    for cls, tc, pic, music, g, fem, q in class_rows:
        out.append('    [JBT_CLASS_%s] = {%s, %s, (%s) & 0x7F, %s, %d},   // as %s' % (cls, tc, pic, music, g, fem, q))
    out.append('};')
    out.append('')
    out.append('static const struct JbtTrainer { u8 name[11]; u8 cls; } sJbtTrainers[70] = {')
    for cls, nm in trainers:
        out.append('    {_("%s"), JBT_CLASS_%s},' % (nm, cls))
    out.append('};')
    out.append('')
    # the POKéMON
    parties = rd('data/battle_tower/parties.asm')
    groups = re.split(r'^; BattleTowerMons group (\d+)\s*$', parties, flags=re.M)
    out.append('static const struct BattleTowerPokemon sJbtMons[10][21] = {')
    total = 0
    for gi in range(1, len(groups), 2):
        body = groups[gi + 1]
        mons = re.split(r'\n\s*\n(?=\s*db [A-Z])', body.strip())
        mons = [m for m in mons if re.search(r'^\s*db [A-Z_]+\s*$', m, re.M)]
        assert len(mons) == 21, (groups[gi], len(mons))
        out.append('    { // L:%d' % (int(groups[gi]) * 10))
        for m in mons:
            lines = [l.split(';')[0].strip() for l in m.split('\n') if l.strip() and not l.strip().startswith(';')]
            sp = lines[0].split()[1]
            item = lines[1].split()[1]
            moves = [x.strip() for x in lines[2][3:].split(',')]
            statexp = [int(re.search(r'bigdw (\d+)', l).group(1)) for l in lines if l.startswith('bigdw')][:5]
            dvs = [int(x) for x in re.search(r'dn ([\d, ]+)', m).group(1).split(',')]
            happy = int(re.findall(r'^\s*db (\d+)\s*(?:;\s*Happiness)', m, re.M)[0]) if re.search(r'Happiness', m) else 255
            lvl = int(re.search(r'^\s*db (\d+)\s*;\s*Level', m, re.M).group(1))
            atk, de, spd, spc = dvs
            hp = ((atk & 1) << 3) | ((de & 1) << 2) | ((spd & 1) << 1) | (spc & 1)
            iv = lambda d: d * 2 + 1
            ev = lambda s: min(255, int(math.ceil(math.sqrt(s))))
            species = orig.kai('SPECIES_' + orig.CR_SPECIES.get(sp, sp), 'SPECIES_NONE')
            itemc = 'ITEM_NONE' if item == 'NO_ITEM' else orig.kai('ITEM_' + orig.CR_ITEMS.get(item, item), 'ITEM_NONE')
            mv = ['MOVE_NONE' if x in ('NO_MOVE', '0') else orig.kai('MOVE_' + orig.CR_MOVES.get(x, x), 'MOVE_NONE')
                  for x in moves]
            mv += ['MOVE_NONE'] * (4 - len(mv))
            out.append('        {.species = %s, .heldItem = %s, .moves = {%s}, .level = %d, .hpEV = %d, .attackEV = %d, '
                       '.defenseEV = %d, .speedEV = %d, .spAttackEV = %d, .spDefenseEV = %d, .hpIV = %d, .attackIV = %d, '
                       '.defenseIV = %d, .speedIV = %d, .spAttackIV = %d, .spDefenseIV = %d, .personality = 0xC8, '
                       '.friendship = %d},'
                       % (species, itemc, ', '.join(mv), lvl, ev(statexp[0]), ev(statexp[1]), ev(statexp[2]), ev(statexp[3]),
                          ev(statexp[4]), ev(statexp[4]), iv(hp), iv(atk), iv(de), iv(spd), iv(spc), iv(spc), happy))
            total += 1
        out.append('    },')
    out.append('};')
    out.append('')
    # the trainers' words: 25 sets for men, 15 for women (greeting, the player lost, the player won)
    eng = rd('engine/events/battle_tower/trainer_text.asm')
    for sex, lab in (('M', 'BTMaleTrainerTexts'), ('F', 'BTFemaleTrainerTexts')):
        blk = eng[eng.index(lab + ':'):]
        lists = []
        for part in ('Greetings', 'PlayerLost', 'PlayerWon'):
            seg = blk[blk.index('.%s:' % part):]
            seg = seg[seg.index('\n') + 1:]
            labels = []
            for ln in seg.split('\n'):
                m = re.match(r'\s*dw (\w+)', ln)
                if not m:
                    break
                labels.append(m.group(1))
            lists.append(labels)
        n = len(lists[0])
        out.append('static const u8 *const sJbtTexts%s[%d][3] = {' % (sex, n))
        for k in range(n):
            row = []
            for j in range(3):
                lab_ = '_' + lists[j][k]
                row.append(c_text(crystal_words.words('data/battle_tower/trainer_text', lab_, 'X')))
            out.append('    {%s},' % ', '.join('(const u8 []) ' + r for r in row))
        out.append('};')
        out.append('')
    open(OUT, 'w').write('\n'.join(out) + '\n')
    print('%d classes, %d trainers, %d POKeMON' % (len(class_rows), len(trainers), total))
    if orig.warn:
        print('unknown, dropped:', ' '.join(sorted(orig.warn)))


if __name__ == '__main__':
    main()
