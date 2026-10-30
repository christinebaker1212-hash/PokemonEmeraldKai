Johto_Route35NationalParkGate_MapScripts::
	.byte 0

@ Johto_Route35NationalParkGate object 4 (local id 4, gfx 0xDF) at 3,2
Johto_Route35NationalParkGate_EventScript_83D13E2::
	lockall
	faceplayer
	specialvar VAR_RESULT, QzIsNuzlockeActiveInRegion
	goto_if_eq VAR_RESULT, 1, Johto_Route35NationalParkGate_EventScript_83D14B3
	msgbox Johto_Route35NationalParkGate_Text_83D15D0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route35NationalParkGate_EventScript_83D1411
	msgbox Johto_Route35NationalParkGate_Text_83D1929, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_83D1411::
	msgbox Johto_Route35NationalParkGate_Text_83D180F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_Route35NationalParkGate_EventScript_83D14BD
	setvar VAR_0x8004, 0
	special QzBugContestStart
	goto_if_eq VAR_RESULT, 4, Johto_Route35NationalParkGate_EventScript_83D14B3
	goto_if_eq VAR_RESULT, 1, Johto_Route35NationalParkGate_EventScript_83D1495
	goto_if_eq VAR_RESULT, 2, Johto_Route35NationalParkGate_EventScript_83D149F
	goto_if_eq VAR_RESULT, 3, Johto_Route35NationalParkGate_EventScript_83D14A9
	msgbox Johto_Route35NationalParkGate_Text_83D1679, MSGBOX_DEFAULT
	closemessage
	msgbox Johto_Route35NationalParkGate_Text_83D170D, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 3, Johto_Route35NationalParkGate_EventScript_83D14CF
	call_if_eq VAR_FACING, 2, Johto_Route35NationalParkGate_EventScript_83D14DA
	playse SE_EXIT
	clearflag FLAG_QZ_0x1042
	setvar VAR_QZ_0x5019, 1
	warp MAP_JOHTO_NATIONAL_PARK_BUG_CONTEST, 1, -1, -1
	waitstate
	end

Johto_Route35NationalParkGate_EventScript_83D1495::
	msgbox Johto_Route35NationalParkGate_Text_83D1955, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_83D149F::
	msgbox Johto_Route35NationalParkGate_Text_83D19CF, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_83D14A9::
	msgbox Johto_Route35NationalParkGate_Text_83D1CF5, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_83D14B3::
	msgbox Quetzal_Text_83C36A1, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_83D14BD::
	msgbox Johto_Route35NationalParkGate_Text_83D1929, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_83D14CF::
	applymovement LOCALID_PLAYER, Johto_Route35NationalParkGate_Movement_83D14E5
	waitmovement 0
	return

Johto_Route35NationalParkGate_EventScript_83D14DA::
	applymovement LOCALID_PLAYER, Johto_Route35NationalParkGate_Movement_83D14EC
	waitmovement 0
	return

@ Johto_Route35NationalParkGate object 3 (local id 3, gfx 0xDF) at 38,2
Johto_Route35NationalParkGate_EventScript_83D14F3::
	lockall
	faceplayer
	specialvar VAR_RESULT, QzIsNuzlockeActiveInRegion
	goto_if_eq VAR_RESULT, 1, Johto_Route35NationalParkGate_EventScript_83D14B3
	msgbox Johto_Route35NationalParkGate_Text_83D15D0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route35NationalParkGate_EventScript_83D1522
	msgbox Johto_Route35NationalParkGate_Text_83D1929, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_83D1522::
	msgbox Johto_Route35NationalParkGate_Text_83D180F, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_Route35NationalParkGate_EventScript_83D14BD
	setvar VAR_0x8004, 1
	special QzBugContestStart
	goto_if_eq VAR_RESULT, 4, Johto_Route35NationalParkGate_EventScript_83D14B3
	goto_if_eq VAR_RESULT, 1, Johto_Route35NationalParkGate_EventScript_83D1495
	goto_if_eq VAR_RESULT, 2, Johto_Route35NationalParkGate_EventScript_83D149F
	goto_if_eq VAR_RESULT, 3, Johto_Route35NationalParkGate_EventScript_83D14A9
	msgbox Johto_Route35NationalParkGate_Text_83D1679, MSGBOX_DEFAULT
	closemessage
	msgbox Johto_Route35NationalParkGate_Text_83D170D, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 4, Johto_Route35NationalParkGate_EventScript_83D15A6
	call_if_eq VAR_FACING, 2, Johto_Route35NationalParkGate_EventScript_83D15B1
	playse SE_EXIT
	setflag FLAG_QZ_0x1042
	setvar VAR_QZ_0x5019, 5
	warp MAP_JOHTO_NATIONAL_PARK_BUG_CONTEST, 255, 40, 19
	waitstate
	end

