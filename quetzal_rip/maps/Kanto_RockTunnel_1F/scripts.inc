Kanto_RockTunnel_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_RockTunnel_1F_EventScript_8412A94
	.byte 0

Kanto_RockTunnel_1F_EventScript_8412A94::
	setflag FLAG_QZ_0x224C
	end

@ Kanto_RockTunnel_1F sign / bg event at 14,33
Kanto_RockTunnel_1F_EventScript_8412A98::
	msgbox Kanto_RockTunnel_1F_Text_8429CF0, MSGBOX_SIGN
	end

@ Kanto_RockTunnel_1F object 6 (local id 6, gfx 0x3F0C) at 7,5
Kanto_RockTunnel_1F_EventScript_8412AA1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8384, 0, Kanto_RockTunnel_1F_Text_8429A29, Kanto_RockTunnel_1F_Text_8429A4B
	msgbox Kanto_RockTunnel_1F_Text_8429A59, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_1F object 5 (local id 5, gfx 0x3F0C) at 5,20
Kanto_RockTunnel_1F_EventScript_8412AB8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8385, 0, Kanto_RockTunnel_1F_Text_8429AB4, Kanto_RockTunnel_1F_Text_8429AD1
	msgbox Kanto_RockTunnel_1F_Text_8429AFD, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_1F object 4 (local id 4, gfx 0x3F0C) at 20,17
Kanto_RockTunnel_1F_EventScript_8412ACF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8386, 0, Kanto_RockTunnel_1F_Text_8429B3E, Kanto_RockTunnel_1F_Text_8429B6F
	msgbox Kanto_RockTunnel_1F_Text_8429B77, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_1F object 7 (local id 7, gfx 0x3F4B) at 27,10
Kanto_RockTunnel_1F_EventScript_8412AE6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8360, 0, Kanto_RockTunnel_1F_Text_8429B97, Kanto_RockTunnel_1F_Text_8429BB1
	msgbox Kanto_RockTunnel_1F_Text_8429BBC, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_1F object 3 (local id 3, gfx 0x3F31) at 45,24
Kanto_RockTunnel_1F_EventScript_8412AFD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8668, 0, Kanto_RockTunnel_1F_Text_8429BDF, Kanto_RockTunnel_1F_Text_8429C0A
	msgbox Kanto_RockTunnel_1F_Text_8429C1B, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_1F object 2 (local id 2, gfx 0x3F31) at 39,27
Kanto_RockTunnel_1F_EventScript_8412B14::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8667, 0, Kanto_RockTunnel_1F_Text_8429CA8, Kanto_RockTunnel_1F_Text_8429CC7
	msgbox Kanto_RockTunnel_1F_Text_8429CD6, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_1F object 1 (local id 1, gfx 0x3F31) at 26,28
Kanto_RockTunnel_1F_EventScript_8412B2B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8666, 0, Kanto_RockTunnel_1F_Text_8429C3A, Kanto_RockTunnel_1F_Text_8429C64
	msgbox Kanto_RockTunnel_1F_Text_8429C78, MSGBOX_AUTOCLOSE
	end

@ Kanto_RockTunnel_1F object 8 (local id 8, gfx 0x3F0E) at 37,5
Kanto_RockTunnel_1F_EventScript_846BFF0::
	setorcopyvar VAR_0x8000, 0x72
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RockTunnel_1F object 9 (local id 9, gfx 0x3F0E) at 33,22
Kanto_RockTunnel_1F_EventScript_846BFFD::
	setorcopyvar VAR_0x8000, 0x8C
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RockTunnel_1F object 10 (local id 10, gfx 0x3F0E) at 22,22
Kanto_RockTunnel_1F_EventScript_846C00A::
	setorcopyvar VAR_0x8000, 0x78
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_RockTunnel_1F_Text_8429A29::
	.string "This tunnel goes a long way, kid!$"

Kanto_RockTunnel_1F_Text_8429A4B::
	.string "Doh!\n"
	.string "You win!$"

Kanto_RockTunnel_1F_Text_8429A59::
	.string "Watch for ONIX.\n"
	.string "They appear only occasionally.\p"
	.string "High-level ones can put the\n"
	.string "squeeze on you!$"

Kanto_RockTunnel_1F_Text_8429AB4::
	.string "Hmm.\n"
	.string "Maybe I’m lost in here…$"

Kanto_RockTunnel_1F_Text_8429AD1::
	.string "Ease up!\n"
	.string "What am I doing?\l"
	.string "Which way is out?$"

Kanto_RockTunnel_1F_Text_8429AFD::
	.string "That sleeping POKéMON on ROUTE 12\n"
	.string "forced me to take this detour.$"

Kanto_RockTunnel_1F_Text_8429B3E::
	.string "Outsiders like you need to show me\n"
	.string "some respect!$"

Kanto_RockTunnel_1F_Text_8429B6F::
	.string "I give!$"

Kanto_RockTunnel_1F_Text_8429B77::
	.string "You’re talented enough to hike!$"

Kanto_RockTunnel_1F_Text_8429B97::
	.string "POKéMON fight!\n"
	.string "Ready, go!$"

Kanto_RockTunnel_1F_Text_8429BB1::
	.string "Game over!$"

Kanto_RockTunnel_1F_Text_8429BBC::
	.string "Oh well, I’ll get a ZUBAT as I go!$"

Kanto_RockTunnel_1F_Text_8429BDF::
	.string "Eek!\p"
	.string "Don’t try anything funny in the\n"
	.string "dark!$"

Kanto_RockTunnel_1F_Text_8429C0A::
	.string "It was too dark…$"

Kanto_RockTunnel_1F_Text_8429C1B::
	.string "I saw a MACHOP in this tunnel.$"

Kanto_RockTunnel_1F_Text_8429C3A::
	.string "I came this far looking for wild\n"
	.string "POKéMON.$"

Kanto_RockTunnel_1F_Text_8429C64::
	.string "I’m out of POKéMON!$"

Kanto_RockTunnel_1F_Text_8429C78::
	.string "You looked cute and harmless.\n"
	.string "Was I ever wrong!$"

Kanto_RockTunnel_1F_Text_8429CA8::
	.string "You have POKéMON!\n"
	.string "Let’s start!$"

Kanto_RockTunnel_1F_Text_8429CC7::
	.string "You play hard!$"

Kanto_RockTunnel_1F_Text_8429CD6::
	.string "Whew!\n"
	.string "I’m all sweaty now.$"

Kanto_RockTunnel_1F_Text_8429CF0::
	.string "ROCK TUNNEL\n"
	.string "CERULEAN CITY - LAVENDER TOWN$"
