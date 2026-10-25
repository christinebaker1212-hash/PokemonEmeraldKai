Johto_IcePathB2FBlackthornSide_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Johto_IcePathB2FBlackthornSide_EventScript_83DBA41
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IcePathB2FBlackthornSide_EventScript_83DBA47
	.byte 0

Johto_IcePathB2FBlackthornSide_EventScript_83DBA41::
	call Johto_IcePathB2FBlackthornSide_EventScript_83DBA52
	end

Johto_IcePathB2FBlackthornSide_EventScript_83DBA47::
	call Johto_IcePathB2FBlackthornSide_EventScript_83DBA52
	call Quetzal_EventScript_840EA62
	end

Johto_IcePathB2FBlackthornSide_EventScript_83DBA52::
	setvar VAR_QZ_0x4010, 0x1DC
	goto_if_set FLAG_QZ_0x1492, Johto_IcePathB2FBlackthornSide_EventScript_83DBA61
	return

Johto_IcePathB2FBlackthornSide_EventScript_83DBA61::
	setvar VAR_QZ_0x4010, 0x2D9
	return

@ Johto_IcePathB2FBlackthornSide trigger at 9,22 (VAR_TEMP_0 == 0)
Johto_IcePathB2FBlackthornSide_EventScript_83DBA67::
	goto_if_set FLAG_QZ_0x1016, Johto_IcePathB2FBlackthornSide_EventScript_83DBAB2
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	delay 0x3C
	waitse
	turnobject LOCALID_PLAYER, 3
	playmoncry SPECIES_SWINUB, 0
	applymovement 3, Johto_IcePathB2FBlackthornSide_Movement_83DBD11
	waitmovement 3
	delay 0x14
	playse SE_LEDGE
	applymovement 3, Johto_IcePathB2FBlackthornSide_Movement_83DBD13
	waitmovement 0
	setobjectxyperm 3, 3, 0x13
	setobjectmovementtype 3, 8
	setvar VAR_TEMP_0, 1
	end

Johto_IcePathB2FBlackthornSide_EventScript_83DBAB2::
	setvar VAR_TEMP_0, 1
	end

@ Johto_IcePathB2FBlackthornSide object 1 (local id 1, gfx 0xF0) at 22,21
Johto_IcePathB2FBlackthornSide_EventScript_83DBAB8::
	lockall
	faceplayer
	goto_if_set FLAG_QZ_0x1492, Johto_IcePathB2FBlackthornSide_EventScript_83DBB6B
	playmoncry SPECIES_SWINUB, 0
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBD18, MSGBOX_DEFAULT
	waitmoncry
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBD2A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_IcePathB2FBlackthornSide_EventScript_83DBC0A
	special Bag_ChooseBerry
	waitstate
	goto_if_eq VAR_ITEM_ID, 0, Johto_IcePathB2FBlackthornSide_EventScript_83DBBFA
	goto_if_ne VAR_ITEM_ID, 518, Johto_IcePathB2FBlackthornSide_EventScript_83DBC2A
	removeitem ITEM_ASPEAR_BERRY, 1
	goto_if_eq VAR_RESULT, 0, Johto_IcePathB2FBlackthornSide_EventScript_83DBBFA
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBD43, MSGBOX_DEFAULT
	playmoncry SPECIES_SWINUB, 0
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBD7C, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	playbgm MUS_EVOLUTION, 0
	callnative 0x80DF0BD
	fadescreen 3
	callnative 0x80DF0D9
	removeobject 1
	clearflag FLAG_QZ_0x1493
	setflag FLAG_QZ_0x1492
	setvar VAR_QZ_0x4010, 0x2D9
	addobject 1
	delay 0x18
	fadescreen 2
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBDAC, MSGBOX_DEFAULT

Johto_IcePathB2FBlackthornSide_EventScript_83DBB6B::
	playmoncry SPECIES_MAMOSWINE, 0
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBDFE, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	setwildbattle SPECIES_MAMOSWINE, 0x2D, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	special QzSetupIcePathLegendary_771
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Johto_IcePathB2FBlackthornSide_EventScript_83DBBB1
	goto_if_eq VAR_RESULT, 7, Johto_IcePathB2FBlackthornSide_EventScript_83DBBB1
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBE4A, MSGBOX_DEFAULT
	goto Johto_IcePathB2FBlackthornSide_EventScript_83DBBFA

Johto_IcePathB2FBlackthornSide_EventScript_83DBBB1::
	setflag FLAG_QZ_0x1493
	removeobject 1
	goto Johto_IcePathB2FBlackthornSide_EventScript_83DBBCF

@ Johto_IcePathB2FBlackthornSide sign / bg event at 22,21
Johto_IcePathB2FBlackthornSide_EventScript_83DBBBC::
	goto_if_unset FLAG_QZ_0x1493, Johto_IcePathB2FBlackthornSide_EventScript_83DBC09
	goto_if_set FLAG_QZ_0x114F, Johto_IcePathB2FBlackthornSide_EventScript_83DBC09
	lockall

