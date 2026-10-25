Kanto_ViridianCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_ViridianCity_PokemonCenter_1F_EventScript_841A2D3
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_ViridianCity_PokemonCenter_1F_EventScript_841A2D3::
	setrespawn 0x39
	end

@ Kanto_ViridianCity_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_ViridianCity_PokemonCenter_1F_EventScript_841A2D7::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_ViridianCity_PokemonCenter_1F object 3 (local id 3, gfx 0x3EEE) at 4,7
Kanto_ViridianCity_PokemonCenter_1F_EventScript_841A2E0::
	msgbox Kanto_ViridianCity_PokemonCenter_1F_Text_8440741, MSGBOX_NPC
	end

@ Kanto_ViridianCity_PokemonCenter_1F object 2 (local id 2, gfx 0x3F09) at 12,5
Kanto_ViridianCity_PokemonCenter_1F_EventScript_841A2E9::
	msgbox Kanto_ViridianCity_PokemonCenter_1F_Text_84406E0, MSGBOX_NPC
	end

@ Kanto_ViridianCity_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 2,3
Kanto_ViridianCity_PokemonCenter_1F_EventScript_841A2F2::
	msgbox Kanto_ViridianCity_PokemonCenter_1F_Text_84407AC, MSGBOX_NPC
	end

Kanto_ViridianCity_PokemonCenter_1F_Text_84406E0::
	.string "Please feel free to use that PC in\n"
	.string "the corner.\p"
	.string "The receptionist told me so.\n"
	.string "It’s so kind of her!$"

Kanto_ViridianCity_PokemonCenter_1F_Text_8440741::
	.string "There’s a POKéMON CENTER in every\n"
	.string "town ahead.\p"
	.string "They charge no money, so don’t\n"
	.string "be shy about healing POKéMON.$"

Kanto_ViridianCity_PokemonCenter_1F_Text_84407AC::
	.string "POKéMON CENTERS heal your tired,\n"
	.string "hurt, or fainted POKéMON.\p"
	.string "They make all POKéMON completely\n"
	.string "healthy.$"
