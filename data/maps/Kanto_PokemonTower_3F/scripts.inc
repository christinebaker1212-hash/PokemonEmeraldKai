Kanto_PokemonTower_3F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_PokemonTower_3F object 1 (local id 1, gfx 0x3EF6) at 10,14
Kanto_PokemonTower_3F_EventScript_8413081::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8633, 0, Kanto_PokemonTower_3F_Text_842A667, Kanto_PokemonTower_3F_Text_842A683
	msgbox Kanto_PokemonTower_3F_Text_842A69A, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_3F object 2 (local id 2, gfx 0x3EF6) at 9,9
Kanto_PokemonTower_3F_EventScript_8413098::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8634, 0, Kanto_PokemonTower_3F_Text_842A629, Kanto_PokemonTower_3F_Text_842A639
	msgbox Kanto_PokemonTower_3F_Text_842A64F, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_3F object 3 (local id 3, gfx 0x3EF6) at 12,4
Kanto_PokemonTower_3F_EventScript_84130AF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8635, 0, Kanto_PokemonTower_3F_Text_842A5D0, Kanto_PokemonTower_3F_Text_842A5E8
	msgbox Kanto_PokemonTower_3F_Text_842A5F8, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_3F object 4 (local id 4, gfx 0x3F0E) at 13,2
Kanto_PokemonTower_3F_EventScript_846BC0B::
	setorcopyvar VAR_0x8000, ITEM_ESCAPE_ROPE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PokemonTower_3F_Text_842A5D0::
	.string "Urrg… Awaa…\n"
	.string "Huhu… Graa…$"

Kanto_PokemonTower_3F_Text_842A5E8::
	.string "Hwa!\n"
	.string "I’m saved!$"

Kanto_PokemonTower_3F_Text_842A5F8::
	.string "The ghosts can be identified by\n"
	.string "the SILPH SCOPE.$"

Kanto_PokemonTower_3F_Text_842A629::
	.string "Kekeke…\n"
	.string "Kwaaah!$"

Kanto_PokemonTower_3F_Text_842A639::
	.string "Hmm?\n"
	.string "What am I doing?$"

Kanto_PokemonTower_3F_Text_842A64F::
	.string "Sorry!\n"
	.string "I was possessed!$"

Kanto_PokemonTower_3F_Text_842A667::
	.string "Be gone!\n"
	.string "Malevolent spirit!$"

Kanto_PokemonTower_3F_Text_842A683::
	.string "Whew!\n"
	.string "The spirit left!$"

Kanto_PokemonTower_3F_Text_842A69A::
	.string "The others above…\n"
	.string "They must have been possessed.$"
