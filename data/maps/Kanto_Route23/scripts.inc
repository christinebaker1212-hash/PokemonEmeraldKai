Kanto_Route23_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route23_EventScript_84181C9
	.byte 0

Kanto_Route23_EventScript_84181C9::
	clearflag FLAG_QZ_0x21D9
	setflag FLAG_QZ_0x21D8
	setvar VAR_QZ_0x5136, 0
	setvar VAR_QZ_0x5137, 0
	setvar VAR_QZ_0x5138, 0
	setvar VAR_QZ_0x5139, 0
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_Route23 object 1 (local id 1, gfx 0x3F36) at 15,149
Kanto_Route23_EventScript_84181E9::
	lock
	faceplayer
	setvar VAR_TEMP_1, 2
	bufferstdstring 0, 0x26
	goto Kanto_Route23_EventScript_8457D2F

@ Kanto_Route23 object 2 (local id 2, gfx 0x3F36) at 8,140
Kanto_Route23_EventScript_84181FA::
	lock
	faceplayer
	setvar VAR_TEMP_1, 3
	bufferstdstring 0, 0x27
	goto Kanto_Route23_EventScript_8457D2F

@ Kanto_Route23 object 3 (local id 3, gfx 0x3F36) at 14,123
Kanto_Route23_EventScript_841820B::
	lock
	faceplayer
	setvar VAR_TEMP_1, 4
	bufferstdstring 0, 0x28
	goto Kanto_Route23_EventScript_8457D2F

@ Kanto_Route23 object 4 (local id 4, gfx 0x3F36) at 16,112
Kanto_Route23_EventScript_841821C::
	lock
	faceplayer
	setvar VAR_TEMP_1, 5
	bufferstdstring 0, 0x29
	goto Kanto_Route23_EventScript_8457D2F

@ Kanto_Route23 object 5 (local id 5, gfx 0x3F36) at 10,94
Kanto_Route23_EventScript_841822D::
	lock
	faceplayer
	setvar VAR_TEMP_1, 6
	bufferstdstring 0, 0x2A
	goto Kanto_Route23_EventScript_8457D2F

@ Kanto_Route23 object 6 (local id 6, gfx 0x3F36) at 14,61
Kanto_Route23_EventScript_841823E::
	lock
	faceplayer
	setvar VAR_TEMP_1, 7
	bufferstdstring 0, 0x2B
	goto Kanto_Route23_EventScript_8457D2F

@ Kanto_Route23 object 7 (local id 7, gfx 0x3F36) at 6,35
Kanto_Route23_EventScript_841824F::
	lock
	faceplayer
	setvar VAR_TEMP_1, 8
	bufferstdstring 0, 0x2C
	goto Kanto_Route23_EventScript_8457D2F

@ Kanto_Route23 trigger at 13,149 (VAR_QZ_0x5126 == 1)
@ Kanto_Route23 trigger at 12,148 (VAR_QZ_0x5126 == 1)
@ Kanto_Route23 trigger at 11,148 (VAR_QZ_0x5126 == 1)
@ Kanto_Route23 trigger at 14,149 (VAR_QZ_0x5126 == 1)
@ Kanto_Route23 trigger at 10,148 (VAR_QZ_0x5126 == 1)
Kanto_Route23_EventScript_8418260::
	lockall
	setvar VAR_TEMP_1, 2
	setvar VAR_0x8009, 1
	bufferstdstring 0, 0x26
	goto Quetzal_EventScript_8457E31

@ Kanto_Route23 trigger at 7,140 (VAR_QZ_0x5126 == 2)
@ Kanto_Route23 trigger at 4,139 (VAR_QZ_0x5126 == 2)
@ Kanto_Route23 trigger at 5,139 (VAR_QZ_0x5126 == 2)
@ Kanto_Route23 trigger at 6,140 (VAR_QZ_0x5126 == 2)
Kanto_Route23_EventScript_8418275::
	lockall
	setvar VAR_TEMP_1, 3
	setvar VAR_0x8009, 2
	bufferstdstring 0, 0x27
	goto Quetzal_EventScript_8457E31

