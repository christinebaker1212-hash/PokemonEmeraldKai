Kanto_Route25_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route25 sign / bg event at 48,4
Kanto_Route25_EventScript_841842B::
	msgbox Kanto_Route25_Text_8439835, MSGBOX_SIGN
	end

@ Kanto_Route25 object 13 (local id 13, gfx 0x3F1E) at 49,11
Kanto_Route25_EventScript_8418434::
	msgbox Kanto_Route25_Text_84398AB, MSGBOX_NPC
	end

@ Kanto_Route25 object 2 (local id 2, gfx 0x3F5C) at 18,2
Kanto_Route25_EventScript_8459B3C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8285, 0, Kanto_Route25_Text_84393A9, Kanto_Route25_Text_84393CF
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459B63
	msgbox Kanto_Route25_Text_84393DE, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459B63::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8285, 0, Kanto_Route25_Text_847088D, Kanto_Route25_Text_84393CF
	msgbox Kanto_Route25_Text_84393DE, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 4 (local id 4, gfx 0x3F5C) at 22,4
Kanto_Route25_EventScript_8459B7A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8286, 0, Kanto_Route25_Text_843944E, Kanto_Route25_Text_843948F
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459BA1
	msgbox Kanto_Route25_Text_843949C, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459BA1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8286, 0, Kanto_Route25_Text_84708AA, Kanto_Route25_Text_843948F
	msgbox Kanto_Route25_Text_843949C, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 8 (local id 8, gfx 0x3F5C) at 36,4
Kanto_Route25_EventScript_8459BB8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8287, 0, Kanto_Route25_Text_8439592, Kanto_Route25_Text_84395C2
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459BDF
	msgbox Kanto_Route25_Text_84395D8, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459BDF::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8287, 0, Kanto_Route25_Text_847097A, Kanto_Route25_Text_84395C2
	msgbox Kanto_Route25_Text_84395D8, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 5 (local id 5, gfx 0x3F31) at 22,8
Kanto_Route25_EventScript_8459BF6::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8345, 0, Kanto_Route25_Text_8439530, Kanto_Route25_Text_843954A
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459C1D
	msgbox Kanto_Route25_Text_843956A, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459C1D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8345, 0, Kanto_Route25_Text_8470942, Kanto_Route25_Text_843954A
	msgbox Kanto_Route25_Text_843956A, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 9 (local id 9, gfx 0x3F16) at 42,5
Kanto_Route25_EventScript_8459C34::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8317, 0, Kanto_Route25_Text_843961C, Kanto_Route25_Text_843964D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459C5B
	msgbox Kanto_Route25_Text_8439661, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459C5B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8317, 0, Kanto_Route25_Text_84709B0, Kanto_Route25_Text_843964D
	msgbox Kanto_Route25_Text_8439661, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 1 (local id 1, gfx 0x3F0C) at 11,4
Kanto_Route25_EventScript_8459C72::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8374, 0, Kanto_Route25_Text_8439690, Kanto_Route25_Text_84396D3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459C99
	msgbox Kanto_Route25_Text_84396E4, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459C99::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8374, 0, Kanto_Route25_Text_84709E1, Kanto_Route25_Text_84396D3
	msgbox Kanto_Route25_Text_84396E4, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 6 (local id 6, gfx 0x3F0C) at 27,9
Kanto_Route25_EventScript_8459CB0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8375, 0, Kanto_Route25_Text_843970C, Kanto_Route25_Text_8439742
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459CD7
	msgbox Kanto_Route25_Text_8439767, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459CD7::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8375, 0, Kanto_Route25_Text_8470A24, Kanto_Route25_Text_8439742
	msgbox Kanto_Route25_Text_8439767, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 3 (local id 3, gfx 0x3F0C) at 17,7
Kanto_Route25_EventScript_8459CEE::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8376, 0, Kanto_Route25_Text_84397C7, Kanto_Route25_Text_84397F3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459D15
	msgbox Kanto_Route25_Text_8439805, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459D15::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8376, 0, Kanto_Route25_Text_8470A62, Kanto_Route25_Text_84397F3
	msgbox Kanto_Route25_Text_8439805, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 7 (local id 7, gfx 0x3EF3) at 28,4
Kanto_Route25_EventScript_8459D2C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8663, 0, Kanto_Route25_Text_84394D4, Kanto_Route25_Text_84394FB
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route25_EventScript_8459D53
	msgbox Kanto_Route25_Text_8439506, MSGBOX_AUTOCLOSE
	end

Kanto_Route25_EventScript_8459D53::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8663, 0, Kanto_Route25_Text_84708EB, Kanto_Route25_Text_84394FB
	msgbox Kanto_Route25_Text_8439506, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route25 object 12 (local id 12, gfx 0x3EE8) at 49,10
Kanto_Route25_EventScript_845D220::
	lock
	faceplayer
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 4
	special QzQuestLogNop_737
	msgbox Kanto_Route25_Text_8439852, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route25 object 10 (local id 10, gfx 0x3F0E) at 26,2
Kanto_Route25_EventScript_846BB07::
	setorcopyvar VAR_0x8000, 0x1A9
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_Route25_Text_84393A9::
	.string "Local TRAINERS come here to\n"
	.string "practice.$"

