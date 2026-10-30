Kanto_PokemonTower_2F_MapScripts::
	.byte 0

@ Kanto_PokemonTower_2F object 2 (local id 2, gfx 0x3EF6) at 3,7
Kanto_PokemonTower_2F_EventScript_8412F77::
	msgbox Kanto_PokemonTower_2F_Text_842A575, MSGBOX_NPC
	end

@ Kanto_PokemonTower_2F trigger at 17,5 (VAR_QZ_0x5121 == 0)
Kanto_PokemonTower_2F_EventScript_8412F80::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_PokemonTower_2F_EventScript_8412F98

@ Kanto_PokemonTower_2F trigger at 16,6 (VAR_QZ_0x5121 == 0)
Kanto_PokemonTower_2F_EventScript_8412F8C::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_PokemonTower_2F_EventScript_8412F98

Kanto_PokemonTower_2F_EventScript_8412F98::
	textcolor 0
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	call_if_eq VAR_TEMP_1, 0, Kanto_PokemonTower_2F_EventScript_841300E
	call_if_eq VAR_TEMP_1, 1, Kanto_PokemonTower_2F_EventScript_8413020
	applymovement 1, Quetzal_Movement_8457A9E
	waitmovement 0
	applymovement 1, Quetzal_Movement_8457AA0
	waitmovement 0
	msgbox Kanto_PokemonTower_2F_Text_842A3C2, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 1
	call Kanto_PokemonTower_2F_EventScript_8413032
	msgbox Kanto_PokemonTower_2F_Text_842A49B, MSGBOX_DEFAULT
	closemessage
	playbgm MUS_RG_RIVAL_EXIT, 0
	call_if_eq VAR_TEMP_1, 0, Kanto_PokemonTower_2F_EventScript_8413053
	call_if_eq VAR_TEMP_1, 1, Kanto_PokemonTower_2F_EventScript_841305E
	playse SE_EXIT
	delay 0x19
	fadedefaultbgm
	removeobject 1
	setvar VAR_QZ_0x5121, 1
	releaseall
	end

Kanto_PokemonTower_2F_EventScript_841300E::
	applymovement 1, Quetzal_Movement_8457AAE
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_PokemonTower_2F_EventScript_8413020::
	applymovement 1, Quetzal_Movement_8457AB0
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAC
	waitmovement 0
	return

Kanto_PokemonTower_2F_EventScript_8413032::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8621, 0, Kanto_PokemonTower_2F_Text_842A438
	return

Kanto_PokemonTower_2F_EventScript_8413053::
	applymovement 1, Kanto_PokemonTower_2F_Movement_8413069
	waitmovement 0
	return

Kanto_PokemonTower_2F_EventScript_841305E::
	applymovement 1, Kanto_PokemonTower_2F_Movement_8413072
	waitmovement 0
	return

Kanto_PokemonTower_2F_Movement_8413069:
	walk_down
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_right
	step_end

Kanto_PokemonTower_2F_Movement_8413072:
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_right
	step_end

Kanto_PokemonTower_2F_Text_842A3C2::
	.string "BLUE: Hey, {PLAYER}!\n"
	.string "What brings you here?\l"
	.string "Is your POKéMON dead?\p"
	.string "Hey! It’s alive!\p"
	.string "I can at least make them faint!\n"
	.string "Let’s go!$"

Kanto_PokemonTower_2F_Text_842A438::
	.string "What?\n"
	.string "You stinker!\p"
	.string "I took it easy on you, too!$"

Kanto_PokemonTower_2F_Text_842A49B::
	.string "How’s your POKéDEX coming?\n"
	.string "I just caught a CUBONE!\p"
	.string "I can’t find the bigger MAROWAK.\n"
	.string "Where could they be?\p"
	.string "I bet there aren’t any left!\p"
	.string "Well, I better get going.\n"
	.string "I’ve got a lot to accomplish,\l"
	.string "unlike you.\p"
	.string "Smell ya later!$"

Kanto_PokemonTower_2F_Text_842A575::
	.string "Even we could not identify the\n"
	.string "wayward ghosts.\p"
	.string "A SILPH SCOPE might be able to\n"
	.string "unmask them.$"
