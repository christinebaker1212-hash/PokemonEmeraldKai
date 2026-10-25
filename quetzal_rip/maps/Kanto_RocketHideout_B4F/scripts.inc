Kanto_RocketHideout_B4F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_RocketHideout_B4F_EventScript_8410F9F
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

Kanto_RocketHideout_B4F_EventScript_8410F9F::
	setvar VAR_TEMP_1, 0
	checktrainerflag 0x216E
	call_if_eq Kanto_RocketHideout_B4F_EventScript_8410FE3
	checktrainerflag 0x216F
	call_if_eq Kanto_RocketHideout_B4F_EventScript_8410FE3
	call_if_ne VAR_TEMP_1, 2, Kanto_RocketHideout_B4F_EventScript_8411165
	checktrainerflag 0x215C
	call_if_eq Kanto_RocketHideout_B4F_EventScript_8410FCB
	end

Kanto_RocketHideout_B4F_EventScript_8410FCB::
	checkitem ITEM_SILPH_SCOPE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_RocketHideout_B4F_EventScript_8410FDF
	clearflag FLAG_QZ_0x21CB
	return

Kanto_RocketHideout_B4F_EventScript_8410FDF::
	setflag FLAG_QZ_0x21CB
	return

Kanto_RocketHideout_B4F_EventScript_8410FE3::
	addvar VAR_TEMP_1, 1
	return

@ Kanto_RocketHideout_B4F object 1 (local id 1, gfx 0x3F0A) at 19,4
Kanto_RocketHideout_B4F_EventScript_8410FE9::
	lock
	faceplayer
	setvar VAR_0x8004, 0xF
	setvar VAR_0x8005, 0
	special QzQuestLogNop_737
	message Kanto_RocketHideout_B4F_Text_84258DE
	waitmessage
	playbgm MUS_RG_ENCOUNTER_ROCKET, 0
	waitbuttonpress
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_8540, 0, Kanto_RocketHideout_B4F_Text_84259D0
	msgbox Kanto_RocketHideout_B4F_Text_84259E5, MSGBOX_DEFAULT
	fadescreen 1
	closemessage
	removeobject 1
	addobject 2
	clearflag FLAG_QZ_0x21CB
	setflag FLAG_QZ_0x217C
	setvar VAR_0x8004, 0xF
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	fadescreen 0
	release
	end

@ Kanto_RocketHideout_B4F object 2 (local id 2, gfx 0x3F0E) at 20,5
Kanto_RocketHideout_B4F_EventScript_8411035::
	lock
	faceplayer
	checkitem ITEM_SILPH_SCOPE, 1
	goto_if_eq VAR_RESULT, 1, Kanto_RocketHideout_B4F_EventScript_841105E
	setorcopyvar VAR_0x8000, 0x2F0
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_84570B3

Kanto_RocketHideout_B4F_EventScript_841105E::
	removeobject 2
	release
	end

@ Kanto_RocketHideout_B4F object 3 (local id 3, gfx 0x3F3D) at 4,2
Kanto_RocketHideout_B4F_EventScript_8411063::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8560, 0, Kanto_RocketHideout_B4F_Text_8425B40, Kanto_RocketHideout_B4F_Text_8425B71, Kanto_RocketHideout_B4F_EventScript_841107F

Kanto_RocketHideout_B4F_EventScript_8411075::
	msgbox Kanto_RocketHideout_B4F_Text_8425B75, MSGBOX_DEFAULT
	release
	end

Kanto_RocketHideout_B4F_EventScript_841107F::
	goto_if_set FLAG_QZ_0x2112, Kanto_RocketHideout_B4F_EventScript_8411075
	msgbox Kanto_RocketHideout_B4F_Text_8425B75, MSGBOX_DEFAULT
	addobject 4
	clearflag FLAG_QZ_0x218F
	release
	end

@ Kanto_RocketHideout_B4F object 4 (local id 4, gfx 0x3F0E) at 3,2
Kanto_RocketHideout_B4F_EventScript_8411098::
	lock
	faceplayer
	checkitem ITEM_LIFT_KEY, 1
	goto_if_eq VAR_RESULT, 1, Kanto_RocketHideout_B4F_EventScript_84110C1
	setorcopyvar VAR_0x8000, 0x2EF
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_84570B3

Kanto_RocketHideout_B4F_EventScript_84110C1::
	setflag FLAG_QZ_0x2112
	removeobject 4
	release
	end

@ Kanto_RocketHideout_B4F object 6 (local id 6, gfx 0x3F3D) at 16,14
Kanto_RocketHideout_B4F_EventScript_84110C9::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8558, 0, Kanto_RocketHideout_B4F_Text_8425A88, Kanto_RocketHideout_B4F_Text_8425AB6, Kanto_RocketHideout_B4F_EventScript_84110E4
	msgbox Kanto_RocketHideout_B4F_Text_8425AC4, MSGBOX_AUTOCLOSE
	end

