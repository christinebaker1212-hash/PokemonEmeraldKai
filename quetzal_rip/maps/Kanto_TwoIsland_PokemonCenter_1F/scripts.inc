Kanto_TwoIsland_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_TwoIsland_PokemonCenter_1F_EventScript_842152D
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_TwoIsland_PokemonCenter_1F_EventScript_842152D::
	setrespawn 0x37
	end

@ Kanto_TwoIsland_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_TwoIsland_PokemonCenter_1F_EventScript_8421531::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_TwoIsland_PokemonCenter_1F object 2 (local id 2, gfx 0x3F08) at 11,5
Kanto_TwoIsland_PokemonCenter_1F_EventScript_842153A::
	msgbox Kanto_TwoIsland_PokemonCenter_1F_Text_84533F1, MSGBOX_NPC
	end

@ Kanto_TwoIsland_PokemonCenter_1F object 3 (local id 3, gfx 0x3EF1) at 4,4
Kanto_TwoIsland_PokemonCenter_1F_EventScript_8421543::
	msgbox Kanto_TwoIsland_PokemonCenter_1F_Text_8453456, MSGBOX_NPC
	end

Kanto_TwoIsland_PokemonCenter_1F_Text_84533F1::
	.string "Have you visited the GAME CORNER\n"
	.string "already?\p"
	.string "It’s no good to go alone.\n"
	.string "You have to take a friend along.$"

Kanto_TwoIsland_PokemonCenter_1F_Text_8453456::
	.string "Not many people live on\n"
	.string "TWO ISLAND.\p"
	.string "There is this old lady who lives\n"
	.string "out on the cape.\p"
	.string "She’s been there since I don’t know\n"
	.string "when.$"
