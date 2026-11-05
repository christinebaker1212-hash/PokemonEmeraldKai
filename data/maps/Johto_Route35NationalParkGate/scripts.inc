Johto_Route35NationalParkGate_MapScripts::
	.byte 0

@ Johto_Route35NationalParkGate object 4 (local id 4, gfx 0xDF) at 3,2: ROUTE 35's contest officer
@ Moemon Kai: Crystal's (maps/Route35NationalParkGate.asm Route35OfficerScriptContest; audit case 97): TUESDAY,
@ THURSDAY and SATURDAY, once a day, the first POKeMON alone (src/qz_bugcontest.c)
Johto_Route35NationalParkGate_EventScript_83D13E2::
	lockall
	faceplayer
	setvar VAR_0x8009, 0
	goto Johto_Route35NationalParkGate_EventScript_ContestOfficer

@ Johto_Route35NationalParkGate object 3 (local id 3, gfx 0xDF) at 38,2: ROUTE 36's (Route36NationalParkGate.asm)
Johto_Route35NationalParkGate_EventScript_83D14F3::
	lockall
	faceplayer
	setvar VAR_0x8009, 1
	goto Johto_Route35NationalParkGate_EventScript_ContestOfficer

Johto_Route35NationalParkGate_EventScript_ContestOfficer::
	specialvar VAR_RESULT, QzBugContestDay
	goto_if_eq VAR_RESULT, 0, Johto_Route35NationalParkGate_EventScript_NoContestToday
	goto_if_eq VAR_RESULT, 2, Johto_Route35NationalParkGate_EventScript_ContestIsOver
	msgbox Johto_Route35NationalParkGate_Text_AskToParticipate, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_Route35NationalParkGate_EventScript_DeclinedToParticipate
	specialvar VAR_RESULT, QzBugContestCanEnter
	copyvar VAR_0x800A, VAR_RESULT
	specialvar VAR_RESULT, QzBugContestBufferLead
	goto_if_eq VAR_RESULT, 1, Johto_Route35NationalParkGate_EventScript_OnlyOneMon
	goto_if_eq VAR_0x800A, 2, Johto_Route35NationalParkGate_EventScript_NoRoomInBox
	goto_if_eq VAR_0x800A, 5, Johto_Route35NationalParkGate_EventScript_FirstMonIsEgg
	msgbox Johto_Route35NationalParkGate_Text_AskToUseFirstMon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_Route35NationalParkGate_EventScript_DeclinedToLeaveMonsBehind
	goto_if_eq VAR_0x800A, 1, Johto_Route35NationalParkGate_EventScript_FirstMonIsFainted
	msgbox Johto_Route35NationalParkGate_Text_WellHoldYourMon, MSGBOX_DEFAULT
	message Johto_Route35NationalParkGate_Text_PlayersMonLeftWithHelper
	waitmessage
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	waitbuttonpress
	goto Johto_Route35NationalParkGate_EventScript_OkayToProceed

Johto_Route35NationalParkGate_EventScript_OnlyOneMon::
	goto_if_ne VAR_0x800A, 0, Johto_Route35NationalParkGate_EventScript_FirstMonIsFainted
Johto_Route35NationalParkGate_EventScript_OkayToProceed::
	copyvar VAR_0x8004, VAR_0x8009
	special QzBugContestStart
	goto_if_eq VAR_RESULT, 1, Johto_Route35NationalParkGate_EventScript_FirstMonIsFainted
	goto_if_eq VAR_RESULT, 2, Johto_Route35NationalParkGate_EventScript_NoRoomInBox
	goto_if_eq VAR_RESULT, 3, Johto_Route35NationalParkGate_EventScript_NoRoomForBalls
	msgbox Johto_Route35NationalParkGate_Text_GiveParkBalls, MSGBOX_DEFAULT
	message Johto_Route35NationalParkGate_Text_PlayerReceivedParkBalls
	waitmessage
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	waitbuttonpress
	msgbox Johto_Route35NationalParkGate_Text_ExplainsRules, MSGBOX_DEFAULT
	closemessage
	goto_if_eq VAR_0x8009, 1, Johto_Route35NationalParkGate_EventScript_EnterFromRoute36
	call_if_eq VAR_FACING, DIR_WEST, Johto_Route35NationalParkGate_EventScript_83D14CF
	call_if_eq VAR_FACING, DIR_NORTH, Johto_Route35NationalParkGate_EventScript_83D14DA
	playse SE_EXIT
	clearflag FLAG_QZ_0x1042
	setvar VAR_QZ_0x5019, 1
	warp MAP_JOHTO_NATIONAL_PARK_BUG_CONTEST, 1, -1, -1
	waitstate
	end