Johto_IcePathB2FBlackthornSide_EventScript_83DBBCF::
	goto_if_set FLAG_QZ_0x114F, Johto_IcePathB2FBlackthornSide_EventScript_83DBBFA
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBE87, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x16A
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_IcePathB2FBlackthornSide_EventScript_83DBBFC
	setflag FLAG_QZ_0x114F

Johto_IcePathB2FBlackthornSide_EventScript_83DBBFA::
	releaseall
	end

Johto_IcePathB2FBlackthornSide_EventScript_83DBBFC::
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBEC8, MSGBOX_DEFAULT
	goto Johto_IcePathB2FBlackthornSide_EventScript_83DBBFA

Johto_IcePathB2FBlackthornSide_EventScript_83DBC09::
	end

Johto_IcePathB2FBlackthornSide_EventScript_83DBC0A::
	closemessage
	playse SE_M_BLIZZARD
	setweather 4
	doweather
	call_if_eq VAR_FACING, 1, Johto_IcePathB2FBlackthornSide_EventScript_83DBC37
	call_if_eq VAR_FACING, 4, Johto_IcePathB2FBlackthornSide_EventScript_83DBC49
	releaseall
	end

Johto_IcePathB2FBlackthornSide_EventScript_83DBC2A::
	msgbox Johto_IcePathB2FBlackthornSide_Text_83DBD64, MSGBOX_DEFAULT
	goto Johto_IcePathB2FBlackthornSide_EventScript_83DBC0A

Johto_IcePathB2FBlackthornSide_EventScript_83DBC37::
	applymovement 1, Johto_IcePathB2FBlackthornSide_Movement_83DBD0D
	applymovement LOCALID_PLAYER, Johto_IcePathB2FBlackthornSide_Movement_83DBD03
	waitmovement 0
	return

Johto_IcePathB2FBlackthornSide_EventScript_83DBC49::
	applymovement 1, Johto_IcePathB2FBlackthornSide_Movement_83DBD0F
	applymovement LOCALID_PLAYER, Johto_IcePathB2FBlackthornSide_Movement_83DBD08
	waitmovement 0
	return

Johto_IcePathB2FBlackthornSide_Movement_83DBD03:
	lock_facing_direction
	walk_up
	walk_up
	unlock_facing_direction
	step_end

Johto_IcePathB2FBlackthornSide_Movement_83DBD08:
	lock_facing_direction
	walk_left
	walk_left
	unlock_facing_direction
	step_end

Johto_IcePathB2FBlackthornSide_Movement_83DBD0D:
	jump_in_place_up
	step_end

Johto_IcePathB2FBlackthornSide_Movement_83DBD0F:
	jump_in_place_left
	step_end

Johto_IcePathB2FBlackthornSide_Movement_83DBD11:
	jump_in_place_up
	step_end

Johto_IcePathB2FBlackthornSide_Movement_83DBD13:
	jump_2_up
	jump_in_place_left_right
	jump_in_place_left_right
	face_left
	step_end

Johto_IcePathB2FBlackthornSide_Text_83DBD18::
	.string "SWINUB: Swiiiiii!$"

Johto_IcePathB2FBlackthornSide_Text_83DBD2A::
	.string "Give the SWINUB a BERRY?$"

Johto_IcePathB2FBlackthornSide_Text_83DBD43::
	.string "SWINUB enjoyed the ASPEAR BERRY!$"

Johto_IcePathB2FBlackthornSide_Text_83DBD64::
	.string "SWINUB doesn’t like it!$"

Johto_IcePathB2FBlackthornSide_Text_83DBD7C::
	.string "SWINUUUUUUUB!\p"
	.string "The whole cave begins\n"
	.string "to tremble!$"

Johto_IcePathB2FBlackthornSide_Text_83DBDAC::
	.string "SWINUB evolved straight\n"
	.string "into MAMOSWINE!\p"
	.string "An ancient power awakens\n"
	.string "beneath the ice!$"

Johto_IcePathB2FBlackthornSide_Text_83DBDFE::
	.string "MAMMOOOOOOOOTH!\p"
	.string "The mammoth pounds the ground\n"
	.string "and challenges you to battle!$"

Johto_IcePathB2FBlackthornSide_Text_83DBE4A::
	.string "The mammoth returns to its spot.\n"
	.string "Its challenge still stands!$"

Johto_IcePathB2FBlackthornSide_Text_83DBE87::
	.string "The tremors stop... A crystal\n"
	.string "gleams in the mammoth’s footprint.$"

Johto_IcePathB2FBlackthornSide_Text_83DBEC8::
	.string "The BAG is full.\p"
	.string "The crystal will stay here.\n"
	.string "Make room and come back for it.$"
