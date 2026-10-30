Kanto_FuchsiaCity_Gym_MapScripts::
	.byte 0

@ Kanto_FuchsiaCity_Gym object 7 (local id 7, gfx 0x3F12) at 7,13
Kanto_FuchsiaCity_Gym_EventScript_841D6DA::
	setvar VAR_0x8004, 6
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_8610, 0, Kanto_FuchsiaCity_Gym_Text_84482BE, Kanto_FuchsiaCity_Gym_Text_84483D4, Kanto_FuchsiaCity_Gym_EventScript_841D70C

Kanto_FuchsiaCity_Gym_EventScript_841D6F9::
	goto_if_unset FLAG_QZ_0x2168, Kanto_FuchsiaCity_Gym_EventScript_841D744
	msgbox Kanto_FuchsiaCity_Gym_Text_8448411, MSGBOX_DEFAULT
	release
	end

Kanto_FuchsiaCity_Gym_EventScript_841D70C::
	goto_if_set FLAG_QZ_0x2104, Kanto_FuchsiaCity_Gym_EventScript_841D6F9
	setvar VAR_0x8004, 6
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	clearflag FLAG_QZ_0x2188
	setflag FLAG_QZ_0x211E
	call_if_unset FLAG_QZ_0x2104, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x2104
	setvar VAR_0x8008, 5
	call Quetzal_EventScript_8456FD2
	goto Kanto_FuchsiaCity_Gym_EventScript_841D744

Kanto_FuchsiaCity_Gym_EventScript_841D744::
	msgbox Kanto_FuchsiaCity_Gym_Text_844849E, MSGBOX_DEFAULT
	checkitemspace ITEM_BLACK_SLUDGE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_FuchsiaCity_Gym_EventScript_841D78C
	bufferitemname 0, ITEM_BLACK_SLUDGE
	additem ITEM_BLACK_SLUDGE, 1
	loadword 0, Kanto_FuchsiaCity_Gym_Text_844851F
	setorcopyvar VAR_0x8000, 0x1E7
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2168
	msgbox Kanto_FuchsiaCity_Gym_Text_844852F, MSGBOX_DEFAULT
	release
	end

Kanto_FuchsiaCity_Gym_EventScript_841D78C::
	msgbox Kanto_FuchsiaCity_Gym_Text_8448584, MSGBOX_DEFAULT
	release
	end

@ Kanto_FuchsiaCity_Gym object 5 (local id 5, gfx 0x3F1E) at 4,6
Kanto_FuchsiaCity_Gym_EventScript_841D796::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8486, 0, Kanto_FuchsiaCity_Gym_Text_84487DD, Kanto_FuchsiaCity_Gym_Text_844882D
	msgbox Kanto_FuchsiaCity_Gym_Text_8448842, MSGBOX_AUTOCLOSE
	end

@ Kanto_FuchsiaCity_Gym object 4 (local id 4, gfx 0x3F1E) at 13,4
Kanto_FuchsiaCity_Gym_EventScript_841D7AD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8487, 0, Kanto_FuchsiaCity_Gym_Text_8448896, Kanto_FuchsiaCity_Gym_Text_84488F6
	msgbox Kanto_FuchsiaCity_Gym_Text_84488FC, MSGBOX_AUTOCLOSE
	end

@ Kanto_FuchsiaCity_Gym object 3 (local id 3, gfx 0x3F3B) at 11,11
Kanto_FuchsiaCity_Gym_EventScript_841D7C4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8480, 0, Kanto_FuchsiaCity_Gym_Text_8448695, Kanto_FuchsiaCity_Gym_Text_84486F0
	setvar VAR_0x8004, 6
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_FuchsiaCity_Gym_Text_84486FE, MSGBOX_AUTOCLOSE
	end

@ Kanto_FuchsiaCity_Gym object 2 (local id 2, gfx 0x3F3B) at 2,9
Kanto_FuchsiaCity_Gym_EventScript_841D7E8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8481, 0, Kanto_FuchsiaCity_Gym_Text_8448918, Kanto_FuchsiaCity_Gym_Text_8448961
	msgbox Kanto_FuchsiaCity_Gym_Text_8448987, MSGBOX_AUTOCLOSE
	end

