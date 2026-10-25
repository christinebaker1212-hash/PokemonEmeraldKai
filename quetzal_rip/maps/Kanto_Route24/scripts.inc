Kanto_Route24_MapScripts::
	.byte 0

@ Kanto_Route24 object 1 (local id 1, gfx 0x3F1E) at 12,15
Kanto_Route24_EventScript_84182FD::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5127, 1, Kanto_Route24_EventScript_8418333
	msgbox Kanto_Route24_Text_8438F25, MSGBOX_DEFAULT
	checkitemspace ITEM_NUGGET, 1
	goto_if_eq VAR_RESULT, 0, Kanto_Route24_EventScript_8418329
	call Kanto_Route24_EventScript_8418396
	release
	end

Kanto_Route24_EventScript_8418329::
	msgbox Kanto_Route24_Text_8438FAB, MSGBOX_DEFAULT
	release
	end

Kanto_Route24_EventScript_8418333::
	msgbox Kanto_Route24_Text_84390DA, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route24 trigger at 10,15 (VAR_QZ_0x5127 == 0)
Kanto_Route24_EventScript_841833D::
	lockall
	setvar VAR_TEMP_1, 0
	goto Kanto_Route24_EventScript_8418355

@ Kanto_Route24 trigger at 11,15 (VAR_QZ_0x5127 == 0)
Kanto_Route24_EventScript_8418349::
	lockall
	setvar VAR_TEMP_1, 1
	goto Kanto_Route24_EventScript_8418355

Kanto_Route24_EventScript_8418355::
	textcolor 0
	call_if_eq VAR_TEMP_1, 0, Kanto_Route24_EventScript_84183DD
	call_if_eq VAR_TEMP_1, 1, Kanto_Route24_EventScript_84183E8
	applymovement LOCALID_PLAYER, Quetzal_Movement_8457AAE
	waitmovement 0
	msgbox Kanto_Route24_Text_8438F25, MSGBOX_DEFAULT
	checkitemspace ITEM_NUGGET, 1
	goto_if_eq VAR_RESULT, 0, Kanto_Route24_EventScript_84183F3
	call Kanto_Route24_EventScript_8418396
	releaseall
	end

Kanto_Route24_EventScript_8418396::
	additem ITEM_NUGGET, 1
	loadword 0, Kanto_Route24_Text_8438F7C
	setorcopyvar VAR_0x8000, 0x87
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	message Kanto_Route24_Text_8438FC4
	waitmessage
	playbgm MUS_RG_ENCOUNTER_ROCKET, 0
	waitbuttonpress
	setvar VAR_LAST_TALKED, 1
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8548, 0, Kanto_Route24_Text_84390C5
	msgbox Kanto_Route24_Text_84390DA, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5127, 1
	return

Kanto_Route24_EventScript_84183DD::
	applymovement 1, Kanto_Route24_Movement_841841E
	waitmovement 0
	return

Kanto_Route24_EventScript_84183E8::
	applymovement 1, Quetzal_Movement_8457AAA
	waitmovement 0
	return

Kanto_Route24_EventScript_84183F3::
	msgbox Kanto_Route24_Text_8438FAB, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Kanto_Route24_Movement_8418423
	waitmovement 0
	call_if_eq VAR_TEMP_1, 0, Kanto_Route24_EventScript_8418413
	release
	end

Kanto_Route24_EventScript_8418413::
	applymovement 1, Kanto_Route24_Movement_8418420
	waitmovement 0
	return

@ Kanto_Route24 object 4 (local id 4, gfx 0x3F5C) at 12,25
Kanto_Route24_EventScript_84599C8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8284, 0, Kanto_Route24_Text_843925F, Kanto_Route24_Text_843927E
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route24_EventScript_84599EF
	msgbox Kanto_Route24_Text_8439290, MSGBOX_AUTOCLOSE
	end

Kanto_Route24_EventScript_84599EF::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8284, 0, Kanto_Route24_Text_8470AFD, Kanto_Route24_Text_843927E
	msgbox Kanto_Route24_Text_8439290, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route24 object 6 (local id 6, gfx 0x3EF1) at 12,31
Kanto_Route24_EventScript_8459A06::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8302, 0, Kanto_Route24_Text_8439304, Kanto_Route24_Text_8439375
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route24_EventScript_8459A2D
	msgbox Kanto_Route24_Text_8439387, MSGBOX_AUTOCLOSE
	end

Kanto_Route24_EventScript_8459A2D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8302, 0, Kanto_Route24_Text_8470B80, Kanto_Route24_Text_8439375
	msgbox Kanto_Route24_Text_8439387, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route24 object 3 (local id 3, gfx 0x3F16) at 10,22
Kanto_Route24_EventScript_8459A44::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8314, 0, Kanto_Route24_Text_8439215, Kanto_Route24_Text_843922F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route24_EventScript_8459A6B
	msgbox Kanto_Route24_Text_843923C, MSGBOX_AUTOCLOSE
	end

Kanto_Route24_EventScript_8459A6B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8314, 0, Kanto_Route24_Text_8470ACA, Kanto_Route24_Text_843922F
	msgbox Kanto_Route24_Text_843923C, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route24 object 5 (local id 5, gfx 0x3F16) at 10,28
Kanto_Route24_EventScript_8459A82::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8315, 0, Kanto_Route24_Text_84392B2, Kanto_Route24_Text_84392D0
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route24_EventScript_8459AA9
	msgbox Kanto_Route24_Text_84392E2, MSGBOX_AUTOCLOSE
	end

Kanto_Route24_EventScript_8459AA9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8315, 0, Kanto_Route24_Text_8470B2B, Kanto_Route24_Text_84392D0
	msgbox Kanto_Route24_Text_84392E2, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route24 object 7 (local id 7, gfx 0x3EF3) at 5,21
