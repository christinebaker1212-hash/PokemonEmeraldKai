Kanto_SeafoamIslands_B4F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SeafoamIslands_B4F_EventScript_8412D2D
	map_script MAP_SCRIPT_ON_RESUME, Kanto_SeafoamIslands_B4F_EventScript_8412D0F
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SeafoamIslands_B4F_EventScript_8412D78
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, Kanto_SeafoamIslands_B4F_OnWarp_8412DAE
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_SeafoamIslands_B4F_OnFrame_8412DC0
	.byte 0

Kanto_SeafoamIslands_B4F_OnWarp_8412DAE:
	map_script_2 VAR_QZ_0x512B, 1, Kanto_SeafoamIslands_B4F_EventScript_8412DB8
	.2byte 0

Kanto_SeafoamIslands_B4F_OnFrame_8412DC0:
	map_script_2 VAR_QZ_0x512B, 1, Kanto_SeafoamIslands_B4F_EventScript_8412DD2
	map_script_2 VAR_TEMP_1, 1, Kanto_SeafoamIslands_B4F_EventScript_8412DE8
	.2byte 0

Kanto_SeafoamIslands_B4F_EventScript_8412D0F::
	call_if_set FLAG_QZ_0x2215, Kanto_SeafoamIslands_B4F_EventScript_8412D19
	end

Kanto_SeafoamIslands_B4F_EventScript_8412D19::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457C6C
	removeobject VAR_LAST_TALKED
	return

Kanto_SeafoamIslands_B4F_EventScript_8412D2D::
	call_if_unset FLAG_QZ_0x2133, Kanto_SeafoamIslands_B4F_EventScript_8412D74
	call_if_unset FLAG_QZ_0x2209, Kanto_SeafoamIslands_B4F_EventScript_8412D49
	call_if_set FLAG_QZ_0x2209, Kanto_SeafoamIslands_B4F_EventScript_8412D70
	end

Kanto_SeafoamIslands_B4F_EventScript_8412D49::
	setvar VAR_TEMP_2, 0
	call_if_unset FLAG_QZ_0x21C7, Kanto_SeafoamIslands_B4F_EventScript_8412E30
	call_if_unset FLAG_QZ_0x21C8, Kanto_SeafoamIslands_B4F_EventScript_8412E30
	call_if_eq VAR_TEMP_2, 2, Kanto_SeafoamIslands_B4F_EventScript_8412D6C
	return

Kanto_SeafoamIslands_B4F_EventScript_8412D6C::
	setflag FLAG_QZ_0x2209
	return

Kanto_SeafoamIslands_B4F_EventScript_8412D70::
	setmaplayoutindex 0x384
	return

Kanto_SeafoamIslands_B4F_EventScript_8412D74::
	clearflag FLAG_QZ_0x2178
	return

Kanto_SeafoamIslands_B4F_EventScript_8412D78::
	setvar VAR_TEMP_2, 0
	call_if_unset FLAG_QZ_0x21C7, Kanto_SeafoamIslands_B4F_EventScript_8412E30
	call_if_unset FLAG_QZ_0x21C8, Kanto_SeafoamIslands_B4F_EventScript_8412E30
	goto_if_eq VAR_TEMP_2, 2, Kanto_SeafoamIslands_B4F_EventScript_8412D9B
	end

Kanto_SeafoamIslands_B4F_EventScript_8412D9B::
	setmetatile 0xC, 0xE, 0x12B, 0
	setmetatile 0xD, 0xE, 0x12B, 0
	end

Kanto_SeafoamIslands_B4F_EventScript_8412DB8::
	turnobject LOCALID_PLAYER, 2
	special QzForcePlayerToStartSurfing
	end

Kanto_SeafoamIslands_B4F_EventScript_8412DD2::
	lockall
	applymovement LOCALID_PLAYER, Kanto_SeafoamIslands_B4F_Movement_8412DE4
	waitmovement 0
	setvar VAR_QZ_0x512B, 0
	releaseall
	end

Kanto_SeafoamIslands_B4F_EventScript_8412DE8::
	lockall
	setvar VAR_TEMP_2, 0
	call_if_unset FLAG_QZ_0x21C7, Kanto_SeafoamIslands_B4F_EventScript_8412E30
	call_if_unset FLAG_QZ_0x21C8, Kanto_SeafoamIslands_B4F_EventScript_8412E30
	goto_if_eq VAR_TEMP_2, 2, Kanto_SeafoamIslands_B4F_EventScript_8412E4C
	getplayerxy VAR_0x8008, VAR_0x8009
	call_if_lt VAR_0x8008, 9, Kanto_SeafoamIslands_B4F_EventScript_8412E36
	call_if_ge VAR_0x8008, 9, Kanto_SeafoamIslands_B4F_EventScript_8412E41
	special QzSeafoamCurrentEffect
	setvar VAR_TEMP_1, 0
	releaseall
	end

