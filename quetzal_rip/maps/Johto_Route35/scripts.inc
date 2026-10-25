Johto_Route35_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_Route35_EventScript_83F1391
	.byte 0

Johto_Route35_EventScript_83F1391::
	special QzSetGiovanniModeFlag14A4_794
	call Quetzal_EventScript_840EA62
	end

@ Johto_Route35 object 5 (local id 5, gfx 0x1F) at 14,23
Johto_Route35_EventScript_83F139A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4433, 0, Johto_Route35_Text_83F14A8, Johto_Route35_Text_83F14E7
	msgbox Johto_Route35_Text_83F14EF, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 object 4 (local id 4, gfx 0x1F) at 22,23
Johto_Route35_EventScript_83F13B1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4722, 0, Johto_Route35_Text_83F1525, Johto_Route35_Text_83F1551
	msgbox Johto_Route35_Text_83F1574, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 object 3 (local id 3, gfx 0x20) at 21,23
Johto_Route35_EventScript_83F13C8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4908, 0, Johto_Route35_Text_83F15A0, Johto_Route35_Text_83F15CD
	msgbox Johto_Route35_Text_83F15E7, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 object 1 (local id 1, gfx 0x20) at 10,38
Johto_Route35_EventScript_83F13DF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4480, 0, Johto_Route35_Text_83F1619, Johto_Route35_Text_83F163B
	msgbox Johto_Route35_Text_83F164F, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 object 2 (local id 2, gfx 0x3EAE) at 18,41
Johto_Route35_EventScript_83F13F6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4840, 0, Johto_Route35_Text_83F167A, Johto_Route35_Text_83F169A
	msgbox Johto_Route35_Text_83F16B2, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 object 7 (local id 7, gfx 0x3E9B) at 18,16
Johto_Route35_EventScript_83F140D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4926, 0, Johto_Route35_Text_83F174C, Johto_Route35_Text_83F176F
	msgbox Johto_Route35_Text_83F1786, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 object 9 (local id 9, gfx 0x24) at 30,7
Johto_Route35_EventScript_83F1424::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4853, 0, Johto_Route35_Text_83F17D8, Johto_Route35_Text_83F1806
	msgbox Johto_Route35_Text_83F1827, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 object 6 (local id 6, gfx 0x3E94) at 15,11
Johto_Route35_EventScript_83F143B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4682, 0, Johto_Route35_Text_83F18A7, Johto_Route35_Text_83F18C9
	msgbox Johto_Route35_Text_83F18EC, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 object 8 (local id 8, gfx 0x3EA9) at 20,6
Johto_Route35_EventScript_83F1452::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4870, 0, Johto_Route35_Text_83F191F, Johto_Route35_Text_83F1945
	msgbox Johto_Route35_Text_83F194D, MSGBOX_AUTOCLOSE
	end

@ Johto_Route35 sign / bg event at 11,46
Johto_Route35_EventScript_83F1469::
	lock
	msgbox Johto_Route35_Text_83F19BE, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route35 sign / bg event at 15,7
Johto_Route35_EventScript_83F1475::
	lock
	msgbox Johto_Route35_Text_83F19C7, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_Route35 object 11 (local id 11, gfx 0x3A9E) at 35,45
Johto_Route35_EventScript_83F1481::
	setorcopyvar VAR_0x8000, 0xED
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route35 object 12 (local id 12, gfx 0x3A9E) at 11,12
Johto_Route35_EventScript_83F148E::
	setorcopyvar VAR_0x8000, 0x88
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_Route35 object 13 (local id 13, gfx 0x3A9E) at 27,17
Johto_Route35_EventScript_83F149B::
	setorcopyvar VAR_0x8000, 0x2C
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_Route35_Text_83F14A8::
	.string "I’ve been getting POKéMON data off\n"
	.string "my radio. I think I’m good.$"

Johto_Route35_Text_83F14E7::
	.string "I give!$"

Johto_Route35_Text_83F14EF::
	.string "Music on the radio changes the\n"
	.string "moods of wild POKéMON.$"

Johto_Route35_Text_83F1525::
	.string "I’m gonna show my girlfriend I’m\n"
	.string "hot stuff!$"

Johto_Route35_Text_83F1551::
	.string "I wish you would have lost for me…$"

Johto_Route35_Text_83F1574::
	.string "I was humiliated in front of my\n"
	.string "girlfriend…$"

Johto_Route35_Text_83F15A0::
	.string "My boyfriend’s weak, so I can’t rely\n"
	.string "on him.$"

Johto_Route35_Text_83F15CD::
	.string "Oh, my! You’re so strong!$"

Johto_Route35_Text_83F15E7::
	.string "I can count on my POKéMON more than\n"
	.string "my boyfriend.$"

Johto_Route35_Text_83F1619::
	.string "Are you going to the GYM? Me too!$"

Johto_Route35_Text_83F163B::
	.string "Oh. I couldn’t win…$"

Johto_Route35_Text_83F164F::
	.string "The GYM BADGES are pretty. I collect\n"
	.string "them.$"

Johto_Route35_Text_83F167A::
	.string "What kinds of BALLS do you use?$"

Johto_Route35_Text_83F169A::
	.string "Yikes! Not fast enough!$"

Johto_Route35_Text_83F16B2::
	.string "Some POKéMON flee right away.\p"
	.string "Try catching them with KURT’s\n"
	.string "FAST BALL.\p"
	.string "Whenever I find a ASPEAR BERRY, I\n"
	.string "take it to KURT.\p"
	.string "He turns it into a custom BALL.$"

Johto_Route35_Text_83F174C::
	.string "Behold my graceful BALL dexterity!$"

Johto_Route35_Text_83F176F::
	.string "Whew! That was a jolt!$"

Johto_Route35_Text_83F1786::
	.string "I was going to dazzle you with my\n"
	.string "prize POKéMON.\p"
	.string "But your prowess electrified me!$"

Johto_Route35_Text_83F17D8::
	.string "I’ll go anywhere if bug POKéMON\n"
	.string "appear there.$"

Johto_Route35_Text_83F1806::
	.string "Huh? I shouldn’t have lost that…$"

Johto_Route35_Text_83F1827::
	.string "My VENONAT won me the Bug-Catching\n"
	.string "Contest at the NATIONAL PARK.$"

Johto_Route35_Text_83F18A7::
	.string "I’m practicing my fire breathing.$"

Johto_Route35_Text_83F18C9::
	.string "Ow! I scorched the tip of my nose!$"

Johto_Route35_Text_83F18EC::
	.string "The POKéMON March on the radio\n"
	.string "lures wild POKéMON.$"

Johto_Route35_Text_83F191F::
	.string "I’m always alert while\n"
	.string "I’m on patrol!$"

Johto_Route35_Text_83F1945::
	.string "Whoops!$"

Johto_Route35_Text_83F194D::
	.string "Exploring these routes is fun.\p"
	.string "But stay alert, OK?$"

Johto_Route35_Text_83F19BE::
	.string "ROUTE 35$"

Johto_Route35_Text_83F19C7::
	.string "NATIONAL PARK GATE$"
