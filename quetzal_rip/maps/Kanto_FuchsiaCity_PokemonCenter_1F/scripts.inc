Kanto_FuchsiaCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FuchsiaCity_PokemonCenter_1F_EventScript_841D8A1
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_FuchsiaCity_PokemonCenter_1F_EventScript_841D8A1::
	setrespawn 0x2B
	end

@ Kanto_FuchsiaCity_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_FuchsiaCity_PokemonCenter_1F_EventScript_841D8A5::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_FuchsiaCity_PokemonCenter_1F object 2 (local id 2, gfx 0x3F1E) at 12,5
Kanto_FuchsiaCity_PokemonCenter_1F_EventScript_841D8AE::
	msgbox Kanto_FuchsiaCity_PokemonCenter_1F_Text_8448C32, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity_PokemonCenter_1F object 3 (local id 3, gfx 0x3EFC) at 13,3
Kanto_FuchsiaCity_PokemonCenter_1F_EventScript_841D8B7::
	msgbox Kanto_FuchsiaCity_PokemonCenter_1F_Text_8448CA9, MSGBOX_NPC
	end

@ Kanto_FuchsiaCity_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 4,8
Kanto_FuchsiaCity_PokemonCenter_1F_EventScript_841D8C0::
	msgbox Kanto_FuchsiaCity_PokemonCenter_1F_Text_8448D16, MSGBOX_NPC
	end

Kanto_FuchsiaCity_PokemonCenter_1F_Text_8448C32::
	.string "You can’t become a good TRAINER\n"
	.string "with just one strong POKéMON.\p"
	.string "But raising many POKéMON evenly\n"
	.string "is no easy task, either.$"

Kanto_FuchsiaCity_PokemonCenter_1F_Text_8448CA9::
	.string "There’s a narrow trail west of\n"
	.string "VIRIDIAN CITY.\p"
	.string "It goes to the POKéMON LEAGUE HQ.\n"
	.string "The HQ governs all TRAINERS.$"

Kanto_FuchsiaCity_PokemonCenter_1F_Text_8448D16::
	.string "If you’re working on a POKéDEX,\n"
	.string "visit the SAFARI ZONE.\p"
	.string "All sorts of rare POKéMON breed\n"
	.string "there.$"