Kanto_SeafoamIslands_B4F_EventScript_8412E30::
	addvar VAR_TEMP_2, 1
	return

Kanto_SeafoamIslands_B4F_EventScript_8412E36::
	applymovement LOCALID_PLAYER, Kanto_SeafoamIslands_B4F_Movement_8412E53
	waitmovement 0
	return

Kanto_SeafoamIslands_B4F_EventScript_8412E41::
	applymovement LOCALID_PLAYER, Kanto_SeafoamIslands_B4F_Movement_8412E5C
	waitmovement 0
	return

Kanto_SeafoamIslands_B4F_EventScript_8412E4C::
	setvar VAR_TEMP_1, 0
	releaseall
	end

@ Kanto_SeafoamIslands_B4F trigger at 26,19 (VAR_QZ_0x512B == 0)
@ Kanto_SeafoamIslands_B4F trigger at 27,19 (VAR_QZ_0x512B == 0)
@ Kanto_SeafoamIslands_B4F trigger at 28,19 (VAR_QZ_0x512B == 0)
Kanto_SeafoamIslands_B4F_EventScript_8412E64::
	lockall
	applymovement LOCALID_PLAYER, Kanto_SeafoamIslands_B4F_Movement_8412E71
	waitmovement 0
	releaseall
	end

@ Kanto_SeafoamIslands_B4F object 3 (local id 3, gfx 0x3EE5) at 9,2
Kanto_SeafoamIslands_B4F_EventScript_8412E73::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	lock
	faceplayer
	setvar VAR_0x8004, SPECIES_ARTICUNO
	setvar VAR_0x8005, 0x32
	setvar VAR_0x8006, 0
	special QzKantoStaticEncounterLookupBySpecies
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457FA3
	special CreateEnemyEventMon
	waitse
	playmoncry SPECIES_ARTICUNO, 2
	message Kanto_SeafoamIslands_B4F_Text_84568B9
	waitmessage
	waitmoncry
	delay 0xA
	playbgm MUS_RG_ENCOUNTER_GYM_LEADER, 0
	waitbuttonpress
	setflag FLAG_QZ_0x2215
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0x2215
	special QzKantoStaticEncounterAfterBattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_SeafoamIslands_B4F_EventScript_8412F00
	goto_if_eq VAR_RESULT, 4, Kanto_SeafoamIslands_B4F_EventScript_8412F09
	goto_if_eq VAR_RESULT, 5, Kanto_SeafoamIslands_B4F_EventScript_8412F09
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457FA3
	setflag FLAG_QZ_0x2133
	release
	end

Kanto_SeafoamIslands_B4F_EventScript_8412F00::
	setflag FLAG_QZ_0x2133
	goto Quetzal_EventScript_8459513

Kanto_SeafoamIslands_B4F_EventScript_8412F09::
	setvar VAR_0x8004, 0x90
	goto Quetzal_EventScript_845951C

@ Kanto_SeafoamIslands_B4F sign / bg event at 14,18
Kanto_SeafoamIslands_B4F_EventScript_8412F14::
	msgbox Kanto_SeafoamIslands_B4F_Text_842A1F9, MSGBOX_SIGN
	end

@ Kanto_SeafoamIslands_B4F sign / bg event at 30,2
Kanto_SeafoamIslands_B4F_EventScript_8412F1D::
	msgbox Kanto_SeafoamIslands_B4F_Text_842A228, MSGBOX_SIGN
	end

@ Kanto_SeafoamIslands_B4F object 4 (local id 4, gfx 0x3F0E) at 22,19
Kanto_SeafoamIslands_B4F_EventScript_846C101::
	setorcopyvar VAR_0x8000, ITEM_ULTRA_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SeafoamIslands_B4F_Movement_8412DE4:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Kanto_SeafoamIslands_B4F_Movement_8412E53:
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	step_end

Kanto_SeafoamIslands_B4F_Movement_8412E5C:
	walk_up
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_up
	step_end

Kanto_SeafoamIslands_B4F_Movement_8412E71:
	walk_up
	step_end

Kanto_SeafoamIslands_B4F_Text_842A1F9::
	.string "Hint: Boulders might change the\n"
	.string "flow of water.$"

Kanto_SeafoamIslands_B4F_Text_842A228::
	.string "DANGER\n"
	.string "Fast current!$"

Kanto_SeafoamIslands_B4F_Text_84568B9::
	.string "Articuno!$"
