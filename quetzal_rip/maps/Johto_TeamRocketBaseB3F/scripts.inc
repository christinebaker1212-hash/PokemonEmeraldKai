Johto_TeamRocketBaseB3F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_TeamRocketBaseB3F_OnFrame_83EACA8
	map_script MAP_SCRIPT_ON_LOAD, Johto_TeamRocketBaseB3F_EventScript_83EAC55
	.byte 0

Johto_TeamRocketBaseB3F_OnFrame_83EACA8:
	map_script_2 VAR_QZ_0x500C, 6, Johto_TeamRocketBaseB3F_EventScript_83EACB2
	.2byte 0

Johto_TeamRocketBaseB3F_EventScript_83EAC55::
	call_if_set FLAG_QZ_0x10C8, Quetzal_EventScript_83EAC6A
	call_if_eq VAR_QZ_0x5024, 2, Johto_TeamRocketBaseB3F_EventScript_83EACA1
	end

Johto_TeamRocketBaseB3F_EventScript_83EACA1::
	setflag FLAG_QZ_0x10CB
	clearflag FLAG_QZ_0x10CC
	return

Johto_TeamRocketBaseB3F_EventScript_83EACB2::
	lock
	delay 0x37
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	delay 0x14
	applymovement 3, Quetzal_Movement_836083B
	msgbox Johto_TeamRocketBaseB3F_Text_83EAD16, MSGBOX_DEFAULT
	closemessage
	applymovement 3, Johto_TeamRocketBaseB3F_Movement_83EACF7
	applymovement 2, Johto_TeamRocketBaseB3F_Movement_83EAD03
	waitmovement 3
	waitmovement 2
	setflag FLAG_QZ_0x10C6
	removeobject 2
	removeobject 3
	setvar VAR_QZ_0x500C, 7
	release
	end

@ Johto_TeamRocketBaseB3F object 4 (local id 4, gfx 0x3EB0) at 26,18
Johto_TeamRocketBaseB3F_EventScript_83EADD3::
	lock
	faceplayer
	msgbox Johto_TeamRocketBaseB3F_Text_83EADDF, MSGBOX_DEFAULT
	release
	end

@ Johto_TeamRocketBaseB3F trigger at 32,3 (VAR_QZ_0x500C == 10)
@ Johto_TeamRocketBaseB3F trigger at 31,2 (VAR_QZ_0x500C == 10)
@ Johto_TeamRocketBaseB3F trigger at 27,2 (VAR_QZ_0x500C == 10)
@ Johto_TeamRocketBaseB3F trigger at 27,3 (VAR_QZ_0x500C == 10)
@ Johto_TeamRocketBaseB3F trigger at 27,4 (VAR_QZ_0x500C == 10)
@ Johto_TeamRocketBaseB3F trigger at 27,5 (VAR_QZ_0x500C == 10)
@ Johto_TeamRocketBaseB3F trigger at 33,4 (VAR_QZ_0x500C == 10)
@ Johto_TeamRocketBaseB3F trigger at 33,5 (VAR_QZ_0x500C == 10)
Johto_TeamRocketBaseB3F_EventScript_83EAE10::
	lock
	applymovement 17, Johto_TeamRocketBaseB3F_Movement_83EAE43
	waitmovement 0
	playmoncry SPECIES_MURKROW, 0
	msgbox Quetzal_Text_83EB7ED, MSGBOX_DEFAULT
	closemessage
	applymovement 17, Johto_TeamRocketBaseB3F_Movement_83EAE46
	waitmovement 0
	setflag FLAG_QZ_0x10D0
	removeobject 17
	clearflag FLAG_QZ_0x10CF
	setvar VAR_QZ_0x500C, 0xB
	release
	end

@ Johto_TeamRocketBaseB3F trigger at 8,13 (VAR_QZ_0x500C == 7)
Johto_TeamRocketBaseB3F_EventScript_83EAE56::
	lock
	clearflag FLAG_QZ_0x10C7
	addobject 5
	playbgm MUS_QZ_648, 1
	applymovement 5, Johto_TeamRocketBaseB3F_Movement_83EAE90
	waitmovement 0
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_TeamRocketBaseB3F_Text_83EB049, MSGBOX_DEFAULT
	closemessage
	applymovement 5, Johto_TeamRocketBaseB3F_Movement_83EAE9C
	waitmovement 0
	setflag FLAG_QZ_0x10C7
	removeobject 5
	setvar VAR_QZ_0x500C, 8
	fadedefaultbgm
	release
	end

