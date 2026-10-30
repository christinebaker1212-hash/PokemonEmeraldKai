Johto_VioletCity_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_VioletCity_OnFrame_83FE8B8
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_VioletCity_EventScript_83FE8CA
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_VioletCity_OnFrame_83FE8B8:
	map_script_2 VAR_QZ_0x5012, 1, Johto_VioletCity_EventScript_83FEA06
	map_script_2 VAR_QZ_0x5008, 3, Johto_VioletCity_EventScript_83FE9F0
	.2byte 0

Johto_VioletCity_EventScript_83FE8CA::
	special QzJohtoTravelingEventCheck
	end

@ Johto_VioletCity object 4 (local id 4, gfx 0x3EB7) at 18,35
Johto_VioletCity_EventScript_83FE8D7::
	lock
	faceplayer
	applymovement 4, Johto_VioletCity_Movement_83FEAA6
	waitmovement 0
	faceplayer
	msgbox Johto_VioletCity_Text_83FEBAB, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_VioletCity_EventScript_83FE902
	msgbox Johto_VioletCity_Text_83FEBE3, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_VioletCity_EventScript_83FE902::
	msgbox Johto_VioletCity_Text_83FEC01, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_QZ_647, 0
	copyvar VAR_0x8000, VAR_FACING
	goto_if_eq VAR_0x8000, 2, Johto_VioletCity_EventScript_83FE92C
	goto_if_eq VAR_0x8000, 3, Johto_VioletCity_EventScript_83FE951
	release
	end

Johto_VioletCity_EventScript_83FE92C::
	applymovement 4, Johto_VioletCity_Movement_83FEABF
	waitmovement 0
	applymovement 4, Johto_VioletCity_Movement_83FEAA6
	applymovement LOCALID_PLAYER, Johto_VioletCity_Movement_83FEAC5
	waitmovement 0
	waitmovement 0
	goto Johto_VioletCity_EventScript_83FE96C

Johto_VioletCity_EventScript_83FE951::
	applymovement 4, Johto_VioletCity_Movement_83FEAB4
	applymovement LOCALID_PLAYER, Johto_VioletCity_Movement_83FEAB9
	waitmovement 0
	waitmovement 0
	goto Johto_VioletCity_EventScript_83FE96C

Johto_VioletCity_EventScript_83FE96C::
	applymovement 4, Johto_VioletCity_Movement_83FEAC8
	applymovement LOCALID_PLAYER, Johto_VioletCity_Movement_83FEAC8
	waitmovement 0
	waitmovement 0
	applymovement 4, Johto_VioletCity_Movement_83FEAA6
	waitmovement 0
	turnobject 4, 3
	msgbox Johto_VioletCity_Text_83FEC2E, MSGBOX_DEFAULT
	closemessage
	applymovement 4, Johto_VioletCity_Movement_83FEAF2
	delay 0x14
	opendoor 0x27, 0x24
	waitdooranim
	waitmovement 0
	setflag FLAG_QZ_0x1025
	removeobject 4
	closedoor 0x27, 0x24
	waitdooranim
	clearflag FLAG_QZ_0x1026
	fadedefaultbgm
	release
	end

@ Johto_VioletCity object 5 (local id 5, gfx 0x23) at 10,27
Johto_VioletCity_EventScript_83FE9BC::
	lock
	faceplayer
	msgbox Johto_VioletCity_Text_83FEDBD, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletCity object 2 (local id 2, gfx 0x3E87) at 28,25
Johto_VioletCity_EventScript_83FE9C9::
	lock
	faceplayer
	msgbox Johto_VioletCity_Text_83FECEA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletCity object 3 (local id 3, gfx 0x3EA7) at 28,36
Johto_VioletCity_EventScript_83FE9D6::
	lock
	faceplayer
	msgbox Johto_VioletCity_Text_83FED44, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletCity object 1 (local id 1, gfx 0x14) at 32,48
Johto_VioletCity_EventScript_83FE9E3::
	lock
	faceplayer
	msgbox Johto_VioletCity_Text_83FEC5D, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletCity trigger at 22,37 (VAR_QZ_0x5008 == 3)
@ Johto_VioletCity trigger at 23,38 (VAR_QZ_0x5008 == 3)
@ Johto_VioletCity trigger at 24,37 (VAR_QZ_0x5008 == 3)
Johto_VioletCity_EventScript_83FE9F0::
	lock
	msgbox Johto_VioletCity_Text_83FEEFF, MSGBOX_DEFAULT
	clearflag FLAG_QZ_0x1081
	setflag FLAG_QZ_0x106A
	setvar VAR_QZ_0x5008, 4
	release
	end

Johto_VioletCity_EventScript_83FEA06::
	lock
	playbgm MUS_QZ_655, 1
	playse SE_PIN
	applymovement 11, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 11, Johto_VioletCity_Movement_83FEAF5
	waitmovement 0
	msgbox Johto_VioletCity_Text_83FEF92, MSGBOX_DEFAULT
	closemessage
	applymovement 11, Johto_VioletCity_Movement_83FEAFB
	waitmovement 0
	msgbox Johto_VioletCity_Text_83FEFB3, MSGBOX_DEFAULT
	closemessage
	applymovement 11, Johto_VioletCity_Movement_83FEAFD
	waitmovement 0
	msgbox Johto_VioletCity_Text_83FF017, MSGBOX_DEFAULT
	closemessage
	playse SE_M_PETAL_DANCE
	applymovement 11, Johto_VioletCity_Movement_83FEB01
	waitmovement 0
	msgbox Johto_VioletCity_Text_83FF076, MSGBOX_DEFAULT
	closemessage
	applymovement 11, Johto_VioletCity_Movement_83FEB12
	waitmovement 0
	fadedefaultbgm
	setvar VAR_QZ_0x5012, 2
	setflag FLAG_QZ_0x108B
	removeobject 11
	release
	end

