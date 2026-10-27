@ Moemon Fight: the VS cabinet in MAUVILLE's GAME CORNER, two slot machines back to back (the right
@ block's bottom row). Sitting down offers WAIT FOR 2P or FIGHT CPU (src/mp/mp_fight.c,
@ docs/FIGHT_DECISIONS.md). The west seat plays the left side.
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
	message VsCabinet_Text_Sit
	waitmessage
	multichoice 19, 6, MULTI_VS_CABINET, FALSE
	switch VAR_RESULT
	case 0, EventScript_VsCabinet_Wait
	case 1, EventScript_VsCabinet_Cpu
	releaseall
	end

EventScript_VsCabinet_Wait::
	msgbox VsCabinet_Text_NobodyYet, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_Cpu::
	closemessage
	special MP_Fight_StartCpu
	special MP_Fight_Result
	switch VAR_RESULT
	case 1, EventScript_VsCabinet_Won
	case 2, EventScript_VsCabinet_Lost
	case 3, EventScript_VsCabinet_Draw
	case 5, EventScript_VsCabinet_Broken
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

VsCabinet_Text_Sit:
	.string "It's a VS cabinet!\n"
	.string "Fight a friend, or the CPU?$"

VsCabinet_Text_NobodyYet:
	.string "Nobody can take the other seat\n"
	.string "right now…$"

VsCabinet_Text_Won:
	.string "YOU WIN!$"

VsCabinet_Text_Lost:
	.string "YOU LOSE…$"

VsCabinet_Text_Draw:
	.string "DRAW GAME!$"

VsCabinet_Text_Broken:
	.string "The screen flickered and went dark.\n"
	.string "The cabinet is out of order…$"
