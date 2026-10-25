Kanto_VictoryRoad_3F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_VictoryRoad_3F_EventScript_8410D57
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

Kanto_VictoryRoad_3F_EventScript_8410D57::
	call_if_ne VAR_QZ_0x5139, 100, Kanto_VictoryRoad_3F_EventScript_8410D63
	end

Kanto_VictoryRoad_3F_EventScript_8410D63::
	setmetatile 0xC, 0xC, 0x307, 1
	setmetatile 0xC, 0xD, 0x317, 1
	return

@ Kanto_VictoryRoad_3F trigger at 7,7 (VAR_QZ_0x5139 == 99)
Kanto_VictoryRoad_3F_EventScript_8410D76::
	lockall
	goto_if_eq VAR_QZ_0x5139, 100, Kanto_VictoryRoad_3F_EventScript_8410DA8
	setmetatile 0xC, 0xC, 0x2D1, 0
	setmetatile 0xC, 0xD, 0x2E1, 0
	playse SE_ICE_BREAK
	special DrawWholeMapView
	waitse
	copyobjectxytoperm 7
	copyobjectxytoperm 10
	setvar VAR_QZ_0x5139, 0x64
	releaseall
	end

Kanto_VictoryRoad_3F_EventScript_8410DA8::
	releaseall
	end

@ Kanto_VictoryRoad_3F object 1 (local id 1, gfx 0x3EFD) at 40,7
Kanto_VictoryRoad_3F_EventScript_8410DAA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8585, 0, Kanto_VictoryRoad_3F_Text_8425218, Kanto_VictoryRoad_3F_Text_842523B
	msgbox Kanto_VictoryRoad_3F_Text_8425251, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_3F object 3 (local id 3, gfx 0x3EFD) at 10,17
Kanto_VictoryRoad_3F_EventScript_8410DC1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8586, 0, Kanto_VictoryRoad_3F_Text_8425342, Kanto_VictoryRoad_3F_Text_8425361
	msgbox Kanto_VictoryRoad_3F_Text_8425375, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_3F object 4 (local id 4, gfx 0x3EFC) at 11,16
Kanto_VictoryRoad_3F_EventScript_8410DD8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8595, 0, Kanto_VictoryRoad_3F_Text_84252E8, Kanto_VictoryRoad_3F_Text_8425312
	msgbox Kanto_VictoryRoad_3F_Text_842531F, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_3F object 2 (local id 2, gfx 0x3EFC) at 21,5
Kanto_VictoryRoad_3F_EventScript_8410DEF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8596, 0, Kanto_VictoryRoad_3F_Text_8425282, Kanto_VictoryRoad_3F_Text_84252AC
	msgbox Kanto_VictoryRoad_3F_Text_84252BB, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_3F object 11 (local id 11, gfx 0x3EFD) at 38,13
Kanto_VictoryRoad_3F_EventScript_8410E06::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8677, 0, Kanto_VictoryRoad_3F_Text_84253C4, Kanto_VictoryRoad_3F_Text_84253FD, Kanto_VictoryRoad_3F_Text_842544E
	msgbox Kanto_VictoryRoad_3F_Text_842541C, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_3F object 12 (local id 12, gfx 0x3EFC) at 39,13
Kanto_VictoryRoad_3F_EventScript_8410E21::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8677, 0, Kanto_VictoryRoad_3F_Text_84254C0, Kanto_VictoryRoad_3F_Text_84254F1, Kanto_VictoryRoad_3F_Text_8425545
	msgbox Kanto_VictoryRoad_3F_Text_8425500, MSGBOX_AUTOCLOSE
	end

@ Kanto_VictoryRoad_3F object 5 (local id 5, gfx 0x3F0E) at 38,7
Kanto_VictoryRoad_3F_EventScript_846BF06::
	setorcopyvar VAR_0x8000, 0x22
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_VictoryRoad_3F object 6 (local id 6, gfx 0x3F0E) at 12,9
Kanto_VictoryRoad_3F_EventScript_846BF13::
	setorcopyvar VAR_0x8000, 0x4F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_VictoryRoad_3F_Text_8425218::
	.string "I heard rumors of a child prodigy.$"

Kanto_VictoryRoad_3F_Text_842523B::
	.string "The rumors were true!$"

Kanto_VictoryRoad_3F_Text_8425251::
	.string "So, it was you who beat GIOVANNI\n"
	.string "of TEAM ROCKET?$"

Kanto_VictoryRoad_3F_Text_8425282::
	.string "TRAINERS live to seek stronger\n"
	.string "opponents.$"

Kanto_VictoryRoad_3F_Text_84252AC::
	.string "Oh!\n"
	.string "So strong!$"

Kanto_VictoryRoad_3F_Text_84252BB::
	.string "By fighting tough battles, you get\n"
	.string "stronger.$"

Kanto_VictoryRoad_3F_Text_84252E8::
	.string "I’ll show you just how good you\n"
	.string "are. Not!$"

Kanto_VictoryRoad_3F_Text_8425312::
	.string "I’m furious!$"

Kanto_VictoryRoad_3F_Text_842531F::
	.string "You showed me just how good\n"
	.string "I was…$"

Kanto_VictoryRoad_3F_Text_8425342::
	.string "Only the chosen can pass here!$"

Kanto_VictoryRoad_3F_Text_8425361::
	.string "I don’t believe it!$"

Kanto_VictoryRoad_3F_Text_8425375::
	.string "All TRAINERS here are headed to\n"
	.string "the POKéMON LEAGUE.\p"
	.string "Don’t let down your guard.$"

Kanto_VictoryRoad_3F_Text_84253C4::
	.string "RAY: Together, the two of us are\n"
	.string "destined for greatness!$"

Kanto_VictoryRoad_3F_Text_84253FD::
	.string "RAY: Ludicrous!\n"
	.string "This can’t be!$"

Kanto_VictoryRoad_3F_Text_842541C::
	.string "RAY: You’ve beaten us.\n"
	.string "Greatness remains elusive…$"

Kanto_VictoryRoad_3F_Text_842544E::
	.string "RAY: Together, the two of us are\n"
	.string "striving for the pinnacle.\p"
	.string "We need you to bring two POKéMON\n"
	.string "into battle with us.$"

Kanto_VictoryRoad_3F_Text_84254C0::
	.string "TYRA: We’re trying to become\n"
	.string "champions together.$"

Kanto_VictoryRoad_3F_Text_84254F1::
	.string "TYRA: Oh, but…$"

Kanto_VictoryRoad_3F_Text_8425500::
	.string "TYRA: You’ve taught me that power\n"
	.string "can be infinite in shape and form.$"

Kanto_VictoryRoad_3F_Text_8425545::
	.string "TYRA: You can’t battle with us if\n"
	.string "you have only one POKéMON.$"
