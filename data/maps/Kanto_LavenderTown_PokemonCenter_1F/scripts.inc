Kanto_LavenderTown_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_LavenderTown_PokemonCenter_1F_EventScript_841B0DD
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_LavenderTown_PokemonCenter_1F_EventScript_841B0DD::
	setrespawn 0x2D
	end

@ Kanto_LavenderTown_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_LavenderTown_PokemonCenter_1F_EventScript_841B0E1::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_LavenderTown_PokemonCenter_1F object 2 (local id 2, gfx 0x3F09) at 9,4
Kanto_LavenderTown_PokemonCenter_1F_EventScript_841B0EA::
	msgbox Kanto_LavenderTown_PokemonCenter_1F_Text_84435C7, MSGBOX_NPC
	end

@ Kanto_LavenderTown_PokemonCenter_1F object 3 (local id 3, gfx 0x3F16) at 5,6
Kanto_LavenderTown_PokemonCenter_1F_EventScript_841B0F3::
	msgbox Kanto_LavenderTown_PokemonCenter_1F_Text_8443640, MSGBOX_NPC
	end

@ Kanto_LavenderTown_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 2,2
Kanto_LavenderTown_PokemonCenter_1F_EventScript_841B0FC::
	msgbox Kanto_LavenderTown_PokemonCenter_1F_Text_844369C, MSGBOX_NPC
	end

@ Kanto_LavenderTown_PokemonCenter_1F object 5 (local id 5, gfx 0x3EE6) at 14,6
Kanto_LavenderTown_PokemonCenter_1F_EventScript_845D471::
	lock
	faceplayer
	setvar VAR_0x8004, 0xE
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	msgbox Kanto_LavenderTown_PokemonCenter_1F_Text_84436F0, MSGBOX_DEFAULT
	release
	end

Kanto_LavenderTown_PokemonCenter_1F_Text_84435C7::
	.string "TEAM ROCKET will do anything for\n"
	.string "the sake of money!\p"
	.string "There is no job too dirty, no deed\n"
	.string "too heinous, no crime too wicked!$"

Kanto_LavenderTown_PokemonCenter_1F_Text_8443640::
	.string "I saw CUBONE’s mother trying to\n"
	.string "escape from TEAM ROCKET.\p"
	.string "She was killed trying to get away…$"

Kanto_LavenderTown_PokemonCenter_1F_Text_844369C::
	.string "You know how the CUBONE species\n"
	.string "wears skulls, right?\p"
	.string "People will pay a lot for one.$"

Kanto_LavenderTown_PokemonCenter_1F_Text_84436F0::
	.string "I recently moved to this town.\p"
	.string "I hear that MR. FUJI’s not from\n"
	.string "these parts originally, either.$"