Kanto_Route24_EventScript_8459AC0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8335, 0, Kanto_Route24_Text_843915D, Kanto_Route24_Text_843917D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route24_EventScript_8459AE7
	msgbox Kanto_Route24_Text_843918C, MSGBOX_AUTOCLOSE
	end

Kanto_Route24_EventScript_8459AE7::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8335, 0, Kanto_Route24_Text_8470A94, Kanto_Route24_Text_843917D
	msgbox Kanto_Route24_Text_843918C, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route24 object 2 (local id 2, gfx 0x3EF3) at 12,19
Kanto_Route24_EventScript_8459AFE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8336, 0, Kanto_Route24_Text_84391C2, Kanto_Route24_Text_84391E3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route24_EventScript_8459B25
	msgbox Kanto_Route24_Text_84391F3, MSGBOX_AUTOCLOSE
	end

Kanto_Route24_EventScript_8459B25::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8336, 0, Kanto_Route24_Text_8470AB4, Kanto_Route24_Text_84391E3
	msgbox Kanto_Route24_Text_84391F3, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route24 object 8 (local id 8, gfx 0x3F0E) at 11,4
Kanto_Route24_EventScript_846BAFA::
	setorcopyvar VAR_0x8000, 0x1E6
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_Route24_Movement_841841E:
	walk_left
	step_end

Kanto_Route24_Movement_8418420:
	walk_right
	walk_in_place_faster_left
	step_end

Kanto_Route24_Movement_8418423:
	walk_down
	step_end

Kanto_Route24_Text_8438F25::
	.string "Congratulations! You beat our\n"
	.string "five contest TRAINERS!\p"
	.string "You just earned a fabulous prize!$"

Kanto_Route24_Text_8438F7C::
	.string "{PLAYER} received a NUGGET\n"
	.string "from the mystery TRAINER!$"

Kanto_Route24_Text_8438FAB::
	.string "You don’t have any room!$"

Kanto_Route24_Text_8438FC4::
	.string "By the way, how would you like to\n"
	.string "join TEAM ROCKET?\p"
	.string "We’re a group of professional\n"
	.string "criminals specializing in POKéMON!\p"
	.string "Want to join?\p"
	.string "Are you sure?\p"
	.string "Come on, join us!\p"
	.string "I’m telling you to join!\p"
	.string "…Okay, you need convincing!\p"
	.string "I’ll make you an offer you can’t\n"
	.string "refuse!$"

Kanto_Route24_Text_84390C5::
	.string "Arrgh!\n"
	.string "You are good!$"

Kanto_Route24_Text_84390DA::
	.string "With your ability, you’d become\n"
	.string "a top leader in TEAM ROCKET.\p"
	.string "Come on, think of the opportunity!\n"
	.string "Don’t let this chance go to waste.$"

Kanto_Route24_Text_843915D::
	.string "I saw your feat from the grass!$"

Kanto_Route24_Text_843917D::
	.string "I thought not!$"

Kanto_Route24_Text_843918C::
	.string "I hid because the people on the\n"
	.string "bridge frightened me.$"

Kanto_Route24_Text_84391C2::
	.string "Okay! I’m No. 5!\n"
	.string "I’ll stomp you!$"

Kanto_Route24_Text_84391E3::
	.string "Whoa!\n"
	.string "Too much!$"

Kanto_Route24_Text_84391F3::
	.string "I did my best. I have no regrets!$"

Kanto_Route24_Text_8439215::
	.string "I’m No. 4!\n"
	.string "Getting tired?$"

Kanto_Route24_Text_843922F::
	.string "I lost, too!$"

Kanto_Route24_Text_843923C::
	.string "I did my best, so I’ve no regrets!$"

Kanto_Route24_Text_843925F::
	.string "Here’s No. 3!\n"
	.string "I won’t be easy!$"

Kanto_Route24_Text_843927E::
	.string "Ow!\n"
	.string "Stomped flat!$"

Kanto_Route24_Text_8439290::
	.string "I did my best. I have no regrets!$"

Kanto_Route24_Text_84392B2::
	.string "I’m second!\n"
	.string "Now it’s serious!$"

Kanto_Route24_Text_84392D0::
	.string "How could I lose?$"

Kanto_Route24_Text_84392E2::
	.string "I did my best. I have no regrets!$"

Kanto_Route24_Text_8439304::
	.string "People call this the NUGGET\n"
	.string "BRIDGE!\p"
	.string "Beat us five TRAINERS and win\n"
	.string "a fabulous prize!\p"
	.string "Think you got what it takes?$"

Kanto_Route24_Text_8439375::
	.string "Whoo!\n"
	.string "Good stuff!$"

Kanto_Route24_Text_8439387::
	.string "I did my best. I have no regrets!$"

Kanto_Route24_Text_8470A94::
	.string "I saw your feat from the grass!$"

Kanto_Route24_Text_8470AB4::
	.string "Okay!\n"
	.string "I’ll stomp you!$"

Kanto_Route24_Text_8470ACA::
	.string "You always look so busy…\n"
	.string "Aren’t you getting tired?$"

Kanto_Route24_Text_8470AFD::
	.string "You really must love coming to\n"
	.string "NUGGET BRIDGE.$"

Kanto_Route24_Text_8470B2B::
	.string "The time we battled…\p"
	.string "Even though I was the second in\n"
	.string "line, I was the best, wasn’t I?$"

Kanto_Route24_Text_8470B80::
	.string "People call this the NUGGET\n"
	.string "BRIDGE!\p"
	.string "You’ve already beaten us, so you’re\n"
	.string "not allowed to take the challenge…\p"
	.string "…But, you’re welcome to battle with\n"
	.string "us again.$"