@ Johto_TeamRocketBaseB3F sign / bg event at 11,12
@ Johto_TeamRocketBaseB3F sign / bg event at 12,12
@ Johto_TeamRocketBaseB3F sign / bg event at 13,12
Johto_TeamRocketBaseB3F_EventScript_83EAEA8::
	lock
	goto_if_set FLAG_QZ_0x10C8, Johto_TeamRocketBaseB3F_EventScript_83EAF12
	goto_if_eq VAR_QZ_0x5024, 2, Johto_TeamRocketBaseB3F_EventScript_83EAEC8
	msgbox Johto_TeamRocketBaseB3F_Text_83EAFD4, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_TeamRocketBaseB3F_EventScript_83EAEC8::
	msgbox Johto_TeamRocketBaseB3F_Text_83EB007, MSGBOX_DEFAULT
	closemessage
	playse SE_CLICK
	setmetatile 0xB, 0xB, 0x378, 0
	setmetatile 0xB, 0xC, 0x380, 0
	setmetatile 0xC, 0xB, 0x379, 0
	setmetatile 0xC, 0xC, 0x379, 0
	setmetatile 0xD, 0xB, 0x379, 0
	setmetatile 0xD, 0xC, 0x379, 0
	special DrawWholeMapView
	setflag FLAG_QZ_0x10C8
	release
	end

Johto_TeamRocketBaseB3F_EventScript_83EAF12::
	end

@ Johto_TeamRocketBaseB3F object 1 (local id 1, gfx 0x3EAF) at 29,9
Johto_TeamRocketBaseB3F_EventScript_83EAF13::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4836, 0, Johto_TeamRocketBaseB3F_Text_83EB870, Johto_TeamRocketBaseB3F_Text_83EB8B2
	msgbox Johto_TeamRocketBaseB3F_Text_83EB8D2, MSGBOX_NPC
	goto_if_unset FLAG_QZ_0x10CA, Johto_TeamRocketBaseB3F_EventScript_83EAF33
	end

Johto_TeamRocketBaseB3F_EventScript_83EAF33::
	addvar VAR_QZ_0x5024, 1
	setflag FLAG_QZ_0x10CA
	end

@ Johto_TeamRocketBaseB3F object 10 (local id 10, gfx 0x3EB0) at 4,16
Johto_TeamRocketBaseB3F_EventScript_83EAF3C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4874, 0, Johto_TeamRocketBaseB3F_Text_83EB932, Johto_TeamRocketBaseB3F_Text_83EBA02
	msgbox Johto_TeamRocketBaseB3F_Text_83EBA22, MSGBOX_NPC
	goto_if_unset FLAG_QZ_0x10C9, Johto_TeamRocketBaseB3F_EventScript_83EAF5C
	end

Johto_TeamRocketBaseB3F_EventScript_83EAF5C::
	addvar VAR_QZ_0x5024, 1
	setflag FLAG_QZ_0x10C9
	end

@ Johto_TeamRocketBaseB3F object 13 (local id 13, gfx 0x3EB4) at 24,12
Johto_TeamRocketBaseB3F_EventScript_83EAF65::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4937, 0, Johto_TeamRocketBaseB3F_Text_83EBA77, Johto_TeamRocketBaseB3F_Text_83EBAEB
	msgbox Johto_TeamRocketBaseB3F_Text_83EBB0A, MSGBOX_AUTOCLOSE
	end

@ Johto_TeamRocketBaseB3F object 11 (local id 11, gfx 0x3EB4) at 12,18
Johto_TeamRocketBaseB3F_EventScript_83EAF7C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4596, 0, Johto_TeamRocketBaseB3F_Text_83EBB94, Johto_TeamRocketBaseB3F_Text_83EBBCA
	msgbox Johto_TeamRocketBaseB3F_Text_83EBBF4, MSGBOX_AUTOCLOSE
	end

