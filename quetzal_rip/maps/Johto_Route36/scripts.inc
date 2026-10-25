Johto_Route36_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route36_EventScript_83F19E0
	.byte 0

Johto_Route36_EventScript_83F19E0::
	end

@ Johto_Route36 object 3 (local id 3, gfx 0x2C) at 46,20
Johto_Route36_EventScript_83F19E1::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102E, Johto_Route36_EventScript_83F19F7
	msgbox Johto_Route36_Text_83F1CAC, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route36_EventScript_83F19F7::
	goto_if_set FLAG_QZ_0x1003, Johto_Route36_EventScript_83F1A0B
	msgbox Johto_Route36_Text_83F1D0D, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x1003

Johto_Route36_EventScript_83F1A0B::
	msgbox Johto_Route36_Text_83F1D57, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route36 object 1 (local id 1, gfx 0x3E85) at 57,20
Johto_Route36_EventScript_83F1A16::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102E, Johto_Route36_EventScript_83F1A2C
	msgbox Johto_Route36_Text_83F1E04, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route36_EventScript_83F1A2C::
	msgbox Johto_Route36_Text_83F1E72, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route36 object 2 (local id 2, gfx 0x3E87) at 47,16
Johto_Route36_EventScript_83F1A37::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102C, Johto_Route36_EventScript_83F1AF7
	msgbox Johto_Route36_Text_83F1FD3, MSGBOX_DEFAULT
	closemessage
	multichoice 0x14, 0, 0x4A, 1
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Johto_Route36_EventScript_83F1AE9
	goto_if_eq VAR_0x8000, 1, Johto_Route36_EventScript_83F1AE9
	goto_if_eq VAR_0x8000, 2, Johto_Route36_EventScript_83F1AE9
	goto_if_eq VAR_0x8000, 3, Johto_Route36_EventScript_83F1AE9
	goto_if_eq VAR_0x8000, 4, Johto_Route36_EventScript_83F1ABC
	goto_if_eq VAR_0x8000, 5, Johto_Route36_EventScript_83F1AE9
	goto_if_eq VAR_0x8000, 6, Johto_Route36_EventScript_83F1AE9
	goto_if_eq VAR_0x8000, 7, Johto_Route36_EventScript_83F1AF4
	goto_if_eq VAR_0x8000, 127, Johto_Route36_EventScript_83F1AF4
	closemessage
	release
	end

Johto_Route36_EventScript_83F1ABC::
	msgbox Johto_Route36_Text_83F204F, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x1B5
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x102C
	msgbox Johto_Route36_Text_83F2068, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route36_EventScript_83F1AE9::
	msgbox Johto_Route36_Text_83F2044, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route36_EventScript_83F1AF4::
	closemessage
	release
	end

Johto_Route36_EventScript_83F1AF7::
	msgbox Johto_Route36_Text_83F20CC, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route36 object 6 (local id 6, gfx 0x1B9) at 39,19
Johto_Route36_EventScript_83F1B02::
	lock
	msgbox Johto_Route36_Text_83F1DE2, MSGBOX_DEFAULT
	checkitem ITEM_SQUIRTBOTTLE, 1
	goto_if_eq VAR_RESULT, 1, Johto_Route36_EventScript_83F1B1E
	closemessage
	release
	end

Johto_Route36_EventScript_83F1B1E::
	msgbox Johto_Route36_Text_83F1C39, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_Route36_EventScript_83F1B34
	closemessage
	release
	end

Johto_Route36_EventScript_83F1B34::
	msgbox Johto_Route36_Text_83F1C4B, MSGBOX_DEFAULT
	playse SE_DOWNPOUR
	waitse
	closemessage
	playse SE_SUDOWOODO_SHAKE
	applymovement 6, Johto_Route36_Movement_83F1C29
	waitmovement 0
	faceplayer
	msgbox Johto_Route36_Text_83F1C65, MSGBOX_DEFAULT
	closemessage
	waitse
	playmoncry SPECIES_SUDOWOODO, 2
	delay 0x28
	waitmoncry
	setvar VAR_LAST_TALKED, 6
	setwildbattle SPECIES_SUDOWOODO, 0x14, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Johto_Route36_EventScript_83F1B99
	goto_if_eq VAR_RESULT, 4, Johto_Route36_EventScript_83F1BAA
	goto_if_eq VAR_RESULT, 5, Johto_Route36_EventScript_83F1BAA

Johto_Route36_EventScript_83F1B99::
	setflag FLAG_QZ_0x102E
	removeobject 6
	call_if_unset FLAG_QZ_0x1003, Johto_Route36_EventScript_83F1BB0
	release
	end

Johto_Route36_EventScript_83F1BAA::
	turnobject 6, 2
	release
	end

Johto_Route36_EventScript_83F1BB0::
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	turnobject 3, 3
	waitmovement 0
	applymovement 3, Johto_Route36_Movement_83F1BDB
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitmovement 0
	return

