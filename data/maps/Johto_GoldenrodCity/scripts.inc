Johto_GoldenrodCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_GoldenrodCity_EventScript_83D23CC
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_GoldenrodCity_EventScript_83D23CC::
	special QzSetGiovanniModeFlag14A4
	special QzJohtoTravelingEventCheck
	call Quetzal_EventScript_840EA62
	end

@ Johto_GoldenrodCity trigger at 34,26 (VAR_TEMP_0 == 0)
@ Johto_GoldenrodCity trigger at 32,26 (VAR_TEMP_0 == 0)
@ Johto_GoldenrodCity trigger at 33,26 (VAR_TEMP_0 == 0)
@ Johto_GoldenrodCity trigger at 35,26 (VAR_TEMP_0 == 0)
@ Johto_GoldenrodCity trigger at 30,31 (VAR_TEMP_0 == 0)
@ Johto_GoldenrodCity trigger at 30,32 (VAR_TEMP_0 == 0)
Johto_GoldenrodCity_EventScript_83D23E5::
	goto_if_lt VAR_QZ_0x500A, 6, Johto_GoldenrodCity_EventScript_83D2405
	goto_if_ge VAR_QZ_0x500A, 11, Johto_GoldenrodCity_EventScript_83D2405
	playbgm MUS_QZ_642, 1
	setvar VAR_TEMP_0, 1
	end

Johto_GoldenrodCity_EventScript_83D2405::
	setvar VAR_TEMP_0, 1
	end

@ Johto_GoldenrodCity object 12 (local id 12, gfx 0x11) at 7,23
Johto_GoldenrodCity_EventScript_83D240B::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D26FE, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 7 (local id 7, gfx 0x3E87) at 39,22
Johto_GoldenrodCity_EventScript_83D2418::
	msgbox Johto_GoldenrodCity_Text_83D273C, MSGBOX_NPC
	end

@ Johto_GoldenrodCity object 8 (local id 8, gfx 0x23) at 30,21
Johto_GoldenrodCity_EventScript_83D2421::
	msgbox Johto_GoldenrodCity_Text_83D28BC, MSGBOX_NPC
	end

@ Johto_GoldenrodCity object 9 (local id 9, gfx 0x3E85) at 20,25
Johto_GoldenrodCity_EventScript_83D242A::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1096, Johto_GoldenrodCity_EventScript_83D2440
	msgbox Johto_GoldenrodCity_Text_83D277A, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodCity_EventScript_83D2440::
	msgbox Johto_GoldenrodCity_Text_83D27C0, MSGBOX_NPC
	end

@ Johto_GoldenrodCity object 1 (local id 1, gfx 0x2F) at 25,13
Johto_GoldenrodCity_EventScript_83D2449::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D2901, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 4 (local id 4, gfx 0x3EA6) at 17,36
Johto_GoldenrodCity_EventScript_83D2456::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D2948, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 27,13
Johto_GoldenrodCity_EventScript_83D2463::
	msgbox Johto_GoldenrodCity_Text_83D2C27, MSGBOX_SIGN
	end

@ Johto_GoldenrodCity object 11 (local id 11, gfx 0x3EB0) at 18,22
Johto_GoldenrodCity_EventScript_83D246C::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2981, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement 11, Quetzal_Movement_83607F9
	delay 0x14
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D299D, MSGBOX_DEFAULT
	closemessage
	turnobject 11, 2
	release
	end

