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
	multichoice 0xB, 0, MULTI_QZ_0x61, 0
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
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, SPECIES_ABRA
	setvar VAR_TEMP_2, 180
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF39::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, SPECIES_CLEFAIRY
	setvar VAR_TEMP_2, 500
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF49::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, SPECIES_DRATINI
	setvar VAR_TEMP_2, 2800
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF59::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, SPECIES_SCYTHER
	setvar VAR_TEMP_2, 5500
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF69::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, SPECIES_PORYGON
	setvar VAR_TEMP_2, 9999
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF79::
	bufferspeciesname 0, VAR_TEMP_1
	msgbox Kanto_CeladonCity_GameCorner_PrizeRoom_Text_844723D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26
	checkcoins VAR_RESULT
	compare_var_to_var VAR_RESULT, VAR_TEMP_2
	goto_if_lt Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D00A
	textcolor 3
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	goto_if_eq VAR_TEMP_1, SPECIES_ABRA, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_ABRA
	goto_if_eq VAR_TEMP_1, SPECIES_CLEFAIRY, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_CLEFAIRY
	goto_if_eq VAR_TEMP_1, SPECIES_DRATINI, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_DRATINI
	goto_if_eq VAR_TEMP_1, SPECIES_SCYTHER, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_SCYTHER
	goto_if_eq VAR_TEMP_1, SPECIES_PORYGON, Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_PORYGON
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CF26
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
	multichoice 0xB, 0, MULTI_QZ_0x62, 0
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
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_TM13
	setvar VAR_TEMP_2, 4000
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D113::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_TM23
	setvar VAR_TEMP_2, 3500
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D123::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_TM24
	setvar VAR_TEMP_2, 4000
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D133::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_TM30
	setvar VAR_TEMP_2, 4500
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D143::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_TM35
	setvar VAR_TEMP_2, 4000
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
	multichoice 0xA, 0, MULTI_QZ_0x5F, 0
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
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_SMOKE_BALL
	setvar VAR_TEMP_2, 800
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D26F::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_MIRACLE_SEED
	setvar VAR_TEMP_2, 1000
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D27F::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_CHARCOAL
	setvar VAR_TEMP_2, 1000
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D28F::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_MYSTIC_WATER
	setvar VAR_TEMP_2, 1000
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D16C

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841D29F::
	@ Moemon Kai: the original game's prize (tools/quetzal_port/fix_prize_room.py)
	setvar VAR_TEMP_1, ITEM_YELLOW_FLUTE
	setvar VAR_TEMP_2, 1600
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

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_ABRA::
	givemon VAR_TEMP_1, 9, ITEM_NONE
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFB5

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_CLEFAIRY::
	givemon VAR_TEMP_1, 8, ITEM_NONE
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFB5

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_DRATINI::
	givemon VAR_TEMP_1, 18, ITEM_NONE
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFB5

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_SCYTHER::
	givemon VAR_TEMP_1, 25, ITEM_NONE
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFB5

Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_Give_PORYGON::
	givemon VAR_TEMP_1, 26, ITEM_NONE
	goto Kanto_CeladonCity_GameCorner_PrizeRoom_EventScript_841CFB5
