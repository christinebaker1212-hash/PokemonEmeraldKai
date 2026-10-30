Kanto_CeladonCity_DepartmentStore_Roof_MapScripts::
	.byte 0

@ Kanto_CeladonCity_DepartmentStore_Roof object 2 (local id 2, gfx 0x3F18) at 5,4
Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BF7F::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	lock
	faceplayer
	call Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFAA
	goto_if_eq VAR_TEMP_1, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CC
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFF2

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFAA::
	setvar VAR_TEMP_1, 0
	checkitem ITEM_FRESH_WATER, 1
	call_if_eq VAR_RESULT, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFE0
	checkitem ITEM_SODA_POP, 1
	call_if_eq VAR_RESULT, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFE6
	checkitem ITEM_LEMONADE, 1
	call_if_eq VAR_RESULT, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFEC
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFE0::
	addvar VAR_TEMP_1, 1
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFE6::
	addvar VAR_TEMP_1, 2
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFEC::
	addvar VAR_TEMP_1, 4
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841BFF2::
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8445E6F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	textcolor 3
	message Kanto_CeladonCity_DepartmentStore_Roof_Text_8445EA9
	waitmessage
	textcolor 1
	copyvar VAR_0x8000, VAR_TEMP_1
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C062
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C08F
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C0BC
	goto_if_eq VAR_0x8000, 4, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C0F4
	goto_if_eq VAR_0x8000, 5, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C121
	goto_if_eq VAR_0x8000, 6, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C159
	goto_if_eq VAR_0x8000, 7, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C191
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C062::
	multichoice 0, 0, MULTI_QZ_0x78, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1D4
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C08F::
	multichoice 0, 0, MULTI_QZ_0x7D, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1F5
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C0BC::
	multichoice 0, 0, MULTI_QZ_0x7A, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1D4
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1F5
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C0F4::
	multichoice 0, 0, MULTI_QZ_0x7C, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C216
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C121::
	multichoice 0, 0, MULTI_QZ_0x79, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1D4
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C216
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C159::
	multichoice 0, 0, MULTI_QZ_0x7E, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1F5
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C216
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C191::
	multichoice 0, 0, MULTI_QZ_0x7B, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1D4
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1F5
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C216
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1D4::
	setvar VAR_0x8008, 0x23
	setvar VAR_0x8009, 0xDD
	goto_if_set FLAG_QZ_0x2169, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2D6
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8445EBF, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C237

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C1F5::
	setvar VAR_0x8008, 0x24
	setvar VAR_0x8009, 0xDE
	goto_if_set FLAG_QZ_0x216B, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2D6
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8445F4F, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C237

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C216::
	setvar VAR_0x8008, 0x25
	setvar VAR_0x8009, 0x31F
	goto_if_set FLAG_QZ_0x2170, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2D6
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8445FB4, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C237

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C237::
	bufferitemname 0, VAR_0x8008
	bufferitemname 1, VAR_0x8009
	removeitem VAR_0x8008, 1
	checkitemspace VAR_0x8009, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2C0
	additem VAR_0x8009, 1
	textcolor 3
	playfanfare MUS_LEVEL_UP
	message Kanto_CeladonCity_DepartmentStore_Roof_Text_8445EF7
	waitmessage
	waitfanfare
	setorcopyvar VAR_0x8000, VAR_0x8009
	setorcopyvar VAR_0x8001, 1
	call Quetzal_EventScript_845931B
	call Quetzal_EventScript_8456AB7
	call_if_eq VAR_0x8008, 35, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C29C
	call_if_eq VAR_0x8008, 36, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2A8
	call_if_eq VAR_0x8008, 37, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2B4
	release
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C29C::
	setflag FLAG_QZ_0x2169
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8445F1E, MSGBOX_DEFAULT
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2A8::
	setflag FLAG_QZ_0x216B
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8445F84, MSGBOX_DEFAULT
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2B4::
	setflag FLAG_QZ_0x2170
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8445FE9, MSGBOX_DEFAULT
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2C0::
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_844601B, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CA::
	release
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2CC::
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_84460BB, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2D6::
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_844603A, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity_DepartmentStore_Roof object 1 (local id 1, gfx 0x3EFD) at 9,5
Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2E0::
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8446064, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_Roof sign / bg event at 14,3
Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2E9::
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_84460E3, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_Roof sign / bg event at 10,3
@ Kanto_CeladonCity_DepartmentStore_Roof sign / bg event at 11,3
@ Kanto_CeladonCity_DepartmentStore_Roof sign / bg event at 12,3
Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C2F2::
	lockall
	message Kanto_CeladonCity_DepartmentStore_Roof_Text_8446104
	waitmessage
	showmoneybox 0, 0, 0
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C303

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C303::
	multichoice 0x10, 0, MULTI_QZ_0x58, 1
	nop
	copyvar VAR_TEMP_1, VAR_RESULT
	copyvar VAR_0x8000, VAR_TEMP_1
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C342
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C353
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C364
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_844617A, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C40C

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C342::
	setvar VAR_TEMP_0, 0x23
	checkmoney 0xC8, 0
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C38A

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C353::
	setvar VAR_TEMP_0, 0x24
	checkmoney 0x12C, 0
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C38A

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C364::
	setvar VAR_TEMP_0, 0x25
	checkmoney 0x15E, 0
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C38A

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C375::
	removemoney 0xC8, 0
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C37C::
	removemoney 0x12C, 0
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C383::
	removemoney 0x15E, 0
	return

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C38A::
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C3F0
	checkitemspace VAR_TEMP_0, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C3FE
	call_if_eq VAR_TEMP_1, 0, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C375
	call_if_eq VAR_TEMP_1, 1, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C37C
	call_if_eq VAR_TEMP_1, 2, Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C383
	updatemoneybox 0
	bufferitemname 0, VAR_TEMP_0
	playse SE_VEND
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_8446142, MSGBOX_DEFAULT
	additem VAR_TEMP_0, 1
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C3E4

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C3E4::
	message Kanto_CeladonCity_DepartmentStore_Roof_Text_8446104
	waitmessage
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C303

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C3F0::
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_844612A, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C40C

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C3FE::
	msgbox Kanto_CeladonCity_DepartmentStore_Roof_Text_844615A, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C40C

