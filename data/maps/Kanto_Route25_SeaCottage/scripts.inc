Kanto_Route25_SeaCottage_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route25_SeaCottage_EventScript_8420653
	.byte 0

Kanto_Route25_SeaCottage_EventScript_8420653::
	call_if_unset FLAG_QZ_0x2177, Kanto_Route25_SeaCottage_EventScript_842066B
	call_if_set FLAG_QZ_0x2162, Kanto_Route25_SeaCottage_EventScript_8420676
	call Quetzal_EventScript_840FBB5
	end

Kanto_Route25_SeaCottage_EventScript_842066B::
	clearflag FLAG_QZ_0x2179
	setobjectxyperm 1, 3, 3
	return

Kanto_Route25_SeaCottage_EventScript_8420676::
	setflag FLAG_TEMP_3
	return

@ Kanto_Route25_SeaCottage object 1 (local id 1, gfx 0x3EEA) at 7,5
@ Kanto_Route25_SeaCottage object 2 (local id 2, gfx 0x3EF9) at 10,6
Kanto_Route25_SeaCottage_EventScript_842067A::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_3, Kanto_Route25_SeaCottage_EventScript_8420805
	goto_if_set FLAG_QZ_0x2162, Kanto_Route25_SeaCottage_EventScript_842076F
	goto_if_set FLAG_QZ_0x2177, Kanto_Route25_SeaCottage_EventScript_8420779
	checkplayergender
	goto_if_eq VAR_RESULT, 0, Kanto_Route25_SeaCottage_EventScript_84206AF
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_SeaCottage_EventScript_84206BD
	end

Kanto_Route25_SeaCottage_EventScript_84206AF::
	msgbox Kanto_Route25_SeaCottage_Text_84502AA, MSGBOX_YESNO
	goto Kanto_Route25_SeaCottage_EventScript_84206CB

Kanto_Route25_SeaCottage_EventScript_84206BD::
	msgbox Kanto_Route25_SeaCottage_Text_8450396, MSGBOX_YESNO
	goto Kanto_Route25_SeaCottage_EventScript_84206CB

Kanto_Route25_SeaCottage_EventScript_84206CB::
	call_if_eq VAR_RESULT, 0, Kanto_Route25_SeaCottage_EventScript_842072F
	msgbox Kanto_Route25_SeaCottage_Text_8450483, MSGBOX_DEFAULT
	closemessage
	delay 0xA
	call_if_eq VAR_FACING, 1, Kanto_Route25_SeaCottage_EventScript_8420759
	call_if_eq VAR_FACING, 2, Kanto_Route25_SeaCottage_EventScript_8420764
	call_if_eq VAR_FACING, 3, Kanto_Route25_SeaCottage_EventScript_8420764
	call_if_eq VAR_FACING, 4, Kanto_Route25_SeaCottage_EventScript_8420764
	opendoor 0xA, 3
	waitdooranim
	applymovement 2, Kanto_Route25_SeaCottage_Movement_8420818
	waitmovement 0
	removeobject 2
	playse SE_CONTEST_CURTAIN_FALL
	closedoor 0xA, 3
	waitdooranim
	setflag FLAG_TEMP_2
	release
	end

Kanto_Route25_SeaCottage_EventScript_842072F::
	checkplayergender
	call_if_eq VAR_RESULT, 0, Kanto_Route25_SeaCottage_EventScript_8420747
	call_if_eq VAR_RESULT, 1, Kanto_Route25_SeaCottage_EventScript_8420750
	return

Kanto_Route25_SeaCottage_EventScript_8420747::
	msgbox Kanto_Route25_SeaCottage_Text_84504E5, MSGBOX_DEFAULT
	return

Kanto_Route25_SeaCottage_EventScript_8420750::
	msgbox Kanto_Route25_SeaCottage_Text_8450566, MSGBOX_DEFAULT
	return

Kanto_Route25_SeaCottage_EventScript_8420759::
	applymovement 2, Kanto_Route25_SeaCottage_Movement_8420812
	waitmovement 0
	return

