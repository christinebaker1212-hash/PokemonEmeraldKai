Kanto_PewterCity_Gym_MapScripts::
	.byte 0

@ Kanto_PewterCity_Gym object 1 (local id 1, gfx 0x3EEF) at 6,5
Kanto_PewterCity_Gym_EventScript_841A584::
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 2
	special QzQuestLogNop
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_QZ_8606, 0, Kanto_PewterCity_Gym_Text_84410AB, Kanto_PewterCity_Gym_Text_8441226, Kanto_PewterCity_Gym_EventScript_841A5B6

Kanto_PewterCity_Gym_EventScript_841A5A3::
	goto_if_unset FLAG_QZ_0x2173, Kanto_PewterCity_Gym_EventScript_841A5F3
	msgbox Kanto_PewterCity_Gym_Text_8441411, MSGBOX_DEFAULT
	release
	end

Kanto_PewterCity_Gym_EventScript_841A5B6::
	goto_if_set FLAG_QZ_0x2100, Kanto_PewterCity_Gym_EventScript_841A5A3
	setvar VAR_0x8004, 2
	setvar VAR_0x8005, 1
	special QzQuestLogNop
	setflag FLAG_QZ_0x211B
	call_if_unset FLAG_QZ_0x2100, Quetzal_EventScript_835FF3F
	setflag FLAG_QZ_0x2100
	setvar VAR_QZ_0x511D, 2
	setflag FLAG_QZ_0x21A5
	setvar VAR_0x8008, 1
	call Quetzal_EventScript_8456FD2
	goto Kanto_PewterCity_Gym_EventScript_841A5F3

Kanto_PewterCity_Gym_EventScript_841A5F3::
	msgbox Kanto_PewterCity_Gym_Text_8441392, MSGBOX_DEFAULT
	checkitemspace ITEM_LEFTOVERS, 1
	goto_if_eq VAR_RESULT, 0, Kanto_PewterCity_Gym_EventScript_841A63B
	bufferitemname 0, ITEM_LEFTOVERS
	additem ITEM_LEFTOVERS, 1
	loadword 0, Kanto_PewterCity_Gym_Text_84413AC
	setorcopyvar VAR_0x8000, 0x1F7
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	setflag FLAG_QZ_0x2173
	msgbox Kanto_PewterCity_Gym_Text_84413BC, MSGBOX_DEFAULT
	release
	end

Kanto_PewterCity_Gym_EventScript_841A63B::
	msgbox Kanto_PewterCity_Gym_Text_84414D0, MSGBOX_DEFAULT
	release
	end

@ Kanto_PewterCity_Gym object 2 (local id 2, gfx 0x3EF3) at 3,8
Kanto_PewterCity_Gym_EventScript_841A645::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_8334, 0, Kanto_PewterCity_Gym_Text_84414EE, Kanto_PewterCity_Gym_Text_8441539
	msgbox Kanto_PewterCity_Gym_Text_844156D, MSGBOX_AUTOCLOSE
	end

@ Kanto_PewterCity_Gym object 3 (local id 3, gfx 0x3F0B) at 7,12
Kanto_PewterCity_Gym_EventScript_841A65C::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x211B, Kanto_PewterCity_Gym_EventScript_841A686
	msgbox Kanto_PewterCity_Gym_Text_844159A, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Kanto_PewterCity_Gym_EventScript_841A690
	goto_if_eq VAR_RESULT, 0, Kanto_PewterCity_Gym_EventScript_841A69E
	end

Kanto_PewterCity_Gym_EventScript_841A686::
	msgbox Kanto_PewterCity_Gym_Text_844173E, MSGBOX_DEFAULT
	release
	end

Kanto_PewterCity_Gym_EventScript_841A690::
	msgbox Kanto_PewterCity_Gym_Text_844163E, MSGBOX_DEFAULT
	goto Kanto_PewterCity_Gym_EventScript_841A6AC

Kanto_PewterCity_Gym_EventScript_841A69E::
	msgbox Kanto_PewterCity_Gym_Text_8441714, MSGBOX_DEFAULT
	goto Kanto_PewterCity_Gym_EventScript_841A6AC

Kanto_PewterCity_Gym_EventScript_841A6AC::
	msgbox Kanto_PewterCity_Gym_Text_844165E, MSGBOX_DEFAULT
	release
	end

@ Kanto_PewterCity_Gym sign / bg event at 4,12
@ Kanto_PewterCity_Gym sign / bg event at 8,12
Kanto_PewterCity_Gym_EventScript_841A6B6::
	lockall
	goto_if_set FLAG_QZ_0x2100, Kanto_PewterCity_Gym_EventScript_841A6CA
	msgbox Kanto_PewterCity_Gym_Text_8441770, MSGBOX_DEFAULT
	releaseall
	end

