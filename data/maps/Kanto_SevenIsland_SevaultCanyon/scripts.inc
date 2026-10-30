Kanto_SevenIsland_SevaultCanyon_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_SevenIsland_SevaultCanyon sign / bg event at 16,71
Kanto_SevenIsland_SevaultCanyon_EventScript_8418A07::
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843D243, MSGBOX_SIGN
	end

@ Kanto_SevenIsland_SevaultCanyon object 1 (local id 1, gfx 0x3EE7) at 13,43
Kanto_SevenIsland_SevaultCanyon_EventScript_845CE1E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8783, 0, Kanto_SevenIsland_SevaultCanyon_Text_843CB9F, Kanto_SevenIsland_SevaultCanyon_Text_843CBBE
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_EventScript_845CE45
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CBDA, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_EventScript_845CE45::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8783, 0, Kanto_SevenIsland_SevaultCanyon_Text_8471937, Kanto_SevenIsland_SevaultCanyon_Text_843CBBE
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CBDA, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon object 2 (local id 2, gfx 0x3F1E) at 13,36
Kanto_SevenIsland_SevaultCanyon_EventScript_845CE5C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8785, 0, Kanto_SevenIsland_SevaultCanyon_Text_843CC15, Kanto_SevenIsland_SevaultCanyon_Text_843CC98
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_EventScript_845CE83
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CCB1, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_EventScript_845CE83::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8785, 0, Kanto_SevenIsland_SevaultCanyon_Text_8471971, Kanto_SevenIsland_SevaultCanyon_Text_843CC98
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CCB1, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon object 3 (local id 3, gfx 0x3EF3) at 3,35
Kanto_SevenIsland_SevaultCanyon_EventScript_845CE9A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8788, 0, Kanto_SevenIsland_SevaultCanyon_Text_843CD47, Kanto_SevenIsland_SevaultCanyon_Text_843CD87
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_EventScript_845CEC1
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CDB4, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_EventScript_845CEC1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8788, 0, Kanto_SevenIsland_SevaultCanyon_Text_84719F4, Kanto_SevenIsland_SevaultCanyon_Text_843CD87
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CDB4, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon object 4 (local id 4, gfx 0x3F31) at 3,36
Kanto_SevenIsland_SevaultCanyon_EventScript_845CED8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8790, 0, Kanto_SevenIsland_SevaultCanyon_Text_843CE2E, Kanto_SevenIsland_SevaultCanyon_Text_843CE56
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_EventScript_845CEFF
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CE84, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_EventScript_845CEFF::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8790, 0, Kanto_SevenIsland_SevaultCanyon_Text_8471A34, Kanto_SevenIsland_SevaultCanyon_Text_843CE56
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CE84, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon object 5 (local id 5, gfx 0x3EFD) at 7,56
Kanto_SevenIsland_SevaultCanyon_EventScript_845CF16::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8791, 0, Kanto_SevenIsland_SevaultCanyon_Text_843CECC, Kanto_SevenIsland_SevaultCanyon_Text_843CEF8
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_EventScript_845CF3D
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CF21, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_EventScript_845CF3D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8791, 0, Kanto_SevenIsland_SevaultCanyon_Text_8471A68, Kanto_SevenIsland_SevaultCanyon_Text_843CEF8
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843CF21, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon object 6 (local id 6, gfx 0x3EFC) at 11,63
Kanto_SevenIsland_SevaultCanyon_EventScript_845CF54::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8792, 0, Kanto_SevenIsland_SevaultCanyon_Text_843CF67, Kanto_SevenIsland_SevaultCanyon_Text_843CFC6
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_EventScript_845CF7B
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843D002, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_EventScript_845CF7B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8792, 0, Kanto_SevenIsland_SevaultCanyon_Text_8471AA1, Kanto_SevenIsland_SevaultCanyon_Text_843CFC6
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843D002, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon object 7 (local id 7, gfx 0x3EFD) at 14,13
Kanto_SevenIsland_SevaultCanyon_EventScript_845CF92::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8793, 0, Kanto_SevenIsland_SevaultCanyon_Text_843D061, Kanto_SevenIsland_SevaultCanyon_Text_843D08D, Kanto_SevenIsland_SevaultCanyon_Text_843D0FA
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_EventScript_845CFBD
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843D0B4, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_EventScript_845CFBD::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8793, 0, Kanto_SevenIsland_SevaultCanyon_Text_8471B08, Kanto_SevenIsland_SevaultCanyon_Text_843D08D, Kanto_SevenIsland_SevaultCanyon_Text_843D0FA
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843D0B4, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon object 8 (local id 8, gfx 0x3EFC) at 14,14
Kanto_SevenIsland_SevaultCanyon_EventScript_845CFD8::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8793, 0, Kanto_SevenIsland_SevaultCanyon_Text_843D138, Kanto_SevenIsland_SevaultCanyon_Text_843D172, Kanto_SevenIsland_SevaultCanyon_Text_843D20C
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_SevenIsland_SevaultCanyon_EventScript_845D003
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843D190, MSGBOX_AUTOCLOSE
	end