@ Johto_GoldenrodCity object 10 (local id 10, gfx 0x3EB0) at 22,22
Johto_GoldenrodCity_EventScript_83D2493::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D29C4, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 13 (local id 13, gfx 0x3EB0) at 44,12
Johto_GoldenrodCity_EventScript_83D24A0::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D29E7, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 14 (local id 14, gfx 0x3EB0) at 48,16
Johto_GoldenrodCity_EventScript_83D24AD::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D2A24, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 5 (local id 5, gfx 0x3EB0) at 43,26
Johto_GoldenrodCity_EventScript_83D24BA::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D2A56, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 6 (local id 6, gfx 0x3EB0) at 44,26
@ Johto_GoldenrodCity object 15 (local id 15, gfx 0x3EB0) at 14,34
@ Johto_GoldenrodCity object 16 (local id 16, gfx 0x3EB0) at 39,14
@ Johto_GoldenrodCity object 17 (local id 17, gfx 0x3EB0) at 29,14
Johto_GoldenrodCity_EventScript_83D24C7::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D2A93, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 3 (local id 3, gfx 0x3EAF) at 28,30
Johto_GoldenrodCity_EventScript_83D24D4::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D2AC7, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 18 (local id 18, gfx 0x14) at 30,39
Johto_GoldenrodCity_EventScript_83D24E1::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D25E2, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity object 2 (local id 2, gfx 0x37) at 20,30
Johto_GoldenrodCity_EventScript_83D24EE::
	lock
	faceplayer
	@ Moemon Kai: Crystal's RADIO CARD woman (pokecrystal GoldenrodCity.asm CooltrainerF2Script, docs/REGIONS_AUDIT.md 144)
	goto_if_set FLAG_JOHTO_RADIO_CARD, Johto_GoldenrodCity_EventScript_83D2504
	msgbox Johto_GoldenrodCity_Text_83D262B, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodCity_EventScript_83D2504::
	msgbox Johto_GoldenrodCity_Text_83D267B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 19,13
@ Johto_GoldenrodCity sign / bg event at 19,38
Johto_GoldenrodCity_EventScript_83D250F::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2C68, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 41,37
Johto_GoldenrodCity_EventScript_83D251B::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2B23, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 43,39
Johto_GoldenrodCity_EventScript_83D2527::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2BD6, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 26,22
Johto_GoldenrodCity_EventScript_83D2533::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2AF1, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 41,13
Johto_GoldenrodCity_EventScript_83D253F::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2B5E, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 45,12
Johto_GoldenrodCity_EventScript_83D254B::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2C7D, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 12,22
Johto_GoldenrodCity_EventScript_83D2557::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2B08, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_GoldenrodCity sign / bg event at 30,30
Johto_GoldenrodCity_EventScript_83D2563::
	lock
	msgbox Johto_GoldenrodCity_Text_83D2BFB, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_GoldenrodCity object 19 (local id 19, gfx 0x2D) at 39,14
Johto_GoldenrodCity_EventScript_83D256F::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_83D257C, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_GoldenrodCity_Text_83D257C::
	.string "This is the GOLDENROD GYM!\p"
	.string "Our leader WHITNEY is not here\n"
	.string "right now. She might be at the\l"
	.string "RADIO TOWER.$"

Johto_GoldenrodCity_Text_83D25E2::
	.string "I used to train in WHITNEY’s GYM,\n"
	.string "but all her crying gave me a\l"
	.string "headache.$"

Johto_GoldenrodCity_Text_83D262B::
	.string "The RADIO TOWER in GOLDENROD CITY\n"
	.string "is a landmark.\p"
	.string "They’re running a promotional\n"
	.string "campaign right now.\p"
	.string "They’ll modify your POKéGEAR, so\n"
	.string "it can also serve as a radio.$"

Johto_GoldenrodCity_Text_83D267B::
	.string "Oh, your POKéGEAR works as a\n"
	.string "radio!$"

Johto_GoldenrodCity_Text_83D26FE::
	.string "They built the new RADIO TOWER\n"
	.string "to replace the old creaky one.$"

Johto_GoldenrodCity_Text_83D273C::
	.string "I know there’s a new BIKE SHOP,\n"
	.string "but I can’t find it anywhere.$"

Johto_GoldenrodCity_Text_83D277A::
	.string "Is that man in black dressed up\n"
	.string "like a TEAM ROCKET member?\p"
	.string "How silly!$"

Johto_GoldenrodCity_Text_83D27C0::
	.string "Was that man in black really part\n"
	.string "of TEAM ROCKET? I can’t believe it!$"

Johto_GoldenrodCity_Text_83D28BC::
	.string "E-he-he-he…\p"
	.string "I got in trouble for playing in\n"
	.string "the DEPT.STORE basement.$"

Johto_GoldenrodCity_Text_83D2901::
	.string "That man inside rates your\n"
	.string "MOéMON nicknames.\p"
	.string "He can even rename them.$"

Johto_GoldenrodCity_Text_83D2948::
	.string "Whew! This is one big town. I\n"
	.string "don’t know where anything is.$"

Johto_GoldenrodCity_Text_83D2981::
	.string "So this is the RADIO TOWER…$"

