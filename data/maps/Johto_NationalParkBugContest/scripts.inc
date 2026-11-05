@ Moemon Kai: the NATIONAL PARK during Crystal's Bug-Catching Contest (audit case 97; maps/NationalParkBugContest.asm,
@ engine/events/std_scripts.asm BugContestResultsScript; the rules in src/qz_bugcontest.c). Crystal's ten
@ contestants are objects 29-38, today's five shown (QzBugContestSetupPark); the judging is here, the officer
@ (object 3) facing the player and the five in a line.
Johto_NationalParkBugContest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_NationalParkBugContest_EventScript_83E0755
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_NationalParkBugContest_OnFrame_83E07AE
	.byte 0

Johto_NationalParkBugContest_OnFrame_83E07AE:
	map_script_2 VAR_QZ_0x5019, 1, Johto_NationalParkBugContest_EventScript_83E0878
	map_script_2 VAR_QZ_0x5019, 5, Johto_NationalParkBugContest_EventScript_83E0893
	map_script_2 VAR_QZ_0x5019, 3, Johto_NationalParkBugContest_EventScript_83E085D
	.2byte 0

Johto_NationalParkBugContest_EventScript_83E0755::
	special QzBugContestSetupPark
	goto_if_eq VAR_QZ_0x5019, 3, Johto_NationalParkBugContest_EventScript_83E082C
	setobjectxyperm 3, 10, 49
	setobjectmovementtype 3, MOVEMENT_TYPE_FACE_RIGHT
	end

Johto_NationalParkBugContest_EventScript_83E082C::
	setobjectxyperm 3, 20, 26
	setobjectmovementtype 3, MOVEMENT_TYPE_FACE_DOWN
	setobjectmovementtype LOCALID_PLAYER, MOVEMENT_TYPE_FACE_UP
	end

@ in from ROUTE 35's gate (south) / ROUTE 36's (east)
Johto_NationalParkBugContest_EventScript_83E0878::
	lockall
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	setvar VAR_QZ_0x5019, 2
	releaseall
	end

Johto_NationalParkBugContest_EventScript_83E0893::
	lockall
	turnobject LOCALID_PLAYER, DIR_WEST
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	delay 20
	clearflag FLAG_QZ_0x1042
	addobject 6
	setvar VAR_TEMP_1, 1
	setvar VAR_QZ_0x5019, 2
	releaseall
	end

@ Leaving early: Crystal's gate officers (Route35NationalParkGateLeavingContestEarlyScript and ROUTE 36's), here
@ the officers at the park's two ways out. VAR_0x8009: 0 the south (ROUTE 35) one, 1 the east (ROUTE 36) one
@ Johto_NationalParkBugContest trigger at 11..14,49 (VAR_TEMP_0 == 0)
Johto_NationalParkBugContest_EventScript_83E08CA::
	lock
	turnobject 3, DIR_EAST
	setvar VAR_0x800A, 1
	goto Johto_NationalParkBugContest_EventScript_LeaveEarly

@ Johto_NationalParkBugContest object 3 (local id 3) at 10,49 and object 6 (local id 6) at 40,19
Johto_NationalParkBugContest_EventScript_83E08D4::
	lock
	faceplayer
	setvar VAR_0x800A, 0
Johto_NationalParkBugContest_EventScript_LeaveEarly::
	getplayerxy VAR_0x8008, VAR_0x8009
	setvar VAR_0x8009, 0
	call_if_gt VAR_0x8008, 30, Johto_NationalParkBugContest_EventScript_EastSide
	special QzBugContestMinutesLeft
	msgbox Johto_NationalParkBugContest_Text_WantToFinish, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_NationalParkBugContest_EventScript_GoBackToContest
	call_if_eq VAR_0x8009, 0, Johto_NationalParkBugContest_EventScript_WaitAtNorthGate
	call_if_eq VAR_0x8009, 1, Johto_NationalParkBugContest_EventScript_WaitHere
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C33FA

Johto_NationalParkBugContest_EventScript_EastSide::
	setvar VAR_0x8009, 1
	return

Johto_NationalParkBugContest_EventScript_WaitAtNorthGate::
	msgbox Johto_NationalParkBugContest_Text_WaitAtNorthGate, MSGBOX_DEFAULT
	return

