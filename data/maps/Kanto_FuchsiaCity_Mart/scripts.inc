Kanto_FuchsiaCity_Mart_MapScripts::
	.byte 0

@ Kanto_FuchsiaCity_Mart object 3 (local id 3, gfx 0x3EFC) at 9,5
Kanto_FuchsiaCity_Mart_EventScript_841D66A::
	msgbox Kanto_FuchsiaCity_Mart_Text_84480C4, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity_Mart object 2 (local id 2, gfx 0x3F09) at 6,3
Kanto_FuchsiaCity_Mart_EventScript_841D673::
	msgbox Kanto_FuchsiaCity_Mart_Text_844804C, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_FuchsiaCity_Mart_EventScript_841D67C::
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

Kanto_FuchsiaCity_Mart_Text_844804C::
	.string "Don’t they have any pennants\n"
	.string "promoting the SAFARI ZONE?\p"
	.string "How about some paper lanterns?\n"
	.string "Aren’t there even any calendars?$"

Kanto_FuchsiaCity_Mart_Text_84480C4::
	.string "Did you try X SPEED?\n"
	.string "It speeds up a POKéMON in battle.$"
