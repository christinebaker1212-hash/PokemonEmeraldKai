Kanto_PokemonTower_5F_MapScripts::
	.byte 0

@ Kanto_PokemonTower_5F object 5 (local id 5, gfx 0x3EF6) at 12,8
Kanto_PokemonTower_5F_EventScript_841310D::
	msgbox Kanto_PokemonTower_5F_Text_842A7AA, MSGBOX_NPC
	end

@ Kanto_PokemonTower_5F trigger at 10,8 (VAR_TEMP_1 == 0)
@ Kanto_PokemonTower_5F trigger at 11,8 (VAR_TEMP_1 == 0)
@ Kanto_PokemonTower_5F trigger at 12,8 (VAR_TEMP_1 == 0)
@ Kanto_PokemonTower_5F trigger at 10,9 (VAR_TEMP_1 == 0)
@ Kanto_PokemonTower_5F trigger at 12,9 (VAR_TEMP_1 == 0)
@ Kanto_PokemonTower_5F trigger at 10,10 (VAR_TEMP_1 == 0)
@ Kanto_PokemonTower_5F trigger at 11,10 (VAR_TEMP_1 == 0)
@ Kanto_PokemonTower_5F trigger at 12,10 (VAR_TEMP_1 == 0)
Kanto_PokemonTower_5F_EventScript_8413116::
	lockall
	callnative 0x80DF0BD
	fadescreen 3
	callnative 0x80DF0D9
	special QzHealPlayerPartyEx_0
	fadescreen 2
	msgbox Kanto_PokemonTower_5F_Text_842A8C1, MSGBOX_DEFAULT
	setvar VAR_TEMP_1, 1
	releaseall
	end

@ Kanto_PokemonTower_5F trigger at 9,8 (VAR_TEMP_1 == 1)
@ Kanto_PokemonTower_5F trigger at 9,9 (VAR_TEMP_1 == 1)
@ Kanto_PokemonTower_5F trigger at 9,10 (VAR_TEMP_1 == 1)
@ Kanto_PokemonTower_5F trigger at 10,11 (VAR_TEMP_1 == 1)
@ Kanto_PokemonTower_5F trigger at 11,11 (VAR_TEMP_1 == 1)
@ Kanto_PokemonTower_5F trigger at 12,11 (VAR_TEMP_1 == 1)
@ Kanto_PokemonTower_5F trigger at 13,8 (VAR_TEMP_1 == 1)
@ Kanto_PokemonTower_5F trigger at 13,9 (VAR_TEMP_1 == 1)
@ Kanto_PokemonTower_5F trigger at 13,10 (VAR_TEMP_1 == 1)
Kanto_PokemonTower_5F_EventScript_8413137::
	lockall
	setvar VAR_TEMP_1, 0
	releaseall
	end

@ Kanto_PokemonTower_5F object 2 (local id 2, gfx 0x3EF6) at 19,7
Kanto_PokemonTower_5F_EventScript_841313F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8639, 0, Kanto_PokemonTower_5F_Text_842A7E6, Kanto_PokemonTower_5F_Text_842A7F9
	msgbox Kanto_PokemonTower_5F_Text_842A7FF, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_5F object 1 (local id 1, gfx 0x3EF6) at 11,4
Kanto_PokemonTower_5F_EventScript_8413156::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8640, 0, Kanto_PokemonTower_5F_Text_842A817, Kanto_PokemonTower_5F_Text_842A82B
	msgbox Kanto_PokemonTower_5F_Text_842A83D, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_5F object 3 (local id 3, gfx 0x3EF6) at 7,12
Kanto_PokemonTower_5F_EventScript_841316D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8641, 0, Kanto_PokemonTower_5F_Text_842A84E, Kanto_PokemonTower_5F_Text_842A857
	msgbox Kanto_PokemonTower_5F_Text_842A85B, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_5F object 4 (local id 4, gfx 0x3EF6) at 11,16
Kanto_PokemonTower_5F_EventScript_8413184::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8642, 0, Kanto_PokemonTower_5F_Text_842A871, Kanto_PokemonTower_5F_Text_842A87E
	msgbox Kanto_PokemonTower_5F_Text_842A884, MSGBOX_AUTOCLOSE
	end

@ Kanto_PokemonTower_5F object 6 (local id 6, gfx 0x3F0E) at 6,16
Kanto_PokemonTower_5F_EventScript_846BC3F::
	setorcopyvar VAR_0x8000, 0x87
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PokemonTower_5F object 7 (local id 7, gfx 0x3F0E) at 11,9
Kanto_PokemonTower_5F_EventScript_846C099::
	setorcopyvar VAR_0x8000, 0x1D3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PokemonTower_5F_Text_842A7AA::
	.string "Come, child!\n"
	.string "I have purified this space.\l"
	.string "You can rest here.$"

Kanto_PokemonTower_5F_Text_842A7E6::
	.string "Give…me…\n"
	.string "your…all…$"

Kanto_PokemonTower_5F_Text_842A7F9::
	.string "Gasp!$"

Kanto_PokemonTower_5F_Text_842A7FF::
	.string "I was under possession.$"

Kanto_PokemonTower_5F_Text_842A817::
	.string "You…shall…\n"
	.string "join…us…$"

Kanto_PokemonTower_5F_Text_842A82B::
	.string "What a nightmare!$"

Kanto_PokemonTower_5F_Text_842A83D::
	.string "I was possessed.$"

Kanto_PokemonTower_5F_Text_842A84E::
	.string "Zombies!$"

Kanto_PokemonTower_5F_Text_842A857::
	.string "Ha?$"

Kanto_PokemonTower_5F_Text_842A85B::
	.string "I regained my senses.$"

Kanto_PokemonTower_5F_Text_842A871::
	.string "Urgah…\n"
	.string "Urff…$"

Kanto_PokemonTower_5F_Text_842A87E::
	.string "Whoo!$"

Kanto_PokemonTower_5F_Text_842A884::
	.string "I fell to evil spirits despite my\n"
	.string "training in the mountains…$"

Kanto_PokemonTower_5F_Text_842A8C1::
	.string "Entered the purified and protected\n"
	.string "zone.\p"
	.string "{PLAYER}’s POKéMON were fully\n"
	.string "healed.$"