@ Johto_TeamRocketBaseB3F object 7 (local id 7, gfx 0x3B) at 22,4
Johto_TeamRocketBaseB3F_EventScript_83EAF93::
	setorcopyvar VAR_0x8000, 0x2E
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB3F object 6 (local id 6, gfx 0x3B) at 31,13
Johto_TeamRocketBaseB3F_EventScript_83EAFA0::
	setorcopyvar VAR_0x8000, 0x42
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB3F object 12 (local id 12, gfx 0x3B) at 1,16
Johto_TeamRocketBaseB3F_EventScript_83EAFAD::
	setorcopyvar VAR_0x8000, 0x30
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB3F object 9 (local id 9, gfx 0x3B) at 3,16
Johto_TeamRocketBaseB3F_EventScript_83EAFBA::
	setorcopyvar VAR_0x8000, 0x80
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB3F object 8 (local id 8, gfx 0x3B) at 16,13
Johto_TeamRocketBaseB3F_EventScript_83EAFC7::
	setorcopyvar VAR_0x8000, 0x1E
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB3F object 16 (local id 16, gfx 0x3E97) at 10,3
Johto_TeamRocketBaseB3F_EventScript_83EB218::
	lock
	faceplayer
	msgbox Johto_TeamRocketBaseB3F_Text_83EB404, MSGBOX_DEFAULT
	playse SE_PIN
	applymovement 16, Quetzal_Movement_83607F7
	delay 0x3C
	msgbox Johto_TeamRocketBaseB3F_Text_83EB496, MSGBOX_DEFAULT
	closemessage
	applymovement 16, Johto_TeamRocketBaseB3F_Movement_83EB3E4
	waitmovement 0
	setflag FLAG_QZ_0x10CC
	removeobject 16
	clearflag FLAG_QZ_0x10CB
	addobject 15
	applymovement 15, Johto_TeamRocketBaseB3F_Movement_83EB3FC
	waitmovement 0
	call_if_eq VAR_FACING, 4, Johto_TeamRocketBaseB3F_EventScript_83EB345
	call_if_eq VAR_FACING, 3, Johto_TeamRocketBaseB3F_EventScript_83EB34A
	msgbox Johto_TeamRocketBaseB3F_Text_83EB511, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4563, 0, Johto_TeamRocketBaseB3F_Text_83EB67C
	msgbox Johto_TeamRocketBaseB3F_Text_83EB6B3, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, 4, Johto_TeamRocketBaseB3F_EventScript_83EB354
	call_if_eq VAR_FACING, 3, Johto_TeamRocketBaseB3F_EventScript_83EB35F
	call_if_eq VAR_FACING, 2, Johto_TeamRocketBaseB3F_EventScript_83EB354
	setflag FLAG_QZ_0x10CB
	playse SE_EXIT
	removeobject 15
	applymovement 14, Johto_TeamRocketBaseB3F_Movement_83EB36A
	delay 0x78
	call_if_eq VAR_FACING, 4, Johto_TeamRocketBaseB3F_EventScript_83EB31B
	call_if_eq VAR_FACING, 3, Johto_TeamRocketBaseB3F_EventScript_83EB330
	turnobject LOCALID_PLAYER, 1
	waitmovement 14
	playmoncry SPECIES_MURKROW, 0
	msgbox Quetzal_Text_83EB7ED, MSGBOX_DEFAULT
	msgbox Johto_TeamRocketBaseB3F_Text_83EB805, MSGBOX_DEFAULT
	closemessage
	applymovement 14, Johto_TeamRocketBaseB3F_Movement_83EB377
	waitmovement 14
	setflag FLAG_QZ_0x10CD
	playse SE_EXIT
	removeobject 14
	clearflag FLAG_QZ_0x10CE
	setflag FLAG_QZ_0x10CF
	setflag FLAG_QZ_0x10D0
	setflag FLAG_QZ_0x10D1
	setvar VAR_QZ_0x5024, 3
	setvar VAR_QZ_0x500C, 9
	release
	end

Johto_TeamRocketBaseB3F_EventScript_83EB31B::
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083B
	waitmovement LOCALID_PLAYER
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement LOCALID_PLAYER
	return

Johto_TeamRocketBaseB3F_EventScript_83EB330::
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement LOCALID_PLAYER
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839
	waitmovement LOCALID_PLAYER
	return

Johto_TeamRocketBaseB3F_EventScript_83EB345::
	turnobject 15, 3
	return

Johto_TeamRocketBaseB3F_EventScript_83EB34A::
	turnobject 15, 4
	return

Johto_TeamRocketBaseB3F_EventScript_83EB354::
	applymovement 15, Johto_TeamRocketBaseB3F_Movement_83EB398
	waitmovement 0
	return

Johto_TeamRocketBaseB3F_EventScript_83EB35F::
	applymovement 15, Johto_TeamRocketBaseB3F_Movement_83EB3BC
	waitmovement 0
	return

