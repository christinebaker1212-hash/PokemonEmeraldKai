Kanto_VictoryRoad_2F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_VictoryRoad_2F_EventScript_8410C3A
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

Kanto_VictoryRoad_2F_EventScript_8410C3A::
	call_if_ne VAR_QZ_0x5137, 100, Kanto_VictoryRoad_2F_EventScript_8410C51
	call_if_ne VAR_QZ_0x5138, 100, Kanto_VictoryRoad_2F_EventScript_8410C64
	end

Kanto_VictoryRoad_2F_EventScript_8410C51::
	setmetatile 0xD, 0xA, 0x307, 1
	setmetatile 0xD, 0xB, 0x317, 1
	return

Kanto_VictoryRoad_2F_EventScript_8410C64::
	setmetatile 0x21, 0x10, 0x307, 1
	setmetatile 0x21, 0x11, 0x317, 1
	return

@ Kanto_VictoryRoad_2F trigger at 2,19 (VAR_QZ_0x5137 == 99)
Kanto_VictoryRoad_2F_EventScript_8410C77::
	lockall
	goto_if_eq VAR_QZ_0x5137, 100, Kanto_VictoryRoad_2F_EventScript_8410CA6
	setmetatile 0xD, 0xA, 0x2D1, 0
	setmetatile 0xD, 0xB, 0x2E1, 0
	playse SE_ICE_BREAK
	special DrawWholeMapView
	waitse
	copyobjectxytoperm 11
	setvar VAR_QZ_0x5137, 0x64
	releaseall
	end

Kanto_VictoryRoad_2F_EventScript_8410CA6::
	releaseall
	end

@ Kanto_VictoryRoad_2F trigger at 14,19 (VAR_QZ_0x5138 == 99)
Kanto_VictoryRoad_2F_EventScript_8410CA8::
	lockall
	goto_if_eq VAR_QZ_0x5138, 100, Kanto_VictoryRoad_2F_EventScript_8410CD7
	setmetatile 0x21, 0x10, 0x2D1, 0
	setmetatile 0x21, 0x11, 0x2E1, 0
	playse SE_ICE_BREAK
	special DrawWholeMapView
	waitse
	copyobjectxytoperm 12
	setvar VAR_QZ_0x5138, 0x64
	releaseall
	end

Kanto_VictoryRoad_2F_EventScript_8410CD7::
	releaseall
	end

@ Kanto_VictoryRoad_2F object 1 (local id 1, gfx 0x3F4B) at 7,4
Kanto_VictoryRoad_2F_EventScript_8410CD9::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8359, 0, Kanto_VictoryRoad_2F_Text_8424EA5, Kanto_VictoryRoad_2F_Text_8424EE2
	msgbox Kanto_VictoryRoad_2F_Text_8424EF4, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_2F object 2 (local id 2, gfx 0x3EEB) at 20,11
Kanto_VictoryRoad_2F_EventScript_8410CF0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8517, 0, Kanto_VictoryRoad_2F_Text_8424F2D, Kanto_VictoryRoad_2F_Text_8424F6A
	msgbox Kanto_VictoryRoad_2F_Text_8424F71, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_2F object 3 (local id 3, gfx 0x3F3B) at 31,16
Kanto_VictoryRoad_2F_EventScript_8410D07::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8479, 0, Kanto_VictoryRoad_2F_Text_8424FA4, Kanto_VictoryRoad_2F_Text_8424FD1
	msgbox Kanto_VictoryRoad_2F_Text_8424FDD, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_2F object 5 (local id 5, gfx 0x3F3B) at 36,5
Kanto_VictoryRoad_2F_EventScript_8410D1E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8482, 0, Kanto_VictoryRoad_2F_Text_842504D, Kanto_VictoryRoad_2F_Text_8425068
	msgbox Kanto_VictoryRoad_2F_Text_8425073, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_2F object 4 (local id 4, gfx 0x3F1E) at 26,6
Kanto_VictoryRoad_2F_EventScript_8410D35::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8490, 0, Kanto_VictoryRoad_2F_Text_8424FFA, Kanto_VictoryRoad_2F_Text_8425012
	msgbox Kanto_VictoryRoad_2F_Text_8425021, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_2F object 6 (local id 6, gfx 0x3F0E) at 17,6
Kanto_VictoryRoad_2F_EventScript_846BED2::
	setorcopyvar VAR_0x8000, ITEM_GUARD_SPEC
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_VictoryRoad_2F object 7 (local id 7, gfx 0x3F0E) at 40,7
Kanto_VictoryRoad_2F_EventScript_846BEDF::
	@ Moemon Kai: FireRed's TM (tools/quetzal_port/fix_kanto_tms.py) (Quetzal had ITEM_NEVER_MELT_ICE here)
	setorcopyvar VAR_0x8000, ITEM_TM07
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_VictoryRoad_2F object 8 (local id 8, gfx 0x3F0E) at 25,13
Kanto_VictoryRoad_2F_EventScript_846BEEC::
	setorcopyvar VAR_0x8000, ITEM_FULL_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_VictoryRoad_2F object 9 (local id 9, gfx 0x3F0E) at 14,13
