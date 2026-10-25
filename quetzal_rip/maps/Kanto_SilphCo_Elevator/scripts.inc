Kanto_SilphCo_Elevator_MapScripts::
	.byte 0

@ Kanto_SilphCo_Elevator sign / bg event at 0,2
Kanto_SilphCo_Elevator_EventScript_8411CB1::
	lockall
	setvar VAR_0x8004, 1
	call_if_unset FLAG_TEMP_2, Quetzal_EventScript_8457F7C
	copyvar VAR_0x8005, VAR_QZ_0x5104
	special ShowDeptStoreElevatorFloorSelect
	message Quetzal_Text_84565ED
	waitmessage
	setvar VAR_0x8004, 1
	specialvar VAR_RESULT, QzGetElevatorFloorCursor_738
	special QzListMenu_742
	waitstate
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_SilphCo_Elevator_EventScript_8411F01
	goto_if_eq VAR_0x8000, 1, Kanto_SilphCo_Elevator_EventScript_8411ED9
	goto_if_eq VAR_0x8000, 2, Kanto_SilphCo_Elevator_EventScript_8411EB1
	goto_if_eq VAR_0x8000, 3, Kanto_SilphCo_Elevator_EventScript_8411E89
	goto_if_eq VAR_0x8000, 4, Kanto_SilphCo_Elevator_EventScript_8411E61
	goto_if_eq VAR_0x8000, 5, Kanto_SilphCo_Elevator_EventScript_8411E39
	goto_if_eq VAR_0x8000, 6, Kanto_SilphCo_Elevator_EventScript_8411E11
	goto_if_eq VAR_0x8000, 7, Kanto_SilphCo_Elevator_EventScript_8411DE9
	goto_if_eq VAR_0x8000, 8, Kanto_SilphCo_Elevator_EventScript_8411DC1
	goto_if_eq VAR_0x8000, 9, Kanto_SilphCo_Elevator_EventScript_8411D99
	goto_if_eq VAR_0x8000, 10, Kanto_SilphCo_Elevator_EventScript_8411D71
	goto_if_eq VAR_0x8000, 11, Kanto_SilphCo_Elevator_EventScript_8411F29
	goto_if_eq VAR_0x8000, 127, Kanto_SilphCo_Elevator_EventScript_8411F29
	end

Kanto_SilphCo_Elevator_EventScript_8411D71::
	setvar VAR_0x8006, 4
	setdynamicwarp MAP_KANTO_SILPH_CO_1F, 255, 22, 3
	goto_if_eq VAR_QZ_0x5104, 4, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 4
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411D99::
	setvar VAR_0x8006, 5
	setdynamicwarp MAP_KANTO_SILPH_CO_2F, 255, 22, 3
	goto_if_eq VAR_QZ_0x5104, 5, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 5
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411DC1::
	setvar VAR_0x8006, 6
	setdynamicwarp MAP_KANTO_SILPH_CO_3F, 255, 22, 3
	goto_if_eq VAR_QZ_0x5104, 6, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 6
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411DE9::
	setvar VAR_0x8006, 7
	setdynamicwarp MAP_KANTO_SILPH_CO_4F, 255, 22, 3
	goto_if_eq VAR_QZ_0x5104, 7, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 7
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411E11::
	setvar VAR_0x8006, 8
	setdynamicwarp MAP_KANTO_SILPH_CO_5F, 255, 22, 3
	goto_if_eq VAR_QZ_0x5104, 8, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 8
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411E39::
	setvar VAR_0x8006, 9
	setdynamicwarp MAP_KANTO_SILPH_CO_6F, 255, 20, 3
	goto_if_eq VAR_QZ_0x5104, 9, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 9
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411E61::
	setvar VAR_0x8006, 0xA
	setdynamicwarp MAP_KANTO_SILPH_CO_7F, 255, 23, 3
	goto_if_eq VAR_QZ_0x5104, 10, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 0xA
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411E89::
	setvar VAR_0x8006, 0xB
	setdynamicwarp MAP_KANTO_SILPH_CO_8F, 255, 22, 3
	goto_if_eq VAR_QZ_0x5104, 11, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 0xB
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411EB1::
	setvar VAR_0x8006, 0xC
	setdynamicwarp MAP_KANTO_SILPH_CO_9F, 255, 24, 3
	goto_if_eq VAR_QZ_0x5104, 12, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 0xC
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411ED9::
	setvar VAR_0x8006, 0xD
	setdynamicwarp MAP_KANTO_SILPH_CO_10F, 255, 13, 3
	goto_if_eq VAR_QZ_0x5104, 13, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 0xD
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411F01::
	setvar VAR_0x8006, 0xE
	setdynamicwarp MAP_KANTO_SILPH_CO_11F, 255, 13, 3
	goto_if_eq VAR_QZ_0x5104, 14, Kanto_SilphCo_Elevator_EventScript_8411F29
	call Kanto_SilphCo_Elevator_EventScript_8411F2E
	setvar VAR_QZ_0x5104, 0xE
	goto Kanto_SilphCo_Elevator_EventScript_8411F29

Kanto_SilphCo_Elevator_EventScript_8411F29::
	special QzCloseDeptStoreElevatorWindow_538
	releaseall
	end

Kanto_SilphCo_Elevator_EventScript_8411F2E::
	special QzCloseDeptStoreElevatorWindow_538
	closemessage
	waitse
	special MoveElevator
	waitstate
	setflag FLAG_TEMP_2
	return
