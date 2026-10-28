@ Moemon Rhythm: the DANCE machine on the GAME CENTER's 3F (top left: the screen and two pads). A player steps up
@ from the row in front of it and dances on the left pad (1P) or the right (2P): PLAY ALONE, or WAIT FOR 2P with
@ "Let's dance!" over them; a friend stepping up to the other pad is asked to join (src/mp/mp_rhythm.c,
@ docs/RHYTHM_DECISIONS.md).
EventScript_DanceMachine::
	lockall
	special MP_Rhythm_StepUp
	goto_if_eq VAR_RESULT, 1, EventScript_DanceMachine_Join
	goto_if_eq VAR_RESULT, 2, EventScript_DanceMachine_Taken
	goto_if_eq VAR_RESULT, 3, EventScript_DanceMachine_InUse
	goto_if_eq VAR_RESULT, 4, EventScript_DanceMachine_SoloInUse
	message DanceMachine_Text_StepUp
	waitmessage
	multichoice 19, 6, MULTI_DANCE_MACHINE, FALSE
	switch VAR_RESULT
	case 0, EventScript_DanceMachine_Alone
	case 1, EventScript_DanceMachine_Wait
	special MP_Rhythm_Result
	releaseall
	end

EventScript_DanceMachine_Alone::
	closemessage
	special MP_Rhythm_PlayAlone
	goto EventScript_DanceMachine_Results
	end

EventScript_DanceMachine_Wait::
	message DanceMachine_Text_Waiting
	waitmessage
	special MP_Rhythm_Wait
	closemessage
	goto EventScript_DanceMachine_Results
	end

EventScript_DanceMachine_Join::
	msgbox DanceMachine_Text_DanceWith, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_DanceMachine_StepOff
	closemessage
	special MP_Rhythm_Join
	goto EventScript_DanceMachine_Results
	end

EventScript_DanceMachine_StepOff::
	special MP_Rhythm_Result
	releaseall
	end

EventScript_DanceMachine_Taken::
	msgbox DanceMachine_Text_Taken, MSGBOX_DEFAULT
	releaseall
	end

EventScript_DanceMachine_InUse::
	msgbox DanceMachine_Text_InUse, MSGBOX_DEFAULT
	releaseall
	end

EventScript_DanceMachine_SoloInUse::
	msgbox DanceMachine_Text_SoloInUse, MSGBOX_DEFAULT
	releaseall
	end

EventScript_DanceMachine_Results::
	special MP_Rhythm_Result
	switch VAR_RESULT
	case 1, EventScript_DanceMachine_Played
	case 2, EventScript_DanceMachine_Refused
	releaseall
	end

EventScript_DanceMachine_Played::
	msgbox DanceMachine_Text_Played, MSGBOX_DEFAULT
	releaseall
	end

EventScript_DanceMachine_Refused::
	msgbox DanceMachine_Text_Refused, MSGBOX_DEFAULT
	releaseall
	end

DanceMachine_Text_StepUp:
	.string "It's a DANCE machine!\n"
	.string "Dance alone, or with a friend?$"

DanceMachine_Text_Waiting:
	.string "Waiting for a dance partner…\n"
	.string "B: step off$"

DanceMachine_Text_DanceWith:
	.string "Dance with {STR_VAR_1}?$"

DanceMachine_Text_Taken:
	.string "{STR_VAR_1} is on this pad.$"

DanceMachine_Text_InUse:
	.string "{STR_VAR_1} is in the middle\n"
	.string "of a song!$"

DanceMachine_Text_SoloInUse:
	.string "{STR_VAR_1} is dancing alone.\n"
	.string "Wait for the next credit!$"

DanceMachine_Text_Played:
	.string "Phew! What a workout!$"

DanceMachine_Text_Refused:
	.string "{STR_VAR_1} can't dance\n"
	.string "right now.$"
