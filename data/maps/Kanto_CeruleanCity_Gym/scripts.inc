Kanto_CeruleanCity_Gym_MapScripts::
	.byte 0

@ Kanto_CeruleanCity_Gym object 3 (local id 3, gfx 0x3F23) at 8,6
Kanto_CeruleanCity_Gym_EventScript_841AAA0::
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_8607, 0, Kanto_CeruleanCity_Gym_Text_84424E1, Kanto_CeruleanCity_Gym_Text_8442717, Kanto_CeruleanCity_Gym_EventScript_841AAD2

Kanto_CeruleanCity_Gym_EventScript_841AABF::
	goto_if_unset FLAG_QZ_0x2166, Kanto_CeruleanCity_Gym_EventScript_841AB0A
	msgbox Kanto_CeruleanCity_Gym_Text_84425B1, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_Gym_EventScript_841AAD2::
	goto_if_set FLAG_QZ_0x2101, Kanto_CeruleanCity_Gym_EventScript_841AABF
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	setflag FLAG_QZ_0x2123
	call_if_unset FLAG_QZ_0x2101, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x2101
	setrespawn 0x27
	setvar VAR_0x8008, 2
	call Quetzal_EventScript_8456FD2
	goto Kanto_CeruleanCity_Gym_EventScript_841AB0A

Kanto_CeruleanCity_Gym_EventScript_841AB0A::
	msgbox Kanto_CeruleanCity_Gym_Text_84425F4, MSGBOX_DEFAULT
	checkitemspace ITEM_LEFTOVERS, 1
	goto_if_eq VAR_RESULT, 0, Kanto_CeruleanCity_Gym_EventScript_841AB52
	bufferitemname 0, ITEM_LEFTOVERS
	additem ITEM_LEFTOVERS, 1
	loadword 0, Kanto_CeruleanCity_Gym_Text_84426E8
	setorcopyvar VAR_0x8000, 0x1D8
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2166
	msgbox Kanto_CeruleanCity_Gym_Text_84425B1, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_Gym_EventScript_841AB52::
	msgbox Kanto_CeruleanCity_Gym_Text_84426F8, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeruleanCity_Gym object 2 (local id 2, gfx 0x3F31) at 4,7
Kanto_CeruleanCity_Gym_EventScript_841AB5C::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8342, 0, Kanto_CeruleanCity_Gym_Text_844276B, Kanto_CeruleanCity_Gym_Text_84427BA
	msgbox Kanto_CeruleanCity_Gym_Text_84427CE, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeruleanCity_Gym object 1 (local id 1, gfx 0x3F4F) at 10,12
Kanto_CeruleanCity_Gym_EventScript_841AB73::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8426, 0, Kanto_CeruleanCity_Gym_Text_844280E, Kanto_CeruleanCity_Gym_Text_8442831
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	msgbox Kanto_CeruleanCity_Gym_Text_8442840, MSGBOX_AUTOCLOSE
	end

@ Kanto_CeruleanCity_Gym object 4 (local id 4, gfx 0x3F0B) at 7,16
Kanto_CeruleanCity_Gym_EventScript_841AB97::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2123, Kanto_CeruleanCity_Gym_EventScript_841ABAC
	msgbox Kanto_CeruleanCity_Gym_Text_8442896, MSGBOX_DEFAULT
	release
	end

Kanto_CeruleanCity_Gym_EventScript_841ABAC::
	msgbox Kanto_CeruleanCity_Gym_Text_8442959, MSGBOX_DEFAULT
	release
	end

@ Kanto_CeruleanCity_Gym sign / bg event at 6,17
@ Kanto_CeruleanCity_Gym sign / bg event at 10,17
Kanto_CeruleanCity_Gym_EventScript_841ABB6::
	lockall
	goto_if_set FLAG_QZ_0x2101, Kanto_CeruleanCity_Gym_EventScript_841ABCA
	msgbox Kanto_CeruleanCity_Gym_Text_84429B2, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CeruleanCity_Gym_EventScript_841ABCA::
	msgbox Kanto_CeruleanCity_Gym_Text_84429EC, MSGBOX_DEFAULT
	releaseall
	end

Kanto_CeruleanCity_Gym_Text_84424E1::
	.string "Hi, you’re a new face!\p"
	.string "Only those TRAINERS who have a\n"
	.string "policy about POKéMON can turn pro.\p"
	.string "What is your approach when you\n"
	.string "catch and train POKéMON?\p"
	.string "My policy is an all-out offensive\n"
	.string "with WATER-type POKéMON!{PLAY_BGM 0x022A}$"

Kanto_CeruleanCity_Gym_Text_84425B1::
	.string "A POKéMON holding it restores\n"
	.string "a little HP at the end of each turn.$"

Kanto_CeruleanCity_Gym_Text_84425F4::
	.string "The CASCADEBADGE makes all\n"
	.string "POKéMON up to Lv. 30 obey.\p"
	.string "That includes even outsiders you\n"
	.string "got in trades.\p"
	.string "There’s more. You can now use CUT\n"
	.string "anytime, even out of battle.\p"
	.string "You can CUT down small trees to\n"
	.string "open new pathways.\p"
	.string "Please take this gift, too.$"

Kanto_CeruleanCity_Gym_Text_84426E8::
	.string "{PLAYER} received\n"
	.string "{STR_VAR_1}!$"

Kanto_CeruleanCity_Gym_Text_84426F8::
	.string "You better make room for this!$"

Kanto_CeruleanCity_Gym_Text_8442717::
	.string "Wow!\n"
	.string "You’re too much!\p"
	.string "All right!\p"
	.string "You can have the CASCADEBADGE to\n"
	.string "show you beat me.$"

Kanto_CeruleanCity_Gym_Text_844276B::
	.string "What? You?\n"
	.string "I’m more than good enough for you!\p"
	.string "MISTY won’t have to be bothered.$"

Kanto_CeruleanCity_Gym_Text_84427BA::
	.string "You overwhelmed me!$"

Kanto_CeruleanCity_Gym_Text_84427CE::
	.string "You have to face other TRAINERS\n"
	.string "to see how good you really are.$"

Kanto_CeruleanCity_Gym_Text_844280E::
	.string "Splash!\p"
	.string "I’m first up!\n"
	.string "Let’s do it!$"

Kanto_CeruleanCity_Gym_Text_8442831::
	.string "That can’t be!$"

Kanto_CeruleanCity_Gym_Text_8442840::
	.string "MISTY is a TRAINER who’s going to\n"
	.string "keep improving.\p"
	.string "She won’t lose to someone like you!$"

Kanto_CeruleanCity_Gym_Text_8442896::
	.string "Yo!\n"
	.string "Champ in the making!\p"
	.string "Let me give you some advice!\p"
	.string "The LEADER, MISTY, is a pro who\n"
	.string "uses WATER-type POKéMON.\p"
	.string "GRASS and ELECTRIC types\n"
	.string "can help!\p"
	.string "But watch out for Abilities\n"
	.string "and secondary types!$"

Kanto_CeruleanCity_Gym_Text_8442959::
	.string "You beat MISTY!\n"
	.string "See, what’d I tell ya?\p"
	.string "You and me, kid, we make a pretty\n"
	.string "darn-good team!$"

Kanto_CeruleanCity_Gym_Text_84429B2::
	.string "CERULEAN POKéMON GYM\n"
	.string "LEADER: MISTY\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE$"

Kanto_CeruleanCity_Gym_Text_84429EC::
	.string "CERULEAN POKéMON GYM\n"
	.string "LEADER: MISTY\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE, {PLAYER}$"
