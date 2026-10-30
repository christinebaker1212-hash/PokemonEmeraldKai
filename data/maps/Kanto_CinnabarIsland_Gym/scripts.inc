Kanto_CinnabarIsland_Gym_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_CinnabarIsland_Gym_EventScript_841DB57
	.byte 0

Kanto_CinnabarIsland_Gym_EventScript_841DB57::
	goto_if_set FLAG_QZ_0x211A, Kanto_CinnabarIsland_Gym_EventScript_841DB97
	call_if_set FLAG_QZ_0x2113, Kanto_CinnabarIsland_Gym_EventScript_841DBB6
	call_if_set FLAG_QZ_0x2114, Kanto_CinnabarIsland_Gym_EventScript_841DBBC
	call_if_set FLAG_QZ_0x2115, Kanto_CinnabarIsland_Gym_EventScript_841DBC2
	call_if_set FLAG_QZ_0x2116, Kanto_CinnabarIsland_Gym_EventScript_841DBC8
	call_if_set FLAG_QZ_0x2117, Kanto_CinnabarIsland_Gym_EventScript_841DBCE
	call_if_set FLAG_QZ_0x2118, Kanto_CinnabarIsland_Gym_EventScript_841DBD4
	end

Kanto_CinnabarIsland_Gym_EventScript_841DB97::
	call Kanto_CinnabarIsland_Gym_EventScript_841E336
	call Kanto_CinnabarIsland_Gym_EventScript_841E376
	call Kanto_CinnabarIsland_Gym_EventScript_841E3B6
	call Kanto_CinnabarIsland_Gym_EventScript_841E3F6
	call Kanto_CinnabarIsland_Gym_EventScript_841E412
	call Kanto_CinnabarIsland_Gym_EventScript_841E452
	end

Kanto_CinnabarIsland_Gym_EventScript_841DBB6::
	call Kanto_CinnabarIsland_Gym_EventScript_841E336
	return

Kanto_CinnabarIsland_Gym_EventScript_841DBBC::
	call Kanto_CinnabarIsland_Gym_EventScript_841E376
	return

Kanto_CinnabarIsland_Gym_EventScript_841DBC2::
	call Kanto_CinnabarIsland_Gym_EventScript_841E3B6
	return

Kanto_CinnabarIsland_Gym_EventScript_841DBC8::
	call Kanto_CinnabarIsland_Gym_EventScript_841E3F6
	return

Kanto_CinnabarIsland_Gym_EventScript_841DBCE::
	call Kanto_CinnabarIsland_Gym_EventScript_841E412
	return

Kanto_CinnabarIsland_Gym_EventScript_841DBD4::
	call Kanto_CinnabarIsland_Gym_EventScript_841E452
	return

@ Kanto_CinnabarIsland_Gym object 8 (local id 8, gfx 0x3EEC) at 5,4
Kanto_CinnabarIsland_Gym_EventScript_841DBDA::
	setvar VAR_0x8004, 8
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_8611, 0, Kanto_CinnabarIsland_Gym_Text_844948B, Kanto_CinnabarIsland_Gym_Text_8449542, Kanto_CinnabarIsland_Gym_EventScript_841DC0C

Kanto_CinnabarIsland_Gym_EventScript_841DBF9::
	goto_if_unset FLAG_QZ_0x2172, Kanto_CinnabarIsland_Gym_EventScript_841DC4C
	msgbox Kanto_CinnabarIsland_Gym_Text_844965A, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DC0C::
	goto_if_set FLAG_QZ_0x2106, Kanto_CinnabarIsland_Gym_EventScript_841DBF9
	setvar VAR_0x8004, 8
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	setflag FLAG_QZ_0x211A
	call_if_unset FLAG_QZ_0x2106, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x2106
	setrespawn 0x28
	setvar VAR_QZ_0x510A, 1
	clearflag FLAG_QZ_0x217F
	setvar VAR_0x8008, 7
	call Quetzal_EventScript_8456FD2
	goto Kanto_CinnabarIsland_Gym_EventScript_841DC4C

Kanto_CinnabarIsland_Gym_EventScript_841DC4C::
	msgbox Kanto_CinnabarIsland_Gym_Text_84495E9, MSGBOX_DEFAULT
	checkitemspace ITEM_LIFE_ORB, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_Gym_EventScript_841DC94
	bufferitemname 0, ITEM_LIFE_ORB
	additem ITEM_LIFE_ORB, 1
	loadword 0, Kanto_CinnabarIsland_Gym_Text_844964A
	setorcopyvar VAR_0x8000, 0x1DF
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2172
	msgbox Kanto_CinnabarIsland_Gym_Text_844959A, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DC94::
	msgbox Kanto_CinnabarIsland_Gym_Text_84496CD, MSGBOX_DEFAULT
	release
	end

