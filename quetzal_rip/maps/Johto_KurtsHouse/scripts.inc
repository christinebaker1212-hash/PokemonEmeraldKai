Johto_KurtsHouse_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_KurtsHouse_OnFrame_83C5A2F
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_KurtsHouse_EventScript_83C5A23
	.byte 0

Johto_KurtsHouse_OnFrame_83C5A2F:
	map_script_2 VAR_TEMP_0, 1, Johto_KurtsHouse_EventScript_83C5B29
	.2byte 0

Johto_KurtsHouse_EventScript_83C5A23::
	goto_if_eq VAR_QZ_0x5009, 3, Johto_KurtsHouse_EventScript_83C5B1F
	end

@ Johto_KurtsHouse object 1 (local id 1, gfx 0x3E9F) at 4,3
Johto_KurtsHouse_EventScript_83C5A39::
	lock
	faceplayer
	msgbox Johto_KurtsHouse_Text_83C5D0F, MSGBOX_DEFAULT
	closemessage
	goto_if_eq VAR_FACING, 2, Johto_KurtsHouse_EventScript_83C5A6C
	applymovement 1, Johto_KurtsHouse_Movement_83C5CE6
	waitmovement 0
	playse SE_EXIT
	removeobject 1
	setflag FLAG_QZ_0x1088
	setflag FLAG_QZ_0x108A
	setvar VAR_QZ_0x5009, 2
	release
	end

Johto_KurtsHouse_EventScript_83C5A6C::
	applymovement LOCALID_PLAYER, Johto_KurtsHouse_Movement_83C5CE1
	waitmovement 0
	applymovement 1, Johto_KurtsHouse_Movement_83C5CE6
	waitmovement 0
	playse SE_EXIT
	removeobject 1
	setflag FLAG_QZ_0x1088
	setflag FLAG_QZ_0x108A
	setvar VAR_QZ_0x5009, 2
	release
	end

@ Johto_KurtsHouse object 2 (local id 2, gfx 0x10) at 10,5
Johto_KurtsHouse_EventScript_83C5A93::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x1089, Johto_KurtsHouse_EventScript_83C5AB1
	goto_if_set FLAG_QZ_0x1088, Johto_KurtsHouse_EventScript_83C5AFE
	msgbox Johto_KurtsHouse_Text_83C61ED, MSGBOX_DEFAULT
	release
	end

Johto_KurtsHouse_EventScript_83C5AB1::
	goto_if_eq VAR_QZ_0x5009, 3, Johto_KurtsHouse_EventScript_83C5B09
	goto_if_eq VAR_QZ_0x5009, 4, Johto_KurtsHouse_EventScript_83C5B09
	goto_if_eq VAR_QZ_0x5009, 5, Johto_KurtsHouse_EventScript_83C5B09
	goto_if_eq VAR_QZ_0x5009, 6, Johto_KurtsHouse_EventScript_83C5B09
	goto_if_eq VAR_QZ_0x5009, 8, Johto_KurtsHouse_EventScript_83C5B14
	goto_if_eq VAR_QZ_0x5009, 9, Johto_KurtsHouse_EventScript_83C5B14
	msgbox Johto_KurtsHouse_Text_83C628D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_KurtsHouse_EventScript_83C5AFE::
	msgbox Johto_KurtsHouse_Text_83C6229, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_KurtsHouse_EventScript_83C5B09::
	msgbox Johto_KurtsHouse_Text_83C6248, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_KurtsHouse_EventScript_83C5B14::
	msgbox Johto_KurtsHouse_Text_83C634E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_KurtsHouse_EventScript_83C5B1F::
	setobjectmovementtype 3, 8
	setvar VAR_TEMP_0, 1
	end

Johto_KurtsHouse_EventScript_83C5B29::
	lock
	setvar VAR_TEMP_0, 0
	turnobject LOCALID_PLAYER, 2

Johto_KurtsHouse_EventScript_83C5B33::
	msgbox Johto_KurtsHouse_Text_83C5EBA, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x14
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	msgbox Johto_KurtsHouse_Text_83C5F6D, MSGBOX_DEFAULT
	closemessage
	turnobject 3, 2
	setvar VAR_QZ_0x5009, 4
	release
	end

