Johto_VioletGym_MapScripts::
	.byte 0

@ Johto_VioletGym object 4 (local id 4, gfx 0x3E98) at 11,17
Johto_VioletGym_EventScript_83FF08D::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11A8, Johto_VioletGym_EventScript_83FF0A3
	msgbox Johto_VioletGym_Text_83FF6E0, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_VioletGym_EventScript_83FF0A3::
	msgbox Johto_VioletGym_Text_83FF898, MSGBOX_DEFAULT
	release
	end

@ Johto_VioletGym object 3 (local id 3, gfx 0x3EAE) at 6,13
Johto_VioletGym_EventScript_83FF0AD::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4105, 0, Johto_VioletGym_Text_83FF5B4, Johto_VioletGym_Text_83FF618
	msgbox Johto_VioletGym_Text_83FF61F, MSGBOX_AUTOCLOSE
	end

@ Johto_VioletGym object 2 (local id 2, gfx 0x3EAE) at 12,9
Johto_VioletGym_EventScript_83FF0C4::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4454, 0, Johto_VioletGym_Text_83FF668, Johto_VioletGym_Text_83FF69B
	msgbox Johto_VioletGym_Text_83FF6AF, MSGBOX_AUTOCLOSE
	end

@ Johto_VioletGym object 1 (local id 1, gfx 0x3E93) at 9,4
Johto_VioletGym_EventScript_83FF0DB::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11A8, Johto_VioletGym_EventScript_83FF175
	msgbox Johto_VioletGym_Text_83FF1C2, MSGBOX_DEFAULT
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_VioletGym_EventScript_83FF10D
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4115, 0, Johto_VioletGym_Text_83FF2A8
	goto Johto_VioletGym_EventScript_83FF117

Johto_VioletGym_EventScript_83FF10D::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4963, 0, Johto_VioletGym_Text_83FF2A8

Johto_VioletGym_EventScript_83FF117::
	message Johto_VioletGym_Text_83FF314
	waitmessage
	call Quetzal_EventScript_83602D4
	msgbox Johto_VioletGym_Text_83FF32D, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x11B2, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x11B2
	setflag FLAG_QZ_0x11A8
	addvar VAR_QZ_0x502E, 1
	setvar VAR_0x8008, 1
	call Quetzal_EventScript_83601F0
	setvar VAR_QZ_0x5008, 3
	setrespawn 0x19

Johto_VioletGym_EventScript_83FF150::
	setorcopyvar VAR_0x8000, 0x1FE
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1494
	msgbox Johto_VioletGym_Text_83FF3B7, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_VioletGym_EventScript_83FF175::
	goto_if_unset FLAG_QZ_0x1494, Johto_VioletGym_EventScript_83FF150
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_VioletGym_EventScript_83FF1A5
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4115, 0, Johto_VioletGym_Text_83FF1C2, Johto_VioletGym_Text_83FF2A8, Johto_VioletGym_EventScript_83FF1B7
	goto Johto_VioletGym_EventScript_83FF1B7

Johto_VioletGym_EventScript_83FF1A5::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4963, 0, Johto_VioletGym_Text_83FF1C2, Johto_VioletGym_Text_83FF2A8, Johto_VioletGym_EventScript_83FF1B7

Johto_VioletGym_EventScript_83FF1B7::
	msgbox Johto_VioletGym_Text_83FF42E, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletGym sign / bg event at 6,17
@ Johto_VioletGym sign / bg event at 12,17
Johto_VioletGym_EventScript_83FF8DC::
	lock
	goto_if_set FLAG_QZ_0x11A8, Johto_VioletGym_EventScript_83FF8F1
	msgbox Johto_VioletGym_Text_83FF8FC, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_VioletGym_EventScript_83FF8F1::
	msgbox Johto_VioletGym_Text_83FF914, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_VioletGym_Text_83FF1C2::
	.string "I’m FALKNER, the VIOLET POKéMON\n"
	.string "GYM leader!\p"
	.string "People say you can clip\n"
	.string "FLYING-type POKéMON’s wings\p"
	.string "with a jolt of electricity…\n"
	.string "I won’t allow such insults to bird\l"
	.string "POKéMON!\p"
	.string "I’ll show you the real power\n"
	.string "of the magnificent bird POKéMON!$"

Johto_VioletGym_Text_83FF2A8::
	.string "…Darn! My dad’s cherished bird\n"
	.string "POKéMON…\p"
	.string "All right. Take this.\p"
	.string "It’s the official POKéMON LEAGUE\n"
	.string "ZEPHYRBADGE.$"

Johto_VioletGym_Text_83FF314::
	.string "{PLAYER} received ZEPHYRBADGE.$"

Johto_VioletGym_Text_83FF32D::
	.string "ZEPHYRBADGE raises the ATTACK\n"
	.string "power of POKéMON.\p"
	.string "It also enables POKéMON to use\n"
	.string "FLASH, if they can learn it,\l"
	.string "anytime.\p"
	.string "Here--take this too.$"

Johto_VioletGym_Text_83FF3B7::
	.string "HEAVY-DUTY BOOTS protect against\n"
	.string "hazards when entering battle.\p"
	.string "My flying POKéMON appreciate\n"
	.string "being able to land safely!$"

Johto_VioletGym_Text_83FF42E::
	.string "There are POKéMON GYMS in cities\n"
	.string "and towns ahead.\p"
	.string "You should test your skills at\n"
	.string "these GYMS.\p"
	.string "I’m going to train harder to\n"
	.string "become the greatest bird master!$"

Johto_VioletGym_Text_83FF5B4::
	.string "The keyword is guts!\p"
	.string "Those here are training night and\n"
	.string "day to become bird POKéMON masters.\p"
	.string "Come on!$"

Johto_VioletGym_Text_83FF618::
	.string "Gaaah!$"

Johto_VioletGym_Text_83FF61F::
	.string "FALKNER’s skills are for real!\p"
	.string "Don’t get cocky just because\n"
	.string "you beat me!$"

Johto_VioletGym_Text_83FF668::
	.string "Let me see if you are good\n"
	.string "enough to face FALKNER!$"

Johto_VioletGym_Text_83FF69B::
	.string "This can’t be true!$"

Johto_VioletGym_Text_83FF6AF::
	.string "This is pathetic, losing to some\n"
	.string "rookie trainer…$"

Johto_VioletGym_Text_83FF6E0::
	.string "Hey! What’s up? I’m no trainer,\n"
	.string "but I can give some advice!\p"
	.string "Believe me! If you believe, a\n"
	.string "championship dream can come true.\p"
	.string "You believe? Then listen.\p"
	.string "FALKNER’s ace is a bulky owl\n"
	.string "that loves to eat BERRIES.\p"
	.string "While it’s not the strongest,\n"
	.string "it can take a lot of hits.\p"
	.string "You can hurt it with ELECTRIC \n"
	.string "and ROCK-type moves, but watch out\l"
	.string "for MUD-SLAP!\p"
	.string "If you want more help, go visit the\n"
	.string "POKéMON ACADEMY.\p"
	.string "The students there really know\n"
	.string "their stuff!$"

Johto_VioletGym_Text_83FF898::
	.string "Nice battle! Keep it up,\n"
	.string "and you’ll be the CHAMP in no\l"
	.string "time at all!$"

Johto_VioletGym_Text_83FF8FC::
	.string "VIOLET CITY POKéMON GYM$"

Johto_VioletGym_Text_83FF914::
	.string "VIOLET CITY POKéMON GYM\p"
	.string "FALKNER’S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"
