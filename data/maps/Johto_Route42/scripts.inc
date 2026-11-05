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
	@ Moemon Kai: SUICUNE waits here only after the CIANWOOD sighting (VAR_QZ_0x502D 2), as in Crystal (audit case 113)
	goto_if_eq VAR_QZ_0x502D, 2, Johto_Route42_EventScript_SuicuneHere
	setflag FLAG_QZ_0x10B9
	setvar VAR_TEMP_0, 1
	end

Johto_Route42_EventScript_SuicuneHere::
	clearflag FLAG_QZ_0x10B9
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
	setvar VAR_TEMP_0, 1
	setvar VAR_QZ_0x502D, 3
	release
	end

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
	setorcopyvar VAR_0x8000, ITEM_SUPER_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route42 object 9 (local id 9, gfx 0x3A9E) at 52,8
Johto_Route42_EventScript_83F4DF4::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route42 object 2 (local id 2, gfx 0x3A9E) at 31,9
Johto_Route42_EventScript_83F4E01::
	setorcopyvar VAR_0x8000, ITEM_ULTRA_BALL
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

Johto_Route42_Text_83F4E0E::
	.string "Let me demonstrate the power\n"
	.string "of the MOéMON I caught!$"

Johto_Route42_Text_83F4E44::
	.string "What? That’s not right.\n"
	.string "I can’t believe this!$"

Johto_Route42_Text_83F4E72::
	.string "I want to become the trainer\n"
	.string "CHAMP using the MOéMON I\l"
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
	.string "You’re working on a MOéDEX?\p"
	.string "Wow, you must know some\n"
	.string "pretty rare MOéMON!\p"
	.string "May I please see it?\n"
	.string "Please?$"

Johto_Route42_Text_83F5020::
	.string "ROUTE 42\p"
	.string "ECRUTEAK CITY - MAHOGANY TOWN$"

Johto_Route42_Text_83F5047::
	.string "MT.MORTAR\p"
	.string "WATERFALL CAVE INSIDE$"
