Kanto_CeruleanCity_House5_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_House5 object 1 (local id 1, gfx 0x3F2D) at 7,4
Kanto_CeruleanCity_House5_EventScript_841AE49::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x215F, Kanto_CeruleanCity_House5_EventScript_841AE9C
	msgbox Kanto_CeruleanCity_House5_Text_8443292, MSGBOX_DEFAULT
	msgbox Kanto_CeruleanCity_House5_Text_84432ED, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0xF0
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_84570B3
	setflag FLAG_QZ_0x215F
	msgbox Kanto_CeruleanCity_House5_Text_844330C, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_House5_EventScript_841AE9C::
	msgbox Kanto_CeruleanCity_House5_Text_8443404, MSGBOX_NPC
	end

@ Kanto_CeruleanCity_House5 sign / bg event at 3,1
Kanto_CeruleanCity_House5_EventScript_841B0B7::
	lockall
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	msgbox Kanto_CeruleanCity_House5_Text_84435A6, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CeruleanCity_House5_Text_8443292::
	.string "I study rare sweets made from\n"
	.string "BERRIES.\p"
	.string "This one belongs with a promising\n"
	.string "TRAINER like you.$"

Kanto_CeruleanCity_House5_Text_84432ED::
	.string "Please, take this BERRY SWEET.$"

Kanto_CeruleanCity_House5_Text_844330C::
	.string "A BERRY SWEET can help a certain\n"
	.string "POKéMON evolve.\p"
	.string "I hope you put it to good use.$"

Kanto_CeruleanCity_House5_Text_8443404::
	.string "I hope that BERRY SWEET helps\n"
	.string "your POKéMON grow.$"

Kanto_CeruleanCity_House5_Text_84435A6::
	.string "No records have been registered.$"