Johto_GoldenrodCity_Text_83D299D::
	.string "What do you want, pest?\n"
	.string "Scram already!$"

Johto_GoldenrodCity_Text_83D29C4::
	.string "Stay out of the way!\n"
	.string "Beat it, kid!$"

Johto_GoldenrodCity_Text_83D29E7::
	.string "Take over the RADIO TOWER…\p"
	.string "What? It’s none of your business!$"

Johto_GoldenrodCity_Text_83D2A24::
	.string "MOéMON? They’re just tools\n"
	.string "we use to make money!$"

Johto_GoldenrodCity_Text_83D2A56::
	.string "Our dream will soon come true…\p"
	.string "It was a long, hard struggle…$"

Johto_GoldenrodCity_Text_83D2A93::
	.string "Hey, brat! You don’t belong here!\n"
	.string "Get lost already!$"

Johto_GoldenrodCity_Text_83D2AC7::
	.string "Come and taste the terror of\n"
	.string "TEAM ROCKET!$"

Johto_GoldenrodCity_Text_83D2AF1::
	.string "GOLDENROD CITY STATION$"

Johto_GoldenrodCity_Text_83D2B08::
	.string "GOLDENROD CITY RADIO TOWER$"

Johto_GoldenrodCity_Text_83D2B23::
	.string "Full Selection of MOéMON Goods!\p"
	.string "GOLDENROD CITY DEPT.STORE$"

Johto_GoldenrodCity_Text_83D2B5E::
	.string "GOLDENROD CITY MOéMON GYM\n"
	.string "LEADER: WHITNEY\p"
	.string "The Incredibly Pretty Girl!$"

Johto_GoldenrodCity_Text_83D2BD6::
	.string "The World is a Cycle Path!\n"
	.string "BIKE SHOP$"

Johto_GoldenrodCity_Text_83D2BFB::
	.string "Your Playground!\p"
	.string "GOLDENROD CITY GAME CORNER$"

Johto_GoldenrodCity_Text_83D2C27::
	.string "NAME RATER\p"
	.string "Get Your MOéMON Nicknames Rated$"

Johto_GoldenrodCity_Text_83D2C68::
	.string "UNDERGROUND ENTRANCE$"

Johto_GoldenrodCity_Text_83D2C7D::
	.string "Blooming Beautiful FLOWER SHOP$"


@ Moemon Kai: Crystal's GOLDENROD CITY sign (docs/REGIONS_AUDIT.md 144)
Johto_GoldenrodCity_EventScript_CitySign::
	msgbox Johto_GoldenrodCity_Text_CitySign, MSGBOX_SIGN
	end

@ Moemon Kai: Crystal's move tutor (pokecrystal GoldenrodCity.asm MoveTutorScript, docs/REGIONS_AUDIT.md 144): after the
@ LEAGUE, with a COIN CASE, Wednesdays and Saturdays (FLAG_JOHTO_HIDE_GOLDENROD_TUTOR, mp_region.c
@ MP_JohtoWeekdayPeople); FLAMETHROWER, THUNDERBOLT or ICE BEAM for 4000 coins to a POKéMON that learns the TM, then he
@ goes into the GAME CORNER for the day (FLAG_DAILY_JOHTO_TUTOR)
Johto_GoldenrodCity_EventScript_MoveTutor::
	lock
	faceplayer
	msgbox Johto_GoldenrodCity_Text_TutorAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodCity_EventScript_TutorRefused
	showcoinsbox 0, 0
	msgbox Johto_GoldenrodCity_Text_TutorCost, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Johto_GoldenrodCity_EventScript_TutorRefused2
	checkcoins VAR_TEMP_1
	goto_if_lt VAR_TEMP_1, 4000, Johto_GoldenrodCity_EventScript_TutorNoCoins
	msgbox Johto_GoldenrodCity_Text_TutorWhich, MSGBOX_DEFAULT
	multichoice 0, 0, MULTI_KAI_GOLDENROD_TUTOR, FALSE
	switch VAR_RESULT
	case 0, Johto_GoldenrodCity_EventScript_TutorFlamethrower
	case 1, Johto_GoldenrodCity_EventScript_TutorThunderbolt
	case 2, Johto_GoldenrodCity_EventScript_TutorIceBeam
	goto Johto_GoldenrodCity_EventScript_TutorBBut
