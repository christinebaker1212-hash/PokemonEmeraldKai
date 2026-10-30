Johto_TeamRocketBaseB2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_LOAD, Johto_TeamRocketBaseB2F_EventScript_83E983E
	.byte 0

Johto_TeamRocketBaseB2F_EventScript_83E983E::
	call_if_set FLAG_QZ_0x10C2, Quetzal_EventScript_83E9A0F
	call_if_eq VAR_QZ_0x500C, 13, Johto_TeamRocketBaseB2F_EventScript_83E9A46
	call_if_ge VAR_QZ_0x5025, 4, Johto_TeamRocketBaseB2F_EventScript_83E985E
	end

Johto_TeamRocketBaseB2F_EventScript_83E985E::
	setmetatile 0xD, 8, 0x303, 1
	setmetatile 0xD, 9, 0x303, 1
	setmetatile 0xD, 0xA, 0x303, 1
	setmetatile 0xD, 0xB, 0x303, 1
	setmetatile 0xD, 0xC, 0x303, 1
	setmetatile 0xD, 0xD, 0x303, 1
	setmetatile 0xD, 0xE, 0x303, 1
	setmetatile 0xD, 0xF, 0x303, 1
	setmetatile 0xE, 8, 0x303, 1
	setmetatile 0xE, 9, 0x303, 1
	setmetatile 0xE, 0xA, 0x303, 1
	setmetatile 0xE, 0xB, 0x303, 1
	setmetatile 0xE, 0xC, 0x303, 1
	setmetatile 0xE, 0xD, 0x303, 1
	setmetatile 0xE, 0xE, 0x303, 1
	setmetatile 0xE, 0xF, 0x303, 1
	setmetatile 0xF, 8, 0x303, 1
	setmetatile 0xF, 9, 0x303, 1
	setmetatile 0xF, 0xA, 0x303, 1
	setmetatile 0xF, 0xB, 0x303, 1
	setmetatile 0xF, 0xC, 0x303, 1
	setmetatile 0xF, 0xD, 0x303, 1
	setmetatile 0xF, 0xE, 0x303, 1
	setmetatile 0xF, 0xF, 0x303, 1
	setmetatile 0x10, 8, 0x303, 1
	setmetatile 0x10, 9, 0x303, 1
	setmetatile 0x10, 0xA, 0x303, 1
	setmetatile 0x10, 0xB, 0x303, 1
	setmetatile 0x10, 0xC, 0x303, 1
	setmetatile 0x10, 0xD, 0x303, 1
	setmetatile 0x10, 0xE, 0x303, 1
	setmetatile 0x10, 0xF, 0x303, 1
	setmetatile 0x11, 8, 0x303, 1
	setmetatile 0x11, 9, 0x303, 1
	setmetatile 0x11, 0xA, 0x303, 1
	setmetatile 0x11, 0xB, 0x303, 1
	setmetatile 0x11, 0xC, 0x303, 1
	setmetatile 0x11, 0xD, 0x303, 1
	setmetatile 0x11, 0xE, 0x303, 1
	setmetatile 0x11, 0xF, 0x303, 1
	setmetatile 0x12, 8, 0x303, 1
	setmetatile 0x12, 9, 0x303, 1
	setmetatile 0x12, 0xA, 0x303, 1
	setmetatile 0x12, 0xB, 0x303, 1
	setmetatile 0x12, 0xC, 0x303, 1
	setmetatile 0x12, 0xD, 0x303, 1
	setmetatile 0x12, 0xE, 0x303, 1
	setmetatile 0x12, 0xF, 0x303, 1
	return

Johto_TeamRocketBaseB2F_EventScript_83E9A46::
	setobjectxyperm 8, 0x14, 0x10
	setobjectxyperm 7, 0x13, 0x10
	setobjectmovementtype 8, 0x41
	setobjectmovementtype 7, 7
	return

