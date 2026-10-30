Kanto_VermilionCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_VermilionCity_PokemonCenter_1F_EventScript_841B411
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_VermilionCity_PokemonCenter_1F_EventScript_841B411::
	setrespawn 0x38
	end

@ Kanto_VermilionCity_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_VermilionCity_PokemonCenter_1F_EventScript_841B415::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_VermilionCity_PokemonCenter_1F object 2 (local id 2, gfx 0x3F1E) at 4,8
Kanto_VermilionCity_PokemonCenter_1F_EventScript_841B41E::
	msgbox Kanto_VermilionCity_PokemonCenter_1F_Text_84442B1, MSGBOX_NPC
	end

@ Kanto_VermilionCity_PokemonCenter_1F object 3 (local id 3, gfx 0x3F0C) at 12,5
Kanto_VermilionCity_PokemonCenter_1F_EventScript_841B427::
	msgbox Kanto_VermilionCity_PokemonCenter_1F_Text_844421B, MSGBOX_NPC
	end

@ Kanto_VermilionCity_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 14,4
Kanto_VermilionCity_PokemonCenter_1F_EventScript_841B430::
	msgbox Kanto_VermilionCity_PokemonCenter_1F_Text_84442EC, MSGBOX_NPC
	end

@ Kanto_VermilionCity_PokemonCenter_1F object 5 (local id 5, gfx 0x3EFC) at 6,4
Kanto_VermilionCity_PokemonCenter_1F_EventScript_84591CB::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2176, Kanto_VermilionCity_PokemonCenter_1F_EventScript_8459202
	msgbox Kanto_VermilionCity_PokemonCenter_1F_Text_84443A0, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_PP_MAX
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_84570B3
	setflag FLAG_QZ_0x2176
	msgbox Kanto_VermilionCity_PokemonCenter_1F_Text_8444410, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_PokemonCenter_1F_EventScript_8459202::
	msgbox Kanto_VermilionCity_PokemonCenter_1F_Text_844446F, MSGBOX_DEFAULT
	release
	end

Kanto_VermilionCity_PokemonCenter_1F_Text_844421B::
	.string "Even if they are the same level,\n"
	.string "POKéMON can have very different\l"
	.string "stats and abilities.\p"
	.string "A POKéMON raised by a TRAINER is\n"
	.string "stronger than one in the wild.$"

Kanto_VermilionCity_PokemonCenter_1F_Text_84442B1::
	.string "My POKéMON was poisoned!\n"
	.string "It fainted while we were walking!$"

Kanto_VermilionCity_PokemonCenter_1F_Text_84442EC::
	.string "It is true that a higher-level\n"
	.string "POKéMON will be more powerful…\p"
	.string "But, all POKéMON will have weak\n"
	.string "points against specific types.\p"
	.string "So, there appears to be no\n"
	.string "universally strong POKéMON.$"

Kanto_VermilionCity_PokemonCenter_1F_Text_84443A0::
	.string "I collect useful training items.\p"
	.string "This ABILITY PATCH is rare, but\n"
	.string "you look like you can use it.\p"
	.string "Please, take it!$"

Kanto_VermilionCity_PokemonCenter_1F_Text_8444410::
	.string "An ABILITY PATCH can change a\n"
	.string "POKéMON to its Hidden Ability.\p"
	.string "Choose carefully when you use it!$"

Kanto_VermilionCity_PokemonCenter_1F_Text_844446F::
	.string "That ABILITY PATCH can reveal a\n"
	.string "POKéMON’s Hidden Ability.\p"
	.string "It’s rare, so use it wisely.$"
