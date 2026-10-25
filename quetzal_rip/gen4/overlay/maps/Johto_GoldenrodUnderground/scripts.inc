Johto_GoldenrodUnderground_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_LOAD, Johto_GoldenrodUnderground_EventScript_83DAF6C
	.byte 0

Johto_GoldenrodUnderground_EventScript_82E40D4::
	pokemart Johto_GoldenrodUnderground_Mart_82E40F0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodUnderground object 2 (local id 2, gfx 0x1E) at 6,24
Johto_GoldenrodUnderground_EventScript_83DAF41::
	lock
	faceplayer
	message Johto_GoldenrodUnderground_Text_82E4142
	waitmessage
	goto_if_set FLAG_QZ_0x11B6, Johto_GoldenrodUnderground_EventScript_82E40D4
	pokemart Johto_GoldenrodUnderground_Mart_82E40E4
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodUnderground_EventScript_83DAF6C::
	goto Johto_GoldenrodUnderground_EventScript_83DB290

@ Johto_GoldenrodUnderground trigger at 8,12 (VAR_QZ_0x500A == 7)
Johto_GoldenrodUnderground_EventScript_83DAF72::
	lock
	playbgm MUS_QZ_655, 1
	applymovement 12, Johto_GoldenrodUnderground_Movement_83DB040
	waitmovement 0
	msgbox Johto_GoldenrodUnderground_Text_83DB062, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 12, Quetzal_Movement_83607F9
	delay 0x3C
	msgbox Johto_GoldenrodUnderground_Text_83DB123, MSGBOX_DEFAULT
	closemessage
	applymovement 12, Quetzal_Movement_8360837
	waitmovement 0
	applymovement 12, Johto_GoldenrodUnderground_Movement_83DB04B
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 12, Johto_GoldenrodUnderground_Movement_83DB053
	waitmovement 0
	applymovement 12, Johto_GoldenrodUnderground_Movement_83DB05B
	waitmovement 0
	setflag FLAG_QZ_0x109B
	removeobject 12
	fadedefaultbgm
	setvar VAR_QZ_0x500A, 8
	releaseall
	end

@ Johto_GoldenrodUnderground trigger at 8,11 (VAR_QZ_0x500A == 7)
Johto_GoldenrodUnderground_EventScript_83DAFD9::
	lock
	playbgm MUS_QZ_655, 1
	applymovement 12, Johto_GoldenrodUnderground_Movement_83DB045
	waitmovement 0
	msgbox Johto_GoldenrodUnderground_Text_83DB062, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 12, Quetzal_Movement_83607F9
	delay 0x3C
	msgbox Johto_GoldenrodUnderground_Text_83DB123, MSGBOX_DEFAULT
	closemessage
	applymovement 12, Quetzal_Movement_8360839
	waitmovement 0
	applymovement 12, Johto_GoldenrodUnderground_Movement_83DB04B
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 12, Johto_GoldenrodUnderground_Movement_83DB053
	waitmovement 0
	applymovement 12, Johto_GoldenrodUnderground_Movement_83DB05B
	waitmovement 0
	setflag FLAG_QZ_0x109B
	removeobject 12
	fadedefaultbgm
	setvar VAR_QZ_0x500A, 8
	releaseall
	end

@ Johto_GoldenrodUnderground object 6 (local id 6, gfx 0x3EB7) at 4,7
Johto_GoldenrodUnderground_EventScript_83DB146::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4728, 0, Johto_GoldenrodUnderground_Text_83DB336, Johto_GoldenrodUnderground_Text_83DB383
	msgbox Johto_GoldenrodUnderground_Text_83DB38D, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUnderground object 5 (local id 5, gfx 0x3EB7) at 4,10
Johto_GoldenrodUnderground_EventScript_83DB15D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4242, 0, Johto_GoldenrodUnderground_Text_83DB3BA, Johto_GoldenrodUnderground_Text_83DB423
	msgbox Johto_GoldenrodUnderground_Text_83DB42F, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUnderground object 1 (local id 1, gfx 0x3EB7) at 4,30
Johto_GoldenrodUnderground_EventScript_83DB174::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4927, 0, Johto_GoldenrodUnderground_Text_83DB476, Johto_GoldenrodUnderground_Text_83DB4B5
	msgbox Johto_GoldenrodUnderground_Text_83DB4BE, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUnderground object 7 (local id 7, gfx 0x3EB7) at 1,27
Johto_GoldenrodUnderground_EventScript_83DB18B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4320, 0, Johto_GoldenrodUnderground_Text_83DB4F9, Johto_GoldenrodUnderground_Text_83DB537
	msgbox Johto_GoldenrodUnderground_Text_83DB558, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUnderground object 3 (local id 3, gfx 0x3EBF) at 6,20
Johto_GoldenrodUnderground_EventScript_83DB1A2::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodUnderground_Mart_83DB1BC
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodUnderground object 13 (local id 13, gfx 0x3E8E) at 1,20
Johto_GoldenrodUnderground_EventScript_83DB1E2::
	lock
	faceplayer
	message Johto_GoldenrodUnderground_Text_83DB208
	waitmessage
	pokemart Johto_GoldenrodUnderground_Mart_83DB1FC
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodUnderground object 4 (local id 4, gfx 0x3E8E) at 6,16
Johto_GoldenrodUnderground_EventScript_83DB260::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_GoldenrodUnderground_Mart_83DB278
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodUnderground_EventScript_83DB290::
	goto_if_set FLAG_QZ_0x109A, Quetzal_EventScript_83DB29A
	end

