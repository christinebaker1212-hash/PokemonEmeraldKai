Johto_NationalParkBugContest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_NationalParkBugContest_EventScript_83E0755
	map_script MAP_SCRIPT_ON_RESUME, Johto_NationalParkBugContest_EventScript_83E0792
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_NationalParkBugContest_OnFrame_83E07AE
	.byte 0

Johto_NationalParkBugContest_OnFrame_83E07AE:
	map_script_2 VAR_QZ_0x5019, 1, Johto_NationalParkBugContest_EventScript_83E0878
	map_script_2 VAR_QZ_0x5019, 5, Johto_NationalParkBugContest_EventScript_83E0893
	map_script_2 VAR_QZ_0x5019, 3, Johto_NationalParkBugContest_EventScript_83E085D
	.2byte 0

Johto_NationalParkBugContest_EventScript_835FB5E::
	special QzRandomizeItemInVar8000_142
	return

Johto_NationalParkBugContest_EventScript_83C339A::
	lockall
	special QzBugContestClearFlag1_788
	msgbox Quetzal_Text_83C36A1, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C359B

Johto_NationalParkBugContest_EventScript_83C33D1::
	lockall
	special QzBugContestClearFlag1_788
	msgbox Johto_NationalParkBugContest_Text_83C3765, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C33FA

Johto_NationalParkBugContest_EventScript_83C33FA::
	special QzBugContestClearFlag1_788
	special QzBugContestGetCaughtCount_682
	goto_if_eq VAR_RESULT, 0, Johto_NationalParkBugContest_EventScript_83C341C
	fadescreen 1
	setvar VAR_QZ_0x5019, 3
	warp MAP_JOHTO_NATIONAL_PARK_BUG_CONTEST, 5, -1, -1
	waitstate
	end

Johto_NationalParkBugContest_EventScript_83C341C::
	msgbox Johto_NationalParkBugContest_Text_83C37DF, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C3570

Johto_NationalParkBugContest_EventScript_83C342B::
	msgbox Johto_NationalParkBugContest_Text_83C381A, MSGBOX_DEFAULT
	closemessage
	special QzBugContestJudge_683
	setvar VAR_0x8004, 3
	special QzBugContestBufferPlaceWinner_684
	msgbox Johto_NationalParkBugContest_Text_83C388E, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	setvar VAR_0x8004, 2
	special QzBugContestBufferPlaceWinner_684
	msgbox Johto_NationalParkBugContest_Text_83C38AD, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	setvar VAR_0x8004, 1
	special QzBugContestBufferPlaceWinner_684
	msgbox Johto_NationalParkBugContest_Text_83C38CD, MSGBOX_DEFAULT
	playfanfare MUS_OBTAIN_ITEM
	waitfanfare
	special QzBugContestGetPlayerPlace_685
	copyvar VAR_0x800A, VAR_RESULT
	copyvar VAR_0x8004, VAR_RESULT
	special QzBugContestBufferPlaceWinner_684
	goto_if_eq VAR_0x800A, 4, Johto_NationalParkBugContest_EventScript_83C349D
	msgbox Johto_NationalParkBugContest_Text_83C38ED, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C34A6

Johto_NationalParkBugContest_EventScript_83C349D::
	msgbox Johto_NationalParkBugContest_Text_83C391D, MSGBOX_DEFAULT
	closemessage

Johto_NationalParkBugContest_EventScript_83C34A6::
	special QzBugContestGetFlag2_687
	goto_if_eq VAR_RESULT, 1, Johto_NationalParkBugContest_EventScript_83C34C8
	call Johto_NationalParkBugContest_EventScript_83C3552
	msgbox Johto_NationalParkBugContest_Text_83C395A, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C34D1

Johto_NationalParkBugContest_EventScript_83C34C8::
	msgbox Johto_NationalParkBugContest_Text_83C39B1, MSGBOX_DEFAULT
	closemessage

Johto_NationalParkBugContest_EventScript_83C34D1::
	copyvar VAR_0x8000, VAR_0x8005
	specialvar VAR_RESULT, QzIsItemRandomizerOn_141
	call_if_eq VAR_RESULT, 1, Johto_NationalParkBugContest_EventScript_835FB5E
	copyvar VAR_0x800A, VAR_0x8000

