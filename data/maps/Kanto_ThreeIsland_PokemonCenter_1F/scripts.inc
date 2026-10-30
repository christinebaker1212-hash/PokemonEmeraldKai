Kanto_ThreeIsland_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_ThreeIsland_PokemonCenter_1F_EventScript_84215A1
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_ThreeIsland_PokemonCenter_1F_EventScript_84215A1::
	setrespawn 0x36
	end

@ Kanto_ThreeIsland_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_ThreeIsland_PokemonCenter_1F_EventScript_84215A5::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_ThreeIsland_PokemonCenter_1F object 4 (local id 4, gfx 0x3F41) at 11,5
Kanto_ThreeIsland_PokemonCenter_1F_EventScript_84215AE::
	msgbox Kanto_ThreeIsland_PokemonCenter_1F_Text_84535D3, MSGBOX_NPC
	end

@ Kanto_ThreeIsland_PokemonCenter_1F object 3 (local id 3, gfx 0x3F16) at 3,4
Kanto_ThreeIsland_PokemonCenter_1F_EventScript_84215B7::
	msgbox Kanto_ThreeIsland_PokemonCenter_1F_Text_845363B, MSGBOX_NPC
	end

@ Kanto_ThreeIsland_PokemonCenter_1F object 2 (local id 2, gfx 0x3F3B) at 12,2
Kanto_ThreeIsland_PokemonCenter_1F_EventScript_84215C0::
	msgbox Kanto_ThreeIsland_PokemonCenter_1F_Text_845354D, MSGBOX_NPC
	end

Kanto_ThreeIsland_PokemonCenter_1F_Text_845354D::
	.string "Hey, did you hear the news?\p"
	.string "The PC network here can now link\n"
	.string "with PCs in KANTO.\p"
	.string "I don’t know how that came about,\n"
	.string "but it’s fantastic!$"

Kanto_ThreeIsland_PokemonCenter_1F_Text_84535D3::
	.string "It’s impossible to SURF between the\n"
	.string "islands around these parts.\p"
	.string "The tides are too fast and\n"
	.string "treacherous.$"

Kanto_ThreeIsland_PokemonCenter_1F_Text_845363B::
	.string "Let’s crush BERRIES!\n"
	.string "…That’d be wasting BERRIES?\p"
	.string "You can always find some BERRIES\n"
	.string "on the ground in BERRY FOREST.$"
