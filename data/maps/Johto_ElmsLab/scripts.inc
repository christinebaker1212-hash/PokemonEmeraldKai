Johto_ElmsLab_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_ElmsLab_EventScript_83E1BCE
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_ElmsLab_OnFrame_83E1BF5
	.byte 0

Johto_ElmsLab_OnFrame_83E1BF5:
	map_script_2 VAR_QZ_0x5004, 4, Johto_ElmsLab_EventScript_83E20AA
	.2byte 0

@ Johto_ElmsLab sign / bg event at 1,4
@ Johto_ElmsLab sign / bg event at 2,4
Johto_ElmsLab_EventScript_82DFEF0::
	msgbox Quetzal_Text_82E0BEC, MSGBOX_SIGN
	end

@ Johto_ElmsLab sign / bg event at 3,1
@ Johto_ElmsLab sign / bg event at 3,4
Johto_ElmsLab_EventScript_82DFFC8::
	msgbox Johto_ElmsLab_Text_82E0C31, MSGBOX_SIGN
	end

@ Johto_ElmsLab sign / bg event at 8,1
@ Johto_ElmsLab sign / bg event at 9,1
@ Johto_ElmsLab sign / bg event at 10,1
@ Johto_ElmsLab sign / bg event at 11,1
@ Johto_ElmsLab sign / bg event at 11,8
@ Johto_ElmsLab sign / bg event at 10,8
@ Johto_ElmsLab sign / bg event at 9,8
@ Johto_ElmsLab sign / bg event at 8,8
@ Johto_ElmsLab sign / bg event at 4,8
@ Johto_ElmsLab sign / bg event at 3,8
@ Johto_ElmsLab sign / bg event at 2,8
@ Johto_ElmsLab sign / bg event at 1,8
Johto_ElmsLab_EventScript_82DFFD1::
	msgbox Johto_ElmsLab_Text_82E0C6E, MSGBOX_SIGN
	end

Johto_ElmsLab_EventScript_83E1BCE::
	call Quetzal_EventScript_840EA62
	goto_if_eq VAR_QZ_0x5004, 6, Johto_ElmsLab_EventScript_83E1BFF
	goto_if_eq VAR_QZ_0x5004, 2, Johto_ElmsLab_EventScript_83E1BFF
	goto_if_eq VAR_QZ_0x5004, 3, Johto_ElmsLab_EventScript_83E1BFF
	end

Johto_ElmsLab_EventScript_83E1BFF::
	setobjectxyperm 2, 3, 2
	setobjectmovementtype 2, 7
	end

@ Johto_ElmsLab trigger at 5,7 (VAR_QZ_0x5004 == 0)
Johto_ElmsLab_EventScript_83E1C0B::
	lock
	applymovement LOCALID_PLAYER, Johto_ElmsLab_Movement_83E2349
	waitmovement 0
	goto Johto_ElmsLab_EventScript_83E1C2D

@ Johto_ElmsLab trigger at 7,7 (VAR_QZ_0x5004 == 0)
Johto_ElmsLab_EventScript_83E1C1C::
	lock
	applymovement LOCALID_PLAYER, Johto_ElmsLab_Movement_83E234B
	waitmovement 0
	goto Johto_ElmsLab_EventScript_83E1C2D

@ Johto_ElmsLab trigger at 6,7 (VAR_QZ_0x5004 == 0)
Johto_ElmsLab_EventScript_83E1C2D::
	lock
	applymovement LOCALID_PLAYER, Johto_ElmsLab_Movement_83E234D
	waitmovement 0
	goto Johto_ElmsLab_EventScript_83E1D7C

@ Johto_ElmsLab object 1 (local id 1, gfx 0x3EB4) at 3,10
Johto_ElmsLab_EventScript_83E1C3E::
	lock
	faceplayer
	msgbox Johto_ElmsLab_Text_83E3740, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_ElmsLab object 3 (local id 3, gfx 0x3EB3) at 9,9
Johto_ElmsLab_EventScript_83E1C4B::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5004, 5, Johto_ElmsLab_EventScript_83E1C63
	msgbox Johto_ElmsLab_Text_83E3772, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_83E1C63::
	msgbox Johto_ElmsLab_Text_83E37C8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_ElmsLab object 2 (local id 2, gfx 0x3EAA) at 6,3
