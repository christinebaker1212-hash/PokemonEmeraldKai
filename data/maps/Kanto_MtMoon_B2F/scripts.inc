Kanto_MtMoon_B2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_MtMoon_B2F_EventScript_840FFDF
	.byte 0

Kanto_MtMoon_B2F_EventScript_840FFDF::
	call_if_unset FLAG_QZ_0x2149, Kanto_MtMoon_B2F_EventScript_840FFEE
	call Quetzal_EventScript_840FBB5
	end

Kanto_MtMoon_B2F_EventScript_840FFEE::
	clearflag FLAG_QZ_0x2185
	clearflag FLAG_QZ_0x218C
	return

@ Kanto_MtMoon_B2F trigger at 14,11 (VAR_QZ_0x5115 == 0)
Kanto_MtMoon_B2F_EventScript_840FFF5::
	lockall
	applymovement 3, Quetzal_Movement_8457AAE
	waitmovement 0
	call Kanto_MtMoon_B2F_EventScript_8410022
	releaseall
	end

@ Kanto_MtMoon_B2F object 3 (local id 3, gfx 0x3F43) at 13,11
Kanto_MtMoon_B2F_EventScript_8410007::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2149, Kanto_MtMoon_B2F_EventScript_841004C
	checktrainerflag 0x20AA
	goto_if_eq Kanto_MtMoon_B2F_EventScript_8410056
	call Kanto_MtMoon_B2F_EventScript_8410022
	release
	end

Kanto_MtMoon_B2F_EventScript_8410022::
	textcolor 0
	message Kanto_MtMoon_B2F_Text_842314B
	waitmessage
	playbgm MUS_RG_ENCOUNTER_GYM_LEADER, 0
	waitbuttonpress
	setvar VAR_LAST_TALKED, 3
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8362, 0, Kanto_MtMoon_B2F_Text_8423180
	setvar VAR_QZ_0x5115, 1
	msgbox Kanto_MtMoon_B2F_Text_8423192, MSGBOX_DEFAULT
	return

Kanto_MtMoon_B2F_EventScript_841004C::
	msgbox Kanto_MtMoon_B2F_Text_84231E2, MSGBOX_DEFAULT
	release
	end

Kanto_MtMoon_B2F_EventScript_8410056::
	msgbox Kanto_MtMoon_B2F_Text_8423192, MSGBOX_DEFAULT
	release
	end

@ Kanto_MtMoon_B2F object 1 (local id 1, gfx 0x3F07) at 13,7
Kanto_MtMoon_B2F_EventScript_8410060::
	lock
	faceplayer
	msgbox Kanto_MtMoon_B2F_Text_842345E, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_MtMoon_B2F_EventScript_8410147
	additem ITEM_DOME_FOSSIL, 1
	goto_if_eq VAR_RESULT, 0, Kanto_MtMoon_B2F_EventScript_841013F
	removeobject 1
	loadword 0, Kanto_MtMoon_B2F_Text_84234B4
	setorcopyvar VAR_0x8000, 0xA6
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	closemessage
	special QzQuestLogNop
	delay 0xA
	applymovement 3, Kanto_MtMoon_B2F_Movement_84100CD
	waitmovement 0
	copyobjectxytoperm 3
	textcolor 0
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Kanto_MtMoon_B2F_Text_84231BD
	waitmessage
	waitfanfare
	removeobject 2
	setflag FLAG_QZ_0x2144
	setflag FLAG_QZ_0x2149
	release
	end

@ Kanto_MtMoon_B2F object 2 (local id 2, gfx 0x3F07) at 14,7
Kanto_MtMoon_B2F_EventScript_84100D2::
	lock
	faceplayer
	msgbox Kanto_MtMoon_B2F_Text_842347B, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_MtMoon_B2F_EventScript_8410147
	additem ITEM_HELIX_FOSSIL, 1
	goto_if_eq VAR_RESULT, 0, Kanto_MtMoon_B2F_EventScript_841013F
	removeobject 2
	loadword 0, Kanto_MtMoon_B2F_Text_8423499
	setorcopyvar VAR_0x8000, 0xA5
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x212
	call Quetzal_EventScript_845571E
	closemessage
	special QzQuestLogNop
	delay 0xA
	applymovement 3, Kanto_MtMoon_B2F_Movement_8410149
	waitmovement 0
	copyobjectxytoperm 3
	textcolor 0
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message Kanto_MtMoon_B2F_Text_84231BD
	waitmessage
	waitfanfare
	removeobject 1
	setflag FLAG_QZ_0x214C
	setflag FLAG_QZ_0x2149
	release
	end

Kanto_MtMoon_B2F_EventScript_841013F::
	msgbox Quetzal_Text_8440C0A, MSGBOX_DEFAULT

Kanto_MtMoon_B2F_EventScript_8410147::
	release
	end

@ Kanto_MtMoon_B2F object 5 (local id 5, gfx 0x3F3D) at 12,20
Kanto_MtMoon_B2F_EventScript_841014D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8543, 0, Kanto_MtMoon_B2F_Text_8423241, Kanto_MtMoon_B2F_Text_842329F
	msgbox Kanto_MtMoon_B2F_Text_84232B2, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_B2F object 7 (local id 7, gfx 0x3F3D) at 18,27
