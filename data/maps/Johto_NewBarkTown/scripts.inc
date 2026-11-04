Johto_NewBarkTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_NewBarkTown_EventScript_83E13B1
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_NewBarkTown_OnFrame_83E138A
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Johto_NewBarkTown_EventScript_83E1411
	.byte 0

Johto_NewBarkTown_OnFrame_83E138A:
	map_script_2 VAR_QZ_0x5076, 6, Johto_NewBarkTown_EventScript_8403963
	map_script_2 VAR_QZ_0x5003, 6, Johto_NewBarkTown_EventScript_83E139C
	.2byte 0

Johto_NewBarkTown_EventScript_83E139C::
	lock
	@ Moemon Kai: ELM rings the POKéGEAR, in Crystal's words (docs/REGIONS_AUDIT.md 94 / 148)
	playse SE_POKENAV_CALL
	waitse
	playse SE_POKENAV_CALL
	waitse
	msgbox Johto_NewBarkTown_Text_83E1879, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5003, 7
	setvar VAR_QZ_0x5004, 0xA
	release
	end

Johto_NewBarkTown_EventScript_83E13B1::
	special QzJohtoTravelingEventCheck
	call Johto_NewBarkTown_EventScript_83E13FB
	goto_if_set FLAG_QZ_0x1318, Johto_NewBarkTown_EventScript_83E13F5
	goto_if_set FLAG_QZ_0x1425, Johto_NewBarkTown_EventScript_83E13CE
	setflag FLAG_QZ_0x11BA

Johto_NewBarkTown_EventScript_83E13CE::
	call Johto_NewBarkTown_EventScript_83E1426
	call_if_eq VAR_QZ_0x5004, 11, Johto_NewBarkTown_EventScript_83E1420
	goto_if_eq VAR_QZ_0x5003, 2, Johto_NewBarkTown_EventScript_83E1483
	goto_if_eq VAR_QZ_0x5003, 4, Johto_NewBarkTown_EventScript_83E148E
	end

Johto_NewBarkTown_EventScript_83E13F5::
	end

Johto_NewBarkTown_EventScript_83E13FB::
	setvar VAR_TEMP_F, 0
	goto_if_lt VAR_QZ_0x501A, 8, Johto_NewBarkTown_EventScript_83E1410
	setvar VAR_TEMP_F, 1

Johto_NewBarkTown_EventScript_83E1410::
	return

Johto_NewBarkTown_EventScript_83E1411::
	call Johto_NewBarkTown_EventScript_83E13FB
	goto Quetzal_EventScript_83C2D44

Johto_NewBarkTown_EventScript_83E1420::
	setvar VAR_QZ_0x5004, 0xC
	return

