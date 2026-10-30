Kanto_OneIsland_KindleRoad_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_OneIsland_KindleRoad sign / bg event at 8,115
Kanto_OneIsland_KindleRoad_EventScript_8418443::
	msgbox Kanto_OneIsland_KindleRoad_Text_8439FEB, MSGBOX_SIGN
	end

@ Kanto_OneIsland_KindleRoad sign / bg event at 13,59
Kanto_OneIsland_KindleRoad_EventScript_841844C::
	msgbox Kanto_OneIsland_KindleRoad_Text_843A01B, MSGBOX_SIGN
	end

@ Kanto_OneIsland_KindleRoad object 1 (local id 1, gfx 0x3F4D) at 11,32
Kanto_OneIsland_KindleRoad_EventScript_845BFC2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8739, 0, Kanto_OneIsland_KindleRoad_Text_8439908, Kanto_OneIsland_KindleRoad_Text_8439936
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845BFE9
	msgbox Kanto_OneIsland_KindleRoad_Text_8439940, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845BFE9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8739, 0, Kanto_OneIsland_KindleRoad_Text_8470C5B, Kanto_OneIsland_KindleRoad_Text_8439936
	msgbox Kanto_OneIsland_KindleRoad_Text_8439940, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 2 (local id 2, gfx 0x3F4C) at 17,132
Kanto_OneIsland_KindleRoad_EventScript_845C000::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8740, 0, Kanto_OneIsland_KindleRoad_Text_8439975, Kanto_OneIsland_KindleRoad_Text_8439995
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C027
	msgbox Kanto_OneIsland_KindleRoad_Text_84399AA, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C027::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8740, 0, Kanto_OneIsland_KindleRoad_Text_8470C8D, Kanto_OneIsland_KindleRoad_Text_8439995
	msgbox Kanto_OneIsland_KindleRoad_Text_84399AA, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 3 (local id 3, gfx 0x3F4F) at 7,39
Kanto_OneIsland_KindleRoad_EventScript_845C03E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8741, 0, Kanto_OneIsland_KindleRoad_Text_84399E2, Kanto_OneIsland_KindleRoad_Text_8439A11
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C065
	msgbox Kanto_OneIsland_KindleRoad_Text_8439A29, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C065::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8741, 0, Kanto_OneIsland_KindleRoad_Text_8470CAD, Kanto_OneIsland_KindleRoad_Text_8439A11
	msgbox Kanto_OneIsland_KindleRoad_Text_8439A29, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 4 (local id 4, gfx 0x3F4E) at 19,79
Kanto_OneIsland_KindleRoad_EventScript_845C07C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8742, 0, Kanto_OneIsland_KindleRoad_Text_8439A50, Kanto_OneIsland_KindleRoad_Text_8439A8D
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C0A3
	msgbox Kanto_OneIsland_KindleRoad_Text_8439AAE, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C0A3::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8742, 0, Kanto_OneIsland_KindleRoad_Text_8470CDB, Kanto_OneIsland_KindleRoad_Text_8439A8D
	msgbox Kanto_OneIsland_KindleRoad_Text_8439AAE, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 5 (local id 5, gfx 0x3F06) at 15,25
Kanto_OneIsland_KindleRoad_EventScript_845C0BA::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8743, 0, Kanto_OneIsland_KindleRoad_Text_8439AE8, Kanto_OneIsland_KindleRoad_Text_8439B1C
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C0E1
	msgbox Kanto_OneIsland_KindleRoad_Text_8439B34, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C0E1::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8743, 0, Kanto_OneIsland_KindleRoad_Text_8470D10, Kanto_OneIsland_KindleRoad_Text_8439B1C
	msgbox Kanto_OneIsland_KindleRoad_Text_8439B34, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 6 (local id 6, gfx 0x3EE7) at 9,53
Kanto_OneIsland_KindleRoad_EventScript_845C0F8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8710, 0, Kanto_OneIsland_KindleRoad_Text_8439B6C, Kanto_OneIsland_KindleRoad_Text_8439B94
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C11F
	msgbox Kanto_OneIsland_KindleRoad_Text_8439BAD, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C11F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8710, 0, Kanto_OneIsland_KindleRoad_Text_8470D45, Kanto_OneIsland_KindleRoad_Text_8439B94
	msgbox Kanto_OneIsland_KindleRoad_Text_8439BAD, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 7 (local id 7, gfx 0x3EE7) at 18,105