@ Johto_TeamRocketBaseB2F trigger at 7,2 (VAR_QZ_0x500C == 9)
@ Johto_TeamRocketBaseB2F trigger at 7,3 (VAR_QZ_0x500C == 9)
@ Johto_TeamRocketBaseB2F trigger at 13,3 (VAR_QZ_0x500C == 9)
@ Johto_TeamRocketBaseB2F trigger at 13,2 (VAR_QZ_0x500C == 9)
Johto_TeamRocketBaseB2F_EventScript_83E9A5D::
	lock
	applymovement 15, Johto_TeamRocketBaseB2F_Movement_83E9A90
	waitmovement 0
	playmoncry SPECIES_MURKROW, 0
	msgbox Quetzal_Text_83EB7ED, MSGBOX_DEFAULT
	closemessage
	applymovement 15, Johto_TeamRocketBaseB2F_Movement_83E9A93
	waitmovement 0
	setflag FLAG_QZ_0x10CE
	removeobject 15
	clearflag FLAG_QZ_0x10D0
	setvar VAR_QZ_0x500C, 0xA
	release
	end

@ Johto_TeamRocketBaseB2F trigger at 14,24 (VAR_QZ_0x500C == 11)
@ Johto_TeamRocketBaseB2F trigger at 15,24 (VAR_QZ_0x500C == 11)
@ Johto_TeamRocketBaseB2F trigger at 16,24 (VAR_QZ_0x500C == 11)
@ Johto_TeamRocketBaseB2F trigger at 13,23 (VAR_QZ_0x500C == 11)
@ Johto_TeamRocketBaseB2F trigger at 13,22 (VAR_QZ_0x500C == 11)
@ Johto_TeamRocketBaseB2F trigger at 13,21 (VAR_QZ_0x500C == 11)
Johto_TeamRocketBaseB2F_EventScript_83E9AA0::
	lock
	applymovement 16, Johto_TeamRocketBaseB2F_Movement_83E9B17
	waitmovement 0
	playmoncry SPECIES_MURKROW, 0
	msgbox Johto_TeamRocketBaseB2F_Text_83EB847, MSGBOX_DEFAULT
	msgbox Johto_TeamRocketBaseB2F_Text_83E9B2C, MSGBOX_DEFAULT
	playse SE_CLICK
	setmetatile 0xE, 0x13, 0x378, 0
	setmetatile 0xE, 0x14, 0x380, 0
	setmetatile 0xF, 0x13, 0x379, 0
	setmetatile 0xF, 0x14, 0x379, 0
	setmetatile 0x10, 0x13, 0x379, 0
	setmetatile 0x10, 0x14, 0x379, 0
	special DrawWholeMapView
	setflag FLAG_QZ_0x10C2
	closemessage
	applymovement 16, Johto_TeamRocketBaseB2F_Movement_83E9B1A
	waitmovement 0
	setflag FLAG_QZ_0x10CF
	removeobject 16
	setvar VAR_QZ_0x500C, 0xC
	release
	end

Johto_TeamRocketBaseB2F_EventScript_83E9BAD::
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083B
	waitmovement 0
	return

Johto_TeamRocketBaseB2F_EventScript_83E9BB8::
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	return

@ Johto_TeamRocketBaseB2F trigger at 15,18 (VAR_QZ_0x500C == 12)
@ Johto_TeamRocketBaseB2F trigger at 14,18 (VAR_QZ_0x500C == 12)
@ Johto_TeamRocketBaseB2F trigger at 16,18 (VAR_QZ_0x500C == 12)
Johto_TeamRocketBaseB2F_EventScript_83E9BC3::
	lock
	setobjectxyperm 7, 6, 0x1A
	setobjectxyperm 8, 5, 0x1A
	clearflag FLAG_QZ_0x10D1
	addobject 14
	addobject 12
	delay 0x14
	msgbox Johto_TeamRocketBaseB2F_Text_83E9C83, MSGBOX_AUTOCLOSE
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	delay 0x3C
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 14, Johto_TeamRocketBaseB2F_EventScript_83E9BAD
	call_if_eq VAR_TEMP_0, 16, Johto_TeamRocketBaseB2F_EventScript_83E9BB8
	applymovement LOCALID_PLAYER, Johto_TeamRocketBaseB2F_Movement_83E9C6E
	applymovement 14, Johto_TeamRocketBaseB2F_Movement_83E9C72
	delay 0x14
	applymovement 12, Johto_TeamRocketBaseB2F_Movement_83E9C72
	waitmovement 12
	msgbox Johto_TeamRocketBaseB2F_Text_83E9CA0, MSGBOX_DEFAULT
	closemessage
	clearflag FLAG_QZ_0x10C1
	addobject 7
	addobject 8
	applymovement 7, Johto_TeamRocketBaseB2F_Movement_83E9C77
	delay 0x14
	applymovement 8, Johto_TeamRocketBaseB2F_Movement_83E9C77
	waitmovement 0
	msgbox Johto_TeamRocketBaseB2F_Text_83E9DFE, MSGBOX_DEFAULT
	closemessage
	applymovement 14, Quetzal_Movement_8360831
	msgbox Johto_TeamRocketBaseB2F_Text_83E9E7B, MSGBOX_DEFAULT
	closemessage
	goto Johto_TeamRocketBaseB2F_EventScript_83E9F48

