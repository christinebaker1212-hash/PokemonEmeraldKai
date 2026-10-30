Kanto_PokemonLeague_LancesRoom_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_PokemonLeague_LancesRoom_EventScript_84126A8
	map_script MAP_SCRIPT_ON_LOAD, Kanto_PokemonLeague_LancesRoom_EventScript_84126B3
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_PokemonLeague_LancesRoom_OnWarp_84126D4
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_PokemonLeague_LancesRoom_OnFrame_84126E3
	.byte 0

Kanto_PokemonLeague_LancesRoom_OnWarp_84126D4:
	map_script_2 VAR_TEMP_1, 0, Kanto_PokemonLeague_LancesRoom_EventScript_84126DE
	.2byte 0

Kanto_PokemonLeague_LancesRoom_OnFrame_84126E3:
	map_script_2 VAR_QZ_0x5120, 3, Kanto_PokemonLeague_LancesRoom_EventScript_84126ED
	.2byte 0

Kanto_PokemonLeague_LancesRoom_EventScript_84126A8::
	setvar VAR_0x8004, 3
	call Quetzal_EventScript_8457A98
	end

Kanto_PokemonLeague_LancesRoom_EventScript_84126B3::
	call_if_eq VAR_QZ_0x5120, 4, Kanto_PokemonLeague_LancesRoom_EventScript_84126C8
	call_if_set FLAG_QZ_0x211F, Kanto_PokemonLeague_LancesRoom_EventScript_84126CE
	end

Kanto_PokemonLeague_LancesRoom_EventScript_84126C8::
	call Kanto_PokemonLeague_LancesRoom_EventScript_841270E
	return

Kanto_PokemonLeague_LancesRoom_EventScript_84126CE::
	call Kanto_PokemonLeague_LancesRoom_EventScript_8457A4C
	return

Kanto_PokemonLeague_LancesRoom_EventScript_84126DE::
	turnobject LOCALID_PLAYER, 2
	end

Kanto_PokemonLeague_LancesRoom_EventScript_84126ED::
	lockall
	applymovement LOCALID_PLAYER, Kanto_PokemonLeague_LancesRoom_Movement_8412757
	waitmovement 0
	setflag FLAG_TEMP_2
	call Kanto_PokemonLeague_LancesRoom_EventScript_841270E
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	setvar VAR_QZ_0x5120, 4
	releaseall
	end

Kanto_PokemonLeague_LancesRoom_EventScript_841270E::
	setmetatile 5, 0xE, 0x29D, 1
	setmetatile 6, 0xE, 0x29E, 1
	setmetatile 7, 0xE, 0x29F, 1
	setmetatile 5, 0xF, 0x2A5, 1
	setmetatile 6, 0xF, 0x2A6, 1
	setmetatile 7, 0xF, 0x2A7, 1
	setmetatile 6, 0x10, 0x311, 0
	setmetatile 7, 0x10, 0x311, 0
	return

@ Kanto_PokemonLeague_LancesRoom object 1 (local id 1, gfx 0x3F13) at 6,8
Kanto_PokemonLeague_LancesRoom_EventScript_841277A::
	lock
	faceplayer
	setvar VAR_0x8004, 0xC
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	goto_if_set FLAG_QZ_0x211F, Kanto_PokemonLeague_LancesRoom_EventScript_84127EF
	setvar VAR_0x8004, 0xC
	setvar VAR_0x8005, 0
	special QzQuestLogNop
	setvar VAR_0x8004, 0xC
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	call_if_unset FLAG_QZ_0x2108, Kanto_PokemonLeague_LancesRoom_EventScript_84127D2
	call_if_set FLAG_QZ_0x2108, Kanto_PokemonLeague_LancesRoom_EventScript_84127DB
	setflag FLAG_TEMP_3
	setflag FLAG_TEMP_5
	call Kanto_PokemonLeague_LancesRoom_EventScript_84127E4
	clearflag FLAG_TEMP_5
	goto Kanto_PokemonLeague_LancesRoom_EventScript_84127F9

