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
	goto_if_set FLAG_QZ_0x1016, Johto_GoldenrodCity_EventScript_83D2504
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
	.string "When I went into the GAME CORNER\n"
	.string "it was night time... How long was I\l"
	.string "in there?!$"

Johto_GoldenrodCity_Text_83D267B::
	.string "When I went into the GAME CORNER\n"
	.string "it was day time... How long was I\l"
	.string "in there?!$"

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
	.string "POKéMON nicknames.\p"
	.string "He can even rename them.$"

Johto_GoldenrodCity_Text_83D2948::
	.string "Whew! This town is so big,\n"
	.string "I can’t find anything at all.$"

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
	.string "POKéMON? They’re just tools\n"
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
	.string "Full Selection of POKéMON Goods!\p"
	.string "GOLDENROD CITY DEPT.STORE$"

Johto_GoldenrodCity_Text_83D2B5E::
	.string "GOLDENROD CITY POKéMON GYM\n"
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
	.string "Get Your POKéMON Nicknames Rated$"

Johto_GoldenrodCity_Text_83D2C68::
	.string "UNDERGROUND ENTRANCE$"

Johto_GoldenrodCity_Text_83D2C7D::
	.string "Blooming Beautiful FLOWER SHOP$"