Kanto_VictoryRoad_2F_EventScript_846BEF9::
	@ Moemon Kai: FireRed's TM (tools/quetzal_port/fix_kanto_tms.py) (Quetzal had ITEM_SMOOTH_ROCK here)
	setorcopyvar VAR_0x8000, ITEM_TM37
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_VictoryRoad_2F object 13 (local id 13, gfx 0x3EFD) at 40,9
Kanto_VictoryRoad_2F_EventScript_8471C54::
	@ Moemon Kai: FireRed's DOUBLE EDGE tutor (tools/quetzal_port/fix_move_tutors.py), on Kai's move tutors
	lock
	faceplayer
	goto_if_set 0x210D, Kanto_VictoryRoad_2F_EventScript_8471C54_Taught
	msgbox Kanto_VictoryRoad_2F_EventScript_8471C54_Teach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kanto_VictoryRoad_2F_EventScript_8471C54_Declined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, Kanto_VictoryRoad_2F_EventScript_8471C54_Declined
	msgbox Kanto_VictoryRoad_2F_EventScript_8471C54_WhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_DOUBLE_EDGE
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, Kanto_VictoryRoad_2F_EventScript_8471C54_Declined
	setflag 0x210D
	goto Kanto_VictoryRoad_2F_EventScript_8471C54_Taught
	end

Kanto_VictoryRoad_2F_EventScript_8471C54_Declined:
	msgbox Kanto_VictoryRoad_2F_EventScript_8471C54_DeclinedText, MSGBOX_DEFAULT
	release
	end

Kanto_VictoryRoad_2F_EventScript_8471C54_Taught:
	msgbox Kanto_VictoryRoad_2F_EventScript_8471C54_TaughtText, MSGBOX_DEFAULT
	release
	end

Kanto_VictoryRoad_2F_EventScript_8471C54_Teach:
	.string "You should be proud of yourself,\n"
	.string "having battled your way through\l"
	.string "VICTORY ROAD so courageously.\p"
	.string "In recognition of your feat,\n"
	.string "I'll teach you DOUBLE-EDGE.\p"
	.string "Would you like me to teach that\n"
	.string "technique?$"

Kanto_VictoryRoad_2F_EventScript_8471C54_DeclinedText:
	.string "I'll teach you the technique\n"
	.string "anytime.$"

Kanto_VictoryRoad_2F_EventScript_8471C54_WhichMon:
	.string "Which MOéMON should I teach\n"
	.string "DOUBLE-EDGE?$"

Kanto_VictoryRoad_2F_EventScript_8471C54_TaughtText:
	.string "Keep that drive going for the\n"
	.string "MOéMON LEAGUE!\p"
	.string "Take a run at them and knock 'em\n"
	.string "out!$"

Kanto_VictoryRoad_2F_Text_8424EA5::
	.string "If you can get through here,\n"
	.string "you can go meet the ELITE FOUR.$"

Kanto_VictoryRoad_2F_Text_8424EE2::
	.string "No!\n"
	.string "Unbelievable!$"

Kanto_VictoryRoad_2F_Text_8424EF4::
	.string "I can beat you when it comes to\n"
	.string "knowledge about MOéMON!$"

Kanto_VictoryRoad_2F_Text_8424F2D::
	.string "This here is VICTORY ROAD.\n"
	.string "It’s the final test for TRAINERS!$"

Kanto_VictoryRoad_2F_Text_8424F6A::
	.string "Atcho!$"

Kanto_VictoryRoad_2F_Text_8424F71::
	.string "If you get stuck, try moving some\n"
	.string "boulders around.$"

Kanto_VictoryRoad_2F_Text_8424FA4::
	.string "Ah, so you wish to challenge the\n"
	.string "ELITE FOUR?$"

Kanto_VictoryRoad_2F_Text_8424FD1::
	.string "You got me!$"

Kanto_VictoryRoad_2F_Text_8424FDD::
	.string "{RIVAL} also came through here.$"

Kanto_VictoryRoad_2F_Text_8424FFA::
	.string "Come on!\n"
	.string "I’ll whip you!$"

Kanto_VictoryRoad_2F_Text_8425012::
	.string "I got whipped!$"

Kanto_VictoryRoad_2F_Text_8425021::
	.string "You earned the right to be on\n"
	.string "VICTORY ROAD…$"

Kanto_VictoryRoad_2F_Text_842504D::
	.string "Is VICTORY ROAD too tough?$"

Kanto_VictoryRoad_2F_Text_8425068::
	.string "Well done!$"

Kanto_VictoryRoad_2F_Text_8425073::
	.string "Many TRAINERS give up their\n"
	.string "challenge here and go home.$"
