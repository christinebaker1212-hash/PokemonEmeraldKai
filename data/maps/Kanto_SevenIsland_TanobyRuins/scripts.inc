Kanto_SevenIsland_TanobyRuins_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_SevenIsland_TanobyRuins object 1 (local id 1, gfx 0x3F0C) at 35,7
Kanto_SevenIsland_TanobyRuins_EventScript_845D01E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8794, 0, Kanto_SevenIsland_TanobyRuins_Text_843D313, Kanto_SevenIsland_TanobyRuins_Text_843D342
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_TanobyRuins_EventScript_845D045
	msgbox Kanto_SevenIsland_TanobyRuins_Text_843D379, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_TanobyRuins_EventScript_845D045::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8794, 0, Kanto_SevenIsland_TanobyRuins_Text_8471B79, Kanto_SevenIsland_TanobyRuins_Text_843D342
	msgbox Kanto_SevenIsland_TanobyRuins_Text_843D379, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_TanobyRuins object 2 (local id 2, gfx 0x3F0C) at 121,11
Kanto_SevenIsland_TanobyRuins_EventScript_845D05C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8795, 0, Kanto_SevenIsland_TanobyRuins_Text_843D3F1, Kanto_SevenIsland_TanobyRuins_Text_843D420
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_TanobyRuins_EventScript_845D083
	msgbox Kanto_SevenIsland_TanobyRuins_Text_843D43D, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_TanobyRuins_EventScript_845D083::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8795, 0, Kanto_SevenIsland_TanobyRuins_Text_8471BB0, Kanto_SevenIsland_TanobyRuins_Text_843D420
	msgbox Kanto_SevenIsland_TanobyRuins_Text_843D43D, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_TanobyRuins object 3 (local id 3, gfx 0x3F16) at 85,8
Kanto_SevenIsland_TanobyRuins_EventScript_845D09A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8796, 0, Kanto_SevenIsland_TanobyRuins_Text_843D4A7, Kanto_SevenIsland_TanobyRuins_Text_843D4CD
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_TanobyRuins_EventScript_845D0C1
	msgbox Kanto_SevenIsland_TanobyRuins_Text_843D4FB, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_TanobyRuins_EventScript_845D0C1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8796, 0, Kanto_SevenIsland_TanobyRuins_Text_8471BDF, Kanto_SevenIsland_TanobyRuins_Text_843D4CD
	msgbox Kanto_SevenIsland_TanobyRuins_Text_843D4FB, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_TanobyRuins object 4 (local id 4, gfx 0x3F09) at 85,5
Kanto_SevenIsland_TanobyRuins_EventScript_845D0D8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8797, 0, Kanto_SevenIsland_TanobyRuins_Text_843D547, Kanto_SevenIsland_TanobyRuins_Text_843D585
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_TanobyRuins_EventScript_845D0FF
	msgbox Kanto_SevenIsland_TanobyRuins_Text_843D59C, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_TanobyRuins_EventScript_845D0FF::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8797, 0, Kanto_SevenIsland_TanobyRuins_Text_8471C10, Kanto_SevenIsland_TanobyRuins_Text_843D585
	msgbox Kanto_SevenIsland_TanobyRuins_Text_843D59C, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_TanobyRuins_Text_843D313::
	.string "Do you know anything about this\n"
	.string "stone chamber?$"

Kanto_SevenIsland_TanobyRuins_Text_843D342::
	.string "That was uncalled for.\n"
	.string "You didn’t have to be so rough.$"

Kanto_SevenIsland_TanobyRuins_Text_843D379::
	.string "The TANOBY RUINS date back at\n"
	.string "least 1,500 years.\p"
	.string "But it’s a baffling mystery as to\n"
	.string "why it was built in the first place.$"

Kanto_SevenIsland_TanobyRuins_Text_843D3F1::
	.string "A mystic POKéMON is said to sleep\n"
	.string "inside here.$"

Kanto_SevenIsland_TanobyRuins_Text_843D420::
	.string "I’m not much good in battle.$"

Kanto_SevenIsland_TanobyRuins_Text_843D43D::
	.string "I hear there are other ruins like\n"
	.string "the one here.\p"
	.string "Might they be the resting places\n"
	.string "of other mystic POKéMON?$"

Kanto_SevenIsland_TanobyRuins_Text_843D4A7::
	.string "CLIFFORD is teaching me how to\n"
	.string "paint.$"

Kanto_SevenIsland_TanobyRuins_Text_843D4CD::
	.string "We might get in trouble for being\n"
	.string "noisy here.$"

Kanto_SevenIsland_TanobyRuins_Text_843D4FB::
	.string "I said that I wanted to paint an\n"
	.string "old building.\p"
	.string "So CLIFFORD brought me here.$"

Kanto_SevenIsland_TanobyRuins_Text_843D547::
	.string "Today, in addition to our lesson,\n"
	.string "we came to see the chamber.$"

Kanto_SevenIsland_TanobyRuins_Text_843D585::
	.string "My, how extraordinary.$"

Kanto_SevenIsland_TanobyRuins_Text_843D59C::
	.string "Ah, so you are studying POKéMON\n"
	.string "on your own…\p"
	.string "Yes, you certainly are quite the\n"
	.string "extraordinary person, indeed.$"

Kanto_SevenIsland_TanobyRuins_Text_8471B79::
	.string "Have you discovered anything about\n"
	.string "this stone chamber?$"

Kanto_SevenIsland_TanobyRuins_Text_8471BB0::
	.string "A mystic POKéMON is said to sleep\n"
	.string "inside here.$"

Kanto_SevenIsland_TanobyRuins_Text_8471BDF::
	.string "My teacher is showing me how to\n"
	.string "paint some more.$"

Kanto_SevenIsland_TanobyRuins_Text_8471C10::
	.string "Today, in addition to our lesson,\n"
	.string "we came to see the chamber again.$"
