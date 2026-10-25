Kanto_PokemonTower_7F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_PokemonTower_7F object 1 (local id 1, gfx 0x3F26) at 11,4
Kanto_PokemonTower_7F_EventScript_8413267::
	lock
	faceplayer
	setvar VAR_0x8004, 0xE
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	setflag FLAG_QZ_0x21D0
	clearflag FLAG_QZ_0x21A8
	setflag FLAG_QZ_0x21ED
	msgbox Kanto_PokemonTower_7F_Text_842AC32, MSGBOX_DEFAULT
	closemessage
	warp MAP_KANTO_LAVENDER_TOWN_VOLUNTEER_POKEMON_HOUSE, 255, 4, 7
	waitstate
	release
	end

@ Kanto_PokemonTower_7F object 2 (local id 2, gfx 0x3F3D) at 9,10
Kanto_PokemonTower_7F_EventScript_8413293::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8561, 0, Kanto_PokemonTower_7F_Text_842AA89, Kanto_PokemonTower_7F_Text_842AAAD, Kanto_PokemonTower_7F_EventScript_84132AE
	msgbox Kanto_PokemonTower_7F_Text_842AAB8, MSGBOX_AUTOCLOSE
	end

Kanto_PokemonTower_7F_EventScript_84132AE::
	msgbox Kanto_PokemonTower_7F_Text_842AAB8, MSGBOX_DEFAULT
	closemessage
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_eq VAR_0x8004, 10, Kanto_PokemonTower_7F_EventScript_84132ED
	goto_if_eq VAR_0x8004, 11, Kanto_PokemonTower_7F_EventScript_84132FD
	goto_if_eq VAR_0x8004, 9, Kanto_PokemonTower_7F_EventScript_841330D
	applymovement 2, Kanto_PokemonTower_7F_Movement_8413339
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_8413323

Kanto_PokemonTower_7F_EventScript_84132ED::
	applymovement 2, Kanto_PokemonTower_7F_Movement_8413328
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_8413323

Kanto_PokemonTower_7F_EventScript_84132FD::
	applymovement 2, Kanto_PokemonTower_7F_Movement_8413331
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_8413323

Kanto_PokemonTower_7F_EventScript_841330D::
	applymovement 2, Kanto_PokemonTower_7F_Movement_8413340
	waitmovement 0
	playse SE_EXIT
	delay 0x19
	goto Kanto_PokemonTower_7F_EventScript_8413323

Kanto_PokemonTower_7F_EventScript_8413323::
	removeobject 2
	release
	end

@ Kanto_PokemonTower_7F object 3 (local id 3, gfx 0x3F3D) at 13,8
Kanto_PokemonTower_7F_EventScript_841334A::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8562, 0, Kanto_PokemonTower_7F_Text_842AAD6, Kanto_PokemonTower_7F_Text_842AB6F, Kanto_PokemonTower_7F_EventScript_8413365
	msgbox Kanto_PokemonTower_7F_Text_842AB80, MSGBOX_AUTOCLOSE
	end

Kanto_PokemonTower_7F_EventScript_8413365::
	setvar VAR_0x8004, 0xE
	setvar VAR_0x8005, 1
	special QzQuestLogNop_737
	msgbox Kanto_PokemonTower_7F_Text_842AB80, MSGBOX_DEFAULT
	closemessage
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_eq VAR_0x8004, 12, Kanto_PokemonTower_7F_EventScript_84133A6
	goto_if_eq VAR_0x8004, 13, Kanto_PokemonTower_7F_EventScript_84133B6
	applymovement 3, Kanto_PokemonTower_7F_Movement_84133D3
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_84133C6

Kanto_PokemonTower_7F_EventScript_84133A6::
	applymovement 3, Kanto_PokemonTower_7F_Movement_84133CB
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_84133C6

Kanto_PokemonTower_7F_EventScript_84133B6::
	applymovement 3, Kanto_PokemonTower_7F_Movement_84133DA
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_84133C6

Kanto_PokemonTower_7F_EventScript_84133C6::
	removeobject 3
	release
	end

