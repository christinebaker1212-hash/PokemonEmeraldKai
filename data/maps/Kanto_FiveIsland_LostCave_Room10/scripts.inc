Kanto_FiveIsland_LostCave_Room10_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_FiveIsland_LostCave_Room10_EventScript_84149F9
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_FiveIsland_LostCave_Room10_OnFrame_8414A09
	.byte 0

Kanto_FiveIsland_LostCave_Room10_OnFrame_8414A09:
	map_script_2 VAR_QZ_0x510E, 0, Kanto_FiveIsland_LostCave_Room10_EventScript_8414A13
	.2byte 0

Kanto_FiveIsland_LostCave_Room10_EventScript_84149F9::
	checktrainerflag 0x225E
	call_if_eq Kanto_FiveIsland_LostCave_Room10_EventScript_8414A03
	end

Kanto_FiveIsland_LostCave_Room10_EventScript_8414A03::
	setvar VAR_QZ_0x510E, 1
	return

Kanto_FiveIsland_LostCave_Room10_EventScript_8414A13::
	lockall
	textcolor 1
	applymovement 1, Kanto_FiveIsland_LostCave_Room10_Movement_8414A83
	waitmovement 0
	delay 0x64
	applymovement 1, Kanto_FiveIsland_LostCave_Room10_Movement_8414A8B
	waitmovement 0
	msgbox Kanto_FiveIsland_LostCave_Room10_Text_842D76B, MSGBOX_DEFAULT
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Kanto_FiveIsland_LostCave_Room10_EventScript_8414A79
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8798, 0, Kanto_FiveIsland_LostCave_Room10_Text_842D78F
	msgbox Kanto_FiveIsland_LostCave_Room10_Text_842D7BB, MSGBOX_DEFAULT
	closemessage
	call Kanto_FiveIsland_LostCave_Room10_EventScript_8414A68
	warp MAP_KANTO_FIVE_ISLAND_RESORT_GORGEOUS, 255, 39, 10
	waitstate
	releaseall
	end

Kanto_FiveIsland_LostCave_Room10_EventScript_8414A68::
	setflag FLAG_QZ_0x219B
	clearflag FLAG_QZ_0x21AE
	setvar VAR_QZ_0x510E, 1
	setvar VAR_QZ_0x510F, 1
	return

Kanto_FiveIsland_LostCave_Room10_EventScript_8414A79::
	settrainerflag 0x225E
	call Kanto_FiveIsland_LostCave_Room10_EventScript_8414A68
	releaseall
	end

@ Kanto_FiveIsland_LostCave_Room10 object 2 (local id 2, gfx 0x3F0E) at 5,2
Kanto_FiveIsland_LostCave_Room10_EventScript_846C2A1::
	setorcopyvar VAR_0x8000, ITEM_SILK_SCARF
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_FiveIsland_LostCave_Room10_Movement_8414A83:
	walk_left
	delay_16
	walk_right
	delay_16
	walk_in_place_faster_up
	delay_16
	walk_in_place_faster_down
	step_end

Kanto_FiveIsland_LostCave_Room10_Movement_8414A8B:
	walk_down
	walk_down
	step_end

Kanto_FiveIsland_LostCave_Room10_Text_842D76B::
	.string "W-what do you intend to do with\n"
	.string "me?$"

Kanto_FiveIsland_LostCave_Room10_Text_842D78F::
	.string "Oh… I was afraid that you meant to\n"
	.string "harm me.$"

Kanto_FiveIsland_LostCave_Room10_Text_842D7BB::
	.string "I am so glad that you happened\n"
	.string "here.\p"
	.string "I grew rather bored of walking,\n"
	.string "and I am feeling fatigued.\p"
	.string "May I ask you to take me home\n"
	.string "from here?$"
