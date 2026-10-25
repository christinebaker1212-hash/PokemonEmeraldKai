Kanto_Route6_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route6 sign / bg event at 21,15
Kanto_Route6_EventScript_8417C6E::
	msgbox Kanto_Route6_Text_843432E, MSGBOX_SIGN
	end

@ Kanto_Route6 object 1 (local id 1, gfx 0x3EF1) at 3,16
Kanto_Route6_EventScript_8459D6A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8303, 0, Kanto_Route6_Text_8434166, Kanto_Route6_Text_8434187
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route6_EventScript_8459D91
	msgbox Kanto_Route6_Text_843419B, MSGBOX_AUTOCLOSE
	end

Kanto_Route6_EventScript_8459D91::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8303, 0, Kanto_Route6_Text_846EB88, Kanto_Route6_Text_8434187
	msgbox Kanto_Route6_Text_843419B, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route6 object 4 (local id 4, gfx 0x3EF1) at 20,25
Kanto_Route6_EventScript_8459DA8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8304, 0, Kanto_Route6_Text_84342B7, Kanto_Route6_Text_84342E1
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route6_EventScript_8459DCF
	msgbox Kanto_Route6_Text_84342F2, MSGBOX_AUTOCLOSE
	end

Kanto_Route6_EventScript_8459DCF::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8304, 0, Kanto_Route6_Text_846EC11, Kanto_Route6_Text_84342E1
	msgbox Kanto_Route6_Text_84342F2, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route6 object 2 (local id 2, gfx 0x3EF3) at 12,21
Kanto_Route6_EventScript_8459DE6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8337, 0, Kanto_Route6_Text_84340CB, Kanto_Route6_Text_84340F1
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route6_EventScript_8459E0D
	msgbox Kanto_Route6_Text_8434103, MSGBOX_AUTOCLOSE
	end

Kanto_Route6_EventScript_8459E0D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8337, 0, Kanto_Route6_Text_846EAE2, Kanto_Route6_Text_84340F1
	msgbox Kanto_Route6_Text_8434103, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route6 object 6 (local id 6, gfx 0x3EF3) at 13,33
Kanto_Route6_EventScript_8459E24::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8338, 0, Kanto_Route6_Text_84341CE, Kanto_Route6_Text_84341EB
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route6_EventScript_8459E4B
	msgbox Kanto_Route6_Text_8434218, MSGBOX_AUTOCLOSE
	end

Kanto_Route6_EventScript_8459E4B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8338, 0, Kanto_Route6_Text_846EBC9, Kanto_Route6_Text_84341EB
	msgbox Kanto_Route6_Text_8434218, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route6 object 3 (local id 3, gfx 0x3F31) at 13,21
Kanto_Route6_EventScript_8459E62::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8343, 0, Kanto_Route6_Text_8434115, Kanto_Route6_Text_8434140
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route6_EventScript_8459E89
	msgbox Kanto_Route6_Text_8434154, MSGBOX_AUTOCLOSE
	end

Kanto_Route6_EventScript_8459E89::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8343, 0, Kanto_Route6_Text_846EB13, Kanto_Route6_Text_8434140
	msgbox Kanto_Route6_Text_8434154, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route6 object 5 (local id 5, gfx 0x3F31) at 13,32
Kanto_Route6_EventScript_8459EA0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8344, 0, Kanto_Route6_Text_8434257, Kanto_Route6_Text_8434272
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route6_EventScript_8459EC7
	msgbox Kanto_Route6_Text_843428B, MSGBOX_AUTOCLOSE
	end

Kanto_Route6_EventScript_8459EC7::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8344, 0, Kanto_Route6_Text_846EBEC, Kanto_Route6_Text_8434272
	msgbox Kanto_Route6_Text_843428B, MSGBOX_AUTOCLOSE
	end

Kanto_Route6_Text_84340CB::
	.string "Who’s there?\n"
	.string "Quit listening in on us!$"

Kanto_Route6_Text_84340F1::
	.string "I just can’t win!$"

Kanto_Route6_Text_8434103::
	.string "Whisper…\n"
	.string "Whisper…$"

Kanto_Route6_Text_8434115::
	.string "Excuse me!\n"
	.string "This is a private conversation!$"

Kanto_Route6_Text_8434140::
	.string "Ugh!\n"
	.string "I hate losing.$"

Kanto_Route6_Text_8434154::
	.string "Whisper…\n"
	.string "Whisper…$"

Kanto_Route6_Text_8434166::
	.string "There aren’t many bugs out here.$"

Kanto_Route6_Text_8434187::
	.string "No!\n"
	.string "You’re kidding!$"

Kanto_Route6_Text_843419B::
	.string "I like bugs, so I’m going back to\n"
	.string "VIRIDIAN FOREST.$"

Kanto_Route6_Text_84341CE::
	.string "Huh?\n"
	.string "You want to talk to me?$"

Kanto_Route6_Text_84341EB::
	.string "This stinks…\n"
	.string "I couldn’t beat your challenge…$"

Kanto_Route6_Text_8434218::
	.string "I should bring more POKéMON with\n"
	.string "me. I’ll feel safer that way.$"

Kanto_Route6_Text_8434257::
	.string "Me?\n"
	.string "Well, okay. I’ll play!$"

Kanto_Route6_Text_8434272::
	.string "Things just didn’t work…$"

Kanto_Route6_Text_843428B::
	.string "I want to get stronger.\n"
	.string "What’s your secret?$"

Kanto_Route6_Text_84342B7::
	.string "I’ve never seen you around.\n"
	.string "Are you good?$"

Kanto_Route6_Text_84342E1::
	.string "You’re too good!$"

Kanto_Route6_Text_84342F2::
	.string "Are my POKéMON weak?\n"
	.string "Or, am I just bad?\l"
	.string "Which do you think?$"

Kanto_Route6_Text_843432E::
	.string "UNDERGROUND PATH\n"
	.string "CERULEAN CITY - VERMILION CITY$"

Kanto_Route6_Text_846EAE2::
	.string "Oh! You’re that nosy kid who\n"
	.string "eavesdropped on us!$"

Kanto_Route6_Text_846EB13::
	.string "Excuse me! Didn’t I tell you that\n"
	.string "this is a private conversation?\p"
	.string "You shouldn’t be listening in,\n"
	.string "you uncouth person!$"

Kanto_Route6_Text_846EB88::
	.string "I’m trying to find something good\n"
	.string "that’s not a BUG POKéMON, but…$"

Kanto_Route6_Text_846EBC9::
	.string "Huh?\n"
	.string "You want to go with me again?$"

Kanto_Route6_Text_846EBEC::
	.string "Me?\n"
	.string "Well, okay. I’ll play this once.$"

Kanto_Route6_Text_846EC11::
	.string "Hey, long time!\n"
	.string "Have you gotten better?$"
