Kanto_ViridianCity_School_MapScripts::
	.byte 0

@ Kanto_ViridianCity_School object 2 (local id 2, gfx 0x3F16) at 4,5
Kanto_ViridianCity_School_EventScript_841A07D::
	lock
	faceplayer
	msgbox Kanto_ViridianCity_School_Text_843FF5C, MSGBOX_DEFAULT
	closemessage
	applymovement 2, Quetzal_Movement_8457AAC
	waitmovement 0
	release
	end

@ Kanto_ViridianCity_School object 1 (local id 1, gfx 0x3F58) at 6,2
Kanto_ViridianCity_School_EventScript_841A094::
	lock
	faceplayer
	msgbox Kanto_ViridianCity_School_Text_843FF87, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

@ Kanto_ViridianCity_School sign / bg event at 4,4
Kanto_ViridianCity_School_EventScript_841A0AB::
	lockall
	msgbox Kanto_ViridianCity_School_Text_843FFC1, MSGBOX_DEFAULT
	msgbox Kanto_ViridianCity_School_Text_8440275, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_ViridianCity_School_EventScript_841A11B
	msgbox Kanto_ViridianCity_School_Text_844007F, MSGBOX_DEFAULT
	msgbox Kanto_ViridianCity_School_Text_8440275, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_ViridianCity_School_EventScript_841A11B
	msgbox Kanto_ViridianCity_School_Text_8440104, MSGBOX_DEFAULT
	msgbox Kanto_ViridianCity_School_Text_8440275, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Kanto_ViridianCity_School_EventScript_841A11B
	msgbox Kanto_ViridianCity_School_Text_84401B8, MSGBOX_DEFAULT
	applymovement 2, Quetzal_Movement_8457AAC
	waitmovement 0
	textcolor 1
	msgbox Kanto_ViridianCity_School_Text_8440284, MSGBOX_DEFAULT
	releaseall
	end

Kanto_ViridianCity_School_EventScript_841A11B::
	releaseall
	end

@ Kanto_ViridianCity_School sign / bg event at 5,1
@ Kanto_ViridianCity_School sign / bg event at 4,1
Kanto_ViridianCity_School_EventScript_841A11D::
	lockall
	msgbox Kanto_ViridianCity_School_Text_84402A7, MSGBOX_DEFAULT
	goto Kanto_ViridianCity_School_EventScript_841A12C

Kanto_ViridianCity_School_EventScript_841A12C::
	message Kanto_ViridianCity_School_Text_84402E4
	waitmessage
	multichoicegrid 7, 1, MULTI_QZ_0x80, 1, 3
	nop
	copyvar VAR_0x8000, VAR_RESULT
	goto_if_eq VAR_0x8000, 0, Kanto_ViridianCity_School_EventScript_841A18C
	goto_if_eq VAR_0x8000, 1, Kanto_ViridianCity_School_EventScript_841A1A8
	goto_if_eq VAR_0x8000, 2, Kanto_ViridianCity_School_EventScript_841A1C4
	goto_if_eq VAR_0x8000, 3, Kanto_ViridianCity_School_EventScript_841A19A
	goto_if_eq VAR_0x8000, 4, Kanto_ViridianCity_School_EventScript_841A1B6
	goto_if_eq VAR_0x8000, 5, Kanto_ViridianCity_School_EventScript_841A1D2
	goto_if_eq VAR_0x8000, 127, Kanto_ViridianCity_School_EventScript_841A1D2
	end

Kanto_ViridianCity_School_EventScript_841A18C::
	msgbox Kanto_ViridianCity_School_Text_8440305, MSGBOX_DEFAULT
	goto Kanto_ViridianCity_School_EventScript_841A12C

Kanto_ViridianCity_School_EventScript_841A19A::
	msgbox Kanto_ViridianCity_School_Text_8440381, MSGBOX_DEFAULT
	goto Kanto_ViridianCity_School_EventScript_841A12C

Kanto_ViridianCity_School_EventScript_841A1A8::
	msgbox Kanto_ViridianCity_School_Text_84403FB, MSGBOX_DEFAULT
	goto Kanto_ViridianCity_School_EventScript_841A12C

Kanto_ViridianCity_School_EventScript_841A1B6::
	msgbox Kanto_ViridianCity_School_Text_844046B, MSGBOX_DEFAULT
	goto Kanto_ViridianCity_School_EventScript_841A12C

Kanto_ViridianCity_School_EventScript_841A1C4::
	msgbox Kanto_ViridianCity_School_Text_84404F5, MSGBOX_DEFAULT
	goto Kanto_ViridianCity_School_EventScript_841A12C

