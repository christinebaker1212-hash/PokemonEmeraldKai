Johto_TeamRocketBaseB1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Johto_TeamRocketBaseB1F_EventScript_83E914F
	.byte 0

Johto_TeamRocketBaseB1F_EventScript_83E914F::
	setflag FLAG_QZ_0x10BF
	call_if_eq VAR_QZ_0x501F, 2, Johto_TeamRocketBaseB1F_EventScript_83E918A
	call_if_eq VAR_QZ_0x5020, 2, Johto_TeamRocketBaseB1F_EventScript_83E9194
	call_if_eq VAR_QZ_0x5021, 2, Johto_TeamRocketBaseB1F_EventScript_83E919E
	call_if_eq VAR_QZ_0x5022, 2, Johto_TeamRocketBaseB1F_EventScript_83E91A8
	call_if_eq VAR_QZ_0x5023, 2, Johto_TeamRocketBaseB1F_EventScript_83E91B2
	end

Johto_TeamRocketBaseB1F_EventScript_83E918A::
	setmetatile 0x2D, 1, 0x2A9, 1
	return

Johto_TeamRocketBaseB1F_EventScript_83E9194::
	setmetatile 0x18, 1, 0x2A9, 1
	return

Johto_TeamRocketBaseB1F_EventScript_83E919E::
	setmetatile 0x2D, 6, 0x2A9, 1
	return

Johto_TeamRocketBaseB1F_EventScript_83E91A8::
	setmetatile 0x2D, 0x13, 0x2A9, 1
	return

Johto_TeamRocketBaseB1F_EventScript_83E91B2::
	setmetatile 0x1B, 0x13, 0x2A9, 1
	return

