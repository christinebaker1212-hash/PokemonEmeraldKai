Kanto_CeladonCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CeladonCity_PokemonCenter_1F_EventScript_841C8D4
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_CeladonCity_PokemonCenter_1F_EventScript_841C8D4::
	setrespawn 0x26
	end

@ Kanto_CeladonCity_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_CeladonCity_PokemonCenter_1F_EventScript_841C8D8::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_CeladonCity_PokemonCenter_1F object 2 (local id 2, gfx 0x3F09) at 4,4
Kanto_CeladonCity_PokemonCenter_1F_EventScript_841C8E1::
	msgbox Kanto_CeladonCity_PokemonCenter_1F_Text_84469AA, MSGBOX_NPC
	end

@ Kanto_CeladonCity_PokemonCenter_1F object 3 (local id 3, gfx 0x3EFC) at 10,6
Kanto_CeladonCity_PokemonCenter_1F_EventScript_841C8EA::
	msgbox Kanto_CeladonCity_PokemonCenter_1F_Text_8446A11, MSGBOX_NPC
	end

@ Kanto_CeladonCity_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 12,5
Kanto_CeladonCity_PokemonCenter_1F_EventScript_841C8F3::
	msgbox Kanto_CeladonCity_PokemonCenter_1F_Text_8446A62, MSGBOX_NPC
	end

Kanto_CeladonCity_PokemonCenter_1F_Text_84469AA::
	.string "A POKé FLUTE awakens sleeping\n"
	.string "POKéMON. You know that.\p"
	.string "It does so with a sound that only\n"
	.string "they can hear.$"

Kanto_CeladonCity_PokemonCenter_1F_Text_8446A11::
	.string "I rode here from FUCHSIA.\p"
	.string "It’s an uphill ride on CYCLING\n"
	.string "ROAD, so I’m exhausted.$"

Kanto_CeladonCity_PokemonCenter_1F_Text_8446A62::
	.string "If I had a BIKE, I would go to\n"
	.string "CYCLING ROAD!$"
