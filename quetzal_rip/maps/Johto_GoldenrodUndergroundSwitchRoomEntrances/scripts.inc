Johto_GoldenrodUndergroundSwitchRoomEntrances_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_LOAD, Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA15D
	.byte 0

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA15D::
	call_if_set FLAG_QZ_0x10A1, Quetzal_EventScript_83DA173
	clearflag FLAG_QZ_0x109D
	clearflag FLAG_QZ_0x109E
	clearflag FLAG_QZ_0x109F
	clearflag FLAG_QZ_0x10A0
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances sign / bg event at 22,1
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA192::
	lock
	goto_if_unset FLAG_QZ_0x109D, Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA1CF
	clearflag FLAG_QZ_0x109D
	playse SE_BANG
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAEFF, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_83DA3A2
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA3D9
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA410
	call Quetzal_EventScript_83DA447
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA1CF::
	setflag FLAG_QZ_0x109D
	goto_if_set FLAG_QZ_0x10A0, Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA20E
	clearflag FLAG_QZ_0x10A0
	playse SE_BANG
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAEC3, MSGBOX_DEFAULT
	closemessage
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA4EC
	call Quetzal_EventScript_83DA523
	call Quetzal_EventScript_83DA55A
	call Quetzal_EventScript_83DA447
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA20E::
	playse SE_CLICK
	setflag FLAG_QZ_0x10A1
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAEC3, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_83DA3A2
	call Quetzal_EventScript_83DA523
	call Quetzal_EventScript_83DA55A
	call Quetzal_EventScript_83DA447
	call Quetzal_EventScript_83DA5C8
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances sign / bg event at 13,1
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA241::
	lock
	goto_if_unset FLAG_QZ_0x109E, Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA281
	clearflag FLAG_QZ_0x109E
	clearflag FLAG_QZ_0x10A0
	playse SE_BANG
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAF15, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_83DA3A2
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA3D9
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA410
	call Quetzal_EventScript_83DA447
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA281::
	setflag FLAG_QZ_0x109E
	goto_if_set FLAG_QZ_0x109D, Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA2C6
	goto_if_set FLAG_QZ_0x109F, Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA2F9
	playse SE_BANG
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAED7, MSGBOX_DEFAULT
	closemessage
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA4EC
	call Quetzal_EventScript_83DA523
	call Quetzal_EventScript_83DA55A
	call Quetzal_EventScript_83DA447
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA2C6::
	clearflag FLAG_QZ_0x10A0
	playse SE_BANG
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAED7, MSGBOX_DEFAULT
	closemessage
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA4EC
	call Quetzal_EventScript_83DA523
	call Quetzal_EventScript_83DA55A
	call Quetzal_EventScript_83DA447
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA2F9::
	setflag FLAG_QZ_0x10A0
	playse SE_CLICK
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAED7, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_83DA3A2
	call Quetzal_EventScript_83DA523
	call Quetzal_EventScript_83DA55A
	call Quetzal_EventScript_83DA447
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances sign / bg event at 5,1
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA32C::
	lock
	goto_if_unset FLAG_QZ_0x109F, Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA36C
	clearflag FLAG_QZ_0x109F
	clearflag FLAG_QZ_0x10A0
	playse SE_BANG
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAF2B, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_83DA3A2
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA3D9
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA410
	call Quetzal_EventScript_83DA447
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA36C::
	setflag FLAG_QZ_0x109F
	clearflag FLAG_QZ_0x10A0
	playse SE_CLICK
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAEEB, MSGBOX_DEFAULT
	closemessage
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA4EC
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA591
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA3D9
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA410
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E
	call Quetzal_EventScript_83DA4B5
	special DrawWholeMapView
	waitse
	release
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA3D9::
	setmetatile 0xE, 7, 0x399, 0
	setmetatile 0xF, 7, 0x39A, 0
	setmetatile 0x10, 7, 0x39B, 0
	setmetatile 0xE, 8, 0x3A1, 0
	setmetatile 0xF, 8, 0x3A2, 0
	setmetatile 0x10, 8, 0x3A3, 0
	return

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA410::
	setmetatile 0x17, 7, 0x399, 0
	setmetatile 0x18, 7, 0x39A, 0
	setmetatile 0x19, 7, 0x39B, 0
	setmetatile 0x17, 8, 0x3A1, 0
	setmetatile 0x18, 8, 0x3A2, 0
	setmetatile 0x19, 8, 0x3A3, 0
	return

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA47E::
	setmetatile 0xE, 0xB, 0x399, 0
	setmetatile 0xF, 0xB, 0x39A, 0
	setmetatile 0x10, 0xB, 0x39B, 0
	setmetatile 0xE, 0xC, 0x3A1, 0
	setmetatile 0xF, 0xC, 0x3A2, 0
	setmetatile 0x10, 0xC, 0x3A3, 0
	return

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA4EC::
	setmetatile 5, 7, 0x378, 0
	setmetatile 6, 7, 0x379, 0
	setmetatile 7, 7, 0x37A, 0
	setmetatile 5, 8, 0x380, 0
	setmetatile 6, 8, 0x381, 0
	setmetatile 7, 8, 0x382, 0
	return

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA591::
	setmetatile 5, 0xB, 0x378, 0
	setmetatile 6, 0xB, 0x379, 0
	setmetatile 7, 0xB, 0x37A, 0
	setmetatile 5, 0xC, 0x380, 0
	setmetatile 6, 0xC, 0x381, 0
	setmetatile 7, 0xC, 0x382, 0
	return