Kanto_Route25_SeaCottage_EventScript_8420764::
	applymovement 2, Kanto_Route25_SeaCottage_Movement_842080F
	waitmovement 0
	return

Kanto_Route25_SeaCottage_EventScript_842076F::
	msgbox Kanto_Route25_SeaCottage_Text_845077F, MSGBOX_DEFAULT
	release
	end

Kanto_Route25_SeaCottage_EventScript_8420779::
	setvar VAR_0x8004, 0xD
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	checkplayergender
	call_if_eq VAR_RESULT, 0, Kanto_Route25_SeaCottage_EventScript_84207E9
	call_if_eq VAR_RESULT, 1, Kanto_Route25_SeaCottage_EventScript_84207F2
	checkitem ITEM_SS_TICKET, 1
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_SeaCottage_EventScript_84207D7
	additem ITEM_SS_TICKET, 1
	goto_if_eq VAR_RESULT, 0, Kanto_Route25_SeaCottage_EventScript_84207FB
	loadword 0, Kanto_Route25_SeaCottage_Text_845073E
	setorcopyvar VAR_0x8000, 0x2D7
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E

Kanto_Route25_SeaCottage_EventScript_84207D7::
	setflag FLAG_QZ_0x2163
	setflag FLAG_QZ_0x21A0
	setflag FLAG_QZ_0x2162
	setflag FLAG_QZ_0x220F
	goto Kanto_Route25_SeaCottage_EventScript_842076F

Kanto_Route25_SeaCottage_EventScript_84207E9::
	msgbox Kanto_Route25_SeaCottage_Text_84505EB, MSGBOX_DEFAULT
	return

Kanto_Route25_SeaCottage_EventScript_84207F2::
	msgbox Kanto_Route25_SeaCottage_Text_8450694, MSGBOX_DEFAULT
	return

Kanto_Route25_SeaCottage_EventScript_84207FB::
	msgbox Kanto_Route25_SeaCottage_Text_8450764, MSGBOX_DEFAULT
	release
	end

Kanto_Route25_SeaCottage_EventScript_8420805::
	msgbox Kanto_Route25_SeaCottage_Text_845085B, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route25_SeaCottage sign / bg event at 4,5
Kanto_Route25_SeaCottage_EventScript_842081A::
	lockall
	goto_if_set FLAG_TEMP_3, Kanto_Route25_SeaCottage_EventScript_8420911
	goto_if_set FLAG_TEMP_2, Kanto_Route25_SeaCottage_EventScript_8420837
	msgbox Kanto_Route25_SeaCottage_Text_84508B2, MSGBOX_DEFAULT
	releaseall
	end

Kanto_Route25_SeaCottage_EventScript_8420837::
	fadeoutbgm 0
	msgbox Kanto_Route25_SeaCottage_Text_84508DD, MSGBOX_DEFAULT
	closemessage
	clearflag FLAG_TEMP_2
	setflag FLAG_QZ_0x2177
	special SpawnCameraObject
	applymovement LOCALID_CAMERA, Kanto_Route25_SeaCottage_Movement_8420905
	waitmovement 0
	delay 0x23
	playse SE_PIN
	waitse
	setvar VAR_0x8004, 0
	special QzAnimateTeleporterHousing
	delay 0xA
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	gotonative 0x82A22B5
	setvar VAR_0x8004, 1
	special QzAnimateTeleporterHousing
	playse SE_WARP_IN
	special QzAnimateTeleporterCable
	waitse
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	call Kanto_Route25_SeaCottage_EventScript_84208FD
	gotonative 0x82A22B5
	opendoor 3, 3
	waitdooranim
	fadeinbgm 0
	addobject 1
	clearflag FLAG_QZ_0x217A
	delay 0x32
	playfanfare MUS_LEVEL_UP
	applymovement 1, Kanto_Route25_SeaCottage_Movement_84209D0
	waitmovement 0
	waitfanfare
	applymovement LOCALID_CAMERA, Kanto_Route25_SeaCottage_Movement_842090A
	waitmovement 0
	playse SE_CONTEST_CURTAIN_FALL
	closedoor 3, 3
	waitdooranim
	applymovement 1, Kanto_Route25_SeaCottage_Movement_84209C9
	waitmovement 0
	special RemoveCameraObject
	releaseall
	end

