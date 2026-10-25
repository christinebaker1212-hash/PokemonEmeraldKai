Kanto_Route22_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route22 trigger at 33,4 (VAR_QZ_0x5125 == 1)
Kanto_Route22_EventScript_8417F9C::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_Route22_EventScript_8417FCE

@ Kanto_Route22 trigger at 33,5 (VAR_QZ_0x5125 == 1)
Kanto_Route22_EventScript_8417FA8::
	lockall
	setvar VAR_TEMP_1, 1
	setobjectxyperm 1, 0x19, 5
	goto Kanto_Route22_EventScript_8417FCE

@ Kanto_Route22 trigger at 33,6 (VAR_QZ_0x5125 == 1)
Kanto_Route22_EventScript_8417FBB::
	lockall
	setvar VAR_TEMP_1, 2
	setobjectxyperm 1, 0x19, 5
	goto Kanto_Route22_EventScript_8417FCE

Kanto_Route22_EventScript_8417FCE::
	textcolor 0
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	addobject 1
	call_if_eq VAR_TEMP_1, 0, Kanto_Route22_EventScript_8418044
	call_if_eq VAR_TEMP_1, 1, Kanto_Route22_EventScript_8418044
	call_if_eq VAR_TEMP_1, 2, Kanto_Route22_EventScript_841804F
	delay 6
	msgbox Kanto_Route22_Text_8438B71, MSGBOX_DEFAULT
	call Kanto_Route22_EventScript_8418061
	msgbox Kanto_Route22_Text_8438C57, MSGBOX_DEFAULT
	closemessage
	delay 0xA
	playbgm MUS_RG_RIVAL_EXIT, 0
	call_if_eq VAR_TEMP_1, 0, Kanto_Route22_EventScript_841808E
	call_if_eq VAR_TEMP_1, 1, Kanto_Route22_EventScript_841808E
	call_if_eq VAR_TEMP_1, 2, Kanto_Route22_EventScript_8418099
	fadedefaultbgm
	removeobject 1
	setvar VAR_QZ_0x5125, 2
	releaseall
	end

Kanto_Route22_EventScript_8418044::
	applymovement 1, Kanto_Route22_Movement_84180C5
	waitmovement 0
	return

Kanto_Route22_EventScript_841804F::
	applymovement 1, Kanto_Route22_Movement_84180CD
	applymovement LOCALID_PLAYER, Kanto_Route22_Movement_84180D7
	waitmovement 0
	return

Kanto_Route22_EventScript_8418061::
	qz_trainerbattle_13 TRAINER_QZ_8521, 0, Kanto_Route22_Text_8438C3C, Quetzal_Text_843E142
	return

Kanto_Route22_EventScript_841808E::
	applymovement 1, Kanto_Route22_Movement_84180AC
	waitmovement 0
	return

Kanto_Route22_EventScript_8418099::
	applymovement 1, Kanto_Route22_Movement_84180B9
	waitmovement 0
	return

@ Kanto_Route22 trigger at 33,4 (VAR_QZ_0x5125 == 3)
Kanto_Route22_EventScript_84180E1::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_Route22_EventScript_8418113

@ Kanto_Route22 trigger at 33,5 (VAR_QZ_0x5125 == 3)
Kanto_Route22_EventScript_84180ED::
	lockall
	setvar VAR_TEMP_1, 1
	setobjectxyperm 1, 0x19, 5
	goto Kanto_Route22_EventScript_8418113

@ Kanto_Route22 trigger at 33,6 (VAR_QZ_0x5125 == 3)
Kanto_Route22_EventScript_8418100::
	lockall
	setvar VAR_TEMP_1, 2
	setobjectxyperm 1, 0x19, 5
	goto Kanto_Route22_EventScript_8418113

Kanto_Route22_EventScript_8418113::
	textcolor 0
	playbgm MUS_RG_ENCOUNTER_RIVAL, 0
	addobject 1
	call_if_eq VAR_TEMP_1, 0, Kanto_Route22_EventScript_8418174
	call_if_eq VAR_TEMP_1, 1, Kanto_Route22_EventScript_8418174
	call_if_eq VAR_TEMP_1, 2, Kanto_Route22_EventScript_841817F
	msgbox Kanto_Route22_Text_8438D21, MSGBOX_DEFAULT
	setvar VAR_LAST_TALKED, 1
	call Kanto_Route22_EventScript_8418191
	msgbox Kanto_Route22_Text_8438E10, MSGBOX_DEFAULT
	closemessage
	delay 0xA
	playbgm MUS_RG_RIVAL_EXIT, 0
	applymovement 1, Kanto_Route22_Movement_84181B2
	waitmovement 0
	fadedefaultbgm
	removeobject 1
	setvar VAR_QZ_0x5125, 4
	releaseall
	end

Kanto_Route22_EventScript_8418174::
	applymovement 1, Kanto_Route22_Movement_84180C5
	waitmovement 0
	return

Kanto_Route22_EventScript_841817F::
	applymovement 1, Kanto_Route22_Movement_84180CD
	applymovement LOCALID_PLAYER, Kanto_Route22_Movement_84180D7
	waitmovement 0
	return

Kanto_Route22_EventScript_8418191::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8627, 0, Kanto_Route22_Text_8438DEF
	return

@ Kanto_Route22 sign / bg event at 7,12
Kanto_Route22_EventScript_84181BA::
	msgbox Kanto_Route22_Text_8438EE8, MSGBOX_SIGN
	end

Kanto_Route22_Movement_84180AC:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_Route22_Movement_84180B9:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

Kanto_Route22_Movement_84180C5:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Kanto_Route22_Movement_84180CD:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_down
	step_end

Kanto_Route22_Movement_84180D7:
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_8
	walk_in_place_faster_up
	step_end

Kanto_Route22_Movement_84181B2:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Kanto_Route22_Text_8438B71::
	.string "BLUE: Hey! {PLAYER}!\p"
	.string "You’re off to the POKéMON LEAGUE?\n"
	.string "Forget about it!\p"
	.string "You probably don’t have any\n"
	.string "BADGES, do you?\p"
	.string "The guard won’t let you through\n"
	.string "without them.\p"
	.string "By the way, did your POKéMON get\n"
	.string "any stronger?$"

Kanto_Route22_Text_8438C3C::
	.string "Awww!\n"
	.string "You just lucked out!$"

Kanto_Route22_Text_8438C57::
	.string "I heard the POKéMON LEAGUE is\n"
	.string "crawling with tough TRAINERS.\p"
	.string "I have to figure out how to get\n"
	.string "past them.\p"
	.string "You should quit dawdling and get a\n"
	.string "move on!$"

Kanto_Route22_Text_8438D21::
	.string "BLUE: What? {PLAYER}!\n"
	.string "What a surprise to see you here!\p"
	.string "So you’re going to the POKéMON\n"
	.string "LEAGUE?\p"
	.string "You collected all the BADGES, too?\n"
	.string "That’s cool!\p"
	.string "Then I’ll whip you, {PLAYER}, as a\n"
	.string "warm-up for the POKéMON LEAGUE!\p"
	.string "Come on!$"

Kanto_Route22_Text_8438DEF::
	.string "What!?\p"
	.string "I was just careless, you!$"

Kanto_Route22_Text_8438E10::
	.string "That loosened me up.\n"
	.string "I’m ready for the POKéMON LEAGUE!\p"
	.string "{PLAYER}, you need more practice.\p"
	.string "But hey, you know that!\n"
	.string "I’m out of here. Smell ya!$"

Kanto_Route22_Text_8438EE8::
	.string "POKéMON LEAGUE\n"
	.string "Front Gate$"
