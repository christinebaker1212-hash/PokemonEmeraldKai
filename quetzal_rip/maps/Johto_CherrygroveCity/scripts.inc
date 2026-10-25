Johto_CherrygroveCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_CherrygroveCity_EventScript_83C8D64
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_CherrygroveCity_EventScript_83C8D64::
	special QzSetGiovanniModeFlag14A4_794
	special QzJohtoTravelingEventCheck_770
	end

@ Johto_CherrygroveCity object 1 (local id 1, gfx 0x3EA6) at 57,8
@ Johto_CherrygroveCity trigger at 57,9 (VAR_QZ_0x5005 == 0)
Johto_CherrygroveCity_EventScript_83C8D6B::
	lock
	setflag FLAG_QZ_0x11BB
	setflag FLAG_QZ_0x108C
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_CherrygroveCity_Text_83C905B, MSGBOX_YESNO
	waitmessage
	goto_if_eq VAR_RESULT, 0, Johto_CherrygroveCity_EventScript_83C8E55
	msgbox Johto_CherrygroveCity_Text_83C90E8, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C9685
	waitmovement 0
	applymovement 1, Johto_CherrygroveCity_Movement_83C9680
	waitmovement 0
	goto Johto_CherrygroveCity_EventScript_83C8E65

@ Johto_CherrygroveCity trigger at 57,10 (VAR_QZ_0x5005 == 0)
Johto_CherrygroveCity_EventScript_83C8DBB::
	lock
	setflag FLAG_QZ_0x11BB
	setflag FLAG_QZ_0x108C
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_CherrygroveCity_Text_83C905B, MSGBOX_YESNO
	waitmessage
	goto_if_eq VAR_RESULT, 0, Johto_CherrygroveCity_EventScript_83C8E55
	msgbox Johto_CherrygroveCity_Text_83C90E8, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_CherrygroveCity_Movement_83C9680
	waitmovement 0
	turnobject LOCALID_PLAYER, 3
	goto Johto_CherrygroveCity_EventScript_83C8E65

@ Johto_CherrygroveCity trigger at 57,11 (VAR_QZ_0x5005 == 0)
Johto_CherrygroveCity_EventScript_83C8E05::
	lock
	setflag FLAG_QZ_0x11BB
	setflag FLAG_QZ_0x108C
	playse SE_PIN
	applymovement 1, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_CherrygroveCity_Text_83C905B, MSGBOX_YESNO
	waitmessage
	goto_if_eq VAR_RESULT, 0, Johto_CherrygroveCity_EventScript_83C8E55
	msgbox Johto_CherrygroveCity_Text_83C90E8, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_CherrygroveCity_Movement_83C9680
	waitmovement 0
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C9688
	waitmovement 0
	goto Johto_CherrygroveCity_EventScript_83C8E65

Johto_CherrygroveCity_EventScript_83C8E55::
	msgbox Johto_CherrygroveCity_Text_83C92F3, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x5005, 1
	release
	end

Johto_CherrygroveCity_EventScript_83C8E65::
	playbgm MUS_QZ_647, 0
	applymovement 1, Johto_CherrygroveCity_Movement_83C968B
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C968B
	waitmovement 0
	waitmovement 0
	msgbox Johto_CherrygroveCity_Text_83C90FD, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_CherrygroveCity_Movement_83C9696
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C9696
	waitmovement 0
	waitmovement 0
	msgbox Johto_CherrygroveCity_Text_83C9182, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_CherrygroveCity_Movement_83C969F
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C969F
	waitmovement 0
	waitmovement 0
	msgbox Johto_CherrygroveCity_Text_83C91DC, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_CherrygroveCity_Movement_83C96A9
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C96A9
	waitmovement 0
	waitmovement 0
	msgbox Johto_CherrygroveCity_Text_83C922C, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_CherrygroveCity_Movement_83C96B6
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C96CC
	waitmovement 0
	waitmovement 0
	delay 0x14
	msgbox Johto_CherrygroveCity_Text_83C9276, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Johto_CherrygroveCity_Movement_83C96E2
	turnobject LOCALID_PLAYER, 2
	delay 0x14
	opendoor 0x2A, 0xE
	waitdooranim
	waitmovement 0
	removeobject 1
	closedoor 0x2A, 0xE
	waitdooranim
	clearflag FLAG_QZ_0x108C
	setflag FLAG_QZ_0x107C
	setvar VAR_QZ_0x5005, 1
	fadedefaultbgm
	release
	end

