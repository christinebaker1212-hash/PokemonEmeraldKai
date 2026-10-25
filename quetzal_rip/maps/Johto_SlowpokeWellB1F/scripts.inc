Johto_SlowpokeWellB1F_MapScripts::
	.byte 0

@ Johto_SlowpokeWellB1F object 7 (local id 7, gfx 0x3A9E) at 22,6
Johto_SlowpokeWellB1F_EventScript_83F9B57::
	setorcopyvar VAR_0x8000, 0x1D
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SlowpokeWellB1F object 11 (local id 11, gfx 0x3A9E) at 12,15
Johto_SlowpokeWellB1F_EventScript_83F9B64::
	setorcopyvar VAR_0x8000, 0x1D
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SlowpokeWellB1F object 12 (local id 12, gfx 0x3A9E) at 21,15
Johto_SlowpokeWellB1F_EventScript_83F9B71::
	setorcopyvar VAR_0x8000, 2
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SlowpokeWellB1F object 13 (local id 13, gfx 0x3A9E) at 23,21
Johto_SlowpokeWellB1F_EventScript_83F9B7E::
	setorcopyvar VAR_0x8000, 0x30
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SlowpokeWellB1F object 1 (local id 1, gfx 0x3EC3) at 27,26
Johto_SlowpokeWellB1F_EventScript_83F9B8B::
	lock
	msgbox Johto_SlowpokeWellB1F_Text_83F9C67, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_SlowpokeWellB1F object 2 (local id 2, gfx 0x3EB0) at 28,8
Johto_SlowpokeWellB1F_EventScript_83F9B97::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4217, 0, Johto_SlowpokeWellB1F_Text_83F9DE0, Johto_SlowpokeWellB1F_Text_83F9E85
	msgbox Johto_SlowpokeWellB1F_Text_83F9EA0, MSGBOX_AUTOCLOSE
	end

@ Johto_SlowpokeWellB1F object 3 (local id 3, gfx 0x3EAF) at 21,5
Johto_SlowpokeWellB1F_EventScript_83F9BAE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4218, 0, Johto_SlowpokeWellB1F_Text_83FA178, Johto_SlowpokeWellB1F_Text_83FA1AF
	msgbox Johto_SlowpokeWellB1F_Text_83FA1C0, MSGBOX_AUTOCLOSE
	end

@ Johto_SlowpokeWellB1F object 5 (local id 5, gfx 0x3EB0) at 13,7
Johto_SlowpokeWellB1F_EventScript_83F9BC5::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4859, 0, Johto_SlowpokeWellB1F_Text_83FA0A6, Johto_SlowpokeWellB1F_Text_83FA116
	msgbox Johto_SlowpokeWellB1F_Text_83FA129, MSGBOX_AUTOCLOSE
	end

@ Johto_SlowpokeWellB1F object 4 (local id 4, gfx 0x3EAC) at 14,3
Johto_SlowpokeWellB1F_EventScript_83F9BDC::
	lock
	faceplayer
	msgbox Johto_SlowpokeWellB1F_Text_83F9F44, MSGBOX_DEFAULT
	closemessage
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4958, 0, Johto_SlowpokeWellB1F_Text_83F9F89
	msgbox Johto_SlowpokeWellB1F_Text_83F9FB4, MSGBOX_AUTOCLOSE
	closemessage
	fadescreen 1
	setflag FLAG_QZ_0x1084
	setflag FLAG_QZ_0x108E
	removeobject 2
	removeobject 3
	removeobject 4
	removeobject 5
	removeobject 1
	removeobject 8
	removeobject 9
	clearflag FLAG_QZ_0x1088
	addobject 10
	hideobjectat 10, MAP_JOHTO_SLOWPOKE_WELL_B1F
	delay 0x78
	fadescreen 0
	special DrawWholeMapView
	showobjectat 10, MAP_JOHTO_SLOWPOKE_WELL_B1F
	turnobject LOCALID_PLAYER, 1
	applymovement 10, Johto_SlowpokeWellB1F_Movement_83F9C60
	waitmovement 0
	msgbox Johto_SlowpokeWellB1F_Text_83F9D83, MSGBOX_DEFAULT
	closemessage
	fadescreen 1
	setflag FLAG_QZ_0x1088
	clearflag FLAG_QZ_0x1089
	clearflag FLAG_QZ_0x1085
	setvar VAR_QZ_0x5009, 3
	warp MAP_JOHTO_KURTS_HOUSE, 255, 15, 4
	waitstate
	end

