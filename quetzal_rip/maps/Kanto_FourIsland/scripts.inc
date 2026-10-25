Kanto_FourIsland_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C3036
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FourIsland_EventScript_8417962
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_FourIsland_OnFrame_8417993
	.byte 0

Kanto_FourIsland_OnFrame_8417993:
	map_script_2 VAR_QZ_0x5110, 0, Kanto_FourIsland_EventScript_841799D
	.2byte 0

Kanto_FourIsland_EventScript_8417962::
	special QzKantoTravelingEventCheck_773
	call Quetzal_EventScript_840FBB5
	setflag FLAG_QZ_0x223C
	call_if_eq VAR_QZ_0x5110, 0, Kanto_FourIsland_EventScript_841797E
	call Kanto_FourIsland_EventScript_8417982
	end

Kanto_FourIsland_EventScript_841797E::
	clearflag FLAG_QZ_0x218B
	return

Kanto_FourIsland_EventScript_8417982::
	goto_if_unset FLAG_QZ_0x21E9, Kanto_FourIsland_EventScript_8417992
	setobjectxyperm 1, 0x10, 0xE

Kanto_FourIsland_EventScript_8417992::
	return

Kanto_FourIsland_EventScript_841799D::
	lockall
	textcolor 0
	applymovement 10, Quetzal_Movement_8457AB0
	waitmovement 0
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	applymovement 10, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 10, Quetzal_Movement_8457AA0
	waitmovement 0
	delay 0x19
	applymovement 10, Kanto_FourIsland_Movement_84179FA
	waitmovement 0
	msgbox Kanto_FourIsland_Text_8432DFC, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_FourIsland_Movement_8417A03
	applymovement 10, Kanto_FourIsland_Movement_84179FF
	waitmovement 0
	fadedefaultbgm
	playse SE_EXIT
	delay 0x23
	removeobject 10
	setvar VAR_QZ_0x5110, 1
	releaseall
	end

@ Kanto_FourIsland object 1 (local id 1, gfx 0x3F2C) at 16,13
Kanto_FourIsland_EventScript_8417A06::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	goto Quetzal_EventScript_83F08A1

@ Kanto_FourIsland object 6 (local id 6, gfx 0x3F2F) at 26,19
Kanto_FourIsland_EventScript_8417AAF::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x220A, Kanto_FourIsland_EventScript_8417AC4
	msgbox Kanto_FourIsland_Text_8432CF0, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_EventScript_8417AC4::
	setvar VAR_0x8004, 9
	setvar VAR_0x8005, 5
	special QzQuestLogNop_737
	msgbox Kanto_FourIsland_Text_8432D3A, MSGBOX_DEFAULT
	release
	end

@ Kanto_FourIsland object 12 (local id 12, gfx 0x3F18) at 31,21
Kanto_FourIsland_EventScript_8417ADB::
	lock
	faceplayer
	setvar VAR_0x8004, 9
	setvar VAR_0x8005, 4
	special QzQuestLogNop_737
	msgbox Kanto_FourIsland_Text_8432FAA, MSGBOX_DEFAULT
	release
	end

@ Kanto_FourIsland object 11 (local id 11, gfx 0x3F04) at 36,13
Kanto_FourIsland_EventScript_8417AF4::
	msgbox Kanto_FourIsland_Text_8433055, MSGBOX_NPC
	end

@ Kanto_FourIsland sign / bg event at 13,19
Kanto_FourIsland_EventScript_8417AFD::
	msgbox Kanto_FourIsland_Text_8432CC8, MSGBOX_SIGN
	end

@ Kanto_FourIsland sign / bg event at 31,23
Kanto_FourIsland_EventScript_8417B06::
	msgbox Kanto_FourIsland_Text_84330B2, MSGBOX_SIGN
	end

@ Kanto_FourIsland object 8 (local id 8, gfx 0x3F0E) at 5,6
Kanto_FourIsland_EventScript_846C10E::
	setorcopyvar VAR_0x8000, 0x90
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_FourIsland object 9 (local id 9, gfx 0x3F0E) at 32,19
Kanto_FourIsland_EventScript_846C11B::
	setorcopyvar VAR_0x8000, 0x8F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_FourIsland_Movement_84179FA:
	walk_right
	walk_right
	walk_down
	walk_down
	step_end

Kanto_FourIsland_Movement_84179FF:
	delay_16
	delay_4
	walk_down
	step_end

Kanto_FourIsland_Movement_8417A03:
	walk_left
	walk_in_place_faster_right
	step_end

Kanto_FourIsland_Text_8432CC8::
	.string "FOUR ISLAND\n"
	.string "The Warm, Blue, Floe Island$"

Kanto_FourIsland_Text_8432CF0::
	.string "LORELEI has returned.\p"
	.string "Did she find the city life too\n"
	.string "stressful, I wonder?$"

Kanto_FourIsland_Text_8432D3A::
	.string "LORELEI has gone back.\p"
	.string "The LAPRAS she has, I imagine it\n"
	.string "to be the one she met as a child.\p"
	.string "I believe it was in ICEFALL CAVE\n"
	.string "that she caught it.\p"
	.string "Perhaps that POKéMON has been with\n"
	.string "her ever since.$"

Kanto_FourIsland_Text_8432DFC::
	.string "BLUE: Hey!\n"
	.string "{PLAYER}!\p"
	.string "What are you doing here in the\n"
	.string "SEVII ISLANDS?\p"
	.string "You should quit copying me,\n"
	.string "you know?\p"
	.string "Anyways, I already got my POKéMON\n"
	.string "EGG, so I’m done with this island.\p"
	.string "Heh, I bet you don’t even know\n"
	.string "about POKéMON EGGS.\p"
	.string "You’ll never fill your NATIONAL\n"
	.string "POKéDEX that way.\p"
	.string "By the way, I saw someone we both\n"
	.string "know on this island.\p"
	.string "Why don’t you go look around if\n"
	.string "it makes you curious?\p"
	.string "Me, I don’t have the time to\n"
	.string "waste.\p"
	.string "Be smelling ya!$"

Kanto_FourIsland_Text_8432FAA::
	.string "Oh, you found me!\n"
	.string "Please don’t tell LORELEI.\p"
	.string "Did you know that LORELEI has lots\n"
	.string "and lots of stuffed dolls?\p"
	.string "Every time she comes back to\n"
	.string "FOUR ISLAND, her collection grows!$"

Kanto_FourIsland_Text_8433055::
	.string "Ooh, chilly!\p"
	.string "I made a slushie using ice I\n"
	.string "shaved inside ICEFALL CAVE.\p"
	.string "It’s chilly and tasty!$"

Kanto_FourIsland_Text_84330B2::
	.string "LORELEI’S HOUSE$"