Johto_ElmsLab_EventScript_83E1C6E::
	lock
	faceplayer
	special QzGiovanniStoryElmCheck
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_8401D20
	call_if_set FLAG_QZ_0x11D1, Johto_ElmsLab_EventScript_83E1F8C
	goto_if_set FLAG_QZ_0x106A, Johto_ElmsLab_EventScript_83E1D23
	goto_if_eq VAR_QZ_0x5004, 7, Johto_ElmsLab_EventScript_83E21B8
	goto_if_eq VAR_QZ_0x5004, 8, Johto_ElmsLab_EventScript_83E2210
	goto_if_eq VAR_QZ_0x5004, 9, Johto_ElmsLab_EventScript_83E2249
	goto_if_eq VAR_QZ_0x5004, 10, Johto_ElmsLab_EventScript_83E2256
	goto_if_unset FLAG_QZ_0x1080, Johto_ElmsLab_EventScript_83E1CE2
	goto_if_set FLAG_QZ_0x1034, Johto_ElmsLab_EventScript_83E233E
	call_if_unset FLAG_QZ_0x1034, Johto_ElmsLab_EventScript_83E22E4
	goto_if_ge VAR_QZ_0x500A, 1, Johto_ElmsLab_EventScript_83E22D9

Johto_ElmsLab_EventScript_83E1CE2::
	goto_if_eq VAR_QZ_0x5004, 5, Johto_ElmsLab_EventScript_83E21A4
	goto_if_ge VAR_QZ_0x5004, 6, Johto_ElmsLab_EventScript_83E21A4
	goto_if_eq VAR_QZ_0x5004, 2, Johto_ElmsLab_EventScript_83E2015
	goto_if_eq VAR_QZ_0x5004, 3, Johto_ElmsLab_EventScript_83E2015
	goto_if_set FLAG_QZ_0x11D1, Johto_ElmsLab_EventScript_83E1FC7
	goto_if_eq VAR_QZ_0x5004, 1, Johto_ElmsLab_EventScript_83E1E20
	end

Johto_ElmsLab_EventScript_83E1D23::
	msgbox Johto_ElmsLab_Text_83E1D2E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_83E1D7C::
	msgbox Johto_ElmsLab_Text_83E23AC, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_83E1DBA
	goto_if_eq VAR_RESULT, 0, Johto_ElmsLab_EventScript_83E1D9B
	end

Johto_ElmsLab_EventScript_83E1D9B::
	msgbox Johto_ElmsLab_Text_83E251C, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_83E1DBA
	goto_if_eq VAR_RESULT, 0, Johto_ElmsLab_EventScript_83E1D9B
	end

Johto_ElmsLab_EventScript_83E1DBA::
	msgbox Johto_ElmsLab_Text_83E24FB, MSGBOX_DEFAULT
	msgbox Johto_ElmsLab_Text_83E253B, MSGBOX_DEFAULT
	delay 0x1E
	playse SE_PC_LOGIN
	waitse
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F7
	delay 0x3C
	applymovement 2, Johto_ElmsLab_Movement_83E2353
	applymovement LOCALID_PLAYER, Johto_ElmsLab_Movement_83E2369
	waitmovement 0
	waitmovement 0
	msgbox Johto_ElmsLab_Text_83E25AD, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Johto_ElmsLab_Movement_83E2363
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_ElmsLab_Movement_83E236B
	waitmovement 0
	delay 0x1E
	msgbox Johto_ElmsLab_Text_83E25E9, MSGBOX_DEFAULT
	goto Johto_ElmsLab_EventScript_83E1E20

Johto_ElmsLab_EventScript_83E1E20::
	msgbox Johto_ElmsLab_Text_83E2711, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5004, 1
	release
	end

@ Johto_ElmsLab trigger at 5,8 (VAR_QZ_0x5004 == 1)
@ Johto_ElmsLab trigger at 6,8 (VAR_QZ_0x5004 == 1)
@ Johto_ElmsLab trigger at 7,8 (VAR_QZ_0x5004 == 1)
Johto_ElmsLab_EventScript_83E1E30::
	lock
	msgbox Johto_ElmsLab_Text_83E278F, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_ElmsLab_Movement_83E2351
	waitmovement 0
	release
	end

Johto_ElmsLab_EventScript_83E1E46::
	msgbox Johto_ElmsLab_Text_83E2392, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_ElmsLab object 4 (local id 4, gfx 0x3A9E) at 8,4
Johto_ElmsLab_EventScript_83E1E51::
	lock
	goto_if_set FLAG_QZ_0x11D1, Johto_ElmsLab_EventScript_83E1E46
	goto_if_eq VAR_QZ_0x5004, 1, Johto_ElmsLab_EventScript_83E1E71
	msgbox Johto_ElmsLab_Text_83E2392, MSGBOX_DEFAULT
	release
	closemessage
	end

Johto_ElmsLab_EventScript_83E1E71::
	setvar VAR_QZ_0x5000, 1
	goto Johto_ElmsLab_EventScript_83E1ECF

