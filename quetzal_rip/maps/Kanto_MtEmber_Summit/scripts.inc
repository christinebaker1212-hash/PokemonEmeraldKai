Kanto_MtEmber_Summit_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_MtEmber_Summit_EventScript_841389E
	map_script MAP_SCRIPT_ON_RESUME, Kanto_MtEmber_Summit_EventScript_8413880
	.byte 0

Kanto_MtEmber_Summit_EventScript_8413880::
	call_if_set FLAG_QZ_0x2215, Kanto_MtEmber_Summit_EventScript_841388A
	end

Kanto_MtEmber_Summit_EventScript_841388A::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457C6C
	removeobject VAR_LAST_TALKED
	return

Kanto_MtEmber_Summit_EventScript_841389E::
	call_if_unset FLAG_QZ_0x2138, Kanto_MtEmber_Summit_EventScript_84138A8
	end

Kanto_MtEmber_Summit_EventScript_84138A8::
	clearflag FLAG_QZ_0x219F
	return

@ Kanto_MtEmber_Summit object 1 (local id 1, gfx 0x3F24) at 9,6
Kanto_MtEmber_Summit_EventScript_84138AC::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop_737
	lock
	faceplayer
	setvar VAR_0x8004, 0x92
	setvar VAR_0x8005, 0x32
	setvar VAR_0x8006, 0
	special QzKantoStaticEncounterLookupBySpecies_755
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457FA3
	special CreateEnemyEventMon
	waitse
	playmoncry SPECIES_MOLTRES, 2
	message Kanto_MtEmber_Summit_Text_84568CB
	waitmessage
	waitmoncry
	delay 0xA
	playbgm MUS_RG_ENCOUNTER_GYM_LEADER, 0
	waitbuttonpress
	setflag FLAG_QZ_0x2215
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0x2215
	special QzKantoStaticEncounterAfterBattle_756
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_MtEmber_Summit_EventScript_8413939
	goto_if_eq VAR_RESULT, 4, Kanto_MtEmber_Summit_EventScript_8413942
	goto_if_eq VAR_RESULT, 5, Kanto_MtEmber_Summit_EventScript_8413942
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457FA3
	setflag FLAG_QZ_0x2138
	release
	end

Kanto_MtEmber_Summit_EventScript_8413939::
	setflag FLAG_QZ_0x2138
	goto Quetzal_EventScript_8459513

Kanto_MtEmber_Summit_EventScript_8413942::
	setvar VAR_0x8004, 0x92
	goto Quetzal_EventScript_845951C

Kanto_MtEmber_Summit_Text_84568CB::
	.string "Moltres!$"