Johto_Route35NationalParkGate_EventScript_EnterFromRoute36::
	call_if_eq VAR_FACING, DIR_EAST, Johto_Route35NationalParkGate_EventScript_83D15A6
	call_if_eq VAR_FACING, DIR_NORTH, Johto_Route35NationalParkGate_EventScript_83D15B1
	playse SE_EXIT
	setflag FLAG_QZ_0x1042
	setvar VAR_QZ_0x5019, 5
	warp MAP_JOHTO_NATIONAL_PARK_BUG_CONTEST, 255, 40, 19
	waitstate
	end

Johto_Route35NationalParkGate_EventScript_DeclinedToParticipate::
	msgbox Johto_Route35NationalParkGate_Text_TakePartInFuture, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_DeclinedToLeaveMonsBehind::
	msgbox Johto_Route35NationalParkGate_Text_ChooseMonAndComeBack, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_FirstMonIsFainted::
	msgbox Johto_Route35NationalParkGate_Text_FirstMonCantBattle, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_NoRoomInBox::
	msgbox Johto_Route35NationalParkGate_Text_MakeRoom, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_FirstMonIsEgg::
	msgbox Johto_Route35NationalParkGate_Text_EggAsFirstMon, MSGBOX_DEFAULT
	releaseall
	end

@ (the BAG's POKe BALLS pocket can't take the contest's SAFARI BALLS: this game's own case)
Johto_Route35NationalParkGate_EventScript_NoRoomForBalls::
	msgbox Johto_Route35NationalParkGate_Text_83D1CF5, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_NoContestToday::
	goto_if_eq VAR_0x8009, 1, Johto_Route35NationalParkGate_EventScript_SomeMonOnlyInPark
	msgbox Johto_Route35NationalParkGate_Text_WeHoldContests, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_SomeMonOnlyInPark::
	msgbox Johto_Route35NationalParkGate_Text_SomeMonOnlySeenInPark, MSGBOX_DEFAULT
	releaseall
	end

