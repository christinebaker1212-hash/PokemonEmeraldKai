Kanto_RocketHideout_Elevator_MapScripts::
	.byte 0

@ Kanto_RocketHideout_Elevator sign / bg event at 0,2
Kanto_RocketHideout_Elevator_EventScript_84111D4::
	lockall
	goto_if_unset FLAG_QZ_0x2112, Kanto_RocketHideout_Elevator_EventScript_8411319
	setvar VAR_0x8004, 2
	call_if_unset FLAG_TEMP_2, Quetzal_EventScript_8457F7C
	copyvar VAR_0x8005, VAR_QZ_0x5104
	special ShowDeptStoreElevatorFloorSelect
	message Quetzal_Text_84565ED
	waitmessage
	setvar VAR_0x8004, 2
	specialvar VAR_RESULT, QzGetElevatorFloorCursor
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_RocketHideout_Elevator_EventScript_841122B
	goto_if_eq VAR_0x8000, 1, Kanto_RocketHideout_Elevator_EventScript_8411238
	goto_if_eq VAR_0x8000, 2, Kanto_RocketHideout_Elevator_EventScript_8411245
	end

Kanto_RocketHideout_Elevator_EventScript_841122B::
	multichoicedefault 0, 0, MULTI_QZ_0x6E, 1, 0
	nop
	goto Kanto_RocketHideout_Elevator_EventScript_8411252

Kanto_RocketHideout_Elevator_EventScript_8411238::
	multichoicedefault 0, 0, MULTI_QZ_0x6E, 1, 1
	nop
	goto Kanto_RocketHideout_Elevator_EventScript_8411252

Kanto_RocketHideout_Elevator_EventScript_8411245::
	multichoicedefault 0, 0, MULTI_QZ_0x6E, 1, 2
	nop
	goto Kanto_RocketHideout_Elevator_EventScript_8411252

Kanto_RocketHideout_Elevator_EventScript_8411252::
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_RocketHideout_Elevator_EventScript_841128F
	goto_if_eq VAR_0x8000, 1, Kanto_RocketHideout_Elevator_EventScript_84112B7
	goto_if_eq VAR_0x8000, 2, Kanto_RocketHideout_Elevator_EventScript_84112DF
	goto_if_eq VAR_0x8000, 3, Kanto_RocketHideout_Elevator_EventScript_8411307
	goto_if_eq VAR_0x8000, 127, Kanto_RocketHideout_Elevator_EventScript_8411307
	end

Kanto_RocketHideout_Elevator_EventScript_841128F::
	setvar VAR_0x8006, 3
	setdynamicwarp MAP_KANTO_ROCKET_HIDEOUT_B1F, 255, 24, 25
	goto_if_eq VAR_QZ_0x5104, 3, Kanto_RocketHideout_Elevator_EventScript_8411307
	call Kanto_RocketHideout_Elevator_EventScript_841130C
	setvar VAR_QZ_0x5104, 3
	goto Kanto_RocketHideout_Elevator_EventScript_8411307

Kanto_RocketHideout_Elevator_EventScript_84112B7::
	setvar VAR_0x8006, 2
	setdynamicwarp MAP_KANTO_ROCKET_HIDEOUT_B2F, 255, 28, 16
	goto_if_eq VAR_QZ_0x5104, 2, Kanto_RocketHideout_Elevator_EventScript_8411307
	call Kanto_RocketHideout_Elevator_EventScript_841130C
	setvar VAR_QZ_0x5104, 2
	goto Kanto_RocketHideout_Elevator_EventScript_8411307

Kanto_RocketHideout_Elevator_EventScript_84112DF::
	setvar VAR_0x8006, 0
	setdynamicwarp MAP_KANTO_ROCKET_HIDEOUT_B4F, 255, 20, 23
	goto_if_eq VAR_QZ_0x5104, 0, Kanto_RocketHideout_Elevator_EventScript_8411307
	call Kanto_RocketHideout_Elevator_EventScript_841130C
	setvar VAR_QZ_0x5104, 0
	goto Kanto_RocketHideout_Elevator_EventScript_8411307

Kanto_RocketHideout_Elevator_EventScript_8411307::
	special QzCloseDeptStoreElevatorWindow
	releaseall
	end

Kanto_RocketHideout_Elevator_EventScript_841130C::
	special QzCloseDeptStoreElevatorWindow
	closemessage
	waitse
	special MoveElevator
	waitstate
	setflag FLAG_TEMP_2
	return

Kanto_RocketHideout_Elevator_EventScript_8411319::
	msgbox Kanto_RocketHideout_Elevator_Text_8425B95, MSGBOX_DEFAULT
	releaseall
	end

Kanto_RocketHideout_Elevator_Text_8425B95::
	.string "It appears to need a key.$"
