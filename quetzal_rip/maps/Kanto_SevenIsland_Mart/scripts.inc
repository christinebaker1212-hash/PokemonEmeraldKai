Kanto_SevenIsland_Mart_MapScripts::
	.byte 0

@ Kanto_SevenIsland_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_SevenIsland_Mart_EventScript_8420A88::
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

@ Kanto_SevenIsland_Mart object 2 (local id 2, gfx 0x3F16) at 6,2
Kanto_SevenIsland_Mart_EventScript_8420AC6::
	msgbox Kanto_SevenIsland_Mart_Text_8450FF0, MSGBOX_NPC
	end

@ Kanto_SevenIsland_Mart object 4 (local id 4, gfx 0x3F0C) at 5,4
Kanto_SevenIsland_Mart_EventScript_8420ACF::
	msgbox Kanto_SevenIsland_Mart_Text_845106D, MSGBOX_NPC
	end

@ Kanto_SevenIsland_Mart object 3 (local id 3, gfx 0x3F06) at 9,5
Kanto_SevenIsland_Mart_EventScript_8420AD8::
	msgbox Kanto_SevenIsland_Mart_Text_84510AA, MSGBOX_NPC
	end

Kanto_SevenIsland_Mart_Text_8450FF0::
	.string "POKéMON have personalities of their\n"
	.string "own, just like people.\p"
	.string "My PIKACHU has a HASTY nature,\n"
	.string "so it grew to be a speedy POKéMON.$"

Kanto_SevenIsland_Mart_Text_845106D::
	.string "Okay, preparations are complete\n"
	.string "for me to explore the RUINS.$"

Kanto_SevenIsland_Mart_Text_84510AA::
	.string "I need to fish on SEVEN ISLAND.\p"
	.string "That will complete my fishing tour\n"
	.string "of the SEVII ISLANDS.\p"
	.string "I’d better stock up on some\n"
	.string "POKé BALLS first, though.$"
