Johto_BurnedTowerB1F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_BurnedTowerB1F_OnFrame_83C8A58
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_BurnedTowerB1F_EventScript_83C8A52
	.byte 0

Johto_BurnedTowerB1F_OnFrame_83C8A58:
	map_script_2 VAR_QZ_0x500B, 3, Johto_BurnedTowerB1F_EventScript_83C8A62
	.2byte 0

Johto_BurnedTowerB1F_EventScript_83C8A52::
	call Quetzal_EventScript_840EA62
	end

Johto_BurnedTowerB1F_EventScript_83C8A62::
	lock
	turnobject LOCALID_PLAYER, 3
	delay 0x3C
	turnobject LOCALID_PLAYER, 4
	delay 0x3C
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	turnobject LOCALID_PLAYER, 2
	delay 0x3C
	playmoncry SPECIES_ENTEI, 0
	applymovement 1, Johto_BurnedTowerB1F_Movement_83C8BA8
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	waitmovement 0
	playmoncry SPECIES_RAIKOU, 0
	applymovement 2, Johto_BurnedTowerB1F_Movement_83C8BA8
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	waitmovement 0
	playmoncry SPECIES_SUICUNE, 0
	applymovement 3, Johto_BurnedTowerB1F_Movement_83C8BA8
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	waitmovement 0
	applymovement 1, Johto_BurnedTowerB1F_Movement_83C8BAC
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	waitmovement 0
	applymovement 2, Johto_BurnedTowerB1F_Movement_83C8BB2
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	waitmovement 0
	applymovement 3, Johto_BurnedTowerB1F_Movement_83C8BB8
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playmoncry SPECIES_SUICUNE, 0
	waitse
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	waitmovement 0
	setflag FLAG_QZ_0x10AF
	removeobject 1
	removeobject 2
	removeobject 3
	delay 0x37
	playse SE_EXIT
	turnobject LOCALID_PLAYER, 4
	applymovement 7, Johto_BurnedTowerB1F_Movement_83C8BC1
	waitmovement 0
	msgbox Johto_BurnedTowerB1F_Text_83C8BD9, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Johto_BurnedTowerB1F_Movement_83C8BCD
	waitmovement 0
	playse SE_EXIT
	setflag FLAG_QZ_0x10B0
	removeobject 7
	setflag FLAG_QZ_0x10AE
	setvar VAR_0x8004, 1
	special InitRoamer
	setflag FLAG_QZ_0x10A9
	setvar VAR_QZ_0x500B, 4
	release
	end

@ Johto_BurnedTowerB1F object 5 (local id 5, gfx 0x3A9E) at 5,15
Johto_BurnedTowerB1F_EventScript_83C8D3A::
	setorcopyvar VAR_0x8000, ITEM_DUSK_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_BurnedTowerB1F object 4 (local id 4, gfx 0x3A9E) at 6,9
Johto_BurnedTowerB1F_EventScript_83C8D47::
	setorcopyvar VAR_0x8000, ITEM_ULTRA_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_BurnedTowerB1F_Movement_83C8BA8:
	jump_in_place_down
	jump_in_place_down
	jump_in_place_down
	step_end

Johto_BurnedTowerB1F_Movement_83C8BAC:
	jump_down
	jump_2_right
	jump_2_right
	jump_2_right
	jump_2_right
	step_end

Johto_BurnedTowerB1F_Movement_83C8BB2:
	jump_down
	jump_2_left
	jump_2_left
	jump_2_left
	jump_2_left
	step_end

Johto_BurnedTowerB1F_Movement_83C8BB8:
	jump_2_down
	delay_8
	jump_in_place_right_left
	jump_in_place_right_left
	jump_left
	jump_2_down
	jump_2_down
	jump_2_down
	step_end

Johto_BurnedTowerB1F_Movement_83C8BC1:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_BurnedTowerB1F_Movement_83C8BCD:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

Johto_BurnedTowerB1F_Text_83C8BD9::
	.string "EUSINE: I’m shocked!\p"
	.string "SUICUNE raced by like a blur,\n"
	.string "right in front of my eyes!\p"
	.string "For ten years I chased\n"
	.string "SUICUNE, and I finally got to\l"
	.string "see it.\p"
	.string "I’m all choked up!\p"
	.string "{PLAYER}, I owe this all to you!\p"
	.string "Thank you!\p"
	.string "I heard that the legendary\n"
	.string "POKéMON of ECRUTEAK test\l"
	.string "chosen humans by allowing them\l"
	.string "to get close.\p"
	.string "I’m going to track SUICUNE.\p"
	.string "{PLAYER}, let’s meet again!\p"
	.string "Farewell!$"