@ Johto_Route36 object 4 (local id 4, gfx 0x25) at 24,24
Johto_Route36_EventScript_83F1BE3::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4241, 0, Johto_Route36_Text_83F1EC1, Johto_Route36_Text_83F1EE2
	msgbox Johto_Route36_Text_83F1EF1, MSGBOX_AUTOCLOSE
	end

@ Johto_Route36 object 5 (local id 5, gfx 0xB) at 31,23
Johto_Route36_EventScript_83F1BFA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4726, 0, Johto_Route36_Text_83F1F33, Johto_Route36_Text_83F1F64
	msgbox Johto_Route36_Text_83F1F7D, MSGBOX_AUTOCLOSE
	end

@ Johto_Route36 sign / bg event at 54,23
Johto_Route36_EventScript_83F1C11::
	lock
	msgbox Johto_Route36_Text_83F2115, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route36 sign / bg event at 25,16
Johto_Route36_EventScript_83F1C1D::
	lock
	msgbox Johto_Route36_Text_83F2132, MSGBOX_SIGN
	closemessage
	release
	end

Johto_Route36_Movement_83F1BDB:
	jump_in_place_left
	delay_2
	jump_in_place_left
	delay_2
	jump_in_place_left
	delay_2
	jump_in_place_left
	step_end

Johto_Route36_Movement_83F1C29:
	face_right
	delay_8
	face_down
	delay_8
	face_left
	delay_8
	face_down
	delay_8
	face_right
	delay_8
	face_down
	delay_8
	face_left
	delay_8
	face_down
	step_end

Johto_Route36_Text_83F1C39::
	.string "Use SQUIRTBOTTLE?$"

Johto_Route36_Text_83F1C4B::
	.string "{PLAYER} used the SQUIRTBOTTLE.$"

Johto_Route36_Text_83F1C65::
	.string "The weird tree doesn’t like the\n"
	.string "SQUIRTBOTTLE!\p"
	.string "The weird tree attacked!$"

Johto_Route36_Text_83F1CAC::
	.string "Wa-hey!\p"
	.string "I was going to snap that tree\n"
	.string "with my straight-arm punch.\p"
	.string "But I couldn’t! I’m a failure!$"

Johto_Route36_Text_83F1D0D::
	.string "Did you clear that wretched tree?\p"
	.string "I’m impressed!\n"
	.string "I want you to have this.$"

Johto_Route36_Text_83F1D57::
	.string "That happens to be ROCK SMASH.\p"
	.string "You can shatter rocks with just a\n"
	.string "single well-aimed smack.\p"
	.string "If any rocks are in your way,\n"
	.string "just smash ’em up!$"

Johto_Route36_Text_83F1DE2::
	.string "A weird tree is blocking the way.$"

Johto_Route36_Text_83F1E04::
	.string "Be careful at night on this route!\p"
	.string "I saw a POKéMON that looked like\n"
	.string "GROWLITHE, but it had a scary aura\l"
	.string "to it.$"

Johto_Route36_Text_83F1E72::
	.string "That odd tree disappeared without\n"
	.string "a trace.\p"
	.string "Oh! That tree was really a\n"
	.string "POKéMON?$"

Johto_Route36_Text_83F1EC1::
	.string "I’m going to read your thoughts!$"

Johto_Route36_Text_83F1EE2::
	.string "I misread you!$"

Johto_Route36_Text_83F1EF1::
	.string "I’d be strong if only I could tell\n"
	.string "what my opponent was thinking.$"

Johto_Route36_Text_83F1F33::
	.string "Thanks to my studies, I’m ready\n"
	.string "for any POKéMON!$"

Johto_Route36_Text_83F1F64::
	.string "Oops! Computation error?$"

Johto_Route36_Text_83F1F7D::
	.string "Darn. I study five hours a day too.\p"
	.string "There’s more to learning than just\n"
	.string "reading books.$"

Johto_Route36_Text_83F1FD3::
	.string "ARTHUR: Who are you?\n"
	.string "I’m ARTHUR of the WEEKDAY FAMILY.\p"
	.string "If you can guess my favorite day,\n"
	.string "I’ll give you a reward.$"

Johto_Route36_Text_83F2044::
	.string "Not quite!$"

Johto_Route36_Text_83F204F::
	.string "Here. You can have this.$"

Johto_Route36_Text_83F2068::
	.string "ARTHUR: A POKéMON that uses ROCK-\n"
	.string "type moves should hold on to that.\p"
	.string "It pumps up ROCK-type attacks.$"

Johto_Route36_Text_83F20CC::
	.string "ARTHUR: I’m ARTHUR of Thursday.\n"
	.string "I’m the fifth son out of seven.$"

Johto_Route36_Text_83F2115::
	.string "RUINS OF ALPH\n"
	.string "NORTH ENTRANCE$"

Johto_Route36_Text_83F2132::
	.string "TRAINER TIPS\n"
	.string "POKéMON stats vary--even within\l"
	.string "the same species.\p"
	.string "Their stats may be similar at first.\p"
	.string "However, differences will become\n"
	.string "pronounced as the POKéMON grow.$"
