Kanto_SixIsland_WaterPath_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_SixIsland_WaterPath sign / bg event at 8,14
Kanto_SixIsland_WaterPath_EventScript_84188F8::
	msgbox Kanto_SixIsland_WaterPath_Text_843BFF5, MSGBOX_SIGN
	end

@ Kanto_SixIsland_WaterPath sign / bg event at 12,54
Kanto_SixIsland_WaterPath_EventScript_8418901::
	msgbox Kanto_SixIsland_WaterPath_Text_843C010, MSGBOX_SIGN
	end

@ Kanto_SixIsland_WaterPath object 1 (local id 1, gfx 0x3F58) at 12,13
Kanto_SixIsland_WaterPath_EventScript_845C926::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8769, 0, Kanto_SixIsland_WaterPath_Text_843BB7F, Kanto_SixIsland_WaterPath_Text_843BBB1
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_WaterPath_EventScript_845C94D
	msgbox Kanto_SixIsland_WaterPath_Text_843BBE0, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_WaterPath_EventScript_845C94D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8769, 0, Kanto_SixIsland_WaterPath_Text_84714CE, Kanto_SixIsland_WaterPath_Text_843BBB1
	msgbox Kanto_SixIsland_WaterPath_Text_843BBE0, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_WaterPath object 2 (local id 2, gfx 0x3F3B) at 11,52
Kanto_SixIsland_WaterPath_EventScript_845C964::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8483, 0, Kanto_SixIsland_WaterPath_Text_843BC42, Kanto_SixIsland_WaterPath_Text_843BC71
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_WaterPath_EventScript_845C98B
	msgbox Kanto_SixIsland_WaterPath_Text_843BC8E, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_WaterPath_EventScript_845C98B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8483, 0, Kanto_SixIsland_WaterPath_Text_84714FD, Kanto_SixIsland_WaterPath_Text_843BC71
	msgbox Kanto_SixIsland_WaterPath_Text_843BC8E, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_WaterPath object 3 (local id 3, gfx 0x3F4F) at 13,35
Kanto_SixIsland_WaterPath_EventScript_845C9A2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8770, 0, Kanto_SixIsland_WaterPath_Text_843BCD5, Kanto_SixIsland_WaterPath_Text_843BD30
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_WaterPath_EventScript_845C9C9
	msgbox Kanto_SixIsland_WaterPath_Text_843BD40, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_WaterPath_EventScript_845C9C9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8770, 0, Kanto_SixIsland_WaterPath_Text_8471525, Kanto_SixIsland_WaterPath_Text_843BD30
	msgbox Kanto_SixIsland_WaterPath_Text_843BD40, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_WaterPath object 4 (local id 4, gfx 0x3F4D) at 15,45
Kanto_SixIsland_WaterPath_EventScript_845C9E0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8771, 0, Kanto_SixIsland_WaterPath_Text_843BD6F, Kanto_SixIsland_WaterPath_Text_843BDA9
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_WaterPath_EventScript_845CA07
	msgbox Kanto_SixIsland_WaterPath_Text_843BDC9, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_WaterPath_EventScript_845CA07::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8771, 0, Kanto_SixIsland_WaterPath_Text_847157F, Kanto_SixIsland_WaterPath_Text_843BDA9
	msgbox Kanto_SixIsland_WaterPath_Text_843BDC9, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_WaterPath object 5 (local id 5, gfx 0x3F18) at 6,21
Kanto_SixIsland_WaterPath_EventScript_845CA1E::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8772, 0, Kanto_SixIsland_WaterPath_Text_843BEA3, Kanto_SixIsland_WaterPath_Text_843BECB, Kanto_SixIsland_WaterPath_Text_843BF0D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_WaterPath_EventScript_845CA49
	msgbox Kanto_SixIsland_WaterPath_Text_843BEDF, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_WaterPath_EventScript_845CA49::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8772, 0, Kanto_SixIsland_WaterPath_Text_84715DE, Kanto_SixIsland_WaterPath_Text_843BECB, Kanto_SixIsland_WaterPath_Text_843BF0D
	msgbox Kanto_SixIsland_WaterPath_Text_843BEDF, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_WaterPath object 6 (local id 6, gfx 0x3F18) at 7,21
Kanto_SixIsland_WaterPath_EventScript_845CA64::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8772, 0, Kanto_SixIsland_WaterPath_Text_843BF43, Kanto_SixIsland_WaterPath_Text_843BF6D, Kanto_SixIsland_WaterPath_Text_843BFB8
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_WaterPath_EventScript_845CA8F
	msgbox Kanto_SixIsland_WaterPath_Text_843BF97, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_WaterPath_EventScript_845CA8F::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8772, 0, Kanto_SixIsland_WaterPath_Text_847160C, Kanto_SixIsland_WaterPath_Text_843BF6D, Kanto_SixIsland_WaterPath_Text_843BFB8
	msgbox Kanto_SixIsland_WaterPath_Text_843BF97, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_WaterPath object 7 (local id 7, gfx 0x3F0C) at 11,76
Kanto_SixIsland_WaterPath_EventScript_845CAAA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8773, 0, Kanto_SixIsland_WaterPath_Text_843BE16, Kanto_SixIsland_WaterPath_Text_843BE4B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_WaterPath_EventScript_845CAD1
	msgbox Kanto_SixIsland_WaterPath_Text_843BE74, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_WaterPath_EventScript_845CAD1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8773, 0, Kanto_SixIsland_WaterPath_Text_84715A0, Kanto_SixIsland_WaterPath_Text_843BE4B
	msgbox Kanto_SixIsland_WaterPath_Text_843BE74, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_WaterPath object 8 (local id 8, gfx 0x3F0E) at 17,19