@ Kanto_FuchsiaCity_Gym object 1 (local id 1, gfx 0x3F3B) at 12,16
Kanto_FuchsiaCity_Gym_EventScript_841D7FF::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8484, 0, Kanto_FuchsiaCity_Gym_Text_84485A0, Kanto_FuchsiaCity_Gym_Text_844862B
	msgbox Kanto_FuchsiaCity_Gym_Text_8448640, MSGBOX_AUTOCLOSE
	end

@ Kanto_FuchsiaCity_Gym object 6 (local id 6, gfx 0x3F3B) at 2,16
Kanto_FuchsiaCity_Gym_EventScript_841D816::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8485, 0, Kanto_FuchsiaCity_Gym_Text_844875F, Kanto_FuchsiaCity_Gym_Text_8448789
	msgbox Kanto_FuchsiaCity_Gym_Text_844879C, MSGBOX_AUTOCLOSE
	end

@ Kanto_FuchsiaCity_Gym object 8 (local id 8, gfx 0x3F0B) at 11,19
Kanto_FuchsiaCity_Gym_EventScript_841D82D::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x211E, Kanto_FuchsiaCity_Gym_EventScript_841D842
	msgbox Kanto_FuchsiaCity_Gym_Text_84489D5, MSGBOX_DEFAULT
	release
	end

Kanto_FuchsiaCity_Gym_EventScript_841D842::
	msgbox Kanto_FuchsiaCity_Gym_Text_8448A94, MSGBOX_DEFAULT
	release
	end

@ Kanto_FuchsiaCity_Gym sign / bg event at 4,19
@ Kanto_FuchsiaCity_Gym sign / bg event at 10,19
Kanto_FuchsiaCity_Gym_EventScript_841D84C::
	lockall
	goto_if_set FLAG_QZ_0x2104, Kanto_FuchsiaCity_Gym_EventScript_841D860
	msgbox Kanto_FuchsiaCity_Gym_Text_8448AC2, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FuchsiaCity_Gym_EventScript_841D860::
	msgbox Kanto_FuchsiaCity_Gym_Text_8448AFA, MSGBOX_DEFAULT
	releaseall
	end

Kanto_FuchsiaCity_Gym_Text_84482BE::
	.string "KOGA: Fwahahaha!\p"
	.string "A mere child like you dares to\n"
	.string "challenge me?\p"
	.string "The very idea makes me shiver\n"
	.string "with mirth!\p"
	.string "Very well, I shall show you true\n"
	.string "terror as a ninja master.\p"
	.string "Poison brings steady doom.\n"
	.string "Sleep renders foes helpless.\p"
	.string "Despair to the creeping horror of\n"
	.string "POISON-type POKéMON!{PLAY_BGM 0x022A}$"

Kanto_FuchsiaCity_Gym_Text_84483D4::
	.string "Humph!\n"
	.string "You have proven your worth!\p"
	.string "Here!\n"
	.string "Take the SOULBADGE!$"

Kanto_FuchsiaCity_Gym_Text_8448411::
	.string "When afflicted by TOXIC, a POKéMON\n"
	.string "suffers more and more.\p"
	.string "It suffers worsening damage as the\n"
	.string "battle wears on!\p"
	.string "It will surely terrorize foes!$"

Kanto_FuchsiaCity_Gym_Text_844849E::
	.string "Now that you have the SOULBADGE,\n"
	.string "the DEFENSE of your POKéMON rises.\p"
	.string "It also lets you SURF outside of\n"
	.string "battle.\p"
	.string "Ah!\n"
	.string "Take this, too!$"

Kanto_FuchsiaCity_Gym_Text_844851F::
	.string "{PLAYER} received\n"
	.string "{STR_VAR_1}!$"

Kanto_FuchsiaCity_Gym_Text_844852F::
	.string "It restores a Poison-type holder’s\n"
	.string "HP each turn, but damages holders\l"
	.string "of other types.$"

