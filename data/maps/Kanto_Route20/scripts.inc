Kanto_Route20_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_Route20_EventScript_8417F3D
	.byte 0

Kanto_Route20_EventScript_8417F3D::
	call_if_unset FLAG_QZ_0x2208, Kanto_Route20_EventScript_8417F55
	call_if_unset FLAG_QZ_0x2209, Kanto_Route20_EventScript_8417F6E
	call Quetzal_EventScript_840FBB5
	end

Kanto_Route20_EventScript_8417F55::
	clearflag FLAG_QZ_0x21BB
	clearflag FLAG_QZ_0x21BC
	setflag FLAG_QZ_0x21BD
	setflag FLAG_QZ_0x21BE
	setflag FLAG_QZ_0x21BF
	setflag FLAG_QZ_0x21C0
	setflag FLAG_QZ_0x21C1
	setflag FLAG_QZ_0x21C2
	return

Kanto_Route20_EventScript_8417F6E::
	clearflag FLAG_QZ_0x21C3
	clearflag FLAG_QZ_0x21C4
	clearflag FLAG_QZ_0x21C5
	clearflag FLAG_QZ_0x21C6
	setflag FLAG_QZ_0x21C7
	setflag FLAG_QZ_0x21C8
	return

@ Kanto_Route20 sign / bg event at 68,14
@ Kanto_Route20 sign / bg event at 64,8
Kanto_Route20_EventScript_8417F81::
	msgbox Kanto_Route20_Text_84385C8, MSGBOX_SIGN
	end

@ Kanto_Route20 object 7 (local id 7, gfx 0x3F4F) at 111,9
Kanto_Route20_EventScript_845BADA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8434, 0, Kanto_Route20_Text_8438137, Kanto_Route20_Text_8438172
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BB01
	msgbox Kanto_Route20_Text_843817A, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BB01::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8434, 0, Kanto_Route20_Text_8470499, Kanto_Route20_Text_8438172
	msgbox Kanto_Route20_Text_843817A, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 5 (local id 5, gfx 0x3F4F) at 23,10
Kanto_Route20_EventScript_845BB18::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8435, 0, Kanto_Route20_Text_84382D2, Kanto_Route20_Text_84382EE
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BB3F
	msgbox Kanto_Route20_Text_84382F5, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BB3F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8435, 0, Kanto_Route20_Text_8470556, Kanto_Route20_Text_84382EE
	msgbox Kanto_Route20_Text_84382F5, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 8 (local id 8, gfx 0x3F4F) at 106,11
Kanto_Route20_EventScript_845BB56::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8436, 0, Kanto_Route20_Text_8438325, Kanto_Route20_Text_8438353
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BB7D
	msgbox Kanto_Route20_Text_8438364, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BB7D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8436, 0, Kanto_Route20_Text_8470593, Kanto_Route20_Text_8438353
	msgbox Kanto_Route20_Text_8438364, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 10 (local id 10, gfx 0x3F4D) at 54,11
Kanto_Route20_EventScript_845BB94::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8462, 0, Kanto_Route20_Text_843821B, Kanto_Route20_Text_8438251
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BBBB
	msgbox Kanto_Route20_Text_8438258, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BBBB::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8462, 0, Kanto_Route20_Text_8470505, Kanto_Route20_Text_8438251
	msgbox Kanto_Route20_Text_8438258, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 3 (local id 3, gfx 0x3F4D) at 34,7
Kanto_Route20_EventScript_845BBD2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8463, 0, Kanto_Route20_Text_84383EF, Kanto_Route20_Text_8438410
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BBF9
	msgbox Kanto_Route20_Text_8438430, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BBF9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8463, 0, Kanto_Route20_Text_84705EF, Kanto_Route20_Text_8438410
	msgbox Kanto_Route20_Text_8438430, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 1 (local id 1, gfx 0x3F4D) at 8,11
