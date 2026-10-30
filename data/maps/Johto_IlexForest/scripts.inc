Johto_IlexForest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IlexForest_EventScript_83DBFA0
	.byte 0

Johto_IlexForest_EventScript_83DBFA0::
	call_if_eq VAR_QZ_0x5009, 9, Johto_IlexForest_EventScript_83DC3C1
	call Quetzal_EventScript_840EA62
	call_if_eq VAR_QZ_0x5014, 1, Johto_IlexForest_EventScript_83DBFC7
	call_if_eq VAR_QZ_0x5014, 2, Johto_IlexForest_EventScript_83DBFCF
	end

Johto_IlexForest_EventScript_83DBFC7::
	setobjectxyperm 3, 0x21, 0x33
	return

Johto_IlexForest_EventScript_83DBFCF::
	clearflag FLAG_QZ_0x1094
	clearflag FLAG_QZ_0x1093
	setobjectxyperm 3, 0x1C, 0x2A
	return

@ Johto_IlexForest object 2 (local id 2, gfx 0xB) at 27,42
Johto_IlexForest_EventScript_83DBFDD::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5014, 2, Johto_IlexForest_EventScript_83DC000
	goto_if_eq VAR_QZ_0x5014, 3, Johto_IlexForest_EventScript_83DC000
	msgbox Johto_IlexForest_Text_83DC5EA, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_IlexForest_EventScript_83DC000::
	msgbox Johto_IlexForest_Text_83DC6A8, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_IlexForest object 1 (local id 1, gfx 0x2C) at 26,42
Johto_IlexForest_EventScript_83DC00B::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x1008, Johto_IlexForest_EventScript_83DC01F
	goto_if_unset FLAG_QZ_0x1030, Johto_IlexForest_EventScript_83DC20F

Johto_IlexForest_EventScript_83DC01F::
	msgbox Johto_IlexForest_Text_83DC8AA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_IlexForest object 3 (local id 3, gfx 0x153) at 42,47
Johto_IlexForest_EventScript_83DC02A::
	lock
	faceplayer
	playmoncry SPECIES_FARFETCHD, 0
	msgbox Johto_IlexForest_Text_83DC71A, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	goto_if_eq VAR_QZ_0x5014, 3, Johto_IlexForest_EventScript_83DC1CD
	goto_if_eq VAR_QZ_0x5014, 2, Johto_IlexForest_EventScript_83DC1CD
	goto_if_eq VAR_QZ_0x5014, 1, Johto_IlexForest_EventScript_83DC17B
	goto_if_eq VAR_FACING, 1, Johto_IlexForest_EventScript_83DC07F
	applymovement 3, Johto_IlexForest_Movement_83DC539
	waitmovement 0
	setflag FLAG_QZ_0x108F
	removeobject 3
	clearflag FLAG_QZ_0x1090
	addobject 13
	release
	end

Johto_IlexForest_EventScript_83DC07F::
	applymovement 3, Johto_IlexForest_Movement_83DC540
	waitmovement 0
	setvar VAR_QZ_0x5014, 1
	setobjectxyperm 3, 0x21, 0x33
	release
	end

@ Johto_IlexForest object 13 (local id 13, gfx 0x153) at 42,39
Johto_IlexForest_EventScript_83DC097::
	lock
	faceplayer
	playmoncry SPECIES_FARFETCHD, 0
	msgbox Johto_IlexForest_Text_83DC71A, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	goto_if_eq VAR_FACING, 1, Johto_IlexForest_EventScript_83DC0CB
	applymovement 13, Johto_IlexForest_Movement_83DC54E
	waitmovement 0
	setflag FLAG_QZ_0x1090
	removeobject 13
	clearflag FLAG_QZ_0x1091
	addobject 15
	release
	end

Johto_IlexForest_EventScript_83DC0CB::
	applymovement 13, Johto_IlexForest_Movement_83DC558
	waitmovement 0
	setflag FLAG_QZ_0x1090
	removeobject 13
	clearflag FLAG_QZ_0x108F
	addobject 3
	release
	end

@ Johto_IlexForest object 15 (local id 15, gfx 0x153) at 64,40
Johto_IlexForest_EventScript_83DC0E3::
	lock
	faceplayer
	playmoncry SPECIES_FARFETCHD, 0
	msgbox Johto_IlexForest_Text_83DC71A, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	goto_if_eq VAR_FACING, 1, Johto_IlexForest_EventScript_83DC117
	applymovement 15, Johto_IlexForest_Movement_83DC55F
	waitmovement 0
	setflag FLAG_QZ_0x1091
	removeobject 15
	clearflag FLAG_QZ_0x1090
	addobject 13
	release
	end

