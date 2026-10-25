Kanto_SevenIsland_SevaultCanyon_TanobyKey_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414BB1
	.byte 0

Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414BB1::
	setflag FLAG_QZ_0x2253
	call_if_set FLAG_QZ_0x2216, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414BE6
	setvar VAR_TEMP_1, 0
	setvar VAR_TEMP_2, 0
	setvar VAR_TEMP_3, 0
	setvar VAR_TEMP_4, 0
	setvar VAR_TEMP_5, 0
	setvar VAR_TEMP_6, 0
	setvar VAR_TEMP_7, 0
	setvar VAR_TEMP_8, 0
	end

Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414BE6::
	setobjectxyperm 1, 7, 2
	setobjectxyperm 2, 4, 4
	setobjectxyperm 3, 0xA, 4
	setobjectxyperm 4, 5, 6
	setobjectxyperm 5, 9, 6
	setobjectxyperm 6, 5, 8
	setobjectxyperm 7, 9, 8
	return

@ Kanto_SevenIsland_SevaultCanyon_TanobyKey trigger at 7,2 (VAR_TEMP_1 == 99)
Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414C18::
	lockall
	goto_if_eq VAR_TEMP_1, 100, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D69
	call Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D0D
	setvar VAR_TEMP_1, 0x64
	goto_if_eq VAR_TEMP_8, 7, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D6B
	releaseall
	end

@ Kanto_SevenIsland_SevaultCanyon_TanobyKey trigger at 4,4 (VAR_TEMP_2 == 99)
Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414C3B::
	lockall
	goto_if_eq VAR_TEMP_2, 100, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D69
	call Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D0D
	setvar VAR_TEMP_2, 0x64
	goto_if_eq VAR_TEMP_8, 7, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D6B
	releaseall
	end

@ Kanto_SevenIsland_SevaultCanyon_TanobyKey trigger at 10,4 (VAR_TEMP_3 == 99)
Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414C5E::
	lockall
	goto_if_eq VAR_TEMP_3, 100, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D69
	call Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D0D
	setvar VAR_TEMP_3, 0x64
	goto_if_eq VAR_TEMP_8, 7, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D6B
	releaseall
	end

@ Kanto_SevenIsland_SevaultCanyon_TanobyKey trigger at 5,6 (VAR_TEMP_4 == 99)
Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414C81::
	lockall
	goto_if_eq VAR_TEMP_4, 100, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D69
	call Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D0D
	setvar VAR_TEMP_4, 0x64
	goto_if_eq VAR_TEMP_8, 7, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D6B
	releaseall
	end

@ Kanto_SevenIsland_SevaultCanyon_TanobyKey trigger at 9,6 (VAR_TEMP_5 == 99)
Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414CA4::
	lockall
	goto_if_eq VAR_TEMP_5, 100, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D69
	call Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D0D
	setvar VAR_TEMP_5, 0x64
	goto_if_eq VAR_TEMP_8, 7, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D6B
	releaseall
	end

@ Kanto_SevenIsland_SevaultCanyon_TanobyKey trigger at 5,8 (VAR_TEMP_6 == 99)
Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414CC7::
	lockall
	goto_if_eq VAR_TEMP_6, 100, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D69
	call Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D0D
	setvar VAR_TEMP_6, 0x64
	goto_if_eq VAR_TEMP_8, 7, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D6B
	releaseall
	end

@ Kanto_SevenIsland_SevaultCanyon_TanobyKey trigger at 9,8 (VAR_TEMP_7 == 99)
Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414CEA::
	lockall
	goto_if_eq VAR_TEMP_7, 100, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D69
	call Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D0D
	setvar VAR_TEMP_7, 0x64
	goto_if_eq VAR_TEMP_8, 7, Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D6B
	releaseall
	end

Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D0D::
	addvar VAR_TEMP_8, 1
	playse SE_TRUCK_MOVE
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 3
	special ShakeCamera
	delay 0x3C
	return

Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D30::
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 2
	setvar VAR_0x8006, 0xA
	setvar VAR_0x8007, 3
	special ShakeCamera
	delay 0x3C
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 3
	setvar VAR_0x8006, 0xC
	setvar VAR_0x8007, 2
	special ShakeCamera
	delay 0x3C
	playse SE_LAVARIDGE_FALL_WARP
	waitse
	return

Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D69::
	releaseall
	end

Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D6B::
	call Kanto_SevenIsland_SevaultCanyon_TanobyKey_EventScript_8414D30
	msgbox Kanto_SevenIsland_SevaultCanyon_TanobyKey_Text_842DA24, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x2216
	releaseall
	end

Kanto_SevenIsland_SevaultCanyon_TanobyKey_Text_842DA24::
	.string "A mysterious rumble echoed\n"
	.string "somewhere far away.$"