@ Kanto_CinnabarIsland_Gym object 2 (local id 2, gfx 0x3F43) at 25,4
Kanto_CinnabarIsland_Gym_EventScript_841DC9E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8369, 0, Kanto_CinnabarIsland_Gym_Text_84496E9, Kanto_CinnabarIsland_Gym_Text_844971A
	msgbox Kanto_CinnabarIsland_Gym_Text_844972E, MSGBOX_AUTOCLOSE
	end

@ Kanto_CinnabarIsland_Gym object 3 (local id 3, gfx 0x3F43) at 17,5
Kanto_CinnabarIsland_Gym_EventScript_841DCB5::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8370, 0, Kanto_CinnabarIsland_Gym_Text_8449801, Kanto_CinnabarIsland_Gym_Text_8449831, Kanto_CinnabarIsland_Gym_EventScript_841DCD0
	msgbox Kanto_CinnabarIsland_Gym_Text_8449855, MSGBOX_AUTOCLOSE
	end

Kanto_CinnabarIsland_Gym_EventScript_841DCD0::
	call_if_unset FLAG_QZ_0x2114, Kanto_CinnabarIsland_Gym_EventScript_841DCDB
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DCDB::
	call Kanto_CinnabarIsland_Gym_EventScript_841DF50
	return

@ Kanto_CinnabarIsland_Gym object 5 (local id 5, gfx 0x3F43) at 16,18
Kanto_CinnabarIsland_Gym_EventScript_841DCE1::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8371, 0, Kanto_CinnabarIsland_Gym_Text_84498FB, Kanto_CinnabarIsland_Gym_Text_844991F, Kanto_CinnabarIsland_Gym_EventScript_841DD09
	setvar VAR_0x8004, 8
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_CinnabarIsland_Gym_Text_8449923, MSGBOX_AUTOCLOSE
	end

Kanto_CinnabarIsland_Gym_EventScript_841DD09::
	call_if_unset FLAG_QZ_0x2116, Kanto_CinnabarIsland_Gym_EventScript_841DD14
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DD14::
	call Kanto_CinnabarIsland_Gym_EventScript_841E105
	return

@ Kanto_CinnabarIsland_Gym object 7 (local id 7, gfx 0x3F43) at 4,11
Kanto_CinnabarIsland_Gym_EventScript_841DD1A::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8372, 0, Kanto_CinnabarIsland_Gym_Text_8449A40, Kanto_CinnabarIsland_Gym_Text_8449A5A, Kanto_CinnabarIsland_Gym_EventScript_841DD35
	msgbox Kanto_CinnabarIsland_Gym_Text_8449A6B, MSGBOX_AUTOCLOSE
	end

Kanto_CinnabarIsland_Gym_EventScript_841DD35::
	call_if_unset FLAG_QZ_0x2118, Kanto_CinnabarIsland_Gym_EventScript_841DD40
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DD40::
	call Kanto_CinnabarIsland_Gym_EventScript_841E2B3
	return

@ Kanto_CinnabarIsland_Gym object 1 (local id 1, gfx 0x3F4B) at 25,11
Kanto_CinnabarIsland_Gym_EventScript_841DD46::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8405, 0, Kanto_CinnabarIsland_Gym_Text_8449783, Kanto_CinnabarIsland_Gym_Text_84497B4, Kanto_CinnabarIsland_Gym_EventScript_841DD61
	msgbox Kanto_CinnabarIsland_Gym_Text_84497C1, MSGBOX_AUTOCLOSE
	end

Kanto_CinnabarIsland_Gym_EventScript_841DD61::
	call_if_unset FLAG_QZ_0x2113, Kanto_CinnabarIsland_Gym_EventScript_841DD6C
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DD6C::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE77
	return

@ Kanto_CinnabarIsland_Gym object 4 (local id 4, gfx 0x3F4B) at 16,11
Kanto_CinnabarIsland_Gym_EventScript_841DD72::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8406, 0, Kanto_CinnabarIsland_Gym_Text_844988C, Kanto_CinnabarIsland_Gym_Text_84498B7, Kanto_CinnabarIsland_Gym_EventScript_841DD8D
	msgbox Kanto_CinnabarIsland_Gym_Text_84498CA, MSGBOX_AUTOCLOSE
	end

