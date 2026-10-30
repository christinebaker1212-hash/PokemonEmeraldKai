Johto_GoldenrodGym_MapScripts::
	.byte 0

@ Johto_GoldenrodGym object 2 (local id 2, gfx 0x10) at 7,7
Johto_GoldenrodGym_EventScript_83D584A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4598, 0, Johto_GoldenrodGym_Text_83D5D91, Johto_GoldenrodGym_Text_83D5DD0
	msgbox Johto_GoldenrodGym_Text_83D5DEF, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodGym object 6 (local id 6, gfx 0x8) at 16,11
Johto_GoldenrodGym_EventScript_83D5861::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4225, 0, Johto_GoldenrodGym_Text_83D5E0D, Johto_GoldenrodGym_Text_83D5E61
	msgbox Johto_GoldenrodGym_Text_83D5E71, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodGym object 3 (local id 3, gfx 0x14) at 15,18
Johto_GoldenrodGym_EventScript_83D5878::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4395, 0, Johto_GoldenrodGym_Text_83D5F32, Johto_GoldenrodGym_Text_83D5F78
	msgbox Johto_GoldenrodGym_Text_83D5F94, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodGym object 5 (local id 5, gfx 0x2D) at 26,10
Johto_GoldenrodGym_EventScript_83D588F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4341, 0, Johto_GoldenrodGym_Text_83D5FC2, Johto_GoldenrodGym_Text_83D5FF1
	msgbox Johto_GoldenrodGym_Text_83D600F, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodGym object 1 (local id 1, gfx 0x3E98) at 10,20
Johto_GoldenrodGym_EventScript_83D58A6::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11B4, Johto_GoldenrodGym_EventScript_83D58BC
	msgbox Johto_GoldenrodGym_Text_83D603D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodGym_EventScript_83D58BC::
	msgbox Johto_GoldenrodGym_Text_83D6139, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGym object 4 (local id 4, gfx 0x3EBC) at 15,8
Johto_GoldenrodGym_EventScript_83D58C7::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AA, Johto_GoldenrodGym_EventScript_83D599C
	goto_if_eq VAR_QZ_0x500A, 3, Johto_GoldenrodGym_EventScript_83D5929
	goto_if_eq VAR_QZ_0x500A, 4, Johto_GoldenrodGym_EventScript_83D5936
	msgbox Johto_GoldenrodGym_Text_83D5A7B, MSGBOX_DEFAULT
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodGym_EventScript_83D590F
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4700, 0, Johto_GoldenrodGym_Text_83D5B04
	goto Johto_GoldenrodGym_EventScript_83D5919

Johto_GoldenrodGym_EventScript_83D590F::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4965, 0, Johto_GoldenrodGym_Text_83D5B04

Johto_GoldenrodGym_EventScript_83D5919::
	msgbox Johto_GoldenrodGym_Text_83D5B54, MSGBOX_DEFAULT
	setvar VAR_QZ_0x500A, 3
	closemessage
	release
	end

Johto_GoldenrodGym_EventScript_83D5929::
	lock
	faceplayer
	msgbox Johto_GoldenrodGym_Text_83D5B54, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodGym_EventScript_83D5936::
	lock
	faceplayer
	msgbox Johto_GoldenrodGym_Text_83D5B81, MSGBOX_DEFAULT
	closemessage
	message Johto_GoldenrodGym_Text_83D5BD2
	waitmessage
	call Quetzal_EventScript_83602D4
	msgbox Johto_GoldenrodGym_Text_83D5BEA, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x11B4, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x11B4
	setflag FLAG_QZ_0x11AA
	addvar VAR_QZ_0x502E, 1
	setvar VAR_0x8008, 3
	call Quetzal_EventScript_83601F0
	setvar VAR_QZ_0x500A, 5

Johto_GoldenrodGym_EventScript_83D5977::
	setorcopyvar VAR_0x8000, ITEM_CHOICE_SCARF
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1495
	msgbox Johto_GoldenrodGym_Text_83D5C69, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodGym_EventScript_83D599C::
	lock
	faceplayer
	goto_if_unset FLAG_QZ_0x1495, Johto_GoldenrodGym_EventScript_83D5977
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_GoldenrodGym_EventScript_83D59CE
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4700, 0, Johto_GoldenrodGym_Text_83D5A7B, Johto_GoldenrodGym_Text_83D5B04, Johto_GoldenrodGym_EventScript_83D59E0
	goto Johto_GoldenrodGym_EventScript_83D59E0

Johto_GoldenrodGym_EventScript_83D59CE::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4965, 0, Johto_GoldenrodGym_Text_83D5A7B, Johto_GoldenrodGym_Text_83D5B04, Johto_GoldenrodGym_EventScript_83D59E0

Johto_GoldenrodGym_EventScript_83D59E0::
	msgbox Johto_GoldenrodGym_Text_83D5CCA, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodGym trigger at 15,11 (VAR_QZ_0x500A == 3)
@ Johto_GoldenrodGym trigger at 15,11 (VAR_QZ_0x500A == 4)
Johto_GoldenrodGym_EventScript_83D59EB::
	turnobject 6, 3
	msgbox Johto_GoldenrodGym_Text_83D5ED8, MSGBOX_DEFAULT
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360837
	waitmovement 0
	setvar VAR_QZ_0x500A, 4
	closemessage
	release
	end