@ Kanto_Route23 trigger at 12,123 (VAR_QZ_0x5126 == 3)
@ Kanto_Route23 trigger at 10,122 (VAR_QZ_0x5126 == 3)
@ Kanto_Route23 trigger at 11,122 (VAR_QZ_0x5126 == 3)
@ Kanto_Route23 trigger at 13,123 (VAR_QZ_0x5126 == 3)
Kanto_Route23_EventScript_841828A::
	lockall
	setvar VAR_TEMP_1, 4
	setvar VAR_0x8009, 3
	bufferstdstring 0, 0x28
	goto Quetzal_EventScript_8457E31

@ Kanto_Route23 trigger at 11,111 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 8,109 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 9,109 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 10,109 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 11,109 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 12,109 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 12,111 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 14,112 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 13,112 (VAR_QZ_0x5126 == 4)
@ Kanto_Route23 trigger at 15,112 (VAR_QZ_0x5126 == 4)
Kanto_Route23_EventScript_841829F::
	lockall
	setvar VAR_TEMP_1, 5
	setvar VAR_0x8009, 4
	bufferstdstring 0, 0x29
	goto Quetzal_EventScript_8457E31

@ Kanto_Route23 trigger at 9,94 (VAR_QZ_0x5126 == 5)
@ Kanto_Route23 trigger at 8,94 (VAR_QZ_0x5126 == 5)
@ Kanto_Route23 trigger at 5,92 (VAR_QZ_0x5126 == 5)
@ Kanto_Route23 trigger at 7,94 (VAR_QZ_0x5126 == 5)
@ Kanto_Route23 trigger at 4,92 (VAR_QZ_0x5126 == 5)
@ Kanto_Route23 trigger at 3,92 (VAR_QZ_0x5126 == 5)
@ Kanto_Route23 trigger at 2,92 (VAR_QZ_0x5126 == 5)
Kanto_Route23_EventScript_84182B4::
	lockall
	setvar VAR_TEMP_1, 6
	setvar VAR_0x8009, 5
	bufferstdstring 0, 0x2A
	goto Quetzal_EventScript_8457E31

@ Kanto_Route23 trigger at 12,61 (VAR_QZ_0x5126 == 6)
@ Kanto_Route23 trigger at 10,60 (VAR_QZ_0x5126 == 6)
@ Kanto_Route23 trigger at 13,61 (VAR_QZ_0x5126 == 6)
@ Kanto_Route23 trigger at 11,60 (VAR_QZ_0x5126 == 6)
Kanto_Route23_EventScript_84182C9::
	lockall
	setvar VAR_TEMP_1, 7
	setvar VAR_0x8009, 6
	bufferstdstring 0, 0x2B
	goto Quetzal_EventScript_8457E31

@ Kanto_Route23 trigger at 1,35 (VAR_QZ_0x5126 == 7)
@ Kanto_Route23 trigger at 1,34 (VAR_QZ_0x5126 == 7)
@ Kanto_Route23 trigger at 1,32 (VAR_QZ_0x5126 == 7)
@ Kanto_Route23 trigger at 1,33 (VAR_QZ_0x5126 == 7)
@ Kanto_Route23 trigger at 1,31 (VAR_QZ_0x5126 == 7)
@ Kanto_Route23 trigger at 1,36 (VAR_QZ_0x5126 == 7)
@ Kanto_Route23 trigger at 5,35 (VAR_QZ_0x5126 == 7)
@ Kanto_Route23 trigger at 4,35 (VAR_QZ_0x5126 == 7)
Kanto_Route23_EventScript_84182DE::
	lockall
	setvar VAR_TEMP_1, 8
	setvar VAR_0x8009, 7
	bufferstdstring 0, 0x2C
	goto Quetzal_EventScript_8457E31

@ Kanto_Route23 sign / bg event at 3,31
Kanto_Route23_EventScript_84182F3::
	msgbox Kanto_Route23_Text_8438F02, MSGBOX_SIGN
	end

Kanto_Route23_EventScript_8457CBC::
	msgbox Quetzal_Text_8455B3E, MSGBOX_DEFAULT
	release
	end

Kanto_Route23_EventScript_8457D2F::
	compare_var_to_var VAR_QZ_0x5126, VAR_TEMP_1
	goto_if_ge Kanto_Route23_EventScript_8457CBC
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

Kanto_Route23_Text_8438F02::
	.string "VICTORY ROAD GATE -\n"
	.string "POKéMON LEAGUE$"