@ Johto_ElmsLab object 5 (local id 5, gfx 0x3A9E) at 9,4
Johto_ElmsLab_EventScript_83E1E7B::
	lock
	goto_if_set FLAG_QZ_0x11D1, Johto_ElmsLab_EventScript_83E1E46
	goto_if_eq VAR_QZ_0x5004, 1, Johto_ElmsLab_EventScript_83E1E9B
	msgbox Johto_ElmsLab_Text_83E2392, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_83E1E9B::
	setvar VAR_QZ_0x5000, 1
	goto Johto_ElmsLab_EventScript_83E1ECF

@ Johto_ElmsLab object 6 (local id 6, gfx 0x3A9E) at 10,4
Johto_ElmsLab_EventScript_83E1EA5::
	lock
	goto_if_set FLAG_QZ_0x11D1, Johto_ElmsLab_EventScript_83E1E46
	goto_if_eq VAR_QZ_0x5004, 1, Johto_ElmsLab_EventScript_83E1EC5
	msgbox Johto_ElmsLab_Text_83E2392, MSGBOX_DEFAULT
	release
	closemessage
	end

Johto_ElmsLab_EventScript_83E1EC5::
	setvar VAR_QZ_0x5000, 0
	goto Johto_ElmsLab_EventScript_83E1ECF

Johto_ElmsLab_EventScript_83E1ECF::
	special QzChooseStarterJohto
	waitstate
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8361B98
	copyvar VAR_TEMP_2, VAR_RESULT
	bufferspeciesname 0, VAR_TEMP_2
	bufferspeciesname 1, VAR_TEMP_2
	goto Johto_ElmsLab_EventScript_83E1EFC

Johto_ElmsLab_EventScript_83E1EFC::
	special QzGiveStarterJohto
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8361BCF
	removeobject VAR_LAST_TALKED
	msgbox Johto_ElmsLab_Text_83E2858, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x11D1
	playfanfare MUS_OBTAIN_ITEM
	bufferspeciesname 0, VAR_TEMP_2
	message Johto_ElmsLab_Text_83E2881
	waitfanfare
	call Quetzal_EventScript_8361ADB
	call Johto_ElmsLab_EventScript_83E1F77
	call Johto_ElmsLab_EventScript_83E1F8C
	setflag FLAG_QZ_0xA79
	setflag FLAG_QZ_0xA78
	setflag FLAG_QZ_0x149B
	specialvar VAR_RESULT, QzBufferLastReceivedMonForNickname
	goto_if_eq VAR_RESULT, 0, Johto_ElmsLab_EventScript_83E1F70
	specialvar VAR_RESULT, QzIsNuzlockeOption2D4Bit6
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_83E1FBC
	msgbox Johto_ElmsLab_Text_83E2909, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_83E1FB2

Johto_ElmsLab_EventScript_83E1F70::
	setvar VAR_UNUSED_0x8014, 0
	release
	end

Johto_ElmsLab_EventScript_83E1F77::
	msgbox Johto_ElmsLab_Text_83E2891, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	message Johto_ElmsLab_Text_83E28E7
	waitfanfare
	special EnableNationalPokedex
	return

Johto_ElmsLab_EventScript_83E1F8C::
	goto_if_set FLAG_QZ_0x1073, Johto_ElmsLab_EventScript_83E1FA8
	call Quetzal_EventScript_835FF25
	goto_if_ne VAR_0x8007, 1, Johto_ElmsLab_EventScript_83E1FA9
	setflag FLAG_QZ_0x1073

Johto_ElmsLab_EventScript_83E1FA8::
	return

Johto_ElmsLab_EventScript_83E1FA9::
	msgbox Quetzal_Text_8360DB1, MSGBOX_DEFAULT
	return

Johto_ElmsLab_EventScript_83E1FB2::
	call Quetzal_EventScript_8361AF2
	goto Johto_ElmsLab_EventScript_83E1F70

Johto_ElmsLab_EventScript_83E1FBC::
	message Johto_ElmsLab_Text_83E2909
	waitmessage
	goto Johto_ElmsLab_EventScript_83E1FB2

Johto_ElmsLab_EventScript_83E1FC7::
	setvar VAR_QZ_0x5004, 2
	setvar VAR_QZ_0x5003, 3
	msgbox Johto_ElmsLab_Text_83E2934, MSGBOX_DEFAULT
	msgbox Johto_ElmsLab_Text_83E2A18, MSGBOX_DEFAULT
	closemessage
	goto_if_eq VAR_FACING, 4, Johto_ElmsLab_EventScript_83E2001
	applymovement 2, Johto_ElmsLab_Movement_83E2358
	waitmovement 0
	clearflag FLAG_QZ_0x1072
	setvar VAR_QZ_0x5003, 3
	release
	end

