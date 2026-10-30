Johto_VictoryRoad3F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_VictoryRoad3F object 3 (local id 3, gfx 0x3B) at 23,16
Johto_VictoryRoad3F_EventScript_83FE4EB::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_VictoryRoad3F object 2 (local id 2, gfx 0x3B) at 35,5
Johto_VictoryRoad3F_EventScript_83FE4F8::
	setorcopyvar VAR_0x8000, ITEM_MAX_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_VictoryRoad3F object 1 (local id 1, gfx 0x3B) at 20,3
Johto_VictoryRoad3F_EventScript_83FE505::
	setorcopyvar VAR_0x8000, ITEM_ULTRA_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_VictoryRoad3F trigger at 28,7 (VAR_QZ_0x5029 == 2)
@ Johto_VictoryRoad3F trigger at 27,7 (VAR_QZ_0x5029 == 2)
@ Johto_VictoryRoad3F trigger at 29,7 (VAR_QZ_0x5029 == 2)
Johto_VictoryRoad3F_EventScript_83FE512::
	lock
	playbgm MUS_QZ_648, 1
	delay 5
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	delay 0x3C
	addobject 9
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 27, Johto_VictoryRoad3F_EventScript_83FE57F
	call_if_eq VAR_TEMP_0, 28, Johto_VictoryRoad3F_EventScript_83FE591
	call_if_eq VAR_TEMP_0, 29, Johto_VictoryRoad3F_EventScript_83FE5A3
	msgbox Johto_VictoryRoad3F_Text_83FE5F1, MSGBOX_DEFAULT
	call Johto_VictoryRoad3F_EventScript_83FE5B5
	playbgm MUS_QZ_649, 1
	msgbox Johto_VictoryRoad3F_Text_83FE76A, MSGBOX_DEFAULT
	closemessage
	applymovement 9, Johto_VictoryRoad3F_Movement_83FE5E4
	waitmovement 0
	removeobject 9
	fadedefaultbgm
	release
	setvar VAR_QZ_0x5029, 3
	end

Johto_VictoryRoad3F_EventScript_83FE57F::
	applymovement 9, Johto_VictoryRoad3F_Movement_83FE5C0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 9
	return

Johto_VictoryRoad3F_EventScript_83FE591::
	applymovement 9, Johto_VictoryRoad3F_Movement_83FE5CB
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 9
	return

Johto_VictoryRoad3F_EventScript_83FE5A3::
	applymovement 9, Johto_VictoryRoad3F_Movement_83FE5D7
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 9
	return

Johto_VictoryRoad3F_EventScript_83FE5B5::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4322, 0, Johto_VictoryRoad3F_Text_83FE6D9
	return

Johto_VictoryRoad3F_Movement_83FE5C0:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	step_end

Johto_VictoryRoad3F_Movement_83FE5CB:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	step_end

Johto_VictoryRoad3F_Movement_83FE5D7:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	step_end

Johto_VictoryRoad3F_Movement_83FE5E4:
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_VictoryRoad3F_Text_83FE5F1::
	.string "Hold it.\p"
	.string "…Are you going to take the POKéMON\n"
	.string "LEAGUE challenge?\p"
	.string "…Don’t make me laugh.\p"
	.string "You’re so much weaker than I am.\p"
	.string "I’m not like I was before.\p"
	.string "I now have the best and strongest\n"
	.string "POKéMON with me.\p"
	.string "I’m invincible!\p"
	.string "{PLAYER}!\n"
	.string "I challenge you!$"

Johto_VictoryRoad3F_Text_83FE6D9::
	.string "…I couldn’t win…\p"
	.string "I gave it every-thing I had…\p"
	.string "What you possess, and what I lack…\p"
	.string "I’m beginning to understand what\n"
	.string "that dragon master said to me…$"

Johto_VictoryRoad3F_Text_83FE76A::
	.string "…I haven’t given up on becoming the\n"
	.string "greatest trainer…\p"
	.string "I’m going to find out why I can’t\n"
	.string "win and become stronger…\p"
	.string "When I do, I will challenge you.\p"
	.string "And I’ll beat you down with all my\n"
	.string "power.\p"
	.string "…Humph! You keep at it until then.$"