Kanto_RocketHideout_B4F_EventScript_84110E4::
	setvar VAR_TEMP_1, 0
	checktrainerflag 0x216E
	call_if_eq Kanto_RocketHideout_B4F_EventScript_8410FE3
	checktrainerflag 0x216F
	call_if_eq Kanto_RocketHideout_B4F_EventScript_8410FE3
	call_if_eq VAR_TEMP_1, 2, Kanto_RocketHideout_B4F_EventScript_841119C
	call_if_eq VAR_TEMP_1, 2, Kanto_RocketHideout_B4F_EventScript_841115D
	release
	end

@ Kanto_RocketHideout_B4F object 5 (local id 5, gfx 0x3F3D) at 19,14
Kanto_RocketHideout_B4F_EventScript_8411113::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT, TRAINER_QZ_8559, 0, Kanto_RocketHideout_B4F_Text_8425AEF, Kanto_RocketHideout_B4F_Text_8425B1B, Kanto_RocketHideout_B4F_EventScript_841112E
	msgbox Kanto_RocketHideout_B4F_Text_8425B22, MSGBOX_AUTOCLOSE
	end

Kanto_RocketHideout_B4F_EventScript_841112E::
	setvar VAR_TEMP_1, 0
	checktrainerflag 0x216E
	call_if_eq Kanto_RocketHideout_B4F_EventScript_8410FE3
	checktrainerflag 0x216F
	call_if_eq Kanto_RocketHideout_B4F_EventScript_8410FE3
	call_if_eq VAR_TEMP_1, 2, Kanto_RocketHideout_B4F_EventScript_841119C
	call_if_eq VAR_TEMP_1, 2, Kanto_RocketHideout_B4F_EventScript_841115D
	release
	end

Kanto_RocketHideout_B4F_EventScript_841115D::
	playse SE_UNLOCK
	special DrawWholeMapView
	waitse
	return

Kanto_RocketHideout_B4F_EventScript_8411165::
	setmetatile 0x11, 0xC, 0x2CE, 1
	setmetatile 0x12, 0xC, 0x2CF, 1
	setmetatile 0x11, 0xD, 0x2D6, 1
	setmetatile 0x12, 0xD, 0x2D7, 1
	setmetatile 0x11, 0xE, 0x282, 0
	setmetatile 0x12, 0xE, 0x282, 0
	return

Kanto_RocketHideout_B4F_EventScript_841119C::
	setmetatile 0x11, 0xC, 0x282, 0
	setmetatile 0x12, 0xC, 0x281, 0
	setmetatile 0x11, 0xD, 0x282, 0
	setmetatile 0x12, 0xD, 0x281, 0
	setmetatile 0x11, 0xE, 0x282, 0
	setmetatile 0x12, 0xE, 0x281, 0
	return

@ Kanto_RocketHideout_B4F object 7 (local id 7, gfx 0x3F0E) at 1,6
Kanto_RocketHideout_B4F_EventScript_846BBE4::
	setorcopyvar VAR_0x8000, 0x88
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RocketHideout_B4F object 8 (local id 8, gfx 0x3F0E) at 4,14
Kanto_RocketHideout_B4F_EventScript_846BBF1::
	setorcopyvar VAR_0x8000, 0x32
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_RocketHideout_B4F object 9 (local id 9, gfx 0x3F0E) at 6,23
Kanto_RocketHideout_B4F_EventScript_846BBFE::
	setorcopyvar VAR_0x8000, 0x44
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_RocketHideout_B4F_Text_84258DE::
	.string "So! I must say, I am impressed you\n"
	.string "got here.\p"
	.string "TEAM ROCKET captures POKéMON from\n"
	.string "around the world.\p"
	.string "They’re important tools for keeping\n"
	.string "our criminal enterprise going.\p"
	.string "I am the leader, GIOVANNI!\p"
	.string "For your insolence, you will feel a\n"
	.string "world of pain!$"

Kanto_RocketHideout_B4F_Text_84259D0::
	.string "WHAT!\n"
	.string "This can’t be!$"

Kanto_RocketHideout_B4F_Text_84259E5::
	.string "I see that you raise POKéMON with\n"
	.string "utmost care.\p"
	.string "A child like you would never\n"
	.string "understand what I hope to achieve.\p"
	.string "I shall step aside this time!\p"
	.string "I hope we meet again…$"

Kanto_RocketHideout_B4F_Text_8425A88::
	.string "I know you!\n"
	.string "You ruined our plans at MT. MOON!$"

Kanto_RocketHideout_B4F_Text_8425AB6::
	.string "Burned again!$"

Kanto_RocketHideout_B4F_Text_8425AC4::
	.string "Do you have something against TEAM\n"
	.string "ROCKET?$"

Kanto_RocketHideout_B4F_Text_8425AEF::
	.string "How can you not see the beauty of\n"
	.string "our evil?$"

Kanto_RocketHideout_B4F_Text_8425B1B::
	.string "Ayaya!$"

Kanto_RocketHideout_B4F_Text_8425B22::
	.string "BOSS!\n"
	.string "I’m sorry I failed you!$"

Kanto_RocketHideout_B4F_Text_8425B40::
	.string "The elevator doesn’t work?\n"
	.string "Who has the LIFT KEY?$"

Kanto_RocketHideout_B4F_Text_8425B71::
	.string "No!$"

Kanto_RocketHideout_B4F_Text_8425B75::
	.string "Oh, no!\n"
	.string "I dropped the LIFT KEY!$"