Johto_TeamRocketBaseB3F_Movement_83EACF7:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	delay_2
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_TeamRocketBaseB3F_Movement_83EAD03:
	delay_16
	delay_16
	delay_16
	delay_16
	delay_8
	delay_8
	delay_8
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_TeamRocketBaseB3F_Movement_83EAE43:
	jump_in_place_right
	jump_in_place_right
	step_end

Johto_TeamRocketBaseB3F_Movement_83EAE46:
	jump_in_place_right
	walk_fast_down
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_TeamRocketBaseB3F_Movement_83EAE90:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

Johto_TeamRocketBaseB3F_Movement_83EAE9C:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_TeamRocketBaseB3F_Movement_83EB36A:
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	jump_in_place_right
	jump_in_place_left
	jump_in_place_down
	jump_in_place_up
	jump_in_place_up
	jump_in_place_up
	jump_in_place_up
	step_end

Johto_TeamRocketBaseB3F_Movement_83EB377:
	jump_in_place_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_right
	walk_faster_right
	step_end

Johto_TeamRocketBaseB3F_Movement_83EB398:
	lock_facing_direction
	walk_slow_right
	unlock_facing_direction
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_fast_right
	walk_fast_right
	step_end

Johto_TeamRocketBaseB3F_Movement_83EB3BC:
	lock_facing_direction
	walk_slow_left
	walk_slow_left
	unlock_facing_direction
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_down
	walk_faster_down
	walk_faster_down
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_fast_right
	walk_fast_right
	step_end

Johto_TeamRocketBaseB3F_Movement_83EB3E4:
	face_right
	delay_8
	face_up
	delay_8
	face_left
	delay_8
	face_down
	delay_8
	face_right
	delay_4
	face_up
	delay_4
	face_left
	delay_4
	face_down
	delay_4
	face_right
	delay_2
	face_up
	delay_2
	face_left
	delay_2
	face_down
	step_end

Johto_TeamRocketBaseB3F_Movement_83EB3FC:
	face_right
	delay_2
	face_up
	delay_2
	face_left
	delay_2
	face_down
	step_end

Johto_TeamRocketBaseB3F_Text_83EAD16::
	.string "LANCE: It takes two passwords\n"
	.string "to get into the boss’ quarters.\p"
	.string "Those passwords are known only\n"
	.string "to a few ROCKETS.\p"
	.string "That ROCKET there very graciously\n"
	.string "told me so.\p"
	.string "{PLAYER}, let’s go get the\n"
	.string "passwords.$"

Johto_TeamRocketBaseB3F_Text_83EADDF::
	.string "Urrggh… The guy in the cape is\n"
	.string "incredibly tough…$"

Johto_TeamRocketBaseB3F_Text_83EAFD4::
	.string "The door’s closed…\p"
	.string "It needs two passwords to open.$"

Johto_TeamRocketBaseB3F_Text_83EB007::
	.string "The door’s closed…\p"
	.string "{PLAYER} entered the two passwords.\p"
	.string "The door opened!$"

Johto_TeamRocketBaseB3F_Text_83EB049::
	.string "…\p"
	.string "Didn’t I tell you that I was\n"
	.string "going to destroy TEAM ROCKET?\p"
	.string "…Tell me, who was the guy in the\n"
	.string "cape who used dragon POKéMON?\p"
	.string "My POKéMON were no match at all.\p"
	.string "It’s what he said that bothers\n"
	.string "me…\p"
	.string "He told me that I don’t love and\n"
	.string "trust my POKéMON enough.\p"
	.string "It’s not my fault my team is so\n"
	.string "useless.\p"
	.string "I’m furious that I lost to a\n"
	.string "bleeding heart like him.\p"
	.string "But it makes me question myself.\p"
	.string "Is there something I’m doing\n"
	.string "wrong?\p"
	.string "Could it be that love made his\n"
	.string "team so powerful?$"

Johto_TeamRocketBaseB3F_Text_83EB404::
	.string "Muhahaha, we’ve been waiting\n"
	.string "for you. So, you are {PLAYER}...\p"
	.string "Me? You don’t know who I am?\n"
	.string "It is me, GIOVANNI.\p"
	.string "The majestic GIOVANNI himself!\n"
	.string "Wahahaha!$"

