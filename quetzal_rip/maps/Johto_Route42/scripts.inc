Johto_Route42_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route42_EventScript_83F4ACD
	.byte 0

@ Johto_Route42 trigger at 75,6 (VAR_QZ_0x500C == 0)
@ Johto_Route42 trigger at 75,7 (VAR_QZ_0x500C == 0)
@ Johto_Route42 trigger at 75,8 (VAR_QZ_0x500C == 0)
@ Johto_Route42 trigger at 75,9 (VAR_QZ_0x500C == 0)
Johto_Route42_EventScript_83DF6D8::
	setflag FLAG_QZ_0x11C3
	setvar VAR_QZ_0x500C, 1
	end

Johto_Route42_EventScript_83F4ACD::
	call_if_set FLAG_QZ_0x10B9, Quetzal_EventScript_83F4AD7
	end

@ Johto_Route42 trigger at 36,15 (VAR_TEMP_0 == 0)
Johto_Route42_EventScript_83F4ADD::
	lock
	playmoncry SPECIES_SUICUNE, 0
	applymovement 8, Johto_Route42_Movement_83F4BC9
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	waitmovement 0
	waitmoncry
	playse SE_PIN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	delay 0x3C
	applymovement 8, Johto_Route42_Movement_83F4BCC
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	waitmovement 0
	playmoncry SPECIES_SUICUNE, 0
	applymovement 8, Johto_Route42_Movement_83F4BD1
	applymovement LOCALID_PLAYER, Johto_Route42_Movement_83F4BDB
	playse SE_LEDGE
	delay 0x3C
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	delay 0x1E
	playse SE_LEDGE
	waitmovement 8
	waitmovement 0
	delay 0x3C
	removeobject 8
	setflag FLAG_QZ_0x10B9
	clearflag FLAG_QZ_0x10B8
	addobject 10
	playse SE_EXIT
	turnobject LOCALID_PLAYER, 4
	applymovement 10, Johto_Route42_Movement_83F4BE0
	waitmovement 0
	msgbox Johto_Route42_Text_83F4BEF, MSGBOX_DEFAULT
	closemessage
	turnobject 10, 3
	msgbox Johto_Route42_Text_83F4C58, MSGBOX_DEFAULT
	turnobject 10, 4
	msgbox Johto_Route42_Text_83F4D2D, MSGBOX_DEFAULT
	closemessage
	applymovement 10, Johto_Route42_Movement_83F4BE5
	waitmovement 0
	setflag FLAG_QZ_0x10B8
	removeobject 10
	setvar VAR_TEMP_0, 1
	call_if_eq VAR_QZ_0x502D, 2, Johto_Route42_EventScript_83F4BC3
	release
	end

Johto_Route42_EventScript_83F4BC3::
	setvar VAR_QZ_0x502D, 3
	return

@ Johto_Route42 object 4 (local id 4, gfx 0x3E95) at 32,12
Johto_Route42_EventScript_83F4D8A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4103, 0, Johto_Route42_Text_83F4E0E, Johto_Route42_Text_83F4E44
	msgbox Johto_Route42_Text_83F4E72, MSGBOX_AUTOCLOSE
	end

@ Johto_Route42 object 6 (local id 6, gfx 0x37) at 66,9
Johto_Route42_EventScript_83F4DA1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4344, 0, Johto_Route42_Text_83F4ED9, Johto_Route42_Text_83F4F06
	msgbox Johto_Route42_Text_83F4F27, MSGBOX_AUTOCLOSE
	end

@ Johto_Route42 object 5 (local id 5, gfx 0x3EB7) at 63,15
Johto_Route42_EventScript_83F4DB8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4310, 0, Johto_Route42_Text_83F4F61, Johto_Route42_Text_83F4F97
	msgbox Johto_Route42_Text_83F4FB9, MSGBOX_AUTOCLOSE
	end

@ Johto_Route42 sign / bg event at 70,10
@ Johto_Route42 sign / bg event at 12,12
Johto_Route42_EventScript_83F4DCF::
	lock
	msgbox Johto_Route42_Text_83F5020, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route42 sign / bg event at 15,7
