Kanto_SixIsland_RuinValley_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	map_script MAP_SCRIPT_ON_LOAD, Kanto_SixIsland_RuinValley_EventScript_8418915
	.byte 0

Kanto_SixIsland_RuinValley_EventScript_8418915::
	call_if_set FLAG_QZ_0x222F, Kanto_SixIsland_RuinValley_EventScript_841891F
	end

Kanto_SixIsland_RuinValley_EventScript_841891F::
	setmetatile 0x18, 0x18, 0x358, 0
	return

@ Kanto_SixIsland_RuinValley object 1 (local id 1, gfx 0x3F43) at 24,25
Kanto_SixIsland_RuinValley_EventScript_8418929::
	lock
	msgbox Kanto_SixIsland_RuinValley_Text_843C030, MSGBOX_DEFAULT
	applymovement 1, Quetzal_Movement_8457AA4
	waitmovement 0
	msgbox Kanto_SixIsland_RuinValley_Text_843C063, MSGBOX_DEFAULT
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_SixIsland_RuinValley sign / bg event at 24,24
Kanto_SixIsland_RuinValley_EventScript_8418950::
	lockall
	goto_if_set FLAG_QZ_0x222F, Kanto_SixIsland_RuinValley_EventScript_84189C0
	msgbox Kanto_SixIsland_RuinValley_Text_843C0A9, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SixIsland_RuinValley_EventScript_84189CA
	msgbox Kanto_SixIsland_RuinValley_Text_843C0FA, MSGBOX_DEFAULT
	closemessage
	braillemessage Kanto_SixIsland_RuinValley_Braille_845959A
	waitbuttonpress
	closebraillemessage
	goto_if_unset FLAG_QZ_0x21B2, Kanto_SixIsland_RuinValley_EventScript_84189BE
	setvar VAR_0x8004, 0xF
	special QzHasBadgeForFieldMove
	goto_if_eq VAR_RESULT, 0, Kanto_SixIsland_RuinValley_EventScript_84189BE
	msgbox Kanto_SixIsland_RuinValley_Text_843C134, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_SixIsland_RuinValley_EventScript_84189CA
	closemessage
	playse SE_M_CUT
	setflag FLAG_QZ_0x222F
	call Kanto_SixIsland_RuinValley_EventScript_841891F
	special DrawWholeMapView
	waitse
	releaseall
	end

Kanto_SixIsland_RuinValley_EventScript_84189BE::
	releaseall
	end

Kanto_SixIsland_RuinValley_EventScript_84189C0::
	msgbox Kanto_SixIsland_RuinValley_Text_843C163, MSGBOX_DEFAULT
	releaseall
	end

Kanto_SixIsland_RuinValley_EventScript_84189CA::
	msgbox Kanto_SixIsland_RuinValley_Text_843C0E2, MSGBOX_DEFAULT
	releaseall
	end

@ Kanto_SixIsland_RuinValley object 2 (local id 2, gfx 0x3F0C) at 32,11
Kanto_SixIsland_RuinValley_EventScript_845CAE8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8716, 0, Kanto_SixIsland_RuinValley_Text_843C175, Kanto_SixIsland_RuinValley_Text_843C1B3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_RuinValley_EventScript_845CB0F
	msgbox Kanto_SixIsland_RuinValley_Text_843C1B8, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_RuinValley_EventScript_845CB0F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8716, 0, Kanto_SixIsland_RuinValley_Text_847163C, Kanto_SixIsland_RuinValley_Text_843C1B3
	msgbox Kanto_SixIsland_RuinValley_Text_843C1B8, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_RuinValley object 3 (local id 3, gfx 0x3F0C) at 33,16
Kanto_SixIsland_RuinValley_EventScript_845CB26::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8774, 0, Kanto_SixIsland_RuinValley_Text_843C21D, Kanto_SixIsland_RuinValley_Text_843C24B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_RuinValley_EventScript_845CB4D
	msgbox Kanto_SixIsland_RuinValley_Text_843C284, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_RuinValley_EventScript_845CB4D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8774, 0, Kanto_SixIsland_RuinValley_Text_847167A, Kanto_SixIsland_RuinValley_Text_843C24B
	msgbox Kanto_SixIsland_RuinValley_Text_843C284, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_RuinValley object 4 (local id 4, gfx 0x3F0C) at 31,24
Kanto_SixIsland_RuinValley_EventScript_845CB64::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8775, 0, Kanto_SixIsland_RuinValley_Text_843C2DD, Kanto_SixIsland_RuinValley_Text_843C353
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_RuinValley_EventScript_845CB8B
	msgbox Kanto_SixIsland_RuinValley_Text_843C364, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_RuinValley_EventScript_845CB8B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8775, 0, Kanto_SixIsland_RuinValley_Text_84716B6, Kanto_SixIsland_RuinValley_Text_843C353
	msgbox Kanto_SixIsland_RuinValley_Text_843C364, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_RuinValley object 5 (local id 5, gfx 0x3F0C) at 14,10
Kanto_SixIsland_RuinValley_EventScript_845CBA2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8776, 0, Kanto_SixIsland_RuinValley_Text_843C3B4, Kanto_SixIsland_RuinValley_Text_843C3D3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_RuinValley_EventScript_845CBC9
	msgbox Kanto_SixIsland_RuinValley_Text_843C402, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_RuinValley_EventScript_845CBC9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8776, 0, Kanto_SixIsland_RuinValley_Text_8471726, Kanto_SixIsland_RuinValley_Text_843C3D3
	msgbox Kanto_SixIsland_RuinValley_Text_843C402, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_RuinValley object 6 (local id 6, gfx 0x3F4B) at 21,29
