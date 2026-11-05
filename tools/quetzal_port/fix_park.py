#!/usr/bin/env python3
# Run once (2026-11-05, audit case 101): NATIONAL PARK's people as Crystal's (NationalPark.asm): the PERSIAN by the
# QUICK CLAW teacher (Quetzal's man "with her by my side"), the mail mother (Quetzal's healer), the teacher walking by
# the sticker boys and the boy printing his POKéDEX (both missing), the item balls PARLYZ HEAL and TM28 DIG (Quetzal's
# MOON STONE and SOOTHE BELL).
import json, os
os.chdir(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

def sub(p, a, b):
    s = open(p).read()
    assert a in s, (p, a[:60])
    open(p, 'w').write(s.replace(a, b, 1))

if '#define OBJ_EVENT_GFX_MON_PERSIAN' not in open('include/constants/event_objects.h').read():
    sub('include/constants/event_objects.h', "#define OBJ_EVENT_GFX_MON_FEAROW   (OBJ_EVENT_GFX_MON_FIRST + 22)",
        "#define OBJ_EVENT_GFX_MON_FEAROW   (OBJ_EVENT_GFX_MON_FIRST + 22)\n"
        "#define OBJ_EVENT_GFX_MON_PERSIAN  (OBJ_EVENT_GFX_MON_FIRST + 53)")

P = 'Johto_NationalPark_'
mp = 'data/maps/Johto_NationalPark/map.json'
mj = json.load(open(mp))
objs = mj['object_events']
def at(x, y):
    return [o for o in objs if o['x'] == x and o['y'] == y][0]
# the man by the teacher: Crystal's PERSIAN
o = at(33, 42)
o['graphics_id'] = 'OBJ_EVENT_GFX_MON_PERSIAN'
o['script'] = P + 'EventScript_Persian'
# Quetzal's healer in the north: Crystal's mail mother (POKEFAN F, standing down)
o = at(17, 4)
o['graphics_id'] = 'OBJ_EVENT_GFX_POKEFAN_F'
o['movement_type'] = 'MOVEMENT_TYPE_FACE_DOWN'
o['script'] = P + 'EventScript_PokefanF'
def add(gfx, x, y, mv, rx, ry, script):
    if not any(o['script'] == script for o in objs):
        objs.append({'graphics_id': gfx, 'x': x, 'y': y, 'elevation': 0, 'movement_type': mv,
                     'movement_range_x': rx, 'movement_range_y': ry, 'trainer_type': 'TRAINER_TYPE_NONE',
                     'trainer_sight_or_berry_tree_id': '0', 'script': script, 'flag': '0'})
add('OBJ_EVENT_GFX_WOMAN_2', 16, 43, 'MOVEMENT_TYPE_WANDER_AROUND', 1, 2, P + 'EventScript_Teacher2')
add('OBJ_EVENT_GFX_GAMEBOY_KID', 26, 6, 'MOVEMENT_TYPE_FACE_DOWN', 0, 0, P + 'EventScript_GameboyKid')
json.dump(mj, open(mp, 'w'), indent=2)
open(mp, 'a').write('\n')

p = 'data/maps/Johto_NationalPark/scripts.inc'
s = open(p).read()
s = s.replace('\tsetorcopyvar VAR_0x8000, ITEM_MOON_STONE\n', '\tsetorcopyvar VAR_0x8000, ITEM_PARALYZE_HEAL\n', 1)
s = s.replace('\tsetorcopyvar VAR_0x8000, ITEM_SOOTHE_BELL\n', '\tsetorcopyvar VAR_0x8000, ITEM_TM28\n', 1)
if P + 'EventScript_Persian::' not in s:
    s += f"""
@ Moemon Kai: Crystal's NATIONAL PARK people (NationalPark.asm, docs/REGIONS_AUDIT.md 101)
{P}EventScript_Persian::
	lock
	faceplayer
	playmoncry SPECIES_PERSIAN, CRY_MODE_NORMAL
	msgbox {P}Text_Persian, MSGBOX_DEFAULT
	waitmoncry
	release
	end

{P}EventScript_PokefanF::
	msgbox {P}Text_PokefanF, MSGBOX_NPC
	end

{P}EventScript_Teacher2::
	msgbox {P}Text_Teacher2, MSGBOX_NPC
	end

{P}EventScript_GameboyKid::
	lock
	faceplayer
	msgbox {P}Text_GameboyKid, MSGBOX_DEFAULT
	closemessage
	applymovement VAR_LAST_TALKED, Johto_NationalPark_Movement_FaceDown
	waitmovement 0
	release
	end

{P}Movement_FaceDown:
	face_down
	step_end
"""
    for t in ['Persian', 'PokefanF', 'Teacher2', 'GameboyKid']:
        s += f'\n{P}Text_{t}::\n\t.string "x$"\n'
open(p, 'w').write(s)

lines = ['# case 101: NATIONAL PARK']
lines += [f'Johto_NationalPark {P}Text_{a} gsc NationalPark {b}' for a, b in
          [('Persian', 'NationalParkPersianText'), ('PokefanF', 'NationalParkPokefanFText'),
           ('Teacher2', 'NationalParkTeacher2Text'), ('GameboyKid', 'NationalParkGameboyKidText')]]
lst = 'tools/quetzal_port/restore_words.txt'
t = open(lst).read()
if lines[1] not in t:
    open(lst, 'a').write('\n'.join(lines) + '\n')
print('ok')
