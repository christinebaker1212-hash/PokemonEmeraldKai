Kanto_VictoryRoad_1F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_VictoryRoad_1F_EventScript_8410BAD
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_VictoryRoad_1F_EventScript_8410BCC
	.byte 0

Kanto_VictoryRoad_1F_EventScript_8410BAD::
	call_if_ne VAR_QZ_0x5136, 100, Kanto_VictoryRoad_1F_EventScript_8410BB9
	end

Kanto_VictoryRoad_1F_EventScript_8410BB9::
	setmetatile 0xC, 0xE, 0x307, 1
	setmetatile 0xC, 0xF, 0x317, 1
	return

Kanto_VictoryRoad_1F_EventScript_8410BCC::
	setflag FLAG_QZ_0x2263
	end

@ Kanto_VictoryRoad_1F trigger at 20,16 (VAR_QZ_0x5136 == 99)
Kanto_VictoryRoad_1F_EventScript_8410BD0::
	lockall
	goto_if_eq VAR_QZ_0x5136, 100, Kanto_VictoryRoad_1F_EventScript_8410BFF
	setmetatile 0xC, 0xE, 0x2D1, 0
	setmetatile 0xC, 0xF, 0x2E1, 0
	playse SE_ICE_BREAK
	special DrawWholeMapView
	waitse
	copyobjectxytoperm 5
	setvar VAR_QZ_0x5136, 0x64
	releaseall
	end

Kanto_VictoryRoad_1F_EventScript_8410BFF::
	releaseall
	end

@ Kanto_VictoryRoad_1F object 2 (local id 2, gfx 0x3EFC) at 14,6
Kanto_VictoryRoad_1F_EventScript_8410C01::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8598, 0, Kanto_VictoryRoad_1F_Text_8424DC6, Kanto_VictoryRoad_1F_Text_8424DEE
	msgbox Kanto_VictoryRoad_1F_Text_8424DFA, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_1F object 1 (local id 1, gfx 0x3EFD) at 5,2
Kanto_VictoryRoad_1F_EventScript_8410C18::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8588, 0, Kanto_VictoryRoad_1F_Text_8424E3A, Kanto_VictoryRoad_1F_Text_8424E6E
	msgbox Kanto_VictoryRoad_1F_Text_8424E7E, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_1F object 3 (local id 3, gfx 0x3F0E) at 12,3
Kanto_VictoryRoad_1F_EventScript_846BEB8::
	setorcopyvar VAR_0x8000, 0x66
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_VictoryRoad_1F object 4 (local id 4, gfx 0x3F0E) at 14,1
Kanto_VictoryRoad_1F_EventScript_846BEC5::
	setorcopyvar VAR_0x8000, 0xE3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_VictoryRoad_1F_Text_8424DC6::
	.string "I wonder if you are good enough\n"
	.string "for me?$"

Kanto_VictoryRoad_1F_Text_8424DEE::
	.string "I lost out…$"

Kanto_VictoryRoad_1F_Text_8424DFA::
	.string "I never wanted to lose to anybody,\n"
	.string "especially to a younger kid…$"

Kanto_VictoryRoad_1F_Text_8424E3A::
	.string "I can see you’re good.\n"
	.string "Let me see exactly how good!$"

Kanto_VictoryRoad_1F_Text_8424E6E::
	.string "I had a chance…$"

Kanto_VictoryRoad_1F_Text_8424E7E::
	.string "Tch!\n"
	.string "I concede, you are better than I!$"