@ Johto_GoldenrodUndergroundSwitchRoomEntrances trigger at 26,5 (VAR_QZ_0x500A == 8)
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA636::
	lock
	playse SE_EXIT
	waitse
	playse SE_PIN
	playbgm MUS_QZ_648, 1
	addobject 7
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	applymovement 7, Johto_GoldenrodUndergroundSwitchRoomEntrances_Movement_83DA70A
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083B
	waitmovement 0
	waitmovement 0
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DA7E9, MSGBOX_DEFAULT
	closemessage
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA6FF
	playbgm MUS_QZ_649, 1
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DA935, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Johto_GoldenrodUndergroundSwitchRoomEntrances_Movement_83DA715
	waitmovement 0
	playse SE_EXIT
	setflag FLAG_QZ_0x109C
	removeobject 7
	fadedefaultbgm
	setvar VAR_QZ_0x500A, 9
	release
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances trigger at 26,6 (VAR_QZ_0x500A == 8)
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA699::
	lock
	playse SE_EXIT
	waitse
	playse SE_PIN
	playbgm MUS_QZ_648, 1
	clearflag FLAG_QZ_0x109C
	addobject 7
	applymovement LOCALID_PLAYER, Quetzal_Movement_83607F9
	applymovement 7, Johto_GoldenrodUndergroundSwitchRoomEntrances_Movement_83DA720
	waitmovement 0
	applymovement LOCALID_PLAYER, Quetzal_Movement_836083B
	waitmovement 0
	waitmovement 0
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DA7E9, MSGBOX_DEFAULT
	closemessage
	call Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA6FF
	playbgm MUS_QZ_649, 1
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DA935, MSGBOX_DEFAULT
	closemessage
	applymovement 7, Johto_GoldenrodUndergroundSwitchRoomEntrances_Movement_83DA72C
	waitmovement 0
	playse SE_EXIT
	setflag FLAG_QZ_0x109C
	removeobject 7
	fadedefaultbgm
	setvar VAR_QZ_0x500A, 9
	release
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA6FF::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4599, 0, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DA8CC
	return

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 3 (local id 3, gfx 0x3EB0) at 20,3
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA738::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4801, 0, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAAF3, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAB37
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAB57, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 2 (local id 2, gfx 0x3EB0) at 15,3
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA74F::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4216, 0, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAB9F, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DABFC
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAC0D, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 4 (local id 4, gfx 0x3E8A) at 3,9
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA766::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4592, 0, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAC74, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAC8C
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAC93, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 5 (local id 5, gfx 0x3E8A) at 11,13
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA77D::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4428, 0, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DACBF, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAD10
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAD1E, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 1 (local id 1, gfx 0x3EB0) at 6,3
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA794::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4794, 0, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAD73, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DADBF
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DADE2, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 6 (local id 6, gfx 0x3EB0) at 28,13
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA7AB::
	trainerbattle TRAINER_BATTLE_SINGLE, TRAINER_QZ_4607, 0, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAE16, Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAE83
	msgbox Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAE92, MSGBOX_AUTOCLOSE
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 8 (local id 8, gfx 0x3A9E) at 19,9
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA7C2::
	setorcopyvar VAR_0x8000, 0x22
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 9 (local id 9, gfx 0x3A9E) at 20,13
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA7CF::
	setorcopyvar VAR_0x8000, 0x1F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_GoldenrodUndergroundSwitchRoomEntrances object 10 (local id 10, gfx 0x3A9E) at 3,13