Johto_Route35NationalParkGate_EventScript_83D15A6::
	applymovement LOCALID_PLAYER, Johto_Route35NationalParkGate_Movement_83D15BC
	waitmovement 0
	return

Johto_Route35NationalParkGate_EventScript_83D15B1::
	applymovement LOCALID_PLAYER, Johto_Route35NationalParkGate_Movement_83D15C6
	waitmovement 0
	return

@ Johto_Route35NationalParkGate object 1 (local id 1, gfx 0x3EA9) at 1,6
@ Johto_Route35NationalParkGate object 2 (local id 2, gfx 0x3EA9) at 41,2
Johto_Route35NationalParkGate_EventScript_83D1B00::
	lock
	faceplayer
	msgbox Johto_Route35NationalParkGate_Text_83D1BC3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route35NationalParkGate object 6 (local id 6, gfx 0x24) at 14,9
Johto_Route35NationalParkGate_EventScript_83D1B0D::
	lock
	faceplayer
	msgbox Johto_Route35NationalParkGate_Text_83D1C06, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route35NationalParkGate object 7 (local id 7, gfx 0x24) at 39,11
Johto_Route35NationalParkGate_EventScript_83D1B1A::
	lock
	faceplayer
	msgbox Johto_Route35NationalParkGate_Text_83D1B87, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route35NationalParkGate object 5 (local id 5, gfx 0x3EAE) at 11,4
Johto_Route35NationalParkGate_EventScript_83D1B27::
	lock
	faceplayer
	msgbox Johto_Route35NationalParkGate_Text_83D1B41, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route35NationalParkGate object 8 (local id 8, gfx 0x2D) at 36,11
Johto_Route35NationalParkGate_EventScript_83D1B34::
	lock
	faceplayer
	msgbox Johto_Route35NationalParkGate_Text_83D1BA4, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route35NationalParkGate_Movement_83D14E5:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	step_end

Johto_Route35NationalParkGate_Movement_83D14EC:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	step_end

Johto_Route35NationalParkGate_Movement_83D15BC:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_in_place_left
	step_end

Johto_Route35NationalParkGate_Movement_83D15C6:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_left
	step_end

Johto_Route35NationalParkGate_Text_83D15D0::
	.string "Hello Trainer! The Bug-Catching\n"
	.string "Contest is on today.\p"
	.string "The rules are simple.\p"
	.string "Using one of your POKéMON, catch a\n"
	.string "bug POKéMON to be judged.\p"
	.string "Would you like to give it a try?$"

Johto_Route35NationalParkGate_Text_83D1679::
	.string "Any EXP your POKéMON gains during\n"
	.string "the contest will be reverted when\l"
	.string "you leave.\p"
	.string "Here are the SAFARI BALLS for the\n"
	.string "Contest.$"

Johto_Route35NationalParkGate_Text_83D170D::
	.string "You have eight minutes and thirty\n"
	.string "SAFARI BALLS.\p"
	.string "You may catch up to five POKéMON.\n"
	.string "Your highest-scoring catch will be\l"
	.string "entered automatically.\p"
	.string "You keep every catch, even if\n"
	.string "you are eliminated.\p"
	.string "Level, IVs, rarity, catch rate,\n"
	.string "shiny and radiant all affect score.$"

Johto_Route35NationalParkGate_Text_83D180F::
	.string "If you have more than one POKéMON,\n"
	.string "you’ll have to use the\l"
	.string "first POKéMON in your party.\p"
	.string "Is that OK with you?$"

Johto_Route35NationalParkGate_Text_83D1929::
	.string "OK. We hope you’ll take part in the\n"
	.string "future.$"

Johto_Route35NationalParkGate_Text_83D1955::
	.string "Uh-oh… The first POKéMON in your\n"
	.string "party can’t battle.\p"
	.string "Please switch it with the POKéMON\n"
	.string "you want to use, then come see me.$"

Johto_Route35NationalParkGate_Text_83D19CF::
	.string "Make five spaces in your PC boxes,\n"
	.string "then come back to enter.\p"
	.string "Egg storage and empty party\n"
	.string "slots do not count.$"

Johto_Route35NationalParkGate_Text_83D1B41::
	.string "I love bird POKéMON! I found a\n"
	.string "DARK-type bird in the park\l"
	.string "last night!$"

Johto_Route35NationalParkGate_Text_83D1B87::
	.string "I caught a whopping SCYTHER!$"

Johto_Route35NationalParkGate_Text_83D1BA4::
	.string "He always wins these contests!$"

Johto_Route35NationalParkGate_Text_83D1BC3::
	.string "We hold Contests regularly in the\n"
	.string "PARK. You should give it a shot.$"

Johto_Route35NationalParkGate_Text_83D1C06::
	.string "When is the next Bug-Catching Contest\n"
	.string "going to be?$"

Johto_Route35NationalParkGate_Text_83D1CF5::
	.string "There is no room in the BALLS\n"
	.string "pocket for the contest supply.$"