Kanto_SixIsland_RuinValley_EventScript_845CBE0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8777, 0, Kanto_SixIsland_RuinValley_Text_843C422, Kanto_SixIsland_RuinValley_Text_843C458
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SixIsland_RuinValley_EventScript_845CC07
	msgbox Kanto_SixIsland_RuinValley_Text_843C492, MSGBOX_AUTOCLOSE
	end

Kanto_SixIsland_RuinValley_EventScript_845CC07::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8777, 0, Kanto_SixIsland_RuinValley_Text_8471746, Kanto_SixIsland_RuinValley_Text_843C458
	msgbox Kanto_SixIsland_RuinValley_Text_843C492, MSGBOX_AUTOCLOSE
	end

@ Kanto_SixIsland_RuinValley object 16 (local id 16, gfx 0x3F0E) at 19,11
Kanto_SixIsland_RuinValley_EventScript_846C19D::
	setorcopyvar VAR_0x8000, ITEM_FULL_RESTORE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SixIsland_RuinValley object 15 (local id 15, gfx 0x3F0E) at 5,33
Kanto_SixIsland_RuinValley_EventScript_846C1AA::
	setorcopyvar VAR_0x8000, ITEM_HP_UP
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SixIsland_RuinValley object 17 (local id 17, gfx 0x3F0E) at 43,32
Kanto_SixIsland_RuinValley_EventScript_846C1B7::
	setorcopyvar VAR_0x8000, ITEM_SUN_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SixIsland_RuinValley_Braille_845959A:
	@ braille
	.string "      Âぅì$"

Kanto_SixIsland_RuinValley_Text_843C030::
	.string "I can’t figure out how I’m supposed\n"
	.string "to get inside.$"

Kanto_SixIsland_RuinValley_Text_843C063::
	.string "Let me tell you, I found this\n"
	.string "place.\p"
	.string "Don’t look so envious, will you?$"

Kanto_SixIsland_RuinValley_Text_843C0A9::
	.string "The door doesn’t budge at all.\n"
	.string "Check it more thoroughly?$"

Kanto_SixIsland_RuinValley_Text_843C0E2::
	.string "{PLAYER} left the door alone.$"

Kanto_SixIsland_RuinValley_Text_843C0FA::
	.string "On closer inspection, there are\n"
	.string "several dots on the door…$"

Kanto_SixIsland_RuinValley_Text_843C134::
	.string "The inscription says CUT.\n"
	.string "Use CUT on the door?$"

Kanto_SixIsland_RuinValley_Text_843C163::
	.string "The door is open.$"

Kanto_SixIsland_RuinValley_Text_843C175::
	.string "There appear to be many secrets\n"
	.string "still unsolved in this world.$"

Kanto_SixIsland_RuinValley_Text_843C1B3::
	.string "Hmm…$"

Kanto_SixIsland_RuinValley_Text_843C1B8::
	.string "I suggest to you that you, too,\n"
	.string "give a thought to the secrets and\l"
	.string "mysteries of our world once a day.$"

Kanto_SixIsland_RuinValley_Text_843C21D::
	.string "I’m curious, what compelled you to\n"
	.string "come here?$"

Kanto_SixIsland_RuinValley_Text_843C24B::
	.string "It’s been so long since I battled,\n"
	.string "my instincts are off.$"

Kanto_SixIsland_RuinValley_Text_843C284::
	.string "There are several ruins on the\n"
	.string "SEVII ISLANDS.\p"
	.string "The majority of them date back\n"
	.string "a long way.$"

Kanto_SixIsland_RuinValley_Text_843C2DD::
	.string "It’s been said that there are\n"
	.string "mysterious stones on this island.\p"
	.string "I suspect it has something to do\n"
	.string "with the ruins here.$"

Kanto_SixIsland_RuinValley_Text_843C353::
	.string "Oh, interesting.$"

Kanto_SixIsland_RuinValley_Text_843C364::
	.string "Those ruins over there…\p"
	.string "I haven’t had any luck in finding\n"
	.string "how I can get inside.$"

Kanto_SixIsland_RuinValley_Text_843C3B4::
	.string "A battle with you, so high up!$"

Kanto_SixIsland_RuinValley_Text_843C3D3::
	.string "Oh, the heartbreak of losing so\n"
	.string "easily to you!$"

Kanto_SixIsland_RuinValley_Text_843C402::
	.string "Experience the joy of climbing!$"

Kanto_SixIsland_RuinValley_Text_843C422::
	.string "I’m pretty familiar with the land\n"
	.string "around these parts.$"

Kanto_SixIsland_RuinValley_Text_843C458::
	.string "You don’t need to be so pushy.\n"
	.string "I’ll be happy to tell you.$"

Kanto_SixIsland_RuinValley_Text_843C492::
	.string "The door to the ruins…\n"
	.string "It has these strange patterns\l"
	.string "engraved on the outside.\p"
	.string "I don’t know what they mean,\n"
	.string "though.$"

Kanto_SixIsland_RuinValley_Text_847163C::
	.string "There appear to be many secrets\n"
	.string "still unsolved in this world.$"

Kanto_SixIsland_RuinValley_Text_847167A::
	.string "Hi, didn’t we meet before?\n"
	.string "What compelled you to come back?$"

Kanto_SixIsland_RuinValley_Text_84716B6::
	.string "It’s been said that there are\n"
	.string "mysterious stones on this island.\p"
	.string "Have you discovered anything since\n"
	.string "we last met?$"

Kanto_SixIsland_RuinValley_Text_8471726::
	.string "A rematch with you, so high up!$"

Kanto_SixIsland_RuinValley_Text_8471746::
	.string "I’m pretty familiar with the land\n"
	.string "around these parts.$"
