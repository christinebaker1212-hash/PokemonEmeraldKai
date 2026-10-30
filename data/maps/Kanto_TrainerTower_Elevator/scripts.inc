Kanto_TrainerTower_Elevator_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_8471D23
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_TrainerTower_Elevator_OnFrame_8471E1B
	.byte 0

Kanto_TrainerTower_Elevator_OnFrame_8471E1B:
	map_script_2 VAR_TEMP_2, 0, Quetzal_EventScript_8471E25
	.2byte 0

@ Kanto_TrainerTower_Elevator sign / bg event at 0,2
Kanto_TrainerTower_Elevator_EventScript_84148BD::
	lockall
	setvar VAR_0x8004, 6
	call_if_unset FLAG_TEMP_2, Quetzal_EventScript_8457F7C
	copyvar VAR_0x8005, VAR_QZ_0x5104
	special ShowDeptStoreElevatorFloorSelect
	message Quetzal_Text_84565ED
	waitmessage
	setvar VAR_0x8004, 6
	specialvar VAR_RESULT, QzGetElevatorFloorCursor
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_TrainerTower_Elevator_EventScript_8414900
	goto_if_eq VAR_0x8000, 1, Kanto_TrainerTower_Elevator_EventScript_841490D
	end

Kanto_TrainerTower_Elevator_EventScript_8414900::
	multichoicedefault 0, 0, MULTI_QZ_0x6F, 1, 0
	nop
	goto Kanto_TrainerTower_Elevator_EventScript_841491A

Kanto_TrainerTower_Elevator_EventScript_841490D::
	multichoicedefault 0, 0, MULTI_QZ_0x6F, 1, 1
	nop
	goto Kanto_TrainerTower_Elevator_EventScript_841491A

Kanto_TrainerTower_Elevator_EventScript_841491A::
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_TrainerTower_Elevator_EventScript_8414989
	goto_if_eq VAR_0x8000, 1, Kanto_TrainerTower_Elevator_EventScript_841494C
	goto_if_eq VAR_0x8000, 2, Kanto_TrainerTower_Elevator_EventScript_841498F
	goto_if_eq VAR_0x8000, 127, Kanto_TrainerTower_Elevator_EventScript_841498F
	end

Kanto_TrainerTower_Elevator_EventScript_841494C::
	setvar VAR_0x8006, 3
	setdynamicwarp MAP_KANTO_TRAINER_TOWER_LOBBY, 255, 17, 8
	goto_if_eq VAR_QZ_0x5104, 3, Kanto_TrainerTower_Elevator_EventScript_841498F
	call Kanto_TrainerTower_Elevator_EventScript_8414994
	setvar VAR_QZ_0x5104, 3
	special QzCloseDeptStoreElevatorWindow
	delay 0x19
	applymovement LOCALID_PLAYER, Kanto_TrainerTower_Elevator_Movement_84149A1
	waitmovement 0
	warp MAP_KANTO_TRAINER_TOWER_LOBBY, 255, 17, 8
	waitstate
	releaseall
	end

Kanto_TrainerTower_Elevator_EventScript_8414989::
	goto Kanto_TrainerTower_Elevator_EventScript_841498F

Kanto_TrainerTower_Elevator_EventScript_841498F::
	special QzCloseDeptStoreElevatorWindow
	releaseall
	end

Kanto_TrainerTower_Elevator_EventScript_8414994::
	special QzCloseDeptStoreElevatorWindow
	closemessage
	waitse
	special MoveElevator
	waitstate
	setflag FLAG_TEMP_2
	return

Kanto_TrainerTower_Elevator_Movement_84149A1:
	walk_in_place_faster_down
	delay_16
	walk_down
	walk_right
	walk_right
	walk_down
	step_end