@ Johto_CherrygroveCity object 2 (local id 2, gfx 0x3E95) at 18,19
Johto_CherrygroveCity_EventScript_83C8F2B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1023, Johto_CherrygroveCity_EventScript_83C8F66
	msgbox Johto_CherrygroveCity_Text_83C9580, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_CherrygroveCity_EventScript_83C8F66
	setorcopyvar VAR_0x8000, 0x1AB
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	closemessage
	setflag FLAG_QZ_0x1023
	release
	end

Johto_CherrygroveCity_EventScript_83C8F66::
	msgbox Johto_CherrygroveCity_Text_83C95DF, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CherrygroveCity object 4 (local id 4, gfx 0x2F) at 39,11
Johto_CherrygroveCity_EventScript_83C8F71::
	lock
	faceplayer
	msgbox Johto_CherrygroveCity_Text_83C94B5, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CherrygroveCity object 3 (local id 3, gfx 0x3E87) at 48,12
Johto_CherrygroveCity_EventScript_83C8F7E::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0xA79, Johto_CherrygroveCity_EventScript_83C8F94
	msgbox Johto_CherrygroveCity_Text_83C94E6, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CherrygroveCity_EventScript_83C8F94::
	msgbox Johto_CherrygroveCity_Text_83C9515, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CherrygroveCity sign / bg event at 51,12
Johto_CherrygroveCity_EventScript_83C8F9F::
	lock
	msgbox Johto_CherrygroveCity_Text_83C95FD, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_CherrygroveCity trigger at 56,9 (VAR_QZ_0x5005 == 3)
Johto_CherrygroveCity_EventScript_83C8FAB::
	lock
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C96E5
	waitmovement 0
	goto Johto_CherrygroveCity_EventScript_83C8FCD

@ Johto_CherrygroveCity trigger at 56,11 (VAR_QZ_0x5005 == 3)
Johto_CherrygroveCity_EventScript_83C8FBC::
	lock
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C96E8
	waitmovement 0
	goto Johto_CherrygroveCity_EventScript_83C8FCD

@ Johto_CherrygroveCity trigger at 56,10 (VAR_QZ_0x5005 == 3)
Johto_CherrygroveCity_EventScript_83C8FCD::
	lock
	playbgm MUS_QZ_648, 1
	applymovement 5, Johto_CherrygroveCity_Movement_83C96EB
	waitmovement 0
	msgbox Johto_CherrygroveCity_Text_83C933A, MSGBOX_DEFAULT
	closemessage
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4701, 0, Johto_CherrygroveCity_Text_83C93F3
	goto Johto_CherrygroveCity_EventScript_83C8FF4

Johto_CherrygroveCity_EventScript_83C8FF4::
	playbgm MUS_QZ_649, 1
	msgbox Johto_CherrygroveCity_Text_83C9411, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_CherrygroveCity_Movement_83C96FB
	applymovement 5, Johto_CherrygroveCity_Movement_83C96F4
	waitmovement 0
	waitmovement 0
	playse SE_PIN
	applymovement 5, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject 5, 4
	delay 0x3C
	turnobject 5, 1
	delay 0x1E
	applymovement 5, Johto_CherrygroveCity_Movement_83C9700
	waitmovement 0
	msgbox Johto_CherrygroveCity_Text_83C9475, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Johto_CherrygroveCity_Movement_83C9706
	waitmovement 0
	removeobject 5
	setflag FLAG_QZ_0x107B
	setvar VAR_QZ_0x5005, 4
	fadedefaultbgm
	release
	end

Johto_CherrygroveCity_Movement_83C9680:
	walk_down
	walk_left
	walk_down
	face_left
	step_end

Johto_CherrygroveCity_Movement_83C9685:
	walk_down
	face_up
	step_end

Johto_CherrygroveCity_Movement_83C9688:
	walk_up
	face_up
	step_end

Johto_CherrygroveCity_Movement_83C968B:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	step_end

Johto_CherrygroveCity_Movement_83C9696:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	face_up
	step_end

Johto_CherrygroveCity_Movement_83C969F:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	step_end

Johto_CherrygroveCity_Movement_83C96A9:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	step_end

Johto_CherrygroveCity_Movement_83C96B6:
	walk_down
	walk_right
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_up
	step_end

Johto_CherrygroveCity_Movement_83C96CC:
	delay_16
	delay_16
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	face_up
	step_end

Johto_CherrygroveCity_Movement_83C96E2:
	walk_up
	walk_up
	step_end

