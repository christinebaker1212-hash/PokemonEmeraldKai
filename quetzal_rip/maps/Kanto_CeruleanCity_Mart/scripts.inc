Kanto_CeruleanCity_Mart_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_Mart object 3 (local id 3, gfx 0x3F5C) at 1,7
Kanto_CeruleanCity_Mart_EventScript_841AC88::
	msgbox Kanto_CeruleanCity_Mart_Text_8442CED, MSGBOX_NPC
	end

@ Kanto_CeruleanCity_Mart object 2 (local id 2, gfx 0x3F57) at 9,2
Kanto_CeruleanCity_Mart_EventScript_841AC91::
	msgbox Kanto_CeruleanCity_Mart_Text_8442DA4, MSGBOX_NPC
	end

@ Kanto_CeruleanCity_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_CeruleanCity_Mart_EventScript_841AC9A::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	message Quetzal_Text_84563A6
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_84565DA, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_Mart_Text_8442CED::
	.string "REPEL not only keeps bugs away,\n"
	.string "it also works on weak POKéMON.\p"
	.string "Put your strongest POKéMON at the\n"
	.string "left of the POKéMON LIST.\p"
	.string "If your first POKéMON is strong,\n"
	.string "REPEL’s effect is boosted.$"

Kanto_CeruleanCity_Mart_Text_8442DA4::
	.string "Do you know about RARE CANDY?\n"
	.string "They don’t sell it in shops.\p"
	.string "I think it makes POKéMON grow\n"
	.string "very quickly all of a sudden.$"