Johto_NationalParkBugContest_EventScript_83C34EB::
	special QzBugContestCheckState_789
	goto_if_eq VAR_RESULT, 0, Johto_NationalParkBugContest_EventScript_83C352C

Johto_NationalParkBugContest_EventScript_83C34F9::
	copyvar VAR_0x8000, VAR_0x800A
	setvar VAR_0x8001, 1
	additem VAR_0x8000, VAR_0x8001
	copyvar VAR_0x8007, VAR_RESULT
	call Quetzal_EventScript_835FB62
	goto_if_eq VAR_RESULT, 0, Johto_NationalParkBugContest_EventScript_83C353F
	msgbox Johto_NationalParkBugContest_Text_83C3AC4, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C359B

Johto_NationalParkBugContest_EventScript_83C352C::
	msgbox Johto_NationalParkBugContest_Text_83C35EB, MSGBOX_DEFAULT
	closemessage
	special QzBugContestChooseFromBag_790
	waitstate
	goto Johto_NationalParkBugContest_EventScript_83C34EB

Johto_NationalParkBugContest_EventScript_83C353F::
	msgbox Johto_NationalParkBugContest_Text_83C3647, MSGBOX_DEFAULT
	closemessage
	special QzBugContestChooseFromBag_790
	waitstate
	goto Johto_NationalParkBugContest_EventScript_83C34F9

Johto_NationalParkBugContest_EventScript_83C3552::
	special QzBugContestReleaseCaught_688
	goto_if_eq VAR_RESULT, 0, Johto_NationalParkBugContest_EventScript_83C3561
	return

Johto_NationalParkBugContest_EventScript_83C3561::
	msgbox Johto_NationalParkBugContest_Text_83C3A2E, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C3552

Johto_NationalParkBugContest_EventScript_83C3570::
	special QzBugContestGetCaughtCount_682
	goto_if_eq VAR_RESULT, 0, Johto_NationalParkBugContest_EventScript_83C3592
	call Johto_NationalParkBugContest_EventScript_83C3552
	msgbox Johto_NationalParkBugContest_Text_83C395A, MSGBOX_DEFAULT
	closemessage
	goto Johto_NationalParkBugContest_EventScript_83C359B

Johto_NationalParkBugContest_EventScript_83C3592::
	msgbox Johto_NationalParkBugContest_Text_83C3A9A, MSGBOX_DEFAULT
	closemessage

Johto_NationalParkBugContest_EventScript_83C359B::
	special QzBugContestClearFlag1_788
	special QzBugContestGetFlag3_686
	copyvar VAR_0x800B, VAR_RESULT
	special QzBugContestEnd_681
	goto_if_eq VAR_RESULT, 0, Johto_NationalParkBugContest_EventScript_83C35CE
	setvar VAR_QZ_0x5019, 0
	goto_if_eq VAR_0x800B, 1, Johto_NationalParkBugContest_EventScript_83C35E1
	warp MAP_JOHTO_ROUTE35_NATIONAL_PARK_GATE, 255, 7, 2
	waitstate
	end

Johto_NationalParkBugContest_EventScript_83C35CE::
	msgbox Johto_NationalParkBugContest_Text_83C35EB, MSGBOX_DEFAULT
	closemessage
	special QzBugContestChooseFromBag_790
	waitstate
	goto Johto_NationalParkBugContest_EventScript_83C359B

Johto_NationalParkBugContest_EventScript_83C35E1::
	warp MAP_JOHTO_ROUTE35_NATIONAL_PARK_GATE, 255, 35, 7
	waitstate
	end

Johto_NationalParkBugContest_EventScript_83E0755::
	call Johto_NationalParkBugContest_EventScript_83E0798
	goto_if_eq VAR_QZ_0x5019, 3, Johto_NationalParkBugContest_EventScript_83E082C
	setobjectxyperm 3, 0xA, 0x31
	setobjectmovementtype 3, 0xA
	setobjectxyperm 5, 0xE, 0x1C
	setobjectmovementtype 5, 0
	setobjectxyperm 1, 0xB, 0xD
	setobjectmovementtype 1, 2
	setobjectxyperm 2, 0x1A, 9
	setobjectmovementtype 2, 2
	end