Johto_KurtsHouse_EventScript_83C5B66::
	checkitem ITEM_GS_BALL, 1
	goto_if_eq VAR_RESULT, 1, Johto_KurtsHouse_EventScript_83C5C0A
	return

@ Johto_KurtsHouse object 3 (local id 3, gfx 0x3E9F) at 15,3
Johto_KurtsHouse_EventScript_83C5B77::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5009, 3, Johto_KurtsHouse_EventScript_83C5B33
	goto_if_eq VAR_QZ_0x5009, 8, Johto_KurtsHouse_EventScript_83C5C6D
	goto_if_eq VAR_QZ_0x5009, 9, Johto_KurtsHouse_EventScript_83C5C7D
	call_if_unset FLAG_QZ_0x1150, Johto_KurtsHouse_EventScript_83C5B66
	goto Johto_KurtsHouse_EventScript_83C5BA8

Johto_KurtsHouse_EventScript_83C5BA8::
	message Johto_KurtsHouse_Text_83C5BD4
	waitmessage

Johto_KurtsHouse_EventScript_83C5BAE::
	pokemart Johto_KurtsHouse_Mart_83C5BC0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

Johto_KurtsHouse_EventScript_83C5C0A::
	msgbox Johto_KurtsHouse_Text_83C60AE, MSGBOX_DEFAULT
	closemessage
	removeitem ITEM_GS_BALL, 1
	playse SE_BANG
	applymovement 3, Johto_KurtsHouse_Movement_83C5CDA
	waitmovement 0
	playse SE_BANG
	applymovement 3, Johto_KurtsHouse_Movement_83C5CDA
	waitmovement 0
	faceplayer
	msgbox Johto_KurtsHouse_Text_83C615D, MSGBOX_DEFAULT
	closemessage
	playse SE_M_PETAL_DANCE
	waitse
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	delay 0x3C
	waitse
	playse SE_M_PETAL_DANCE
	waitse
	applymovement 3, Johto_KurtsHouse_Movement_83C5CD0
	applymovement LOCALID_PLAYER, Johto_KurtsHouse_Movement_83C5CD0
	waitmovement 0
	waitmovement 0

Johto_KurtsHouse_EventScript_83C5C6D::
	msgbox Johto_KurtsHouse_Text_83C61B6, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5009, 8
	closemessage
	release
	end

Johto_KurtsHouse_EventScript_83C5C7D::
	msgbox Johto_KurtsHouse_Text_83C6377, MSGBOX_DEFAULT
	closemessage
	delay 0x1E
	msgbox Johto_KurtsHouse_Text_83C5C99, MSGBOX_DEFAULT
	closemessage
	goto Johto_KurtsHouse_EventScript_83C5BAE

@ Johto_KurtsHouse sign / bg event at 10,1
@ Johto_KurtsHouse sign / bg event at 9,1
Johto_KurtsHouse_EventScript_83C5CBC::
	msgbox Johto_KurtsHouse_Text_83C63C1, MSGBOX_SIGN
	closemessage
	end

@ Johto_KurtsHouse sign / bg event at 5,1
Johto_KurtsHouse_EventScript_83C5CC6::
	msgbox Johto_KurtsHouse_Text_83C63D5, MSGBOX_SIGN
	closemessage
	end

Johto_KurtsHouse_Mart_83C5BC0:
	.2byte ITEM_SPORT_BALL
	.2byte ITEM_LEVEL_BALL
	.2byte ITEM_LURE_BALL
	.2byte ITEM_MOON_BALL
	.2byte ITEM_FRIEND_BALL
	.2byte ITEM_LOVE_BALL
	.2byte ITEM_FAST_BALL
	.2byte ITEM_HEAVY_BALL
	.2byte ITEM_DREAM_BALL
	.2byte ITEM_NONE

Johto_KurtsHouse_Movement_83C5CD0:
	face_left
	delay_8
	face_right
	delay_8
	face_up
	delay_8
	face_down
	delay_8
	face_left
	step_end