@ Johto_TeamRocketBaseB2F object 14 (local id 14, gfx 0x3E81) at 15,27
Johto_TeamRocketBaseB2F_EventScript_83E9F48::
	msgbox Johto_TeamRocketBaseB2F_Text_83E9F20, MSGBOX_DEFAULT
	goto Johto_TeamRocketBaseB2F_EventScript_83E9F56

Johto_TeamRocketBaseB2F_EventScript_83E9F56::
	setflag FLAG_QZ_0x1158
	special SavePlayerParty
	fadescreen 1
	special QzBackupPartyForOptionBattle
	waitstate
	goto_if_ne VAR_RESULT, 0, Johto_TeamRocketBaseB2F_EventScript_83E9F75
	special LoadPlayerParty
	goto Johto_TeamRocketBaseB2F_EventScript_83E9F48

Johto_TeamRocketBaseB2F_EventScript_83E9F75::
	special ReducePlayerPartyToSelectedMons
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 4
	special CallFrontierUtilFunc
	setvar VAR_0x8004, 0xC
	setvar VAR_0x8005, 0
	applymovement 14, Quetzal_Movement_8360837
	applymovement 12, Quetzal_Movement_8360837
	waitmovement 0
	turnobject 12, 3
	special DoSpecialTrainerBattle
	waitstate
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 1, Johto_TeamRocketBaseB2F_EventScript_83E9FE2
	fadescreen 1
	setflag FLAG_QZ_0x10C1
	setflag FLAG_QZ_0x10D1
	special SetCB2WhiteOut
	waitstate
	trainerbattle TRAINER_BATTLE_SET_TRAINER_A, TRAINER_QZ_4223, 0, Johto_TeamRocketBaseB2F_Text_83EA26C, Johto_TeamRocketBaseB2F_Text_83EA26C
	end

Johto_TeamRocketBaseB2F_EventScript_83E9FE2::
	clearflag FLAG_QZ_0x1158
	msgbox Johto_TeamRocketBaseB2F_Text_83EA2D6, MSGBOX_DEFAULT
	closemessage
	fadescreenswapbuffers 1
	applymovement 7, Quetzal_Movement_8360837
	applymovement 8, Quetzal_Movement_8360837
	waitmovement 0
	applymovement 7, Quetzal_Movement_836083B
	applymovement 8, Quetzal_Movement_836083B
	waitmovement 0
	setflag FLAG_QZ_0x1051
	setflag FLAG_QZ_0x10D1
	removeobject 14
	removeobject 12
	turnobject LOCALID_PLAYER, 3
	fadescreenswapbuffers 0
	msgbox Johto_TeamRocketBaseB2F_Text_83EA3DB, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Johto_TeamRocketBaseB2F_Movement_83EA0A8
	applymovement 8, Johto_TeamRocketBaseB2F_Movement_83EA0AD
	delay 0x37
	applymovement LOCALID_PLAYER, Johto_TeamRocketBaseB2F_Movement_83EA0A8
	waitmovement 0
	applymovement 8, Johto_TeamRocketBaseB2F_Movement_83EA0CB
	applymovement 7, Johto_TeamRocketBaseB2F_Movement_83EA0B4
	delay 0xA5
	playmoncry SPECIES_DRAGONITE, 0
	waitmovement 8
	waitmovement 7
	msgbox Johto_TeamRocketBaseB2F_Text_83EA45C, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Johto_TeamRocketBaseB2F_Movement_83EA0DE
	applymovement 8, Johto_TeamRocketBaseB2F_Movement_83EA0DE
	waitmovement 8
	waitmovement 7
	applymovement LOCALID_PLAYER, Johto_TeamRocketBaseB2F_Movement_83EA0E4
	waitmovement 0
	setobjectxyperm 8, 0x14, 0x10
	setobjectxyperm 7, 0x13, 0x10
	setobjectmovementtype 8, 0x41
	setobjectmovementtype 7, 7
	setvar VAR_QZ_0x500C, 0xD
	release
	end