Johto_SlowpokeWellB1F_Movement_83F9C60:
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	step_end

Johto_SlowpokeWellB1F_Text_83F9C67::
	.string "KURT: Hey there, {PLAYER}!\p"
	.string "The guard up top took off when\n"
	.string "I shouted at him.\p"
	.string "But then I took a tumble down\n"
	.string "the WELL.\p"
	.string "I slammed down hard on my back,\n"
	.string "so I can’t move.\p"
	.string "Rats! If I were fit, my POKéMON\n"
	.string "would’ve punished them…\p"
	.string "Ah, it can’t be helped.\p"
	.string "{PLAYER}, show them how gutsy\n"
	.string "you are in my place!$"

Johto_SlowpokeWellB1F_Text_83F9D83::
	.string "KURT: Way to go, {PLAYER}!\p"
	.string "TEAM ROCKET has taken off.\p"
	.string "My back’s better too.\n"
	.string "Let’s get out of here.$"

Johto_SlowpokeWellB1F_Text_83F9DE0::
	.string "Darn! I was standing guard up\n"
	.string "top when some old coot yelled\l"
	.string "at me.\p"
	.string "He startled me so much that\n"
	.string "I fell down here.\p"
	.string "I think I’ll vent my anger by\n"
	.string "taking it out on you!$"

Johto_SlowpokeWellB1F_Text_83F9E85::
	.string "Arrgh! This is NOT my day!$"

Johto_SlowpokeWellB1F_Text_83F9EA0::
	.string "Sure, we’ve been hacking the\n"
	.string "tails off SLOWPOKE and selling\l"
	.string "them.\p"
	.string "Everything we do is for profit.\p"
	.string "That’s right! We’re TEAM ROCKET,\n"
	.string "and we’ll do anything for money!$"

Johto_SlowpokeWellB1F_Text_83F9F44::
	.string "What do you want?\p"
	.string "If you interrupt our work,\n"
	.string "don’t expect any mercy!$"

Johto_SlowpokeWellB1F_Text_83F9F89::
	.string "You did OK today,\n"
	.string "but wait till next time!$"

Johto_SlowpokeWellB1F_Text_83F9FB4::
	.string "Yeah, TEAM ROCKET was broken up\n"
	.string "three years ago.\p"
	.string "The boss went into hiding.\p"
	.string "But we continued our activities\n"
	.string "underground!\p"
	.string "A small obstacle like you won’t be\n"
	.string "much of a problem for our mission.\p"
	.string "I advise you to be very afraid of\n"
	.string "what is to come!$"

Johto_SlowpokeWellB1F_Text_83FA0A6::
	.string "Hyuck-hyuck-hyuck!\n"
	.string "Quit taking SLOWPOKETAILS?\p"
	.string "You’re nuts!\n"
	.string "If we obeyed you, TEAM ROCKET’s\l"
	.string "rep would be ruined!$"

Johto_SlowpokeWellB1F_Text_83FA116::
	.string "Hyuck…\n"
	.string "Too strong…$"

Johto_SlowpokeWellB1F_Text_83FA129::
	.string "We need the money,\n"
	.string "but selling SLOWPOKETAILS?\p"
	.string "It’s tough being a ROCKET GRUNT!$"

Johto_SlowpokeWellB1F_Text_83FA178::
	.string "Stop taking TAILS?\p"
	.string "Yeah, just try to defeat all\n"
	.string "of us!$"

Johto_SlowpokeWellB1F_Text_83FA1AF::
	.string "You rotten brat!$"

Johto_SlowpokeWellB1F_Text_83FA1C0::
	.string "SLOWPOKETAILS grow back fast!\p"
	.string "What’s wrong with selling them?$"
