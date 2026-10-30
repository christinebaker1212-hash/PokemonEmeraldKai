@ Moemon Fight: the VS cabinet in MAUVILLE's GAME CORNER, two slot machines back to back (the right
@ block's bottom row). Sitting down offers WAIT FOR 2P or FIGHT CPU; a friend already sitting at the
@ other seat gets asked instead (src/mp/mp_fight.c, docs/FIGHT_DECISIONS.md). The west seat plays
@ the left side.
EventScript_VsCabinetWest::
	setvar VAR_0x8004, 0
	goto EventScript_VsCabinet
	end

EventScript_VsCabinetEast::
	setvar VAR_0x8004, 1
	goto EventScript_VsCabinet
	end

EventScript_VsCabinet::
	lockall
	special MP_Fight_SeatCheck
	goto_if_eq VAR_RESULT, 1, EventScript_VsCabinet_Challenge
	goto_if_eq VAR_RESULT, 2, EventScript_VsCabinet_Taken
	goto_if_eq VAR_RESULT, 3, EventScript_VsCabinet_InUse
	message VsCabinet_Text_Sit
	waitmessage
	multichoice 19, 6, MULTI_VS_CABINET, FALSE
	switch VAR_RESULT
	case 0, EventScript_VsCabinet_Wait
	case 1, EventScript_VsCabinet_Cpu
	releaseall
	end

EventScript_VsCabinet_Challenge::
	msgbox VsCabinet_Text_FightFriend, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_VsCabinet_Release
	closemessage
	special MP_Fight_Challenge
	goto EventScript_VsCabinet_Results
	end

EventScript_VsCabinet_Taken::
	msgbox VsCabinet_Text_Taken, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_InUse::
	msgbox VsCabinet_Text_InUse, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_Wait::
	message VsCabinet_Text_Waiting
	waitmessage
	special MP_Fight_Wait
	closemessage
	goto EventScript_VsCabinet_Results
	end

EventScript_VsCabinet_Cpu::
	closemessage
	special MP_Fight_StartCpu
	goto EventScript_VsCabinet_Results
	end

EventScript_VsCabinet_Results::
	special MP_Fight_Result
	switch VAR_RESULT
	case 1, EventScript_VsCabinet_Won
	case 2, EventScript_VsCabinet_Lost
	case 3, EventScript_VsCabinet_Draw
	case 5, EventScript_VsCabinet_Broken
	case 6, EventScript_VsCabinet_Refused
	case 7, EventScript_VsCabinet_NoContest
	case 9, EventScript_VsCabinet_Packs
	releaseall
	end

EventScript_VsCabinet_Release::
	releaseall
	end

EventScript_VsCabinet_Won::
	msgbox VsCabinet_Text_Won, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_Lost::
	msgbox VsCabinet_Text_Lost, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_Draw::
	msgbox VsCabinet_Text_Draw, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_Broken::
	msgbox VsCabinet_Text_Broken, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_Refused::
	msgbox VsCabinet_Text_Refused, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_NoContest::
	msgbox VsCabinet_Text_NoContest, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_Packs::
	msgbox VsCabinet_Text_Packs, MSGBOX_DEFAULT
	releaseall
	end

VsCabinet_Text_Sit:
	.string "It's a VS cabinet!\n"
	.string "Fight a friend, or the CPU?$"

VsCabinet_Text_Waiting:
	.string "Waiting for a challenger…\n"
	.string "B: stand up$"

VsCabinet_Text_FightFriend:
	.string "Fight {STR_VAR_1}?$"

VsCabinet_Text_Taken:
	.string "{STR_VAR_1} is sitting here.$"

VsCabinet_Text_InUse:
	.string "{STR_VAR_1} is in the middle\n"
	.string "of a fight!$"

VsCabinet_Text_Packs:
	.string "{STR_VAR_1}'s fight files aren't\n"
	.string "the same as yours. No fight…$"

VsCabinet_Text_Won:
	.string "YOU WIN!$"

VsCabinet_Text_Lost:
	.string "YOU LOSE…$"

VsCabinet_Text_Draw:
	.string "DRAW GAME!$"

VsCabinet_Text_Broken:
	.string "The screen flickered and went dark.\n"
	.string "The cabinet is out of order…$"

VsCabinet_Text_Refused:
	.string "{STR_VAR_1} can't fight\n"
	.string "right now.$"

VsCabinet_Text_NoContest:
	.string "{STR_VAR_1} left the fight.\n"
	.string "No contest!$"