Kanto_CeladonCity_DepartmentStore_Roof_EventScript_841C40C::
	hidemoneybox
	releaseall
	end

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445E6F::
	.string "I’m thirsty!\n"
	.string "I want something to drink!\p"
	.string "Give her a drink?$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445EA9::
	.string "Give her which drink?$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445EBF::
	.string "Yay!\p"
	.string "FRESH WATER!\p"
	.string "Thank you!\n"
	.string "You can have this from me!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445EF7::
	.string "{PLAYER} received a {STR_VAR_2}\n"
	.string "from the little girl.$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445F1E::
	.string "A SWEET APPLE can make a certain\n"
	.string "POKéMON evolve.$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445F4F::
	.string "Yay!\p"
	.string "SODA POP!\p"
	.string "Thank you!\n"
	.string "You can have this from me!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445F84::
	.string "A TART APPLE can make a certain\n"
	.string "POKéMON evolve.$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445FB4::
	.string "Yay!\p"
	.string "LEMONADE!\p"
	.string "Thank you!\n"
	.string "You can have this from me!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8445FE9::
	.string "A SYRUPY APPLE can make a certain\n"
	.string "POKéMON evolve.$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_844601B::
	.string "You don’t have space for this!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_844603A::
	.string "No, thank you!\n"
	.string "I’m not thirsty after all!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8446064::
	.string "My sister is a TRAINER, believe it\n"
	.string "or not.\p"
	.string "But, she’s so immature, she drives\n"
	.string "me nuts!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_84460BB::
	.string "I’m thirsty!\n"
	.string "I want something to drink!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_84460E3::
	.string "ROOFTOP SQUARE:\n"
	.string "VENDING MACHINES$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8446104::
	.string "A vending machine!\n"
	.string "What does it have?$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_844612A::
	.string "Oops, not enough money!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_8446142::
	.string "A can of {STR_VAR_1} popped out!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_844615A::
	.string "There’s no more room for stuff!$"

Kanto_CeladonCity_DepartmentStore_Roof_Text_844617A::
	.string "Not thirsty!$"
