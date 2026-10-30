Kanto_CeladonCity_Condominiums_1F_MapScripts::
	.byte 0

@ Kanto_CeladonCity_Condominiums_1F object 4 (local id 4, gfx 0x3F2F) at 2,9
Kanto_CeladonCity_Condominiums_1F_EventScript_841C5E5::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x221A, Kanto_CeladonCity_Condominiums_1F_EventScript_841C63E
	goto_if_set FLAG_QZ_0x2108, Kanto_CeladonCity_Condominiums_1F_EventScript_841C648
	goto_if_set FLAG_QZ_0x2165, Kanto_CeladonCity_Condominiums_1F_EventScript_841C63E
	checkitem ITEM_TEA, 1
	goto_if_eq VAR_RESULT, 1, Kanto_CeladonCity_Condominiums_1F_EventScript_841C631
	msgbox Kanto_CeladonCity_Condominiums_1F_Text_8446195, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_TEA
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_84570B3

Kanto_CeladonCity_Condominiums_1F_EventScript_841C631::
	setflag FLAG_QZ_0x2165
	msgbox Kanto_CeladonCity_Condominiums_1F_Text_84461D5, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_Condominiums_1F_EventScript_841C63E::
	msgbox Kanto_CeladonCity_Condominiums_1F_Text_8446214, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_Condominiums_1F_EventScript_841C648::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	setflag FLAG_QZ_0x221A
	msgbox Kanto_CeladonCity_Condominiums_1F_Text_8446254, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity_Condominiums_1F object 1 (local id 1, gfx 0x3F1F) at 1,9
Kanto_CeladonCity_Condominiums_1F_EventScript_841C662::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MEOWTH, 0
	msgbox Kanto_CeladonCity_Condominiums_1F_Text_8446187, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_CeladonCity_Condominiums_1F object 2 (local id 2, gfx 0x3EF9) at 5,13
Kanto_CeladonCity_Condominiums_1F_EventScript_841C675::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_CLEFAIRY, 0
	msgbox Kanto_CeladonCity_Condominiums_1F_Text_8446327, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_CeladonCity_Condominiums_1F object 3 (local id 3, gfx 0x3F27) at 5,8
Kanto_CeladonCity_Condominiums_1F_EventScript_841C688::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_NIDORAN_F, 0
	msgbox Kanto_CeladonCity_Condominiums_1F_Text_844633E, MSGBOX_DEFAULT
	waitmoncry
	release
	end

@ Kanto_CeladonCity_Condominiums_1F sign / bg event at 8,12
@ Kanto_CeladonCity_Condominiums_1F sign / bg event at 7,12
Kanto_CeladonCity_Condominiums_1F_EventScript_841C69B::
	msgbox Kanto_CeladonCity_Condominiums_1F_Text_8446353, MSGBOX_SIGN
	end

Kanto_CeladonCity_Condominiums_1F_Text_8446187::
	.string "MEOWTH: Meow!$"

Kanto_CeladonCity_Condominiums_1F_Text_8446195::
	.string "You shouldn’t spend all your money\n"
	.string "on drinks.\p"
	.string "Try this instead.$"

Kanto_CeladonCity_Condominiums_1F_Text_84461D5::
	.string "Nothing beats thirst like some hot\n"
	.string "TEA.\p"
	.string "It really is the best.$"

Kanto_CeladonCity_Condominiums_1F_Text_8446214::
	.string "My dear POKéMON keep me company.\n"
	.string "MEOWTH even brings money home!$"

Kanto_CeladonCity_Condominiums_1F_Text_8446254::
	.string "Oh, hello, dearie.\n"
	.string "Did you enjoy my TEA?\p"
	.string "By the way, dear. Are you, by any\n"
	.string "chance, from PALLET TOWN?\p"
	.string "A girl from PALLET TOWN, DAISY,\n"
	.string "she enjoys TEA every day.\p"
	.string "She visits the CELADON DEPT. STORE\n"
	.string "to buy some TEA.$"

Kanto_CeladonCity_Condominiums_1F_Text_8446327::
	.string "CLEFAIRY: Pi pippippi!$"

Kanto_CeladonCity_Condominiums_1F_Text_844633E::
	.string "NIDORAN♀: Kya kyaoo!$"

Kanto_CeladonCity_Condominiums_1F_Text_8446353::
	.string "CELADON MANSION\n"
	.string "Manager’s Suite$"