Kanto_OneIsland_KindleRoad_EventScript_845C136::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8744, 0, Kanto_OneIsland_KindleRoad_Text_8439BD9, Kanto_OneIsland_KindleRoad_Text_8439C04
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C15D
	msgbox Kanto_OneIsland_KindleRoad_Text_8439C15, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C15D::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8744, 0, Kanto_OneIsland_KindleRoad_Text_8470D70, Kanto_OneIsland_KindleRoad_Text_8439C04
	msgbox Kanto_OneIsland_KindleRoad_Text_8439C15, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 8 (local id 8, gfx 0x3EEB) at 16,64
Kanto_OneIsland_KindleRoad_EventScript_845C174::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8745, 0, Kanto_OneIsland_KindleRoad_Text_8439C39, Kanto_OneIsland_KindleRoad_Text_8439C75
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C19B
	msgbox Kanto_OneIsland_KindleRoad_Text_8439C81, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C19B::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8745, 0, Kanto_OneIsland_KindleRoad_Text_8470DA0, Kanto_OneIsland_KindleRoad_Text_8439C75
	msgbox Kanto_OneIsland_KindleRoad_Text_8439C81, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 9 (local id 9, gfx 0x3EEB) at 11,64
Kanto_OneIsland_KindleRoad_EventScript_845C1B2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8746, 0, Kanto_OneIsland_KindleRoad_Text_8439CB7, Kanto_OneIsland_KindleRoad_Text_8439CF3
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C1D9
	msgbox Kanto_OneIsland_KindleRoad_Text_8439CFC, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C1D9::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8746, 0, Kanto_OneIsland_KindleRoad_Text_8470DE9, Kanto_OneIsland_KindleRoad_Text_8439CF3
	msgbox Kanto_OneIsland_KindleRoad_Text_8439CFC, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 10 (local id 10, gfx 0x3EF3) at 11,89
Kanto_OneIsland_KindleRoad_EventScript_845C1F0::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8747, 0, Kanto_OneIsland_KindleRoad_Text_8439D3C, Kanto_OneIsland_KindleRoad_Text_8439D7B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C217
	msgbox Kanto_OneIsland_KindleRoad_Text_8439D88, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C217::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8747, 0, Kanto_OneIsland_KindleRoad_Text_8470E2D, Kanto_OneIsland_KindleRoad_Text_8439D7B
	msgbox Kanto_OneIsland_KindleRoad_Text_8439D88, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 11 (local id 11, gfx 0x3F31) at 9,108
Kanto_OneIsland_KindleRoad_EventScript_845C22E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8748, 0, Kanto_OneIsland_KindleRoad_Text_8439DB4, Kanto_OneIsland_KindleRoad_Text_8439DF4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C255
	msgbox Kanto_OneIsland_KindleRoad_Text_8439E0D, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C255::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8748, 0, Kanto_OneIsland_KindleRoad_Text_8470E6C, Kanto_OneIsland_KindleRoad_Text_8439DF4
	msgbox Kanto_OneIsland_KindleRoad_Text_8439E0D, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 13 (local id 13, gfx 0x3EE7) at 9,68
Kanto_OneIsland_KindleRoad_EventScript_845C26C::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8749, 0, Kanto_OneIsland_KindleRoad_Text_8439E36, Kanto_OneIsland_KindleRoad_Text_8439E6D, Kanto_OneIsland_KindleRoad_Text_8439EC7
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C297
	msgbox Kanto_OneIsland_KindleRoad_Text_8439E8C, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C297::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8749, 0, Kanto_OneIsland_KindleRoad_Text_8470EAB, Kanto_OneIsland_KindleRoad_Text_8439E6D, Kanto_OneIsland_KindleRoad_Text_8439EC7
	msgbox Kanto_OneIsland_KindleRoad_Text_8439E8C, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 12 (local id 12, gfx 0x3EEB) at 8,68
Kanto_OneIsland_KindleRoad_EventScript_845C2B2::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8749, 0, Kanto_OneIsland_KindleRoad_Text_8439F1C, Kanto_OneIsland_KindleRoad_Text_8439F4B, Kanto_OneIsland_KindleRoad_Text_8439F94
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_OneIsland_KindleRoad_EventScript_845C2DD
	msgbox Kanto_OneIsland_KindleRoad_Text_8439F67, MSGBOX_AUTOCLOSE
	end

Kanto_OneIsland_KindleRoad_EventScript_845C2DD::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8749, 0, Kanto_OneIsland_KindleRoad_Text_8470EFB, Kanto_OneIsland_KindleRoad_Text_8439F4B, Kanto_OneIsland_KindleRoad_Text_8439F94
	msgbox Kanto_OneIsland_KindleRoad_Text_8439F67, MSGBOX_AUTOCLOSE
	end

