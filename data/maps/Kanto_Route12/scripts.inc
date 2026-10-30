Kanto_Route12_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_Route12_EventScript_8417CDB
	.byte 0

Kanto_Route12_EventScript_8417CDB::
	call_if_set FLAG_QZ_0x2215, Kanto_Route12_EventScript_8417CE5
	end

Kanto_Route12_EventScript_8417CE5::
	removeobject VAR_LAST_TALKED
	return

@ Kanto_Route12 object 5 (local id 5, gfx 0x3F48) at 14,70
Kanto_Route12_EventScript_8417CE9::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x215D, Kanto_Route12_EventScript_8417D75
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	special QzQuestLogNop
	msgbox Quetzal_Text_8435693, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_EventScript_8417D69
	call Quetzal_EventScript_8459531
	setwildbattle SPECIES_SNORLAX, 0x1E, ITEM_NONE
	nop
	nop
	nop
	nop
	nop
	waitse
	playmoncry SPECIES_SNORLAX, 2
	delay 0x28
	waitmoncry
	setflag FLAG_QZ_0x21AF
	setflag FLAG_QZ_0x2215
	setflag FLAG_QZ_0x2232
	dowildbattle
	clearflag FLAG_QZ_0x2215
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_8417D6B
	goto_if_eq VAR_RESULT, 4, Kanto_Route12_EventScript_8417D6B
	goto_if_eq VAR_RESULT, 5, Kanto_Route12_EventScript_8417D6B
	release
	end

Kanto_Route12_EventScript_8417D69::
	release
	end

Kanto_Route12_EventScript_8417D6B::
	msgbox Quetzal_Text_843564A, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_EventScript_8417D75::
	msgbox Kanto_Route12_Text_84355DE, MSGBOX_DEFAULT
	release
	end

@ Kanto_Route12 sign / bg event at 17,13
Kanto_Route12_EventScript_8417D7F::
	msgbox Kanto_Route12_Text_8435A5E, MSGBOX_SIGN
	end

@ Kanto_Route12 sign / bg event at 15,69
Kanto_Route12_EventScript_8417D88::
	msgbox Kanto_Route12_Text_8435A7A, MSGBOX_SIGN
	end

@ Kanto_Route12 object 1 (local id 1, gfx 0x3F06) at 17,32
Kanto_Route12_EventScript_845A822::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8417, 0, Kanto_Route12_Text_84356C9, Kanto_Route12_Text_84356E2
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845A849
	msgbox Kanto_Route12_Text_84356F9, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845A849::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8417, 0, Kanto_Route12_Text_846F443, Kanto_Route12_Text_84356E2
	msgbox Kanto_Route12_Text_84356F9, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 2 (local id 2, gfx 0x3F06) at 9,40
Kanto_Route12_EventScript_845A860::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8418, 0, Kanto_Route12_Text_8435715, Kanto_Route12_Text_843573C
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845A887
	msgbox Kanto_Route12_Text_843574F, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845A887::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8418, 0, Kanto_Route12_Text_846F47E, Kanto_Route12_Text_843573C
	msgbox Kanto_Route12_Text_843574F, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 3 (local id 3, gfx 0x3F06) at 16,47
Kanto_Route12_EventScript_845A89E::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8419, 0, Kanto_Route12_Text_84358A8, Kanto_Route12_Text_84358CD
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845A8C5
	msgbox Kanto_Route12_Text_84358DD, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845A8C5::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8419, 0, Kanto_Route12_Text_846F542, Kanto_Route12_Text_84358CD
	msgbox Kanto_Route12_Text_84358DD, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 4 (local id 4, gfx 0x3F06) at 13,59
Kanto_Route12_EventScript_845A8DC::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8420, 0, Kanto_Route12_Text_8435949, Kanto_Route12_Text_8435997
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845A903
	msgbox Kanto_Route12_Text_84359A6, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845A903::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8420, 0, Kanto_Route12_Text_846F57A, Kanto_Route12_Text_8435997
	msgbox Kanto_Route12_Text_84359A6, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 8 (local id 8, gfx 0x3F06) at 18,108
Kanto_Route12_EventScript_845A91A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8425, 0, Kanto_Route12_Text_84359D5, Kanto_Route12_Text_8435A0B
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845A941
	msgbox Kanto_Route12_Text_8435A14, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845A941::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8425, 0, Kanto_Route12_Text_846F5DA, Kanto_Route12_Text_8435A0B
	msgbox Kanto_Route12_Text_8435A14, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 6 (local id 6, gfx 0x3F3B) at 13,92