Johto_IlexForest_EventScript_83DC117::
	applymovement 15, Johto_IlexForest_Movement_83DC569
	waitmovement 0
	setflag FLAG_QZ_0x1091
	removeobject 15
	clearflag FLAG_QZ_0x1092
	addobject 14
	release
	end

@ Johto_IlexForest object 14 (local id 14, gfx 0x153) at 61,49
Johto_IlexForest_EventScript_83DC12F::
	lock
	faceplayer
	playmoncry SPECIES_FARFETCHD, 0
	msgbox Johto_IlexForest_Text_83DC71A, MSGBOX_DEFAULT
	waitmoncry
	closemessage
	goto_if_eq VAR_FACING, 1, Johto_IlexForest_EventScript_83DC163
	applymovement 14, Johto_IlexForest_Movement_83DC570
	waitmovement 0
	setflag FLAG_QZ_0x1092
	removeobject 14
	clearflag FLAG_QZ_0x1091
	addobject 15
	release
	end

Johto_IlexForest_EventScript_83DC163::
	applymovement 14, Johto_IlexForest_Movement_83DC57A
	waitmovement 0
	setflag FLAG_QZ_0x1092
	removeobject 14
	clearflag FLAG_QZ_0x108F
	addobject 3
	release
	end

Johto_IlexForest_EventScript_83DC17B::
	goto_if_eq VAR_FACING, 4, Johto_IlexForest_EventScript_83DC1B5
	goto_if_eq VAR_FACING, 1, Johto_IlexForest_EventScript_83DC1B5
	applymovement 3, Johto_IlexForest_Movement_83DC584
	waitmovement 0
	setobjectxyperm 3, 0x1C, 0x2A
	setvar VAR_QZ_0x5014, 2
	clearflag FLAG_QZ_0x1095
	clearflag FLAG_QZ_0x1094
	setflag FLAG_QZ_0x106B
	addobject 1
	release
	end

Johto_IlexForest_EventScript_83DC1B5::
	setvar VAR_QZ_0x5014, 0
	applymovement 3, Johto_IlexForest_Movement_83DC594
	waitmovement 0
	setobjectxyperm 3, 0x2A, 0x2F
	release
	end

Johto_IlexForest_EventScript_83DC1CD::
	release
	end

@ Johto_IlexForest trigger at 26,44 (VAR_QZ_0x5014 == 2)
Johto_IlexForest_EventScript_83DC1CF::
	lock
	applymovement LOCALID_PLAYER, Johto_IlexForest_Movement_836082A
	waitmovement 0

@ Johto_IlexForest trigger at 26,43 (VAR_QZ_0x5014 == 2)
Johto_IlexForest_EventScript_83DC1DA::
	lock
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_IlexForest_Text_83DC72C, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_QZ_0x1008
	setflag FLAG_QZ_0x1094
	setflag FLAG_QZ_0x1093
	setflag FLAG_QZ_0x108F
	setvar VAR_QZ_0x5014, 3
	clearflag FLAG_QZ_0x1087
	clearflag FLAG_QZ_0x106B
	goto_if_ge VAR_QZ_0x5009, 7, Johto_IlexForest_EventScript_83DC20F
	setvar VAR_QZ_0x5009, 7

Johto_IlexForest_EventScript_83DC20F::
	goto_if_set FLAG_QZ_0x1030, Johto_IlexForest_EventScript_83DC232
	setorcopyvar VAR_0x8000, ITEM_CHARCOAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_IlexForest_EventScript_83DC23D
	setflag FLAG_QZ_0x1030

Johto_IlexForest_EventScript_83DC232::
	msgbox Johto_IlexForest_Text_83DC7D4, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_IlexForest_EventScript_83DC23D::
	msgbox Johto_IlexForest_Text_83DC84D, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_IlexForest object 5 (local id 5, gfx 0x20) at 37,21
Johto_IlexForest_EventScript_83DC248::
	lock
	faceplayer
	msgbox Johto_IlexForest_Text_83DC93A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_IlexForest object 8 (local id 8, gfx 0x24) at 39,8
