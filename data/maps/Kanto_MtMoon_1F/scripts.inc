Kanto_MtMoon_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_MtMoon_1F_EventScript_840FF25
	.byte 0

Kanto_MtMoon_1F_EventScript_840FF25::
	call Quetzal_EventScript_840FBB5
	setflag FLAG_QZ_0x2242
	end

@ Kanto_MtMoon_1F sign / bg event at 19,26
Kanto_MtMoon_1F_EventScript_840FF2E::
	msgbox Kanto_MtMoon_1F_Text_84230C7, MSGBOX_SIGN
	end

@ Kanto_MtMoon_1F object 7 (local id 7, gfx 0x3F0C) at 7,10
Kanto_MtMoon_1F_EventScript_840FF37::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8373, 0, Kanto_MtMoon_1F_Text_8422D84, Kanto_MtMoon_1F_Text_8422DB2
	msgbox Kanto_MtMoon_1F_Text_8422DC6, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_1F object 6 (local id 6, gfx 0x3F5C) at 13,17
Kanto_MtMoon_1F_EventScript_840FF4E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8283, 0, Kanto_MtMoon_1F_Text_8422E04, Kanto_MtMoon_1F_Text_8422E2B
	msgbox Kanto_MtMoon_1F_Text_8422E4A, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_1F object 4 (local id 4, gfx 0x3F16) at 33,4
Kanto_MtMoon_1F_EventScript_840FF65::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8312, 0, Kanto_MtMoon_1F_Text_8422E7D, Kanto_MtMoon_1F_Text_8422EAA
	msgbox Kanto_MtMoon_1F_Text_8422EB9, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_1F object 1 (local id 1, gfx 0x3F16) at 20,26
Kanto_MtMoon_1F_EventScript_840FF7C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8313, 0, Kanto_MtMoon_1F_Text_8422F59, Kanto_MtMoon_1F_Text_8422F8B
	msgbox Kanto_MtMoon_1F_Text_8422F93, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_1F object 3 (local id 3, gfx 0x3F43) at 30,35
Kanto_MtMoon_1F_EventScript_840FF93::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8361, 0, Kanto_MtMoon_1F_Text_8422EF2, Kanto_MtMoon_1F_Text_8422F0E
	msgbox Kanto_MtMoon_1F_Text_8422F23, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_1F object 5 (local id 5, gfx 0x3EF1) at 7,26
Kanto_MtMoon_1F_EventScript_840FFAA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8300, 0, Kanto_MtMoon_1F_Text_8422FD1, Kanto_MtMoon_1F_Text_8423001
	msgbox Kanto_MtMoon_1F_Text_842300D, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_1F object 2 (local id 2, gfx 0x3EF1) at 36,30
Kanto_MtMoon_1F_EventScript_840FFC1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8301, 0, Kanto_MtMoon_1F_Text_842303C, Kanto_MtMoon_1F_Text_8423076
	msgbox Kanto_MtMoon_1F_Text_842307E, MSGBOX_AUTOCLOSE
	end

@ Kanto_MtMoon_1F object 14 (local id 14, gfx 0x3EE6) at 42,7
Kanto_MtMoon_1F_EventScript_845D1D4::
	lock
	faceplayer
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	msgbox Kanto_MtMoon_1F_Text_84230E7, MSGBOX_DEFAULT
	release
	end

@ Kanto_MtMoon_1F object 8 (local id 8, gfx 0x3F0E) at 2,22
Kanto_MtMoon_1F_EventScript_846BA85::
	setorcopyvar VAR_0x8000, ITEM_PARALYZE_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtMoon_1F object 9 (local id 9, gfx 0x3F0E) at 11,35
Kanto_MtMoon_1F_EventScript_846BA92::
	setorcopyvar VAR_0x8000, ITEM_MIRACLE_SEED
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtMoon_1F object 10 (local id 10, gfx 0x3F0E) at 26,32
Kanto_MtMoon_1F_EventScript_846BA9F::
	setorcopyvar VAR_0x8000, ITEM_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtMoon_1F object 11 (local id 11, gfx 0x3F0E) at 42,35