Kanto_SevenIsland_SevaultCanyon_EventScript_845D003::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8793, 0, Kanto_SevenIsland_SevaultCanyon_Text_8471B3F, Kanto_SevenIsland_SevaultCanyon_Text_843D172, Kanto_SevenIsland_SevaultCanyon_Text_843D20C
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843D190, MSGBOX_AUTOCLOSE
	end

@ Kanto_SevenIsland_SevaultCanyon object 20 (local id 20, gfx 0x3EEB) at 8,26
Kanto_SevenIsland_SevaultCanyon_EventScript_845D366::
	lock
	faceplayer
	setvar VAR_0x8004, 0xA
	setvar VAR_0x8005, 5
	special QzQuestLogNop
	msgbox Kanto_SevenIsland_SevaultCanyon_Text_843D265, MSGBOX_DEFAULT
	release
	end

@ Kanto_SevenIsland_SevaultCanyon object 17 (local id 17, gfx 0x3F0E) at 18,45
Kanto_SevenIsland_SevaultCanyon_EventScript_846C1C4::
	setorcopyvar VAR_0x8000, ITEM_KINGS_ROCK
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SevenIsland_SevaultCanyon object 18 (local id 18, gfx 0x3F0E) at 7,38
Kanto_SevenIsland_SevaultCanyon_EventScript_846C1D1::
	setorcopyvar VAR_0x8000, ITEM_MAX_ELIXIR
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SevenIsland_SevaultCanyon object 19 (local id 19, gfx 0x3F0E) at 17,23
Kanto_SevenIsland_SevaultCanyon_EventScript_846C1DE::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SevenIsland_SevaultCanyon_Text_843CB9F::
	.string "My conditioning’s in top form!$"

Kanto_SevenIsland_SevaultCanyon_Text_843CBBE::
	.string "Huh?\n"
	.string "Something’s not right…$"

Kanto_SevenIsland_SevaultCanyon_Text_843CBDA::
	.string "The way I am now, I may have to\n"
	.string "hold off on TRAINER TOWER…$"

Kanto_SevenIsland_SevaultCanyon_Text_843CC15::
	.string "It doesn’t matter if you happen to\n"
	.string "have the strongest POKéMON…\p"
	.string "It doesn’t mean a thing if you don’t\n"
	.string "know how to use them properly!$"

Kanto_SevenIsland_SevaultCanyon_Text_843CC98::
	.string "You’re a master at this.$"

Kanto_SevenIsland_SevaultCanyon_Text_843CCB1::
	.string "You see them every so often,\n"
	.string "TRAINERS with awesome POKéMON\l"
	.string "who have no idea what to do.\p"
	.string "Before training POKéMON, TRAINERS\n"
	.string "themselves need to grow up.$"

Kanto_SevenIsland_SevaultCanyon_Text_843CD47::
	.string "I work to protect the environment.\n"
	.string "In turn, nature protects me!$"

Kanto_SevenIsland_SevaultCanyon_Text_843CD87::
	.string "Whoa! You’re tough!\n"
	.string "How about protecting me?$"

Kanto_SevenIsland_SevaultCanyon_Text_843CDB4::
	.string "From far out among the stars, our\n"
	.string "planet is just a drop of water.\p"
	.string "But we want to protect that drop\n"
	.string "of water for everyone.$"

Kanto_SevenIsland_SevaultCanyon_Text_843CE2E::
	.string "Those shoes you have are pretty\n"
	.string "snazzy.$"

Kanto_SevenIsland_SevaultCanyon_Text_843CE56::
	.string "You’re hot stuff in spite of your\n"
	.string "cute looks.$"

Kanto_SevenIsland_SevaultCanyon_Text_843CE84::
	.string "Your mom bought those snazzy\n"
	.string "RUNNING SHOES for you?\p"
	.string "She must adore you.$"

Kanto_SevenIsland_SevaultCanyon_Text_843CECC::
	.string "You look strong.\n"
	.string "Let me battle you, please!$"

