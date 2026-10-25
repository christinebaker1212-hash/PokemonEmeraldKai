Johto_OlivineGym_MapScripts::
	.byte 0

@ Johto_OlivineGym object 3 (local id 3, gfx 0x30) at 11,14
Johto_OlivineGym_EventScript_83E4A0B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AD, Johto_OlivineGym_EventScript_83E4A21
	msgbox Johto_OlivineGym_Text_83E4AFC, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGym_EventScript_83E4A21::
	msgbox Johto_OlivineGym_Text_83E4B66, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineGym trigger at 6,14 (VAR_QZ_0x500D == 5)
@ Johto_OlivineGym trigger at 7,14 (VAR_QZ_0x500D == 5)
@ Johto_OlivineGym trigger at 8,14 (VAR_QZ_0x500D == 5)
@ Johto_OlivineGym trigger at 9,14 (VAR_QZ_0x500D == 5)
@ Johto_OlivineGym trigger at 10,14 (VAR_QZ_0x500D == 5)
Johto_OlivineGym_EventScript_83E4A2C::
	lock
	setvar VAR_TEMP_0, 3
	playse SE_PIN
	applymovement VAR_TEMP_0, Quetzal_Movement_83607F9
	delay 0x3C
	getplayerxy VAR_TEMP_1, VAR_TEMP_2
	call_if_eq VAR_TEMP_1, 6, Johto_OlivineGym_EventScript_83E4AD3
	call_if_eq VAR_TEMP_1, 7, Johto_OlivineGym_EventScript_83E4AB4
	call_if_eq VAR_TEMP_1, 8, Johto_OlivineGym_EventScript_83E4A9F
	call_if_eq VAR_TEMP_1, 9, Johto_OlivineGym_EventScript_83E4A94
	call_if_eq VAR_TEMP_1, 10, Johto_OlivineGym_EventScript_83E4A8F
	turnobject LOCALID_PLAYER, 4
	msgbox Johto_OlivineGym_Text_83E4AFC, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x500D, 6
	release
	end

Johto_OlivineGym_EventScript_83E4A8F::
	turnobject VAR_TEMP_0, 3
	return

Johto_OlivineGym_EventScript_83E4A94::
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	return

Johto_OlivineGym_EventScript_83E4A9F::
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	return

Johto_OlivineGym_EventScript_83E4AB4::
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	return

Johto_OlivineGym_EventScript_83E4AD3::
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083D
	waitmovement 0
	return

@ Johto_OlivineGym object 2 (local id 2, gfx 0x3E8F) at 5,10
Johto_OlivineGym_EventScript_83E4BEA::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AD, Johto_OlivineGym_EventScript_83E4C00
	msgbox Johto_OlivineGym_Text_83E4CDB, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGym_EventScript_83E4C00::
	msgbox Johto_OlivineGym_Text_83E4D57, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineGym trigger at 6,10 (VAR_QZ_0x500D == 6)
@ Johto_OlivineGym trigger at 7,10 (VAR_QZ_0x500D == 6)
@ Johto_OlivineGym trigger at 8,10 (VAR_QZ_0x500D == 6)
@ Johto_OlivineGym trigger at 9,10 (VAR_QZ_0x500D == 6)
@ Johto_OlivineGym trigger at 10,10 (VAR_QZ_0x500D == 6)
Johto_OlivineGym_EventScript_83E4C0B::
	lock
	setvar VAR_TEMP_0, 2
	playse SE_PIN
	applymovement VAR_TEMP_0, Quetzal_Movement_83607F9
	delay 0x3C
	getplayerxy VAR_TEMP_1, VAR_TEMP_2
	call_if_eq VAR_TEMP_1, 6, Johto_OlivineGym_EventScript_83E4C6E
	call_if_eq VAR_TEMP_1, 7, Johto_OlivineGym_EventScript_83E4C73
	call_if_eq VAR_TEMP_1, 8, Johto_OlivineGym_EventScript_83E4C7E
	call_if_eq VAR_TEMP_1, 9, Johto_OlivineGym_EventScript_83E4C93
	call_if_eq VAR_TEMP_1, 10, Johto_OlivineGym_EventScript_83E4CB2
	turnobject LOCALID_PLAYER, 3
	msgbox Johto_OlivineGym_Text_83E4CDB, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x500D, 7
	release
	end

