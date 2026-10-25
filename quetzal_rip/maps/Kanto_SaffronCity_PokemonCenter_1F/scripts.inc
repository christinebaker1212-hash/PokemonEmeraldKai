Kanto_SaffronCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SaffronCity_PokemonCenter_1F_EventScript_841F0A3
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_SaffronCity_PokemonCenter_1F_EventScript_841F0A3::
	setrespawn 0x33
	end

@ Kanto_SaffronCity_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_SaffronCity_PokemonCenter_1F_EventScript_841F0A7::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_SaffronCity_PokemonCenter_1F object 2 (local id 2, gfx 0x3F09) at 14,6
Kanto_SaffronCity_PokemonCenter_1F_EventScript_841F0B0::
	msgbox Kanto_SaffronCity_PokemonCenter_1F_Text_844BFB3, MSGBOX_NPC
	end

@ Kanto_SaffronCity_PokemonCenter_1F object 3 (local id 3, gfx 0x3F59) at 4,4
Kanto_SaffronCity_PokemonCenter_1F_EventScript_841F0B9::
	msgbox Kanto_SaffronCity_PokemonCenter_1F_Text_844BF7E, MSGBOX_NPC
	end

@ Kanto_SaffronCity_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 9,6
Kanto_SaffronCity_PokemonCenter_1F_EventScript_841F0C2::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x512D, 1, Kanto_SaffronCity_PokemonCenter_1F_EventScript_841F0D9
	msgbox Kanto_SaffronCity_PokemonCenter_1F_Text_844C00F, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonCenter_1F_EventScript_841F0D9::
	msgbox Kanto_SaffronCity_PokemonCenter_1F_Text_844C05D, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_PokemonCenter_1F_Text_844BF7E::
	.string "POKéMON growth rates differ from\n"
	.string "species to species.$"

Kanto_SaffronCity_PokemonCenter_1F_Text_844BFB3::
	.string "SILPH CO. is widely known to all.\p"
	.string "It’s a victim of its own fame - it\n"
	.string "attracted TEAM ROCKET.$"

Kanto_SaffronCity_PokemonCenter_1F_Text_844C00F::
	.string "Sigh…\n"
	.string "Wouldn’t this be great?\p"
	.string "If the ELITE FOUR came and\n"
	.string "stomped TEAM ROCKET?$"

Kanto_SaffronCity_PokemonCenter_1F_Text_844C05D::
	.string "TEAM ROCKET took off!\n"
	.string "We can go out safely again!\l"
	.string "That’s great!$"
