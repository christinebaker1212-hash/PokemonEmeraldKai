Kanto_CinnabarIsland_Mart_MapScripts::
	.byte 0

@ Kanto_CinnabarIsland_Mart object 2 (local id 2, gfx 0x3F58) at 6,2
Kanto_CinnabarIsland_Mart_EventScript_841EA99::
	msgbox Kanto_CinnabarIsland_Mart_Text_844A6FB, MSGBOX_NPC
	end

@ Kanto_CinnabarIsland_Mart object 3 (local id 3, gfx 0x3F43) at 8,2
Kanto_CinnabarIsland_Mart_EventScript_841EAA2::
	msgbox Kanto_CinnabarIsland_Mart_Text_844A74C, MSGBOX_NPC
	end

@ Kanto_CinnabarIsland_Mart object 1 (local id 1, gfx 0x3EFA) at 2,3
Kanto_CinnabarIsland_Mart_EventScript_841EAAB::
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

Kanto_CinnabarIsland_Mart_Text_844A6FB::
	.string "Don’t they have X ATTACK?\p"
	.string "I like it because it raises the\n"
	.string "ATTACK stat in battle.$"

Kanto_CinnabarIsland_Mart_Text_844A74C::
	.string "It never hurts to have extra items.\n"
	.string "You never know what might happen.$"
