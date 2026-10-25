Johto_MahoganyMart1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_LOAD, Johto_MahoganyMart1F_EventScript_83DF27C
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_MahoganyMart1F_OnFrame_83DF292
	.byte 0

Johto_MahoganyMart1F_OnFrame_83DF292:
	map_script_2 VAR_QZ_0x500C, 4, Johto_MahoganyMart1F_EventScript_83DF29C
	.2byte 0

Johto_MahoganyMart1F_EventScript_83DF27C::
	goto_if_ge VAR_QZ_0x500C, 5, Quetzal_EventScript_83DF288
	end

Johto_MahoganyMart1F_EventScript_83DF29C::
	playbgm MUS_QZ_637, 1
	lock
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	delay 0x3C
	msgbox Johto_MahoganyMart1F_Text_83DF573, MSGBOX_DEFAULT
	closemessage
	playse SE_M_HYPER_BEAM
	applymovement 4, Johto_MahoganyMart1F_Movement_83DF3F1
	delay 0xA
	applymovement 2, Johto_MahoganyMart1F_Movement_83DF3FD
	waitmovement 2
	waitse
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 5
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitmovement 4
	waitse
	delay 0xA
	turnobject 3, 1
	delay 0x32
	applymovement 3, Johto_MahoganyMart1F_Movement_83DF402
	waitmovement 0
	msgbox Johto_MahoganyMart1F_Text_83DF591, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 4, Quetzal_Movement_83607F7
	delay 0xA
	applymovement 4, Johto_MahoganyMart1F_Movement_83DF407
	waitmovement 0
	playmoncry SPECIES_DRAGONITE, 0
	waitmovement 0
	applymovement 3, Johto_MahoganyMart1F_Movement_83DF411
	applymovement LOCALID_PLAYER, Johto_MahoganyMart1F_Movement_83DF418
	waitmovement 3
	waitmovement LOCALID_PLAYER
	playse SE_M_HYPER_BEAM
	applymovement 4, Johto_MahoganyMart1F_Movement_83DF3F8
	delay 0x14
	waitse
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 5
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitmovement 0
	setmetatile 8, 4, 0x28D, 0
	special DrawWholeMapView
	delay 0x28
	msgbox Johto_MahoganyMart1F_Text_83DF5E7, MSGBOX_DEFAULT
	turnobject 4, 3
	playmoncry SPECIES_DRAGONITE, 0
	waitmoncry
	turnobject 4, 4
	turnobject 3, 1
	msgbox Johto_MahoganyMart1F_Text_83DF615, MSGBOX_DEFAULT
	closemessage
	applymovement 4, Quetzal_Movement_836083B
	waitmovement 0
	removeobject 4
	playse SE_EXIT
	applymovement 3, Quetzal_Movement_836083B
	waitmovement 0
	applymovement 3, Quetzal_Movement_836083B
	waitmovement 0
	removeobject 3
	playse SE_EXIT
	delay 0x32
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	delay 0x14
	applymovement 2, Johto_MahoganyMart1F_Movement_83DF3E8
	waitmovement 0
	removeobject 2
	playse SE_EXIT
	setflag FLAG_QZ_0x104F
	setvar VAR_QZ_0x500C, 5
	release
	end

@ Johto_MahoganyMart1F object 1 (local id 1, gfx 0x1E) at 2,5
Johto_MahoganyMart1F_EventScript_83DF41E::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500C, 5, Johto_MahoganyMart1F_EventScript_83DF440
	goto_if_ge VAR_QZ_0x500C, 14, Johto_MahoganyMart1F_EventScript_83DF44B
	msgbox Johto_MahoganyMart1F_Text_83DF47E, MSGBOX_NPC
	release
	end

Johto_MahoganyMart1F_EventScript_83DF440::
	msgbox Johto_MahoganyMart1F_Text_83DF4BF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MahoganyMart1F_EventScript_83DF44B::
	message Quetzal_Text_8360D02
	waitmessage
	pokemart Johto_MahoganyMart1F_Mart_83DF460
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_MahoganyMart1F_Movement_83DF3E8:
	walk_faster_left
	walk_faster_left
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_right
	walk_faster_right
	step_end

Johto_MahoganyMart1F_Movement_83DF3F1:
	lock_facing_direction
	walk_fast_right
	walk_fast_right
	walk_fast_left
	walk_fast_left
	unlock_facing_direction
	step_end

Johto_MahoganyMart1F_Movement_83DF3F8:
	lock_facing_direction
	walk_fast_right
	walk_fast_left
	unlock_facing_direction
	step_end

Johto_MahoganyMart1F_Movement_83DF3FD:
	lock_facing_direction
	slide_right
	slide_right
	unlock_facing_direction
	step_end

Johto_MahoganyMart1F_Movement_83DF402:
	walk_down
	walk_right
	walk_right
	face_down
	step_end

Johto_MahoganyMart1F_Movement_83DF407:
	face_up
	delay_2
	face_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_right
	step_end

Johto_MahoganyMart1F_Movement_83DF411:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	face_right
	step_end

Johto_MahoganyMart1F_Movement_83DF418:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_MahoganyMart1F_Mart_83DF460:
	.2byte ITEM_PEWTER_CRUNCHIES
	.2byte ITEM_RAGE_CANDY_BAR
	.2byte ITEM_LAVA_COOKIE
	.2byte ITEM_OLD_GATEAU
	.2byte ITEM_CASTELIACONE
	.2byte ITEM_LUMIOSE_GALETTE
	.2byte ITEM_SHALOUR_SABLE
	.2byte ITEM_BIG_MALASADA
	.2byte ITEM_NONE

Johto_MahoganyMart1F_Text_83DF47E::
	.string "Please come back another time.\n"
	.string "The store is... closed\l"
	.string "right now.$"

Johto_MahoganyMart1F_Text_83DF4BF::
	.string "ROCKETS have hijacked my basement!$"

Johto_MahoganyMart1F_Text_83DF573::
	.string "LANCE: DRAGONITE, HYPER BEAM.$"

Johto_MahoganyMart1F_Text_83DF591::
	.string "What took you, {PLAYER}?\p"
	.string "Just as we thought, that strange\n"
	.string "radio signal is coming from here.$"

Johto_MahoganyMart1F_Text_83DF5E7::
	.string "Excellent work finding the stairs,\n"
	.string "DRAGONITE!$"

Johto_MahoganyMart1F_Text_83DF615::
	.string "LANCE: {PLAYER}, we should split up\n"
	.string "to check this place.\p"
	.string "I’ll go first.$"
