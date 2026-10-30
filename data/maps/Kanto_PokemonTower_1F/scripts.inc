Kanto_PokemonTower_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PokemonTower_1F_EventScript_8412F2C
	.byte 0

Kanto_PokemonTower_1F_EventScript_8412F2C::
	setflag FLAG_QZ_0x2249
	end

@ Kanto_PokemonTower_1F object 2 (local id 2, gfx 0x3EF6) at 18,7
Kanto_PokemonTower_1F_EventScript_8412F30::
	msgbox Kanto_PokemonTower_1F_Text_842A36E, MSGBOX_NPC
	end

@ Kanto_PokemonTower_1F object 3 (local id 3, gfx 0x3F58) at 15,6
Kanto_PokemonTower_1F_EventScript_8412F39::
	msgbox Kanto_PokemonTower_1F_Text_842A34F, MSGBOX_NPC
	end

@ Kanto_PokemonTower_1F object 5 (local id 5, gfx 0x3EE6) at 8,12
Kanto_PokemonTower_1F_EventScript_8412F42::
	msgbox Kanto_PokemonTower_1F_Text_842A304, MSGBOX_NPC
	end

@ Kanto_PokemonTower_1F object 4 (local id 4, gfx 0x3F59) at 7,8
Kanto_PokemonTower_1F_EventScript_8412F4B::
	lock
	faceplayer
	checkplayergender
	goto_if_eq VAR_RESULT, 0, Kanto_PokemonTower_1F_EventScript_8412F63
	msgbox Kanto_PokemonTower_1F_Text_842A2BF, MSGBOX_DEFAULT
	release
	end

Kanto_PokemonTower_1F_EventScript_8412F63::
	msgbox Kanto_PokemonTower_1F_Text_842A27B, MSGBOX_DEFAULT
	release
	end

@ Kanto_PokemonTower_1F object 1 (local id 1, gfx 0x3F5A) at 18,13
Kanto_PokemonTower_1F_EventScript_8412F6D::
	msgbox Kanto_PokemonTower_1F_Text_842A23D, MSGBOX_NPC
	end

Kanto_PokemonTower_1F_Text_842A23D::
	.string "POKéMON TOWER was erected in the\n"
	.string "memory of POKéMON that died.$"

Kanto_PokemonTower_1F_Text_842A27B::
	.string "Did you come to pay respects?\p"
	.string "Bless your POKéMON-loving heart,\n"
	.string "son.$"

Kanto_PokemonTower_1F_Text_842A2BF::
	.string "Did you come to pay respects?\p"
	.string "Bless your POKéMON-loving heart,\n"
	.string "girl.$"

Kanto_PokemonTower_1F_Text_842A304::
	.string "I came to pray for my dearly\n"
	.string "departed CLEFAIRY.\p"
	.string "Sniff!\n"
	.string "I’m awash in tears…$"

Kanto_PokemonTower_1F_Text_842A34F::
	.string "My GROWLITHE…\n"
	.string "Why did you die?$"

Kanto_PokemonTower_1F_Text_842A36E::
	.string "I am a CHANNELER.\p"
	.string "There are spirits up to mischief.\n"
	.string "I sense them high in the TOWER.$"
