Kanto_ViridianForest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_ViridianForest_EventScript_840FE5B
	.byte 0

Kanto_ViridianForest_EventScript_840FE5B::
	setflag FLAG_QZ_0x2265
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_ViridianForest object 1 (local id 1, gfx 0x3F5C) at 29,58
Kanto_ViridianForest_EventScript_840FE64::
	msgbox Kanto_ViridianForest_Text_842283C, MSGBOX_NPC
	end

@ Kanto_ViridianForest object 2 (local id 2, gfx 0x3EEE) at 45,58
Kanto_ViridianForest_EventScript_840FE6D::
	msgbox Kanto_ViridianForest_Text_8422B40, MSGBOX_NPC
	end

@ Kanto_ViridianForest sign / bg event at 39,59
Kanto_ViridianForest_EventScript_840FE76::
	msgbox Kanto_ViridianForest_Text_8422BAE, MSGBOX_SIGN
	end

@ Kanto_ViridianForest sign / bg event at 28,44
Kanto_ViridianForest_EventScript_840FE7F::
	msgbox Kanto_ViridianForest_Text_8422C10, MSGBOX_SIGN
	end

@ Kanto_ViridianForest sign / bg event at 43,26
Kanto_ViridianForest_EventScript_840FE88::
	msgbox Kanto_ViridianForest_Text_8422C43, MSGBOX_SIGN
	end

@ Kanto_ViridianForest sign / bg event at 9,29
Kanto_ViridianForest_EventScript_840FE91::
	msgbox Kanto_ViridianForest_Text_8422C8A, MSGBOX_SIGN
	end

@ Kanto_ViridianForest sign / bg event at 31,60
Kanto_ViridianForest_EventScript_840FE9A::
	msgbox Kanto_ViridianForest_Text_8422D04, MSGBOX_SIGN
	end

@ Kanto_ViridianForest sign / bg event at 6,12
Kanto_ViridianForest_EventScript_840FEA3::
	msgbox Kanto_ViridianForest_Text_8422D5A, MSGBOX_SIGN
	end

@ Kanto_ViridianForest object 3 (local id 3, gfx 0x3EF1) at 47,45
Kanto_ViridianForest_EventScript_840FEAC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8294, 0, Kanto_ViridianForest_Text_84228AE, Kanto_ViridianForest_Text_84228E0
	msgbox Kanto_ViridianForest_Text_84228FC, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianForest object 4 (local id 4, gfx 0x3EF1) at 47,29
Kanto_ViridianForest_EventScript_840FEC3::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8295, 0, Kanto_ViridianForest_Text_8422931, Kanto_ViridianForest_Text_8422964
	msgbox Kanto_ViridianForest_Text_842297F, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianForest object 5 (local id 5, gfx 0x3EF1) at 7,22
Kanto_ViridianForest_EventScript_840FEDA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8296, 0, Kanto_ViridianForest_Text_84229BB, Kanto_ViridianForest_Text_84229E9
	msgbox Kanto_ViridianForest_Text_8422A06, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianForest object 9 (local id 9, gfx 0x3EF1) at 43,6
Kanto_ViridianForest_EventScript_840FEF1::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8723, 0, Kanto_ViridianForest_Text_8422A69, Kanto_ViridianForest_Text_8422AA6
	msgbox Kanto_ViridianForest_Text_8422AC3, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianForest object 10 (local id 10, gfx 0x3EF1) at 16,5
Kanto_ViridianForest_EventScript_840FF08::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8724, 0, Kanto_ViridianForest_Text_8422AE0, Kanto_ViridianForest_Text_8422B02
	msgbox Kanto_ViridianForest_Text_8422B0E, MSGBOX_AUTOCLOSE
	end

@ Kanto_ViridianForest object 6 (local id 6, gfx 0x3F0E) at 5,41
Kanto_ViridianForest_EventScript_846BA5E::
	setorcopyvar VAR_0x8000, ITEM_POKE_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_ViridianForest object 7 (local id 7, gfx 0x3F0E) at 40,21
Kanto_ViridianForest_EventScript_846BA6B::
	setorcopyvar VAR_0x8000, ITEM_ANTIDOTE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_ViridianForest object 8 (local id 8, gfx 0x3F0E) at 21,34
