Kanto_PokemonMansion_3F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	map_script MAP_SCRIPT_ON_LOAD, Kanto_PokemonMansion_3F_EventScript_8411FEE
	.byte 0

Kanto_PokemonMansion_3F_EventScript_8411FEE::
	call_if_set FLAG_QZ_0x21EA, Quetzal_EventScript_8458335
	end

@ Kanto_PokemonMansion_3F sign / bg event at 12,5
Kanto_PokemonMansion_3F_EventScript_8411FF8::
	lockall
	setvar VAR_0x8004, 2
	call Quetzal_EventScript_8457FA5
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	releaseall
	end

@ Kanto_PokemonMansion_3F sign / bg event at 8,16
Kanto_PokemonMansion_3F_EventScript_841200C::
	msgbox Kanto_PokemonMansion_3F_Text_8427D6C, MSGBOX_SIGN
	end

@ Kanto_PokemonMansion_3F object 1 (local id 1, gfx 0x3F4B) at 7,13
Kanto_PokemonMansion_3F_EventScript_8412015::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8410, 0, Kanto_PokemonMansion_3F_Text_8427CCE, Kanto_PokemonMansion_3F_Text_8427CE9
	msgbox Kanto_PokemonMansion_3F_Text_8427CEF, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonMansion_3F object 2 (local id 2, gfx 0x3F43) at 22,13
Kanto_PokemonMansion_3F_EventScript_841202C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8538, 0, Kanto_PokemonMansion_3F_Text_8427D0F, Kanto_PokemonMansion_3F_Text_8427D2A
	msgbox Kanto_PokemonMansion_3F_Text_8427D3E, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonMansion_3F object 3 (local id 3, gfx 0x3F0E) at 9,19
Kanto_PokemonMansion_3F_EventScript_846BE5A::
	setorcopyvar VAR_0x8000, ITEM_MAX_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonMansion_3F object 4 (local id 4, gfx 0x3F0E) at 32,6
Kanto_PokemonMansion_3F_EventScript_846BE67::
	setorcopyvar VAR_0x8000, ITEM_IRON
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PokemonMansion_3F_Text_8427CCE::
	.string "This place is, like, huge!$"

Kanto_PokemonMansion_3F_Text_8427CE9::
	.string "Ayah!$"

Kanto_PokemonMansion_3F_Text_8427CEF::
	.string "I wonder where my partner went.$"

Kanto_PokemonMansion_3F_Text_8427D0F::
	.string "My mentor once lived here.$"

Kanto_PokemonMansion_3F_Text_8427D2A::
	.string "Whew!\n"
	.string "Overwhelming!$"

Kanto_PokemonMansion_3F_Text_8427D3E::
	.string "So, you’re stuck?\n"
	.string "Try jumping off over there!$"

Kanto_PokemonMansion_3F_Text_8427D6C::
	.string "Diary: Feb. 6\n"
	.string "MEW gave birth.\p"
	.string "We named the newborn MEWTWO.$"