Kanto_SevenIsland_SevaultCanyon_Text_843CEF8::
	.string "I knew it!\n"
	.string "You’re as tough as I thought!$"

Kanto_SevenIsland_SevaultCanyon_Text_843CF21::
	.string "The way you battle has given me\n"
	.string "a lot of useful data.\p"
	.string "Thanks so much!$"

Kanto_SevenIsland_SevaultCanyon_Text_843CF67::
	.string "I was given the best possible\n"
	.string "education to become this strong.\p"
	.string "I don’t want to lose to anyone!$"

Kanto_SevenIsland_SevaultCanyon_Text_843CFC6::
	.string "Thank you. I feel like I’ve been\n"
	.string "set free by losing to you.$"

Kanto_SevenIsland_SevaultCanyon_Text_843D002::
	.string "For some reason, I get furious\n"
	.string "over the slightest thing.\p"
	.string "I’ll calm myself by breathing\n"
	.string "deeply.$"

Kanto_SevenIsland_SevaultCanyon_Text_843D061::
	.string "LEX: My darling NYA, together\n"
	.string "we could win!$"

Kanto_SevenIsland_SevaultCanyon_Text_843D08D::
	.string "LEX: I seem to have been too\n"
	.string "careless…$"

Kanto_SevenIsland_SevaultCanyon_Text_843D0B4::
	.string "LEX: I think it was a good\n"
	.string "experience for NYA.\p"
	.string "You have my gratitude.$"

Kanto_SevenIsland_SevaultCanyon_Text_843D0FA::
	.string "LEX: May I ask you to battle the\n"
	.string "both of us at the same time?$"

Kanto_SevenIsland_SevaultCanyon_Text_843D138::
	.string "NYA: I’ll do my best so I won’t\n"
	.string "drag down my mentor, LEX!$"

Kanto_SevenIsland_SevaultCanyon_Text_843D172::
	.string "NYA: Oh, no…\n"
	.string "I’m so sorry, I…$"

Kanto_SevenIsland_SevaultCanyon_Text_843D190::
	.string "NYA: I think I’m going to need\n"
	.string "much more seasoning and tutoring\l"
	.string "under LEX…\p"
	.string "…So, LEX, please, may I stay with\n"
	.string "you like this?$"

Kanto_SevenIsland_SevaultCanyon_Text_843D20C::
	.string "NYA: I’m sorry, but may we battle\n"
	.string "in two-on-two style?$"

Kanto_SevenIsland_SevaultCanyon_Text_843D243::
	.string "SEVAULT CANYON\n"
	.string "TANOBY RUINS AHEAD$"

Kanto_SevenIsland_SevaultCanyon_Text_843D265::
	.string "Training by oneself is certainly\n"
	.string "not a bad thing.\p"
	.string "But it’s also not bad to aim for\n"
	.string "the top with a partner.\p"
	.string "Even BRUNO…\n"
	.string "He trained with a fellow by the\l"
	.string "name of BRAWLY before.$"

Kanto_SevenIsland_SevaultCanyon_Text_8471937::
	.string "Let’s have another battle.\n"
	.string "My conditioning’s in top form!$"

Kanto_SevenIsland_SevaultCanyon_Text_8471971::
	.string "It doesn’t matter if you happen to\n"
	.string "have the strongest POKéMON…\p"
	.string "It doesn’t mean a thing if you don’t\n"
	.string "know how to use them properly!$"

Kanto_SevenIsland_SevaultCanyon_Text_84719F4::
	.string "I work to protect the environment.\n"
	.string "In turn, nature protects me!$"

Kanto_SevenIsland_SevaultCanyon_Text_8471A34::
	.string "Every time we meet, you have those\n"
	.string "snazzy shoes on.$"

Kanto_SevenIsland_SevaultCanyon_Text_8471A68::
	.string "You must have gotten tougher.\n"
	.string "Let me battle you, please!$"

Kanto_SevenIsland_SevaultCanyon_Text_8471AA1::
	.string "I was given the best possible\n"
	.string "education to become this strong.\p"
	.string "I won’t lose this time.\n"
	.string "Absolutely not!$"

Kanto_SevenIsland_SevaultCanyon_Text_8471B08::
	.string "LEX: My darling NYA, together\n"
	.string "we will win for certain!$"

Kanto_SevenIsland_SevaultCanyon_Text_8471B3F::
	.string "NYA: I won’t drag down my mentor\n"
	.string "LEX! We’re going to win!$"
