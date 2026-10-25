Kanto_PokemonTower_4F_MapScripts::
	.byte 0

@ Kanto_PokemonTower_4F object 3 (local id 3, gfx 0x3EF6) at 4,12
Kanto_PokemonTower_4F_EventScript_84130C7::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8636, 0, Kanto_PokemonTower_4F_Text_842A6CB, Kanto_PokemonTower_4F_Text_842A6DE
	msgbox Kanto_PokemonTower_4F_Text_842A6F2, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_4F object 1 (local id 1, gfx 0x3EF6) at 17,7
Kanto_PokemonTower_4F_EventScript_84130DE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8637, 0, Kanto_PokemonTower_4F_Text_842A70D, Kanto_PokemonTower_4F_Text_842A728
	msgbox Kanto_PokemonTower_4F_Text_842A72E, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_4F object 2 (local id 2, gfx 0x3EF6) at 15,13
Kanto_PokemonTower_4F_EventScript_84130F5::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8638, 0, Kanto_PokemonTower_4F_Text_842A75D, Kanto_PokemonTower_4F_Text_842A772
	msgbox Kanto_PokemonTower_4F_Text_842A782, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_4F object 4 (local id 4, gfx 0x3F0E) at 12,11
Kanto_PokemonTower_4F_EventScript_846BC18::
	setorcopyvar VAR_0x8000, 0x33
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonTower_4F object 5 (local id 5, gfx 0x3F0E) at 8,11
Kanto_PokemonTower_4F_EventScript_846BC25::
	setorcopyvar VAR_0x8000, 0x2F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonTower_4F object 6 (local id 6, gfx 0x3F0E) at 11,16
Kanto_PokemonTower_4F_EventScript_846BC32::
	setorcopyvar VAR_0x8000, 2
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PokemonTower_4F_Text_842A6CB::
	.string "Ghost! No!\n"
	.string "Kwaaah!$"

Kanto_PokemonTower_4F_Text_842A6DE::
	.string "Where is the ghost?$"

Kanto_PokemonTower_4F_Text_842A6F2::
	.string "I must have been dreaming…$"

Kanto_PokemonTower_4F_Text_842A70D::
	.string "Be cursed with me!\n"
	.string "Kwaaah!$"

Kanto_PokemonTower_4F_Text_842A728::
	.string "What!$"

Kanto_PokemonTower_4F_Text_842A72E::
	.string "We can’t determine the identity of\n"
	.string "the ghosts…$"

Kanto_PokemonTower_4F_Text_842A75D::
	.string "Huhuhu…\n"
	.string "Beat me not!$"

Kanto_PokemonTower_4F_Text_842A772::
	.string "Huh?\n"
	.string "Who? What?$"

Kanto_PokemonTower_4F_Text_842A782::
	.string "May the departed POKéMON rest in\n"
	.string "peace…$"
