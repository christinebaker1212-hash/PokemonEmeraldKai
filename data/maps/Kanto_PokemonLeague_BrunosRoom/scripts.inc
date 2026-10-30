Kanto_PokemonLeague_BrunosRoom_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_PokemonLeague_BrunosRoom_EventScript_8412430
	map_script MAP_SCRIPT_ON_LOAD, Kanto_PokemonLeague_BrunosRoom_EventScript_841243B
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_PokemonLeague_BrunosRoom_OnWarp_841245C
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_PokemonLeague_BrunosRoom_OnFrame_841246B
	.byte 0

Kanto_PokemonLeague_BrunosRoom_OnWarp_841245C:
	map_script_2 VAR_TEMP_1, 0, Kanto_PokemonLeague_BrunosRoom_EventScript_8412466
	.2byte 0

Kanto_PokemonLeague_BrunosRoom_OnFrame_841246B:
	map_script_2 VAR_QZ_0x5120, 1, Kanto_PokemonLeague_BrunosRoom_EventScript_8412475
	.2byte 0

Kanto_PokemonLeague_BrunosRoom_EventScript_8412430::
	setvar VAR_0x8004, 1
	call Quetzal_EventScript_8457A98
	end

Kanto_PokemonLeague_BrunosRoom_EventScript_841243B::
	call_if_set FLAG_QZ_0x211C, Kanto_PokemonLeague_BrunosRoom_EventScript_8412456
	call_if_eq VAR_QZ_0x5120, 2, Kanto_PokemonLeague_BrunosRoom_EventScript_8412450
	end

Kanto_PokemonLeague_BrunosRoom_EventScript_8412450::
	call Quetzal_EventScript_8457A5F
	return

Kanto_PokemonLeague_BrunosRoom_EventScript_8412456::
	call Quetzal_EventScript_84579FB
	return

Kanto_PokemonLeague_BrunosRoom_EventScript_8412466::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_PokemonLeague_BrunosRoom_EventScript_8412475::
	lockall
	call Quetzal_EventScript_84579E2
	setvar VAR_QZ_0x5120, 2
	releaseall
	end

@ Kanto_PokemonLeague_BrunosRoom object 1 (local id 1, gfx 0x3EF0) at 6,5
Kanto_PokemonLeague_BrunosRoom_EventScript_8412482::
	lock
	faceplayer
	setvar VAR_0x8004, 0xA
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	goto_if_set FLAG_QZ_0x211C, Kanto_PokemonLeague_BrunosRoom_EventScript_84124F7
	setvar VAR_0x8004, 0xA
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	setvar VAR_0x8004, 0xA
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	call_if_unset FLAG_QZ_0x2108, Kanto_PokemonLeague_BrunosRoom_EventScript_84124DA
	call_if_set FLAG_QZ_0x2108, Kanto_PokemonLeague_BrunosRoom_EventScript_84124E3
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	call Kanto_PokemonLeague_BrunosRoom_EventScript_84124EC
	clearflag FLAG_TEMP_5
	goto Kanto_PokemonLeague_BrunosRoom_EventScript_841252E

Kanto_PokemonLeague_BrunosRoom_EventScript_84124DA::
	msgbox Kanto_PokemonLeague_BrunosRoom_Text_84288C6, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_BrunosRoom_EventScript_84124E3::
	msgbox Kanto_PokemonLeague_BrunosRoom_Text_84289C3, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_BrunosRoom_EventScript_84124EC::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8603, 0, Kanto_PokemonLeague_BrunosRoom_Text_8428AC0
	return

Kanto_PokemonLeague_BrunosRoom_EventScript_84124F7::
	msgbox Kanto_PokemonLeague_BrunosRoom_Text_8428AD7, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 2, Kanto_PokemonLeague_BrunosRoom_EventScript_8412578
	call_if_eq VAR_FACING, 1, Kanto_PokemonLeague_BrunosRoom_EventScript_841256D
	call_if_eq VAR_FACING, 3, Kanto_PokemonLeague_BrunosRoom_EventScript_8412583
	call_if_eq VAR_FACING, 4, Kanto_PokemonLeague_BrunosRoom_EventScript_8412583
	release
	end

Kanto_PokemonLeague_BrunosRoom_EventScript_841252E::
	setflag FLAG_QZ_0x211C
	call Quetzal_EventScript_84579C9
	msgbox Kanto_PokemonLeague_BrunosRoom_Text_8428AD7, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 2, Kanto_PokemonLeague_BrunosRoom_EventScript_8412578
	call_if_eq VAR_FACING, 1, Kanto_PokemonLeague_BrunosRoom_EventScript_841256D
	call_if_eq VAR_FACING, 3, Kanto_PokemonLeague_BrunosRoom_EventScript_8412583
	call_if_eq VAR_FACING, 4, Kanto_PokemonLeague_BrunosRoom_EventScript_8412583
	release
	end

Kanto_PokemonLeague_BrunosRoom_EventScript_841256D::
	applymovement 1, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_PokemonLeague_BrunosRoom_EventScript_8412578::
	applymovement 1, Quetzal_Movement_8457AAE
	waitmovement 0
	return

Kanto_PokemonLeague_BrunosRoom_EventScript_8412583::
	applymovement 1, Quetzal_Movement_8457AB0
	waitmovement 0
	return

Kanto_PokemonLeague_BrunosRoom_Text_84288C6::
	.string "I am BRUNO of the ELITE FOUR!\p"
	.string "Through rigorous training, people\n"
	.string "and POKéMON can become stronger\l"
	.string "without limit.\p"
	.string "I’ve lived and trained with my\n"
	.string "fighting POKéMON!\l"
	.string "And that will never change!\p"
	.string "{PLAYER}!\p"
	.string "We will grind you down with our\n"
	.string "superior power!\p"
	.string "Hoo hah!{PLAY_BGM 0x022A}$"

Kanto_PokemonLeague_BrunosRoom_Text_84289C3::
	.string "I am BRUNO of the ELITE FOUR!\p"
	.string "Through rigorous training, people\n"
	.string "and POKéMON can become stronger\l"
	.string "without limit.\p"
	.string "I’ve lived and trained with my\n"
	.string "fighting POKéMON!\l"
	.string "And that will never change!\p"
	.string "{PLAYER}!\p"
	.string "We will grind you down with our\n"
	.string "superior power!\p"
	.string "Hoo hah!{PLAY_BGM 0x022A}$"

Kanto_PokemonLeague_BrunosRoom_Text_8428AC0::
	.string "Why?\n"
	.string "How could I lose?$"

Kanto_PokemonLeague_BrunosRoom_Text_8428AD7::
	.string "My job is done.\n"
	.string "Go face your next challenge.$"
