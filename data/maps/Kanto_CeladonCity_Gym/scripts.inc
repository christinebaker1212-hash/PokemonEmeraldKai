Kanto_CeladonCity_Gym_MapScripts::
	.byte 0

@ Kanto_CeladonCity_Gym object 7 (local id 7, gfx 0x3F03) at 6,4
Kanto_CeladonCity_Gym_EventScript_841D2B0::
	setvar VAR_0x8004, 5
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_8609, 0, Kanto_CeladonCity_Gym_Text_84472A6, Kanto_CeladonCity_Gym_Text_84473F2, Kanto_CeladonCity_Gym_EventScript_841D2EF

Kanto_CeladonCity_Gym_EventScript_841D2CF::
	goto_if_unset FLAG_QZ_0x216A, Kanto_CeladonCity_Gym_EventScript_841D327
	setvar VAR_0x8004, 5
	setvar VAR_0x8005, 4
	special QzQuestLogNop
	msgbox Kanto_CeladonCity_Gym_Text_844744A, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_Gym_EventScript_841D2EF::
	goto_if_set FLAG_QZ_0x2103, Kanto_CeladonCity_Gym_EventScript_841D2CF
	setvar VAR_0x8004, 5
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	clearflag FLAG_QZ_0x2187
	setflag FLAG_QZ_0x211D
	call_if_unset FLAG_QZ_0x2103, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x2103
	setvar VAR_0x8008, 4
	call Quetzal_EventScript_8456FD2
	goto Kanto_CeladonCity_Gym_EventScript_841D327

Kanto_CeladonCity_Gym_EventScript_841D327::
	msgbox Kanto_CeladonCity_Gym_Text_84474B9, MSGBOX_DEFAULT
	checkitemspace ITEM_BIG_ROOT, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeladonCity_Gym_EventScript_841D36F
	bufferitemname 0, ITEM_BIG_ROOT
	additem ITEM_BIG_ROOT, 1
	loadword 0, Kanto_CeladonCity_Gym_Text_844754C
	setorcopyvar VAR_0x8000, 0x1EB
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x216A
	msgbox Kanto_CeladonCity_Gym_Text_844755C, MSGBOX_DEFAULT
	release
	end

Kanto_CeladonCity_Gym_EventScript_841D36F::
	msgbox Kanto_CeladonCity_Gym_Text_8447597, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeladonCity_Gym object 1 (local id 1, gfx 0x3F16) at 3,11
Kanto_CeladonCity_Gym_EventScript_841D379::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8324, 0, Kanto_CeladonCity_Gym_Text_84475B6, Kanto_CeladonCity_Gym_Text_84475FE
	msgbox Kanto_CeladonCity_Gym_Text_8447610, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeladonCity_Gym object 6 (local id 6, gfx 0x3F16) at 7,4
Kanto_CeladonCity_Gym_EventScript_841D390::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8325, 0, Kanto_CeladonCity_Gym_Text_84477FF, Kanto_CeladonCity_Gym_Text_8447839
	setvar VAR_0x8004, 5
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_CeladonCity_Gym_Text_8447842, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeladonCity_Gym object 3 (local id 3, gfx 0x3F31) at 10,7
Kanto_CeladonCity_Gym_EventScript_841D3B4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8352, 0, Kanto_CeladonCity_Gym_Text_84476CF, Kanto_CeladonCity_Gym_Text_84476F5
	msgbox Kanto_CeladonCity_Gym_Text_844770B, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeladonCity_Gym object 2 (local id 2, gfx 0x3EE8) at 9,10
Kanto_CeladonCity_Gym_EventScript_841D3CB::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8457, 0, Kanto_CeladonCity_Gym_Text_8447634, Kanto_CeladonCity_Gym_Text_8447656
	msgbox Kanto_CeladonCity_Gym_Text_8447661, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeladonCity_Gym object 4 (local id 4, gfx 0x3EE8) at 2,6
Kanto_CeladonCity_Gym_EventScript_841D3E2::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8458, 0, Kanto_CeladonCity_Gym_Text_8447745, Kanto_CeladonCity_Gym_Text_844779A
	setvar VAR_0x8004, 5
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	msgbox Kanto_CeladonCity_Gym_Text_844779E, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeladonCity_Gym object 5 (local id 5, gfx 0x3EE8) at 5,4
Kanto_CeladonCity_Gym_EventScript_841D406::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8459, 0, Kanto_CeladonCity_Gym_Text_8447881, Kanto_CeladonCity_Gym_Text_84478B4
	msgbox Kanto_CeladonCity_Gym_Text_84478C2, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeladonCity_Gym object 8 (local id 8, gfx 0x3EFC) at 8,7
Kanto_CeladonCity_Gym_EventScript_841D41D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8594, 0, Kanto_CeladonCity_Gym_Text_844791F, Kanto_CeladonCity_Gym_Text_844796C
	msgbox Kanto_CeladonCity_Gym_Text_8447978, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeladonCity_Gym sign / bg event at 4,16