Kanto_SixIsland_WaterPath_EventScript_846C183::
	setorcopyvar VAR_0x8000, 0x33
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SixIsland_WaterPath object 9 (local id 9, gfx 0x3F0E) at 17,87
Kanto_SixIsland_WaterPath_EventScript_846C190::
	setorcopyvar VAR_0x8000, 0xE3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SixIsland_WaterPath_Text_843BB7F::
	.string "I let a pleasant breeze lead me\n"
	.string "all the way here.$"

Kanto_SixIsland_WaterPath_Text_843BBB1::
	.string "Giggle… You’re so cute when you’re\n"
	.string "determined.$"

Kanto_SixIsland_WaterPath_Text_843BBE0::
	.string "I want to go to the woods over\n"
	.string "there, but…\p"
	.string "There are lots of BUG POKéMON.\n"
	.string "They scare me a little.$"

Kanto_SixIsland_WaterPath_Text_843BC42::
	.string "I’m practicing in secret so no one\n"
	.string "can see me.$"

Kanto_SixIsland_WaterPath_Text_843BC71::
	.string "Nobody saw me losing, right?$"

Kanto_SixIsland_WaterPath_Text_843BC8E::
	.string "As a performer, I don’t want people\n"
	.string "to know about the effort I put in.$"

Kanto_SixIsland_WaterPath_Text_843BCD5::
	.string "What, you’re suggesting that you’re\n"
	.string "sick of seeing SWIMMERS like me?\p"
	.string "Aww, don’t be hatin’!$"

Kanto_SixIsland_WaterPath_Text_843BD30::
	.string "Oh, no, no, no.$"

Kanto_SixIsland_WaterPath_Text_843BD40::
	.string "I couldn’t lift the stigma of being\n"
	.string "a SWIMMER…$"

Kanto_SixIsland_WaterPath_Text_843BD6F::
	.string "My boyfriend’s always busy, so I\n"
	.string "come swimming by myself.$"

Kanto_SixIsland_WaterPath_Text_843BDA9::
	.string "Awww, I knew this would happen!$"

Kanto_SixIsland_WaterPath_Text_843BDC9::
	.string "When I can’t be with my boyfriend,\n"
	.string "I spend time with the POKéMON he\l"
	.string "gave me.$"

Kanto_SixIsland_WaterPath_Text_843BE16::
	.string "Tell me, where are the mountains\n"
	.string "around these parts?$"

Kanto_SixIsland_WaterPath_Text_843BE4B::
	.string "I lost my way while I was busy\n"
	.string "battling!$"

Kanto_SixIsland_WaterPath_Text_843BE74::
	.string "Aren’t there any mountains around\n"
	.string "these parts?$"

Kanto_SixIsland_WaterPath_Text_843BEA3::
	.string "MIU: Hello, POKéMON!\n"
	.string "It’s time to play!$"

Kanto_SixIsland_WaterPath_Text_843BECB::
	.string "MIU: Oh, no!\n"
	.string "Mommy!$"

Kanto_SixIsland_WaterPath_Text_843BEDF::
	.string "MIU: It makes me all sad inside\n"
	.string "when we lose…$"

Kanto_SixIsland_WaterPath_Text_843BF0D::
	.string "MIU: MIA and I are TWINS.\n"
	.string "We want to battle together.$"

Kanto_SixIsland_WaterPath_Text_843BF43::
	.string "MIA: Hello, POKéMON!\n"
	.string "It’s time to battle!$"

Kanto_SixIsland_WaterPath_Text_843BF6D::
	.string "MIA: You’re mean!\n"
	.string "You’re bad for winning!$"

Kanto_SixIsland_WaterPath_Text_843BF97::
	.string "MIA: Ohh…\n"
	.string "I’m sorry, my POKéMON…$"

Kanto_SixIsland_WaterPath_Text_843BFB8::
	.string "MIA: Oh, you only have one\n"
	.string "POKéMON?\p"
	.string "Don’t you feel lonesome?$"

Kanto_SixIsland_WaterPath_Text_843BFF5::
	.string "Wanted!\n"
	.string "The Ultimate Horn!$"

Kanto_SixIsland_WaterPath_Text_843C010::
	.string "WATER PATH\n"
	.string "Leads to RUIN VALLEY$"

Kanto_SixIsland_WaterPath_Text_84714CE::
	.string "Oh, hello.\n"
	.string "A pleasant breeze is blowing again.$"

Kanto_SixIsland_WaterPath_Text_84714FD::
	.string "Hehehe, I’m practicing in secret\n"
	.string "again.$"

Kanto_SixIsland_WaterPath_Text_8471525::
	.string "What, you’re sick of seeing\n"
	.string "SWIMMERS like me?\p"
	.string "Aww, don’t be hatin’!\n"
	.string "Battle with me again.$"

Kanto_SixIsland_WaterPath_Text_847157F::
	.string "Sigh…\n"
	.string "My boyfriend’s busy again…$"

Kanto_SixIsland_WaterPath_Text_84715A0::
	.string "Come on, tell me, where are the\n"
	.string "mountains around these parts?$"

Kanto_SixIsland_WaterPath_Text_84715DE::
	.string "MIU: Hello, POKéMON!\n"
	.string "It’s time to play again!$"

Kanto_SixIsland_WaterPath_Text_847160C::
	.string "MIA: Hello, POKéMON!\n"
	.string "It’s time to battle again!$"