Johto_ElmsLab_EventScript_83E2001::
	applymovement 2, Johto_ElmsLab_Movement_83E235D
	waitmovement 0
	clearflag FLAG_QZ_0x1072
	setvar VAR_QZ_0x5003, 3
	release
	end

Johto_ElmsLab_EventScript_83E2015::
	msgbox Johto_ElmsLab_Text_83E2934, MSGBOX_DEFAULT
	msgbox Johto_ElmsLab_Text_83E2A18, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8360831
	waitmovement 0
	closemessage
	release
	end

@ Johto_ElmsLab trigger at 6,12 (VAR_QZ_0x5004 == 2)
Johto_ElmsLab_EventScript_83E204E::
	lock
	applymovement 1, Johto_ElmsLab_Movement_83E236D
	waitmovement 0
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_ElmsLab_Text_83E3713, MSGBOX_DEFAULT
	closemessage
	setorcopyvar VAR_0x8000, ITEM_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1072
	setvar VAR_QZ_0x5004, 3
	applymovement 1, Johto_ElmsLab_Movement_83E2372
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	release
	end

Johto_ElmsLab_EventScript_83E2095::
	setflag FLAG_QZ_0x1070
	removeobject 5
	return

Johto_ElmsLab_EventScript_83E209C::
	setflag FLAG_QZ_0x1071
	removeobject 6
	return

Johto_ElmsLab_EventScript_83E20A3::
	setflag FLAG_QZ_0x106F
	removeobject 4
	return

Johto_ElmsLab_EventScript_83E20AA::
	lock
	setmetatile 7, 1, 0x32B, 1
	call_if_eq VAR_QZ_0x5000, 0, Johto_ElmsLab_EventScript_83E2095
	call_if_eq VAR_QZ_0x5000, 1, Johto_ElmsLab_EventScript_83E209C
	call_if_eq VAR_QZ_0x5000, 2, Johto_ElmsLab_EventScript_83E20A3
	applymovement LOCALID_PLAYER, Johto_ElmsLab_Movement_83E2378
	waitmovement 0
	setflag FLAG_QZ_0x1074
	turnobject 7, 3
	msgbox Johto_ElmsLab_Text_83E393B, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_83E2124
	goto_if_eq VAR_RESULT, 0, Johto_ElmsLab_EventScript_83E2105
	end

Johto_ElmsLab_EventScript_83E2105::
	msgbox Johto_ElmsLab_Text_83E3A4E, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_83E2124
	goto_if_eq VAR_RESULT, 0, Johto_ElmsLab_EventScript_83E2105
	end

Johto_ElmsLab_EventScript_83E2124::
	closemessage
	msgbox Johto_ElmsLab_Text_83E3A0D, MSGBOX_DEFAULT
	msgbox Johto_ElmsLab_Text_83E3A29, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Johto_ElmsLab_Movement_83E2383
	waitmovement 0
	removeobject 7
	applymovement LOCALID_PLAYER, Johto_ElmsLab_Movement_83E2389
	waitmovement 0
	msgbox Johto_ElmsLab_Text_83E2B4D, MSGBOX_DEFAULT
	msgbox Johto_ElmsLab_Text_83E2B97, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Johto_ElmsLab_Movement_83E238C
	waitmovement 0
	msgbox Johto_ElmsLab_Text_83E2BBE, MSGBOX_DEFAULT
	delay 0x14
	closemessage
	applymovement 2, Johto_ElmsLab_Movement_83E2390
	waitmovement 0
	msgbox Johto_ElmsLab_Text_83E2BC9, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5004, 5
	setvar VAR_QZ_0x5003, 4
	setflag FLAG_QZ_0x106C
	clearflag FLAG_QZ_0x1075
	setflag FLAG_QZ_0x14A0
	setflag FLAG_QZ_0x115F
	removeitem ITEM_MYSTERY_EGG, 1
	release
	end

Johto_ElmsLab_EventScript_83E21A4::
	lock
	faceplayer
	msgbox Johto_ElmsLab_Text_83E2DCB, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8360803
	release
	end

Johto_ElmsLab_EventScript_83E21B8::
	lock
	faceplayer
	msgbox Johto_ElmsLab_Text_83E3154, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_MASTER_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	msgbox Johto_ElmsLab_Text_83E31B5, MSGBOX_DEFAULT
	closemessage
	delay 0x14
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	delay 0x3C
	msgbox Johto_ElmsLab_Text_83E327E, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5004, 8
	setvar VAR_QZ_0x501A, 4
	setflag FLAG_QZ_0x10D5
	setflag FLAG_QZ_0x10D6
	clearflag FLAG_QZ_0x10AD
	release
	end

