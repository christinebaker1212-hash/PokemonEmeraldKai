Johto_MahoganyTown_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Johto_MahoganyTown_EventScript_83DF66C
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_MahoganyTown_OnFrame_83DF6AA
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_MahoganyTown_OnFrame_83DF6AA:
	map_script_2 VAR_QZ_0x500C, 15, Quetzal_EventScript_83DF6B4
	.2byte 0

Johto_MahoganyTown_EventScript_83DF66C::
	special QzJohtoTravelingEventCheck
	call_if_ge VAR_QZ_0x500C, 13, Johto_MahoganyTown_EventScript_83DF67B
	end

Johto_MahoganyTown_EventScript_83DF67B::
	setmetatile 0x12, 8, 0x378, 0
	setmetatile 0x13, 8, 0x379, 0
	end

@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 1)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 1)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 1)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 2)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 2)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 2)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 3)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 3)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 3)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 4)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 4)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 4)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 5)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 5)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 5)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 6)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 6)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 6)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 7)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 8)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 9)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 10)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 11)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 12)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 13)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 14)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 15)
@ Johto_MahoganyTown trigger at 30,12 (VAR_QZ_0x500C == 16)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 16)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 15)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 14)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 13)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 12)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 11)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 10)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 9)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 8)
@ Johto_MahoganyTown trigger at 30,13 (VAR_QZ_0x500C == 7)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 16)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 15)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 14)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 13)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 12)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 11)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 10)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 9)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 8)
@ Johto_MahoganyTown trigger at 30,14 (VAR_QZ_0x500C == 7)
Johto_MahoganyTown_EventScript_83DF6E1::
	lock
	turnobject 3, 1
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	delay 0x3C
	turnobject LOCALID_PLAYER, 2
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_1, 14, Johto_MahoganyTown_EventScript_83DF787
	call_if_eq VAR_TEMP_1, 13, Johto_MahoganyTown_EventScript_83DF77C
	showmoneybox 0, 0, 0
	msgbox Johto_MahoganyTown_Text_83DF8CF, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_MahoganyTown_EventScript_83DF7F6
	checkmoney 0x12C, 0
	goto_if_eq VAR_RESULT, 0, Johto_MahoganyTown_EventScript_83DF815
	setorcopyvar VAR_0x8000, ITEM_RAGE_CANDY_BAR
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_MahoganyTown_EventScript_83DF834
	removemoney 0x12C, 0
	updatemoneybox 0
	playse SE_SHOP
	msgbox Johto_MahoganyTown_Text_83DF968, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 30, Johto_MahoganyTown_EventScript_83DF85E
	release
	end

Johto_MahoganyTown_EventScript_83DF77C::
	applymovement LOCALID_PLAYER, Johto_MahoganyTown_Movement_83DF8CA
	waitmovement 0
	return

Johto_MahoganyTown_EventScript_83DF787::
	applymovement LOCALID_PLAYER, Johto_MahoganyTown_Movement_83DF8CC
	waitmovement 0
	return

@ Johto_MahoganyTown object 3 (local id 3, gfx 0x3E84) at 30,11
Johto_MahoganyTown_EventScript_83DF792::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500C, 17, Johto_MahoganyTown_EventScript_83DF853
	showmoneybox 0, 0, 0
	msgbox Johto_MahoganyTown_Text_83DF8CF, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_MahoganyTown_EventScript_83DF7F6
	checkmoney 0x12C, 0
	goto_if_eq VAR_RESULT, 0, Johto_MahoganyTown_EventScript_83DF815
	setorcopyvar VAR_0x8000, ITEM_RAGE_CANDY_BAR
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_MahoganyTown_EventScript_83DF834
	removemoney 0x12C, 0
	updatemoneybox 0
	msgbox Johto_MahoganyTown_Text_83DF968, MSGBOX_DEFAULT
	hidemoneybox
	closemessage
	release
	end

Johto_MahoganyTown_EventScript_83DF7F6::
	msgbox Johto_MahoganyTown_Text_83DF995, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	closemessage
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 30, Johto_MahoganyTown_EventScript_83DF85E
	release
	end

Johto_MahoganyTown_EventScript_83DF815::
	msgbox Johto_MahoganyTown_Text_83DF978, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	closemessage
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 30, Johto_MahoganyTown_EventScript_83DF85E
	release
	end