@ Johto_TeamRocketBaseB2F trigger at 14,17 (VAR_QZ_0x500C == 13)
@ Johto_TeamRocketBaseB2F trigger at 14,16 (VAR_QZ_0x500C == 13)
@ Johto_TeamRocketBaseB2F trigger at 14,18 (VAR_QZ_0x500C == 13)
Johto_TeamRocketBaseB2F_EventScript_83EA0E8::
	lock
	goto_if_eq VAR_QZ_0x5025, 3, Johto_TeamRocketBaseB2F_EventScript_83EA14E
	turnobject 7, 3
	msgbox Johto_TeamRocketBaseB2F_Text_83EA57F, MSGBOX_DEFAULT
	closemessage
	turnobject 7, 2
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	release
	end

@ Johto_TeamRocketBaseB2F object 4 (local id 4, gfx 0x165) at 11,9
@ Johto_TeamRocketBaseB2F object 5 (local id 5, gfx 0x165) at 11,12
@ Johto_TeamRocketBaseB2F object 6 (local id 6, gfx 0x165) at 11,15
Johto_TeamRocketBaseB2F_EventScript_83EA111::
	lock
	playmoncry SPECIES_ELECTRODE, 2
	delay 0x28
	waitmoncry
	setwildbattle SPECIES_ELECTRODE, 0x1E, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 4, Johto_TeamRocketBaseB2F_EventScript_83EA14C
	goto_if_eq VAR_RESULT, 5, Johto_TeamRocketBaseB2F_EventScript_83EA14C
	removeobject VAR_LAST_TALKED
	addvar VAR_QZ_0x5025, 1
	release
	end

Johto_TeamRocketBaseB2F_EventScript_83EA14C::
	release
	end

Johto_TeamRocketBaseB2F_EventScript_83EA14E::
	playse SE_M_HYPER_BEAM
	removeobject 3
	removeobject 2
	removeobject 1
	waitse
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_1, 16, Johto_TeamRocketBaseB2F_EventScript_83EA218
	call_if_eq VAR_TEMP_1, 18, Johto_TeamRocketBaseB2F_EventScript_83EA223
	turnobject LOCALID_PLAYER, 4
	applymovement 7, Johto_TeamRocketBaseB2F_Movement_83EA22E
	applymovement 8, Johto_TeamRocketBaseB2F_Movement_83EA22E
	waitmovement 8
	playse SE_CLICK
	turnobject LOCALID_PLAYER, 2
	turnobject 7, 2
	call Johto_TeamRocketBaseB2F_EventScript_83E985E
	special DrawWholeMapView
	delay 0x37
	turnobject 7, 3
	turnobject LOCALID_PLAYER, 4
	delay 0x1E
	msgbox Johto_TeamRocketBaseB2F_Text_83EA59D, MSGBOX_DEFAULT
	msgbox Johto_TeamRocketBaseB2F_Text_83EA67E, MSGBOX_DEFAULT
	closemessage
	turnobject 7, 1
	delay 0x14
	msgbox Johto_TeamRocketBaseB2F_Text_83EA71D, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Johto_TeamRocketBaseB2F_Movement_83EA234
	delay 0x14
	applymovement 8, Johto_TeamRocketBaseB2F_Movement_83EA245
	waitmovement 8
	waitmovement 7
	setflag FLAG_QZ_0x10C1
	removeobject 8
	removeobject 7
	setvar VAR_QZ_0x5025, 4
	setvar VAR_QZ_0x500C, 0xE
	clearflag FLAG_QZ_0x1050
	setflag FLAG_QZ_0x104E
	setvar VAR_QZ_0x501F, 2
	setvar VAR_QZ_0x5020, 2
	setvar VAR_QZ_0x5021, 2
	setvar VAR_QZ_0x5022, 2
	setvar VAR_QZ_0x5023, 2
	release
	end

