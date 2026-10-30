Kanto_CeladonCity_GameCorner_PrizeRoom_MapScripts::
	.byte 0

@ Kanto_CeladonCity_GameCorner_PrizeRoom object 1 (local id 1, gfx 0x3EE6) at 2,6
Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CE88::
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_8447161, MSGBOX_NPC
	end

@ Kanto_CeladonCity_GameCorner_PrizeRoom object 2 (local id 2, gfx 0x3F2D) at 6,7
Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CE91::
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_844719F, MSGBOX_NPC
	end

@ Kanto_CeladonCity_GameCorner_PrizeRoom object 3 (local id 3, gfx 0x3F5B) at 4,2
Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CE9A::
	lock
	faceplayer
	call Quetzal_EventScript_8361A4D
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFFE
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_84471F7, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CECF

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CECF::
	call Quetzal_EventScript_841CE7A
	multichoice 0xB, 0, MULTI_QZ_0x61, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF29
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF39
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF49
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF59
	goto_if_eq VAR_0x8000, 4, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF69
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26::
	closemessage
	release
	end

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF29::
	setvar VAR_TEMP_1, 0x3BA
	setvar VAR_TEMP_2, 0x7D0
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF39::
	setvar VAR_TEMP_1, 0x27C
	setvar VAR_TEMP_2, 0x9C4
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF49::
	setvar VAR_TEMP_1, 0x93
	setvar VAR_TEMP_2, 0xAF0
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF59::
	setvar VAR_TEMP_1, 0x1DF
	setvar VAR_TEMP_2, 0x1388
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF69::
	setvar VAR_TEMP_1, 0x89
	setvar VAR_TEMP_2, 0x270F
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79::
	bufferspeciesname 0, VAR_TEMP_1
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_844723D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26
	checkcoins VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_TEMP_2
	goto_if_lt Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D00A
	textcolor 3
	givemon VAR_TEMP_1, 0x18, ITEM_NONE
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFB5

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFB5::
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D018
	goto_if_eq VAR_RESULT, 1, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D042
	goto_if_eq VAR_RESULT, 2, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFDB
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFDB

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFDB::
	textcolor 3
	msgbox Quetzal_Text_8455FC6, MSGBOX_DEFAULT
	closemessage
	release
	end

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFE8::
	getpartysize
	subvar VAR_RESULT, 1
	copyvar VAR_0x8004, VAR_RESULT
	call Quetzal_EventScript_84579AE
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFFE::
	textcolor 3
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_84471DE, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D00A::
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_8447267, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D018::
	removecoins VAR_TEMP_2
	bufferspeciesname 0, VAR_TEMP_1
	playfanfare MUS_LEVEL_UP
	message Kanto_CeladonCity_GameCorner_PrizeRoom_Text_8455F86
	waitmessage
	waitfanfare
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFE8
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D042::
	removecoins VAR_TEMP_2
	bufferspeciesname 0, VAR_TEMP_1
	playfanfare MUS_LEVEL_UP
	message Kanto_CeladonCity_GameCorner_PrizeRoom_Text_8455F86
	waitmessage
	waitfanfare
	msgbox Quetzal_Text_845675A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D071
	call Quetzal_EventScript_84590F6
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D071

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D071::
	call Quetzal_EventScript_84590FF
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26

@ Kanto_CeladonCity_GameCorner_PrizeRoom object 4 (local id 4, gfx 0x3F5B) at 6,2
Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D07C::
	lock
	faceplayer
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFFE
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_84471F7, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D0AC

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D0AC::
	call Quetzal_EventScript_841CE7A
	multichoice 0xB, 0, MULTI_QZ_0x62, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D103
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D113
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D123
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D133
	goto_if_eq VAR_0x8000, 4, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D143
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D103::
	setvar VAR_TEMP_1, 0x31D
	setvar VAR_TEMP_2, 0xFA0
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D113::
	setvar VAR_TEMP_1, 0xE8
	setvar VAR_TEMP_2, 0xDAC
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D123::
	setvar VAR_TEMP_1, 0xE6
	setvar VAR_TEMP_2, 0xFA0
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D133::
	setvar VAR_TEMP_1, 0xE7
	setvar VAR_TEMP_2, 0x1194
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D143::
	setvar VAR_TEMP_1, 0x326
	setvar VAR_TEMP_2, 0xFA0
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C::
	bufferitemname 0, VAR_TEMP_1
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_844723D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D189

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D189::
	bufferitemname 0, VAR_TEMP_1
	checkcoins VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_TEMP_2
	goto_if_lt Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D00A
	checkitemspace VAR_TEMP_1, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D1CB
	setorcopyvar VAR_0x8000, VAR_TEMP_1
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_ne VAR_0x8007, 1, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D1CB
	removecoins VAR_TEMP_2
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D1CB::
	textcolor 3
	msgbox Quetzal_Text_845584C, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Kanto_CeladonCity_GameCorner_PrizeRoom object 5 (local id 5, gfx 0x3F5B) at 2,2
Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D1D8::
	lock
	faceplayer
	checkitem ITEM_COIN_CASE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFFE
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_84471F7, MSGBOX_DEFAULT
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D208

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D208::
	call Quetzal_EventScript_841CE7A
	multichoice 0xA, 0, MULTI_QZ_0x5F, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D25F
	goto_if_eq VAR_0x8000, 1, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D26F
	goto_if_eq VAR_0x8000, 2, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D27F
	goto_if_eq VAR_0x8000, 3, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D28F
	goto_if_eq VAR_0x8000, 4, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D29F
	goto_if_eq VAR_0x8000, 127, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D25F::
	setvar VAR_TEMP_1, 0x1DD
	setvar VAR_TEMP_2, 0x3E8
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D26F::
	setvar VAR_TEMP_1, 0x1EE
	setvar VAR_TEMP_2, 0x5DC
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D27F::
	setvar VAR_TEMP_1, 0x2FC
	setvar VAR_TEMP_2, 0x7D0
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D28F::
	setvar VAR_TEMP_1, 0x1BC
	setvar VAR_TEMP_2, 0x9C4
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D29F::
	setvar VAR_TEMP_1, 0x1F7
	setvar VAR_TEMP_2, 0x9C4
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_Text_8447161::
	.string "I sure do fancy that PORYGON!\n"
	.string "But, it’s hard to win at slots!$"

Kanto_CeladonCity_GameCorner_PrizeRoom_Text_844719F::
	.string "Gahaha! I raked it in today!\n"
	.string "If only every day were like this…$"

Kanto_CeladonCity_GameCorner_PrizeRoom_Text_84471DE::
	.string "A COIN CASE is required…$"

Kanto_CeladonCity_GameCorner_PrizeRoom_Text_84471F7::
	.string "We exchange your COINS for prizes.$"

Kanto_CeladonCity_GameCorner_PrizeRoom_Text_844723D::
	.string "So, you want the {STR_VAR_1}?$"

Kanto_CeladonCity_GameCorner_PrizeRoom_Text_8447267::
	.string "Sorry, you’ll need more COINS\n"
	.string "than that.$"

Kanto_CeladonCity_GameCorner_PrizeRoom_Text_8455F86::
	.string "{PLAYER} obtained\n"
	.string "the {STR_VAR_1}!$"