Johto_IlexForest_EventScript_83DC255::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4769, 0, Johto_IlexForest_Text_83DCBDC, Johto_IlexForest_Text_83DCC1B
	msgbox Johto_IlexForest_Text_83DCC3E, MSGBOX_AUTOCLOSE
	end

@ Johto_IlexForest sign / bg event at 32,36
Johto_IlexForest_EventScript_83DC278::
	lock
	goto_if_eq VAR_QZ_0x5009, 9, Johto_IlexForest_EventScript_83DC28F
	msgbox Johto_IlexForest_Text_83DC9E3, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_IlexForest_EventScript_83DC28F::
	checkitem ITEM_GS_BALL, 1
	goto_if_eq VAR_RESULT, 0, Johto_IlexForest_EventScript_83DC2C4
	msgbox Johto_IlexForest_Text_83DCA20, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_IlexForest_EventScript_83DC3EC
	msgbox Johto_IlexForest_Text_83DCB00, MSGBOX_DEFAULT
	removeitem ITEM_GS_BALL, 1
	goto Johto_IlexForest_EventScript_83DC2D7

Johto_IlexForest_EventScript_83DC2C4::
	msgbox Johto_IlexForest_Text_83DCAC9, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_IlexForest_EventScript_83DC3EC

Johto_IlexForest_EventScript_83DC2D7::
	closemessage
	removeobject 7
	removeobject 16
	call Johto_IlexForest_EventScript_83DC3C1
	call_if_eq VAR_FACING, 4, Johto_IlexForest_EventScript_83DC3D6
	call_if_eq VAR_FACING, 3, Johto_IlexForest_EventScript_83DC3E1
	turnobject LOCALID_PLAYER, 2
	clearflag FLAG_QZ_0x1031
	addobject 7
	playse SE_M_PETAL_DANCE
	applymovement 7, Johto_IlexForest_Movement_83DC5A8
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 7, Johto_IlexForest_Movement_83DC5BE
	turnobject LOCALID_PLAYER, 1
	waitmovement 0
	playse SE_M_PETAL_DANCE
	applymovement 7, Johto_IlexForest_Movement_83DC5D0
	waitmovement 0
	turnobject LOCALID_PLAYER, 3
	playmoncry SPECIES_CELEBI, 2
	delay 0x28
	waitmoncry
	setwildbattle SPECIES_CELEBI, 0x28, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	special BattleSetup_StartLegendaryBattle
	waitstate
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Johto_IlexForest_EventScript_83DC36A
	goto_if_ne VAR_RESULT, 7, Johto_IlexForest_EventScript_83DC3AF
	call Johto_IlexForest_EventScript_83DC3AB

Johto_IlexForest_EventScript_83DC36A::
	fadescreenswapbuffers 1
	removeobject 7
	setflag FLAG_QZ_0x1031
	fadescreenswapbuffers 0
	removeobject 4
	clearflag FLAG_QZ_0x1032
	addobject 16
	applymovement 16, Johto_IlexForest_Movement_83DC5D8
	waitmovement 0
	turnobject LOCALID_PLAYER, 1
	msgbox Johto_IlexForest_Text_83DCB17, MSGBOX_DEFAULT
	closemessage
	applymovement 16, Johto_IlexForest_Movement_83DC5E1
	waitmovement 0
	setflag FLAG_QZ_0x1032
	removeobject 16
	setvar VAR_QZ_0x5009, 0xA
	release
	end

Johto_IlexForest_EventScript_83DC3AB::
	setflag FLAG_QZ_0x1150
	return

Johto_IlexForest_EventScript_83DC3AF::
	fadescreenswapbuffers 1
	removeobject 7
	call Johto_IlexForest_EventScript_83DC3C1
	turnobject LOCALID_PLAYER, 2
	fadescreenswapbuffers 0
	release
	end

Johto_IlexForest_EventScript_83DC3C1::
	setflag FLAG_QZ_0x1031
	setflag FLAG_QZ_0x1032
	setobjectxyperm 7, 0x18, 0x21
	setobjectxyperm 16, 0x1D, 0x2B
	return

Johto_IlexForest_EventScript_83DC3D6::
	applymovement LOCALID_PLAYER, Johto_IlexForest_Movement_83DC5A2
	waitmovement 0
	return

Johto_IlexForest_EventScript_83DC3E1::
	applymovement LOCALID_PLAYER, Johto_IlexForest_Movement_83DC5A5
	waitmovement 0
	return

Johto_IlexForest_EventScript_83DC3EC::
	closemessage
	release
	end