Johto_TeamRocketBaseB2F_EventScript_83EA218::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement 0
	return

Johto_TeamRocketBaseB2F_EventScript_83EA223::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	return

@ Johto_TeamRocketBaseB2F object 13 (local id 13, gfx 0x3B) at 3,17
Johto_TeamRocketBaseB2F_EventScript_83EA8BE::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB2F trigger at 8,24 (VAR_QZ_0x500C == 5)
Johto_TeamRocketBaseB2F_EventScript_83EA8CB::
	lock
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083B
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	goto Johto_TeamRocketBaseB2F_EventScript_83EA8F3

@ Johto_TeamRocketBaseB2F trigger at 10,24 (VAR_QZ_0x500C == 5)
Johto_TeamRocketBaseB2F_EventScript_83EA8DF::
	lock
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	turnobject LOCALID_PLAYER, 2
	goto Johto_TeamRocketBaseB2F_EventScript_83EA8F3

@ Johto_TeamRocketBaseB2F trigger at 9,24 (VAR_QZ_0x500C == 5)
Johto_TeamRocketBaseB2F_EventScript_83EA8F3::
	lock
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	msgbox Johto_TeamRocketBaseB2F_Text_83EA95D, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_83602D9
	msgbox Johto_TeamRocketBaseB2F_Text_83EA9BD, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Johto_TeamRocketBaseB2F_Movement_83EA939
	applymovement 8, Johto_TeamRocketBaseB2F_Movement_83EA94A
	waitmovement 7
	waitmovement 8
	setflag FLAG_QZ_0x10C1
	removeobject 7
	setvar VAR_QZ_0x500C, 6
	removeobject 8
	release
	end

@ Johto_TeamRocketBaseB2F object 9 (local id 9, gfx 0x3EB0) at 25,26
Johto_TeamRocketBaseB2F_EventScript_83EAA94::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4474, 0, Johto_TeamRocketBaseB2F_Text_83EA9EC, Johto_TeamRocketBaseB2F_Text_83EAA43
	msgbox Johto_TeamRocketBaseB2F_Text_83EAA51, MSGBOX_AUTOCLOSE
	end

@ Johto_TeamRocketBaseB2F object 10 (local id 10, gfx 0x3EB0) at 29,18
Johto_TeamRocketBaseB2F_EventScript_83EAB90::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4504, 0, Johto_TeamRocketBaseB2F_Text_83EAAAB, Johto_TeamRocketBaseB2F_Text_83EAAEA
	msgbox Johto_TeamRocketBaseB2F_Text_83EAAFB, MSGBOX_AUTOCLOSE
	end

@ Johto_TeamRocketBaseB2F object 11 (local id 11, gfx 0x3EB0) at 5,2
Johto_TeamRocketBaseB2F_EventScript_83EAC2E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4141, 0, Johto_TeamRocketBaseB2F_Text_83EABA7, Johto_TeamRocketBaseB2F_Text_83EABBF
	msgbox Johto_TeamRocketBaseB2F_Text_83EABC6, MSGBOX_AUTOCLOSE
	end

Johto_TeamRocketBaseB2F_Movement_83E9A90:
	jump_in_place_left
	jump_in_place_left
	step_end

Johto_TeamRocketBaseB2F_Movement_83E9A93:
	jump_in_place_right
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

Johto_TeamRocketBaseB2F_Movement_83E9B17:
	jump_in_place_up
	jump_in_place_up
	step_end

Johto_TeamRocketBaseB2F_Movement_83E9B1A:
	jump_in_place_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_TeamRocketBaseB2F_Movement_83E9C6E:
	walk_down
	walk_down
	walk_down
	step_end

