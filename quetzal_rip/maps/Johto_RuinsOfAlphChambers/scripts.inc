Johto_RuinsOfAlphChambers_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83F7F66
	.byte 0

@ Johto_RuinsOfAlphChambers sign / bg event at 20,4
Johto_RuinsOfAlphChambers_EventScript_83F8023::
	lock
	msgbox Johto_RuinsOfAlphChambers_Text_83F85E6, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 4,21
Johto_RuinsOfAlphChambers_EventScript_83F802E::
	lock
	msgbox Johto_RuinsOfAlphChambers_Text_83F86A0, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 20,21
Johto_RuinsOfAlphChambers_EventScript_83F8039::
	lock
	msgbox Johto_RuinsOfAlphChambers_Text_83F8665, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 4,4
Johto_RuinsOfAlphChambers_EventScript_83F8044::
	lock
	msgbox Johto_RuinsOfAlphChambers_Text_83F86DF, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphChambers object 1 (local id 1, gfx 0x3E82) at 24,5
Johto_RuinsOfAlphChambers_EventScript_83F8065::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x102A, Johto_RuinsOfAlphChambers_EventScript_83F808F
	goto_if_set FLAG_QZ_0x1076, Johto_RuinsOfAlphChambers_EventScript_83F8084
	msgbox Johto_RuinsOfAlphChambers_Text_83F8476, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RuinsOfAlphChambers_EventScript_83F8084::
	msgbox Johto_RuinsOfAlphChambers_Text_83F8571, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_RuinsOfAlphChambers_EventScript_83F808F::
	msgbox Johto_RuinsOfAlphChambers_Text_83F85B1, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 21,4
Johto_RuinsOfAlphChambers_EventScript_83F809A::
	fadescreen 1
	setvar VAR_0x8004, 0
	goto_if_set FLAG_QZ_0x1076, Johto_RuinsOfAlphChambers_EventScript_83F80BF
	setvar VAR_0x8005, 0
	special QzStartRuinsOfAlphPuzzle_679
	waitstate
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphChambers_EventScript_83F80C9
	end

Johto_RuinsOfAlphChambers_EventScript_83F80BF::
	setvar VAR_0x8005, 1
	special QzStartRuinsOfAlphPuzzle_679
	waitstate
	end

Johto_RuinsOfAlphChambers_EventScript_83F80C9::
	setflag FLAG_QZ_0x1076
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	setmetatile 0x14, 5, 0x302, 1
	setmetatile 0x15, 5, 0x303, 1
	playse SE_CLICK
	special DrawWholeMapView
	delay 0x1E
	clearflag FLAG_QZ_0x1165
	setvar VAR_QZ_0x501C, 1
	applymovement LOCALID_PLAYER, Johto_RuinsOfAlphChambers_Movement_83F811C
	waitmovement 0
	playse SE_FALL
	delay 0x3C
	warphole MAP_JOHTO_RUINS_OF_ALPH_INNER_CHAMBER
	waitstate
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 5,21
Johto_RuinsOfAlphChambers_EventScript_83F8121::
	fadescreen 1
	setvar VAR_0x8004, 1
	goto_if_set FLAG_QZ_0x1078, Johto_RuinsOfAlphChambers_EventScript_83F8146
	setvar VAR_0x8005, 0
	special QzStartRuinsOfAlphPuzzle_679
	waitstate
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphChambers_EventScript_83F8150
	end

Johto_RuinsOfAlphChambers_EventScript_83F8146::
	setvar VAR_0x8005, 1
	special QzStartRuinsOfAlphPuzzle_679
	waitstate
	end

