Kanto_PokemonMansion_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_PokemonMansion_B1F_EventScript_841204E
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

Kanto_PokemonMansion_B1F_EventScript_841204E::
	call_if_set FLAG_QZ_0x21EA, Quetzal_EventScript_845840F
	end

@ Kanto_PokemonMansion_B1F sign / bg event at 24,29
@ Kanto_PokemonMansion_B1F sign / bg event at 27,5
Kanto_PokemonMansion_B1F_EventScript_8412058::
	lockall
	setvar VAR_0x8004, 3
	call Quetzal_EventScript_8457FA5
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	releaseall
	end

@ Kanto_PokemonMansion_B1F sign / bg event at 21,27
Kanto_PokemonMansion_B1F_EventScript_841206C::
	msgbox Kanto_PokemonMansion_B1F_Text_8427E7F, MSGBOX_SIGN
	end

@ Kanto_PokemonMansion_B1F object 2 (local id 2, gfx 0x3F4B) at 20,25
Kanto_PokemonMansion_B1F_EventScript_8412075::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8411, 0, Kanto_PokemonMansion_B1F_Text_8427DA7, Kanto_PokemonMansion_B1F_Text_8427DBE
	msgbox Kanto_PokemonMansion_B1F_Text_8427DC5, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonMansion_B1F object 3 (local id 3, gfx 0x3F43) at 34,13
Kanto_PokemonMansion_B1F_EventScript_841208C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8539, 0, Kanto_PokemonMansion_B1F_Text_8427E07, Kanto_PokemonMansion_B1F_Text_8427E3E
	msgbox Kanto_PokemonMansion_B1F_Text_8427E51, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonMansion_B1F object 4 (local id 4, gfx 0x3F0E) at 23,4
Kanto_PokemonMansion_B1F_EventScript_846BE74::
	setorcopyvar VAR_0x8000, ITEM_NEVER_MELT_ICE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonMansion_B1F object 5 (local id 5, gfx 0x3F0E) at 2,29
Kanto_PokemonMansion_B1F_EventScript_846BE81::
	setorcopyvar VAR_0x8000, ITEM_FULL_RESTORE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonMansion_B1F object 1 (local id 1, gfx 0x3F0E) at 6,21
Kanto_PokemonMansion_B1F_EventScript_846BE8E::
	setorcopyvar VAR_0x8000, ITEM_SHINY_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonMansion_B1F object 6 (local id 6, gfx 0x3F0E) at 5,7
Kanto_PokemonMansion_B1F_EventScript_846BE9B::
	checkitem ITEM_SECRET_KEY, 1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FD2E
	setorcopyvar VAR_0x8000, ITEM_SECRET_KEY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PokemonMansion_B1F_Text_8427DA7::
	.string "Uh-oh…\n"
	.string "Where am I now?$"

Kanto_PokemonMansion_B1F_Text_8427DBE::
	.string "Awooh!$"

Kanto_PokemonMansion_B1F_Text_8427DC5::
	.string "You can find stuff lying around.\n"
	.string "The residents must’ve left them.$"

Kanto_PokemonMansion_B1F_Text_8427E07::
	.string "This place is ideal for a lab.\n"
	.string "Look at all this space.$"

Kanto_PokemonMansion_B1F_Text_8427E3E::
	.string "What was that for?$"

Kanto_PokemonMansion_B1F_Text_8427E51::
	.string "I like it here.\n"
	.string "It’s conducive to my studies.$"

Kanto_PokemonMansion_B1F_Text_8427E7F::
	.string "Diary: Sept. 1\n"
	.string "MEWTWO is far too powerful.\p"
	.string "We have failed to curb its vicious\n"
	.string "tendencies…$"
