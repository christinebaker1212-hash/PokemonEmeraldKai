Kanto_PowerPlant_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_PowerPlant_EventScript_841347C
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PowerPlant_EventScript_841349A
	.byte 0

Kanto_PowerPlant_EventScript_841347C::
	call_if_set FLAG_QZ_0x2215, Kanto_PowerPlant_EventScript_8413486
	end

Kanto_PowerPlant_EventScript_8413486::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457C6C
	removeobject VAR_LAST_TALKED
	return

Kanto_PowerPlant_EventScript_841349A::
	setflag FLAG_QZ_0x224A
	call_if_unset FLAG_QZ_0x213B, Kanto_PowerPlant_EventScript_84134BE
	call_if_unset FLAG_QZ_0x2139, Kanto_PowerPlant_EventScript_84134C2
	call_if_unset FLAG_QZ_0x213A, Kanto_PowerPlant_EventScript_84134C6
	call Quetzal_EventScript_840FBB5
	end

Kanto_PowerPlant_EventScript_84134BE::
	clearflag FLAG_QZ_0x21DA
	return

Kanto_PowerPlant_EventScript_84134C2::
	clearflag FLAG_QZ_0x21AB
	return

Kanto_PowerPlant_EventScript_84134C6::
	clearflag FLAG_QZ_0x21AC
	return

@ Kanto_PowerPlant object 6 (local id 6, gfx 0x3F5D) at 5,11
Kanto_PowerPlant_EventScript_84134CA::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	lock
	faceplayer
	setvar VAR_0x8004, 0x91
	setvar VAR_0x8005, 0x32
	setvar VAR_0x8006, 0
	special QzKantoStaticEncounterLookupBySpecies
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457FA3
	special CreateEnemyEventMon
	waitse
	playmoncry SPECIES_ZAPDOS, 2
	message Kanto_PowerPlant_Text_84568C3
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
	goto_if_eq VAR_RESULT, 1, Kanto_PowerPlant_EventScript_8413557
	goto_if_eq VAR_RESULT, 4, Kanto_PowerPlant_EventScript_8413560
	goto_if_eq VAR_RESULT, 5, Kanto_PowerPlant_EventScript_8413560
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457FA3
	setflag FLAG_QZ_0x213B
	release
	end

Kanto_PowerPlant_EventScript_8413557::
	setflag FLAG_QZ_0x213B
	goto Quetzal_EventScript_8459513

Kanto_PowerPlant_EventScript_8413560::
	setvar VAR_0x8004, 0x91
	goto Quetzal_EventScript_845951C

@ Kanto_PowerPlant object 8 (local id 8, gfx 0x3F0E) at 30,38
Kanto_PowerPlant_EventScript_841356B::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	setwildbattle SPECIES_ELECTRODE, 0x22, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	waitse
	playmoncry SPECIES_ELECTRODE, 2
	delay 0x28
	waitmoncry
	setflag FLAG_QZ_0x2215
	dowildbattle
	clearflag FLAG_QZ_0x2215
	special QzQuestLogNop
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_PowerPlant_EventScript_84135C7
	goto_if_eq VAR_RESULT, 4, Kanto_PowerPlant_EventScript_84135C7
	goto_if_eq VAR_RESULT, 5, Kanto_PowerPlant_EventScript_84135C7
	setflag FLAG_QZ_0x2139
	release
	end

Kanto_PowerPlant_EventScript_84135C7::
	setflag FLAG_QZ_0x2139
	goto Quetzal_EventScript_8459513

@ Kanto_PowerPlant object 7 (local id 7, gfx 0x3F0E) at 36,5
Kanto_PowerPlant_EventScript_84135D0::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	lock
	faceplayer
	setwildbattle SPECIES_ELECTRODE, 0x22, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	waitse
	playmoncry SPECIES_ELECTRODE, 2
	delay 0x28
	waitmoncry
	setflag FLAG_QZ_0x2215
	dowildbattle
	clearflag FLAG_QZ_0x2215
	special QzQuestLogNop
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_PowerPlant_EventScript_841362C
	goto_if_eq VAR_RESULT, 4, Kanto_PowerPlant_EventScript_841362C
	goto_if_eq VAR_RESULT, 5, Kanto_PowerPlant_EventScript_841362C
	setflag FLAG_QZ_0x213A
	release
	end

Kanto_PowerPlant_EventScript_841362C::
	setflag FLAG_QZ_0x213A
	goto Quetzal_EventScript_8459513

@ Kanto_PowerPlant object 1 (local id 1, gfx 0x3F0E) at 7,27
Kanto_PowerPlant_EventScript_846BDF2::
	setorcopyvar VAR_0x8000, ITEM_MAX_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PowerPlant object 2 (local id 2, gfx 0x3F0E) at 40,22
Kanto_PowerPlant_EventScript_846BDFF::
	setorcopyvar VAR_0x8000, ITEM_SILK_SCARF
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PowerPlant object 3 (local id 3, gfx 0x3F0E) at 46,37
Kanto_PowerPlant_EventScript_846BE0C::
	setorcopyvar VAR_0x8000, ITEM_CHARCOAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PowerPlant object 4 (local id 4, gfx 0x3F0E) at 45,4
Kanto_PowerPlant_EventScript_846BE19::
	setorcopyvar VAR_0x8000, ITEM_THUNDER_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_PowerPlant object 5 (local id 5, gfx 0x3F0E) at 26,22
Kanto_PowerPlant_EventScript_846BE26::
	setorcopyvar VAR_0x8000, ITEM_ELIXIR
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_PowerPlant_Text_84568C3::
	.string "Zapdos!$"