Johto_RuinsOfAlphChambers_EventScript_83F8150::
	setflag FLAG_QZ_0x1078
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	setmetatile 4, 0x16, 0x302, 1
	setmetatile 5, 0x16, 0x303, 1
	playse SE_CLICK
	special DrawWholeMapView
	delay 0x1E
	clearflag FLAG_QZ_0x1164
	applymovement LOCALID_PLAYER, Johto_RuinsOfAlphChambers_Movement_83F811C
	waitmovement 0
	playse SE_FALL
	delay 0x3C
	warphole MAP_JOHTO_RUINS_OF_ALPH_INNER_CHAMBER
	waitstate
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 21,21
Johto_RuinsOfAlphChambers_EventScript_83F819E::
	fadescreen 1
	setvar VAR_0x8004, 2
	goto_if_set FLAG_QZ_0x1079, Johto_RuinsOfAlphChambers_EventScript_83F81C3
	setvar VAR_0x8005, 0
	special QzStartRuinsOfAlphPuzzle_679
	waitstate
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphChambers_EventScript_83F81CD
	end

Johto_RuinsOfAlphChambers_EventScript_83F81C3::
	setvar VAR_0x8005, 1
	special QzStartRuinsOfAlphPuzzle_679
	waitstate
	end

Johto_RuinsOfAlphChambers_EventScript_83F81CD::
	setflag FLAG_QZ_0x1079
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	setmetatile 0x14, 0x16, 0x302, 1
	setmetatile 0x15, 0x16, 0x303, 1
	playse SE_CLICK
	special DrawWholeMapView
	delay 0x1E
	clearflag FLAG_QZ_0x1166
	applymovement LOCALID_PLAYER, Johto_RuinsOfAlphChambers_Movement_83F811C
	waitmovement 0
	playse SE_FALL
	delay 0x3C
	warphole MAP_JOHTO_RUINS_OF_ALPH_INNER_CHAMBER
	waitstate
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 5,4
Johto_RuinsOfAlphChambers_EventScript_83F821B::
	fadescreen 1
	setvar VAR_0x8004, 3
	goto_if_set FLAG_QZ_0x1077, Johto_RuinsOfAlphChambers_EventScript_83F8240
	setvar VAR_0x8005, 0
	special QzStartRuinsOfAlphPuzzle_679
	waitstate
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphChambers_EventScript_83F824A
	end

Johto_RuinsOfAlphChambers_EventScript_83F8240::
	setvar VAR_0x8005, 1
	special QzStartRuinsOfAlphPuzzle_679
	waitstate
	end

Johto_RuinsOfAlphChambers_EventScript_83F824A::
	setflag FLAG_QZ_0x1077
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	setmetatile 4, 5, 0x302, 1
	setmetatile 5, 5, 0x303, 1
	playse SE_CLICK
	special DrawWholeMapView
	delay 0x1E
	clearflag FLAG_QZ_0x1167
	applymovement LOCALID_PLAYER, Johto_RuinsOfAlphChambers_Movement_83F811C
	waitmovement 0
	playse SE_FALL
	delay 0x3C
	warphole MAP_JOHTO_RUINS_OF_ALPH_INNER_CHAMBER
	waitstate
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 4,1
Johto_RuinsOfAlphChambers_EventScript_83F8298::
	lock
	goto_if_set FLAG_QZ_0x1028, Johto_RuinsOfAlphChambers_EventScript_83F8400
	specialvar VAR_RESULT, QzIsLeadMonHoOh_675
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphChambers_EventScript_83F82BD
	msgbox Johto_RuinsOfAlphChambers_Text_83F8430, MSGBOX_SIGN
	closemessage
	release
	end

Johto_RuinsOfAlphChambers_EventScript_83F82BD::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	msgbox Johto_RuinsOfAlphChambers_Text_83F840B, MSGBOX_DEFAULT
	closemessage
	playse SE_CLICK
	setmetatile 5, 1, 0x308, 0
	special DrawWholeMapView
	setflag FLAG_QZ_0x1028
	release
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 20,18
Johto_RuinsOfAlphChambers_EventScript_83F82F2::
	lock
	goto_if_set FLAG_QZ_0x1029, Johto_RuinsOfAlphChambers_EventScript_83F8400
	specialvar VAR_RESULT, QzIsLeadMonAerodactyl_676
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphChambers_EventScript_83F8317
	msgbox Johto_RuinsOfAlphChambers_Text_83F8430, MSGBOX_SIGN
	closemessage
	release
	end

