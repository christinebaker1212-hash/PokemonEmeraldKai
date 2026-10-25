Kanto_PokemonMansion_1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_PokemonMansion_1F_EventScript_8411F46
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PokemonMansion_1F_EventScript_8411F50
	.byte 0

Kanto_PokemonMansion_1F_EventScript_8411F46::
	call_if_set FLAG_QZ_0x21EA, Quetzal_EventScript_845803D
	end

Kanto_PokemonMansion_1F_EventScript_8411F50::
	setflag FLAG_QZ_0x2248
	end

@ Kanto_PokemonMansion_1F sign / bg event at 5,5
Kanto_PokemonMansion_1F_EventScript_8411F54::
	lockall
	setvar VAR_0x8004, 0
	call Quetzal_EventScript_8457FA5
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	releaseall
	end

@ Kanto_PokemonMansion_1F object 1 (local id 1, gfx 0x3F43) at 22,24
Kanto_PokemonMansion_1F_EventScript_8411F68::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8527, 0, Kanto_PokemonMansion_1F_Text_8427A4B, Kanto_PokemonMansion_1F_Text_8427A78
	msgbox Kanto_PokemonMansion_1F_Text_8427A7E, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonMansion_1F object 5 (local id 5, gfx 0x3F5C) at 8,8
Kanto_PokemonMansion_1F_EventScript_8411F7F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8726, 0, Kanto_PokemonMansion_1F_Text_8427AAD, Kanto_PokemonMansion_1F_Text_8427AE4
	msgbox Kanto_PokemonMansion_1F_Text_8427AFF, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonMansion_1F object 2 (local id 2, gfx 0x3F0E) at 29,32
Kanto_PokemonMansion_1F_EventScript_846BE33::
	setorcopyvar VAR_0x8000, 0x46
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonMansion_1F object 3 (local id 3, gfx 0x3F0E) at 22,6
Kanto_PokemonMansion_1F_EventScript_846BE40::
	setorcopyvar VAR_0x8000, 0x78
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonMansion_1F object 4 (local id 4, gfx 0x3F0E) at 27,15
Kanto_PokemonMansion_1F_EventScript_846C04B::
	setorcopyvar VAR_0x8000, 0x42
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PokemonMansion_1F_Text_8427A4B::
	.string "Who are you?\n"
	.string "There shouldn’t be anyone here.$"

Kanto_PokemonMansion_1F_Text_8427A78::
	.string "Ouch!$"

Kanto_PokemonMansion_1F_Text_8427A7E::
	.string "A key? I don’t know what you’re\n"
	.string "talking about.$"

Kanto_PokemonMansion_1F_Text_8427AAD::
	.string "W-w-waah! You startled me!\n"
	.string "I thought you were a ghost.$"

Kanto_PokemonMansion_1F_Text_8427AE4::
	.string "Tch!\n"
	.string "I can’t get any wins.$"

Kanto_PokemonMansion_1F_Text_8427AFF::
	.string "I was exploring here by myself,\n"
	.string "but I feel spooked.\p"
	.string "I ought to go soon.$"