Kanto_CinnabarIsland_Gym_EventScript_841DD8D::
	call_if_unset FLAG_QZ_0x2115, Kanto_CinnabarIsland_Gym_EventScript_841DD98
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DD98::
	call Kanto_CinnabarIsland_Gym_EventScript_841E02E
	return

@ Kanto_CinnabarIsland_Gym object 6 (local id 6, gfx 0x3F4B) at 4,19
Kanto_CinnabarIsland_Gym_EventScript_841DD9E::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8407, 0, Kanto_CinnabarIsland_Gym_Text_84499C3, Kanto_CinnabarIsland_Gym_Text_84499FD, Kanto_CinnabarIsland_Gym_EventScript_841DDB9
	msgbox Kanto_CinnabarIsland_Gym_Text_8449A0D, MSGBOX_AUTOCLOSE
	end

Kanto_CinnabarIsland_Gym_EventScript_841DDB9::
	call_if_unset FLAG_QZ_0x2117, Kanto_CinnabarIsland_Gym_EventScript_841DDC4
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DDC4::
	call Kanto_CinnabarIsland_Gym_EventScript_841E1DC
	return

@ Kanto_CinnabarIsland_Gym object 9 (local id 9, gfx 0x3F0B) at 24,20
Kanto_CinnabarIsland_Gym_EventScript_841DDCA::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x211A, Kanto_CinnabarIsland_Gym_EventScript_841DDDF
	msgbox Kanto_CinnabarIsland_Gym_Text_8449AC5, MSGBOX_DEFAULT
	release
	end

Kanto_CinnabarIsland_Gym_EventScript_841DDDF::
	msgbox Kanto_CinnabarIsland_Gym_Text_8449B50, MSGBOX_DEFAULT
	release
	end

@ Kanto_CinnabarIsland_Gym sign / bg event at 23,20
@ Kanto_CinnabarIsland_Gym sign / bg event at 27,20
Kanto_CinnabarIsland_Gym_EventScript_841DDE9::
	lockall
	goto_if_set FLAG_QZ_0x2106, Kanto_CinnabarIsland_Gym_EventScript_841DDFD
	msgbox Kanto_CinnabarIsland_Gym_Text_8449B6D, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841DDFD::
	msgbox Kanto_CinnabarIsland_Gym_Text_8449BA8, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_CinnabarIsland_Gym sign / bg event at 22,10
Kanto_CinnabarIsland_Gym_EventScript_841DE07::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_CinnabarIsland_Gym_EventScript_841DE1F

@ Kanto_CinnabarIsland_Gym sign / bg event at 23,10
Kanto_CinnabarIsland_Gym_EventScript_841DE13::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_CinnabarIsland_Gym_EventScript_841DE1F

Kanto_CinnabarIsland_Gym_EventScript_841DE1F::
	msgbox Kanto_CinnabarIsland_Gym_Text_8449BE7, MSGBOX_DEFAULT
	msgbox Kanto_CinnabarIsland_Gym_Text_8449CA2, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_Gym_EventScript_841DE62
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_Gym_EventScript_841DE89
	end

Kanto_CinnabarIsland_Gym_EventScript_841DE46::
	playfanfare MUS_LEVEL_UP
	waitfanfare
	msgbox Kanto_CinnabarIsland_Gym_Text_8449D99, MSGBOX_DEFAULT
	return

Kanto_CinnabarIsland_Gym_EventScript_841DE53::
	waitse
	playse SE_BOO
	waitse
	msgbox Kanto_CinnabarIsland_Gym_Text_8449DC3, MSGBOX_DEFAULT
	closemessage
	return

Kanto_CinnabarIsland_Gym_EventScript_841DE62::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE46
	goto_if_set FLAG_QZ_0x2113, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841DE77
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841DE77::
	playse SE_UNLOCK
	waitse
	call Kanto_CinnabarIsland_Gym_EventScript_841E336
	special DrawWholeMapView
	setflag FLAG_QZ_0x2113
	return

