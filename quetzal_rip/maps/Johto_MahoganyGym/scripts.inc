Johto_MahoganyGym_MapScripts::
	.byte 0

@ Johto_MahoganyGym sign / bg event at 10,26
@ Johto_MahoganyGym sign / bg event at 14,26
Johto_MahoganyGym_EventScript_83DE565::
	lock
	goto_if_set FLAG_QZ_0x11AE, Johto_MahoganyGym_EventScript_83DE57A
	msgbox Johto_MahoganyGym_Text_83DE585, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MahoganyGym_EventScript_83DE57A::
	msgbox Johto_MahoganyGym_Text_83DE59F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_MahoganyGym object 7 (local id 7, gfx 0x3E98) at 13,26
Johto_MahoganyGym_EventScript_83DE5D8::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AE, Johto_MahoganyGym_EventScript_83DE5EE
	msgbox Johto_MahoganyGym_Text_83DEC22, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MahoganyGym_EventScript_83DE5EE::
	msgbox Johto_MahoganyGym_Text_83DECE3, MSGBOX_DEFAULT
	release
	end

@ Johto_MahoganyGym object 3 (local id 3, gfx 0x30) at 17,23
Johto_MahoganyGym_EventScript_83DE5F8::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4446, 0, Johto_MahoganyGym_Text_83DED49, Johto_MahoganyGym_Text_83DED7C
	msgbox Johto_MahoganyGym_Text_83DED99, MSGBOX_AUTOCLOSE
	end

@ Johto_MahoganyGym object 2 (local id 2, gfx 0x3EA7) at 17,9
Johto_MahoganyGym_EventScript_83DE60F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4797, 0, Johto_MahoganyGym_Text_83DEDD5, Johto_MahoganyGym_Text_83DEE2F
	msgbox Johto_MahoganyGym_Text_83DEE4E, MSGBOX_AUTOCLOSE
	end

@ Johto_MahoganyGym object 4 (local id 4, gfx 0x3EA6) at 5,8
Johto_MahoganyGym_EventScript_83DE626::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4249, 0, Johto_MahoganyGym_Text_83DEE82, Johto_MahoganyGym_Text_83DEE99
	msgbox Johto_MahoganyGym_Text_83DEEBB, MSGBOX_AUTOCLOSE
	end

@ Johto_MahoganyGym object 5 (local id 5, gfx 0x1E) at 12,11
Johto_MahoganyGym_EventScript_83DE63D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4938, 0, Johto_MahoganyGym_Text_83DEF22, Johto_MahoganyGym_Text_83DEF67
	msgbox Johto_MahoganyGym_Text_83DEF89, MSGBOX_AUTOCLOSE
	end

@ Johto_MahoganyGym object 1 (local id 1, gfx 0x1E) at 8,3
Johto_MahoganyGym_EventScript_83DE654::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4531, 0, Johto_MahoganyGym_Text_83DEFCB, Johto_MahoganyGym_Text_83DEFE7
	msgbox Johto_MahoganyGym_Text_83DF001, MSGBOX_AUTOCLOSE
	end

@ Johto_MahoganyGym object 6 (local id 6, gfx 0x3EAD) at 12,4
Johto_MahoganyGym_EventScript_83DE66B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AE, Johto_MahoganyGym_EventScript_83DE70F
	msgbox Johto_MahoganyGym_Text_83DE75C, MSGBOX_DEFAULT
	goto Johto_MahoganyGym_EventScript_83DE6E1

Johto_MahoganyGym_EventScript_83DE683::
	message Johto_MahoganyGym_Text_83DE90D
	waitmessage
	call Quetzal_EventScript_83602D4
	msgbox Johto_MahoganyGym_Text_83DE927, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x11B8, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x11B8
	setflag FLAG_QZ_0x11AE
	addvar VAR_QZ_0x502E, 1
	call Quetzal_EventScript_83DF68E
	setvar VAR_0x8008, 7
	call Quetzal_EventScript_83601F0
	setrespawn 0x20

Johto_MahoganyGym_EventScript_83DE6BC::
	setorcopyvar VAR_0x8000, 0x1E1
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1498
	msgbox Johto_MahoganyGym_Text_83DE9C5, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MahoganyGym_EventScript_83DE6E1::
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_MahoganyGym_EventScript_83DE700
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4803, 0, Johto_MahoganyGym_Text_83DE885
	goto Johto_MahoganyGym_EventScript_83DE683

Johto_MahoganyGym_EventScript_83DE700::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4975, 0, Johto_MahoganyGym_Text_83DE885
	goto Johto_MahoganyGym_EventScript_83DE683

Johto_MahoganyGym_EventScript_83DE70F::
	goto_if_unset FLAG_QZ_0x1498, Johto_MahoganyGym_EventScript_83DE6BC
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_MahoganyGym_EventScript_83DE73F
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4803, 0, Johto_MahoganyGym_Text_83DE75C, Johto_MahoganyGym_Text_83DE885, Johto_MahoganyGym_EventScript_83DE751
	goto Johto_MahoganyGym_EventScript_83DE751