Kanto_Route25_Text_84393CF::
	.string "You’re decent.$"

Kanto_Route25_Text_84393DE::
	.string "All POKéMON have weaknesses.\n"
	.string "Even the strongest ones.\p"
	.string "That’s why it’s best to raise\n"
	.string "POKéMON of different types.$"

Kanto_Route25_Text_843944E::
	.string "Dad took me to a great party on\n"
	.string "the S.S. ANNE at VERMILION CITY.$"

Kanto_Route25_Text_843948F::
	.string "I’m not mad!$"

Kanto_Route25_Text_843949C::
	.string "On the S.S. ANNE, I saw TRAINERS\n"
	.string "from around the world.$"

Kanto_Route25_Text_84394D4::
	.string "I’m a cool guy.\n"
	.string "I’ve got a girlfriend!$"

Kanto_Route25_Text_84394FB::
	.string "Aww, darn…$"

Kanto_Route25_Text_8439506::
	.string "Oh, well.\n"
	.string "My girlfriend will cheer me up.$"

Kanto_Route25_Text_8439530::
	.string "Hi!\n"
	.string "My boyfriend is cool!$"

Kanto_Route25_Text_843954A::
	.string "My conditioning isn’t the best…$"

Kanto_Route25_Text_843956A::
	.string "I wish my boyfriend was as good\n"
	.string "as you.$"

Kanto_Route25_Text_8439592::
	.string "I had this feeling…\n"
	.string "I knew I had to battle you!$"

Kanto_Route25_Text_84395C2::
	.string "I knew I’d lose, too!$"

Kanto_Route25_Text_84395D8::
	.string "If your POKéMON gets confused,\n"
	.string "switch it out.\p"
	.string "That’s a good tactic.$"

Kanto_Route25_Text_843961C::
	.string "My friend has many cute POKéMON.\n"
	.string "I’m so jealous!$"

Kanto_Route25_Text_843964D::
	.string "I’m not so jealous!$"

Kanto_Route25_Text_8439661::
	.string "You came from MT. MOON?\n"
	.string "May I have a CLEFAIRY?$"

Kanto_Route25_Text_8439690::
	.string "I just got down from MT. MOON,\n"
	.string "but I’ve still got gas in the tank!$"

Kanto_Route25_Text_84396D3::
	.string "You worked hard!$"

Kanto_Route25_Text_84396E4::
	.string "Drat!\n"
	.string "A ZUBAT bit me back in that cave.$"

Kanto_Route25_Text_843970C::
	.string "I’m off to see a POKéMANIAC’s\n"
	.string "collection at the cape.$"

Kanto_Route25_Text_8439742::
	.string "You done got me, and real good,\n"
	.string "too!$"

Kanto_Route25_Text_8439767::
	.string "The POKéMANIAC sure lives up to\n"
	.string "his name.\p"
	.string "His collection includes many rare\n"
	.string "species of POKéMON.$"

Kanto_Route25_Text_84397C7::
	.string "You’re going to see BILL?\n"
	.string "First, we battle!$"

Kanto_Route25_Text_84397F3::
	.string "You’re something.$"

Kanto_Route25_Text_8439805::
	.string "The trail below is a shortcut to\n"
	.string "CERULEAN CITY.$"

Kanto_Route25_Text_8439835::
	.string "SEA COTTAGE\n"
	.string "BILL lives here!$"

Kanto_Route25_Text_8439852::
	.string "This cape is a famous date spot.\p"
	.string "MISTY, the GYM LEADER, has high\n"
	.string "hopes about this place.$"

Kanto_Route25_Text_84398AB::
	.string "Hello, are you here alone?\p"
	.string "If you’re out at CERULEAN’s cape…\n"
	.string "Well, it should be as a couple.$"

Kanto_Route25_Text_847088D::
	.string "I won’t lose while I’m here!$"

Kanto_Route25_Text_84708AA::
	.string "Dad took me to a great party on\n"
	.string "the S.S. ANNE at VERMILION CITY.$"

Kanto_Route25_Text_84708EB::
	.string "I’m a cool guy.\n"
	.string "I’ve got a girlfriend!\p"
	.string "I’ll show her how cool I am for\n"
	.string "sure this time!$"

Kanto_Route25_Text_8470942::
	.string "Hi!\n"
	.string "My boyfriend is cool!\l"
	.string "My conditioning’s good today!$"

Kanto_Route25_Text_847097A::
	.string "I had this feeling…\n"
	.string "I knew I had to battle you again!$"

Kanto_Route25_Text_84709B0::
	.string "My friend has many cute POKéMON.\n"
	.string "I’m so jealous!$"

Kanto_Route25_Text_84709E1::
	.string "I just trained up on MT. MOON,\n"
	.string "but I’ve still got gas in the tank!$"

Kanto_Route25_Text_8470A24::
	.string "A POKéMANIAC lives on the cape.\n"
	.string "Have you seen his collection?$"

Kanto_Route25_Text_8470A62::
	.string "You’re going to see BILL again?\n"
	.string "First, we battle!$"