Johto_NationalParkBugContest_EventScript_83E0792::
	call Johto_NationalParkBugContest_EventScript_83E0798
	end

Johto_NationalParkBugContest_EventScript_83E0798::
	specialvar VAR_RESULT, QzIsNuzlockeActiveInRegion_779
	goto_if_ne VAR_RESULT, 1, Johto_NationalParkBugContest_EventScript_83E07AD
	setvar VAR_QZ_0x5019, 3

Johto_NationalParkBugContest_EventScript_83E07AD::
	return

@ Johto_NationalParkBugContest object 2 (local id 2, gfx 0x24) at 26,9
Johto_NationalParkBugContest_EventScript_83E07C8::
	lock
	faceplayer
	msgbox Johto_NationalParkBugContest_Text_83E09D6, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NationalParkBugContest object 1 (local id 1, gfx 0x24) at 11,13
Johto_NationalParkBugContest_EventScript_83E07D5::
	lock
	faceplayer
	msgbox Johto_NationalParkBugContest_Text_83E09A4, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NationalParkBugContest object 5 (local id 5, gfx 0x3E8F) at 14,28
Johto_NationalParkBugContest_EventScript_83E07E2::
	lock
	faceplayer
	msgbox Johto_NationalParkBugContest_Text_83E0978, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NationalParkBugContest object 4 (local id 4, gfx 0xB) at 29,27
Johto_NationalParkBugContest_EventScript_83E07EF::
	lock
	faceplayer
	msgbox Johto_NationalParkBugContest_Text_83E0A21, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_NationalParkBugContest_EventScript_83E082C::
	setobjectxyperm 3, 0x14, 0x1A
	setobjectmovementtype 3, 8
	setobjectxyperm 5, 0x12, 0x1C
	setobjectmovementtype 5, 7
	setobjectxyperm 2, 0x16, 0x1C
	setobjectmovementtype 2, 7
	setobjectxyperm 1, 0x18, 0x1C
	setobjectmovementtype 1, 7
	setobjectmovementtype LOCALID_PLAYER, 7
	end

Johto_NationalParkBugContest_EventScript_83E085D::
	lockall
	specialvar VAR_RESULT, QzIsNuzlockeActiveInRegion_779
	goto_if_eq VAR_RESULT, 1, Johto_NationalParkBugContest_EventScript_83C339A
	turnobject LOCALID_PLAYER, 2
	goto Johto_NationalParkBugContest_EventScript_83C342B

Johto_NationalParkBugContest_EventScript_83E0878::
	lockall
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	setvar VAR_QZ_0x5019, 2
	msgbox Johto_NationalParkBugContest_Text_83E092C, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

Johto_NationalParkBugContest_EventScript_83E0893::
	lockall
	turnobject LOCALID_PLAYER, 3
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	delay 0x14
	clearflag FLAG_QZ_0x1042
	addobject 6
	setvar VAR_TEMP_1, 1
	setvar VAR_QZ_0x5019, 2
	msgbox Johto_NationalParkBugContest_Text_83E092C, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

@ Johto_NationalParkBugContest trigger at 11,49 (VAR_TEMP_0 == 0)
@ Johto_NationalParkBugContest trigger at 12,49 (VAR_TEMP_0 == 0)
@ Johto_NationalParkBugContest trigger at 13,49 (VAR_TEMP_0 == 0)
@ Johto_NationalParkBugContest trigger at 14,49 (VAR_TEMP_0 == 0)
Johto_NationalParkBugContest_EventScript_83E08CA::
	lock
	turnobject 3, 4
	goto Johto_NationalParkBugContest_EventScript_83E08F4

@ Johto_NationalParkBugContest object 3 (local id 3, gfx 0xDF) at 10,49
@ Johto_NationalParkBugContest object 6 (local id 6, gfx 0xDF) at 40,19
Johto_NationalParkBugContest_EventScript_83E08D4::
	lock
	faceplayer
	msgbox Johto_NationalParkBugContest_Text_83E094A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_NationalParkBugContest_EventScript_83C33D1
	msgbox Johto_NationalParkBugContest_Text_83E092C, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_NationalParkBugContest_EventScript_83E08F4::
	msgbox Johto_NationalParkBugContest_Text_83E094A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_NationalParkBugContest_EventScript_83C33D1
	msgbox Johto_NationalParkBugContest_Text_83E092C, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	release
	end