Kanto_Route20_EventScript_845BC10::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8464, 0, Kanto_Route20_Text_843850F, Kanto_Route20_Text_8438553
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BC37
	msgbox Kanto_Route20_Text_8438572, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BC37::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8464, 0, Kanto_Route20_Text_847066A, Kanto_Route20_Text_8438553
	msgbox Kanto_Route20_Text_8438572, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 9 (local id 9, gfx 0x3F4D) at 93,10
Kanto_Route20_EventScript_845BC4E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8471, 0, Kanto_Route20_Text_84381B2, Kanto_Route20_Text_84381E4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BC75
	msgbox Kanto_Route20_Text_84381ED, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BC75::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8471, 0, Kanto_Route20_Text_84704D4, Kanto_Route20_Text_84381E4
	msgbox Kanto_Route20_Text_84381ED, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 4 (local id 4, gfx 0x3F3B) at 43,9
Kanto_Route20_EventScript_845BC8C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8502, 0, Kanto_Route20_Text_8438385, Kanto_Route20_Text_84383A2
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BCB3
	msgbox Kanto_Route20_Text_84383BF, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BCB3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8502, 0, Kanto_Route20_Text_84705D2, Kanto_Route20_Text_84383A2
	msgbox Kanto_Route20_Text_84383BF, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 2 (local id 2, gfx 0x3F31) at 16,14
Kanto_Route20_EventScript_845BCCA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8664, 0, Kanto_Route20_Text_843845C, Kanto_Route20_Text_843849A
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BCF1
	msgbox Kanto_Route20_Text_84384AF, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BCF1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8664, 0, Kanto_Route20_Text_847062B, Kanto_Route20_Text_843849A
	msgbox Kanto_Route20_Text_84384AF, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 6 (local id 6, gfx 0x3F31) at 73,16
Kanto_Route20_EventScript_845BD08::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8665, 0, Kanto_Route20_Text_843826F, Kanto_Route20_Text_8438289
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route20_EventScript_845BD2F
	msgbox Kanto_Route20_Text_843829A, MSGBOX_AUTOCLOSE
	end

Kanto_Route20_EventScript_845BD2F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8665, 0, Kanto_Route20_Text_847053B, Kanto_Route20_Text_8438289
	msgbox Kanto_Route20_Text_843829A, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route20 object 11 (local id 11, gfx 0x3EF3) at 69,3
Kanto_Route20_EventScript_845D207::
	lock
	faceplayer
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	msgbox Kanto_Route20_Text_84385D8, MSGBOX_DEFAULT
	release
	end

Kanto_Route20_Text_8438137::
	.string "The water is shallow here.\n"
	.string "There are many people swimming.$"

Kanto_Route20_Text_8438172::
	.string "Splash!$"

Kanto_Route20_Text_843817A::
	.string "I wish I could ride my POKéMON.\n"
	.string "I bet you’re not tired.$"

Kanto_Route20_Text_84381B2::
	.string "SEAFOAM is a quiet getaway.\n"
	.string "I’m vacationing here.$"

Kanto_Route20_Text_84381E4::
	.string "Quit it!$"

Kanto_Route20_Text_84381ED::
	.string "There’s a huge cavern underneath\n"
	.string "this island.$"

Kanto_Route20_Text_843821B::
	.string "I love floating with the fishes\n"
	.string "here among the waves.$"

Kanto_Route20_Text_8438251::
	.string "Yowch!$"

Kanto_Route20_Text_8438258::
	.string "Want to float with me?$"

Kanto_Route20_Text_843826F::
	.string "Are you on vacation, too?$"

Kanto_Route20_Text_8438289::
	.string "No mercy at all!$"

Kanto_Route20_Text_843829A::
	.string "SEAFOAM used to be a single island\n"
	.string "in the distant past.$"

Kanto_Route20_Text_84382D2::
	.string "Check out my buff physique!$"