Kanto_Route12_EventScript_845A958::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8477, 0, Kanto_Route12_Text_843580E, Kanto_Route12_Text_8435862
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845A97F
	msgbox Kanto_Route12_Text_843586D, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845A97F::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8477, 0, Kanto_Route12_Text_846F4E1, Kanto_Route12_Text_8435862
	msgbox Kanto_Route12_Text_843586D, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 7 (local id 7, gfx 0x3EF3) at 10,99
Kanto_Route12_EventScript_845A996::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8669, 0, Kanto_Route12_Text_8435780, Kanto_Route12_Text_84357B4
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845A9BD
	msgbox Kanto_Route12_Text_84357B9, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845A9BD::
	trainerbattle TRAINER_BATTLE_REMATCH, TRAINER_QZ_8669, 0, Kanto_Route12_Text_846F4AE, Kanto_Route12_Text_84357B4
	msgbox Kanto_Route12_Text_84357B9, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 13 (local id 13, gfx 0x3F1E) at 19,63
Kanto_Route12_EventScript_845A9D4::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8678, 0, Kanto_Route12_Text_8435A8C, Kanto_Route12_Text_8435AB8, Kanto_Route12_Text_8435B09
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845A9FF
	msgbox Kanto_Route12_Text_8435AE6, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845A9FF::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8678, 0, Kanto_Route12_Text_846F611, Kanto_Route12_Text_8435AB8, Kanto_Route12_Text_8435B09
	msgbox Kanto_Route12_Text_8435AE6, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 12 (local id 12, gfx 0x3EE8) at 19,64
Kanto_Route12_EventScript_845AA1A::
	trainerbattle TRAINER_BATTLE_DOUBLE, TRAINER_QZ_8678, 0, Kanto_Route12_Text_8435B6D, Kanto_Route12_Text_8435B97, Kanto_Route12_Text_8435BCF
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, 1, Kanto_Route12_EventScript_845AA45
	msgbox Kanto_Route12_Text_8435BA9, MSGBOX_AUTOCLOSE
	end

Kanto_Route12_EventScript_845AA45::
	trainerbattle TRAINER_BATTLE_REMATCH_DOUBLE, TRAINER_QZ_8678, 0, Kanto_Route12_Text_846F641, Kanto_Route12_Text_8435B97, Kanto_Route12_Text_8435BCF
	msgbox Kanto_Route12_Text_8435BA9, MSGBOX_AUTOCLOSE
	end

@ Kanto_Route12 object 10 (local id 10, gfx 0x3F0E) at 18,36
Kanto_Route12_EventScript_846BC66::
	setorcopyvar VAR_0x8000, ITEM_DAWN_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_Route12 object 11 (local id 11, gfx 0x3F0E) at 9,101
Kanto_Route12_EventScript_846BC73::
	setorcopyvar VAR_0x8000, ITEM_IRON
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_Route12_Text_84355DE::
	.string "A POKéMON is sprawled out in\n"
	.string "a deep and comfortable slumber.$"

Kanto_Route12_Text_84356C9::
	.string "Yeah!\n"
	.string "I got a bite here!$"

Kanto_Route12_Text_84356E2::
	.string "Tch!\n"
	.string "Just a small fry…$"

Kanto_Route12_Text_84356F9::
	.string "Hang on!\n"
	.string "My line’s snagged!$"

Kanto_Route12_Text_8435715::
	.string "Be patient.\n"
	.string "Fishing is a waiting game.$"

Kanto_Route12_Text_843573C::
	.string "That one got away!$"

Kanto_Route12_Text_843574F::
	.string "With a better ROD, I could catch\n"
	.string "better POKéMON…$"

Kanto_Route12_Text_8435780::
	.string "I’m searching for a MOON STONE.\n"
	.string "Have you found one?$"

Kanto_Route12_Text_84357B4::
	.string "Oww!$"

Kanto_Route12_Text_84357B9::
	.string "I could have made my POKéMON\n"
	.string "evolve with a MOON STONE.\p"
	.string "I would have won then, I bet.$"

Kanto_Route12_Text_843580E::
	.string "Electricity is my specialty.\p"
	.string "I don’t know a thing about POKéMON\n"
	.string "of the sea, though.$"