@ Kanto_PokemonTower_7F object 4 (local id 4, gfx 0x3F3D) at 9,6
Kanto_PokemonTower_7F_EventScript_84133E3::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8563, 0, Kanto_PokemonTower_7F_Text_842ABD8, Kanto_PokemonTower_7F_Text_842ABF7, Kanto_PokemonTower_7F_EventScript_84133FE
	msgbox Kanto_PokemonTower_7F_Text_842AC0F, MSGBOX_AUTOCLOSE
	end

Kanto_PokemonTower_7F_EventScript_84133FE::
	msgbox Kanto_PokemonTower_7F_Text_842AC0F, MSGBOX_DEFAULT
	closemessage
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_eq VAR_0x8004, 10, Kanto_PokemonTower_7F_EventScript_8413432
	goto_if_eq VAR_0x8004, 9, Kanto_PokemonTower_7F_EventScript_8413442
	applymovement 4, Kanto_PokemonTower_7F_Movement_8413461
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_8413452

Kanto_PokemonTower_7F_EventScript_8413432::
	applymovement 4, Kanto_PokemonTower_7F_Movement_8413459
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_8413452

Kanto_PokemonTower_7F_EventScript_8413442::
	applymovement 4, Kanto_PokemonTower_7F_Movement_8413468
	waitmovement 0
	goto Kanto_PokemonTower_7F_EventScript_8413452

Kanto_PokemonTower_7F_EventScript_8413452::
	removeobject 4
	release
	end

Kanto_PokemonTower_7F_Movement_8413328:
	walk_down
	walk_right
	walk_down
	walk_down
	walk_down
	walk_right
	walk_down
	walk_down
	step_end

Kanto_PokemonTower_7F_Movement_8413331:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_down
	walk_down
	step_end

Kanto_PokemonTower_7F_Movement_8413339:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_PokemonTower_7F_Movement_8413340:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_down
	walk_down
	walk_in_place_faster_left
	step_end

Kanto_PokemonTower_7F_Movement_84133CB:
	walk_down
	walk_down
	walk_down
	walk_left
	walk_down
	walk_down
	walk_down
	step_end

Kanto_PokemonTower_7F_Movement_84133D3:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_PokemonTower_7F_Movement_84133DA:
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_PokemonTower_7F_Movement_8413459:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_down
	step_end

Kanto_PokemonTower_7F_Movement_8413461:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_PokemonTower_7F_Movement_8413468:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_PokemonTower_7F_Text_842AA89::
	.string "What do you want?\n"
	.string "Why are you here?$"

Kanto_PokemonTower_7F_Text_842AAAD::
	.string "I give up!$"

Kanto_PokemonTower_7F_Text_842AAB8::
	.string "I’m not going to forget this!$"

Kanto_PokemonTower_7F_Text_842AAD6::
	.string "This old guy marched right up to\n"
	.string "our HIDEOUT.\p"
	.string "Then, he starts ranting about how\n"
	.string "TEAM ROCKET’s abusing POKéMON.\p"
	.string "So, we’re just talking it over as\n"
	.string "adults.$"

Kanto_PokemonTower_7F_Text_842AB6F::
	.string "Please!\n"
	.string "No more!$"

Kanto_PokemonTower_7F_Text_842AB80::
	.string "POKéMON are only good for making\n"
	.string "money. Why not use them?\p"
	.string "You stay out of our business!$"

Kanto_PokemonTower_7F_Text_842ABD8::
	.string "You’re not saving anyone, kid!$"

Kanto_PokemonTower_7F_Text_842ABF7::
	.string "Don’t fight us ROCKETS!$"

Kanto_PokemonTower_7F_Text_842AC0F::
	.string "You’re not getting away with this!$"

Kanto_PokemonTower_7F_Text_842AC32::
	.string "MR. FUJI: Heh?\n"
	.string "You came to save me?\p"
	.string "Thank you. But, I came here of my\n"
	.string "own free will.\p"
	.string "I came to calm the spirit of\n"
	.string "CUBONE’s mother.\p"
	.string "I think MAROWAK’s spirit has\n"
	.string "finally left us.\p"
	.string "I must thank you for your kind\n"
	.string "concern.\p"
	.string "Follow me to my home, POKéMON\n"
	.string "HOUSE, at the foot of this tower.$"