@ Johto_GoldenrodGym sign / bg event at 11,20
@ Johto_GoldenrodGym sign / bg event at 7,20
Johto_GoldenrodGym_EventScript_83D5A09::
	lock
	goto_if_set FLAG_QZ_0x11AA, Johto_GoldenrodGym_EventScript_83D5A1E
	msgbox Johto_GoldenrodGym_Text_83D5A29, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodGym_EventScript_83D5A1E::
	msgbox Johto_GoldenrodGym_Text_83D5A3F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodGym_Text_83D5A29::
	.string "GOLDENROD POKéMON GYM$"

Johto_GoldenrodGym_Text_83D5A3F::
	.string "GOLDENROD TOWN POKéMON GYM\p"
	.string "WHITNEY’S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

Johto_GoldenrodGym_Text_83D5A7B::
	.string "Hi! I’m WHITNEY!\p"
	.string "Everyone was into POKéMON, so I got\n"
	.string "into it too!\p"
	.string "POKéMON are super-cute!\p"
	.string "You want to battle? I’m warning\n"
	.string "you- I’m good!$"

Johto_GoldenrodGym_Text_83D5B04::
	.string "Sob…\p"
	.string "…Waaaaaaah! You’re mean!\p"
	.string "You shouldn’t be so serious! You…\n"
	.string "you child, you!$"

Johto_GoldenrodGym_Text_83D5B54::
	.string "Waaaaah!\p"
	.string "Waaaaah!\p"
	.string "…Snivel, hic… …You meanie!$"

Johto_GoldenrodGym_Text_83D5B81::
	.string "…Sniff…\p"
	.string "What? What do you want? A BADGE?\p"
	.string "Oh, right. I forgot. Here’s\n"
	.string "PLAINBADGE.$"

Johto_GoldenrodGym_Text_83D5BD2::
	.string "{PLAYER} received PLAINBADGE.$"

Johto_GoldenrodGym_Text_83D5BEA::
	.string "PLAINBADGE lets your POKéMON use\n"
	.string "STRENGTH outside of battle.\p"
	.string "It also boosts your POKéMON’s\n"
	.string "DEFENSE.\p"
	.string "Oh, you can have this too!$"

Johto_GoldenrodGym_Text_83D5C69::
	.string "A CHOICE SCARF boosts Speed,\n"
	.string "but only lets its holder\l"
	.string "use one move.\p"
	.string "My MILTANK would be so fast!$"

Johto_GoldenrodGym_Text_83D5CCA::
	.string "Ah, that was a good cry!$"

Johto_GoldenrodGym_Text_83D5D91::
	.string "Don’t let my POKéMON’s cute looks\n"
	.string "fool you.\p"
	.string "They can whip you!$"

Johto_GoldenrodGym_Text_83D5DD0::
	.string "Darn… I thought you were weak…$"

Johto_GoldenrodGym_Text_83D5DEF::
	.string "Do my POKéMON think I’m cute?$"

Johto_GoldenrodGym_Text_83D5E0D::
	.string "I like cute POKéMON better than\n"
	.string "strong POKéMON.\p"
	.string "But I have strong and cute POKéMON!$"

Johto_GoldenrodGym_Text_83D5E61::
	.string "Oh, no, no, no!$"

Johto_GoldenrodGym_Text_83D5E71::
	.string "I’m trying to beat WHITNEY, but…\n"
	.string "It’s depressing.\p"
	.string "I’m okay! If I lose, I’ll just try\n"
	.string "harder next time!$"

Johto_GoldenrodGym_Text_83D5ED8::
	.string "Oh, no. You made WHITNEY cry.\p"
	.string "It’s OK. She’ll stop soon. She\n"
	.string "always cries when she loses.$"

Johto_GoldenrodGym_Text_83D5F32::
	.string "Oh, you are a cute little trainer!\p"
	.string "I like you, but I won’t hold back!$"

Johto_GoldenrodGym_Text_83D5F78::
	.string "Let’s see… Oops, it’s over?$"

Johto_GoldenrodGym_Text_83D5F94::
	.string "Wow, you must be good to beat me!\n"
	.string "Keep it up!$"

Johto_GoldenrodGym_Text_83D5FC2::
	.string "Give it your best shot, or I’ll take\n"
	.string "you down!$"

Johto_GoldenrodGym_Text_83D5FF1::
	.string "No! Oh, MEOWTH, I’m so sorry!$"

Johto_GoldenrodGym_Text_83D600F::
	.string "I taught MEOWTH moves for taking on\n"
	.string "any type…$"

Johto_GoldenrodGym_Text_83D603D::
	.string "Yo! CHAMP in making!\p"
	.string "This GYM is home to WHITNEY, the\n"
	.string "master of NORMAL-type POKéMON.\p"
	.string "Be cautious of her cute team.\p"
	.string "Some of the POKéMON in this gym\n"
	.string "have the newly discovered FAIRY\l"
	.string "type as well.\p"
	.string "Check ROUTE 34 if you need a\n"
	.string "FIGHTING-type or POISON-type.$"

Johto_GoldenrodGym_Text_83D6139::
	.string "You won? Great! I was busy admiring\n"
	.string "the ladies here.$"