Johto_GoldenrodCity_EventScript_TutorFlamethrower::
	setvar VAR_0x8005, TUTOR_MOVE_TM_FLAMETHROWER
	goto Johto_GoldenrodCity_EventScript_TutorTeach
Johto_GoldenrodCity_EventScript_TutorThunderbolt::
	setvar VAR_0x8005, TUTOR_MOVE_TM_THUNDERBOLT
	goto Johto_GoldenrodCity_EventScript_TutorTeach
Johto_GoldenrodCity_EventScript_TutorIceBeam::
	setvar VAR_0x8005, TUTOR_MOVE_TM_ICE_BEAM
Johto_GoldenrodCity_EventScript_TutorTeach::
	hidecoinsbox 0, 0
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, Johto_GoldenrodCity_EventScript_TutorBBut
	msgbox Johto_GoldenrodCity_Text_TutorMadeIt, MSGBOX_DEFAULT
	removecoins 4000
	showcoinsbox 0, 0
	playse SE_SHOP
	waitse
	msgbox Johto_GoldenrodCity_Text_TutorFarewell, MSGBOX_DEFAULT
	hidecoinsbox 0, 0
	closemessage
	setflag FLAG_DAILY_JOHTO_TUTOR
	goto_if_eq VAR_FACING, DIR_WEST, Johto_GoldenrodCity_EventScript_TutorAround
	applymovement VAR_LAST_TALKED, Johto_GoldenrodCity_Movement_TutorToGameCorner
	goto Johto_GoldenrodCity_EventScript_TutorInside
Johto_GoldenrodCity_EventScript_TutorAround::
	applymovement VAR_LAST_TALKED, Johto_GoldenrodCity_Movement_TutorAroundToGameCorner
Johto_GoldenrodCity_EventScript_TutorInside::
	waitmovement 0
	playse SE_EXIT
	removeobject VAR_LAST_TALKED
	setflag FLAG_JOHTO_HIDE_GOLDENROD_TUTOR
	release
	end
Johto_GoldenrodCity_EventScript_TutorRefused::
	msgbox Johto_GoldenrodCity_Text_TutorAww, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodCity_EventScript_TutorRefused2::
	hidecoinsbox 0, 0
	msgbox Johto_GoldenrodCity_Text_TutorTooBad, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodCity_EventScript_TutorNoCoins::
	hidecoinsbox 0, 0
	msgbox Johto_GoldenrodCity_Text_TutorNoCoins, MSGBOX_DEFAULT
	release
	end
Johto_GoldenrodCity_EventScript_TutorBBut::
	hidecoinsbox 0, 0
	msgbox Johto_GoldenrodCity_Text_TutorBBut, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodCity_Movement_TutorToGameCorner:
	walk_right
	walk_right
	walk_up
	step_end

Johto_GoldenrodCity_Movement_TutorAroundToGameCorner:
	walk_down
	walk_right
	walk_right
	walk_up
	walk_up
	step_end

Johto_GoldenrodCity_Text_CitySign::
	.string "GOLDENROD CITY\p"
	.string "The Festive City of Opulent Charm$"

Johto_GoldenrodCity_Text_TutorAsk::
	.string "I can teach your MOéMON amazing\n"
	.string "moves if you’d like.\p"
	.string "Should I teach a new move?$"

Johto_GoldenrodCity_Text_TutorCost::
	.string "It will cost you 4000 coins. Okay?$"

Johto_GoldenrodCity_Text_TutorWhich::
	.string "Wahahah! You won’t regret it!\p"
	.string "Which move should I teach?$"

Johto_GoldenrodCity_Text_TutorAww::
	.string "Aww… But they’re amazing…$"

Johto_GoldenrodCity_Text_TutorTooBad::
	.string "Hm, too bad. I’ll have to get some\n"
	.string "cash from home…$"

Johto_GoldenrodCity_Text_TutorMadeIt::
	.string "If you understand what’s so\n"
	.string "amazing about this move, you’ve\l"
	.string "made it as a trainer.$"

Johto_GoldenrodCity_Text_TutorFarewell::
	.string "Wahahah! Farewell, kid!$"

Johto_GoldenrodCity_Text_TutorBBut::
	.string "B-but…$"

Johto_GoldenrodCity_Text_TutorNoCoins::
	.string "…You don’t have enough coins here…$"
