Kanto_SixIsland_Mart_MapScripts::
	.byte 0

@ Kanto_SixIsland_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_SixIsland_Mart_EventScript_8421B66::
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

@ Kanto_SixIsland_Mart object 3 (local id 3, gfx 0x3F31) at 7,7
Kanto_SixIsland_Mart_EventScript_8421BA2::
	msgbox Kanto_SixIsland_Mart_Text_845480B, MSGBOX_NPC
	end

@ Kanto_SixIsland_Mart object 2 (local id 2, gfx 0x3F2F) at 8,3
Kanto_SixIsland_Mart_EventScript_845D3C0::
	lock
	faceplayer
	setvar VAR_0x8004, 0xB
	setvar VAR_0x8005, 5
	special QzQuestLogNop_737
	msgbox Kanto_SixIsland_Mart_Text_8454764, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_Mart_Text_8454764::
	.string "Play with POKéMON lots and live\n"
	.string "a long life!\p"
	.string "Take AGATHA, for example.\p"
	.string "She set a record for being the\n"
	.string "oldest-ever ELITE FOUR member.\p"
	.string "She spurs me to do bigger things.$"

Kanto_SixIsland_Mart_Text_845480B::
	.string "I should buy some REPELS before\n"
	.string "exploring more of the island.$"
