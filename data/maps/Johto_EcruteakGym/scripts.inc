Johto_EcruteakGym_MapScripts::
	.byte 0

@ Johto_EcruteakGym object 1 (local id 1, gfx 0x3E98) at 12,44
Johto_EcruteakGym_EventScript_83CDC13::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AB, Johto_EcruteakGym_EventScript_83CDC29
	msgbox Johto_EcruteakGym_Text_83CE4C9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakGym_EventScript_83CDC29::
	msgbox Johto_EcruteakGym_Text_83CE5A3, MSGBOX_DEFAULT
	release
	end

@ Johto_EcruteakGym object 2 (local id 2, gfx 0x3EB2) at 13,33
Johto_EcruteakGym_EventScript_83CDC33::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4132, 0, Johto_EcruteakGym_Text_83CE297, Johto_EcruteakGym_Text_83CE315
	msgbox Johto_EcruteakGym_Text_83CE33C, MSGBOX_AUTOCLOSE
	end

@ Johto_EcruteakGym object 4 (local id 4, gfx 0x3EB2) at 7,25
Johto_EcruteakGym_EventScript_83CDC4A::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4364, 0, Johto_EcruteakGym_Text_83CE359, Johto_EcruteakGym_Text_83CE384
	msgbox Johto_EcruteakGym_Text_83CE393, MSGBOX_AUTOCLOSE
	end

@ Johto_EcruteakGym object 3 (local id 3, gfx 0x3E8B) at 13,25
Johto_EcruteakGym_EventScript_83CDC61::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4569, 0, Johto_EcruteakGym_Text_83CE3D4, Johto_EcruteakGym_Text_83CE3E1
	msgbox Johto_EcruteakGym_Text_83CE3F3, MSGBOX_AUTOCLOSE
	end

@ Johto_EcruteakGym object 5 (local id 5, gfx 0x3E8B) at 5,14
Johto_EcruteakGym_EventScript_83CDC78::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4546, 0, Johto_EcruteakGym_Text_83CE42A, Johto_EcruteakGym_Text_83CE467
	msgbox Johto_EcruteakGym_Text_83CE471, MSGBOX_AUTOCLOSE
	end

@ Johto_EcruteakGym object 6 (local id 6, gfx 0x3EA3) at 4,4
Johto_EcruteakGym_EventScript_83CDC8F::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x11AB, Johto_EcruteakGym_EventScript_83CDD26
	msgbox Johto_EcruteakGym_Text_83CDDE6, MSGBOX_DEFAULT
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_EcruteakGym_EventScript_83CDCC1
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4704, 0, Johto_EcruteakGym_Text_83CDF82
	goto Johto_EcruteakGym_EventScript_83CDCCB

Johto_EcruteakGym_EventScript_83CDCC1::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4966, 0, Johto_EcruteakGym_Text_83CDF82

Johto_EcruteakGym_EventScript_83CDCCB::
	message Johto_EcruteakGym_Text_83CDFBD
	waitmessage
	call Quetzal_EventScript_83602D4
	msgbox Johto_EcruteakGym_Text_83CDFD7, MSGBOX_DEFAULT
	call_if_unset FLAG_QZ_0x11B5, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x11B5
	setflag FLAG_QZ_0x11AB
	addvar VAR_QZ_0x502E, 1
	setvar VAR_0x8008, 4
	call Quetzal_EventScript_83601F0
	setvar VAR_QZ_0x500B, 4

Johto_EcruteakGym_EventScript_83CDD01::
	setorcopyvar VAR_0x8000, ITEM_LIFE_ORB
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1496
	msgbox Johto_EcruteakGym_Text_83CE06F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakGym_EventScript_83CDD26::
	goto_if_unset FLAG_QZ_0x1496, Johto_EcruteakGym_EventScript_83CDD01
	specialvar VAR_RESULT, QzUseAltGymLeaderTeam
	goto_if_eq VAR_RESULT, 1, Johto_EcruteakGym_EventScript_83CDD56
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4704, 0, Johto_EcruteakGym_Text_83CDDE6, Johto_EcruteakGym_Text_83CDF82, Johto_EcruteakGym_EventScript_83CDD68
	goto Johto_EcruteakGym_EventScript_83CDD68

Johto_EcruteakGym_EventScript_83CDD56::
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_4966, 0, Johto_EcruteakGym_Text_83CDDE6, Johto_EcruteakGym_Text_83CDF82, Johto_EcruteakGym_EventScript_83CDD68

Johto_EcruteakGym_EventScript_83CDD68::
	msgbox Johto_EcruteakGym_Text_83CE0D2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakGym sign / bg event at 7,44