Johto_TeamRocketBaseB2F_Movement_83E9C72:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_TeamRocketBaseB2F_Movement_83E9C77:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA0A8:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA0AD:
	delay_8
	delay_8
	walk_right
	walk_up
	walk_up
	walk_up
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA0B4:
	walk_up
	walk_left
	face_up
	delay_8
	delay_8
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_up
	delay_8
	delay_8
	face_right
	delay_8
	delay_8
	delay_8
	delay_8
	walk_left
	walk_left
	walk_left
	face_down
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA0CB:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_left
	delay_16
	delay_16
	delay_8
	delay_8
	delay_8
	delay_8
	walk_left
	walk_left
	walk_left
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA0DE:
	walk_right
	walk_right
	walk_right
	walk_right
	face_up
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA0E4:
	walk_left
	walk_left
	walk_left
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA22E:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA234:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA245:
	walk_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA939:
	walk_right
	walk_right
	delay_2
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Johto_TeamRocketBaseB2F_Movement_83EA94A:
	delay_8
	delay_8
	delay_8
	delay_8
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	step_end

Johto_TeamRocketBaseB2F_Text_83E9B2C::
	.string "MURKROW screamed the password,\n"
	.string "mimicking the voice of PETREL.\p"
	.string "..................\n"
	.string "The voice-recognition system\l"
	.string "unlocked the door!$"

Johto_TeamRocketBaseB2F_Text_83E9C83::
	.string "ARIANA: Hold it right there!$"

Johto_TeamRocketBaseB2F_Text_83E9CA0::
	.string "ARIANA: We can’t let a brat like you\n"
	.string "do as you please forever.\p"
	.string "It’ll hurt Team Rocket’s pride over\n"
	.string "and over again. Making it lessen,\l"
	.string "you see.\p"
	.string "...So, it’s time to finish you.\n"
	.string "However strong you may be,\p"
	.string "if the two of us fight at the same\n"
	.string "time, you’ll have no chance of\l"
	.string "winning. Don’t you agree?\p"
	.string "Hee hee hee! Sorry, baby.\n"
	.string "Now, get ready to be thrashed.$"

Johto_TeamRocketBaseB2F_Text_83E9DFE::
	.string "LANCE: Hold it!\n"
	.string "A two-on-one battle?\p"
	.string "That is so unfair!\n"
	.string "Typical of Team Rocket to be so\l"
	.string "sneaky.\p"
	.string "Allow me to join the battle!$"

Johto_TeamRocketBaseB2F_Text_83E9E7B::
	.string "ARIANA: What? You had an accomplice?\n"
	.string "Where is your sense of dignity?\p"
	.string "As interim boss in place of GIOVANNI,\n"
	.string "I’ll show you how wrong it is to meddle\l"
	.string "with Team Rocket!$"

Johto_TeamRocketBaseB2F_Text_83E9F20::
	.string "LANCE: Quickly {PLAYER}, choose\n"
	.string "your POKéMON!$"

Johto_TeamRocketBaseB2F_Text_83EA26C::
	.string "Tch, you really are strong.\n"
	.string "It’s too bad.\p"
	.string "If you were to join Team Rocket,\n"
	.string "you could become an Executive.$"

Johto_TeamRocketBaseB2F_Text_83EA2D6::
	.string "ARIANA: But that’s fine.\n"
	.string "The broadcast experiment was a\l"
	.string "total success.\p"
	.string "It doesn’t matter what happens to\n"
	.string "this hideout now.\p"
	.string "We have much bigger plans.\n"
	.string "You’ll come to appreciate Team\l"
	.string "Rocket’s true power soon enough.\p"
	.string "Enjoy yourself while you can...\n"
	.string "Hee hee hee...$"

Johto_TeamRocketBaseB2F_Text_83EA3DB::
	.string "Sorry, {PLAYER}. I saw how well you\n"
	.string "were doing, so I just hung back.\p"
	.string "Now all there is left to do is to turn\n"
	.string "off that odd radio signal.$"

