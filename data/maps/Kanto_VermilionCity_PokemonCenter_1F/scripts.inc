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
	@ Moemon Kai: FireRed's VS SEEKER woman (pokefirered data/event_scripts.s VermilionCity_PokemonCenter_1F_EventScript_
	@ VSSeekerWoman; docs/REGIONS_AUDIT.md 24): Quetzal's ABILITY PATCH lines over a PP MAX are gone (src/mp/mp_vsseeker.c)
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2176, Kanto_VermilionCity_PokemonCenter_1F_EventScript_8459202
	msgbox Kanto_VermilionCity_PokemonCenter_1F_Text_84443A0, MSGBOX_DEFAULT
	giveitem ITEM_VS_SEEKER
	goto_if_eq VAR_RESULT, FALSE, Quetzal_EventScript_84570B3
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
	.string "MOéMON can have very different\l"
	.string "stats and abilities.\p"
	.string "A MOéMON raised by a TRAINER is\n"
	.string "stronger than one in the wild.$"

Kanto_VermilionCity_PokemonCenter_1F_Text_84442B1::
	.string "My MOéMON was poisoned!\n"
	.string "It fainted while we were walking!$"

Kanto_VermilionCity_PokemonCenter_1F_Text_84442EC::
	.string "It is true that a higher-level\n"
	.string "MOéMON will be more powerful…\p"
	.string "But, all MOéMON will have weak\n"
	.string "points against specific types.\p"
	.string "So, there appears to be no\n"
	.string "universally strong MOéMON.$"

Kanto_VermilionCity_PokemonCenter_1F_Text_84443A0::
	.string "The urge to battle with someone\n"
	.string "you’ve tangled with before…\p"
	.string "Have you ever had that urge?\n"
	.string "I’m sure you have.\p"
	.string "I wanted to battle certain people\n"
	.string "again over and over, too.\p"
	.string "So, I’ve been giving these away.\n"
	.string "Please, take one!$"

Kanto_VermilionCity_PokemonCenter_1F_Text_8444410::
	.string "Use that device and you’ll find\n"
	.string "TRAINERS looking for a rematch.\p"
	.string "You have to charge its battery to\n"
	.string "use it, though.$"

Kanto_VermilionCity_PokemonCenter_1F_Text_844446F::
	.string "How do you use the VS SEEKER?\n"
	.string "There’s nothing to it.\p"
	.string "Use it like beep-beep-beep, and\n"
	.string "TRAINERS around you will notice.\p"
	.string "If any TRAINER wants a rematch,\n"
	.string "it will let you know immediately.\p"
	.string "Charge its battery and use it\n"
	.string "while you’re on a road.$"
