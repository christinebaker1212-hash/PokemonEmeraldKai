Kanto_LavenderTown_Mart_MapScripts::
	.byte 0

@ Kanto_LavenderTown_Mart object 2 (local id 2, gfx 0x3EE6) at 6,2
Kanto_LavenderTown_Mart_EventScript_841B322::
	msgbox Kanto_LavenderTown_Mart_Text_8443E51, MSGBOX_NPC
	end

@ Kanto_LavenderTown_Mart object 3 (local id 3, gfx 0x3F3B) at 9,2
Kanto_LavenderTown_Mart_EventScript_841B32B::
	msgbox Kanto_LavenderTown_Mart_Text_8443F22, MSGBOX_NPC
	end

@ Kanto_LavenderTown_Mart object 4 (local id 4, gfx 0x3F5C) at 9,5
Kanto_LavenderTown_Mart_EventScript_841B334::
	msgbox Kanto_LavenderTown_Mart_Text_8443F5D, MSGBOX_NPC
	end

@ Kanto_LavenderTown_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_LavenderTown_Mart_EventScript_841B33D::
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

Kanto_LavenderTown_Mart_Text_8443E51::
	.string "I’m searching for items that raise\n"
	.string "the stats of POKéMON.\p"
	.string "They’re effective over the course\n"
	.string "of a single battle.\p"
	.string "X ATTACK, X DEFEND, X SPEED, \n"
	.string "and X SPECIAL are what I’m after.\p"
	.string "Do you know where I can get them?$"

Kanto_LavenderTown_Mart_Text_8443F22::
	.string "Did you buy some REVIVES?\n"
	.string "They revive any fainted POKéMON!$"

Kanto_LavenderTown_Mart_Text_8443F5D::
	.string "Sometimes, a TRAINER duo will\n"
	.string "challenge you with two POKéMON\l"
	.string "at the same time.\p"
	.string "If that happens, you have to send\n"
	.string "out two POKéMON to battle, too.$"
