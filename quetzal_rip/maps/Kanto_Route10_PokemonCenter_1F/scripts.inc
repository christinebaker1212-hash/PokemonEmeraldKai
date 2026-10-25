Kanto_Route10_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route10_PokemonCenter_1F_EventScript_841FBAE
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8365E4C
	.byte 0

Kanto_Route10_PokemonCenter_1F_EventScript_841FBAE::
	setflag FLAG_QZ_0x224D
	setrespawn 0x31
	end

@ Kanto_Route10_PokemonCenter_1F object 1 (local id 1, gfx 0x3F2A) at 7,2
Kanto_Route10_PokemonCenter_1F_EventScript_841FBB5::
	lock
	faceplayer
	call Quetzal_EventScript_84569F1
	release
	end

@ Kanto_Route10_PokemonCenter_1F object 3 (local id 3, gfx 0x3F04) at 2,3
Kanto_Route10_PokemonCenter_1F_EventScript_841FBBE::
	msgbox Kanto_Route10_PokemonCenter_1F_Text_844EA3F, MSGBOX_NPC
	end

@ Kanto_Route10_PokemonCenter_1F object 2 (local id 2, gfx 0x3F09) at 9,4
Kanto_Route10_PokemonCenter_1F_EventScript_841FBC7::
	msgbox Kanto_Route10_PokemonCenter_1F_Text_844E9C5, MSGBOX_NPC
	end

@ Kanto_Route10_PokemonCenter_1F object 4 (local id 4, gfx 0x3F5C) at 14,6
Kanto_Route10_PokemonCenter_1F_EventScript_841FBD0::
	msgbox Kanto_Route10_PokemonCenter_1F_Text_844EA74, MSGBOX_NPC
	end

@ Kanto_Route10_PokemonCenter_1F object 5 (local id 5, gfx 0x3F43) at 12,5
Kanto_Route10_PokemonCenter_1F_EventScript_841FBD9::
	lock
	faceplayer
	call Kanto_Route10_PokemonCenter_1F_EventScript_841FC68
	goto_if_set FLAG_QZ_0x2146, Kanto_Route10_PokemonCenter_1F_EventScript_841FC5E
	msgbox Kanto_Route10_PokemonCenter_1F_Text_844EA9D, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F94
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, QzGetPokedexCount_731
	buffernumberstring 2, VAR_0x8006
	call Kanto_Route10_PokemonCenter_1F_EventScript_841FC68
	goto_if_lt VAR_0x8006, 20, Quetzal_EventScript_8457F80
	msgbox Kanto_Route10_PokemonCenter_1F_Text_844EBD5, MSGBOX_DEFAULT
	checkitemspace ITEM_EVERSTONE, 1
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457F8A
	additem ITEM_EVERSTONE, 1
	loadword 0, Kanto_Route10_PokemonCenter_1F_Text_844EC28
	setorcopyvar VAR_0x8000, 0xF5
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2146
	msgbox Kanto_Route10_PokemonCenter_1F_Text_844EC51, MSGBOX_DEFAULT
	release
	end

Kanto_Route10_PokemonCenter_1F_EventScript_841FC5E::
	msgbox Kanto_Route10_PokemonCenter_1F_Text_844EC51, MSGBOX_DEFAULT
	release
	end

Kanto_Route10_PokemonCenter_1F_EventScript_841FC68::
	buffernumberstring 0, 0x14
	bufferitemname 1, ITEM_EVERSTONE
	return

Kanto_Route10_PokemonCenter_1F_Text_844E9C5::
	.string "The types of POKéMON match up\n"
	.string "differently with each other.\p"
	.string "Every type is stronger than some\n"
	.string "types and weaker than others.$"

Kanto_Route10_PokemonCenter_1F_Text_844EA3F::
	.string "A NUGGET is totally useless.\n"
	.string "So I sold it for ¥5000.$"

Kanto_Route10_PokemonCenter_1F_Text_844EA74::
	.string "I heard that ghosts haunt\n"
	.string "LAVENDER TOWN.$"

Kanto_Route10_PokemonCenter_1F_Text_844EA9D::
	.string "Oh… {PLAYER}!\n"
	.string "I’ve been looking for you!\p"
	.string "It’s me, one of the ever-present\n"
	.string "AIDES to PROF. OAK.\p"
	.string "If your POKéDEX has complete data\n"
	.string "on twenty species, I’m supposed to\l"
	.string "give you a reward from PROF. OAK.\p"
	.string "He entrusted me with this\n"
	.string "EVERSTONE.\p"
	.string "So, {PLAYER}, let me ask you.\p"
	.string "Have you gathered data on at least\n"
	.string "twenty kinds of POKéMON?$"

Kanto_Route10_PokemonCenter_1F_Text_844EBD5::
	.string "Great! You have caught or owned\n"
	.string "{STR_VAR_3} kinds of POKéMON!\p"
	.string "Congratulations!\n"
	.string "Here you go!$"

Kanto_Route10_PokemonCenter_1F_Text_844EC28::
	.string "{PLAYER} received the EVERSTONE\n"
	.string "from the AIDE.$"

Kanto_Route10_PokemonCenter_1F_Text_844EC51::
	.string "Making POKéMON evolve certainly\n"
	.string "can add to the POKéDEX.\p"
	.string "However, at times, you may not\n"
	.string "want a certain POKéMON to evolve.\p"
	.string "In that case, give the EVERSTONE\n"
	.string "to that POKéMON.\p"
	.string "It will prevent evolution according\n"
	.string "to the PROFESSOR.$"