Kanto_CinnabarIsland_Gym_EventScript_841DE87::
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841DE89::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE53
	checktrainerflag 0x20D5
	goto_if_lt Kanto_CinnabarIsland_Gym_EventScript_841DE99
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841DE99::
	call_if_eq VAR_TEMP_1, 0, Kanto_CinnabarIsland_Gym_EventScript_841DED3
	call_if_eq VAR_TEMP_1, 1, Kanto_CinnabarIsland_Gym_EventScript_841DEE5
	textcolor 0
	msgbox Kanto_CinnabarIsland_Gym_Text_8449783, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8405, 0, Kanto_CinnabarIsland_Gym_Text_84497B4
	goto_if_set FLAG_QZ_0x2113, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841DE77
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841DED3::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 1, Kanto_CinnabarIsland_Gym_Movement_841DEF7
	waitmovement 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841DEE5::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 1, Kanto_CinnabarIsland_Gym_Movement_841DEFA
	waitmovement 0
	return

@ Kanto_CinnabarIsland_Gym sign / bg event at 15,2
Kanto_CinnabarIsland_Gym_EventScript_841DEFC::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_CinnabarIsland_Gym_EventScript_841DF14

@ Kanto_CinnabarIsland_Gym sign / bg event at 16,2
Kanto_CinnabarIsland_Gym_EventScript_841DF08::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_CinnabarIsland_Gym_EventScript_841DF14

Kanto_CinnabarIsland_Gym_EventScript_841DF14::
	msgbox Kanto_CinnabarIsland_Gym_Text_8449BE7, MSGBOX_DEFAULT
	msgbox Kanto_CinnabarIsland_Gym_Text_8449CC1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_Gym_EventScript_841DF60
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_Gym_EventScript_841DF3B
	end

Kanto_CinnabarIsland_Gym_EventScript_841DF3B::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE46
	goto_if_set FLAG_QZ_0x2114, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841DF50
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841DF50::
	playse SE_UNLOCK
	waitse
	call Kanto_CinnabarIsland_Gym_EventScript_841E376
	special DrawWholeMapView
	setflag FLAG_QZ_0x2114
	return

Kanto_CinnabarIsland_Gym_EventScript_841DF60::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE53
	checktrainerflag 0x20B2
	goto_if_lt Kanto_CinnabarIsland_Gym_EventScript_841DF70
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841DF70::
	call_if_eq VAR_TEMP_1, 0, Kanto_CinnabarIsland_Gym_EventScript_841DFAA
	call_if_eq VAR_TEMP_1, 1, Kanto_CinnabarIsland_Gym_EventScript_841DFBC
	textcolor 0
	msgbox Kanto_CinnabarIsland_Gym_Text_8449801, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8370, 0, Kanto_CinnabarIsland_Gym_Text_8449831
	goto_if_set FLAG_QZ_0x2114, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841DF50
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841DFAA::
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_Gym_Movement_841DFD6
	applymovement 3, Kanto_CinnabarIsland_Gym_Movement_841DFCE
	waitmovement 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841DFBC::
	applymovement LOCALID_PLAYER, Kanto_CinnabarIsland_Gym_Movement_841DFD6
	applymovement 3, Kanto_CinnabarIsland_Gym_Movement_841DFD2
	waitmovement 0
	return

@ Kanto_CinnabarIsland_Gym sign / bg event at 13,10
Kanto_CinnabarIsland_Gym_EventScript_841DFDA::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_CinnabarIsland_Gym_EventScript_841DFF2

@ Kanto_CinnabarIsland_Gym sign / bg event at 14,10
Kanto_CinnabarIsland_Gym_EventScript_841DFE6::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_CinnabarIsland_Gym_EventScript_841DFF2

Kanto_CinnabarIsland_Gym_EventScript_841DFF2::
	msgbox Kanto_CinnabarIsland_Gym_Text_8449BE7, MSGBOX_DEFAULT
	msgbox Kanto_CinnabarIsland_Gym_Text_8449CF1, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_Gym_EventScript_841E03E
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_Gym_EventScript_841E019
	end

Kanto_CinnabarIsland_Gym_EventScript_841E019::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE46
	goto_if_set FLAG_QZ_0x2115, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841E02E
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E02E::
	playse SE_UNLOCK
	waitse
	call Kanto_CinnabarIsland_Gym_EventScript_841E3B6
	special DrawWholeMapView
	setflag FLAG_QZ_0x2115
	return

Kanto_CinnabarIsland_Gym_EventScript_841E03E::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE53
	checktrainerflag 0x20D6
	goto_if_lt Kanto_CinnabarIsland_Gym_EventScript_841E04E
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E04E::
	call_if_eq VAR_TEMP_1, 0, Kanto_CinnabarIsland_Gym_EventScript_841E088
	call_if_eq VAR_TEMP_1, 1, Kanto_CinnabarIsland_Gym_EventScript_841E09A
	textcolor 0
	msgbox Kanto_CinnabarIsland_Gym_Text_844988C, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8406, 0, Kanto_CinnabarIsland_Gym_Text_84498B7
	goto_if_set FLAG_QZ_0x2115, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841E02E
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E088::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 4, Kanto_CinnabarIsland_Gym_Movement_841E0AC
	waitmovement 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E09A::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 4, Kanto_CinnabarIsland_Gym_Movement_841E0AF
	waitmovement 0
	return

