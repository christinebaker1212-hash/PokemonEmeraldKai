Kanto_PewterCity_Mart_MapScripts::
	.byte 0

@ Kanto_PewterCity_Mart object 1 (local id 1, gfx 0x3F5C) at 8,2
Kanto_PewterCity_Mart_EventScript_841A6D5::
	msgbox Kanto_PewterCity_Mart_Text_84417E4, MSGBOX_NPC
	end

@ Kanto_PewterCity_Mart object 2 (local id 2, gfx 0x3EEE) at 6,4
Kanto_PewterCity_Mart_EventScript_841A6DE::
	msgbox Kanto_PewterCity_Mart_Text_8441847, MSGBOX_NPC
	end

@ Kanto_PewterCity_Mart object 3 (local id 3, gfx 0x3EFA) at 2,3
Kanto_PewterCity_Mart_EventScript_841A6E7::
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

Kanto_PewterCity_Mart_Text_84417E4::
	.string "A shady old guy sucked me into\n"
	.string "buying this weird fish POKéMON!\p"
	.string "It’s totally weak and it cost ¥500!$"

Kanto_PewterCity_Mart_Text_8441847::
	.string "Good things can happen if you raise\n"
	.string "POKéMON diligently.\p"
	.string "Even the weak ones can surprise\n"
	.string "you if you don’t give up on them.$"
