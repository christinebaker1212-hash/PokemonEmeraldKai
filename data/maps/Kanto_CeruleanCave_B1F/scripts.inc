Kanto_CeruleanCave_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_CeruleanCave_B1F_EventScript_8412227
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CeruleanCave_B1F_EventScript_8412245
	.byte 0

Kanto_CeruleanCave_B1F_EventScript_8412227::
	call_if_set FLAG_QZ_0x2215, Kanto_CeruleanCave_B1F_EventScript_8412231
	end

Kanto_CeruleanCave_B1F_EventScript_8412231::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457C6C
	removeobject VAR_LAST_TALKED
	return

Kanto_CeruleanCave_B1F_EventScript_8412245::
	call_if_unset FLAG_QZ_0x2137, Kanto_CeruleanCave_B1F_EventScript_841224F
	end

Kanto_CeruleanCave_B1F_EventScript_841224F::
	clearflag FLAG_QZ_0x219D
	return

@ Kanto_CeruleanCave_B1F object 3 (local id 3, gfx 0x3F21) at 7,12
Kanto_CeruleanCave_B1F_EventScript_8412253::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	lock
	faceplayer
	waitse
	playmoncry SPECIES_MEWTWO, 2
	message Kanto_CeruleanCave_B1F_Text_8428656
	waitmessage
	waitmoncry
	delay 0x14
	playbgm MUS_RG_ENCOUNTER_GYM_LEADER, 0
	waitbuttonpress
	setvar VAR_0x8004, SPECIES_MEWTWO
	setvar VAR_0x8005, 0x46
	setvar VAR_0x8006, 0
	special QzKantoStaticEncounterLookupBySpecies
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457FA3
	special CreateEnemyEventMon
	setflag FLAG_QZ_0x2215
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0x2215
	special QzKantoStaticEncounterAfterBattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_CeruleanCave_B1F_EventScript_84122E0
	goto_if_eq VAR_RESULT, 4, Kanto_CeruleanCave_B1F_EventScript_84122E9
	goto_if_eq VAR_RESULT, 5, Kanto_CeruleanCave_B1F_EventScript_84122E9
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457FA3
	setflag FLAG_QZ_0x2137
	release
	end

Kanto_CeruleanCave_B1F_EventScript_84122E0::
	setflag FLAG_QZ_0x2137
	goto Quetzal_EventScript_8459513

Kanto_CeruleanCave_B1F_EventScript_84122E9::
	setvar VAR_0x8004, 0x96
	goto Quetzal_EventScript_845951C

@ Kanto_CeruleanCave_B1F object 2 (local id 2, gfx 0x3F0E) at 32,2
Kanto_CeruleanCave_B1F_EventScript_846BF6E::
	setorcopyvar VAR_0x8000, ITEM_MAX_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_CeruleanCave_B1F object 1 (local id 1, gfx 0x3F0E) at 31,9
Kanto_CeruleanCave_B1F_EventScript_846BF7B::
	setorcopyvar VAR_0x8000, ITEM_ULTRA_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_CeruleanCave_B1F_Text_8428656::
	.string "Mew!$"