@ Johto_IlexForest sign / bg event at 23,27
Johto_IlexForest_EventScript_83DC427::
	lock
	msgbox Johto_IlexForest_Text_83DC969, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_IlexForest trigger at 55,11 (VAR_QZ_0x5014 == 3)
Johto_IlexForest_EventScript_83DC433::
	lock
	playbgm MUS_QZ_655, 1
	playse SE_PIN
	applymovement 17, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 17, Johto_IlexForest_Movement_83DC4F0
	waitmovement 0
	msgbox Johto_IlexForest_Text_83DCCB7, MSGBOX_DEFAULT
	closemessage
	playse SE_M_PETAL_DANCE
	applymovement 17, Johto_IlexForest_Movement_83DC4F3
	waitmovement 0
	turnobject 17, 1
	msgbox Johto_IlexForest_Text_83DCD0D, MSGBOX_YESNO
	closemessage
	msgbox Johto_IlexForest_Text_83DCD7E, MSGBOX_DEFAULT
	closemessage
	waitmovement 0
	turnobject 17, 3
	msgbox Johto_IlexForest_Text_83DCDAD, MSGBOX_DEFAULT
	closemessage
	turnobject 17, 1
	delay 0x10
	msgbox Johto_IlexForest_Text_83DCDF9, MSGBOX_DEFAULT
	closemessage
	delay 0x10
	applymovement 17, Johto_IlexForest_Movement_83DC51C
	waitmovement 0
	waitmovement 0
	fadedefaultbgm
	setvar VAR_QZ_0x5014, 4
	setflag FLAG_QZ_0x1095
	removeobject 17
	waitmovement 0
	release
	end

@ Johto_IlexForest object 12 (local id 12, gfx 0x3B) at 50,48
Johto_IlexForest_EventScript_83DC4BC::
	setorcopyvar VAR_0x8000, ITEM_SUPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IlexForest object 11 (local id 11, gfx 0x3B) at 34,28
Johto_IlexForest_EventScript_83DC4C9::
	setorcopyvar VAR_0x8000, ITEM_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IlexForest object 10 (local id 10, gfx 0x3B) at 45,16
Johto_IlexForest_EventScript_83DC4D6::
	setorcopyvar VAR_0x8000, ITEM_FULL_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IlexForest object 9 (local id 9, gfx 0x3B) at 71,42
Johto_IlexForest_EventScript_83DC4E3::
	setorcopyvar VAR_0x8000, ITEM_SUN_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_IlexForest_Movement_836082A:
	walk_up
	step_end

Johto_IlexForest_Movement_83DC4F0:
	walk_right
	face_down
	step_end

Johto_IlexForest_Movement_83DC4F3:
	face_left
	delay_8
	face_up
	delay_8
	face_right
	delay_8
	face_down
	delay_8
	face_left
	delay_8
	face_up
	delay_8
	face_right
	delay_8
	face_down
	step_end

Johto_IlexForest_Movement_83DC51C:
	face_left
	delay_16
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

Johto_IlexForest_Movement_83DC539:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Johto_IlexForest_Movement_83DC540:
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
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_IlexForest_Movement_83DC54E:
	walk_fast_up
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

Johto_IlexForest_Movement_83DC558:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_IlexForest_Movement_83DC55F:
	walk_fast_up
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_IlexForest_Movement_83DC569:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_IlexForest_Movement_83DC570:
	walk_fast_up
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Johto_IlexForest_Movement_83DC57A:
	walk_fast_down
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_IlexForest_Movement_83DC584:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	face_down
	step_end

Johto_IlexForest_Movement_83DC594:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Johto_IlexForest_Movement_83DC5A2:
	walk_down
	walk_right
	step_end

Johto_IlexForest_Movement_83DC5A5:
	walk_down
	walk_left
	step_end

Johto_IlexForest_Movement_83DC5A8:
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
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_IlexForest_Movement_83DC5BE:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_up
	step_end

Johto_IlexForest_Movement_83DC5D0:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

Johto_IlexForest_Movement_83DC5D8:
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_IlexForest_Movement_83DC5E1:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	step_end

Johto_IlexForest_Text_83DC5EA::
	.string "Oh, man… My boss is going to be\n"
	.string "steaming…\p"
	.string "The FARFETCH’D that CUTS trees\n"
	.string "for charcoal took off on me.\p"
	.string "I can’t go looking for it here\n"
	.string "in the ILEX FOREST.\p"
	.string "It’s too big, dark and scary\n"
	.string "for me…$"

