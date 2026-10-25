Kanto_FourIsland_Mart_MapScripts::
	.byte 0

@ Kanto_FourIsland_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_FourIsland_Mart_EventScript_84219C1::
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

@ Kanto_FourIsland_Mart object 3 (local id 3, gfx 0x3EF3) at 8,3
Kanto_FourIsland_Mart_EventScript_84219FC::
	msgbox Kanto_FourIsland_Mart_Text_84540AC, MSGBOX_NPC
	end

@ Kanto_FourIsland_Mart object 2 (local id 2, gfx 0x3F2C) at 6,5
Kanto_FourIsland_Mart_EventScript_845D317::
	lock
	faceplayer
	setvar VAR_0x8004, 9
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	msgbox Kanto_FourIsland_Mart_Text_8453FF8, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_Mart_Text_8453FF8::
	.string "We’ve had a great and powerful\n"
	.string "TRAINER grow up on this island.\p"
	.string "I bet even you’d know her.\n"
	.string "It’s LORELEI of the ELITE FOUR!\p"
	.string "When LORELEI was just a tyke,\n"
	.string "I taught her about POKéMON.$"

Kanto_FourIsland_Mart_Text_84540AC::
	.string "The SEVII ISLANDS are in a warm\n"
	.string "region overall.\p"
	.string "But there is one exception you\n"
	.string "need to know about.\p"
	.string "The ICEFALL CAVE is frigid because\n"
	.string "of the POKéMON that live in it.$"