Kanto_PewterCity_Gym_EventScript_841A6CA::
	msgbox Kanto_PewterCity_Gym_Text_84417A8, MSGBOX_DEFAULT
	releaseall
	end

Kanto_PewterCity_Gym_Text_84410AB::
	.string "So, you’re here. I’m BROCK.\n"
	.string "I’m PEWTER’s GYM LEADER.\p"
	.string "My rock-hard willpower is evident\n"
	.string "even in my POKéMON.\p"
	.string "My POKéMON are all rock hard, and\n"
	.string "have true-grit determination.\p"
	.string "That’s right - my POKéMON are all\n"
	.string "the ROCK type!\p"
	.string "Fuhaha! You’re going to challenge\n"
	.string "me knowing that you’ll lose?\p"
	.string "That’s the TRAINER’s honor that\n"
	.string "compels you to challenge me.\p"
	.string "Fine, then!\n"
	.string "Show me your best!{PLAY_BGM 0x022A}$"

Kanto_PewterCity_Gym_Text_8441226::
	.string "I took you for granted, and so\n"
	.string "I lost.\p"
	.string "As proof of your victory, I confer\n"
	.string "on you this…the official POKéMON\l"
	.string "LEAGUE BOULDERBADGE.\p"
	.string "{PLAYER} received the BOULDERBADGE\n"
	.string "from BROCK!{PAUSE_MUSIC}{PLAY_BGM 0x0171}{PAUSE 0xFE}{PAUSE 0x56}{RESUME_MUSIC}\p"
	.string "Just having the BOULDERBADGE makes\n"
	.string "your POKéMON more powerful.\p"
	.string "It also enables the use of the\n"
	.string "move FLASH outside of battle.\p"
	.string "Of course, a POKéMON must know the\n"
	.string "move FLASH to use it.$"

Kanto_PewterCity_Gym_Text_8441392::
	.string "Wait!\n"
	.string "Take this with you.$"

Kanto_PewterCity_Gym_Text_84413AC::
	.string "{PLAYER} received\n"
	.string "{STR_VAR_1}!$"

Kanto_PewterCity_Gym_Text_84413BC::
	.string "A POKéMON holding it gains Special\n"
	.string "Defense, but can only use moves\l"
	.string "that deal damage.$"

Kanto_PewterCity_Gym_Text_8441411::
	.string "There are all kinds of TRAINERS in\n"
	.string "this huge world of ours.\p"
	.string "You appear to be very gifted as a\n"
	.string "POKéMON TRAINER.\p"
	.string "So let me make a suggestion.\p"
	.string "Go to the GYM in CERULEAN and test\n"
	.string "your abilities.$"

Kanto_PewterCity_Gym_Text_84414D0::
	.string "You don’t have room for this.$"

Kanto_PewterCity_Gym_Text_84414EE::
	.string "Stop right there, kid!\p"
	.string "You’re ten thousand light-years \n"
	.string "from facing BROCK!$"

Kanto_PewterCity_Gym_Text_8441539::
	.string "Darn!\p"
	.string "Light-years isn’t time…\n"
	.string "It measures distance!$"

Kanto_PewterCity_Gym_Text_844156D::
	.string "You’re pretty hot.\n"
	.string "…But not as hot as BROCK!$"

Kanto_PewterCity_Gym_Text_844159A::
	.string "Hiya!\n"
	.string "Do you want to dream big?\p"
	.string "Do you dare to dream of becoming\n"
	.string "the POKéMON champ?\p"
	.string "I’m no TRAINER, but I can advise\n"
	.string "you on how to win.\p"
	.string "Let me take you to the top!$"

Kanto_PewterCity_Gym_Text_844163E::
	.string "All right!\n"
	.string "Let’s get happening!$"

Kanto_PewterCity_Gym_Text_844165E::
	.string "The first POKéMON out in a match is\n"
	.string "at the left of the POKéMON LIST.\p"
	.string "By changing the order of POKéMON,\n"
	.string "you may gain an advantage.\p"
	.string "Try different orders to suit your\n"
	.string "opponent’s party.$"

Kanto_PewterCity_Gym_Text_8441714::
	.string "It’s a free service!\n"
	.string "Let’s get happening!$"

Kanto_PewterCity_Gym_Text_844173E::
	.string "Just as I thought!\n"
	.string "You’re POKéMON champ material!$"

Kanto_PewterCity_Gym_Text_8441770::
	.string "PEWTER POKéMON GYM\n"
	.string "LEADER: BROCK\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE$"

Kanto_PewterCity_Gym_Text_84417A8::
	.string "PEWTER POKéMON GYM\n"
	.string "LEADER: BROCK\p"
	.string "WINNING TRAINERS:\n"
	.string "BLUE, {PLAYER}$"