@ Kanto_OneIsland_KindleRoad object 27 (local id 27, gfx 0x3F0E) at 15,70
Kanto_OneIsland_KindleRoad_EventScript_846C128::
	setorcopyvar VAR_0x8000, ITEM_ETHER
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_OneIsland_KindleRoad object 28 (local id 28, gfx 0x3F0E) at 17,103
Kanto_OneIsland_KindleRoad_EventScript_846C135::
	setorcopyvar VAR_0x8000, ITEM_MAX_REPEL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_OneIsland_KindleRoad object 29 (local id 29, gfx 0x3F0E) at 18,114
Kanto_OneIsland_KindleRoad_EventScript_846C142::
	setorcopyvar VAR_0x8000, ITEM_CARBOS
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_OneIsland_KindleRoad_Text_8439908::
	.string "The weather’s gorgeous!\n"
	.string "Perfect for a battle!$"

Kanto_OneIsland_KindleRoad_Text_8439936::
	.string "Aiyeeeeh!$"

Kanto_OneIsland_KindleRoad_Text_8439940::
	.string "Please don’t splash my face!\n"
	.string "You’ll ruin the makeup.$"

Kanto_OneIsland_KindleRoad_Text_8439975::
	.string "I’m beginning to get a sunburn…$"

Kanto_OneIsland_KindleRoad_Text_8439995::
	.string "Oh, you’re so awful.$"

Kanto_OneIsland_KindleRoad_Text_84399AA::
	.string "I like that cap you have.\n"
	.string "Maybe I should wear one, too…$"

Kanto_OneIsland_KindleRoad_Text_84399E2::
	.string "Seen from here, MT. EMBER towers\n"
	.string "into the sky.$"

Kanto_OneIsland_KindleRoad_Text_8439A11::
	.string "Well, aren’t you tough?$"

Kanto_OneIsland_KindleRoad_Text_8439A29::
	.string "The sky is immense around these\n"
	.string "parts.$"

Kanto_OneIsland_KindleRoad_Text_8439A50::
	.string "I took a break from swimming,\n"
	.string "only to have you challenge me?$"

Kanto_OneIsland_KindleRoad_Text_8439A8D::
	.string "Hey, what the…\n"
	.string "You’re good, you!$"

Kanto_OneIsland_KindleRoad_Text_8439AAE::
	.string "Instead of using SURF all the time,\n"
	.string "you should swim some.$"

Kanto_OneIsland_KindleRoad_Text_8439AE8::
	.string "Wait! Wait a second!\n"
	.string "I think I’ve hooked a big one!$"

Kanto_OneIsland_KindleRoad_Text_8439B1C::
	.string "I’ve lost it all again…$"

Kanto_OneIsland_KindleRoad_Text_8439B34::
	.string "Not only did I lose, the big one\n"
	.string "got away from me, too!$"

Kanto_OneIsland_KindleRoad_Text_8439B6C::
	.string "Could you help me out with my\n"
	.string "training?$"

Kanto_OneIsland_KindleRoad_Text_8439B94::
	.string "You were a cut above me.$"

Kanto_OneIsland_KindleRoad_Text_8439BAD::
	.string "It’s clear that you’re skilled.\n"
	.string "I like you!$"

Kanto_OneIsland_KindleRoad_Text_8439BD9::
	.string "There isn’t a day in which we don’t\n"
	.string "train!$"

Kanto_OneIsland_KindleRoad_Text_8439C04::
	.string "I’m such a fool!$"

Kanto_OneIsland_KindleRoad_Text_8439C15::
	.string "I’ll train even harder from now on!$"

Kanto_OneIsland_KindleRoad_Text_8439C39::
	.string "Every morning, before breakfast,\n"
	.string "I swim around this island.$"

Kanto_OneIsland_KindleRoad_Text_8439C75::
	.string "Gasp…\n"
	.string "Gasp…$"

Kanto_OneIsland_KindleRoad_Text_8439C81::
	.string "I lost because I’m pooped from all\n"
	.string "the running I did…$"

Kanto_OneIsland_KindleRoad_Text_8439CB7::
	.string "Dress appropriately for battle!\n"
	.string "Lose that frivolous outfit!$"

Kanto_OneIsland_KindleRoad_Text_8439CF3::
	.string "Why me?!$"

Kanto_OneIsland_KindleRoad_Text_8439CFC::
	.string "Even martial artists are becoming\n"
	.string "fashion conscious these days…$"