Kanto_Route12_Text_8435862::
	.string "Unplugged!$"

Kanto_Route12_Text_843586D::
	.string "Water conducts electricity, so you\n"
	.string "should zap sea POKéMON.$"

Kanto_Route12_Text_84358A8::
	.string "The FISHING FOOL versus POKéMON\n"
	.string "KID!$"

Kanto_Route12_Text_84358CD::
	.string "Touch too much!$"

Kanto_Route12_Text_84358DD::
	.string "I guess you get to be good at\n"
	.string "what you like.\p"
	.string "Well, you beat me at POKéMON,\n"
	.string "but you can’t top me at fishing.$"

Kanto_Route12_Text_8435949::
	.string "I love fishing, don’t get me wrong.\p"
	.string "But it’d be best if I also had more\n"
	.string "work.$"

Kanto_Route12_Text_8435997::
	.string "It’s not easy…$"

Kanto_Route12_Text_84359A6::
	.string "It’s all right.\n"
	.string "Losing doesn’t bug me anymore.$"

Kanto_Route12_Text_84359D5::
	.string "What’s catching?\p"
	.string "You never know what you could\n"
	.string "catch!$"

Kanto_Route12_Text_8435A0B::
	.string "Lost it!$"

Kanto_Route12_Text_8435A14::
	.string "What, MAGIKARP?\p"
	.string "I catch them all the time, sure.\n"
	.string "But, boy are they wimpy.$"

Kanto_Route12_Text_8435A5E::
	.string "ROUTE 12 \n"
	.string "North to LAVENDER$"

Kanto_Route12_Text_8435A7A::
	.string "SPORTFISHING AREA$"

Kanto_Route12_Text_8435A8C::
	.string "JES: If I win, I’m going to\n"
	.string "propose to GIA.$"

Kanto_Route12_Text_8435AB8::
	.string "JES: Oh, please, why couldn’t you\n"
	.string "let us win?$"

Kanto_Route12_Text_8435AE6::
	.string "JES: Oh, GIA, forgive me,\n"
	.string "my love!$"

Kanto_Route12_Text_8435B09::
	.string "JES: GIA and I, we’ll be\n"
	.string "together forever.\p"
	.string "We won’t battle unless you have\n"
	.string "two POKéMON of your own.$"

Kanto_Route12_Text_8435B6D::
	.string "GIA: Hey, JES…\p"
	.string "If we win, I’ll marry you!$"

Kanto_Route12_Text_8435B97::
	.string "GIA: Oh, but why?$"

Kanto_Route12_Text_8435BA9::
	.string "GIA: JES, you silly!\n"
	.string "You ruined this!$"

Kanto_Route12_Text_8435BCF::
	.string "GIA: I can’t bear to battle\n"
	.string "without my JES!\p"
	.string "Don’t you have one more POKéMON?$"

Kanto_Route12_Text_846F443::
	.string "Yeah!\n"
	.string "I got a bite, here!\l"
	.string "Th-this might be the real thing!$"

Kanto_Route12_Text_846F47E::
	.string "You’re finally here.\n"
	.string "Fishing is a waiting game.$"

Kanto_Route12_Text_846F4AE::
	.string "Still can’t find a MOON STONE…\n"
	.string "Have you found one?$"

Kanto_Route12_Text_846F4E1::
	.string "Electricity has always been my\n"
	.string "specialty.\p"
	.string "I don’t know a thing about POKéMON\n"
	.string "of the sea, though.$"

Kanto_Route12_Text_846F542::
	.string "The FISHING FOOL vs. POKéMON KID!\n"
	.string "Another round, fight!$"

Kanto_Route12_Text_846F57A::
	.string "I love fishing, don’t get me wrong.\n"
	.string "But, I wish I had more work…\l"
	.string "…It’s hard to give up fishing!$"

Kanto_Route12_Text_846F5DA::
	.string "What’s catching?\p"
	.string "You’ll never know unless you beat\n"
	.string "me!$"

Kanto_Route12_Text_846F611::
	.string "JES: I’ll win here today and\n"
	.string "propose to my GIA.$"

Kanto_Route12_Text_846F641::
	.string "GIA: Hey, JES…\n"
	.string "I’ve been waiting a long time now.\p"
	.string "If we win today, I’ll marry you!$"