@ Kanto_CinnabarIsland_Gym sign / bg event at 13,17
Kanto_CinnabarIsland_Gym_EventScript_841E0B1::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_CinnabarIsland_Gym_EventScript_841E0C9

@ Kanto_CinnabarIsland_Gym sign / bg event at 14,17
Kanto_CinnabarIsland_Gym_EventScript_841E0BD::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_CinnabarIsland_Gym_EventScript_841E0C9

Kanto_CinnabarIsland_Gym_EventScript_841E0C9::
	msgbox Kanto_CinnabarIsland_Gym_Text_8449BE7, MSGBOX_DEFAULT
	msgbox Kanto_CinnabarIsland_Gym_Text_8449D0E, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_Gym_EventScript_841E115
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_Gym_EventScript_841E0F0
	end

Kanto_CinnabarIsland_Gym_EventScript_841E0F0::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE46
	goto_if_set FLAG_QZ_0x2116, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841E105
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E105::
	playse SE_UNLOCK
	waitse
	call Kanto_CinnabarIsland_Gym_EventScript_841E3F6
	special DrawWholeMapView
	setflag FLAG_QZ_0x2116
	return

Kanto_CinnabarIsland_Gym_EventScript_841E115::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE53
	checktrainerflag 0x20B3
	goto_if_lt Kanto_CinnabarIsland_Gym_EventScript_841E125
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E125::
	call_if_eq VAR_TEMP_1, 0, Kanto_CinnabarIsland_Gym_EventScript_841E15F
	call_if_eq VAR_TEMP_1, 1, Kanto_CinnabarIsland_Gym_EventScript_841E171
	textcolor 0
	msgbox Kanto_CinnabarIsland_Gym_Text_84498FB, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8371, 0, Kanto_CinnabarIsland_Gym_Text_844991F
	goto_if_set FLAG_QZ_0x2116, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841E105
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E15F::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 5, Kanto_CinnabarIsland_Gym_Movement_841E183
	waitmovement 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E171::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 5, Kanto_CinnabarIsland_Gym_Movement_841E186
	waitmovement 0
	return

@ Kanto_CinnabarIsland_Gym sign / bg event at 1,18
Kanto_CinnabarIsland_Gym_EventScript_841E188::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_CinnabarIsland_Gym_EventScript_841E1A0

@ Kanto_CinnabarIsland_Gym sign / bg event at 2,18
Kanto_CinnabarIsland_Gym_EventScript_841E194::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_CinnabarIsland_Gym_EventScript_841E1A0

Kanto_CinnabarIsland_Gym_EventScript_841E1A0::
	msgbox Kanto_CinnabarIsland_Gym_Text_8449BE7, MSGBOX_DEFAULT
	msgbox Kanto_CinnabarIsland_Gym_Text_8449D48, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_Gym_EventScript_841E1C7
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_Gym_EventScript_841E1EC
	end

Kanto_CinnabarIsland_Gym_EventScript_841E1C7::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE46
	goto_if_set FLAG_QZ_0x2117, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841E1DC
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E1DC::
	playse SE_UNLOCK
	waitse
	call Kanto_CinnabarIsland_Gym_EventScript_841E412
	special DrawWholeMapView
	setflag FLAG_QZ_0x2117
	return

Kanto_CinnabarIsland_Gym_EventScript_841E1EC::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE53
	checktrainerflag 0x20D7
	goto_if_lt Kanto_CinnabarIsland_Gym_EventScript_841E1FC
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E1FC::
	call_if_eq VAR_TEMP_1, 0, Kanto_CinnabarIsland_Gym_EventScript_841E236
	call_if_eq VAR_TEMP_1, 1, Kanto_CinnabarIsland_Gym_EventScript_841E248
	textcolor 0
	msgbox Kanto_CinnabarIsland_Gym_Text_84499C3, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8407, 0, Kanto_CinnabarIsland_Gym_Text_84499FD
	goto_if_set FLAG_QZ_0x2117, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841E1DC
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E236::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 6, Kanto_CinnabarIsland_Gym_Movement_841E25A
	waitmovement 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E248::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 6, Kanto_CinnabarIsland_Gym_Movement_841E25D
	waitmovement 0
	return

