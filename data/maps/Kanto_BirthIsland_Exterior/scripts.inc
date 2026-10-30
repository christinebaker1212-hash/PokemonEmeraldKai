Kanto_BirthIsland_Exterior_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_BirthIsland_Exterior_EventScript_8415032
	map_script MAP_SCRIPT_ON_RESUME, Kanto_BirthIsland_Exterior_EventScript_841507F
	map_script MAP_SCRIPT_ON_RETURN_TO_FIELD, Kanto_BirthIsland_Exterior_EventScript_841502E
	.byte 0

Kanto_BirthIsland_Exterior_EventScript_841502E::
	special SetDeoxysRockPalette
	end

Kanto_BirthIsland_Exterior_EventScript_8415032::
	setflag FLAG_QZ_0x2233
	setvar VAR_QZ_0x513E, 0xE8
	setvar VAR_QZ_0x5102, 0
	setvar VAR_QZ_0x5101, 0
	setobjectxyperm 1, 0xF, 0xC
	setobjectxyperm 2, 0xF, 3
	call_if_set FLAG_QZ_0x2134, Kanto_BirthIsland_Exterior_EventScript_8415065
	call_if_unset FLAG_QZ_0x2134, Kanto_BirthIsland_Exterior_EventScript_841506C
	end

Kanto_BirthIsland_Exterior_EventScript_8415065::
	setflag FLAG_QZ_0x2184
	setflag FLAG_QZ_0x217B
	return

Kanto_BirthIsland_Exterior_EventScript_841506C::
	goto_if_set FLAG_QZ_0x2126, Kanto_BirthIsland_Exterior_EventScript_8415065
	setflag FLAG_QZ_0x2184
	clearflag FLAG_QZ_0x217B
	clearflag FLAG_QZ_0x220B
	return

Kanto_BirthIsland_Exterior_EventScript_841507F::
	call_if_set FLAG_QZ_0x2215, Kanto_BirthIsland_Exterior_EventScript_8415089
	end

Kanto_BirthIsland_Exterior_EventScript_8415089::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457C6C
	removeobject 2
	return

@ Kanto_BirthIsland_Exterior object 1 (local id 1, gfx 0x3F20) at 15,12
Kanto_BirthIsland_Exterior_EventScript_841509D::
	lock
	faceplayer
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	special DoDeoxysRockInteraction
	waitstate
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_BirthIsland_Exterior_EventScript_84150E8
	goto_if_eq VAR_0x8000, 1, Kanto_BirthIsland_Exterior_EventScript_84150EA
	goto_if_eq VAR_0x8000, 2, Kanto_BirthIsland_Exterior_EventScript_84150EE
	goto_if_eq VAR_0x8000, 3, Kanto_BirthIsland_Exterior_EventScript_84150EC
	end

Kanto_BirthIsland_Exterior_EventScript_84150E8::
	release
	end

Kanto_BirthIsland_Exterior_EventScript_84150EA::
	release
	end

Kanto_BirthIsland_Exterior_EventScript_84150EC::
	release
	end

Kanto_BirthIsland_Exterior_EventScript_84150EE::
	waitse
	setfieldeffectargument 0, 1
	setfieldeffectargument 1, 0x35
	setfieldeffectargument 2, 0x25
	dofieldeffect 0x41
	playbgm MUS_RG_ENCOUNTER_DEOXYS, 0
	waitfieldeffect 0x41
	setflag FLAG_QZ_0x217B
	addobject 2
	applymovement 2, Kanto_BirthIsland_Exterior_Movement_84151A8
	waitmovement 0
	waitse
	playmoncry SPECIES_DEOXYS, 2
	delay 0x28
	waitmoncry
	setvar VAR_LAST_TALKED, 2
	setvar VAR_0x8004, SPECIES_DEOXYS
	setvar VAR_0x8005, 0x1E
	setvar VAR_0x8006, 0
	special QzKantoStaticEncounterLookupBySpecies
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457FA3
	special CreateEnemyEventMon
	setflag FLAG_QZ_0x2215
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0x2215
	special QzKantoStaticEncounterAfterBattle
	setvar VAR_LAST_TALKED, 2
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_BirthIsland_Exterior_EventScript_841518F
	goto_if_eq VAR_RESULT, 4, Kanto_BirthIsland_Exterior_EventScript_841519D
	goto_if_eq VAR_RESULT, 5, Kanto_BirthIsland_Exterior_EventScript_841519D
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457FA3
	setflag FLAG_QZ_0x2134
	removeobject 2
	release
	end

Kanto_BirthIsland_Exterior_EventScript_841518F::
	setflag FLAG_QZ_0x2126
	setvar VAR_0x8004, 0x182
	goto Quetzal_EventScript_845951C

Kanto_BirthIsland_Exterior_EventScript_841519D::
	setvar VAR_0x8004, 0x182
	goto Quetzal_EventScript_845951C

Kanto_BirthIsland_Exterior_Movement_84151A8:
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	walk_slow_down
	step_end