Johto_OlivineGym_EventScript_83E4C6E::
	turnobject VAR_TEMP_0, 4
	return

Johto_OlivineGym_EventScript_83E4C73::
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	return

Johto_OlivineGym_EventScript_83E4C7E::
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	return

Johto_OlivineGym_EventScript_83E4C93::
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	return

Johto_OlivineGym_EventScript_83E4CB2::
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	applymovement VAR_TEMP_0, Quetzal_Movement_836083B
	waitmovement 0
	return

@ Johto_OlivineGym object 4 (local id 4, gfx 0x3E98) at 10,18
Johto_OlivineGym_EventScript_83E4DC5::
	lock
	faceplayer
	goto_if_lt VAR_QZ_0x500D, 5, Johto_OlivineGym_EventScript_83E4DE6
	goto_if_set FLAG_QZ_0x11AD, Johto_OlivineGym_EventScript_83E4DF1
	msgbox Johto_OlivineGym_Text_83E4DFC, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGym_EventScript_83E4DE6::
	msgbox Johto_OlivineGym_Text_83E4FCD, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGym_EventScript_83E4DF1::
	msgbox Johto_OlivineGym_Text_83E4F77, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineGym object 1 (local id 1, gfx 0x3E9A) at 8,4
Johto_OlivineGym_EventScript_83E504C::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AD, Johto_OlivineGym_EventScript_83E50F2
	msgbox Johto_OlivineGym_Text_83E513F, MSGBOX_DEFAULT
	goto Johto_OlivineGym_EventScript_83E50C4

Johto_OlivineGym_EventScript_83E5064::
	message Johto_OlivineGym_Text_83E52BB
	waitmessage
	call Quetzal_EventScript_83602D4
	msgbox Johto_OlivineGym_Text_83E52D5, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x11B7, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x11B7
	setflag FLAG_QZ_0x11AD
	addvar VAR_QZ_0x502E, 1
	call Quetzal_EventScript_83DF68E
	setvar VAR_0x8008, 6
	call Quetzal_EventScript_83601F0
	setvar VAR_QZ_0x500D, 8

Johto_OlivineGym_EventScript_83E509F::
	setorcopyvar VAR_0x8000, 0x1F0
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1497
	msgbox Johto_OlivineGym_Text_83E5320, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGym_EventScript_83E50C4::
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_OlivineGym_EventScript_83E50E3
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4747, 0, Johto_OlivineGym_Text_83E523D
	goto Johto_OlivineGym_EventScript_83E5064

Johto_OlivineGym_EventScript_83E50E3::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4971, 0, Johto_OlivineGym_Text_83E523D
	goto Johto_OlivineGym_EventScript_83E5064

Johto_OlivineGym_EventScript_83E50F2::
	goto_if_unset FLAG_QZ_0x1497, Johto_OlivineGym_EventScript_83E509F
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam_670
	goto_if_eq VAR_RESULT, 1, Johto_OlivineGym_EventScript_83E5122
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4747, 0, Johto_OlivineGym_Text_83E513F, Johto_OlivineGym_Text_83E523D, Johto_OlivineGym_EventScript_83E5134
	goto Johto_OlivineGym_EventScript_83E5134

Johto_OlivineGym_EventScript_83E5122::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4971, 0, Johto_OlivineGym_Text_83E513F, Johto_OlivineGym_Text_83E523D, Johto_OlivineGym_EventScript_83E5134

Johto_OlivineGym_EventScript_83E5134::
	msgbox Johto_OlivineGym_Text_83E5398, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineGym sign / bg event at 11,18
