Kanto_Route22_NorthEntrance_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route22_NorthEntrance_EventScript_84205EA
	map_script MAP_SCRIPT_ON_LOAD, Kanto_Route22_NorthEntrance_EventScript_84205EA
	map_script MAP_SCRIPT_ON_RESUME, Kanto_Route22_NorthEntrance_EventScript_84205EA
	map_script MAP_SCRIPT_ON_RETURN_TO_FIELD, Kanto_Route22_NorthEntrance_EventScript_8420608
	.byte 0

Kanto_Route22_NorthEntrance_EventScript_84205EA::
	call Kanto_Route22_NorthEntrance_EventScript_84205F0
	end

Kanto_Route22_NorthEntrance_EventScript_84205F0::
	specialvar VAR_RESULT, QzVictoryRoadGateCheck
	goto_if_eq VAR_RESULT, 1, Kanto_Route22_NorthEntrance_EventScript_8420604
	clearflag FLAG_QZ_0x210A
	return

Kanto_Route22_NorthEntrance_EventScript_8420604::
	setflag FLAG_QZ_0x210A
	return

Kanto_Route22_NorthEntrance_EventScript_8420608::
	call Kanto_Route22_NorthEntrance_EventScript_84205F0
	goto_if_set FLAG_QZ_0x210A, Kanto_Route22_NorthEntrance_EventScript_842061A
	addobject 2
	end

Kanto_Route22_NorthEntrance_EventScript_842061A::
	removeobject 2
	end

@ Kanto_Route22_NorthEntrance object 2 (local id 2, gfx 0x3F36) at 3,2
Kanto_Route22_NorthEntrance_EventScript_842061E::
	msgbox Quetzal_Text_83E90EF, MSGBOX_NPC
	end

@ Kanto_Route22_NorthEntrance object 1 (local id 1, gfx 0x3F36) at 8,2
Kanto_Route22_NorthEntrance_EventScript_8420627::
	lock
	faceplayer
	setvar VAR_TEMP_1, 1
	bufferstdstring 0, 0x25
	goto Kanto_Route22_NorthEntrance_EventScript_8457CC6

@ Kanto_Route22_NorthEntrance trigger at 7,2 (VAR_QZ_0x5126 == 0)
Kanto_Route22_NorthEntrance_EventScript_8420638::
	lockall
	setvar VAR_TEMP_1, 1
	setvar VAR_0x8009, 1
	bufferstdstring 0, 0x25
	goto Quetzal_EventScript_8457E31

Kanto_Route22_NorthEntrance_EventScript_8457CAF::
	message Quetzal_Text_8455AA2
	waitmessage
	playfanfare MUS_LEVEL_UP
	waitfanfare
	waitbuttonpress
	release
	end

Kanto_Route22_NorthEntrance_EventScript_8457CC6::
	compare_var_to_var VAR_QZ_0x5126, VAR_TEMP_1
	goto_if_ge Kanto_Route22_NorthEntrance_EventScript_8457CAF
	copyvar VAR_0x8000, VAR_TEMP_1
	goto_if_eq VAR_0x8000, 1, Quetzal_EventScript_8457D98
	goto_if_eq VAR_0x8000, 2, Quetzal_EventScript_8457DA7
	goto_if_eq VAR_0x8000, 3, Quetzal_EventScript_8457DB6
	goto_if_eq VAR_0x8000, 4, Quetzal_EventScript_8457DC5
	goto_if_eq VAR_0x8000, 5, Quetzal_EventScript_8457DD4
	goto_if_eq VAR_0x8000, 6, Quetzal_EventScript_8457DE3
	goto_if_eq VAR_0x8000, 7, Quetzal_EventScript_8457DF2
	goto_if_eq VAR_0x8000, 8, Quetzal_EventScript_8457E01
	end
