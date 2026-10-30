Kanto_Route18_EastEntrance_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route18_EastEntrance_1F_EventScript_8420422
	.byte 0

Kanto_Route18_EastEntrance_1F_EventScript_8420422::
	clearflag FLAG_QZ_0x2210
	setvar VAR_TEMP_1, 0
	checkitem ITEM_BICYCLE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route18_EastEntrance_1F_EventScript_842045B
	checkitem ITEM_MACH_BIKE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route18_EastEntrance_1F_EventScript_842045B
	checkitem ITEM_ACRO_BIKE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route18_EastEntrance_1F_EventScript_842045B
	end

Kanto_Route18_EastEntrance_1F_EventScript_842045B::
	setvar VAR_TEMP_1, 1
	end

@ Kanto_Route18_EastEntrance_1F object 1 (local id 1, gfx 0x3F36) at 6,2
Kanto_Route18_EastEntrance_1F_EventScript_8420461::
	msgbox Kanto_Route18_EastEntrance_1F_Text_84501AE, MSGBOX_NPC
	end

@ Kanto_Route18_EastEntrance_1F trigger at 6,4 (VAR_TEMP_1 == 0)
Kanto_Route18_EastEntrance_1F_EventScript_842046A::
	lockall
	setvar VAR_0x8008, 0
	goto Kanto_Route18_EastEntrance_1F_EventScript_84204A6

@ Kanto_Route18_EastEntrance_1F trigger at 6,5 (VAR_TEMP_1 == 0)
Kanto_Route18_EastEntrance_1F_EventScript_8420476::
	lockall
	setvar VAR_0x8008, 1
	goto Kanto_Route18_EastEntrance_1F_EventScript_84204A6

@ Kanto_Route18_EastEntrance_1F trigger at 6,6 (VAR_TEMP_1 == 0)
Kanto_Route18_EastEntrance_1F_EventScript_8420482::
	lockall
	setvar VAR_0x8008, 2
	goto Kanto_Route18_EastEntrance_1F_EventScript_84204A6

@ Kanto_Route18_EastEntrance_1F trigger at 6,7 (VAR_TEMP_1 == 0)
Kanto_Route18_EastEntrance_1F_EventScript_842048E::
	lockall
	setvar VAR_0x8008, 3
	goto Kanto_Route18_EastEntrance_1F_EventScript_84204A6

@ Kanto_Route18_EastEntrance_1F trigger at 6,8 (VAR_TEMP_1 == 0)
Kanto_Route18_EastEntrance_1F_EventScript_842049A::
	lockall
	setvar VAR_0x8008, 4
	goto Kanto_Route18_EastEntrance_1F_EventScript_84204A6

Kanto_Route18_EastEntrance_1F_EventScript_84204A6::
	textcolor 0
	msgbox Kanto_Route18_EastEntrance_1F_Text_8450210, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	delay 0x14
	call_if_eq VAR_0x8008, 1, Kanto_Route18_EastEntrance_1F_EventScript_84204FF
	call_if_eq VAR_0x8008, 2, Kanto_Route18_EastEntrance_1F_EventScript_842050A
	call_if_eq VAR_0x8008, 3, Kanto_Route18_EastEntrance_1F_EventScript_8420515
	call_if_eq VAR_0x8008, 4, Kanto_Route18_EastEntrance_1F_EventScript_8420520
	msgbox Kanto_Route18_EastEntrance_1F_Text_8450184, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_Route18_EastEntrance_1F_Movement_8420539
	waitmovement 0
	releaseall
	end

Kanto_Route18_EastEntrance_1F_EventScript_84204FF::
	applymovement LOCALID_PLAYER, Kanto_Route18_EastEntrance_1F_Movement_842052B
	waitmovement 0
	return

Kanto_Route18_EastEntrance_1F_EventScript_842050A::
	applymovement LOCALID_PLAYER, Kanto_Route18_EastEntrance_1F_Movement_842052D
	waitmovement 0
	return

Kanto_Route18_EastEntrance_1F_EventScript_8420515::
	applymovement LOCALID_PLAYER, Kanto_Route18_EastEntrance_1F_Movement_8420530
	waitmovement 0
	return

Kanto_Route18_EastEntrance_1F_EventScript_8420520::
	applymovement LOCALID_PLAYER, Kanto_Route18_EastEntrance_1F_Movement_8420534
	waitmovement 0
	return

Kanto_Route18_EastEntrance_1F_Movement_842052B:
	walk_up
	step_end

Kanto_Route18_EastEntrance_1F_Movement_842052D:
	walk_up
	walk_up
	step_end

Kanto_Route18_EastEntrance_1F_Movement_8420530:
	walk_up
	walk_up
	walk_up
	step_end

Kanto_Route18_EastEntrance_1F_Movement_8420534:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_Route18_EastEntrance_1F_Movement_8420539:
	walk_right
	step_end

Kanto_Route18_EastEntrance_1F_Text_8450184::
	.string "You need a BICYCLE to enter\n"
	.string "CYCLING ROAD.$"

Kanto_Route18_EastEntrance_1F_Text_84501AE::
	.string "You need a BICYCLE to enter.\p"
	.string "If you can use SURF, you can also\n"
	.string "explore the water along the route.$"

Kanto_Route18_EastEntrance_1F_Text_8450210::
	.string "Excuse me!$"