Kanto_Route25_SeaCottage_EventScript_84208FD::
	playse SE_BALL
	waitse
	delay 0x14
	return

Kanto_Route25_SeaCottage_EventScript_8420911::
	msgbox Kanto_Route25_SeaCottage_Text_845090B, MSGBOX_DEFAULT
	goto Kanto_Route25_SeaCottage_EventScript_842091F

Kanto_Route25_SeaCottage_EventScript_842091F::
	message Kanto_Route25_SeaCottage_Text_8450929
	waitmessage
	multichoice 0, 0, 0x5C, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_Route25_SeaCottage_EventScript_8420973
	goto_if_eq VAR_0x8000, 1, Kanto_Route25_SeaCottage_EventScript_8420988
	goto_if_eq VAR_0x8000, 2, Kanto_Route25_SeaCottage_EventScript_842099D
	goto_if_eq VAR_0x8000, 3, Kanto_Route25_SeaCottage_EventScript_84209B2
	goto_if_eq VAR_0x8000, 4, Kanto_Route25_SeaCottage_EventScript_84209C7
	goto_if_eq VAR_0x8000, 127, Kanto_Route25_SeaCottage_EventScript_84209C7
	end

Kanto_Route25_SeaCottage_EventScript_8420973::
	showmonpic SPECIES_EEVEE, 0xA, 3
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, SPECIES_EEVEE
	special QzSetSeenMon
	goto Kanto_Route25_SeaCottage_EventScript_842091F

Kanto_Route25_SeaCottage_EventScript_8420988::
	showmonpic SPECIES_FLAREON, 0xA, 3
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, SPECIES_FLAREON
	special QzSetSeenMon
	goto Kanto_Route25_SeaCottage_EventScript_842091F

Kanto_Route25_SeaCottage_EventScript_842099D::
	showmonpic SPECIES_JOLTEON, 0xA, 3
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, SPECIES_JOLTEON
	special QzSetSeenMon
	goto Kanto_Route25_SeaCottage_EventScript_842091F

Kanto_Route25_SeaCottage_EventScript_84209B2::
	showmonpic SPECIES_VAPOREON, 0xA, 3
	waitbuttonpress
	hidemonpic
	setvar VAR_0x8004, SPECIES_VAPOREON
	special QzSetSeenMon
	goto Kanto_Route25_SeaCottage_EventScript_842091F

Kanto_Route25_SeaCottage_EventScript_84209C7::
	releaseall
	end

Kanto_Route25_SeaCottage_Movement_842080F:
	walk_up
	walk_up
	step_end

Kanto_Route25_SeaCottage_Movement_8420812:
	walk_right
	walk_up
	walk_up
	walk_left
	walk_in_place_faster_up
	step_end

Kanto_Route25_SeaCottage_Movement_8420818:
	walk_up
	step_end

Kanto_Route25_SeaCottage_Movement_8420905:
	walk_up
	walk_up
	walk_right
	walk_right
	step_end

Kanto_Route25_SeaCottage_Movement_842090A:
	delay_16
	delay_16
	walk_down
	walk_down
	walk_left
	walk_left
	step_end

Kanto_Route25_SeaCottage_Movement_84209C9:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	step_end

Kanto_Route25_SeaCottage_Movement_84209D0:
	walk_down
	step_end

Kanto_Route25_SeaCottage_Text_84502AA::
	.string "Hiya! I’m a POKéMON…\n"
	.string "…No I’m not!\p"
	.string "Call me BILL!\n"
	.string "I’m a true-blue POKéMANIAC!\p"
	.string "Hey!\n"
	.string "What’s with that skeptical look?\p"
	.string "I’m not joshing you, pal.\p"
	.string "I screwed up an experiment and got\n"
	.string "combined with a POKéMON!\p"
	.string "So, how about it?\n"
	.string "Help me out here!$"