Johto_ElmsLab_EventScript_83E2210::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5006, 2, Johto_ElmsLab_EventScript_83E223E
	goto_if_eq VAR_QZ_0x5007, 2, Johto_ElmsLab_EventScript_83E2233
	msgbox Johto_ElmsLab_Text_83E337F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_83E2233::
	msgbox Johto_ElmsLab_Text_83E33CD, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_83E223E::
	msgbox Johto_ElmsLab_Text_83E3428, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_83E2249::
	lock
	faceplayer
	msgbox Johto_ElmsLab_Text_83E3485, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_83E2256::
	msgbox Johto_ElmsLab_Text_83E360F, MSGBOX_DEFAULT
	checkitem ITEM_SS_TICKET, 1
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_83E2285
	setorcopyvar VAR_0x8000, ITEM_SS_TICKET
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA

Johto_ElmsLab_EventScript_83E2285::
	msgbox Johto_ElmsLab_Text_83E3656, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5004, 0xB
	release
	end

@ Johto_ElmsLab trigger at 6,12 (VAR_QZ_0x5004 == 5)
Johto_ElmsLab_EventScript_83E2295::
	lock
	goto_if_set FLAG_QZ_0x1073, Johto_ElmsLab_EventScript_83E22D2
	applymovement 1, Johto_ElmsLab_Movement_83E236D
	waitmovement 0
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_ElmsLab_Text_83E38B5, MSGBOX_DEFAULT
	closemessage
	call Johto_ElmsLab_EventScript_83E1F8C
	applymovement 1, Johto_ElmsLab_Movement_83E2372
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	goto_if_unset FLAG_QZ_0x1073, Johto_ElmsLab_EventScript_83E22D7

Johto_ElmsLab_EventScript_83E22D2::
	setvar VAR_QZ_0x5004, 6

Johto_ElmsLab_EventScript_83E22D7::
	release
	end

Johto_ElmsLab_EventScript_83E22D9::
	msgbox Johto_ElmsLab_Text_83E2EB0, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_83E22E4::
	specialvar VAR_RESULT, QzPartyHasTogepiLineOrTogekiss
	goto_if_eq VAR_RESULT, 1, Johto_ElmsLab_EventScript_83E22F5
	return

Johto_ElmsLab_EventScript_83E22F5::
	msgbox Johto_ElmsLab_Text_83E2F09, MSGBOX_DEFAULT
	waitmessage
	msgbox Johto_ElmsLab_Text_83E2F22, MSGBOX_DEFAULT
	waitmessage
	msgbox Johto_ElmsLab_Text_83E2F38, MSGBOX_DEFAULT
	waitmessage
	msgbox Johto_ElmsLab_Text_83E2FBB, MSGBOX_DEFAULT
	waitmessage
	setorcopyvar VAR_0x8000, ITEM_EVERSTONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	msgbox Johto_ElmsLab_Text_83E3030, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x1034
	release
	end

Johto_ElmsLab_EventScript_83E233E::
	msgbox Johto_ElmsLab_Text_83E3A68, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsLab_EventScript_8401D20::
	msgbox Johto_ElmsLab_Text_8401E3B, MSGBOX_DEFAULT
	closemessage
	fadescreen 3
	clearflag FLAG_QZ_0x131F
	addobject 8
	setvar VAR_0x8004, 0xFF
	setvar VAR_0x8005, 8
	special QzObjectFaceObject
	setvar VAR_0x8004, 2
	special QzObjectFaceObject
	fadescreen 2
	playmoncry SPECIES_ARCEUS, 2
	waitmoncry
	msgbox Johto_ElmsLab_Text_8401E64, MSGBOX_DEFAULT
	fadescreen 3
	setflag FLAG_QZ_0x131F
	removeobject 8
	special QzGiovanniStoryElmDone
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject
	fadescreen 2
	release
	end

Johto_ElmsLab_Movement_83E2349:
	walk_right
	step_end

Johto_ElmsLab_Movement_83E234B:
	walk_left
	step_end

Johto_ElmsLab_Movement_83E234D:
	walk_up
	walk_up
	walk_up
	step_end

Johto_ElmsLab_Movement_83E2351:
	walk_up
	step_end

Johto_ElmsLab_Movement_83E2353:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_up
	step_end

Johto_ElmsLab_Movement_83E2358:
	walk_left
	walk_left
	walk_left
	walk_up
	step_end