Kanto_MtMoon_1F_EventScript_846BAAC::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtMoon_1F object 12 (local id 12, gfx 0x3F0E) at 44,21
Kanto_MtMoon_1F_EventScript_846BAB9::
	setorcopyvar VAR_0x8000, ITEM_ESCAPE_ROPE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_MtMoon_1F object 13 (local id 13, gfx 0x3F0E) at 3,2
Kanto_MtMoon_1F_EventScript_846BAC6::
	setorcopyvar VAR_0x8000, ITEM_MOON_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_MtMoon_1F_Text_8422D84::
	.string "WHOA!\n"
	.string "You shocked me!\l"
	.string "…Oh, you’re just a kid!$"

Kanto_MtMoon_1F_Text_8422DB2::
	.string "Wow!\n"
	.string "Shocked again!$"

Kanto_MtMoon_1F_Text_8422DC6::
	.string "Kids like you shouldn’t be\n"
	.string "wandering around here in the dark.$"

Kanto_MtMoon_1F_Text_8422E04::
	.string "Did you come to explore the cave,\n"
	.string "too?$"

Kanto_MtMoon_1F_Text_8422E2B::
	.string "Losing stinks!\n"
	.string "It’s so uncool.$"

Kanto_MtMoon_1F_Text_8422E4A::
	.string "I came all the way down here to\n"
	.string "show off to girls.$"

Kanto_MtMoon_1F_Text_8422E7D::
	.string "Wow! It’s way bigger in here than\n"
	.string "I thought!$"

Kanto_MtMoon_1F_Text_8422EAA::
	.string "Oh!\n"
	.string "I lost it!$"

Kanto_MtMoon_1F_Text_8422EB9::
	.string "How do you get out of here?\n"
	.string "It’s so big, I may get lost.$"

Kanto_MtMoon_1F_Text_8422EF2::
	.string "What!\n"
	.string "Don’t sneak up on me!$"

Kanto_MtMoon_1F_Text_8422F0E::
	.string "My POKéMON won’t do!$"

Kanto_MtMoon_1F_Text_8422F23::
	.string "I have to find stronger POKéMON.\n"
	.string "Where might they be?$"

Kanto_MtMoon_1F_Text_8422F59::
	.string "What?\n"
	.string "I’m waiting for my friends to find\l"
	.string "me here.$"

Kanto_MtMoon_1F_Text_8422F8B::
	.string "I lost?$"

Kanto_MtMoon_1F_Text_8422F93::
	.string "I came because I heard there are\n"
	.string "some very rare fossils here.$"

Kanto_MtMoon_1F_Text_8422FD1::
	.string "Suspicious men are in the cave.\n"
	.string "What about you?$"

Kanto_MtMoon_1F_Text_8423001::
	.string "You got me!$"

Kanto_MtMoon_1F_Text_842300D::
	.string "I saw them!\n"
	.string "I’m sure they’re from TEAM ROCKET!$"

Kanto_MtMoon_1F_Text_842303C::
	.string "You need to go through this cave\n"
	.string "to get to CERULEAN CITY.$"

Kanto_MtMoon_1F_Text_8423076::
	.string "I lost.$"

Kanto_MtMoon_1F_Text_842307E::
	.string "ZUBAT is tough!\n"
	.string "But if you can catch one, you’ll\l"
	.string "be able to count on it.$"

Kanto_MtMoon_1F_Text_84230C7::
	.string "Beware!\n"
	.string "ZUBAT is a bloodsucker!$"

Kanto_MtMoon_1F_Text_84230E7::
	.string "Hi, I’m excavating for fossils here\n"
	.string "under MT. MOON.\p"
	.string "Sometimes, BROCK of PEWTER GYM\n"
	.string "lends me a hand.$"