Johto_IlexForest_Text_83DC6A8::
	.string "Wow! Thanks a whole bunch!\p"
	.string "My boss’ POKéMON won’t obey me\n"
	.string "because I don’t have a BADGE.$"

Johto_IlexForest_Text_83DC71A::
	.string "FARFETCH’D: Kwaa!$"

Johto_IlexForest_Text_83DC72C::
	.string "Ah! My FARFETCH’D!\p"
	.string "You found it for us, kid?\p"
	.string "Without it, we wouldn’t be able\n"
	.string "to CUT trees for charcoal.\p"
	.string "Thanks, kid!\p"
	.string "Now, how can I thank you…\p"
	.string "I know! Here, take this.$"

Johto_IlexForest_Text_83DC7D4::
	.string "A POKéMON holding this CHARCOAL\n"
	.string "gets stronger FIRE-type attacks.\p"
	.string "With JOHTO’s second GYM BADGE,\n"
	.string "you can already use CUT.$"

Johto_IlexForest_Text_83DC84D::
	.string "There is no room in your BAG.\p"
	.string "My apprentice will keep your gift\n"
	.string "at our house in AZALEA TOWN.$"

Johto_IlexForest_Text_83DC8AA::
	.string "Do you want to apprentice as a\n"
	.string "charcoal maker with me?\p"
	.string "You’ll be first-rate in ten years!$"

Johto_IlexForest_Text_83DC93A::
	.string "Did something happen to the\n"
	.string "forest’s guardian?$"

Johto_IlexForest_Text_83DC969::
	.string "ILEX FOREST is so overgrown with\n"
	.string "trees that you can’t see the sky.\p"
	.string "Please watch out for items that may\n"
	.string "have been dropped.$"

Johto_IlexForest_Text_83DC9E3::
	.string "ILEX FOREST SHRINE…\p"
	.string "It’s in honor of the forest’s\n"
	.string "protector…$"

Johto_IlexForest_Text_83DCA20::
	.string "ILEX FOREST SHRINE…\p"
	.string "It’s in honor of the forest’s\n"
	.string "protector…\p"
	.string "Oh? What is this?\p"
	.string "It’s a hole.\n"
	.string "It looks like the GS BALL would fit\l"
	.string "inside it.\p"
	.string "Want to put the GS BALL here?$"

Johto_IlexForest_Text_83DCAC9::
	.string "The GS BALL is still in the shrine.\p"
	.string "Call CELEBI again?$"

Johto_IlexForest_Text_83DCB00::
	.string "{PLAYER} put in the GS BALL.$"

Johto_IlexForest_Text_83DCB17::
	.string "Whew, wasn’t that something!\p"
	.string "{PLAYER}, that was fantastic. Thanks!\p"
	.string "The legends about that SHRINE were\n"
	.string "real after all.\p"
	.string "I feel inspired by what I just saw.\p"
	.string "It motivates me to make better BALLS!\p"
	.string "I’m going!$"

Johto_IlexForest_Text_83DCBDC::
	.string "Don’t sneak up on me like that!\p"
	.string "You frightened a POKéMON away!$"

Johto_IlexForest_Text_83DCC1B::
	.string "I hadn’t seen that POKéMON before…$"

Johto_IlexForest_Text_83DCC3E::
	.string "A POKéMON I’ve never seen before\n"
	.string "fell out of the tree when I used\l"
	.string "HEADBUTT.\p"
	.string "I ought to use HEADBUTT in other\n"
	.string "places too.$"

Johto_IlexForest_Text_83DCCB7::
	.string "KIMONO GIRL, KIMONO GIRL, lost and\n"
	.string "all alone. Poor girl lost in the\l"
	.string "dark ILEX FOREST.$"

Johto_IlexForest_Text_83DCD0D::
	.string "You remember me from VIOLET CITY?\n"
	.string "You must be imagining.\p"
	.string "Anyway, will you show me how to\n"
	.string "get out of this forest?$"

Johto_IlexForest_Text_83DCD7E::
	.string "Me oh my... You are lost yourself,\n"
	.string "aren’t you?$"

Johto_IlexForest_Text_83DCDAD::
	.string "Wow! Are you going to show me how\n"
	.string "to get out?\p"
	.string "You are such a smart POKéMON!$"

Johto_IlexForest_Text_83DCDF9::
	.string "See you.$"