@ Johto_EcruteakGym sign / bg event at 13,44
Johto_EcruteakGym_EventScript_83CDD73::
	lock
	goto_if_set FLAG_QZ_0x11AB, Johto_EcruteakGym_EventScript_83CDD88
	msgbox Johto_EcruteakGym_Text_83CDD93, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakGym_EventScript_83CDD88::
	msgbox Johto_EcruteakGym_Text_83CDDAD, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakGym_Text_83CDD93::
	.string "ECRUTEAK CITY POKéMON GYM$"

Johto_EcruteakGym_Text_83CDDAD::
	.string "ECRUTEAK CITY POKéMON GYM\p"
	.string "MORTY’S CERTIFIED TRAINERS:\n"
	.string "{PLAYER}$"

Johto_EcruteakGym_Text_83CDDE6::
	.string "Good of you to have come.\p"
	.string "Here in ECRUTEAK, POKéMON have\n"
	.string "always been revered.\p"
	.string "It’s said that a rainbow-colored\n"
	.string "POKéMON will come down to\l"
	.string "appear before a true Trainer.\p"
	.string "I believed that tale, so I’ve\n"
	.string "trained here in secret.\p"
	.string "As a result, I can now see what\n"
	.string "others cannot.\p"
	.string "Just a bit more...\p"
	.string "With a little more, I could see\n"
	.string "a future where I meet the\l"
	.string "rainbow-colored POKéMON.\p"
	.string "You’re going to help me reach\n"
	.string "that level!$"

Johto_EcruteakGym_Text_83CDF82::
	.string "I’m not good enough yet...\p"
	.string "All right. This BADGE is yours.$"

Johto_EcruteakGym_Text_83CDFBD::
	.string "{PLAYER} received the FOGBADGE.$"

Johto_EcruteakGym_Text_83CDFD7::
	.string "With the FOGBADGE, POKéMON up\n"
	.string "to Level 50 will obey you.\p"
	.string "Also, any POKéMON that knows\n"
	.string "SURF will be able to use it\l"
	.string "anytime.\p"
	.string "I want you to have this too.$"

Johto_EcruteakGym_Text_83CE06F::
	.string "A LIFE ORB boosts attack damage,\n"
	.string "but costs HP when damage is dealt.\p"
	.string "Power always comes at a price.$"

Johto_EcruteakGym_Text_83CE0D2::
	.string "I see...\p"
	.string "Your journey has taken you to\n"
	.string "far-off places.\p"
	.string "You’ve witnessed more than I\n"
	.string "ever have.\p"
	.string "I envy you for that...$"

Johto_EcruteakGym_Text_83CE297::
	.string "I spent the spring with my\n"
	.string "POKéMON.\p"
	.string "Then summer, fall, and winter...\p"
	.string "Then spring came again.\n"
	.string "We’ve been together a long time.$"

Johto_EcruteakGym_Text_83CE315::
	.string "Wins and losses--I’ve known\n"
	.string "them both.$"

Johto_EcruteakGym_Text_83CE33C::
	.string "Where did POKéMON come from?$"

Johto_EcruteakGym_Text_83CE359::
	.string "Can you inflict any damage\n"
	.string "on our POKéMON?$"

Johto_EcruteakGym_Text_83CE384::
	.string "Ah! Well done!$"

Johto_EcruteakGym_Text_83CE393::
	.string "We use GHOST-type POKéMON.\p"
	.string "No NORMAL-type attacks can\n"
	.string "harm them!$"

Johto_EcruteakGym_Text_83CE3D4::
	.string "I shall win!$"

Johto_EcruteakGym_Text_83CE3E1::
	.string "I... I... I lost!$"

Johto_EcruteakGym_Text_83CE3F3::
	.string "The one who wants to win the\n"
	.string "most is the one who will!$"

Johto_EcruteakGym_Text_83CE42A::
	.string "Stumped by our dark corridors?\n"
	.string "Defeat me if you want a hint!$"

Johto_EcruteakGym_Text_83CE467::
	.string "Wha-what?$"

Johto_EcruteakGym_Text_83CE471::
	.string "Fine. I’ll tell you the secret\n"
	.string "of the shadows.\p"
	.string "The path is right in front of\n"
	.string "your eyes!$"

Johto_EcruteakGym_Text_83CE4C9::
	.string "Hey, CHAMP! You’re almost\n"
	.string "halfway there!\p"
	.string "Everyone here uses GHOST-type\n"
	.string "POKéMON.\p"
	.string "NORMAL- and FIGHTING-type hits\n"
	.string "usually can’t hurt them.\p"
	.string "Watch out for MORTY’s illusions\n"
	.string "and his powerful GENGAR!\p"
	.string "Don’t trust appearances!$"

Johto_EcruteakGym_Text_83CE5A3::
	.string "Whew, {PLAYER}. You did great!\p"
	.string "I was cowering in the corner\n"
	.string "out of pure terror!$"