@ Kanto_CinnabarIsland_Gym sign / bg event at 1,10
Kanto_CinnabarIsland_Gym_EventScript_841E25F::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_CinnabarIsland_Gym_EventScript_841E277

@ Kanto_CinnabarIsland_Gym sign / bg event at 2,10
Kanto_CinnabarIsland_Gym_EventScript_841E26B::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_CinnabarIsland_Gym_EventScript_841E277

Kanto_CinnabarIsland_Gym_EventScript_841E277::
	msgbox Kanto_CinnabarIsland_Gym_Text_8449BE7, MSGBOX_DEFAULT
	msgbox Kanto_CinnabarIsland_Gym_Text_8449D7E, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_CinnabarIsland_Gym_EventScript_841E2C3
	goto_if_eq VAR_RESULT, 0, Kanto_CinnabarIsland_Gym_EventScript_841E29E
	end

Kanto_CinnabarIsland_Gym_EventScript_841E29E::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE46
	goto_if_set FLAG_QZ_0x2118, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841E2B3
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E2B3::
	playse SE_UNLOCK
	waitse
	call Kanto_CinnabarIsland_Gym_EventScript_841E452
	special DrawWholeMapView
	setflag FLAG_QZ_0x2118
	return

Kanto_CinnabarIsland_Gym_EventScript_841E2C3::
	call Kanto_CinnabarIsland_Gym_EventScript_841DE53
	checktrainerflag 0x20B4
	goto_if_lt Kanto_CinnabarIsland_Gym_EventScript_841E2D3
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E2D3::
	call_if_eq VAR_TEMP_1, 0, Kanto_CinnabarIsland_Gym_EventScript_841E30D
	call_if_eq VAR_TEMP_1, 1, Kanto_CinnabarIsland_Gym_EventScript_841E31F
	textcolor 0
	msgbox Kanto_CinnabarIsland_Gym_Text_8449A40, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8372, 0, Kanto_CinnabarIsland_Gym_Text_8449A5A
	goto_if_set FLAG_QZ_0x2118, Kanto_CinnabarIsland_Gym_EventScript_841DE87
	call Kanto_CinnabarIsland_Gym_EventScript_841E2B3
	releaseall
	end

Kanto_CinnabarIsland_Gym_EventScript_841E30D::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 7, Kanto_CinnabarIsland_Gym_Movement_841E331
	waitmovement 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E31F::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	applymovement 7, Kanto_CinnabarIsland_Gym_Movement_841E334
	waitmovement 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E336::
	setmetatile 0x1A, 8, 0x2C7, 0
	setmetatile 0x1B, 8, 0x2C6, 0
	setmetatile 0x1A, 9, 0x2CF, 0
	setmetatile 0x1B, 9, 0x2CE, 0
	setmetatile 0x1A, 0xA, 0x289, 0
	setmetatile 0x1B, 0xA, 0x281, 0
	setmetatile 0x1C, 0xA, 0x282, 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E376::
	setmetatile 0x11, 8, 0x2C7, 0
	setmetatile 0x12, 8, 0x2C6, 0
	setmetatile 0x11, 9, 0x2CF, 0
	setmetatile 0x12, 9, 0x2CE, 0
	setmetatile 0x11, 0xA, 0x289, 0
	setmetatile 0x12, 0xA, 0x281, 0
	setmetatile 0x13, 0xA, 0x282, 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E3B6::
	setmetatile 0x11, 0xF, 0x2C7, 0
	setmetatile 0x12, 0xF, 0x2C6, 0
	setmetatile 0x11, 0x10, 0x2CF, 0
	setmetatile 0x12, 0x10, 0x2CE, 0
	setmetatile 0x11, 0x11, 0x289, 0
	setmetatile 0x12, 0x11, 0x281, 0
	setmetatile 0x13, 0x11, 0x282, 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E3F6::
	setmetatile 0xB, 0x15, 0x2D1, 1
	setmetatile 0xB, 0x16, 0x289, 0
	setmetatile 0xB, 0x17, 0x281, 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E412::
	setmetatile 5, 0x10, 0x2C7, 0
	setmetatile 6, 0x10, 0x2C6, 0
	setmetatile 5, 0x11, 0x2CF, 0
	setmetatile 6, 0x11, 0x2CE, 0
	setmetatile 5, 0x12, 0x289, 0
	setmetatile 6, 0x12, 0x281, 0
	setmetatile 7, 0x12, 0x282, 0
	return