Johto_MahoganyTown_EventScript_83DF834::
	msgbox Quetzal_Text_8360DB1, MSGBOX_DEFAULT
	closemessage
	hidemoneybox
	closemessage
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_0, 30, Johto_MahoganyTown_EventScript_83DF85E
	release
	end

Johto_MahoganyTown_EventScript_83DF853::
	msgbox Johto_MahoganyTown_Text_83DF9A4, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MahoganyTown_EventScript_83DF85E::
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	waitmovement 0
	return

@ Johto_MahoganyTown object 1 (local id 1, gfx 0x3EA6) at 7,13
Johto_MahoganyTown_EventScript_83DF869::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x500C, 6, Johto_MahoganyTown_EventScript_83DF881
	msgbox Johto_MahoganyTown_Text_83DF9E5, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MahoganyTown_EventScript_83DF881::
	msgbox Johto_MahoganyTown_Text_83DFA1A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_MahoganyTown object 4 (local id 4, gfx 0x11) at 10,20
Johto_MahoganyTown_EventScript_83DF88C::
	lock
	faceplayer
	msgbox Johto_MahoganyTown_Text_83DFA72, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_MahoganyTown object 2 (local id 2, gfx 0x10) at 24,8
Johto_MahoganyTown_EventScript_83DF899::
	lock
	faceplayer
	msgbox Johto_MahoganyTown_Text_83DFAED, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_MahoganyTown sign / bg event at 5,8
Johto_MahoganyTown_EventScript_83DF8A6::
	lock
	msgbox Johto_MahoganyTown_Text_83DFB29, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_MahoganyTown sign / bg event at 13,10
Johto_MahoganyTown_EventScript_83DF8B2::
	lock
	msgbox Johto_MahoganyTown_Text_83DFB58, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_MahoganyTown sign / bg event at 12,19
Johto_MahoganyTown_EventScript_83DF8BE::
	lock
	msgbox Johto_MahoganyTown_Text_83DFB8A, MSGBOX_SIGN
	closemessage
	release
	end

Johto_MahoganyTown_Movement_83DF8CA:
	walk_up
	step_end

Johto_MahoganyTown_Movement_83DF8CC:
	walk_up
	walk_up
	step_end

Johto_MahoganyTown_Text_83DF8CF::
	.string "Hiya, kid!\p"
	.string "I see you’re new in MAHOGANY TOWN.\p"
	.string "Since you’re new, you should try a\n"
	.string "yummy RAGECANDYBAR!\p"
	.string "Right now, it can be yours for just\n"
	.string "¥300! Want one?$"

Johto_MahoganyTown_Text_83DF968::
	.string "Good! Savor it!$"

Johto_MahoganyTown_Text_83DF978::
	.string "You don’t have enough money.$"

Johto_MahoganyTown_Text_83DF995::
	.string "Oh, fine then…$"

Johto_MahoganyTown_Text_83DF9A4::
	.string "RAGECANDYBAR’s sold out.\p"
	.string "I’m packing up. Don’t bother me,\n"
	.string "kiddo.$"

Johto_MahoganyTown_Text_83DF9E5::
	.string "Are you off to see the GYARADOS\n"
	.string "rampage at the LAKE?$"

Johto_MahoganyTown_Text_83DFA1A::
	.string "MAGIKARP have returned to LAKE\n"
	.string "OF RAGE.\p"
	.string "That should be good news for the\n"
	.string "anglers there.$"

Johto_MahoganyTown_Text_83DFA72::
	.string "Since you came this far, take the\n"
	.string "time to do some sightseeing.\p"
	.string "You should head north and check\n"
	.string "out LAKE OF RAGE right now.$"

Johto_MahoganyTown_Text_83DFAED::
	.string "Visit Grandma’s shop. She sells\n"
	.string "stuff that nobody else has.$"

Johto_MahoganyTown_Text_83DFB29::
	.string "MAHOGANY TOWN\p"
	.string "Welcome to the Home of the Ninja$"

Johto_MahoganyTown_Text_83DFB58::
	.string "While visiting MAHOGANY TOWN, try\n"
	.string "a RAGECANDYBAR!$"

Johto_MahoganyTown_Text_83DFB8A::
	.string "MAHOGANY TOWN POKéMON GYM\p"
	.string "LEADER: PRYCE\p"
	.string "The Teacher of Winter’s Harshness$"