Johto_NewBarkTown_EventScript_83E1426::
	goto_if_set FLAG_QZ_0x11D0, Johto_NewBarkTown_EventScript_83E1482
	@ Moemon Kai: Quetzal planted BERRIES here into tree ids 90-109 (JOHTO\'s own patches, mp_apricorn.c, and the
	@ GARDEN farm\'s plots): they\'re Kai\'s and planted by their own code
	setflag FLAG_QZ_0x11D0

Johto_NewBarkTown_EventScript_83E1482::
	return

Johto_NewBarkTown_EventScript_83E1483::
	clearflag FLAG_QZ_0x106E
	setobjectxyperm 2, 0, 0xB
	end

Johto_NewBarkTown_EventScript_83E148E::
	setobjectxyperm 2, 0, 0xB
	end

@ Johto_NewBarkTown sign / bg event at 9,9
Johto_NewBarkTown_EventScript_83E1496::
	msgbox Johto_NewBarkTown_Text_83E1674, MSGBOX_SIGN
	end

@ Johto_NewBarkTown sign / bg event at 10,14
Johto_NewBarkTown_EventScript_83E149F::
	msgbox Johto_NewBarkTown_Text_83E180C, MSGBOX_SIGN
	closemessage
	end

@ Johto_NewBarkTown object 2 (local id 2, gfx 0x2F) at 9,16
Johto_NewBarkTown_EventScript_83E14A9::
	lock
	faceplayer
	goto_if_eq VAR_QZ_0x5003, 2, Johto_NewBarkTown_EventScript_83E14D7
	goto_if_eq VAR_QZ_0x5003, 3, Johto_NewBarkTown_EventScript_83E14F3
	goto_if_eq VAR_QZ_0x5003, 4, Johto_NewBarkTown_EventScript_83E14FE
	msgbox Johto_NewBarkTown_Text_83E1793, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_NewBarkTown_EventScript_83E14D7::
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	waitmovement 0
	faceplayer
	waitmovement 0
	msgbox Johto_NewBarkTown_Text_83E16A9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_NewBarkTown_EventScript_83E14F3::
	msgbox Johto_NewBarkTown_Text_83E171F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_NewBarkTown_EventScript_83E14FE::
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	waitmovement 0
	faceplayer
	waitmovement 0
	msgbox Johto_NewBarkTown_Text_83E174F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NewBarkTown trigger at 0,12 (VAR_QZ_0x5003 == 2)
@ Johto_NewBarkTown trigger at 0,13 (VAR_QZ_0x5003 == 2)
@ Johto_NewBarkTown trigger at 0,14 (VAR_QZ_0x5003 == 2)
@ Johto_NewBarkTown trigger at 0,12 (VAR_QZ_0x5003 == 4)
@ Johto_NewBarkTown trigger at 0,13 (VAR_QZ_0x5003 == 4)
@ Johto_NewBarkTown trigger at 0,14 (VAR_QZ_0x5003 == 4)
Johto_NewBarkTown_EventScript_83E151A::
	lock
	call_if_eq VAR_QZ_0x5003, 2, Johto_NewBarkTown_EventScript_83E153D
	call_if_eq VAR_QZ_0x5003, 4, Johto_NewBarkTown_EventScript_83E155E
	applymovement LOCALID_PLAYER, Johto_NewBarkTown_Movement_82CD382
	waitmovement 0
	release
	end

Johto_NewBarkTown_EventScript_83E153D::
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	waitmovement 0
	applymovement 2, Johto_NewBarkTown_Movement_83E1672
	waitmovement 0
	msgbox Johto_NewBarkTown_Text_83E16A9, MSGBOX_DEFAULT
	closemessage
	return

Johto_NewBarkTown_EventScript_83E155E::
	playse SE_PIN
	applymovement 2, Quetzal_Movement_83607F9
	waitmovement 0
	applymovement 2, Johto_NewBarkTown_Movement_83E1672
	waitmovement 0
	msgbox Johto_NewBarkTown_Text_83E174F, MSGBOX_DEFAULT
	closemessage
	return

@ Johto_NewBarkTown object 1 (local id 1, gfx 0x11) at 24,11
Johto_NewBarkTown_EventScript_83E157F::
	lock
	goto_if_ge VAR_QZ_0x501A, 8, Johto_NewBarkTown_EventScript_83E1596
	msgbox Johto_NewBarkTown_Text_83E18D9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_NewBarkTown_EventScript_83E1596::
	msgbox Johto_NewBarkTown_Text_83E1949, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_NewBarkTown object 3 (local id 3, gfx 0x3EB5) at 7,7
Johto_NewBarkTown_EventScript_83E15A1::
	lock
	msgbox Johto_NewBarkTown_Text_83E17C5, MSGBOX_DEFAULT
	delay 0x3C
	playse SE_PIN
	applymovement 3, Quetzal_Movement_83607F9
	faceplayer
	waitmovement 0
	msgbox Johto_NewBarkTown_Text_83E17F2, MSGBOX_DEFAULT
	playse SE_BANG
	applymovement 3, Johto_NewBarkTown_Movement_83E165F
	applymovement LOCALID_PLAYER, Johto_NewBarkTown_Movement_83E1669
	waitmovement 0
	applymovement 3, Quetzal_Movement_8360803
	waitmovement 0
	waitmovement 0
	release
	end

@ Johto_NewBarkTown sign / bg event at 12,7
Johto_NewBarkTown_EventScript_83E15E6::
	msgbox Johto_NewBarkTown_Text_83E1683, MSGBOX_SIGN
	end

@ Johto_NewBarkTown trigger at 24,11 (VAR_TEMP_F == 0)
@ Johto_NewBarkTown trigger at 24,12 (VAR_TEMP_F == 0)
@ Johto_NewBarkTown trigger at 24,13 (VAR_TEMP_F == 0)
@ Johto_NewBarkTown trigger at 24,14 (VAR_TEMP_F == 0)
@ Johto_NewBarkTown trigger at 25,15 (VAR_TEMP_F == 0)
Johto_NewBarkTown_EventScript_83E15EF::
	goto_if_ge VAR_QZ_0x501A, 8, Johto_NewBarkTown_EventScript_83E162C
	lockall
	msgbox Johto_NewBarkTown_Text_83E18D9, MSGBOX_DEFAULT
	closemessage
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_eq VAR_0x8004, 25, Johto_NewBarkTown_EventScript_83E1620
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083D
	goto Johto_NewBarkTown_EventScript_83E1627

Johto_NewBarkTown_EventScript_83E1620::
	applymovement LOCALID_PLAYER, Quetzal_Movement_8360839

Johto_NewBarkTown_EventScript_83E1627::
	waitmovement LOCALID_PLAYER
	releaseall
	end

Johto_NewBarkTown_EventScript_83E162C::
	setvar VAR_TEMP_F, 1
	end

@ Johto_NewBarkTown sign / bg event at 17,11
Johto_NewBarkTown_EventScript_83E184B::
	msgbox Johto_NewBarkTown_Text_83E185D, MSGBOX_SIGN
	end

@ Johto_NewBarkTown sign / bg event at 12,21
Johto_NewBarkTown_EventScript_83E1854::
	msgbox Johto_NewBarkTown_Text_83E1868, MSGBOX_SIGN
	end

Johto_NewBarkTown_EventScript_8403935::
	goto_if_unset FLAG_QZ_0x1318, Johto_NewBarkTown_EventScript_8403962
	goto_if_set FLAG_QZ_0x1319, Johto_NewBarkTown_EventScript_8403962
	goto_if_ne VAR_QZ_0x506C, 2, Johto_NewBarkTown_EventScript_8403962
	goto_if_eq VAR_QZ_0x5076, 7, Johto_NewBarkTown_EventScript_8403962
	setvar VAR_QZ_0x5076, 6

Johto_NewBarkTown_EventScript_8403962::
	return

Johto_NewBarkTown_EventScript_8403963::
	setvar VAR_QZ_0x5076, 0
	goto_if_unset FLAG_QZ_0x1318, Quetzal_EventScript_8401C95
	goto_if_set FLAG_QZ_0x1319, Quetzal_EventScript_8401C95
	goto_if_ne VAR_QZ_0x506C, 2, Quetzal_EventScript_8401C95
	lockall
	callnative 0x8140821
	special QzResetFollowerObjects
	addobject 3
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_0x8004, 20, Johto_NewBarkTown_EventScript_84039AC
	goto_if_eq VAR_0x8005, 12, Johto_NewBarkTown_EventScript_84039BE

Johto_NewBarkTown_EventScript_84039AC::
	fadescreen 1
	setobjectxy LOCALID_PLAYER, 0x14, 0xC
	setobjectxy 3, 0x16, 0xD
	fadescreen 0

Johto_NewBarkTown_EventScript_84039BE::
	closemessage
	fadenewbgm MUS_QZ_648
	applymovement 3, Johto_NewBarkTown_Movement_8403A53
	waitmovement 3
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject
	msgbox Johto_NewBarkTown_Text_8403CE8, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5076, 7
	cleartrainerflag TRAINER_QZ_4988
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4988, 0, Johto_NewBarkTown_Text_8403DB2
	fadedefaultbgm
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, 1, Quetzal_EventScript_8401C4F
	setvar VAR_0x8004, 3
	setvar VAR_0x8005, 0xFF
	special QzObjectFaceObject
	msgbox Johto_NewBarkTown_Text_8403DD9, MSGBOX_DEFAULT
	setvar VAR_QZ_0x506C, 3
	setvar VAR_QZ_0x5076, 0
	msgbox Johto_NewBarkTown_Text_8404041, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_NewBarkTown_EventScript_8403A3E
	closemessage
	fadescreen 3
	goto Quetzal_EventScript_8401B9D

Johto_NewBarkTown_EventScript_8403A3E::
	msgbox Johto_NewBarkTown_Text_8404090, MSGBOX_DEFAULT
	closemessage
	fadescreen 3
	removeobject 3
	fadescreen 2
	goto Quetzal_EventScript_8401C94

Johto_NewBarkTown_Movement_82CD382:
	delay_8
	walk_right
	step_end

Johto_NewBarkTown_Movement_83E165F:
	walk_fast_left
	walk_fast_left
	walk_fast_down
	face_down
	lock_facing_direction
	unlock_facing_direction
	walk_fast_up
	walk_fast_right
	walk_fast_right
	step_end

Johto_NewBarkTown_Movement_83E1669:
	walk_fast_left
	face_up
	walk_fast_down
	face_up
	lock_facing_direction
	jump_2_down
	unlock_facing_direction
	face_down
	step_end

Johto_NewBarkTown_Movement_83E1672:
	face_down
	step_end

Johto_NewBarkTown_Movement_8403A53:
	walk_left
	walk_up
	step_end

Johto_NewBarkTown_Text_83E1674::
	.string "PROF.ELM’S LAB$"

Johto_NewBarkTown_Text_83E1683::
	.string "The door is locked.$"

Johto_NewBarkTown_Text_83E16A9::
	.string "Wait, {PLAYER}!\p"
	.string "It’s dangerous to go out\n"
	.string "without a MOéMON!\p"
	.string "Wild MOéMON jump out of the grass\n"
	.string "on the way to the next town.$"

Johto_NewBarkTown_Text_83E171F::
	.string "Oh! Your MOéMON is adorable!\n"
	.string "I wish I had one!$"

Johto_NewBarkTown_Text_83E174F::
	.string "Hi, {PLAYER}! Leaving again?\p"
	.string "You should tell your mom if\n"
	.string "you are leaving.$"

Johto_NewBarkTown_Text_83E1793::
	.string "Don’t forget to visit your mom\n"
	.string "from time to time.$"

Johto_NewBarkTown_Text_83E17C5::
	.string ". . .\p"
	.string "So this is the famous ELM\n"
	.string "MOéMON LAB…$"

Johto_NewBarkTown_Text_83E17F2::
	.string "…What are you staring at?$"

Johto_NewBarkTown_Text_83E180C::
	.string "NEW BARK TOWN\p"
	.string "The Town Where the Winds of\n"
	.string "a New Beginning Blow$"

Johto_NewBarkTown_Text_83E185D::
	.string "{PLAYER}’S HOUSE$"

Johto_NewBarkTown_Text_83E1868::
	.string "PROF.ELM’S HOUSE$"

Johto_NewBarkTown_Text_83E1879::
	.string "Hello, {PLAYER}? How’s it going?\p"
	.string "I got ahold of something neat.\p"
	.string "Swing by my LAB and pick it up!\p"
	.string "See you later!$"

Johto_NewBarkTown_Text_83E18D9::
	.string "Wait! That crossing leads to the\n"
	.string "MOéMON LEAGUE.\p"
	.string "You still have things to do in\n"
	.string "JOHTO. Don’t rush off just yet!$"

Johto_NewBarkTown_Text_83E1949::
	.string "You can head for the LEAGUE now!\n"
	.string "It’s a dangerous crossing.\p"
	.string "Take care of your MOéMON.\n"
	.string "Good luck!$"

Johto_NewBarkTown_Text_8403CE8::
	.string "{RIVAL}: I heard what\n"
	.string "was happening in there.\p"
	.string "While it was other families,\n"
	.string "you obeyed GIOVANNI.\p"
	.string "Now you have saved\n"
	.string "your mother. So what?\p"
	.string "Are you going to leave everyone\n"
	.string "else in the hands of TEAM ROCKET?$"

Johto_NewBarkTown_Text_8403DB2::
	.string "...You have more than enough strength.$"

Johto_NewBarkTown_Text_8403DD9::
	.string "{RIVAL}: ...You are strong. You could\n"
	.string "have stood up to them long ago.\p"
	.string "You say you are going back\n"
	.string "to the RADIO TOWER... To stop\l"
	.string "GIOVANNI and save the other families.\p"
	.string "Your mother is safe now. You could\n"
	.string "leave... And you are still going back.\p"
	.string "I wanted to reach GIOVANNI too.\n"
	.string "I wanted to beat him.\l"
	.string "Make him admit I was stronger than him.\p"
	.string "Whenever someone offered\n"
	.string "to help, I pushed them away.\p"
	.string "I thought that if I could not do\n"
	.string "it alone, I had proved nothing.\p"
	.string "But while I was trying to prove\n"
	.string "something... he kept doing this.\p"
	.string "I am coming with you. I have not\n"
	.string "forgotten that you helped him.\l"
	.string "But right now, we have to stop him.$"

Johto_NewBarkTown_Text_8404041::
	.string "{RIVAL}: I am going to the RADIO TOWER.\n"
	.string "CELEBI can take us.\p"
	.string "Are you coming now?$"

Johto_NewBarkTown_Text_8404090::
	.string "{RIVAL}: CELEBI, take me there first.\n"
	.string "I will wait upstairs, by GIOVANNI.$"