Kanto_CinnabarIsland_Gym_EventScript_841E452::
	setmetatile 5, 8, 0x2C7, 0
	setmetatile 6, 8, 0x2C6, 0
	setmetatile 5, 9, 0x2CF, 0
	setmetatile 6, 9, 0x2CE, 0
	setmetatile 5, 0xA, 0x289, 0
	setmetatile 6, 0xA, 0x281, 0
	setmetatile 7, 0xA, 0x282, 0
	return

@ Kanto_CinnabarIsland_Gym sign / bg event at 3,1
Kanto_CinnabarIsland_Gym_EventScript_845D2D8::
	lockall
	setvar VAR_0x8004, 8
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	setvar VAR_0x8004, 0xE
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	msgbox Kanto_CinnabarIsland_Gym_Text_8449DD4, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CinnabarIsland_Gym_Movement_841DEF7:
	walk_left
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841DEFA:
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841DFCE:
	walk_up
	walk_up
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841DFD2:
	walk_up
	walk_up
	walk_in_place_faster_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841DFD6:
	delay_16
	delay_16
	walk_in_place_faster_right
	step_end

Kanto_CinnabarIsland_Gym_Movement_841E0AC:
	walk_left
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841E0AF:
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841E183:
	walk_left
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841E186:
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841E25A:
	walk_left
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841E25D:
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841E331:
	walk_left
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Movement_841E334:
	walk_left
	step_end

Kanto_CinnabarIsland_Gym_Text_844948B::
	.string "Hah!\p"
	.string "I am BLAINE, the red-hot LEADER\n"
	.string "of CINNABAR GYM!\p"
	.string "My fiery POKéMON are all rough\n"
	.string "and ready with intense heat!\p"
	.string "They incinerate all challengers!\p"
	.string "Hah!\n"
	.string "You better have BURN HEAL!{PLAY_BGM 0x022A}$"

Kanto_CinnabarIsland_Gym_Text_8449542::
	.string "I have burned down to nothing!\n"
	.string "Not even ashes remain!\p"
	.string "You have earned the VOLCANOBADGE.$"

Kanto_CinnabarIsland_Gym_Text_844959A::
	.string "It makes the holder’s attacks\n"
	.string "stronger, but costs a little HP\l"
	.string "when it attacks.$"

Kanto_CinnabarIsland_Gym_Text_84495E9::
	.string "Hah!\p"
	.string "The VOLCANOBADGE heightens the\n"
	.string "SPECIAL stats of your POKéMON.\p"
	.string "Here, you can have this, too!$"

Kanto_CinnabarIsland_Gym_Text_844964A::
	.string "{PLAYER} received\n"
	.string "{STR_VAR_1}!$"

Kanto_CinnabarIsland_Gym_Text_844965A::
	.string "That item will make your attacks\n"
	.string "even more powerful!\p"
	.string "But watch your POKéMON’s HP.\n"
	.string "All that power comes at a price!$"

Kanto_CinnabarIsland_Gym_Text_84496CD::
	.string "Make space for this, child!$"

Kanto_CinnabarIsland_Gym_Text_84496E9::
	.string "Do you know how hot POKéMON fire\n"
	.string "breath can get?$"

Kanto_CinnabarIsland_Gym_Text_844971A::
	.string "Yow!\n"
	.string "Hot, hot, hot!$"

Kanto_CinnabarIsland_Gym_Text_844972E::
	.string "Fire, or to be more precise,\n"
	.string "combustion…\p"
	.string "…Oxygen in the air…\n"
	.string "Blah, blah, blah, blah…$"

Kanto_CinnabarIsland_Gym_Text_8449783::
	.string "I was a thief, but I went straight\n"
	.string "as a TRAINER.$"

Kanto_CinnabarIsland_Gym_Text_84497B4::
	.string "I surrender!$"

Kanto_CinnabarIsland_Gym_Text_84497C1::
	.string "I get this uncontrollable urge to\n"
	.string "steal other people’s POKéMON.$"

Kanto_CinnabarIsland_Gym_Text_8449801::
	.string "I’ve studied POKéMON thoroughly.\n"
	.string "You can’t win!$"

Kanto_CinnabarIsland_Gym_Text_8449831::
	.string "Waah!\n"
	.string "My studies were insufficient!$"

Kanto_CinnabarIsland_Gym_Text_8449855::
	.string "My theories are too complicated\n"
	.string "for you to understand.$"

