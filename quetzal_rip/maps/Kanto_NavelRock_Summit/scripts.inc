Kanto_NavelRock_Summit_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_NavelRock_Summit_EventScript_8414D89
	map_script MAP_SCRIPT_ON_RESUME, Kanto_NavelRock_Summit_EventScript_8414DBC
	.byte 0

Kanto_NavelRock_Summit_EventScript_8414D89::
	call_if_set FLAG_QZ_0x2135, Kanto_NavelRock_Summit_EventScript_8414D9C
	call_if_unset FLAG_QZ_0x2135, Kanto_NavelRock_Summit_EventScript_8414DA5
	end

Kanto_NavelRock_Summit_EventScript_8414D9C::
	setvar VAR_TEMP_1, 1
	setflag FLAG_QZ_0x218D
	return

Kanto_NavelRock_Summit_EventScript_8414DA5::
	setvar VAR_TEMP_1, 1
	goto_if_set FLAG_QZ_0x21DB, Quetzal_EventScript_8457C6C
	setvar VAR_TEMP_1, 0
	clearflag FLAG_QZ_0x218D
	return

Kanto_NavelRock_Summit_EventScript_8414DBC::
	call_if_set FLAG_QZ_0x2215, Kanto_NavelRock_Summit_EventScript_8414DC6
	end

Kanto_NavelRock_Summit_EventScript_8414DC6::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457C6C
	removeobject 1
	return

@ Kanto_NavelRock_Summit trigger at 9,12 (VAR_TEMP_1 == 0)
Kanto_NavelRock_Summit_EventScript_8414DDA::
	specialvar VAR_RESULT, QzReturnZero_732
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop_737
	lockall
	setvar VAR_TEMP_1, 1
	special SpawnCameraObject
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0x23
	special LoopWingFlapSE
	applymovement 1, Kanto_NavelRock_Summit_Movement_8414EDD
	applymovement LOCALID_CAMERA, Kanto_NavelRock_Summit_Movement_8414ECA
	waitmovement 0
	delay 0x32
	setweather 0
	doweather
	waitse
	playmoncry SPECIES_HO_OH, 2
	delay 0x1E
	waitmoncry
	delay 0x3C
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0x23
	special LoopWingFlapSE
	applymovement LOCALID_CAMERA, Kanto_NavelRock_Summit_Movement_8414ECE
	applymovement 1, Kanto_NavelRock_Summit_Movement_8414ED4
	waitmovement 0
	special RemoveCameraObject
	setvar VAR_0x8004, 0xFA
	setvar VAR_0x8005, 0x46
	setvar VAR_0x8006, 0
	special QzKantoStaticEncounterLookupBySpecies_755
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_8457FA3
	special CreateEnemyEventMon
	setflag FLAG_QZ_0x2215
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_QZ_0x2215
	setvar VAR_LAST_TALKED, 1
	special QzKantoStaticEncounterAfterBattle_756
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_NavelRock_Summit_EventScript_8414EB1
	goto_if_eq VAR_RESULT, 4, Kanto_NavelRock_Summit_EventScript_8414EBF
	goto_if_eq VAR_RESULT, 5, Kanto_NavelRock_Summit_EventScript_8414EBF
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457FA3
	setflag FLAG_QZ_0x2135
	releaseall
	end

Kanto_NavelRock_Summit_EventScript_8414EB1::
	setflag FLAG_QZ_0x21DB
	setvar VAR_0x8004, 0xFA
	goto Quetzal_EventScript_845951C

Kanto_NavelRock_Summit_EventScript_8414EBF::
	setvar VAR_0x8004, 0xFA
	goto Quetzal_EventScript_845951C

Kanto_NavelRock_Summit_Movement_8414ECA:
	walk_up
	walk_up
	walk_up
	step_end

Kanto_NavelRock_Summit_Movement_8414ECE:
	delay_16
	delay_16
	walk_down
	walk_down
	walk_down
	step_end

Kanto_NavelRock_Summit_Movement_8414ED4:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_in_place_down
	delay_16
	delay_16
	step_end

Kanto_NavelRock_Summit_Movement_8414EDD:
	delay_16
	delay_16
	walk_in_place_down
	walk_in_place_down
	walk_in_place_down
	walk_in_place_down
	step_end
