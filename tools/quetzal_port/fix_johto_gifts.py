#!/usr/bin/env python3
"""JOHTO's gift POKéMON as Crystal's where Quetzal changed them (Crystal / HGSS > Quetzal: docs/REGIONS_TODO.md).

    python3 tools/quetzal_port/fix_johto_gifts.py [POKECRYSTAL_DIR]   (safe to run again; without the folder Crystal's
                                                                     maps are fetched from pret/pokecrystal)

- GOLDENROD, BILL's family's house: BILL gives Crystal's EEVEE (Lv. 20; Quetzal: a ROTOM "that moved into his
  computer"), Crystal's words.
- MT. MORTAR B1F: the KARATE KING, once beaten, gives Crystal's TYROGUE (Lv. 10; Quetzal: two "scrolls" for a KUBFU,
  NUGGETS in this game, behind a seven-badge gate), Crystal's words.
Each gift goes through the game's own givemon (party, else the PC; a full PC says so and the gift waits) and the
region's NUZLOCKE check (Quetzal_EventScript_8361A4D).
"""
import os
import re
import sys
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fix_johto_tms import REPO, RAW, crystal_text, gba_text   # noqa: E402

MARK = "@ Moemon Kai: Crystal's gift (tools/quetzal_port/fix_johto_gifts.py)"


def asm_of(name, crystal):
    if crystal:
        return open(os.path.join(crystal, 'maps', name + '.asm')).read()
    return urllib.request.urlopen(RAW % name).read().decode()


def words(asm, label):
    return gba_text([p.replace('<PLAYER>', '{PLAYER}') for p in crystal_text(asm, label)])


def set_text(s, label, text):
    t = re.search(re.escape(label) + r'::\n((?:\t\.string .*\n)+)', s)
    if not t:
        raise SystemExit('no ' + label)
    return s[:t.start(1)] + text + s[t.end(1):]


def give(species, level, done_flag, after):
    """the gift: NUZLOCKE check, givemon, Crystal's "received" (Quetzal_EventScript_840181E), the done flag"""
    return ('\tsetvar VAR_TEMP_1, %s\n\tcall Quetzal_EventScript_8361A4D\n\tbufferspeciesname 0, %s\n'
            '\tqz_givemon_ex %s, %d, ITEM_NONE, 1, 2, 3\n\tcall Quetzal_EventScript_840181E\n\tsetflag %s\n%s'
            % (species, species, species, level, done_flag, after))


def bill(crystal):
    path = os.path.join(REPO, 'data/maps/Johto_BillsFamilysHouse/scripts.inc')
    s = open(path).read()
    if MARK in s:
        return
    asm = asm_of('BillsFamilysHouse', crystal)
    p = 'Johto_BillsFamilysHouse_'
    a = s.index(p + 'EventScript_83D319D::\n')
    b = s.index(p + 'EventScript_83D31DC::\n')
    s = s[:a] + p + 'EventScript_83D319D::\n\t' + MARK + ' (Quetzal: ROTOM)\n' \
        + '\tmsgbox %sText_83D31E7, MSGBOX_YESNO\n\tgoto_if_eq VAR_RESULT, 0, %sEventScript_83D31DC\n' % (p, p) \
        + '\tmsgbox %sText_83D3251, MSGBOX_DEFAULT\n' % p \
        + give('SPECIES_EEVEE', 20, 'FLAG_QZ_0x103B',
               '\tmsgbox %sText_83D32D5, MSGBOX_DEFAULT\n\tclosemessage\n\trelease\n\tend\n\n' % p) \
        + s[b:]
    for label, crystal_label in (('83D31E7', 'BillTakeThisEeveeText'), ('83D3251', 'BillImCountingOnYouText'),
                                 ('83D32D5', 'BillEeveeMayEvolveText'), ('83D335B', 'BillNoEeveeText'),
                                 ('83D336F', 'BillPopWontWorkText')):
        s = set_text(s, p + 'Text_' + label, words(asm, crystal_label))
    open(path, 'w').write(s)
    print('BillsFamilysHouse: EEVEE')


def kiyo(crystal):
    path = os.path.join(REPO, 'data/maps/Johto_MountMortarB1F/scripts.inc')
    s = open(path).read()
    if MARK in s:
        return
    asm = asm_of('MountMortarB1F', crystal)
    p = 'Johto_MountMortarB1F_'
    a = s.index(p + 'EventScript_83E0004::\n')
    b = s.index(p + 'EventScript_83E00AB::\n')
    s = s[:a] + p + 'EventScript_83E0004::\n\t' + MARK + ' (Quetzal: two scrolls, seven badges first)\n' \
        + '\tlock\n\tfaceplayer\n\tgoto_if_set FLAG_QZ_0x10BA, %sEventScript_83E00A0\n' % p \
        + '\tgoto_if_set FLAG_QZ_0x10BB, %sEventScript_83E003D\n' % p \
        + '\tmsgbox %sText_83E0106, MSGBOX_DEFAULT\n' % p \
        + '\ttrainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4277, 0, %sText_83E0161\n' % p \
        + '\tsetflag FLAG_QZ_0x10BB\n\n' + p + 'EventScript_83E003D::\n' \
        + '\tmsgbox %sText_83E0178, MSGBOX_DEFAULT\n' % p \
        + give('SPECIES_TYROGUE', 10, 'FLAG_QZ_0x10BA', '\n') \
        + p + 'EventScript_83E00A0::\n\tmsgbox %sText_83E0221, MSGBOX_DEFAULT\n\tclosemessage\n\trelease\n\tend\n\n' % p \
        + s[b:]
    # the seven-badge gate's script and words go: nothing calls them now
    s = re.sub(re.escape(p) + r'EventScript_83E00AE::\n(?:\t[^\n]*\n)+\n', '', s)
    s = re.sub(re.escape(p) + r'Text_83E00B9::\n(?:\t\.string [^\n]*\n)+\n', '', s)
    for label, crystal_label in (('83E0106', 'MountMortarB1FKiyoIntroText'), ('83E0161', 'MountMortarB1FKiyoWinText'),
                                 ('83E0178', 'MountMortarB1FTyrogueRewardText'),
                                 ('83E0221', 'MountMortarB1FKiyoGotTyrogueText')):
        s = set_text(s, p + 'Text_' + label, words(asm, crystal_label))
    open(path, 'w').write(s)
    print('MountMortarB1F: TYROGUE')


if __name__ == '__main__':
    crystal = sys.argv[1] if len(sys.argv) > 1 else None
    bill(crystal)
    kiyo(crystal)
