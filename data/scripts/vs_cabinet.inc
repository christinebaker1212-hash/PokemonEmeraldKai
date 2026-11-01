@ Moemon Fight: the GAME CENTER 3F's VS cabinets, the five back-to-back pairs in the middle of the floor (each pair is
@ one cabinet with a seat on each side: the west seat plays 1P, the left side; the east seat 2P). Row 9 plays
@ SUPER STREET FIGHTER II X, 10 VAMPIRE SAVIOR, 11 CYBERBOTS, 13 STREET FIGHTER ALPHA 3 (CPS-2 arcade boards,
@ include/arcade.h, from each player's own ROM set in the "arcade" folder beside the game), 12 the MUGEN cabinet,
@ "a weird bootleg" (src/mp/mp_fight.c CABINET_*,
@ docs/FIGHT_DECISIONS.md). Each says which game it is first. Sitting down offers WAIT FOR 2P or 1P play; a friend
@ already sitting at the other side gets asked instead.
EventScript_GcVsSsf2xWest::
	setvar VAR_0x8005, 1
	setvar VAR_0x8004, 0
	lockall
	msgbox VsCabinet_Text_Ssf2x, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsSsf2xEast::
	setvar VAR_0x8005, 1
	setvar VAR_0x8004, 1
	lockall
	msgbox VsCabinet_Text_Ssf2x, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsVampireWest::
	setvar VAR_0x8005, 2
	setvar VAR_0x8004, 0
	lockall
	msgbox VsCabinet_Text_Vampire, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsVampireEast::
	setvar VAR_0x8005, 2
	setvar VAR_0x8004, 1
	lockall
	msgbox VsCabinet_Text_Vampire, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsCyberbotsWest::
	setvar VAR_0x8005, 3
	setvar VAR_0x8004, 0
	lockall
	msgbox VsCabinet_Text_Cyberbots, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsCyberbotsEast::
	setvar VAR_0x8005, 3
	setvar VAR_0x8004, 1
	lockall
	msgbox VsCabinet_Text_Cyberbots, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsMugenWest::
	setvar VAR_0x8005, 4
	setvar VAR_0x8004, 0
	lockall
	msgbox VsCabinet_Text_Mugen, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsMugenEast::
	setvar VAR_0x8005, 4
	setvar VAR_0x8004, 1
	lockall
	msgbox VsCabinet_Text_Mugen, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsAlpha3West::
	setvar VAR_0x8005, 5
	setvar VAR_0x8004, 0
	lockall
	msgbox VsCabinet_Text_Alpha3, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_GcVsAlpha3East::
	setvar VAR_0x8005, 5
	setvar VAR_0x8004, 1
	lockall
	msgbox VsCabinet_Text_Alpha3, MSGBOX_DEFAULT
	goto EventScript_VsCabinet
	end

EventScript_VsCabinet::
	special MP_Fight_CabinetReady
	goto_if_eq VAR_RESULT, 0, EventScript_VsCabinet_NoSet
	special MP_Fight_SeatCheck
	goto_if_eq VAR_RESULT, 1, EventScript_VsCabinet_Challenge
	goto_if_eq VAR_RESULT, 2, EventScript_VsCabinet_Taken
	goto_if_eq VAR_RESULT, 3, EventScript_VsCabinet_InUse
	goto_if_eq VAR_0x8005, 4, EventScript_VsCabinet_MugenMenu
	message VsCabinet_Text_Play
	waitmessage
	multichoice 18, 6, MULTI_ARCADE_CABINET, FALSE
	switch VAR_RESULT
	case 0, EventScript_VsCabinet_Wait
	case 1, EventScript_VsCabinet_Cpu
	releaseall
	end

EventScript_VsCabinet_MugenMenu::
	message VsCabinet_Text_Sit
	waitmessage
	multichoice 19, 6, MULTI_VS_CABINET, FALSE
	switch VAR_RESULT
	case 0, EventScript_VsCabinet_Wait
	case 1, EventScript_VsCabinet_Cpu
	releaseall
	end

EventScript_VsCabinet_NoSet::
	msgbox VsCabinet_Text_NoSet, MSGBOX_DEFAULT
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
	goto_if_ne VAR_0x8005, 4, EventScript_VsCabinet_PacksArcade
	msgbox VsCabinet_Text_Packs, MSGBOX_DEFAULT
	releaseall
	end

EventScript_VsCabinet_PacksArcade::
	msgbox VsCabinet_Text_PacksArcade, MSGBOX_DEFAULT
	releaseall
	end

VsCabinet_Text_Ssf2x:
	.string "SUPER STREET FIGHTER II X\n"
	.string "CAPCOM's arcade fighter, 1994.\p"
	.string "Sixteen world warriors, now with\n"
	.string "SUPER COMBOS.$"

VsCabinet_Text_Alpha3:
	.string "STREET FIGHTER ALPHA 3\n"
	.string "CAPCOM's arcade fighter, 1998.\p"
	.string "Pick a fighting style, then fight\n"
	.string "the biggest cast of the ALPHA games.$"

VsCabinet_Text_Vampire:
	.string "VAMPIRE SAVIOR\n"
	.string "CAPCOM's arcade fighter, 1997.\p"
	.string "Creatures of the night battle it out\n"
	.string "in a demon lord's world.$"

VsCabinet_Text_Cyberbots:
	.string "CYBERBOTS\n"
	.string "CAPCOM's arcade fighter, 1995.\p"
	.string "Pilots in giant robots slug it out,\n"
	.string "arms and legs flying.$"

VsCabinet_Text_Mugen:
	.string "A VS cabinet with no name on it.\n"
	.string "A weird bootleg.$"

VsCabinet_Text_Play:
	.string "START puts in a coin and starts.\n"
	.string "Hold START to stand up.$"

VsCabinet_Text_NoSet:
	.string "The screen is dark. This cabinet\n"
	.string "needs {STR_VAR_1}'s ROM set.\p"
	.string "Put {STR_VAR_2} in the\n"
	.string "arcade folder next to the game.$"

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

VsCabinet_Text_PacksArcade:
	.string "{STR_VAR_1} doesn't have this\n"
	.string "game's ROM set. No match…$"

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
