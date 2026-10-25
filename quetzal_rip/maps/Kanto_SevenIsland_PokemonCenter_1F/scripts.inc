Kanto_SevenIsland_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SevenIsland_PokemonCenter_1F_EventScript_8420AEC
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_SevenIsland_PokemonCenter_1F_EventScript_8420AEC::
	setrespawn 0x34
	end

@ Kanto_SevenIsland_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_SevenIsland_PokemonCenter_1F_EventScript_8420AF0::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_SevenIsland_PokemonCenter_1F object 2 (local id 2, gfx 0x3F2C) at 11,5
Kanto_SevenIsland_PokemonCenter_1F_EventScript_8420AF9::
	msgbox Kanto_SevenIsland_PokemonCenter_1F_Text_8451139, MSGBOX_NPC
	end

@ Kanto_SevenIsland_PokemonCenter_1F object 3 (local id 3, gfx 0x3F41) at 14,6
Kanto_SevenIsland_PokemonCenter_1F_EventScript_8420B02::
	msgbox Kanto_SevenIsland_PokemonCenter_1F_Text_84511C0, MSGBOX_NPC
	end

@ Kanto_SevenIsland_PokemonCenter_1F object 4 (local id 4, gfx 0x3EE8) at 4,4
Kanto_SevenIsland_PokemonCenter_1F_EventScript_8420B0B::
	msgbox Kanto_SevenIsland_PokemonCenter_1F_Text_8451246, MSGBOX_NPC
	end

Kanto_SevenIsland_PokemonCenter_1F_Text_8451139::
	.string "On this island, different events\n"
	.string "occur depending on connections.\p"
	.string "What are those connections?\p"
	.string "They are for you to discover on\n"
	.string "your own.$"

Kanto_SevenIsland_PokemonCenter_1F_Text_84511C0::
	.string "Did you ride a SEAGALLOP ferry\n"
	.string "to get here?\p"
	.string "There are at least ten of those\n"
	.string "high-speed ships.\p"
	.string "I don’t know where they all go,\n"
	.string "though.$"

Kanto_SevenIsland_PokemonCenter_1F_Text_8451246::
	.string "If you’re confident, you should\n"
	.string "check out TRAINER TOWER.$"