Johto_NationalParkBugContest_EventScript_WaitHere::
	msgbox Johto_NationalParkBugContest_Text_WaitHereForAnnouncement, MSGBOX_DEFAULT
	return

Johto_NationalParkBugContest_EventScript_GoBackToContest::
	call_if_eq VAR_0x8009, 0, Johto_NationalParkBugContest_EventScript_OkGoFinish35
	call_if_eq VAR_0x8009, 1, Johto_NationalParkBugContest_EventScript_OkGoFinish36
	closemessage
	goto_if_eq VAR_0x800A, 0, Johto_NationalParkBugContest_EventScript_Release
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
Johto_NationalParkBugContest_EventScript_Release::
	release
	end

Johto_NationalParkBugContest_EventScript_OkGoFinish35::
	msgbox Johto_NationalParkBugContest_Text_OkGoFinish35, MSGBOX_DEFAULT
	return

Johto_NationalParkBugContest_EventScript_OkGoFinish36::
	msgbox Johto_NationalParkBugContest_Text_OkGoFinish36, MSGBOX_DEFAULT
	return

@ To the judging (Crystal's BugContestResultsWarpScript): the time up, the last SAFARI BALL (mp_region.inc), or
@ finishing early
Johto_NationalParkBugContest_EventScript_83C33FA::
	fadescreen FADE_TO_BLACK
	setvar VAR_QZ_0x5019, 3
	warp MAP_JOHTO_NATIONAL_PARK_BUG_CONTEST, 5, -1, -1
	waitstate
	end

@ Crystal's BugContestResultsScript
Johto_NationalParkBugContest_EventScript_83E085D::
	lockall
	turnobject LOCALID_PLAYER, DIR_NORTH
	msgbox Johto_NationalParkBugContest_Text_ReadyToJudge, MSGBOX_DEFAULT
	special QzBugContestJudge
	setvar VAR_0x8004, 3
	special QzBugContestBufferPlaceWinner
	message Johto_NationalParkBugContest_Text_ThirdPlace
	waitmessage
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	waitbuttonpress
	msgbox Johto_NationalParkBugContest_Text_ThirdPlaceScore, MSGBOX_DEFAULT
	setvar VAR_0x8004, 2
	special QzBugContestBufferPlaceWinner
	message Johto_NationalParkBugContest_Text_SecondPlace
	waitmessage
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	waitbuttonpress
	msgbox Johto_NationalParkBugContest_Text_SecondPlaceScore, MSGBOX_DEFAULT
	setvar VAR_0x8004, 1
	special QzBugContestBufferPlaceWinner
	message Johto_NationalParkBugContest_Text_FirstPlace
	waitmessage
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	waitbuttonpress
	msgbox Johto_NationalParkBugContest_Text_FirstPlaceScore, MSGBOX_DEFAULT
	special QzBugContestGetPlayerPlace
	goto_if_eq VAR_RESULT, 4, Johto_NationalParkBugContest_EventScript_Consolation
	msgbox Johto_NationalParkBugContest_Text_PlayerWonAPrize, MSGBOX_DEFAULT
	giveitem VAR_0x8005
	call_if_eq VAR_RESULT, FALSE, Johto_NationalParkBugContest_EventScript_PrizeNoRoom
	msgbox Johto_NationalParkBugContest_Text_JoinUsNextTime, MSGBOX_DEFAULT
	goto Johto_NationalParkBugContest_EventScript_FinishUp

Johto_NationalParkBugContest_EventScript_Consolation::
	msgbox Johto_NationalParkBugContest_Text_ConsolationPrize, MSGBOX_DEFAULT
	setvar VAR_0x8005, ITEM_ORAN_BERRY
	giveitem ITEM_ORAN_BERRY
	call_if_eq VAR_RESULT, FALSE, Johto_NationalParkBugContest_EventScript_PrizeNoRoom
	msgbox Johto_NationalParkBugContest_Text_DidNotWin, MSGBOX_DEFAULT
	goto Johto_NationalParkBugContest_EventScript_FinishUp

@ (the officer keeps it for today: ROUTE 36's gate officer gives it)
Johto_NationalParkBugContest_EventScript_PrizeNoRoom::
	msgbox Johto_NationalParkBugContest_Text_PrizeNoRoom, MSGBOX_DEFAULT
	special QzBugContestHoldPrize
	return

Johto_NationalParkBugContest_EventScript_FinishUp::
	specialvar VAR_RESULT, QzBugContestGetFlag2
	call_if_eq VAR_RESULT, 1, Johto_NationalParkBugContest_EventScript_ReturnParty
Johto_NationalParkBugContest_EventScript_83C359B::
	special QzBugContestGetFlag3
	copyvar VAR_0x800B, VAR_RESULT
	special QzBugContestEnd
	goto_if_eq VAR_RESULT, 0, Johto_NationalParkBugContest_EventScript_83C35CE
	call_if_eq VAR_RESULT, 2, Johto_NationalParkBugContest_EventScript_PartyFull
	closemessage
	setvar VAR_QZ_0x5019, 0
	goto_if_eq VAR_0x800B, 1, Johto_NationalParkBugContest_EventScript_83C35E1
	warp MAP_JOHTO_ROUTE35_NATIONAL_PARK_GATE, 255, 7, 2
	waitstate
	end

Johto_NationalParkBugContest_EventScript_ReturnParty::
	msgbox Johto_NationalParkBugContest_Text_ReturnParty, MSGBOX_DEFAULT
	return

Johto_NationalParkBugContest_EventScript_PartyFull::
	msgbox Johto_NationalParkBugContest_Text_PartyFull, MSGBOX_DEFAULT
	return

@ (the BAG can't take the player's own SAFARI BALLS back: this game's own case)
Johto_NationalParkBugContest_EventScript_83C35CE::
	msgbox Johto_NationalParkBugContest_Text_83C35EB, MSGBOX_DEFAULT
	closemessage
	special QzBugContestChooseFromBag
	waitstate
	goto Johto_NationalParkBugContest_EventScript_83C359B

Johto_NationalParkBugContest_EventScript_83C35E1::
	warp MAP_JOHTO_ROUTE35_NATIONAL_PARK_GATE, 255, 35, 7
	waitstate
	end

@ Quetzal's four people (objects 1, 2, 4, 5: FLAG_KAI_GONE)
Johto_NationalParkBugContest_EventScript_83E07C8::
Johto_NationalParkBugContest_EventScript_83E07D5::
Johto_NationalParkBugContest_EventScript_83E07E2::
Johto_NationalParkBugContest_EventScript_83E07EF::
	end

@ Crystal's contestants (BugCatchingContestant1AScript...)
Johto_NationalParkBugContest_EventScript_Contestant1::
	msgbox Johto_NationalParkBugContest_Text_Contestant1, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant2::
	msgbox Johto_NationalParkBugContest_Text_Contestant2, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant3::
	msgbox Johto_NationalParkBugContest_Text_Contestant3, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant4::
	msgbox Johto_NationalParkBugContest_Text_Contestant4, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant5::
	msgbox Johto_NationalParkBugContest_Text_Contestant5, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant6::
	msgbox Johto_NationalParkBugContest_Text_Contestant6, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant7::
	msgbox Johto_NationalParkBugContest_Text_Contestant7, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant8::
	msgbox Johto_NationalParkBugContest_Text_Contestant8, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant9::
	msgbox Johto_NationalParkBugContest_Text_Contestant9, MSGBOX_NPC
	end

Johto_NationalParkBugContest_EventScript_Contestant10::
	msgbox Johto_NationalParkBugContest_Text_Contestant10, MSGBOX_NPC
	end

Johto_NationalParkBugContest_Text_83C35EB::
	.string "We need room to return\n"
	.string "your Safari Balls.\p"
	.string "Make room in your Bag, then\n"
	.string "close it to continue.$"

Johto_NationalParkBugContest_Text_WantToFinish::
	.string "You still have {STR_VAR_1}\n"
	.string "minute(s) left.\p"
	.string "Do you want to finish now?$"

Johto_NationalParkBugContest_Text_WaitAtNorthGate::
	.string "OK. Please wait at the North Gate\n"
	.string "for the announcement of the\l"
	.string "winners.$"

Johto_NationalParkBugContest_Text_WaitHereForAnnouncement::
	.string "OK. Please wait here for the\n"
	.string "announcement of the winners.$"

Johto_NationalParkBugContest_Text_OkGoFinish35::
	.string "OK. Please get back outside and\n"
	.string "finish up.$"

Johto_NationalParkBugContest_Text_OkGoFinish36::
	.string "OK. Please go back outside and\n"
	.string "finish up.$"

Johto_NationalParkBugContest_Text_ReadyToJudge::
	.string "We will now judge the MOéMON\n"
	.string "you’ve caught.\p"
	.string "…… ……\p"
	.string "We have chosen the winners!\p"
	.string "Are you ready for this?$"

Johto_NationalParkBugContest_Text_ThirdPlace::
	.string "Placing third was {STR_VAR_1}, who\n"
	.string "caught a {STR_VAR_2}!$"

Johto_NationalParkBugContest_Text_ThirdPlaceScore::
	.string "The score was {STR_VAR_3} points!$"

Johto_NationalParkBugContest_Text_SecondPlace::
	.string "Placing second was {STR_VAR_1},\n"
	.string "who caught a {STR_VAR_2}!$"

Johto_NationalParkBugContest_Text_SecondPlaceScore::
	.string "The score was {STR_VAR_3} points!$"

Johto_NationalParkBugContest_Text_FirstPlace::
	.string "This Bug-Catching Contest winner\n"
	.string "is…\p"
	.string "{STR_VAR_1}, who caught a\n"
	.string "{STR_VAR_2}!$"

Johto_NationalParkBugContest_Text_FirstPlaceScore::
	.string "The winning score was {STR_VAR_3}\n"
	.string "points!$"

Johto_NationalParkBugContest_Text_PlayerWonAPrize::
	.string "{PLAYER}, the No.{STR_VAR_1}\n"
	.string "finisher, wins {STR_VAR_2}!$"

Johto_NationalParkBugContest_Text_JoinUsNextTime::
	.string "Please join us for the next\n"
	.string "Contest!$"

Johto_NationalParkBugContest_Text_ConsolationPrize::
	.string "Everyone else gets a BERRY as a\n"
	.string "consolation prize!$"

Johto_NationalParkBugContest_Text_DidNotWin::
	.string "We hope you do better next time.$"

Johto_NationalParkBugContest_Text_PrizeNoRoom::
	.string "Oh? Your PACK is full.\p"
	.string "We’ll keep this for you today, so\n"
	.string "come back when you make room for\l"
	.string "it.$"

Johto_NationalParkBugContest_Text_ReturnParty::
	.string "We’ll return the MOéMON we kept\n"
	.string "for you. Here you go!$"

Johto_NationalParkBugContest_Text_PartyFull::
	.string "Your party’s full, so the MOéMON\n"
	.string "was sent to your BOX in BILL’s PC.$"

Johto_NationalParkBugContest_Text_Contestant1::
	.string "DON: I’m going to win! Don’t\n"
	.string "bother me.$"

Johto_NationalParkBugContest_Text_Contestant2::
	.string "ED: My PARASECT puts MOéMON to\n"
	.string "sleep with SPORE.$"

Johto_NationalParkBugContest_Text_Contestant3::
	.string "NICK: I’m raising fast MOéMON for\n"
	.string "battles.$"

Johto_NationalParkBugContest_Text_Contestant4::
	.string "WILLIAM: I’m not concerned about\n"
	.string "winning.\p"
	.string "I’m just looking for rare MOéMON.$"

Johto_NationalParkBugContest_Text_Contestant5::
	.string "BENNY: Ssh! You’ll scare off\n"
	.string "SCYTHER.\p"
	.string "I’ll talk to you later.$"

Johto_NationalParkBugContest_Text_Contestant6::
	.string "BARRY: You should weaken bug\n"
	.string "MOéMON first, then throw a BALL.$"

Johto_NationalParkBugContest_Text_Contestant7::
	.string "CINDY: I love bug MOéMON.\p"
	.string "I guess you must like them too.$"

Johto_NationalParkBugContest_Text_Contestant8::
	.string "JOSH: I’ve been collecting bug\n"
	.string "MOéMON since I was just a baby.\p"
	.string "There’s no way I’m going to lose!$"

Johto_NationalParkBugContest_Text_Contestant9::
	.string "SAMUEL: If you’ve got the time to\n"
	.string "chat, go find some bug MOéMON.$"

Johto_NationalParkBugContest_Text_Contestant10::
	.string "KIPP: I’ve studied about bug\n"
	.string "MOéMON a lot.\p"
	.string "I’m going to win for sure.$"