Johto_CherrygroveCity_Movement_83C96E5:
	walk_down
	face_right
	step_end

Johto_CherrygroveCity_Movement_83C96E8:
	walk_up
	face_right
	step_end

Johto_CherrygroveCity_Movement_83C96EB:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_CherrygroveCity_Movement_83C96F4:
	delay_16
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_CherrygroveCity_Movement_83C96FB:
	lock_facing_direction
	walk_fast_down
	unlock_facing_direction
	face_up
	step_end

Johto_CherrygroveCity_Movement_83C9700:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	face_down
	step_end

Johto_CherrygroveCity_Movement_83C9706:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_CherrygroveCity_Text_83C905B::
	.string "You’re a rookie trainer,\n"
	.string "aren’t you? I can tell!\p"
	.string "That’s OK! Everyone is a\n"
	.string "rookie at some point!\p"
	.string "If you’d like, I can teach\n"
	.string "you a few things.$"

Johto_CherrygroveCity_Text_83C90E8::
	.string "OK, then!\n"
	.string "Follow me!$"

Johto_CherrygroveCity_Text_83C90FD::
	.string "This is a POKéMON CENTER.\p"
	.string "They heal your POKéMON in no\n"
	.string "time at all.\p"
	.string "You’ll be relying on them a lot,\n"
	.string "so you better learn about them.$"

Johto_CherrygroveCity_Text_83C9182::
	.string "This is a POKéMON MART.\p"
	.string "They sell BALLS for catching wild\n"
	.string "POKéMON and other useful items.$"

Johto_CherrygroveCity_Text_83C91DC::
	.string "ROUTE 30 is out this way.\p"
	.string "Trainers will be battling\n"
	.string "their prized POKéMON there.$"

Johto_CherrygroveCity_Text_83C922C::
	.string "This is the sea, as you can see.\p"
	.string "Some POKéMON are found only in\n"
	.string "in water.$"

Johto_CherrygroveCity_Text_83C9276::
	.string "Here...\p"
	.string "It’s my house! Thanks for\n"
	.string "your company.\p"
	.string "If you’re ever lost, check your\n"
	.string "REGION MAP!\p"
	.string "I wish you luck on your journey!$"

Johto_CherrygroveCity_Text_83C92F3::
	.string "Oh… It’s something I enjoy\n"
	.string "doing…\p"
	.string "Fine. Come see me whenever\n"
	.string "you like.$"

Johto_CherrygroveCity_Text_83C933A::
	.string "… … … … … … … …\p"
	.string "You got a POKéMON at the LAB.\p"
	.string "What a waste.\n"
	.string "A wimp like you.\p"
	.string "… … … … … … … …\p"
	.string "Don’t you get what I’m saying?\n"
	.string "Well, I too, have a good POKéMON.\p"
	.string "I’ll show you what I mean!$"

Johto_CherrygroveCity_Text_83C93F3::
	.string "Humph. Are you happy you won?$"

Johto_CherrygroveCity_Text_83C9411::
	.string "… … … … … … … …\p"
	.string "Do you want to know who I am?\p"
	.string "I’m going to be the world’s\n"
	.string "greatest POKéMON trainer.$"

Johto_CherrygroveCity_Text_83C9475::
	.string "Give it back!\n"
	.string "That’s my TRAINER CARD!\p"
	.string "Oh no!\n"
	.string "You saw my name...$"

Johto_CherrygroveCity_Text_83C94B5::
	.string "When you’re with POKéMON,\n"
	.string "going anywhere is fun.$"

Johto_CherrygroveCity_Text_83C94E6::
	.string "MR. POKéMON’s house is still\n"
	.string "farther up ahead.$"

Johto_CherrygroveCity_Text_83C9515::
	.string "I battled the trainers on the\n"
	.string "road.\p"
	.string "My POKéMON lost. They’re a\n"
	.string "mess! I must take them to a\l"
	.string "POKéMON CENTER.$"

Johto_CherrygroveCity_Text_83C9580::
	.string "A POKéMON I caught had an\n"
	.string "item.\p"
	.string "I think it’s MYSTIC WATER.\p"
	.string "I don’t need it, so do you\n"
	.string "want it?$"

Johto_CherrygroveCity_Text_83C95DF::
	.string "Back to fishing for me, then.$"

Johto_CherrygroveCity_Text_83C95FD::
	.string "CHERRYGROVE CITY\p"
	.string "The City of Cute, Fragrant\n"
	.string "Flowers$"