Kanto_PokemonLeague_LancesRoom_EventScript_84127D2::
	msgbox Kanto_PokemonLeague_LancesRoom_Text_8428E1B, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_LancesRoom_EventScript_84127DB::
	msgbox Kanto_PokemonLeague_LancesRoom_Text_8428F74, MSGBOX_DEFAULT
	return

Kanto_PokemonLeague_LancesRoom_EventScript_84127E4::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8605, 0, Kanto_PokemonLeague_LancesRoom_Text_84290CC
	return

Kanto_PokemonLeague_LancesRoom_EventScript_84127EF::
	msgbox Kanto_PokemonLeague_LancesRoom_Text_8429109, MSGBOX_DEFAULT
	release
	end

Kanto_PokemonLeague_LancesRoom_EventScript_84127F9::
	setvar VAR_0x8004, 0
	special Script_TryGainNewFanFromCounter
	setflag FLAG_QZ_0x211F
	call Kanto_PokemonLeague_LancesRoom_EventScript_8457A26
	msgbox Kanto_PokemonLeague_LancesRoom_Text_8429109, MSGBOX_DEFAULT
	closemessage
	release
	end

Kanto_PokemonLeague_LancesRoom_EventScript_8457A26::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AC1
	waitmovement 0
	playse SE_DOOR
	setmetatile 6, 4, 0x28E, 1
	setmetatile 6, 5, 0x296, 0
	special DrawWholeMapView
	setflag FLAG_TEMP_4
	return

Kanto_PokemonLeague_LancesRoom_EventScript_8457A4C::
	setmetatile 6, 4, 0x28E, 1
	setmetatile 6, 5, 0x296, 0
	return

Kanto_PokemonLeague_LancesRoom_Movement_8412757:
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Kanto_PokemonLeague_LancesRoom_Text_8428E1B::
	.string "Ah! I’ve heard about you,\n"
	.string "{PLAYER}!\p"
	.string "I lead the ELITE FOUR.\p"
	.string "You can call me LANCE the dragon\n"
	.string "TRAINER.\p"
	.string "You know that dragons are\n"
	.string "mythical POKéMON.\p"
	.string "They’re hard to catch and raise,\n"
	.string "but their powers are superior.\p"
	.string "They’re virtually indestructible.\n"
	.string "There’s no being clever with them.\p"
	.string "Well, are you ready to lose?\p"
	.string "Your LEAGUE challenge ends with\n"
	.string "me, {PLAYER}!{PLAY_BGM 0x022A}$"

Kanto_PokemonLeague_LancesRoom_Text_8428F74::
	.string "Ah!\n"
	.string "So, you’ve returned, {PLAYER}!\p"
	.string "I lead the ELITE FOUR.\p"
	.string "You can call me LANCE the dragon\n"
	.string "TRAINER.\p"
	.string "You know that dragons are\n"
	.string "mythical POKéMON.\p"
	.string "They’re hard to catch and raise,\n"
	.string "but their powers are superior.\p"
	.string "They’re virtually indestructible.\n"
	.string "There’s no being clever with them.\p"
	.string "Well, are you ready to lose?\p"
	.string "Your LEAGUE challenge ends with\n"
	.string "me, {PLAYER}!{PLAY_BGM 0x022A}$"

Kanto_PokemonLeague_LancesRoom_Text_84290CC::
	.string "That’s it!\p"
	.string "I hate to admit it, but you are a\n"
	.string "POKéMON master!$"

Kanto_PokemonLeague_LancesRoom_Text_8429109::
	.string "I still can’t believe my dragons\n"
	.string "lost to you, {PLAYER}.\p"
	.string "You are now the POKéMON LEAGUE\n"
	.string "CHAMPION!\p"
	.string "…Or, you would have been, but\n"
	.string "you have one more challenge left.\p"
	.string "There is one more TRAINER to face!\n"
	.string "His name is…\p"
	.string "BLUE!\p"
	.string "He beat the ELITE FOUR before\n"
	.string "you.\p"
	.string "He is the real POKéMON LEAGUE\n"
	.string "CHAMPION.$"