@ Johto_VioletCity object 10 (local id 10, gfx 0x3A9E) at 46,33
Johto_VioletCity_EventScript_83FEA7F::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_VioletCity object 6 (local id 6, gfx 0x3A9E) at 34,12
Johto_VioletCity_EventScript_83FEA8C::
	setorcopyvar VAR_0x8000, ITEM_PECHA_BERRY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_VioletCity object 7 (local id 7, gfx 0x3A9E) at 17,12
Johto_VioletCity_EventScript_83FEA99::
	setorcopyvar VAR_0x8000, ITEM_HYPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_VioletCity sign / bg event at 25,36
Johto_VioletCity_EventScript_83FEB2A::
	msgbox Johto_VioletCity_Text_83FEE32, MSGBOX_SIGN
	end

Johto_VioletCity_Movement_83FEAA6:
	face_down
	face_right
	face_up
	face_left
	face_down
	face_right
	face_up
	face_left
	face_down
	face_right
	face_up
	face_left
	face_down
	step_end

Johto_VioletCity_Movement_83FEAB4:
	walk_down
	walk_down
	walk_down
	walk_right
	step_end

Johto_VioletCity_Movement_83FEAB9:
	walk_down
	walk_down
	walk_left
	walk_down
	face_right
	step_end

Johto_VioletCity_Movement_83FEABF:
	walk_right
	walk_down
	walk_down
	walk_down
	face_right
	step_end

Johto_VioletCity_Movement_83FEAC5:
	walk_down
	walk_down
	step_end

Johto_VioletCity_Movement_83FEAC8:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

Johto_VioletCity_Movement_83FEAF2:
	walk_up
	walk_up
	step_end

Johto_VioletCity_Movement_83FEAF5:
	walk_left
	walk_left
	walk_left
	walk_left
	face_up
	step_end

Johto_VioletCity_Movement_83FEAFB:
	walk_up
	step_end

Johto_VioletCity_Movement_83FEAFD:
	lock_facing_direction
	walk_down
	unlock_facing_direction
	step_end

Johto_VioletCity_Movement_83FEB01:
	face_right
	delay_8
	face_down
	delay_8
	face_left
	delay_8
	face_up
	delay_8
	face_right
	delay_8
	face_down
	delay_8
	face_left
	delay_8
	face_up
	delay_8
	step_end

Johto_VioletCity_Movement_83FEB12:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_VioletCity_Text_83FEBAB::
	.string "Hello!\n"
	.string "You are trainer?\p"
	.string "Battle GYM LEADER,\n"
	.string "win you did?$"

Johto_VioletCity_Text_83FEBE3::
	.string "Ooh, la la!\n"
	.string "Very indeed nice!$"

Johto_VioletCity_Text_83FEC01::
	.string "Is that so? Then study shall\n"
	.string "you! Follow me!$"

Johto_VioletCity_Text_83FEC2E::
	.string "Here, teacher I am.\n"
	.string "Good it is you study here!$"

Johto_VioletCity_Text_83FEC5D::
	.string "Ghosts are rumored to appear\n"
	.string "in SPROUT TOWER.\p"
	.string "They say that GHOST-type\n"
	.string "moves can bypass the defenses\p"
	.string "of tough POKéMON like ONIX\n"
	.string "and GEODUDE.$"

Johto_VioletCity_Text_83FECEA::
	.string "Hey, you’re a POKéMON trainer?\p"
	.string "If you head to the ACADEMY,\n"
	.string "you might learn something new!$"

Johto_VioletCity_Text_83FED44::
	.string "FALKNER, from the VIOLET\n"
	.string "POKéMON GYM, is a fine trainer!\p"
	.string "He inherited his father’s gym\n"
	.string "and has done a great job with\l"
	.string "it.$"

Johto_VioletCity_Text_83FEDBD::
	.string "I saw a wiggly tree up ahead!\p"
	.string "If you touch it, it squirms\n"
	.string "and dances! Cool!$"

Johto_VioletCity_Text_83FEE32::
	.string "VIOLET CITY POKéMON GYM\p"
	.string "LEADER: FALKNER, The Elegant\n"
	.string "Master of Flying POKéMON$"

Johto_VioletCity_Text_83FEEFF::
	.string "A message from PROF. ELM arrived:\p"
	.string "We discovered something about\n"
	.string "the EGG!\p"
	.string "My assistant is at the POKéMON\n"
	.string "CENTER in VIOLET CITY.\p"
	.string "Please talk to him.$"

Johto_VioletCity_Text_83FEF92::
	.string "Me oh my...\n"
	.string "That EGG must be...!$"

Johto_VioletCity_Text_83FEFB3::
	.string "I see...\p"
	.string "So it was handed from MR. POKéMON\n"
	.string "to PROF.ELM, then from \l"
	.string "PROF.ELM to you.\p"
	.string "Now I get it...$"

Johto_VioletCity_Text_83FF017::
	.string "That EGG is truly an important EGG.\p"
	.string "Please do take good care of it\n"
	.string "for me. Is that understood?$"

Johto_VioletCity_Text_83FF076::
	.string "I am counting on you.$"