Johto_TeamRocketBaseB3F_Text_83EB496::
	.string "...Huh?\n"
	.string "I don’t sound anything like\l"
	.string "GIOVANNI?\p"
	.string "I don’t even look like GIOVANNI?\n"
	.string "How come? I’ve worked so hard\l"
	.string "to mimic him!$"

Johto_TeamRocketBaseB3F_Text_83EB511::
	.string "PETREL: I am a Team Rocket\n"
	.string "Executive!\p"
	.string "You must be trying to sneak into\n"
	.string "the radio-transmitter room.\p"
	.string "Well, that’s not going to happen.\n"
	.string "That room is protected with a\l"
	.string "special password.\p"
	.string "The password is ’Hail GIOVANNI.’\n"
	.string "Surprised to hear it from me?\p"
	.string "Knowing the password won’t help\n"
	.string "you though.\p"
	.string "The door won’t open unless I say\n"
	.string "the password. It only reacts to\l"
	.string "my voice.$"

Johto_TeamRocketBaseB3F_Text_83EB67C::
	.string "I… I couldn’t do a thing…\p"
	.string "GIOVANNI, please forgive me…$"

Johto_TeamRocketBaseB3F_Text_83EB6B3::
	.string "PETREL: Since disbanding Team\n"
	.string "Rocket three years ago, our\l"
	.string "GIOVANNI has been missing.\p"
	.string "But we’re certain he’s been\n"
	.string "waiting for the right time for\l"
	.string "our revival... Wahahaha!\p"
	.string "Losing to you won’t change the\n"
	.string "fact that you are unable to get\l"
	.string "in the radio-transmitter room!\p"
	.string "You need my voice to unlock it!$"

Johto_TeamRocketBaseB3F_Text_83EB805::
	.string "MURKROW is mimicking\n"
	.string "the voice of PETREL!\p"
	.string "You may be able to...!?$"

Johto_TeamRocketBaseB3F_Text_83EB870::
	.string "Do I know the password?\p"
	.string "Maybe.\p"
	.string "But no weakling’s going to get\n"
	.string "it!$"

Johto_TeamRocketBaseB3F_Text_83EB8B2::
	.string "All right. Stop. I’ll tell you.$"

Johto_TeamRocketBaseB3F_Text_83EB8D2::
	.string "The password to the boss’ room\n"
	.string "is SLOWPOKETAIL.\p"
	.string "But it’s useless unless you have\n"
	.string "two passwords.$"

Johto_TeamRocketBaseB3F_Text_83EB932::
	.string "Hyuck-hyuck-hyuck!\p"
	.string "I remember you from the well!\p"
	.string "You’re challenging me again?\p"
	.string "Hah! You’re nuts, but you have\n"
	.string "guts!\p"
	.string "I’ll warn you, I got better!\p"
	.string "If you can beat me, I’ll tell\l"
	.string "you a password to the boss’\n"
	.string "room!$"

Johto_TeamRocketBaseB3F_Text_83EBA02::
	.string "Hyuck-hyuck-hyuck! You’re good!$"

Johto_TeamRocketBaseB3F_Text_83EBA22::
	.string "Hyuck-hyuck-hyuck!\p"
	.string "The password to the boss’ room…\p"
	.string "Uh…, I think it is RATICATE TAIL.$"

Johto_TeamRocketBaseB3F_Text_83EBA77::
	.string "I used to work for SILPH, but now\n"
	.string "I run research for TEAM ROCKET.\p"
	.string "A meddlesome child like you needs\n"
	.string "to be punished.$"

Johto_TeamRocketBaseB3F_Text_83EBAEB::
	.string "A mere tactical error cost me…$"

Johto_TeamRocketBaseB3F_Text_83EBB0A::
	.string "A radio signal that drives\n"
	.string "POKéMON mad…\p"
	.string "My experiment is a complete\n"
	.string "success.\p"
	.string "My promotion is assured. This\n"
	.string "loss means absolutely nothing.$"

Johto_TeamRocketBaseB3F_Text_83EBB94::
	.string "I don’t care that POKéMON are hurt\n"
	.string "by our experiment.$"

Johto_TeamRocketBaseB3F_Text_83EBBCA::
	.string "Thinking is my strong suit, not\n"
	.string "battling.$"

Johto_TeamRocketBaseB3F_Text_83EBBF4::
	.string "If we turn up the power of our\n"
	.string "radio signal for broadcast\l"
	.string "nationwide…\p"
	.string "The very thought excites me!$"