Kanto_Route20_Text_84382EE::
	.string "Wimpy!$"

Kanto_Route20_Text_84382F5::
	.string "I should’ve been buffing up my\n"
	.string "POKéMON, not me!$"

Kanto_Route20_Text_8438325::
	.string "Why are you riding a POKéMON?\n"
	.string "Can’t you swim?$"

Kanto_Route20_Text_8438353::
	.string "Ouch!\n"
	.string "Torpedoed!$"

Kanto_Route20_Text_8438364::
	.string "Riding a POKéMON sure looks fun!$"

Kanto_Route20_Text_8438385::
	.string "I rode my bird POKéMON here.$"

Kanto_Route20_Text_84383A2::
	.string "Oh, no!\n"
	.string "Now what am I to do?$"

Kanto_Route20_Text_84383BF::
	.string "My birds are exhausted.\n"
	.string "They can’t FLY me back!$"

Kanto_Route20_Text_84383EF::
	.string "My boyfriend gave me big pearls.$"

Kanto_Route20_Text_8438410::
	.string "Oh, no!\n"
	.string "My pearls were in them!$"

Kanto_Route20_Text_8438430::
	.string "Will my pearls grow bigger inside\n"
	.string "CLOYSTER?$"

Kanto_Route20_Text_843845C::
	.string "I swam here from CINNABAR ISLAND.\n"
	.string "It wasn’t easy, I tell you.$"

Kanto_Route20_Text_843849A::
	.string "I’m so disappointed!$"

Kanto_Route20_Text_84384AF::
	.string "POKéMON have taken over an\n"
	.string "abandoned mansion on CINNABAR.\p"
	.string "They call it the POKéMON MANSION\n"
	.string "now.$"

Kanto_Route20_Text_843850F::
	.string "CINNABAR, in the west, has a LAB\n"
	.string "for POKéMON.\p"
	.string "My daddy works there.$"

Kanto_Route20_Text_8438553::
	.string "Wait!\n"
	.string "You’re supposed to wait!$"

Kanto_Route20_Text_8438572::
	.string "CINNABAR is a volcanic island.\p"
	.string "I heard it rose out of the sea\n"
	.string "when a volcano erupted.$"

Kanto_Route20_Text_84385C8::
	.string "SEAFOAM ISLANDS$"

Kanto_Route20_Text_84385D8::
	.string "Strong TRAINERS and WATER POKéMON\n"
	.string "are common sights in these parts.\p"
	.string "They say that MISTY of the\n"
	.string "CERULEAN GYM trains here.$"

Kanto_Route20_Text_8470499::
	.string "The water is shallow here.\n"
	.string "There are many people swimming.$"

Kanto_Route20_Text_84704D4::
	.string "Are you jealous that I’m\n"
	.string "vacationing on SEAFOAM?$"

Kanto_Route20_Text_8470505::
	.string "I love floating with the fishes\n"
	.string "here among the waves.$"

Kanto_Route20_Text_847053B::
	.string "Were you on vacation, too?$"

Kanto_Route20_Text_8470556::
	.string "Check out my physique!\p"
	.string "It’s even more buff than ever\n"
	.string "before!$"

Kanto_Route20_Text_8470593::
	.string "Why are you riding a POKéMON?\n"
	.string "Haven’t you learned to swim yet?$"

Kanto_Route20_Text_84705D2::
	.string "I rode my bird POKéMON here.$"

Kanto_Route20_Text_84705EF::
	.string "My boyfriend gave me big pearls.\n"
	.string "And, they’ve grown bigger!$"

Kanto_Route20_Text_847062B::
	.string "I swam here from CINNABAR ISLAND.\n"
	.string "It wasn’t easy, like I said.$"

Kanto_Route20_Text_847066A::
	.string "CINNABAR, in the west, has a LAB\n"
	.string "for POKéMON.\p"
	.string "My daddy works there.$"