Johto_RuinsOfAlphChambers_EventScript_83F8317::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	msgbox Johto_RuinsOfAlphChambers_Text_83F840B, MSGBOX_DEFAULT
	closemessage
	playse SE_CLICK
	setmetatile 0x15, 0x12, 0x308, 0
	special DrawWholeMapView
	setflag FLAG_QZ_0x1029
	release
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 20,1
Johto_RuinsOfAlphChambers_EventScript_83F834C::
	lock
	goto_if_set FLAG_QZ_0x102A, Johto_RuinsOfAlphChambers_EventScript_83F8400
	specialvar VAR_RESULT, QzIsLeadMonKabuto_677
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphChambers_EventScript_83F8371
	msgbox Johto_RuinsOfAlphChambers_Text_83F8430, MSGBOX_SIGN
	closemessage
	release
	end

Johto_RuinsOfAlphChambers_EventScript_83F8371::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	msgbox Johto_RuinsOfAlphChambers_Text_83F840B, MSGBOX_DEFAULT
	closemessage
	playse SE_CLICK
	setmetatile 0x15, 1, 0x308, 0
	special DrawWholeMapView
	setflag FLAG_QZ_0x102A
	release
	end

@ Johto_RuinsOfAlphChambers sign / bg event at 4,18
Johto_RuinsOfAlphChambers_EventScript_83F83A6::
	lock
	goto_if_set FLAG_QZ_0x102B, Johto_RuinsOfAlphChambers_EventScript_83F8400
	specialvar VAR_RESULT, QzIsLeadMonOmanyte_678
	goto_if_eq VAR_RESULT, 1, Johto_RuinsOfAlphChambers_EventScript_83F83CB
	msgbox Johto_RuinsOfAlphChambers_Text_83F8430, MSGBOX_SIGN
	closemessage
	release
	end

Johto_RuinsOfAlphChambers_EventScript_83F83CB::
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 1
	setvar VAR_0x8006, 0x14
	setvar VAR_0x8007, 6
	special ShakeCamera
	waitstate
	msgbox Johto_RuinsOfAlphChambers_Text_83F840B, MSGBOX_DEFAULT
	closemessage
	playse SE_CLICK
	setmetatile 5, 0x12, 0x308, 0
	special DrawWholeMapView
	setflag FLAG_QZ_0x102B
	release
	end

Johto_RuinsOfAlphChambers_EventScript_83F8400::
	msgbox Johto_RuinsOfAlphChambers_Text_83F8465, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_RuinsOfAlphChambers object 2 (local id 2, gfx 0x3B) at 3,38