Johto_NationalParkBugContest_Text_83C35EB::
	.string "We need room to return\n"
	.string "your Safari Balls.\p"
	.string "Make room in your Bag, then\n"
	.string "close it to continue.$"

Johto_NationalParkBugContest_Text_83C3647::
	.string "Your prize is still reserved.\p"
	.string "Make room in your Bag, then\n"
	.string "close it to receive your prize.$"

Johto_NationalParkBugContest_Text_83C3765::
	.string "You have chosen to end your run.\p"
	.string "It is time for the judging!$"

Johto_NationalParkBugContest_Text_83C37DF::
	.string "You did not catch a POKéMON, so\n"
	.string "there is nothing to judge.$"

Johto_NationalParkBugContest_Text_83C381A::
	.string "We will judge the best of the\n"
	.string "POKéMON you caught.\p"
	.string "The other contestants caught new\n"
	.string "POKéMON during this contest too.$"

Johto_NationalParkBugContest_Text_83C388E::
	.string "Third place: {STR_VAR_1}!\p"
	.string "{STR_VAR_2}\n"
	.string "Score: {STR_VAR_3}.$"

Johto_NationalParkBugContest_Text_83C38AD::
	.string "Second place: {STR_VAR_1}!\p"
	.string "{STR_VAR_2}\n"
	.string "Score: {STR_VAR_3}.$"

Johto_NationalParkBugContest_Text_83C38CD::
	.string "The winner is {STR_VAR_1}!\p"
	.string "{STR_VAR_2}\n"
	.string "Score: {STR_VAR_3}.$"

Johto_NationalParkBugContest_Text_83C38ED::
	.string "Your best catch was {STR_VAR_2}.\p"
	.string "Its final score was {STR_VAR_3}.$"

Johto_NationalParkBugContest_Text_83C391D::
	.string "Your best catch was {STR_VAR_2}.\p"
	.string "It placed fourth with a score of\n"
	.string "{STR_VAR_3}.$"

Johto_NationalParkBugContest_Text_83C395A::
	.string "All your catches have been\n"
	.string "stored in the PC.\p"
	.string "Your original party will now\n"
	.string "be returned.$"

Johto_NationalParkBugContest_Text_83C39B1::
	.string "Because Nuzlocke is active, this was\n"
	.string "an exhibition run.\p"
	.string "All contest captures were released,\n"
	.string "but you still earned your prize.$"

Johto_NationalParkBugContest_Text_83C3A2E::
	.string "Not all your catches could\n"
	.string "be stored in the PC.\p"
	.string "They are still reserved.\n"
	.string "Press A to try sending them again.$"

Johto_NationalParkBugContest_Text_83C3A9A::
	.string "Your original party will now be\n"
	.string "returned.$"

Johto_NationalParkBugContest_Text_83C3AC4::
	.string "Thank you for participating!\p"
	.string "You may enter again whenever you\n"
	.string "like.$"

Johto_NationalParkBugContest_Text_83E092C::
	.string "ATTENDANT: Enjoy the Contest!$"

Johto_NationalParkBugContest_Text_83E094A::
	.string "ATTENDANT: Would you like to end\n"
	.string "the contest?$"

Johto_NationalParkBugContest_Text_83E0978::
	.string "BELINDA: I’m going to win!\n"
	.string "Don’t bother me.$"

Johto_NationalParkBugContest_Text_83E09A4::
	.string "ED: My PARASECT puts POKéMON to\n"
	.string "sleep with SPORE.$"

Johto_NationalParkBugContest_Text_83E09D6::
	.string "SHAUN: I’m not concerned about\n"
	.string "winning.\p"
	.string "I’m just looking for rare POKéMON.$"

Johto_NationalParkBugContest_Text_83E0A21::
	.string "BARRY: You should weaken bug\n"
	.string "POKéMON first,\p"
	.string "then throw a BALL.$"
