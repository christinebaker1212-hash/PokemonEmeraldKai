Kanto_SaffronCity_Mart_MapScripts::
	.byte 0

@ Kanto_SaffronCity_Mart object 3 (local id 3, gfx 0x3F16) at 9,5
Kanto_SaffronCity_Mart_EventScript_841F04F::
	msgbox Kanto_SaffronCity_Mart_Text_844BF4C, MSGBOX_NPC
	end

@ Kanto_SaffronCity_Mart object 2 (local id 2, gfx 0x3F5C) at 1,7
Kanto_SaffronCity_Mart_EventScript_841F058::
	msgbox Kanto_SaffronCity_Mart_Text_844BEEA, MSGBOX_NPC
	end

@ Kanto_SaffronCity_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_SaffronCity_Mart_EventScript_841F061::
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

Kanto_SaffronCity_Mart_Text_844BEEA::
	.string "MAX REPEL keeps weaker POKéMON\n"
	.string "from appearing.\p"
	.string "MAX REPEL stays effective longer\n"
	.string "than SUPER REPEL.$"

Kanto_SaffronCity_Mart_Text_844BF4C::
	.string "REVIVE is costly, but it revives\n"
	.string "fainted POKéMON!$"