Johto_RuinsOfAlphChambers_EventScript_83F8718::
	setorcopyvar VAR_0x8000, 0x20A
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 3 (local id 3, gfx 0x3B) at 6,38
Johto_RuinsOfAlphChambers_EventScript_83F8725::
	setorcopyvar VAR_0x8000, 0x1AA
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 4 (local id 4, gfx 0x3B) at 3,41
Johto_RuinsOfAlphChambers_EventScript_83F8732::
	setorcopyvar VAR_0x8000, 0x22
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 5 (local id 5, gfx 0x3B) at 6,41
Johto_RuinsOfAlphChambers_EventScript_83F873F::
	setorcopyvar VAR_0x8000, 0x36
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 6 (local id 6, gfx 0x3B) at 19,55
Johto_RuinsOfAlphChambers_EventScript_83F874C::
	setorcopyvar VAR_0x8000, 0x29
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 7 (local id 7, gfx 0x3B) at 22,55
Johto_RuinsOfAlphChambers_EventScript_83F8759::
	setorcopyvar VAR_0x8000, 0x28
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 8 (local id 8, gfx 0x3B) at 19,58
Johto_RuinsOfAlphChambers_EventScript_83F8766::
	setorcopyvar VAR_0x8000, 0x20B
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 9 (local id 9, gfx 0x3B) at 22,58
Johto_RuinsOfAlphChambers_EventScript_83F8773::
	setorcopyvar VAR_0x8000, 0xD9
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 12 (local id 12, gfx 0x3B) at 19,38
Johto_RuinsOfAlphChambers_EventScript_83F8780::
	setorcopyvar VAR_0x8000, 0x29
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 11 (local id 11, gfx 0x3B) at 22,38
Johto_RuinsOfAlphChambers_EventScript_83F878D::
	setorcopyvar VAR_0x8000, 0x27
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 13 (local id 13, gfx 0x3B) at 19,41
Johto_RuinsOfAlphChambers_EventScript_83F879A::
	setorcopyvar VAR_0x8000, 0x208
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 10 (local id 10, gfx 0x3B) at 22,41
Johto_RuinsOfAlphChambers_EventScript_83F87A7::
	setorcopyvar VAR_0x8000, 0x204
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 16 (local id 16, gfx 0x3B) at 3,55
Johto_RuinsOfAlphChambers_EventScript_83F87B4::
	setorcopyvar VAR_0x8000, 0x8F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 15 (local id 15, gfx 0x3B) at 6,55
Johto_RuinsOfAlphChambers_EventScript_83F87C1::
	setorcopyvar VAR_0x8000, 0x90
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 17 (local id 17, gfx 0x3B) at 3,58
Johto_RuinsOfAlphChambers_EventScript_83F87CE::
	setorcopyvar VAR_0x8000, 0x20A
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphChambers object 14 (local id 14, gfx 0x3B) at 6,58
Johto_RuinsOfAlphChambers_EventScript_83F87DB::
	setorcopyvar VAR_0x8000, 0xD4
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_RuinsOfAlphChambers_Movement_83F811C:
	walk_in_place_fast_up
	walk_in_place_fast_up
	walk_in_place_fast_up
	set_invisible
	step_end

Johto_RuinsOfAlphChambers_Text_83F840B::
	.string "The chamber reacts!\n"
	.string "The way is open.$"

Johto_RuinsOfAlphChambers_Text_83F8430::
	.string "Some kind of ancient sensor.\n"
	.string "What is it looking for?$"

Johto_RuinsOfAlphChambers_Text_83F8465::
	.string "The way is open.$"

Johto_RuinsOfAlphChambers_Text_83F8476::
	.string "Welcome to this chamber.\p"
	.string "There are sliding panels that depict\n"
	.string "a POKéMON drawn by the ancients.\p"
	.string "Slide the panels around to form\n"
	.string "the picture.\p"
	.string "To the left is a description of\n"
	.string "the POKéMON.\p"
	.string "Scientists in the lab are examin-\n"
	.string "-ing some newly found patterns.$"

Johto_RuinsOfAlphChambers_Text_83F8571::
	.string "A hole just opened up in the floor!\n"
	.string "I’d better tell the others.$"

Johto_RuinsOfAlphChambers_Text_83F85B1::
	.string "Now a hole in the wall too?!\n"
	.string "This is getting spooky.$"

Johto_RuinsOfAlphChambers_Text_83F85E6::
	.string "A POKéMON that hid on the sea floor.\n"
	.string "Eyes on its back scanned the area.$"

Johto_RuinsOfAlphChambers_Text_83F8665::
	.string "This flying POKéMON attacked its\n"
	.string "prey with saw-like fangs.$"

Johto_RuinsOfAlphChambers_Text_83F86A0::
	.string "This POKéMON drifted in the sea\n"
	.string "by twisting its ten tentacles.$"

Johto_RuinsOfAlphChambers_Text_83F86DF::
	.string "A POKéMON that flew gracefully on\n"
	.string "rainbow-colored wings.$"