@ Johto_Route42 sign / bg event at 59,10
Johto_Route42_EventScript_83F4DDB::
	lock
	msgbox Johto_Route42_Text_83F5047, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route42 object 1 (local id 1, gfx 0x3A9E) at 12,6
Johto_Route42_EventScript_83F4DE7::
	setorcopyvar VAR_0x8000, 0x1D
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route42 object 9 (local id 9, gfx 0x3A9E) at 52,8
Johto_Route42_EventScript_83F4DF4::
	setorcopyvar VAR_0x8000, 0x59
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route42 object 2 (local id 2, gfx 0x3A9E) at 31,9
Johto_Route42_EventScript_83F4E01::
	setorcopyvar VAR_0x8000, 3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route42_Movement_83F4BC9:
	jump_in_place_right_left
	jump_in_place_right_left
	step_end

Johto_Route42_Movement_83F4BCC:
	jump_2_left
	jump_2_left
	jump_2_left
	jump_2_up
	step_end

Johto_Route42_Movement_83F4BD1:
	jump_in_place_up
	delay_2
	face_up
	jump_2_up
	jump_2_right
	jump_2_right
	jump_2_right
	jump_2_right
	jump_2_right
	step_end

Johto_Route42_Movement_83F4BDB:
	lock_facing_direction
	delay_2
	jump_2_up
	unlock_facing_direction
	step_end

Johto_Route42_Movement_83F4BE0:
	walk_down
	walk_down
	walk_left
	face_right
	step_end

Johto_Route42_Movement_83F4BE5:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

Johto_Route42_Text_83F4BEF::
	.string "EUSINE: SUICUNE...\n"
	.string "How brave it is!\p"
	.string "How refreshing it is!\n"
	.string "How beautiful it is!\p"
	.string "And how quickly it moves!$"

Johto_Route42_Text_83F4C58::
	.string "{PLAYER}!\p"
	.string "You seem to be around where\n"
	.string "SUICUNE would appear.\p"
	.string "Well, that’s OK.\l"
	.string "My desire to search for\n"
	.string "SUICUNE is far beyond yours...\p"
	.string "My grandpa was... quite into\n"
	.string "myths.\p"
	.string "I’ve heard so many stories\n"
	.string "about SUICUNE from him.$"

Johto_Route42_Text_83F4D2D::
	.string "SUICUNE...\n"
	.string "I won’t stop following you\l"
	.string "until I’ve found out what\l"
	.string "you’re after...\p"
	.string "You hear me!$"

Johto_Route42_Text_83F4E0E::
	.string "Let me demonstrate the power\n"
	.string "of the POKéMON I caught!$"

Johto_Route42_Text_83F4E44::
	.string "What? That’s not right.\n"
	.string "I can’t believe this!$"

Johto_Route42_Text_83F4E72::
	.string "I want to become the trainer\n"
	.string "CHAMP using the POKéMON I\l"
	.string "caught myself.\p"
	.string "That’s the best part of fishing!$"

Johto_Route42_Text_83F4ED9::
	.string "Ah, it’s good to be outside!\n"
	.string "I feel so free!$"

Johto_Route42_Text_83F4F06::
	.string "Gahahah! That was a good\n"
	.string "battle!$"

Johto_Route42_Text_83F4F27::
	.string "Losing feels insignificant\n"
	.string "if you look up at the big sky!$"

Johto_Route42_Text_83F4F61::
	.string "HEY!\p"
	.string "This is my secret place!\n"
	.string "Get lost, you outsider!$"

Johto_Route42_Text_83F4F97::
	.string "I should have used my\n"
	.string "MOON STONE…$"

Johto_Route42_Text_83F4FB9::
	.string "You’re working on a POKéDEX?\p"
	.string "Wow, you must know some\n"
	.string "pretty rare POKéMON!\p"
	.string "May I please see it?\n"
	.string "Please?$"

Johto_Route42_Text_83F5020::
	.string "ROUTE 42\p"
	.string "ECRUTEAK CITY - MAHOGANY TOWN$"

Johto_Route42_Text_83F5047::
	.string "MT.MORTAR\p"
	.string "WATERFALL CAVE INSIDE$"
