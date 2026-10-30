Kanto_PokemonTower_6F_MapScripts::
	.byte 0

@ Kanto_PokemonTower_6F trigger at 11,15 (VAR_QZ_0x5122 == 0)
@ Kanto_PokemonTower_6F trigger at 12,16 (VAR_QZ_0x5122 == 0)
Kanto_PokemonTower_6F_EventScript_841319C::
	lockall
	textcolor 2
	msgbox Kanto_PokemonTower_6F_Text_842A9BF, MSGBOX_DEFAULT
	checkitem ITEM_SILPH_SCOPE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_PokemonTower_6F_EventScript_84131EE
	setwildbattle SPECIES_MAROWAK, 0x1E, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	special QzStartMarowakGhostBattle
	waitstate
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_PokemonTower_6F_EventScript_84131FD
	goto_if_eq VAR_RESULT, 7, Kanto_PokemonTower_6F_EventScript_84131FD

Kanto_PokemonTower_6F_EventScript_84131E1::
	closemessage
	applymovement LOCALID_PLAYER, Kanto_PokemonTower_6F_Movement_841321A
	waitmovement 0
	releaseall
	end

Kanto_PokemonTower_6F_EventScript_84131EE::
	textcolor 3
	msgbox Kanto_PokemonTower_6F_Text_842A9D3, MSGBOX_DEFAULT
	goto Kanto_PokemonTower_6F_EventScript_84131E1

Kanto_PokemonTower_6F_EventScript_84131FD::
	message Kanto_PokemonTower_6F_Text_842AA15
	waitmessage
	waitse
	playmoncry SPECIES_MAROWAK, 0
	waitbuttonpress
	waitmoncry
	msgbox Kanto_PokemonTower_6F_Text_842AA4B, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5122, 1
	releaseall
	end

@ Kanto_PokemonTower_6F object 1 (local id 1, gfx 0x3EF6) at 13,10
Kanto_PokemonTower_6F_EventScript_841321C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8643, 0, Kanto_PokemonTower_6F_Text_842A90A, Kanto_PokemonTower_6F_Text_842A91A
	msgbox Kanto_PokemonTower_6F_Text_842A921, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_6F object 3 (local id 3, gfx 0x3EF6) at 9,6
Kanto_PokemonTower_6F_EventScript_8413233::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8644, 0, Kanto_PokemonTower_6F_Text_842A939, Kanto_PokemonTower_6F_Text_842A946
	msgbox Kanto_PokemonTower_6F_Text_842A95A, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_6F object 2 (local id 2, gfx 0x3EF6) at 18,6
Kanto_PokemonTower_6F_EventScript_841324A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8645, 0, Kanto_PokemonTower_6F_Text_842A98F, Kanto_PokemonTower_6F_Text_842A9A3
	msgbox Kanto_PokemonTower_6F_Text_842A9A9, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_6F object 4 (local id 4, gfx 0x3F0E) at 5,15
Kanto_PokemonTower_6F_EventScript_846BC4C::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonTower_6F object 5 (local id 5, gfx 0x3F0E) at 15,15
Kanto_PokemonTower_6F_EventScript_846BC59::
	setorcopyvar VAR_0x8000, ITEM_X_ACCURACY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PokemonTower_6F_Movement_841321A:
	walk_up
	step_end

Kanto_PokemonTower_6F_Text_842A90A::
	.string "Give…me…\n"
	.string "blood…$"

Kanto_PokemonTower_6F_Text_842A91A::
	.string "Groan!$"

Kanto_PokemonTower_6F_Text_842A921::
	.string "I feel anemic and weak…$"

Kanto_PokemonTower_6F_Text_842A939::
	.string "Urff…\n"
	.string "Kwaah!$"

Kanto_PokemonTower_6F_Text_842A946::
	.string "Something fell out!$"

Kanto_PokemonTower_6F_Text_842A95A::
	.string "My hair didn’t fall out!\n"
	.string "It was a malevolent spirit!$"

Kanto_PokemonTower_6F_Text_842A98F::
	.string "Ke…ke…ke…\n"
	.string "ke…ke…ke!$"

Kanto_PokemonTower_6F_Text_842A9A3::
	.string "Keee!$"

Kanto_PokemonTower_6F_Text_842A9A9::
	.string "What’s going on here?$"

Kanto_PokemonTower_6F_Text_842A9BF::
	.string "Be gone…\n"
	.string "Intruders…$"

Kanto_PokemonTower_6F_Text_842A9D3::
	.string "A spirit blocks the way.\p"
	.string "You need the Silph Scope\n"
	.string "to identify it.$"

Kanto_PokemonTower_6F_Text_842AA15::
	.string "The ghost was the restless spirit\n"
	.string "of CUBONE’s mother!$"

Kanto_PokemonTower_6F_Text_842AA4B::
	.string "The mother’s spirit was calmed.\p"
	.string "It departed to the afterlife…$"