@ Kanto_CeladonCity_Gym sign / bg event at 8,16
Kanto_CeladonCity_Gym_EventScript_841D434::
	lockall
	goto_if_set FLAG_QZ_0x2103, Kanto_CeladonCity_Gym_EventScript_841D448
	msgbox Kanto_CeladonCity_Gym_Text_84479AE, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CeladonCity_Gym_EventScript_841D448::
	msgbox Kanto_CeladonCity_Gym_Text_84479E7, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CeladonCity_Gym_Text_84472A6::
	.string "Hello…\n"
	.string "Lovely weather, isn’t it?\l"
	.string "It’s so pleasant…\p"
	.string "…Oh, dear…\n"
	.string "I must have dozed off. Welcome.\p"
	.string "My name is ERIKA.\n"
	.string "I am the LEADER of CELADON GYM.\p"
	.string "I am a student of the art of\n"
	.string "flower arranging.\p"
	.string "My POKéMON are solely of the\n"
	.string "GRASS type.\p"
	.string "…Oh, I’m sorry, I had no idea that\n"
	.string "you wished to challenge me.\p"
	.string "Very well, but I shall not lose.{PLAY_BGM 0x022A}$"

Kanto_CeladonCity_Gym_Text_84473F2::
	.string "Oh!\n"
	.string "I concede defeat.\l"
	.string "You are remarkably strong.\p"
	.string "I must confer on you the\n"
	.string "RAINBOWBADGE.$"

Kanto_CeladonCity_Gym_Text_844744A::
	.string "You are cataloging POKéMON?\n"
	.string "I must say I’m impressed.\p"
	.string "I would never collect POKéMON if\n"
	.string "they were unattractive.$"

Kanto_CeladonCity_Gym_Text_84474B9::
	.string "The RAINBOWBADGE will make\n"
	.string "POKéMON up to Lv. 50 obey.\p"
	.string "It also allows POKéMON to use\n"
	.string "STRENGTH in and out of battle.\p"
	.string "Please also take this with you.$"

Kanto_CeladonCity_Gym_Text_844754C::
	.string "{PLAYER} received\n"
	.string "{STR_VAR_1}!$"

Kanto_CeladonCity_Gym_Text_844755C::
	.string "A POKéMON holding it restores more\n"
	.string "HP with draining moves.$"

Kanto_CeladonCity_Gym_Text_8447597::
	.string "You should make room for this.$"

Kanto_CeladonCity_Gym_Text_84475B6::
	.string "I should tell you about this GYM.\p"
	.string "Only real ladies are allowed in\n"
	.string "here!$"

Kanto_CeladonCity_Gym_Text_84475FE::
	.string "You’re too rough!$"

Kanto_CeladonCity_Gym_Text_8447610::
	.string "Bleaah!\n"
	.string "I hope ERIKA wipes you out!$"

Kanto_CeladonCity_Gym_Text_8447634::
	.string "Oh, welcome.\n"
	.string "I was getting bored.$"

Kanto_CeladonCity_Gym_Text_8447656::
	.string "My makeup!$"

Kanto_CeladonCity_Gym_Text_8447661::
	.string "GRASS-type POKéMON are tough\n"
	.string "against the WATER type.\p"
	.string "They also have an edge on ROCK-\n"
	.string "and GROUND-type POKéMON.$"

Kanto_CeladonCity_Gym_Text_84476CF::
	.string "…Weren’t you peeking in here\n"
	.string "earlier?$"

Kanto_CeladonCity_Gym_Text_84476F5::
	.string "You’re an eye-opener!$"

Kanto_CeladonCity_Gym_Text_844770B::
	.string "Oh, you were looking at ERIKA…\n"
	.string "You weren’t looking at me…$"

Kanto_CeladonCity_Gym_Text_8447745::
	.string "Look, look!\n"
	.string "See my POKéMON!\p"
	.string "I like the GRASS type.\n"
	.string "I like how they’re easy to raise.$"

Kanto_CeladonCity_Gym_Text_844779A::
	.string "No!$"

Kanto_CeladonCity_Gym_Text_844779E::
	.string "We only use GRASS-type POKéMON at\n"
	.string "our GYM.\p"
	.string "Why? We also use them for making\n"
	.string "flower arrangements!$"

Kanto_CeladonCity_Gym_Text_84477FF::
	.string "Oh, hey!\p"
	.string "We don’t like BUG- or FIRE-type\n"
	.string "POKéMON in here!$"

Kanto_CeladonCity_Gym_Text_8447839::
	.string "Oh!\n"
	.string "You!$"

Kanto_CeladonCity_Gym_Text_8447842::
	.string "Our LEADER ERIKA might be quiet,\n"
	.string "but she’s famous around here.$"

Kanto_CeladonCity_Gym_Text_8447881::
	.string "Pleased to meet you.\n"
	.string "My hobby is POKéMON training.$"

Kanto_CeladonCity_Gym_Text_84478B4::
	.string "Oh!\n"
	.string "Splendid!$"

Kanto_CeladonCity_Gym_Text_84478C2::
	.string "I have a blind date coming up.\n"
	.string "I have to learn to be polite,\l"
	.string "especially if I have to battle.$"

Kanto_CeladonCity_Gym_Text_844791F::
	.string "Welcome to CELADON GYM!\p"
	.string "You’d better not underestimate\n"
	.string "the nice ladies here.$"

Kanto_CeladonCity_Gym_Text_844796C::
	.string "Oh!\n"
	.string "Beaten!$"

Kanto_CeladonCity_Gym_Text_8447978::
	.string "I didn’t bring my best POKéMON.\n"
	.string "Wait until next time!$"

Kanto_CeladonCity_Gym_Text_84479AE::
	.string "CELADON POKéMON GYM\n"
	.string "LEADER: ERIKA\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE$"

Kanto_CeladonCity_Gym_Text_84479E7::
	.string "CELADON POKéMON GYM\n"
	.string "LEADER: ERIKA\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE, {PLAYER}$"