Kanto_FuchsiaCity_Gym_Text_8448584::
	.string "Make space for this, child!$"

Kanto_FuchsiaCity_Gym_Text_84485A0::
	.string "Strength isn’t the key for POKéMON.\n"
	.string "Do you understand this?\p"
	.string "POKéMON is about strategy!\p"
	.string "I’ll show you how strategy can\n"
	.string "beat brute strength.$"

Kanto_FuchsiaCity_Gym_Text_844862B::
	.string "What?\n"
	.string "Extraordinary!$"

Kanto_FuchsiaCity_Gym_Text_8448640::
	.string "So, you mix brawn with brains?\n"
	.string "Good strategy!\p"
	.string "That’s remarkable for a child\n"
	.string "TRAINER.$"

Kanto_FuchsiaCity_Gym_Text_8448695::
	.string "I was a magician once upon a time.\p"
	.string "But I dreamt of becoming a ninja,\n"
	.string "so I joined this GYM.$"

Kanto_FuchsiaCity_Gym_Text_84486F0::
	.string "I’m done for!$"

Kanto_FuchsiaCity_Gym_Text_84486FE::
	.string "Even though I’ve lost, I will keep\n"
	.string "training according to the teachings\l"
	.string "of KOGA, my ninja master.$"

Kanto_FuchsiaCity_Gym_Text_844875F::
	.string "Let’s see you beat my special\n"
	.string "techniques!$"

Kanto_FuchsiaCity_Gym_Text_8448789::
	.string "You had me fooled!$"

Kanto_FuchsiaCity_Gym_Text_844879C::
	.string "I like poison and sleep techniques,\n"
	.string "as they linger after battle!$"

Kanto_FuchsiaCity_Gym_Text_84487DD::
	.string "Stop right there!\p"
	.string "The famed invisible walls of\n"
	.string "FUCHSIA GYM have you frustrated?$"

Kanto_FuchsiaCity_Gym_Text_844882D::
	.string "Whoa!\n"
	.string "You’ve got it!$"

Kanto_FuchsiaCity_Gym_Text_8448842::
	.string "You impressed me!\n"
	.string "Here’s a hint!\p"
	.string "Look very closely for gaps in the\n"
	.string "invisible walls!$"

Kanto_FuchsiaCity_Gym_Text_8448896::
	.string "I also study the way of the ninja\n"
	.string "with Master KOGA!\p"
	.string "Ninja have a long history of using\n"
	.string "animals!$"

Kanto_FuchsiaCity_Gym_Text_84488F6::
	.string "Awoo!$"

Kanto_FuchsiaCity_Gym_Text_84488FC::
	.string "I still have much to learn.$"

Kanto_FuchsiaCity_Gym_Text_8448918::
	.string "Master KOGA comes from a long line\n"
	.string "of ninjas.\p"
	.string "What did you descend from?$"

Kanto_FuchsiaCity_Gym_Text_8448961::
	.string "You’re more skilled than I’d\n"
	.string "thought!$"

Kanto_FuchsiaCity_Gym_Text_8448987::
	.string "Where there is light, there is\n"
	.string "shadow!\p"
	.string "Light and shadow!\n"
	.string "Which do you choose?$"

Kanto_FuchsiaCity_Gym_Text_84489D5::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "FUCHSIA GYM is a tricked-up place.\n"
	.string "It’s riddled with invisible walls!\p"
	.string "KOGA might appear close, but he’s\n"
	.string "blocked off.\p"
	.string "You have to find gaps in the walls\n"
	.string "to reach him.$"

Kanto_FuchsiaCity_Gym_Text_8448A94::
	.string "It’s amazing how ninja can terrify,\n"
	.string "even now!$"

Kanto_FuchsiaCity_Gym_Text_8448AC2::
	.string "FUCHSIA POKéMON GYM\n"
	.string "LEADER: KOGA\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE$"

Kanto_FuchsiaCity_Gym_Text_8448AFA::
	.string "FUCHSIA POKéMON GYM\n"
	.string "LEADER: KOGA\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE, {PLAYER}$"
