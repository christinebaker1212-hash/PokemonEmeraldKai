@ SINNOH's elevators (src/sinnoh/s3d_elevator.c, tools/sinnoh/gen_maps.py): the panel or the attendant asks for a floor,
@ the elevator moves (Emerald's MoveElevator) and its doors open there
Sinnoh_EventScript_Elevator::
	lockall
	message Sinnoh_Text_ElevatorWhich
	waitmessage
	special Sinnoh_ElevatorMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Sinnoh_EventScript_ElevatorDone
	specialvar VAR_RESULT, Sinnoh_ElevatorGo
	goto_if_eq VAR_RESULT, 0, Sinnoh_EventScript_ElevatorDone
	closemessage
	waitse
	special MoveElevator
	waitstate
	msgbox Sinnoh_Text_ElevatorArrived, MSGBOX_DEFAULT
Sinnoh_EventScript_ElevatorDone::
	closemessage
	releaseall
	end

Sinnoh_Text_ElevatorWhich:
	.string "Which floor would you like?$"

Sinnoh_Text_ElevatorArrived:
	.string "Ding! The doors slide open.$"