Johto_TeamRocketBaseB2F_Text_83EA45C::
	.string "It’s this machine that’s causing\n"
	.string "all the problems.\p"
	.string "I don’t see a switch on it…\p"
	.string "We have no choice. We have to\n"
	.string "make all the ELECTRODE faint.\p"
	.string "That should stop this machine\n"
	.string "from transmitting that strange\l"
	.string "signal.\p"
	.string "It’s no fault of the POKéMON, so\n"
	.string "it makes me feel guilty.\p"
	.string "{PLAYER}, let’s split the job.$"

Johto_TeamRocketBaseB2F_Text_83EA57F::
	.string "LANCE: Leave this side to me.$"

Johto_TeamRocketBaseB2F_Text_83EA59D::
	.string "LANCE: That odd signal has\n"
	.string "finally stopped.\p"
	.string "The LAKE should be back to\n"
	.string "normal.\p"
	.string "You’re the hero! Let me thank\n"
	.string "you on behalf of all the\l"
	.string "POKéMON.\p"
	.string "Oh, yes. You should take this.\n"
	.string "I found it here, but I don’t\l"
	.string "have any need for it.$"

Johto_TeamRocketBaseB2F_Text_83EA67E::
	.string "That’s WHIRLPOOL. Teach it to a\n"
	.string "POKéMON to get across wild water.\p"
	.string "But keep this in mind.\p"
	.string "You can use that out of battle\n"
	.string "only with the BADGE from\l"
	.string "MAHOGANY GYM.$"

Johto_TeamRocketBaseB2F_Text_83EA71D::
	.string "{PLAYER}…\p"
	.string "The journey to becoming the\n"
	.string "POKéMON MASTER is long and\l"
	.string "difficult.\p"
	.string "Knowing that, will you keep\n"
	.string "going?\p"
	.string "…\p"
	.string "I see. No, you’re right.\p"
	.string "If you would give up that easily,\n"
	.string "you would have never chased that\l"
	.string "dream in the first place.\p"
	.string "I look forward to seeing you\n"
	.string "again!$"

Johto_TeamRocketBaseB2F_Text_83EA95D::
	.string "LANCE: Are you all right?\p"
	.string "Your POKéMON are hurt and tired.\p"
	.string "Here, give them some of my\n"
	.string "medicine.$"

Johto_TeamRocketBaseB2F_Text_83EA9BD::
	.string "LANCE: {PLAYER}, let’s give it\n"
	.string "our best for POKéMON.$"

Johto_TeamRocketBaseB2F_Text_83EA9EC::
	.string "The door won’t open?\p"
	.string "Well, duh. It has to have a\n"
	.string "password that only TEAM ROCKET\l"
	.string "knows.$"

Johto_TeamRocketBaseB2F_Text_83EAA43::
	.string "What? I lost?$"

Johto_TeamRocketBaseB2F_Text_83EAA51::
	.string "Heh, I’m just a GRUNT.\p"
	.string "I don’t know the password.\n"
	.string "Too bad for you.$"

Johto_TeamRocketBaseB2F_Text_83EAAAB::
	.string "Oh, a kid? I don’t really like\n"
	.string "this, but eliminate you I must.$"

Johto_TeamRocketBaseB2F_Text_83EAAEA::
	.string "I knew I’d lose…$"

Johto_TeamRocketBaseB2F_Text_83EAAFB::
	.string "I got wiped out on the last\n"
	.string "mission by a kid too.\p"
	.string "When we were abducting POKéMON,\n"
	.string "this kid with long red hair and\l"
	.string "mean-looking eyes just\l"
	.string "creamed me…$"

Johto_TeamRocketBaseB2F_Text_83EABA7::
	.string "You rotten little pest!$"

Johto_TeamRocketBaseB2F_Text_83EABBF::
	.string "Grrrr…$"

Johto_TeamRocketBaseB2F_Text_83EABC6::
	.string "Heh, only the boss knows the\n"
	.string "password for that door.\p"
	.string "Where’s the boss? Who knows?\n"
	.string "Go look for yourself.$"

Johto_TeamRocketBaseB2F_Text_83EB847::
	.string "MURKROW: The password is…\p"
	.string "HAIL GIOVANNI.$"
