Kanto_PokemonMansion_2F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_PokemonMansion_2F_EventScript_8411F9C
	.byte 0

Kanto_PokemonMansion_2F_EventScript_8411F9C::
	call_if_set FLAG_QZ_0x21EA, Quetzal_EventScript_8458201
	end

@ Kanto_PokemonMansion_2F sign / bg event at 2,16
Kanto_PokemonMansion_2F_EventScript_8411FA6::
	lockall
	setvar VAR_0x8004, 1
	call Quetzal_EventScript_8457FA5
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	releaseall
	end

@ Kanto_PokemonMansion_2F sign / bg event at 25,4
Kanto_PokemonMansion_2F_EventScript_8411FBA::
	msgbox Kanto_PokemonMansion_2F_Text_8427C06, MSGBOX_SIGN
	end

@ Kanto_PokemonMansion_2F sign / bg event at 6,31
Kanto_PokemonMansion_2F_EventScript_8411FC3::
	msgbox Kanto_PokemonMansion_2F_Text_8427C5B, MSGBOX_SIGN
	end

@ Kanto_PokemonMansion_2F object 1 (local id 1, gfx 0x3F4B) at 5,23
Kanto_PokemonMansion_2F_EventScript_8411FCC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8408, 0, Kanto_PokemonMansion_2F_Text_8427B7F, Kanto_PokemonMansion_2F_Text_8427BB2
	msgbox Kanto_PokemonMansion_2F_Text_8427BCA, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonMansion_2F object 2 (local id 2, gfx 0x3F0E) at 36,8
Kanto_PokemonMansion_2F_EventScript_846BE4D::
	setorcopyvar VAR_0x8000, 0x44
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonMansion_2F object 3 (local id 3, gfx 0x3F0E) at 18,23
Kanto_PokemonMansion_2F_EventScript_846C058::
	setorcopyvar VAR_0x8000, 0x45
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonMansion_2F object 4 (local id 4, gfx 0x3F0E) at 35,17
Kanto_PokemonMansion_2F_EventScript_846C065::
	setorcopyvar VAR_0x8000, 0x41
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PokemonMansion_2F_Text_8427B7F::
	.string "I can’t get out!\n"
	.string "This old place is one big puzzle.$"

Kanto_PokemonMansion_2F_Text_8427BB2::
	.string "Oh, no!\n"
	.string "My bag of loot!$"

Kanto_PokemonMansion_2F_Text_8427BCA::
	.string "The switches here open and close\n"
	.string "alternating sets of doors.$"

Kanto_PokemonMansion_2F_Text_8427C06::
	.string "Diary: July 5\n"
	.string "Guyana, South America\p"
	.string "A new POKéMON was discovered deep\n"
	.string "in the jungle.$"

Kanto_PokemonMansion_2F_Text_8427C5B::
	.string "Diary: July 10\n"
	.string "We christened the newly discovered\l"
	.string "POKéMON, MEW.$"