Johto_ElmsLab_Movement_83E235D:
	walk_up
	walk_left
	walk_left
	walk_left
	face_up
	step_end

Johto_ElmsLab_Movement_83E2363:
	walk_fast_down
	walk_fast_right
	walk_fast_right
	walk_fast_right
	face_down
	step_end

Johto_ElmsLab_Movement_83E2369:
	face_left
	step_end

Johto_ElmsLab_Movement_83E236B:
	face_up
	step_end

Johto_ElmsLab_Movement_83E236D:
	walk_down
	walk_down
	walk_right
	walk_right
	step_end

Johto_ElmsLab_Movement_83E2372:
	walk_left
	walk_left
	walk_up
	walk_up
	face_down
	step_end

Johto_ElmsLab_Movement_83E2378:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_left
	walk_fast_up
	face_right
	step_end

Johto_ElmsLab_Movement_83E2383:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Johto_ElmsLab_Movement_83E2389:
	walk_right
	face_up
	step_end

Johto_ElmsLab_Movement_83E238C:
	lock_facing_direction
	walk_fast_up
	unlock_facing_direction
	step_end

Johto_ElmsLab_Movement_83E2390:
	walk_fast_down
	step_end

Johto_ElmsLab_Text_82E0C31::
	.string "It’s a PC used for research.\n"
	.string "Better not mess around with it.$"

Johto_ElmsLab_Text_82E0C6E::
	.string "It’s crammed with books on POKéMON.$"

Johto_ElmsLab_Text_83E1D2E::
	.string "{PLAYER}!\p"
	.string "What are you doing here?\p"
	.string "My aide is still waiting for you\n"
	.string "In VIOLET CITY!$"

Johto_ElmsLab_Text_83E2392::
	.string "Probably shouldn’t touch!$"

Johto_ElmsLab_Text_83E23AC::
	.string "ELM: {PLAYER}! There you are!\p"
	.string "I needed to ask you a favor.\p"
	.string "I’m conducting new POKéMON\n"
	.string "research right now. I was\l"
	.string "wondering if you could help\l"
	.string "me with it, {PLAYER}.\p"
	.string "You see…\p"
	.string "I’m writing a paper that I\n"
	.string "want to present at a conference.\p"
	.string "But there are some things I\n"
	.string "don’t quite understand yet.\p"
	.string "So!\p"
	.string "I’d like you to raise a\n"
	.string "POKéMON that I recently caught.$"

Johto_ElmsLab_Text_83E24FB::
	.string "Thanks, {PLAYER}!\n"
	.string "You’re a great help!$"

Johto_ElmsLab_Text_83E251C::
	.string "But… Please, I need your\n"
	.string "help!$"

Johto_ElmsLab_Text_83E253B::
	.string "When I announce my findings,\n"
	.string "I’m sure we’ll delve deeper\p"
	.string "into the many mysteries of\n"
	.string "POKéMON.\p"
	.string "You can count on it!$"

Johto_ElmsLab_Text_83E25AD::
	.string "Oh, hey! I got an e-mail!\p"
	.string "… … … … … … … …\n"
	.string "Hm… Uh-huh…\p"
	.string "Okay…$"

Johto_ElmsLab_Text_83E25E9::
	.string "Hey, listen.\p"
	.string "I have an acquaintance called\n"
	.string "MR. POKéMON.\p"
	.string "He keeps finding weird things and\n"
	.string "raving about his discoveries.\p"
	.string "Anyway, I just got an e-mail\n"
	.string "from him saying that this\l"
	.string "time it’s real.\p"
	.string "It is intriguing, but we’re\n"
	.string "busy with our POKéMON research...\p"
	.string "Wait!\n"
	.string "I know!\p"
	.string "{PLAYER}, can you go in our\n"
	.string "place?$"

Johto_ElmsLab_Text_83E2711::
	.string "I want you to raise one of\n"
	.string "the POKéMON contained in these BALLS.\p"
	.string "You’ll be that POKéMON’s\n"
	.string "first partner, {PLAYER}!\p"
	.string "Go on. Pick one!$"

Johto_ElmsLab_Text_83E278F::
	.string "ELM: Wait! Where are you\n"
	.string "going?$"

Johto_ElmsLab_Text_83E2858::
	.string "ELM: I think that’s a great\n"
	.string "POKéMON too!$"

Johto_ElmsLab_Text_83E2881::
	.string "{PLAYER} received {STR_VAR_1}!$"

Johto_ElmsLab_Text_83E2891::
	.string "ELM: Take this POKéDEX, too.\p"
	.string "Record the POKéMON you meet\n"
	.string "and catch some new partners!$"