@ (ROUTE 36's officer gives the prize kept for today: Route36Officer_ContestHasConcluded)
Johto_Route35NationalParkGate_EventScript_ContestIsOver::
	goto_if_eq VAR_0x8009, 0, Johto_Route35NationalParkGate_EventScript_ContestIsOverText
	specialvar VAR_RESULT, QzBugContestHeldPrize
	goto_if_eq VAR_RESULT, 0, Johto_Route35NationalParkGate_EventScript_ContestIsOverText
	msgbox Johto_Route35NationalParkGate_Text_HeresThePrize, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	goto_if_eq VAR_RESULT, FALSE, Johto_Route35NationalParkGate_EventScript_StillHoldPrize
	special QzBugContestClearPrize
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_StillHoldPrize::
	msgbox Johto_Route35NationalParkGate_Text_WellHoldPrize, MSGBOX_DEFAULT
	releaseall
	end

Johto_Route35NationalParkGate_EventScript_ContestIsOverText::
	msgbox Johto_Route35NationalParkGate_Text_ContestIsOver, MSGBOX_DEFAULT
	releaseall
	end

@ (Crystal's BugCatchingContestExplanationSign, on the poster)
Johto_Route35NationalParkGate_EventScript_ContestSign::
	msgbox Johto_Route35NationalParkGate_Text_ContestExplanation, MSGBOX_SIGN
	end

Johto_Route35NationalParkGate_EventScript_83D14CF::
	applymovement LOCALID_PLAYER, Johto_Route35NationalParkGate_Movement_83D14E5
	waitmovement 0
	return

Johto_Route35NationalParkGate_EventScript_83D14DA::
	applymovement LOCALID_PLAYER, Johto_Route35NationalParkGate_Movement_83D14EC
	waitmovement 0
	return

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

Johto_Route35NationalParkGate_Text_83D1B41::
	.string "I love bird MOéMON! I found a\n"
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

Johto_Route35NationalParkGate_Text_AskToParticipate::
	.string "Today’s {STR_VAR_1}. That means\n"
	.string "the Bug-Catching Contest is on\l"
	.string "today.\p"
	.string "The rules are simple.\p"
	.string "Using one of your MOéMON, catch a\n"
	.string "bug MOéMON to be judged.\p"
	.string "Would you like to give it a try?$"

Johto_Route35NationalParkGate_Text_AskToUseFirstMon::
	.string "Uh-oh…\p"
	.string "You have more than one MOéMON.\p"
	.string "You’ll have to use {STR_VAR_1},\n"
	.string "the first MOéMON in your party.\p"
	.string "Is that OK with you?$"

Johto_Route35NationalParkGate_Text_WellHoldYourMon::
	.string "Fine, we’ll hold your other MOéMON\n"
	.string "while you compete.$"

Johto_Route35NationalParkGate_Text_PlayersMonLeftWithHelper::
	.string "{PLAYER}’s MOéMON were left with\n"
	.string "the CONTEST HELPER.$"

Johto_Route35NationalParkGate_Text_GiveParkBalls::
	.string "Here are the SAFARI BALLS for the\n"
	.string "Contest.$"

Johto_Route35NationalParkGate_Text_PlayerReceivedParkBalls::
	.string "{PLAYER} received 20 SAFARI BALLS.$"

Johto_Route35NationalParkGate_Text_ExplainsRules::
	.string "The person who gets the strongest\n"
	.string "bug MOéMON is the winner.\p"
	.string "You have 20 minutes.\p"
	.string "If you run out of SAFARI BALLS,\n"
	.string "you’re done.\p"
	.string "You can keep the last MOéMON you\n"
	.string "catch as your own.\p"
	.string "Go out and catch the strongest bug\n"
	.string "MOéMON you can find!$"

Johto_Route35NationalParkGate_Text_TakePartInFuture::
	.string "OK. We hope you’ll take part in\n"
	.string "the future.$"

Johto_Route35NationalParkGate_Text_ChooseMonAndComeBack::
	.string "Please choose the MOéMON to be\n"
	.string "used in the Contest, then come see\l"
	.string "me.$"

Johto_Route35NationalParkGate_Text_FirstMonCantBattle::
	.string "Uh-oh… The first MOéMON in your\n"
	.string "party can’t battle.\p"
	.string "Please switch it with the MOéMON\n"
	.string "you want to use, then come see me.$"

Johto_Route35NationalParkGate_Text_MakeRoom::
	.string "Uh-oh… Both your party and your PC\n"
	.string "BOX are full.\p"
	.string "You have no room to put the bug\n"
	.string "MOéMON you catch.\p"
	.string "Please make room in your party or\n"
	.string "your PC BOX, then come see me.$"

Johto_Route35NationalParkGate_Text_EggAsFirstMon::
	.string "Uh-oh… You have an EGG as the\n"
	.string "first MOéMON in your party.\p"
	.string "Please switch it with the MOéMON\n"
	.string "you want to use, then come see me.$"

Johto_Route35NationalParkGate_Text_WeHoldContests::
	.string "We hold Contests regularly in the\n"
	.string "PARK. You should give it a shot.$"

Johto_Route35NationalParkGate_Text_SomeMonOnlySeenInPark::
	.string "Some MOéMON can only be seen in\n"
	.string "the PARK.$"

Johto_Route35NationalParkGate_Text_HeresThePrize::
	.string "{PLAYER}?\p"
	.string "Here’s the prize we were holding\n"
	.string "for you.$"

Johto_Route35NationalParkGate_Text_WellHoldPrize::
	.string "Uh-oh… Your PACK is full.\p"
	.string "We’ll hold on to your prize, but\n"
	.string "only for today.\p"
	.string "Please make room, then come see\n"
	.string "me.$"

Johto_Route35NationalParkGate_Text_ContestIsOver::
	.string "Today’s Contest is over. We hope\n"
	.string "you will participate in the\l"
	.string "future.$"

Johto_Route35NationalParkGate_Text_ContestExplanation::
	.string "The Bug-Catching Contest is held\n"
	.string "on Tuesday, Thursday and Saturday.\p"
	.string "Not only do you earn a prize just\n"
	.string "for participating, you also get to\l"
	.string "keep the bug MOéMON you may have\l"
	.string "at the end of the contest.$"