Kanto_MtMoon_B2F_EventScript_8410164::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8544, 0, Kanto_MtMoon_B2F_Text_84232E3, Kanto_MtMoon_B2F_Text_842332D
	msgbox Kanto_MtMoon_B2F_Text_8423338, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_B2F object 6 (local id 6, gfx 0x3F3D) at 35,12
Kanto_MtMoon_B2F_EventScript_841017B::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8545, 0, Kanto_MtMoon_B2F_Text_8423369, Kanto_MtMoon_B2F_Text_8423396
	msgbox Kanto_MtMoon_B2F_Text_84233A8, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_B2F object 4 (local id 4, gfx 0x3F3D) at 37,21
Kanto_MtMoon_B2F_EventScript_8410192::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8546, 0, Kanto_MtMoon_B2F_Text_84233D7, Kanto_MtMoon_B2F_Text_8423425
	msgbox Kanto_MtMoon_B2F_Text_8423432, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_B2F object 8 (local id 8, gfx 0x3F0E) at 30,26
Kanto_MtMoon_B2F_EventScript_846BAD3::
	setorcopyvar VAR_0x8000, ITEM_STAR_PIECE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtMoon_B2F object 9 (local id 9, gfx 0x3F0E) at 35,5
Kanto_MtMoon_B2F_EventScript_846BAE0::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtMoon_B2F object 10 (local id 10, gfx 0x3F0E) at 24,6
Kanto_MtMoon_B2F_EventScript_846BFBC::
	setorcopyvar VAR_0x8000, ITEM_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtMoon_B2F object 11 (local id 11, gfx 0x3F0E) at 3,11
Kanto_MtMoon_B2F_EventScript_846BFC9::
	setorcopyvar VAR_0x8000, ITEM_ANTIDOTE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_MtMoon_B2F_Movement_84100CD:
	walk_right
	walk_up
	walk_up
	walk_up
	step_end

Kanto_MtMoon_B2F_Movement_8410149:
	walk_up
	walk_up
	walk_up
	step_end

Kanto_MtMoon_B2F_Text_842314B::
	.string "Hey, stop!\p"
	.string "I found these fossils!\n"
	.string "They’re both mine!$"

Kanto_MtMoon_B2F_Text_8423180::
	.string "Okay!\n"
	.string "I’ll share!$"

Kanto_MtMoon_B2F_Text_8423192::
	.string "We’ll each take a fossil!\n"
	.string "No being greedy!$"

Kanto_MtMoon_B2F_Text_84231BD::
	.string "All right.\n"
	.string "Then this fossil is mine!$"

Kanto_MtMoon_B2F_Text_84231E2::
	.string "Far away, on CINNABAR ISLAND,\n"
	.string "there’s a POKéMON LAB.\p"
	.string "They do research on regenerating\n"
	.string "fossils.$"

Kanto_MtMoon_B2F_Text_8423241::
	.string "We, TEAM ROCKET, shall find the\n"
	.string "fossils!\p"
	.string "Reviving POKéMON from them will\n"
	.string "earn us huge riches!$"

Kanto_MtMoon_B2F_Text_842329F::
	.string "Urgh!\n"
	.string "Now I’m mad!$"

Kanto_MtMoon_B2F_Text_84232B2::
	.string "You made me mad!\n"
	.string "TEAM ROCKET will blacklist you!$"

Kanto_MtMoon_B2F_Text_84232E3::
	.string "We, TEAM ROCKET, are POKéMON\n"
	.string "gangsters!\l"
	.string "We strike fear with our strength!$"

Kanto_MtMoon_B2F_Text_842332D::
	.string "I blew it!$"

Kanto_MtMoon_B2F_Text_8423338::
	.string "Darn it all!\n"
	.string "My associates won’t stand for this!$"

Kanto_MtMoon_B2F_Text_8423369::
	.string "We’re pulling a big job here!\n"
	.string "Get lost, kid!$"

Kanto_MtMoon_B2F_Text_8423396::
	.string "So, you are good…$"

Kanto_MtMoon_B2F_Text_84233A8::
	.string "If you find a fossil, give it to me\n"
	.string "and scram!$"

Kanto_MtMoon_B2F_Text_84233D7::
	.string "Little kids shouldn’t be messing\n"
	.string "around with grown-ups!\p"
	.string "It could be bad news!$"

Kanto_MtMoon_B2F_Text_8423425::
	.string "I’m steamed!$"

Kanto_MtMoon_B2F_Text_8423432::
	.string "POKéMON lived here long before\n"
	.string "people came.$"

Kanto_MtMoon_B2F_Text_842345E::
	.string "Do you want the DOME FOSSIL?$"

Kanto_MtMoon_B2F_Text_842347B::
	.string "Do you want the HELIX FOSSIL?$"

Kanto_MtMoon_B2F_Text_8423499::
	.string "Obtained the HELIX FOSSIL!$"

Kanto_MtMoon_B2F_Text_84234B4::
	.string "Obtained the DOME FOSSIL!$"
