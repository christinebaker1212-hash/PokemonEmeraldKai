Kanto_CeladonCity_DepartmentStore_2F_MapScripts::
	.byte 0

@ Kanto_CeladonCity_DepartmentStore_2F object 1 (local id 1, gfx 0x3F16) at 5,10
Kanto_CeladonCity_DepartmentStore_2F_EventScript_841BB0D::
	msgbox Kanto_CeladonCity_DepartmentStore_2F_Text_84456F3, MSGBOX_NPC
	end

@ Kanto_CeladonCity_DepartmentStore_2F sign / bg event at 4,1
Kanto_CeladonCity_DepartmentStore_2F_EventScript_841BB16::
	msgbox Kanto_CeladonCity_DepartmentStore_2F_Text_844571C, MSGBOX_SIGN
	end

@ Kanto_CeladonCity_DepartmentStore_2F object 2 (local id 2, gfx 0x3EFA) at 1,8
Kanto_CeladonCity_DepartmentStore_2F_EventScript_841BB1F::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity_DepartmentStore_2F object 3 (local id 3, gfx 0x3EFA) at 1,6
Kanto_CeladonCity_DepartmentStore_2F_EventScript_841BB46::
	@ Moemon Kai: FireRed's clerk, one list (docs/REGIONS_AUDIT.md 71; src/shop.c gives FireRed's list and Kai's newer items)
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart Kanto_CeladonCity_DepartmentStore_2F_Mart_841BBB4
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity_DepartmentStore_2F object 4 (local id 4, gfx 0x3F57) at 11,6
Kanto_CeladonCity_DepartmentStore_2F_EventScript_845D424::
	lock
	faceplayer
	setvar VAR_0x8004, 0xC
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	msgbox Kanto_CeladonCity_DepartmentStore_2F_Text_8445770, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_DepartmentStore_2F_Mart_841BBB4:
	.2byte ITEM_BLUE_FLUTE
	.2byte ITEM_YELLOW_FLUTE
	.2byte ITEM_RED_FLUTE
	.2byte ITEM_BLACK_FLUTE
	.2byte ITEM_WHITE_FLUTE
	.2byte ITEM_NONE

Kanto_CeladonCity_DepartmentStore_2F_Text_84456F3::
	.string "For long outings, you should buy\n"
	.string "REVIVE.$"

Kanto_CeladonCity_DepartmentStore_2F_Text_844571C::
	.string "Top-Grade Items for TRAINERS!\p"
	.string "2F: TRAINER’S MARKET$"

Kanto_CeladonCity_DepartmentStore_2F_Text_8445770::
	.string "We have a customer, LANCE, who\n"
	.string "occasionally comes.\p"
	.string "He always buys capes.\p"
	.string "I wonder… Does he have many\n"
	.string "identical capes at home?$"

Kanto_CeladonCity_DepartmentStore_2F_Text_84457EE::
	.string "We sell special Balls and flutes.\n"
	.string "Which section would you like?$"