@ Johto_TeamRocketBaseB1F trigger at 47,0 (VAR_TEMP_3 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E91BC::
	setflag FLAG_QZ_0x10BF
	clearflag FLAG_QZ_0x10C0
	clearflag FLAG_QZ_0x10C1
	clearflag FLAG_QZ_0x10C2
	setflag FLAG_QZ_0x10C7
	clearflag FLAG_QZ_0x10C8
	clearflag FLAG_QZ_0x10C9
	clearflag FLAG_QZ_0x10CA
	setvar VAR_QZ_0x5024, 0
	clearflag FLAG_QZ_0x10CD
	clearflag FLAG_QZ_0x10CC
	setflag FLAG_QZ_0x10CB
	setflag FLAG_QZ_0x10CE
	setflag FLAG_QZ_0x10CF
	setflag FLAG_QZ_0x10D0
	setflag FLAG_QZ_0x10D1
	setvar VAR_TEMP_3, 1
	end

Johto_TeamRocketBaseB1F_EventScript_83E91F4::
	clearflag FLAG_QZ_0x10BF
	addobject VAR_TEMP_0
	return

Johto_TeamRocketBaseB1F_EventScript_83E91FB::
	clearflag FLAG_QZ_0x10BF
	addobject VAR_TEMP_0
	return

Johto_TeamRocketBaseB1F_EventScript_83E9202::
	applymovement VAR_TEMP_0, Quetzal_Movement_8360839
	waitmovement 0
	return

@ Johto_TeamRocketBaseB1F trigger at 47,24 (VAR_QZ_0x501F == 0)
@ Johto_TeamRocketBaseB1F trigger at 47,25 (VAR_QZ_0x501F == 0)
Johto_TeamRocketBaseB1F_EventScript_83E920D::
	setvar VAR_TEMP_0, 1
	call Johto_TeamRocketBaseB1F_EventScript_83E91F4
	setvar VAR_QZ_0x501F, 1
	end

@ Johto_TeamRocketBaseB1F trigger at 45,3 (VAR_QZ_0x501F == 0)
@ Johto_TeamRocketBaseB1F trigger at 45,4 (VAR_QZ_0x501F == 0)
Johto_TeamRocketBaseB1F_EventScript_83E921D::
	lock
	setvar VAR_TEMP_0, 1
	call Johto_TeamRocketBaseB1F_EventScript_83E91F4
	call Johto_TeamRocketBaseB1F_EventScript_83E91FB
	call Johto_TeamRocketBaseB1F_EventScript_83E9348
	setvar VAR_QZ_0x501F, 2
	fadescreenswapbuffers 1
	removeobject VAR_TEMP_0
	fadescreenswapbuffers 0
	setmetatile 0x2D, 1, 0x2A9, 1
	special DrawWholeMapView
	release
	end

@ Johto_TeamRocketBaseB1F trigger at 26,24 (VAR_QZ_0x5020 == 0)
@ Johto_TeamRocketBaseB1F trigger at 24,24 (VAR_QZ_0x5020 == 0)
@ Johto_TeamRocketBaseB1F trigger at 26,25 (VAR_QZ_0x5020 == 0)
@ Johto_TeamRocketBaseB1F trigger at 24,25 (VAR_QZ_0x5020 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E924C::
	setvar VAR_TEMP_0, 2
	call Johto_TeamRocketBaseB1F_EventScript_83E91FB
	setvar VAR_QZ_0x5020, 1
	end

@ Johto_TeamRocketBaseB1F trigger at 24,3 (VAR_QZ_0x5020 == 0)
@ Johto_TeamRocketBaseB1F trigger at 24,4 (VAR_QZ_0x5020 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E925C::
	lock
	setvar VAR_TEMP_0, 2
	call Johto_TeamRocketBaseB1F_EventScript_83E91F4
	call Johto_TeamRocketBaseB1F_EventScript_83E91FB
	call Johto_TeamRocketBaseB1F_EventScript_83E93D8
	setvar VAR_QZ_0x5020, 2
	fadescreenswapbuffers 1
	removeobject VAR_TEMP_0
	fadescreenswapbuffers 0
	setmetatile 0x18, 1, 0x2A9, 1
	special DrawWholeMapView
	release
	end

@ Johto_TeamRocketBaseB1F trigger at 45,29 (VAR_QZ_0x5021 == 0)
@ Johto_TeamRocketBaseB1F trigger at 47,29 (VAR_QZ_0x5021 == 0)
@ Johto_TeamRocketBaseB1F trigger at 45,30 (VAR_QZ_0x5021 == 0)
@ Johto_TeamRocketBaseB1F trigger at 47,30 (VAR_QZ_0x5021 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E928B::
	setvar VAR_TEMP_0, 3
	call Johto_TeamRocketBaseB1F_EventScript_83E91F4
	setvar VAR_QZ_0x5021, 1
	end

@ Johto_TeamRocketBaseB1F trigger at 45,8 (VAR_QZ_0x5021 == 0)
@ Johto_TeamRocketBaseB1F trigger at 45,9 (VAR_QZ_0x5021 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E929B::
	lock
	setvar VAR_TEMP_0, 3
	call Johto_TeamRocketBaseB1F_EventScript_83E91F4
	call Johto_TeamRocketBaseB1F_EventScript_83E91FB
	call Johto_TeamRocketBaseB1F_EventScript_83E9348
	setvar VAR_QZ_0x5021, 2
	fadescreenswapbuffers 1
	removeobject VAR_TEMP_0
	fadescreenswapbuffers 0
	setmetatile 0x2D, 6, 0x2A9, 1
	special DrawWholeMapView
	release
	end

@ Johto_TeamRocketBaseB1F trigger at 46,27 (VAR_QZ_0x5022 == 0)
@ Johto_TeamRocketBaseB1F trigger at 44,27 (VAR_QZ_0x5022 == 0)
@ Johto_TeamRocketBaseB1F trigger at 44,28 (VAR_QZ_0x5022 == 0)
@ Johto_TeamRocketBaseB1F trigger at 46,28 (VAR_QZ_0x5022 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E92CA::
	setvar VAR_TEMP_0, 4
	call Johto_TeamRocketBaseB1F_EventScript_83E91F4
	setvar VAR_QZ_0x5022, 1
	end

@ Johto_TeamRocketBaseB1F trigger at 45,21 (VAR_QZ_0x5022 == 0)
@ Johto_TeamRocketBaseB1F trigger at 45,22 (VAR_QZ_0x5022 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E92DA::
	lock
	setvar VAR_TEMP_0, 4
	call Johto_TeamRocketBaseB1F_EventScript_83E91F4
	call Johto_TeamRocketBaseB1F_EventScript_83E91FB
	call Johto_TeamRocketBaseB1F_EventScript_83E9348
	setvar VAR_QZ_0x5022, 2
	fadescreenswapbuffers 1
	removeobject VAR_TEMP_0
	fadescreenswapbuffers 0
	setmetatile 0x2D, 0x13, 0x2A9, 1
	special DrawWholeMapView
	release
	end

@ Johto_TeamRocketBaseB1F trigger at 28,27 (VAR_QZ_0x5023 == 0)
@ Johto_TeamRocketBaseB1F trigger at 26,27 (VAR_QZ_0x5023 == 0)
@ Johto_TeamRocketBaseB1F trigger at 28,28 (VAR_QZ_0x5023 == 0)
@ Johto_TeamRocketBaseB1F trigger at 26,28 (VAR_QZ_0x5023 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E9309::
	setvar VAR_TEMP_0, 4
	call Johto_TeamRocketBaseB1F_EventScript_83E91FB
	setvar VAR_QZ_0x5023, 1
	end

@ Johto_TeamRocketBaseB1F trigger at 27,21 (VAR_QZ_0x5023 == 0)
@ Johto_TeamRocketBaseB1F trigger at 27,22 (VAR_QZ_0x5023 == 0)
Johto_TeamRocketBaseB1F_EventScript_83E9319::
	lock
	setvar VAR_TEMP_0, 4
	call Johto_TeamRocketBaseB1F_EventScript_83E91F4
	call Johto_TeamRocketBaseB1F_EventScript_83E91FB
	call Johto_TeamRocketBaseB1F_EventScript_83E93D8
	setvar VAR_QZ_0x5023, 2
	fadescreenswapbuffers 1
	removeobject VAR_TEMP_0
	fadescreenswapbuffers 0
	setmetatile 0x1B, 0x13, 0x2A9, 1
	special DrawWholeMapView
	release
	end

Johto_TeamRocketBaseB1F_EventScript_83E9348::
	playse SE_WARP_IN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	getplayerxy VAR_TEMP_1, VAR_TEMP_2
	call_if_eq VAR_TEMP_2, 4, Johto_TeamRocketBaseB1F_EventScript_83E9202
	call_if_eq VAR_TEMP_2, 9, Johto_TeamRocketBaseB1F_EventScript_83E9202
	call_if_eq VAR_TEMP_2, 22, Johto_TeamRocketBaseB1F_EventScript_83E9202
	applymovement VAR_TEMP_0, Johto_TeamRocketBaseB1F_Movement_83E9468
	waitmovement 0
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_TeamRocketBaseB1F_Text_83E947A, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4789, 0, Johto_TeamRocketBaseB1F_Text_83E948F
	applymovement VAR_TEMP_0, Johto_TeamRocketBaseB1F_Movement_83E9471
	waitmovement 0
	delay 0x37
	applymovement VAR_TEMP_0, Johto_TeamRocketBaseB1F_Movement_83E9468
	waitmovement 0
	msgbox Johto_TeamRocketBaseB1F_Text_83E949F, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4210, 0, Johto_TeamRocketBaseB1F_Text_83E94C0
	applymovement VAR_TEMP_0, Johto_TeamRocketBaseB1F_Movement_83E9471
	waitmovement 0
	setflag FLAG_QZ_0x10BF
	removeobject VAR_TEMP_0
	removeobject 3
	removeobject 1
	return

Johto_TeamRocketBaseB1F_EventScript_83E93D8::
	playse SE_WARP_IN
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	getplayerxy VAR_TEMP_1, VAR_TEMP_2
	call_if_eq VAR_TEMP_2, 4, Johto_TeamRocketBaseB1F_EventScript_83E9202
	call_if_eq VAR_TEMP_2, 9, Johto_TeamRocketBaseB1F_EventScript_83E9202
	call_if_eq VAR_TEMP_2, 22, Johto_TeamRocketBaseB1F_EventScript_83E9202
	applymovement VAR_TEMP_0, Johto_TeamRocketBaseB1F_Movement_83E9471
	waitmovement 0
	turnobject LOCALID_PLAYER, 4
	msgbox Johto_TeamRocketBaseB1F_Text_83E947A, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4789, 0, Johto_TeamRocketBaseB1F_Text_83E948F
	applymovement VAR_TEMP_0, Johto_TeamRocketBaseB1F_Movement_83E9468
	waitmovement 0
	delay 0x37
	applymovement VAR_TEMP_0, Johto_TeamRocketBaseB1F_Movement_83E9471
	waitmovement 0
	msgbox Johto_TeamRocketBaseB1F_Text_83E949F, MSGBOX_DEFAULT
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4210, 0, Johto_TeamRocketBaseB1F_Text_83E94C0
	applymovement VAR_TEMP_0, Johto_TeamRocketBaseB1F_Movement_83E9468
	waitmovement 0
	setflag FLAG_QZ_0x10BF
	removeobject VAR_TEMP_0
	removeobject 3
	removeobject 1
	return

@ Johto_TeamRocketBaseB1F object 5 (local id 5, gfx 0x3EB4) at 36,15
Johto_TeamRocketBaseB1F_EventScript_83E9523::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4285, 0, Johto_TeamRocketBaseB1F_Text_83E953A, Johto_TeamRocketBaseB1F_Text_83E9589
	msgbox Johto_TeamRocketBaseB1F_Text_83E9593, MSGBOX_AUTOCLOSE
	end

@ Johto_TeamRocketBaseB1F object 9 (local id 9, gfx 0x3EB0) at 19,9
Johto_TeamRocketBaseB1F_EventScript_83E96B9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4810, 0, Johto_TeamRocketBaseB1F_Text_83E9633, Johto_TeamRocketBaseB1F_Text_83E964F
	msgbox Johto_TeamRocketBaseB1F_Text_83E9657, MSGBOX_AUTOCLOSE
	end

@ Johto_TeamRocketBaseB1F object 8 (local id 8, gfx 0x3B) at 42,16
Johto_TeamRocketBaseB1F_EventScript_83E96D0::
	setorcopyvar VAR_0x8000, 0x88
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB1F object 6 (local id 6, gfx 0x3B) at 50,12
Johto_TeamRocketBaseB1F_EventScript_83E96DD::
	setorcopyvar VAR_0x8000, 0x1E
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB1F object 7 (local id 7, gfx 0x3B) at 21,13
Johto_TeamRocketBaseB1F_EventScript_83E96EA::
	setorcopyvar VAR_0x8000, 0x21
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TeamRocketBaseB1F sign / bg event at 45,7
@ Johto_TeamRocketBaseB1F sign / bg event at 45,2
@ Johto_TeamRocketBaseB1F sign / bg event at 24,2
@ Johto_TeamRocketBaseB1F sign / bg event at 27,20
@ Johto_TeamRocketBaseB1F sign / bg event at 45,20
Johto_TeamRocketBaseB1F_EventScript_83E96F7::
	msgbox Johto_TeamRocketBaseB1F_Text_83E9700, MSGBOX_NPC
	end

@ Johto_TeamRocketBaseB1F sign / bg event at 39,1
@ Johto_TeamRocketBaseB1F sign / bg event at 30,1
@ Johto_TeamRocketBaseB1F sign / bg event at 38,6
@ Johto_TeamRocketBaseB1F sign / bg event at 34,19
Johto_TeamRocketBaseB1F_EventScript_83E9717::
	msgbox Johto_TeamRocketBaseB1F_Text_83E97D6, MSGBOX_SIGN
	end

@ Johto_TeamRocketBaseB1F sign / bg event at 31,11
@ Johto_TeamRocketBaseB1F sign / bg event at 38,15
Johto_TeamRocketBaseB1F_EventScript_83E9720::
	lock
	goto_if_set FLAG_QZ_0x10C0, Johto_TeamRocketBaseB1F_EventScript_83E9780
	msgbox Johto_TeamRocketBaseB1F_Text_83E97F7, MSGBOX_DEFAULT
	setvar VAR_QZ_0x501F, 2
	setvar VAR_QZ_0x5020, 2
	setvar VAR_QZ_0x5021, 2
	setvar VAR_QZ_0x5022, 2
	setvar VAR_QZ_0x5023, 2
	setmetatile 0x2D, 1, 0x2A9, 1
	setmetatile 0x18, 1, 0x2A9, 1
	setmetatile 0x2D, 6, 0x2A9, 1
	setmetatile 0x2D, 0x13, 0x2A9, 1
	setmetatile 0x1B, 0x13, 0x2A9, 1
	special DrawWholeMapView
	setflag FLAG_QZ_0x10C0
	release
	end

Johto_TeamRocketBaseB1F_EventScript_83E9780::
	msgbox Johto_TeamRocketBaseB1F_Text_83E9819, MSGBOX_DEFAULT
	setvar VAR_QZ_0x501F, 0
	setvar VAR_QZ_0x5020, 0
	setvar VAR_QZ_0x5021, 0
	setvar VAR_QZ_0x5022, 0
	setvar VAR_QZ_0x5023, 0
	setmetatile 0x2D, 1, 0x2A8, 1
	setmetatile 0x18, 1, 0x2A8, 1
	setmetatile 0x2D, 6, 0x2A8, 1
	setmetatile 0x2D, 0x13, 0x2A8, 1
	setmetatile 0x1B, 0x13, 0x2A8, 1
	special DrawWholeMapView
	clearflag FLAG_QZ_0x10C0
	release
	end

Johto_TeamRocketBaseB1F_Movement_83E9468:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	step_end

Johto_TeamRocketBaseB1F_Movement_83E9471:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_TeamRocketBaseB1F_Text_83E947A::
	.string "Hey!\n"
	.string "Intruder alert!$"

Johto_TeamRocketBaseB1F_Text_83E948F::
	.string "Dang… I failed…$"

Johto_TeamRocketBaseB1F_Text_83E949F::
	.string "It’s my turn! There’s no escape!$"

Johto_TeamRocketBaseB1F_Text_83E94C0::
	.string "Surveillance cams are in the\n"
	.string "POKéMON statues.\p"
	.string "We’ll keep appearing until you\n"
	.string "trip a secret switch.$"

Johto_TeamRocketBaseB1F_Text_83E953A::
	.string "This was once a ninja hideout.\p"
	.string "There are traps to confound\n"
	.string "intruders like you.$"

Johto_TeamRocketBaseB1F_Text_83E9589::
	.string "I get it…$"

Johto_TeamRocketBaseB1F_Text_83E9593::
	.string "All right. I’ll divulge a secret\n"
	.string "about our hideout.\p"
	.string "That thing on the floor up ahead\n"
	.string "is a warp panel.\p"
	.string "If you step on it, you’ll be\n"
	.string "warped back to the entrance.$"

Johto_TeamRocketBaseB1F_Text_83E9633::
	.string "Heheh. Feeling lucky, punk?$"

Johto_TeamRocketBaseB1F_Text_83E964F::
	.string "Kaboom!$"

Johto_TeamRocketBaseB1F_Text_83E9657::
	.string "I don’t even know where the traps\n"
	.string "are planted.\p"
	.string "You’ll just have to collect your\n"
	.string "courage and walk.$"

Johto_TeamRocketBaseB1F_Text_83E9700::
	.string "It’s a PERSIAN statue…$"

Johto_TeamRocketBaseB1F_Text_83E97D6::
	.string "A strange panel... Nothing here.$"

Johto_TeamRocketBaseB1F_Text_83E97F7::
	.string "A secret switch! Better press it.$"

Johto_TeamRocketBaseB1F_Text_83E9819::
	.string "The switch is turned off.$"