Kanto_OneIsland_KindleRoad_Text_8439D3C::
	.string "You know, everything tastes great\n"
	.string "when you’re out in the wild.$"

Kanto_OneIsland_KindleRoad_Text_8439D7B::
	.string "Awww, blown!$"

Kanto_OneIsland_KindleRoad_Text_8439D88::
	.string "The water in volcanic regions is\n"
	.string "delicious.$"

Kanto_OneIsland_KindleRoad_Text_8439DB4::
	.string "My lunch filled us up, so will you\n"
	.string "battle us for some exercise?$"

Kanto_OneIsland_KindleRoad_Text_8439DF4::
	.string "That made me all sweaty.$"

Kanto_OneIsland_KindleRoad_Text_8439E0D::
	.string "What should I do?\n"
	.string "I feel famished again.$"

Kanto_OneIsland_KindleRoad_Text_8439E36::
	.string "KIA: My big brother and I make\n"
	.string "an awesome combination!$"

Kanto_OneIsland_KindleRoad_Text_8439E6D::
	.string "KIA: Huh?!\n"
	.string "I can’t believe it!$"

Kanto_OneIsland_KindleRoad_Text_8439E8C::
	.string "KIA: How could there be anyone\n"
	.string "better than my big brother?$"

Kanto_OneIsland_KindleRoad_Text_8439EC7::
	.string "KIA: If you want to battle with\n"
	.string "us, you have to bring at least two\l"
	.string "POKéMON with you.$"

Kanto_OneIsland_KindleRoad_Text_8439F1C::
	.string "MIK: Together with KIA, I’m afraid\n"
	.string "of nothing!$"

Kanto_OneIsland_KindleRoad_Text_8439F4B::
	.string "MIK: Whoa!\n"
	.string "That’s too much!$"

Kanto_OneIsland_KindleRoad_Text_8439F67::
	.string "MIK: How could my combination\n"
	.string "with KIA fail?$"

Kanto_OneIsland_KindleRoad_Text_8439F94::
	.string "MIK: Hey, do you want to battle\n"
	.string "with us two?\p"
	.string "Bring two or more POKéMON with\n"
	.string "you, then.$"

Kanto_OneIsland_KindleRoad_Text_8439FEB::
	.string "This is KINDLE ROAD.\p"
	.string "Go straight for MT. EMBER.$"

Kanto_OneIsland_KindleRoad_Text_843A01B::
	.string "Light the Fire in Your Heart!\n"
	.string "EMBER SPA$"

Kanto_OneIsland_KindleRoad_Text_8470C5B::
	.string "The weather’s gorgeous!\n"
	.string "I’ll try not to spoil it.$"

Kanto_OneIsland_KindleRoad_Text_8470C8D::
	.string "My sunburn is starting to hurt…$"

Kanto_OneIsland_KindleRoad_Text_8470CAD::
	.string "The fire bird’s mountain casts\n"
	.string "a huge shadow…$"

Kanto_OneIsland_KindleRoad_Text_8470CDB::
	.string "I have this strong feeling that\n"
	.string "I can win this time!$"

Kanto_OneIsland_KindleRoad_Text_8470D10::
	.string "Wait! Wait a second!\n"
	.string "I’m sure I’ve hooked a big one!$"

Kanto_OneIsland_KindleRoad_Text_8470D45::
	.string "You’ll help me out with my\n"
	.string "training again?$"

Kanto_OneIsland_KindleRoad_Text_8470D70::
	.string "We haven’t missed a single day of\n"
	.string "training yet!$"

Kanto_OneIsland_KindleRoad_Text_8470DA0::
	.string "Every morning, before breakfast,\n"
	.string "I swim around this island…\l"
	.string "Three times!$"

Kanto_OneIsland_KindleRoad_Text_8470DE9::
	.string "Dress appropriately for battle!\n"
	.string "Lose that frivolous outfit, I said!$"

Kanto_OneIsland_KindleRoad_Text_8470E2D::
	.string "You know, everything tastes great\n"
	.string "when you’re out in the wild.$"

Kanto_OneIsland_KindleRoad_Text_8470E6C::
	.string "I ate too much again, so will you\n"
	.string "battle us for some exercise?$"

Kanto_OneIsland_KindleRoad_Text_8470EAB::
	.string "KIA: My big brother and I make\n"
	.string "an awesome combination!\p"
	.string "We won’t lose this time!$"

Kanto_OneIsland_KindleRoad_Text_8470EFB::
	.string "MIK: Together with KIA, we’re\n"
	.string "afraid of nothing!\p"
	.string "We’ll prove it to you this time!$"