Johto_MahoganyGym_EventScript_83DE73F::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4975, 0, Johto_MahoganyGym_Text_83DE75C, Johto_MahoganyGym_Text_83DE885, Johto_MahoganyGym_EventScript_83DE751

Johto_MahoganyGym_EventScript_83DE751::
	msgbox Johto_MahoganyGym_Text_83DEA4F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_MahoganyGym_Text_83DE585::
	.string "MAHOGANY TOWN POKéMON GYM$"

Johto_MahoganyGym_Text_83DE59F::
	.string "MAHOGANY TOWN POKéMON GYM\p"
	.string "PRYCE’S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

Johto_MahoganyGym_Text_83DE75C::
	.string "POKéMON have many experiences\n"
	.string "in their lives, just like we do.\p"
	.string "I, too, have seen and suffered\n"
	.string "much in my life.\p"
	.string "Since I am your elder, let me\n"
	.string "show you what I mean.\p"
	.string "I have been with POKéMON since\n"
	.string "before you were born.\p"
	.string "I do not lose easily.\p"
	.string "I, PRYCE--the winter trainer--\n"
	.string "shall demonstrate my power!$"

Johto_MahoganyGym_Text_83DE885::
	.string "Ah, I am impressed by your\n"
	.string "prowess.\p"
	.string "With your strong will, I know you\n"
	.string "will overcome all life’s obstacles.\p"
	.string "You are worthy of this BADGE!$"

Johto_MahoganyGym_Text_83DE90D::
	.string "{PLAYER} received GLACIERBADGE.$"

Johto_MahoganyGym_Text_83DE927::
	.string "That BADGE will raise the SPECIAL\n"
	.string "ATTACK of POKéMON.\p"
	.string "It also lets your POKéMON use\n"
	.string "WHIRLPOOL to get across\l"
	.string "real whirlpools.\p"
	.string "And this… This is a gift from me!$"

Johto_MahoganyGym_Text_83DE9C5::
	.string "With full HP, a FOCUS SASH lets\n"
	.string "its holder survive a knockout\l"
	.string "hit with 1 HP.\p"
	.string "It is used up when activated.\p"
	.string "Endurance can decide a battle!$"

Johto_MahoganyGym_Text_83DEA4F::
	.string "When the ice and snow melt,\n"
	.string "spring arrives.\p"
	.string "You and your POKéMON will be\n"
	.string "together for many years to come.\p"
	.string "Cherish your time together!$"

Johto_MahoganyGym_Text_83DEC22::
	.string "PRYCE is a veteran who has\n"
	.string "trained POKéMON for some 50 years.\p"
	.string "He’s got a solid lineup of\n"
	.string "POKéMON to cover whatever\l"
	.string "you bring his way.\p"
	.string "Fire is a good place to start,\n"
	.string "but watch out for the rain!$"

Johto_MahoganyGym_Text_83DECE3::
	.string "PRYCE is something, but you’re\n"
	.string "something else!\p"
	.string "That was a hot battle that\n"
	.string "bridged the generation gap!$"

Johto_MahoganyGym_Text_83DED49::
	.string "I’ll freeze your POKéMON,\n"
	.string "so you can’t do a thing!$"

Johto_MahoganyGym_Text_83DED7C::
	.string "Darn. I couldn’t do a thing.$"

Johto_MahoganyGym_Text_83DED99::
	.string "I think there’s a move a\n"
	.string "POKéMON can use while it’s\l"
	.string "frozen.$"

Johto_MahoganyGym_Text_83DEDD5::
	.string "This GYM has a slippery floor.\p"
	.string "It’s fun, isn’t it?\p"
	.string "But hey--we’re not playing\n"
	.string "games here!$"

Johto_MahoganyGym_Text_83DEE2F::
	.string "Do you see how serious we are?$"

Johto_MahoganyGym_Text_83DEE4E::
	.string "This GYM is great.\n"
	.string "I love boarding with my\l"
	.string "POKéMON!$"

Johto_MahoganyGym_Text_83DEE82::
	.string "I know PRYCE’s secret.$"

Johto_MahoganyGym_Text_83DEE99::
	.string "OK. I’ll tell you PRYCE’s secret.$"

Johto_MahoganyGym_Text_83DEEBB::
	.string "The secret behind PRYCE’s power…\p"
	.string "He meditates under a waterfall\n"
	.string "daily to strengthen his mind\l"
	.string "and body.$"

Johto_MahoganyGym_Text_83DEF22::
	.string "To get to PRYCE, our GYM LEADER,\n"
	.string "you need to think before you skate.$"

Johto_MahoganyGym_Text_83DEF67::
	.string "I wouldn’t lose to you in skiing!$"

Johto_MahoganyGym_Text_83DEF89::
	.string "If you don’t skate with precision,\n"
	.string "you won’t get far in this GYM.$"

Johto_MahoganyGym_Text_83DEFCB::
	.string "Check out my parallel turn!$"

Johto_MahoganyGym_Text_83DEFE7::
	.string "No! You made me wipe out!$"

Johto_MahoganyGym_Text_83DF001::
	.string "I shouldn’t have been bragging\n"
	.string "about my skiing…$"