Johto_ElmsLab_Text_83E28E7::
	.string "{PLAYER} received the\n"
	.string "NATIONAL POKéDEX!$"

Johto_ElmsLab_Text_83E2909::
	.string "Would you like to give your\n"
	.string "{STR_VAR_1} a nickname?$"

Johto_ElmsLab_Text_83E2934::
	.string "MR. POKéMON lives a little bit\n"
	.string "beyond CHERRYGROVE, the next\l"
	.string "city over.\p"
	.string "It’s almost a direct route\n"
	.string "there, so you can’t miss it.\p"
	.string "Take care, and come back if\n"
	.string "anything happens!$"

Johto_ElmsLab_Text_83E2A18::
	.string "If a wild POKéMON appears, let\n"
	.string "your POKéMON battle it!\p"
	.string "If your POKéMON is hurt, you\n"
	.string "should heal it with the machine\l"
	.string "to my right, or at the POKéCENTER.\p"
	.string "I’m counting on you!$"

Johto_ElmsLab_Text_83E2B4D::
	.string "ELM: {PLAYER}, this is terrible…\p"
	.string "Oh, yes, what was MR. POKéMON’s\n"
	.string "big discovery?$"

Johto_ElmsLab_Text_83E2B97::
	.string "{PLAYER} handed the MYSTERY EGG\n"
	.string "to PROF.ELM.$"

Johto_ElmsLab_Text_83E2BBE::
	.string "ELM: This?$"

Johto_ElmsLab_Text_83E2BC9::
	.string "But… Is it a POKéMON EGG?\p"
	.string "If it is, it is a great\n"
	.string "discovery!\p"
	.string "ELM: What?!?\p"
	.string "PROF.OAK gave you a POKéDEX?\p"
	.string "{PLAYER}, is that true?\n"
	.string "Th-that’s incredible!\p"
	.string "He is superb at seeing the\n"
	.string "potential of people as trainers.\p"
	.string "Wow, {PLAYER}. You may have what\n"
	.string "it takes to become the CHAMPION.\p"
	.string "You seem to be getting on\n"
	.string "great with POKéMON too.\p"
	.string "You should take the POKéMON\n"
	.string "GYM challenge.\p"
	.string "The closest GYM would be the\n"
	.string "one in VIOLET CITY.\p"
	.string "…{PLAYER}. The road to the\n"
	.string "championship will be a long one.\p"
	.string "Before you leave, make sure\n"
	.string "that you talk to your mom.$"

Johto_ElmsLab_Text_83E2DCB::
	.string "ELM: Don’t give up!\n"
	.string "I’ll call if I learn anything\l"
	.string "about that EGG!$"

Johto_ElmsLab_Text_83E2EB0::
	.string "ELM: Hey, has that EGG changed\n"
	.string "any?$"

Johto_ElmsLab_Text_83E2F09::
	.string "ELM: {PLAYER}, you look great!$"

Johto_ElmsLab_Text_83E2F22::
	.string "What? That POKéMON!?!$"

Johto_ElmsLab_Text_83E2F38::
	.string "The EGG hatched! So, POKéMON\n"
	.string "are born from EGGS…\p"
	.string "No, perhaps not all POKéMON\n"
	.string "are.\p"
	.string "Wow, there’s still a lot of\n"
	.string "research to be done.$"

Johto_ElmsLab_Text_83E2FBB::
	.string "Thanks, {PLAYER}! You’re helping\n"
	.string "unravel POKéMON mysteries for us!\p"
	.string "I want you to have this as a\n"
	.string "token of our appreciation.$"

Johto_ElmsLab_Text_83E3030::
	.string "That’s an EVERSTONE.\p"
	.string "Some species of POKéMON evolve\n"
	.string "when they grow to certain levels.\p"
	.string "A POKéMON holding the EVERSTONE\n"
	.string "won’t evolve.\p"
	.string "Give it to a POKéMON you don’t\n"
	.string "want to evolve.$"

Johto_ElmsLab_Text_83E3154::
	.string "ELM: Hi, {PLAYER}!\p"
	.string "Thanks to you, my research is\n"
	.string "going great!\p"
	.string "Take this as a token of my\n"
	.string "appreciation.$"

Johto_ElmsLab_Text_83E31B5::
	.string "The MASTER BALL is the best!\p"
	.string "It’s the ultimate BALL! It’ll\n"
	.string "catch any POKéMON without fail.\p"
	.string "It’s given only to recognized\n"
	.string "POKéMON researchers.\p"
	.string "I think you can make much better\n"
	.string "use of it than I can, {PLAYER}!$"