Johto_GoldenrodUndergroundSwitchRoomEntrances_EventScript_83DA7DC::
	setorcopyvar VAR_0x8000, 0x1D4
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_GoldenrodUndergroundSwitchRoomEntrances_Movement_83DA70A:
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodUndergroundSwitchRoomEntrances_Movement_83DA715:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	face_right
	step_end

Johto_GoldenrodUndergroundSwitchRoomEntrances_Movement_83DA720:
	walk_left
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

Johto_GoldenrodUndergroundSwitchRoomEntrances_Movement_83DA72C:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	face_right
	step_end

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DA7E9::
	.string "Hold it!\p"
	.string "I saw you, so I tailed you.\p"
	.string "I don’t need you underfoot while I\n"
	.string "take care of TEAM ROCKET.\p"
	.string "…Wait a second. You beat me before,\n"
	.string "didn’t you?\p"
	.string "That was just a fluke.\p"
	.string "I’ve been training my team and now\n"
	.string "I will repay my debts!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DA8CC::
	.string "…Why… Why do I lose?\p"
	.string "I’ve assembled the toughest POKéMON.\p"
	.string "I didn’t ease up on the gas.\p"
	.string "So why do I lose?$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DA935::
	.string "…I don’t understand…\p"
	.string "Is what that LANCE guy said true?\p"
	.string "That I don’t treat POKéMON properly?\p"
	.string "Love…\p"
	.string "Trust…\p"
	.string "Are they really what I lack?\p"
	.string "Are they keeping me from winning?\p"
	.string "I… I just don’t understand.\p"
	.string "But it’s not going to end here.\p"
	.string "Not now. Not because of this.\p"
	.string "I won’t give up my dream of becoming\n"
	.string "the world’s best POKéMON trainer!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAAF3::
	.string "Open one shutter, another closes.\p"
	.string "Bet you can’t get where you want!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAB37::
	.string "Drat! I was sunk by indecision!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAB57::
	.string "I’m confused too… The switch on the\n"
	.string "end is the one to press first, but…$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAB9F::
	.string "Kwahaha!\p"
	.string "Confounded by the shutters, are we?\p"
	.string "I’ll let you in on a secret if you\n"
	.string "can beat me!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DABFC::
	.string "Uwww… I blew it.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAC0D::
	.string "All right. A hint!\p"
	.string "Change the order of switching.\p"
	.string "That’ll change the ways the shutters\n"
	.string "open and close.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAC74::
	.string "Fork over your goodies!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAC8C::
	.string "Mercy!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAC93::
	.string "Steal and sell! That’s basic in\n"
	.string "crime, kid!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DACBF::
	.string "They ditched this project before they\n"
	.string "finished.\p"
	.string "I’m searching for leftover loot.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAD10::
	.string "Over the top!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAD1E::
	.string "UNDERGROUND WAREHOUSE?\p"
	.string "What do you want to go there for?\p"
	.string "There’s nothing down there.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAD73::
	.string "I don’t care if you’re lost.\p"
	.string "You show up here, you’re nothing but\n"
	.string "a victim!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DADBF::
	.string "Urk! Yeah, think you’re cool, huh?$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DADE2::
	.string "You must have ice in your veins to\n"
	.string "dis TEAM ROCKET.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAE16::
	.string "Are you lost? No, you can’t be.\p"
	.string "You don’t have that scared look.\p"
	.string "I’ll give you something to be\n"
	.string "scared about!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAE83::
	.string "How could you?$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAE92::
	.string "Go wherever you’d like! Get lost!\n"
	.string "See if I care!$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAEC3::
	.string "SWITCH 1 activated.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAED7::
	.string "SWITCH 2 activated.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAEEB::
	.string "SWITCH 3 activated.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAEFF::
	.string "SWITCH 1 deactivated.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAF15::
	.string "SWITCH 2 deactivated.$"

Johto_GoldenrodUndergroundSwitchRoomEntrances_Text_83DAF2B::
	.string "SWITCH 3 deactivated.$"
