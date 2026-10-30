Kanto_FiveIsland_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FiveIsland_PokemonCenter_1F_EventScript_8421A10
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_FiveIsland_PokemonCenter_1F_EventScript_8421A10::
	setrespawn 0x29
	end

@ Kanto_FiveIsland_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_FiveIsland_PokemonCenter_1F_EventScript_8421A14::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_FiveIsland_PokemonCenter_1F object 3 (local id 3, gfx 0x3EF8) at 4,8
Kanto_FiveIsland_PokemonCenter_1F_EventScript_8421A1D::
	msgbox Kanto_FiveIsland_PokemonCenter_1F_Text_8454152, MSGBOX_NPC
	end

@ Kanto_FiveIsland_PokemonCenter_1F object 2 (local id 2, gfx 0x3EFC) at 9,4
Kanto_FiveIsland_PokemonCenter_1F_EventScript_8421A26::
	msgbox Kanto_FiveIsland_PokemonCenter_1F_Text_8454195, MSGBOX_NPC
	end

Kanto_FiveIsland_PokemonCenter_1F_Text_8454152::
	.string "I’m on my way to prepare a meal\n"
	.string "for a certain high-society person.$"

Kanto_FiveIsland_PokemonCenter_1F_Text_8454195::
	.string "You’re on a quest to catch every\n"
	.string "single kind of POKéMON?\p"
	.string "…How incredible. Please tell me if\n"
	.string "you catch a strong one!$"