Johto_ElmsLab_Text_83E327E::
	.string "By the way...\n"
	.string "A group of beautiful KIMONO\l"
	.string "GIRLS were looking for you.\p"
	.string "They told me they are\n"
	.string "waiting for you at the\l"
	.string "ECRUTEAK DANCE THEATER.\p"
	.string "I know you’ve gone through many\n"
	.string "things since that EGG got you\l"
	.string "involved in all this...\p"
	.string "I hope you’ll carry through it.$"

Johto_ElmsLab_Text_83E337F::
	.string "PROF.ELM: The KIMONO\n"
	.string "GIRLS are waiting for you at\l"
	.string "the ECRUTEAK DANCE THEATER.$"

Johto_ElmsLab_Text_83E33CD::
	.string "PROF.ELM: The KIMONO GIRLS have\n"
	.string "gone to ECRUTEAK’s BELL TOWER.\p"
	.string "You had better follow them.$"

Johto_ElmsLab_Text_83E3428::
	.string "PROF.ELM: Now the KIMONO GIRLS have\n"
	.string "gone to the WHIRL ISLANDS?\p"
	.string "You still need to meet LUGIA.$"

Johto_ElmsLab_Text_83E3485::
	.string "PROF.ELM: Hahaha...\n"
	.string "Good job!\p"
	.string "That rare EGG has brought you\n"
	.string "a lot of trouble, huh?\p"
	.string "The KIMONO GIRLS said something\n"
	.string "about me?\p"
	.string "I have nothing to do with it.\p"
	.string "You made your decision and went\n"
	.string "through with it.\p"
	.string "Having done so much, you should\n"
	.string "be able to face the LEAGUE now!\p"
	.string "You should head for INDIGO\n"
	.string "PLATEAU!\p"
	.string "Do you know how to get there?\n"
	.string "Follow ROUTE 27 to ROUTE 26.\p"
	.string "Once there, go straight north!$"

Johto_ElmsLab_Text_83E360F::
	.string "ELM: {PLAYER}! I have a gift\n"
	.string "to celebrate your victory.\p"
	.string "It’s an S.S. Ticket!$"

Johto_ElmsLab_Text_83E3656::
	.string "Keep it to sail from OLIVINE to\n"
	.string "regions where you have already\l"
	.string "won the League.\p"
	.string "To begin another adventure, use\n"
	.string "New region in the START menu.$"

Johto_ElmsLab_Text_83E3713::
	.string "{PLAYER}, I want you to have\n"
	.string "this for your errand.$"

Johto_ElmsLab_Text_83E3740::
	.string "There are only three of us, so\n"
	.string "we’re always busy.$"

Johto_ElmsLab_Text_83E3772::
	.string "Don’t forget to save your game\n"
	.string "regularly!\p"
	.string "Press the START button, then\n"
	.string "SAVE the game!$"

Johto_ElmsLab_Text_83E37C8::
	.string "There was a loud noise\n"
	.string "outside…\p"
	.string "When we went to look,\n"
	.string "someone stole a POKéMON.\p"
	.string "It’s unbelievable that anyone\n"
	.string "would do that!\p"
	.string "…sigh… That stolen POKéMON.\p"
	.string "I wonder how it’s doing.\p"
	.string "They say a POKéMON raised by a\n"
	.string "bad person turns bad itself.$"

Johto_ElmsLab_Text_83E38B5::
	.string "{PLAYER}!\p"
	.string "Use these on your POKéDEX\n"
	.string "quest!$"

Johto_ElmsLab_Text_83E393B::
	.string "I heard a POKéMON was stolen\n"
	.string "here…\p"
	.string "I was just getting some\n"
	.string "information from PROF.ELM.\p"
	.string "Apparently, it was a young\n"
	.string "male with long, red hair…\p"
	.string "What?\p"
	.string "You battled a trainer like\n"
	.string "that?\p"
	.string "Did you happen to get his\n"
	.string "name?$"

Johto_ElmsLab_Text_83E3A0D::
	.string "Right! His name was SILVER.$"

Johto_ElmsLab_Text_83E3A29::
	.string "Thanks for helping my\n"
	.string "investigation!$"

Johto_ElmsLab_Text_83E3A4E::
	.string "Surely you got his name?!$"

Johto_ElmsLab_Text_83E3A68::
	.string "Ah, {PLAYER}!\p"
	.string "How’s your POKéDEX coming along?\n"
	.string "I’m busy now, but visit sometime\l"
	.string "and I can check your progress!$"

Johto_ElmsLab_Text_8401E3B::
	.string "ELM: This Egg belongs to no known era...$"

Johto_ElmsLab_Text_8401E64::
	.string "ELM: Good heavens!\n"
	.string "That Pokémon... is ARCEUS!$"
