Kanto_NavelRock_Base_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_NavelRock_Base_EventScript_8414EEF
	map_script MAP_SCRIPT_ON_RESUME, Kanto_NavelRock_Base_EventScript_8414F13
	.byte 0

Kanto_NavelRock_Base_EventScript_8414EEF::
	call_if_set FLAG_QZ_0x2136, Kanto_NavelRock_Base_EventScript_8414F02
	call_if_unset FLAG_QZ_0x2136, Kanto_NavelRock_Base_EventScript_8414F06
	end

Kanto_NavelRock_Base_EventScript_8414F02::
	setflag FLAG_QZ_0x219C
	return

Kanto_NavelRock_Base_EventScript_8414F06::
	goto_if_set FLAG_QZ_0x21DE, Quetzal_EventScript_8457C6C
	clearflag FLAG_QZ_0x219C
	return

Kanto_NavelRock_Base_EventScript_8414F13::
	call_if_set FLAG_QZ_0x2215, Kanto_NavelRock_Base_EventScript_8414F1D
	end

Kanto_NavelRock_Base_EventScript_8414F1D::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457C6C
	removeobject 1
	return

@ Kanto_NavelRock_Base object 1 (local id 1, gfx 0x3F1B) at 10,15
Kanto_NavelRock_Base_EventScript_8414F31::
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	lock
	faceplayer
	waitse
	delay 0x14
	playse SE_THUNDERSTORM_STOP
	setvar VAR_0x8004, 0
	setvar VAR_0x8005, 3
	setvar VAR_0x8006, 4
	setvar VAR_0x8007, 2
	special ShakeCamera
	delay 0x1E
	playse SE_THUNDERSTORM_STOP
	setvar VAR_0x8004, SPECIES_NONE
	setvar VAR_0x8005, 3
	setvar VAR_0x8006, 4
	setvar VAR_0x8007, 2
	special ShakeCamera
	delay 0x1E
	delay 0x32
	waitse
	playmoncry SPECIES_LUGIA, 2
	waitmoncry
	delay 0x14
	setvar VAR_0x8004, SPECIES_LUGIA
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
	goto_if_eq VAR_RESULT, 1, Kanto_NavelRock_Base_EventScript_8414FF4
	goto_if_eq VAR_RESULT, 4, Kanto_NavelRock_Base_EventScript_8415002
	goto_if_eq VAR_RESULT, 5, Kanto_NavelRock_Base_EventScript_8415002
	goto_if_ne VAR_RESULT, 7, Quetzal_EventScript_8457FA3
	setflag FLAG_QZ_0x2136
	release
	end

Kanto_NavelRock_Base_EventScript_8414FF4::
	setflag FLAG_QZ_0x21DE
	setvar VAR_0x8004, 0xF9
	goto Quetzal_EventScript_845951C

Kanto_NavelRock_Base_EventScript_8415002::
	setvar VAR_0x8004, 0xF9
	goto Quetzal_EventScript_845951C