Kanto_ViridianForest_EventScript_846BA78::
	setorcopyvar VAR_0x8000, ITEM_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_ViridianForest object 11 (local id 11, gfx 0x3F0E) at 49,60
Kanto_ViridianForest_EventScript_846BFAF::
	setorcopyvar VAR_0x8000, ITEM_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_ViridianForest_Text_842283C::
	.string "I came here with some friends to\n"
	.string "catch us some BUG POKéMON!\p"
	.string "They’re all itching to get into\n"
	.string "some POKéMON battles!$"

Kanto_ViridianForest_Text_84228AE::
	.string "Hey! You have POKéMON!\n"
	.string "Come on!\l"
	.string "Let’s battle ’em!$"

Kanto_ViridianForest_Text_84228E0::
	.string "No!\n"
	.string "CATERPIE can’t hack it!$"

Kanto_ViridianForest_Text_84228FC::
	.string "Ssh! You’ll scare the bugs away.\n"
	.string "Another time, okay?$"

Kanto_ViridianForest_Text_8422931::
	.string "Yo!\n"
	.string "You can’t jam out if you’re a\l"
	.string "POKéMON TRAINER!$"

Kanto_ViridianForest_Text_8422964::
	.string "Huh?\n"
	.string "I ran out of POKéMON!$"

Kanto_ViridianForest_Text_842297F::
	.string "That totally stinks! I’m going to\n"
	.string "catch some stronger ones!$"

Kanto_ViridianForest_Text_84229BB::
	.string "Hey, wait up!\n"
	.string "What’s the hurry? Why the rush?$"

Kanto_ViridianForest_Text_84229E9::
	.string "I give!\n"
	.string "You’re good at this!$"

Kanto_ViridianForest_Text_8422A06::
	.string "Sometimes, you can find stuff on\n"
	.string "the ground.\p"
	.string "I’m looking for the stuff I\n"
	.string "dropped. Can you help me?$"

Kanto_ViridianForest_Text_8422A69::
	.string "I might be little, but I won’t like\n"
	.string "it if you go easy on me!$"

Kanto_ViridianForest_Text_8422AA6::
	.string "Oh, boo.\n"
	.string "Nothing went right.$"

Kanto_ViridianForest_Text_8422AC3::
	.string "I lost some of my allowance…$"

Kanto_ViridianForest_Text_8422AE0::
	.string "Did you know that POKéMON evolve?$"

Kanto_ViridianForest_Text_8422B02::
	.string "Oh!\n"
	.string "I lost!$"

Kanto_ViridianForest_Text_8422B0E::
	.string "BUG POKéMON evolve quickly.\n"
	.string "They’re a lot of fun!$"

Kanto_ViridianForest_Text_8422B40::
	.string "I was throwing POKé BALLS to\n"
	.string "catch POKéMON, and I ran out.\p"
	.string "That’s why you can never have too\n"
	.string "many POKé BALLS.$"

Kanto_ViridianForest_Text_8422BAE::
	.string "TRAINER TIPS\p"
	.string "If your POKéMON are weak and you\n"
	.string "want to avoid battles, stay away\l"
	.string "from grassy areas!$"

Kanto_ViridianForest_Text_8422C10::
	.string "For poison, use ANTIDOTE!\n"
	.string "Get it at POKéMON MARTS!$"

Kanto_ViridianForest_Text_8422C43::
	.string "TRAINER TIPS\p"
	.string "Contact PROF. OAK via a PC to\n"
	.string "get your POKéDEX evaluated!$"

Kanto_ViridianForest_Text_8422C8A::
	.string "TRAINER TIPS\p"
	.string "You can’t catch a POKéMON that\n"
	.string "belongs to someone else.\p"
	.string "Throw POKé BALLS only at wild\n"
	.string "POKéMON to catch them!$"

Kanto_ViridianForest_Text_8422D04::
	.string "TRAINER TIPS\p"
	.string "Weaken POKéMON before attempting\n"
	.string "capture!\p"
	.string "When healthy, they may escape!$"

Kanto_ViridianForest_Text_8422D5A::
	.string "LEAVING VIRIDIAN FOREST\n"
	.string "PEWTER CITY AHEAD$"