Kanto_Route25_SeaCottage_Text_8450396::
	.string "Hiya! I’m a POKéMON…\n"
	.string "…No I’m not!\p"
	.string "Call me BILL!\n"
	.string "I’m a true-blue POKéMANIAC!\p"
	.string "Hey!\n"
	.string "What’s with that skeptical look?\p"
	.string "I’m not joshing you, lady.\p"
	.string "I screwed up an experiment and got\n"
	.string "combined with a POKéMON!\p"
	.string "So, how about it?\n"
	.string "Help me out here!$"

Kanto_Route25_SeaCottage_Text_8450483::
	.string "Wait till I get inside the\n"
	.string "TELEPORTER.\p"
	.string "When I do, go to my PC and run\n"
	.string "the Cell Separation System.$"

Kanto_Route25_SeaCottage_Text_84504E5::
	.string "No!?\n"
	.string "Now don’t be so cold!\p"
	.string "Come on, you gotta help a guy in\n"
	.string "deep, deep trouble!\p"
	.string "What do you say, chief?\n"
	.string "Please?\l"
	.string "Okay?\l"
	.string "All right!$"

Kanto_Route25_SeaCottage_Text_8450566::
	.string "No!?\n"
	.string "Now don’t be so cold!\p"
	.string "Come on, you gotta help a guy in\n"
	.string "deep, deep trouble!\p"
	.string "What do you say, beautiful?\n"
	.string "Please?\l"
	.string "Okay?\l"
	.string "All right!$"

Kanto_Route25_SeaCottage_Text_84505EB::
	.string "BILL: Yeehah!\n"
	.string "Thanks, bud! I owe you one!\p"
	.string "So, did you come to see my\n"
	.string "POKéMON collection?\p"
	.string "You didn’t?\n"
	.string "That’s a bummer.\p"
	.string "I’ve got to thank you…\n"
	.string "Oh, here, maybe this’ll do.$"

Kanto_Route25_SeaCottage_Text_8450694::
	.string "BILL: Yeehah!\n"
	.string "Thanks, lady! I owe you one!\p"
	.string "So, did you come to see my\n"
	.string "POKéMON collection?\p"
	.string "You didn’t?\n"
	.string "That’s a bummer.\p"
	.string "I’ve got to thank you…\n"
	.string "Oh, here, maybe this’ll do.$"

Kanto_Route25_SeaCottage_Text_845073E::
	.string "{PLAYER} received an S.S. TICKET\n"
	.string "from BILL.$"

Kanto_Route25_SeaCottage_Text_8450764::
	.string "You’ve got too much stuff!$"

Kanto_Route25_SeaCottage_Text_845077F::
	.string "That cruise ship S.S. ANNE is in\n"
	.string "VERMILION CITY.\p"
	.string "I hear there are lots of TRAINERS\n"
	.string "on board, too.\p"
	.string "They invited me to their party, but\n"
	.string "I can’t stand fancy do’s.\p"
	.string "Why don’t you go instead of me?\n"
	.string "Go on and have a good time.$"

Kanto_Route25_SeaCottage_Text_845085B::
	.string "BILL: Feel like checking out some\n"
	.string "of my rare POKéMON on my PC?\p"
	.string "Go on, check out my PC.$"

Kanto_Route25_SeaCottage_Text_84508B2::
	.string "TELEPORTER is displayed on the PC\n"
	.string "monitor.$"

Kanto_Route25_SeaCottage_Text_84508DD::
	.string "{PLAYER} initiated the TELEPORTER’s\n"
	.string "Cell Separator.$"

Kanto_Route25_SeaCottage_Text_845090B::
	.string "BILL’s favorite POKéMON list!$"

Kanto_Route25_SeaCottage_Text_8450929::
	.string "Which POKéMON do you want to see?$"