Kanto_CinnabarIsland_Gym_Text_844988C::
	.string "I like using FIRE-type POKéMON.\n"
	.string "I just do.$"

Kanto_CinnabarIsland_Gym_Text_84498B7::
	.string "Too hot to handle!$"

Kanto_CinnabarIsland_Gym_Text_84498CA::
	.string "I wish there were a thief POKéMON.\n"
	.string "I’d use that!$"

Kanto_CinnabarIsland_Gym_Text_84498FB::
	.string "I know why BLAINE became a\n"
	.string "TRAINER.$"

Kanto_CinnabarIsland_Gym_Text_844991F::
	.string "Ow!$"

Kanto_CinnabarIsland_Gym_Text_8449923::
	.string "Our LEADER, BLAINE, became lost\n"
	.string "in the mountains but good.\p"
	.string "Night fell when a fiery bird\n"
	.string "POKéMON appeared.\p"
	.string "Its light allowed BLAINE to find\n"
	.string "his way down safely.$"

Kanto_CinnabarIsland_Gym_Text_84499C3::
	.string "I’ve been to many GYMS, but this\n"
	.string "one best suits my style.$"

Kanto_CinnabarIsland_Gym_Text_84499FD::
	.string "Yowza!\n"
	.string "Too hot!$"

Kanto_CinnabarIsland_Gym_Text_8449A0D::
	.string "PONYTA, NINETALES…\n"
	.string "Those are popular FIRE POKéMON.$"

Kanto_CinnabarIsland_Gym_Text_8449A40::
	.string "Fire is weak against H2O.$"

Kanto_CinnabarIsland_Gym_Text_8449A5A::
	.string "Oh!\n"
	.string "Snuffed out!$"

Kanto_CinnabarIsland_Gym_Text_8449A6B::
	.string "Water beats fire, of course.\p"
	.string "But fire melts ice, so the FIRE\n"
	.string "type beats ICE-type POKéMON.$"

Kanto_CinnabarIsland_Gym_Text_8449AC5::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "The hotheaded BLAINE is a FIRE\n"
	.string "POKéMON pro.\p"
	.string "Douse his spirits with water!\p"
	.string "You’d better take some\n"
	.string "BURN HEALS, too.$"

Kanto_CinnabarIsland_Gym_Text_8449B50::
	.string "{PLAYER}!\n"
	.string "You beat that firebrand!$"

Kanto_CinnabarIsland_Gym_Text_8449B6D::
	.string "CINNABAR POKéMON GYM\n"
	.string "LEADER: BLAINE\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE$"

Kanto_CinnabarIsland_Gym_Text_8449BA8::
	.string "CINNABAR POKéMON GYM\n"
	.string "LEADER: BLAINE\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE, {PLAYER}$"

Kanto_CinnabarIsland_Gym_Text_8449BE7::
	.string "POKéMON Quiz!\p"
	.string "Get it right, and the door opens to\n"
	.string "the next room.\p"
	.string "Get it wrong, and face a TRAINER!\p"
	.string "If you want to conserve your\n"
	.string "POKéMON for the GYM LEADER…\p"
	.string "Then get it right!\n"
	.string "Here we go!$"

Kanto_CinnabarIsland_Gym_Text_8449CA2::
	.string "CATERPIE evolves into METAPOD?$"

Kanto_CinnabarIsland_Gym_Text_8449CC1::
	.string "There are nine certified POKéMON\n"
	.string "LEAGUE BADGES?$"

Kanto_CinnabarIsland_Gym_Text_8449CF1::
	.string "POLIWAG evolves three times?$"

Kanto_CinnabarIsland_Gym_Text_8449D0E::
	.string "Are electric moves effective\n"
	.string "against GROUND-type POKéMON?$"

Kanto_CinnabarIsland_Gym_Text_8449D48::
	.string "POKéMON of the same kind and level\n"
	.string "are not identical?$"

Kanto_CinnabarIsland_Gym_Text_8449D7E::
	.string "The move TOMBSTONY exists?$"

Kanto_CinnabarIsland_Gym_Text_8449D99::
	.string "You’re absolutely correct!\p"
	.string "Go on through!$"

Kanto_CinnabarIsland_Gym_Text_8449DC3::
	.string "Sorry!\n"
	.string "Bad call!$"

Kanto_CinnabarIsland_Gym_Text_8449DD4::
	.string "It’s a photo of BLAINE and\n"
	.string "MR. FUJI.\p"
	.string "They’re standing shoulder to\n"
	.string "shoulder with big grins.$"
