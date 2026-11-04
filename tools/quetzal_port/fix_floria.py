#!/usr/bin/env python3
# NOT RUN YET (written 2026-11-04, paused before testing): see docs/AUDIT_HANDOFF.md "Next steps". Run once from the
# repo root, then restore_words.py, build, and test (a scenario talking to FLORIA by the tree, then the FLOWER SHOP).
# case 100: ROUTE 36's FLORIA by the odd tree, the FLOWER SHOP's teacher on Crystal's conditions, the lass and gramps
import json, re, os
os.chdir(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

def sub(p, a, b):
    s = open(p).read()
    assert a in s, (p, a[:60])
    open(p, 'w').write(s.replace(a, b, 1))

# flags
sub('include/constants/kai_regions.h',
    "#define FLAG_JOHTO_HIDE_GOLDENROD_TUTOR 0x153D",
    "#define FLAG_JOHTO_MET_FLORIA         0x1542   // Crystal's EVENT_MET_FLORIA (ROUTE 36, case 100)\n"
    "#define FLAG_JOHTO_TALKED_FLORIA      0x1543   // Crystal's EVENT_TALKED_TO_FLORIA_AT_FLOWER_SHOP\n"
    "#define FLAG_JOHTO_HIDE_FLORIA_ROUTE  0x1544   // FLORIA by the odd tree: there until met (MP_JohtoWeekdayPeople)\n"
    "#define FLAG_JOHTO_HIDE_FLORIA_SHOP   0x1545   // FLORIA in the FLOWER SHOP: there once met\n"
    "#define FLAG_JOHTO_HIDE_GOLDENROD_TUTOR 0x153D")
sub('src/mp/mp_region.c', """    // GOLDENROD UNDERGROUND (Crystal's""", """    // ROUTE 36's FLORIA (Crystal's GoldenrodCityFlypointAndFloriaCallback, EVENT_FLORIA_AT_SUDOWOODO /
    // _AT_FLOWER_SHOP): by the odd tree until met, then in the FLOWER SHOP (a save that already has the SQUIRTBOTTLE or
    // fought the tree has met her)
    {
        bool8 met = FlagGet(FLAG_JOHTO_MET_FLORIA) || FlagGet(FLAG_QZ_0x1099) || FlagGet(FLAG_QZ_0x102E);
        if (met)
        {
            FlagSet(FLAG_JOHTO_HIDE_FLORIA_ROUTE);
            FlagClear(FLAG_JOHTO_HIDE_FLORIA_SHOP);
        }
        else
        {
            FlagClear(FLAG_JOHTO_HIDE_FLORIA_ROUTE);
            FlagSet(FLAG_JOHTO_HIDE_FLORIA_SHOP);
        }
    }
    // GOLDENROD UNDERGROUND (Crystal's""")

# ROUTE 36: FLORIA by the tree, the lass's lines
P = 'Johto_Route36_'
mp = 'data/maps/Johto_Route36/map.json'
mj = json.load(open(mp))
if not any(o['script'] == P + 'EventScript_Floria' for o in mj['object_events']):
    mj['object_events'].append({'graphics_id': 'OBJ_EVENT_GFX_QZ_6', 'x': 39, 'y': 21, 'elevation': 0,
                                'movement_type': 'MOVEMENT_TYPE_FACE_UP', 'movement_range_x': 0,
                                'movement_range_y': 0, 'trainer_type': 'TRAINER_TYPE_NONE',
                                'trainer_sight_or_berry_tree_id': '0', 'script': P + 'EventScript_Floria',
                                'flag': 'FLAG_JOHTO_HIDE_FLORIA_ROUTE'})
lass = mj['object_events'][0]['script']
json.dump(mj, open(mp, 'w'), indent=2)
open(mp, 'a').write('\n')
p = 'data/maps/Johto_Route36/scripts.inc'
s = open(p).read()
i = s.index(lass + '::\n')
j = s.index('\tend\n', i) + 5
s = s[:i] + f"""{lass}::
	@ Moemon Kai: Crystal's lass (Route36LassScript, docs/REGIONS_AUDIT.md 100)
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102E, {P}EventScript_LassCleared
	msgbox {P}Text_Lass, MSGBOX_DEFAULT
	release
	end
{P}EventScript_LassCleared::
	msgbox {P}Text_LassCleared, MSGBOX_DEFAULT
	release
	end
""" + s[j:]
if P + 'EventScript_Floria::' not in s:
    s += f"""
@ Moemon Kai: Crystal's FLORIA by the odd tree (Route36FloriaScript, docs/REGIONS_AUDIT.md 100): met once, she goes to
@ ask her sister for the water bottle (then she is in the FLOWER SHOP: MP_JohtoWeekdayPeople)
{P}EventScript_Floria::
	lock
	faceplayer
	goto_if_set FLAG_JOHTO_TALKED_FLORIA, {P}EventScript_FloriaAgain
	setflag FLAG_JOHTO_MET_FLORIA
	msgbox {P}Text_Floria1, MSGBOX_DEFAULT
	closemessage
	goto_if_eq VAR_FACING, DIR_NORTH, {P}EventScript_FloriaUp
	applymovement VAR_LAST_TALKED, {P}Movement_Floria1
	goto {P}EventScript_FloriaGone
{P}EventScript_FloriaUp::
	applymovement VAR_LAST_TALKED, {P}Movement_Floria2
{P}EventScript_FloriaGone::
	waitmovement 0
	removeobject VAR_LAST_TALKED
	setflag FLAG_JOHTO_HIDE_FLORIA_ROUTE
	release
	end
{P}EventScript_FloriaAgain::
	msgbox {P}Text_Floria2, MSGBOX_DEFAULT
	release
	end

{P}Movement_Floria1:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

{P}Movement_Floria2:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end
"""
for t in ['Lass', 'LassCleared', 'Floria1', 'Floria2']:
    if f'{P}Text_{t}::' not in s:
        s += f'\n{P}Text_{t}::\n\t.string "x$"\n'
open(p, 'w').write(s)

# FLOWER SHOP: Crystal's teacher and FLORIA
F = 'Johto_GoldenrodFlowerShop_'
p = 'data/maps/Johto_GoldenrodFlowerShop/scripts.inc'
s = open(p).read()
i = s.index(F + 'EventScript_83D4C26::\n')
j = s.index('\n@ Johto_GoldenrodFlowerShop object 4', i)
s = s[:i] + f"""{F}EventScript_83D4C26::
	@ Moemon Kai: Crystal's teacher (FlowerShopTeacherScript, docs/REGIONS_AUDIT.md 100): the SQUIRTBOTTLE once FLORIA
	@ was met by the tree, talked to here, and the PLAINBADGE won
	lock
	goto_if_set FLAG_QZ_0x102E, {F}EventScript_Lalala
	faceplayer
	goto_if_set FLAG_QZ_0x1099, {F}EventScript_TeacherGot
	goto_if_unset FLAG_JOHTO_MET_FLORIA, {F}EventScript_TeacherNotMet
	goto_if_unset FLAG_JOHTO_TALKED_FLORIA, {F}EventScript_Lalala
	goto_if_unset FLAG_QZ_0x11B4, {F}EventScript_TeacherNoBadge
	msgbox {F}Text_TeacherHere, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_SQUIRTBOTTLE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	setflag FLAG_QZ_0x1099
	release
	end
{F}EventScript_Lalala::
	msgbox {F}Text_TeacherLalala, MSGBOX_DEFAULT
	release
	end
{F}EventScript_TeacherGot::
	msgbox {F}Text_TeacherDanger, MSGBOX_DEFAULT
	release
	end
{F}EventScript_TeacherNoBadge::
	msgbox {F}Text_TeacherBorrow, MSGBOX_DEFAULT
	release
	end
{F}EventScript_TeacherNotMet::
	msgbox {F}Text_TeacherSister, MSGBOX_DEFAULT
	release
	end
""" + s[j:]
fl = json.load(open('data/maps/Johto_GoldenrodFlowerShop/map.json'))
fl['object_events'][3]['flag'] = 'FLAG_JOHTO_HIDE_FLORIA_SHOP'
floria = fl['object_events'][3]['script']
json.dump(fl, open('data/maps/Johto_GoldenrodFlowerShop/map.json', 'w'), indent=2)
open('data/maps/Johto_GoldenrodFlowerShop/map.json', 'a').write('\n')
i = s.index(floria + '::\n')
j = s.index('\n@ ', i + 1) if '\n@ ' in s[i + 1:] else len(s)
k = re.search(r'^\w+_Text_\w+::', s[i:], re.M)
if k:
    j = min(j, i + k.start())
s = s[:i] + f"""{floria}::
	@ Moemon Kai: Crystal's FLORIA in the shop (FlowerShopFloriaScript)
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102E, {F}EventScript_FloriaFought
	goto_if_set FLAG_QZ_0x1099, {F}EventScript_FloriaGot
	msgbox {F}Text_FloriaWonder, MSGBOX_DEFAULT
	setflag FLAG_JOHTO_TALKED_FLORIA
	release
	end
{F}EventScript_FloriaGot::
	msgbox {F}Text_FloriaWhitney, MSGBOX_DEFAULT
	release
	end
{F}EventScript_FloriaFought::
	msgbox {F}Text_FloriaWasAMon, MSGBOX_DEFAULT
	release
	end
""" + s[j:]
for t in ['TeacherHere', 'TeacherLalala', 'TeacherDanger', 'TeacherBorrow', 'TeacherSister', 'FloriaWonder',
          'FloriaWhitney', 'FloriaWasAMon']:
    if f'{F}Text_{t}::' not in s:
        s += f'\n{F}Text_{t}::\n\t.string "x$"\n'
open(p, 'w').write(s)

# the RUINS gate's gramps (Quetzal's sign-reading man at 3,5: Crystal's gramps words)
lines = ['# case 100: ROUTE 36 FLORIA, the lass, the FLOWER SHOP teacher and FLORIA, the RUINS gate gramps']
lines += [f'Johto_Route36 {P}Text_{a} gsc Route36 {b}' for a, b in
          [('Lass', 'Route36LassText'), ('LassCleared', 'Route36LassText_ClearedSudowoodo'), ('Floria1', 'FloriaText1'),
           ('Floria2', 'FloriaText2')]]
lines += [f'Johto_GoldenrodFlowerShop {F}Text_{a} gsc GoldenrodFlowerShop {b}' for a, b in
          [('TeacherHere', 'GoldenrodFlowerShopTeacherHeresTheSquirtbottleText'),
           ('TeacherLalala', 'GoldenrodFlowerShopTeacherLalalaHavePlentyOfWaterText'),
           ('TeacherDanger', 'GoldenrodFlowerShopTeacherDontDoAnythingDangerousText'),
           ('TeacherBorrow', 'GoldenrodFlowerShopTeacherAskWantToBorrowWaterBottleText'),
           ('TeacherSister', 'GoldenrodFlowerShopTeacherMySisterWentToSeeWigglyTreeRoute36Text'),
           ('FloriaWonder', 'GoldenrodFlowerShopFloriaWonderIfSisWillLendWaterBottleText'),
           ('FloriaWhitney', 'GoldenrodFlowerShopFloriaYouBeatWhitneyText'),
           ('FloriaWasAMon', 'GoldenrodFlowerShopFloriaItReallyWasAMonText')]]
lines += ['Johto_Route36RuinsOfAlphGate Johto_Route36RuinsOfAlphGate_Text_83D22D6 gsc Route36RuinsOfAlphGate Route36RuinsOfAlphGateGrampsText']
lst = 'tools/quetzal_port/restore_words.txt'
t = open(lst).read()
if lines[1] not in t:
    open(lst, 'a').write('\n'.join(lines) + '\n')
print('ok')