Kanto_ViridianCity_School_EventScript_841A1D2::
	releaseall
	end

@ Kanto_ViridianCity_School sign / bg event at 9,2
@ Kanto_ViridianCity_School sign / bg event at 8,2
Kanto_ViridianCity_School_EventScript_845D136::
	lock
	faceplayer
	setvar VAR_0x8004, 0
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 3
	special QzQuestLogNop
	textcolor 3
	msgbox Kanto_ViridianCity_School_Text_8461E90, MSGBOX_DEFAULT
	release
	end

Kanto_ViridianCity_School_Text_843FF5C::
	.string "Whew! I’m trying to memorize all my\n"
	.string "notes.$"

Kanto_ViridianCity_School_Text_843FF87::
	.string "Okay!\p"
	.string "Be sure to read what’s on the\n"
	.string "blackboard carefully!$"

Kanto_ViridianCity_School_Text_843FFC1::
	.string "Let’s check out the notebook.\p"
	.string "First page…\p"
	.string "POKé BALLS are used to catch\n"
	.string "POKéMON.\p"
	.string "Up to six POKéMON can be carried\n"
	.string "in your party.\p"
	.string "People who raise and battle\n"
	.string "with POKéMON are called TRAINERS.$"

Kanto_ViridianCity_School_Text_844007F::
	.string "Second page…\p"
	.string "A healthy POKéMON may be hard to\n"
	.string "catch, so weaken it first.\p"
	.string "Poison, burn, or cause another\n"
	.string "status problem to weaken it.$"

Kanto_ViridianCity_School_Text_8440104::
	.string "Third page…\p"
	.string "POKéMON TRAINERS seek others to\n"
	.string "engage in POKéMON battles.\p"
	.string "To TRAINERS, the taste of victory\n"
	.string "is sweet, indeed.\p"
	.string "Battles are constantly waged at\n"
	.string "POKéMON GYMS everywhere.$"

Kanto_ViridianCity_School_Text_84401B8::
	.string "Fourth page…\p"
	.string "The ultimate goal for all POKéMON\n"
	.string "TRAINERS is simple.\p"
	.string "It is to defeat the top eight\n"
	.string "POKéMON GYM LEADERS.\p"
	.string "Do so to earn the right to face…\p"
	.string "The ELITE FOUR of the POKéMON\n"
	.string "LEAGUE!$"

Kanto_ViridianCity_School_Text_8440275::
	.string "Turn the page?$"

Kanto_ViridianCity_School_Text_8440284::
	.string "GIRL: Hey!\n"
	.string "Don’t look at my notes!$"

Kanto_ViridianCity_School_Text_84402A7::
	.string "The blackboard lists POKéMON\n"
	.string "STATUS problems during battles.$"

Kanto_ViridianCity_School_Text_84402E4::
	.string "Which topic do you want to read?$"

Kanto_ViridianCity_School_Text_8440305::
	.string "A POKéMON can’t attack if it’s\n"
	.string "asleep.\p"
	.string "POKéMON will stay asleep even\n"
	.string "after battles.\p"
	.string "Use AWAKENING to awaken one\n"
	.string "from sleep.$"

Kanto_ViridianCity_School_Text_8440381::
	.string "A burn reduces ATTACK power.\n"
	.string "It also causes ongoing HP loss.\p"
	.string "A burn remains after a battle.\n"
	.string "Use BURN HEAL to cure a burn.$"

Kanto_ViridianCity_School_Text_84403FB::
	.string "When poisoned, a POKéMON’s health\n"
	.string "steadily drops.\p"
	.string "Poison lingers after battles.\n"
	.string "Use an ANTIDOTE to cure poison!$"

Kanto_ViridianCity_School_Text_844046B::
	.string "A frozen POKéMON becomes\n"
	.string "helplessly immobile.\p"
	.string "It stays frozen even after the\n"
	.string "battle ends.\p"
	.string "Use ICE HEAL to thaw out the\n"
	.string "suffering POKéMON.$"

Kanto_ViridianCity_School_Text_84404F5::
	.string "Paralysis reduces SPEED and may\n"
	.string "prevent the POKéMON from moving.\p"
	.string "Paralysis remains after battles.\n"
	.string "Use PARLYZ HEAL for treatment.$"

Kanto_ViridianCity_School_Text_8461E90::
	.string "POKéMON JOURNAL\p"
	.string "Special Feature: PROF. OAK,\n"
	.string "the POKéMON Researcher!\p"
	.string "PROF. OAK reportedly lives with his\n"
	.string "grandchildren DAISY and BLUE.$"