@ Johto_OlivineGym sign / bg event at 5,18
Johto_OlivineGym_EventScript_83E549F::
	lock
	goto_if_set FLAG_QZ_0x11AD, Johto_OlivineGym_EventScript_83E54B4
	msgbox Johto_OlivineGym_Text_83E54BF, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGym_EventScript_83E54B4::
	msgbox Johto_OlivineGym_Text_83E54D8, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineGym_Text_83E4AFC::
	.string "I knew you’d come here.\p"
	.string "Thank you for helping the GYM LEADER,\n"
	.string "but battle is a separate matter.\p"
	.string "Go for it!$"

Johto_OlivineGym_Text_83E4B66::
	.string "Impressive!\n"
	.string "You should continue your journey to\l"
	.string "different places.\p"
	.string "Since you can use SURF, why don’t you\n"
	.string "try it on the LAKE OF RAGE?$"

Johto_OlivineGym_Text_83E4CDB::
	.string "Giggle...\n"
	.string "I know how capable you are\l"
	.string "from the LIGHTHOUSE.\p"
	.string "It’s time you showed that side of\n"
	.string "yourself to JASMINE! Good luck!$"

Johto_OlivineGym_Text_83E4D57::
	.string "You are amazing...\n"
	.string "You should aim even higher, since\l"
	.string "you have such potential.\p"
	.string "Have you been to MAHOGANY TOWN?$"

Johto_OlivineGym_Text_83E4DFC::
	.string "JASMINE uses the newly discovered\n"
	.string "STEEL-type.\p"
	.string "She’s known for having not one,\n"
	.string "but two aces.\p"
	.string "The first is a metal bug that can\n"
	.string "use a technique to boost its ATTACK.\p"
	.string "While fire will hurt it, it can\n"
	.string "sometimes endure powerful blows.\p"
	.string "The second is a giant iron snake\n"
	.string "with lots of powerful moves.\p"
	.string "Approach it with caution.\p"
	.string "Its high DEFENSE lets it take hits\n"
	.string "and strike back with force.$"

Johto_OlivineGym_Text_83E4F77::
	.string "That was awesome.\p"
	.string "The STEEL-type, huh?\p"
	.string "That was a close encounter of an\n"
	.string "unknown kind!$"

Johto_OlivineGym_Text_83E4FCD::
	.string "JASMINE, the GYM LEADER, is at the\n"
	.string "LIGHTHOUSE.\p"
	.string "She’s been tending to a sick POKéMON.\p"
	.string "A strong trainer has to be\n"
	.string "compassionate.$"

Johto_OlivineGym_Text_83E513F::
	.string "…Thank you for your help at the\n"
	.string "LIGHTHOUSE…\p"
	.string "But this is different.\n"
	.string "Please allow me to introduce myself.\p"
	.string "I am JASMINE, a GYM LEADER.\n"
	.string "I use the STEEL-type.\p"
	.string "…Do you know about the STEEL-type?\p"
	.string "It’s a type that was only recently\n"
	.string "discovered.\p"
	.string "…Um… May I begin?$"

Johto_OlivineGym_Text_83E523D::
	.string "…You are a better trainer than me,\n"
	.string "in both skill and kindness.\p"
	.string "In accordance with LEAGUE rules,\n"
	.string "I confer upon you this BADGE.$"

Johto_OlivineGym_Text_83E52BB::
	.string "{PLAYER} received MINERALBADGE.$"

Johto_OlivineGym_Text_83E52D5::
	.string "MINERALBADGE raises POKéMON’s \n"
	.string "SPECIAL DEFENSE.\p"
	.string "…Um… Please take this too…$"

Johto_OlivineGym_Text_83E5320::
	.string "A ROCKY HELMET hurts attackers\n"
	.string "that make contact with its holder.\p"
	.string "…Sometimes, taking a hit\n"
	.string "can be a way to strike back.$"

Johto_OlivineGym_Text_83E5398::
	.string "Um… I don’t know how to say this,\n"
	.string "but good luck…$"

Johto_OlivineGym_Text_83E54BF::
	.string "OLIVINE CITY POKéMON GYM$"

Johto_OlivineGym_Text_83E54D8::
	.string "OLIVINE CITY POKéMON GYM\p"
	.string "JASMINE’S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"