@ Johto_GoldenrodUnderground sign / bg event at 12,10
Johto_GoldenrodUnderground_EventScript_83DB2A4::
	lock
	goto_if_set FLAG_QZ_0x109A, Johto_GoldenrodUnderground_EventScript_83DB2E7
	checkitem ITEM_BASEMENT_KEY, 1
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodUnderground_EventScript_83DB2C9
	msgbox Johto_GoldenrodUnderground_Text_83DB5BF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodUnderground_EventScript_83DB2C9::
	msgbox Johto_GoldenrodUnderground_Text_83DB5E4, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x109A
	setmetatile 0xC, 0xA, 0x2DD, 0
	playse SE_BANG
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUnderground_EventScript_83DB2E7::
	msgbox Johto_GoldenrodUnderground_Text_83DB5D2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodUnderground object 8 (local id 8, gfx 0x3A9E) at 13,13
Johto_GoldenrodUnderground_EventScript_83DB2F2::
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FD2E
	setorcopyvar VAR_0x8000, 0x2CE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUnderground object 11 (local id 11, gfx 0x3B) at 5,26
Johto_GoldenrodUnderground_EventScript_83DB30F::
	setorcopyvar VAR_0x8000, 0x1D
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUnderground object 9 (local id 9, gfx 0x3B) at 1,8
Johto_GoldenrodUnderground_EventScript_83DB31C::
	setorcopyvar VAR_0x8000, 0x2C
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUnderground object 10 (local id 10, gfx 0x3B) at 1,23
Johto_GoldenrodUnderground_EventScript_83DB329::
	setorcopyvar VAR_0x8000, 0x2B
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_GoldenrodUnderground_Mart_82E40E4:
	.2byte ITEM_ENERGY_POWDER
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Mart_82E40F0:
	.2byte ITEM_ENERGY_POWDER
	.2byte ITEM_ENERGY_ROOT
	.2byte ITEM_HEAL_POWDER
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Movement_83DB040:
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodUnderground_Movement_83DB045:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodUnderground_Movement_83DB04B:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodUnderground_Movement_83DB053:
	face_down
	face_right
	face_up
	face_left
	face_down
	face_right
	face_up
	step_end

Johto_GoldenrodUnderground_Movement_83DB05B:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_GoldenrodUnderground_Mart_83DB1BC:
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Mart_83DB1FC:
	.2byte ITEM_BLUE_FLUTE
	.2byte ITEM_YELLOW_FLUTE
	.2byte ITEM_RED_FLUTE
	.2byte ITEM_BLACK_FLUTE
	.2byte ITEM_WHITE_FLUTE
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Mart_83DB278:
	.2byte ITEM_METEORITE
	.2byte ITEM_NONE

Johto_GoldenrodUnderground_Text_82E4142::
	.string "Welcome to the HERB SHOP, home of\n"
	.string "effective and inexpensive medicine!$"

Johto_GoldenrodUnderground_Text_83DB062::
	.string "Team Rocket members in black suits\n"
	.string "have been running around...\p"
	.string "I was wondering what was going on.\n"
	.string "So, you took on Team Rocket?\l"
	.string "You are impressive!\p"
	.string "You may be able to face the\n"
	.string "legendary POKé...$"

Johto_GoldenrodUnderground_Text_83DB123::
	.string "Hahaha! Never mind.\n"
	.string "I must go now.$"

Johto_GoldenrodUnderground_Text_83DB208::
	.string "My glass flutes produce different\n"
	.string "effects depending on their color.\p"
	.string "Would you like one?$"

Johto_GoldenrodUnderground_Text_83DB336::
	.string "I got booted out of the GAME CORNER.\p"
	.string "I was trying to cheat using my\n"
	.string "POKéMON…$"

Johto_GoldenrodUnderground_Text_83DB383::
	.string "…Grumble…$"

Johto_GoldenrodUnderground_Text_83DB38D::
	.string "I guess I have to do things fair and\n"
	.string "square…$"

Johto_GoldenrodUnderground_Text_83DB3BA::
	.string "Do you consider type alignments in\n"
	.string "battle?\p"
	.string "If you know your type advantages,\n"
	.string "you’ll do better in battle.$"

Johto_GoldenrodUnderground_Text_83DB423::
	.string "Ow, ow, ow!$"

Johto_GoldenrodUnderground_Text_83DB42F::
	.string "I know my POKéMON type alignments.\p"
	.string "But I only use one type of POKéMON.$"

Johto_GoldenrodUnderground_Text_83DB476::
	.string "My POKéMON just got a haircut!\p"
	.string "I’ll show you how strong it is!$"

Johto_GoldenrodUnderground_Text_83DB4B5::
	.string "Aiyeeee!$"

Johto_GoldenrodUnderground_Text_83DB4BE::
	.string "Your POKéMON will like you more if\n"
	.string "you give them haircuts.$"

Johto_GoldenrodUnderground_Text_83DB4F9::
	.string "I think you have some rare POKéMON\n"
	.string "with you.\p"
	.string "Let me see them!$"

Johto_GoldenrodUnderground_Text_83DB537::
	.string "Gaah! I lost! That makes me mad!$"

Johto_GoldenrodUnderground_Text_83DB558::
	.string "When I was walking near the BIKE\n"
	.string "SHOP last night, I felt like I was\l"
	.string "being watched from the rooftops...$"

Johto_GoldenrodUnderground_Text_83DB5BF::
	.string "The door’s locked…$"

Johto_GoldenrodUnderground_Text_83DB5D2::
	.string "The door is open.$"

Johto_GoldenrodUnderground_Text_83DB5E4::
	.string "The BASEMENT KEY opened the door.$"