Johto_KurtsHouse_Movement_83C5CDA:
	face_up
	walk_in_place_fast_up
	delay_8
	walk_in_place_fast_up
	delay_8
	walk_in_place_fast_up
	step_end

Johto_KurtsHouse_Movement_83C5CE1:
	face_right
	lock_facing_direction
	walk_left
	unlock_facing_direction
	step_end

Johto_KurtsHouse_Movement_83C5CE6:
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	walk_fast_down
	step_end

Johto_KurtsHouse_Text_83C5BD4::
	.string "I now sell special POKé BALLS.\n"
	.string "Which one do you need?$"

Johto_KurtsHouse_Text_83C5C99::
	.string "Anyway, can I make you some BALLS?$"

Johto_KurtsHouse_Text_83C5D0F::
	.string "Hm? Who are you?\p"
	.string "{PLAYER}, eh? You want me to make\n"
	.string "some BALLS?\p"
	.string "Sorry, but that’ll have to wait.\p"
	.string "Do you know TEAM ROCKET?\n"
	.string "Ah, don’t worry.\p"
	.string "I’ll tell you anyhow.\p"
	.string "TEAM ROCKET’s an evil gang that\n"
	.string "uses POKéMON for their dirty work.\p"
	.string "They’re supposed to have disbanded\n"
	.string "three years ago.\p"
	.string "Anyway, they’re at the WELL,\n"
	.string "cutting off SLOWPOKETAILS to sell!\p"
	.string "So I’m going to go give them a\n"
	.string "lesson in pain!\p"
	.string "Hang on, SLOWPOKE! Old KURT\n"
	.string "is on his way!$"

Johto_KurtsHouse_Text_83C5EBA::
	.string "KURT: Hi, {PLAYER}!\p"
	.string "You handled yourself like a\n"
	.string "real hero at the WELL.\p"
	.string "I like your style!\p"
	.string "I would be honored to make\n"
	.string "BALLS for a trainer like you.\p"
	.string "This is all I have now, \n"
	.string "but take it.$"

Johto_KurtsHouse_Text_83C5F6D::
	.string "KURT: I make BALLS from BERRIES.\p"
	.string "Collect them from trees and bring\n"
	.string "’em to me in batches of 5.\p"
	.string "I’ll make BALLS out of them.$"

Johto_KurtsHouse_Text_83C60AE::
	.string "Wh-what is that?\p"
	.string "I’ve never seen one before.\p"
	.string "It looks a like a POKé BALL, but\n"
	.string "it appears to be something else.\p"
	.string "Let me check it for you.$"

Johto_KurtsHouse_Text_83C615D::
	.string "{PLAYER}!\p"
	.string "This BALL started to shake\n"
	.string "while I was checking it.\p"
	.string "There must be something to this!$"

Johto_KurtsHouse_Text_83C61B6::
	.string "What was that sound?!\p"
	.string "Let’s go outside to investigate!$"

Johto_KurtsHouse_Text_83C61ED::
	.string "The SLOWPOKE are gone… \n"
	.string "Were they taken away by bad people?$"

Johto_KurtsHouse_Text_83C6229::
	.string "Grandpa’s gone… I’m so lonely…$"

Johto_KurtsHouse_Text_83C6248::
	.string "The SLOWPOKE my dad gave me came back!\p"
	.string "Its TAIL is growing back too!$"

Johto_KurtsHouse_Text_83C628D::
	.string "Dad works at SILPH where\n"
	.string "he studies POKé BALLS.\p"
	.string "I have to stay home\n"
	.string "with Grandpa and SLOWPOKE.$"

Johto_KurtsHouse_Text_83C634E::
	.string "Did that sound come from\n"
	.string "ILEX FOREST????$"

Johto_KurtsHouse_Text_83C6377::
	.string "Did you check ILEX FOREST yet?\n"
	.string "It sounds very restless.$"

Johto_KurtsHouse_Text_83C63C1::
	.string "…A young PROF. OAK?$"

Johto_KurtsHouse_Text_83C63D5::
	.string "It’s a statue of the forest’s\n"
	.string "protector.$"
