Johto_AzaleaTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_AzaleaTown_EventScript_83C3B1D
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_AzaleaTown_OnFrame_83C3B39
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_AzaleaTown_OnFrame_83C3B39:
	map_script_2 VAR_TEMP_0, 8, Johto_AzaleaTown_EventScript_83C3B43
	.2byte 0

Johto_AzaleaTown_EventScript_83C3B1D::
	special QzSetGiovanniModeFlag14A4_794
	special QzJohtoTravelingEventCheck_770
	call Quetzal_EventScript_840EA62
	copyvar VAR_TEMP_0, VAR_QZ_0x5009
	goto_if_eq VAR_QZ_0x5009, 5, Johto_AzaleaTown_EventScript_83C3B6F
	end

Johto_AzaleaTown_EventScript_83C3B43::
	setvar VAR_TEMP_0, 0
	goto_if_ne VAR_QZ_0x5009, 8, Johto_AzaleaTown_EventScript_83C3B6E
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_0x8004, 19, Johto_AzaleaTown_EventScript_83C3B6E
	goto_if_eq VAR_0x8005, 12, Johto_AzaleaTown_EventScript_83C3CD5

Johto_AzaleaTown_EventScript_83C3B6E::
	end

Johto_AzaleaTown_EventScript_83C3B6F::
	setflag FLAG_QZ_0x1086
	removeobject 6
	end

@ Johto_AzaleaTown object 1 (local id 1, gfx 0x3EB0) at 52,9
Johto_AzaleaTown_EventScript_83C3B7F::
	lock
	faceplayer
	msgbox Johto_AzaleaTown_Text_83C3FDC, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaTown object 3 (local id 3, gfx 0x3EB0) at 23,25
Johto_AzaleaTown_EventScript_83C3B8C::
	lock
	faceplayer
	msgbox Johto_AzaleaTown_Text_83C402A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaTown object 2 (local id 2, gfx 0x3EA6) at 44,14
Johto_AzaleaTown_EventScript_83C3B99::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1084, Johto_AzaleaTown_EventScript_83C3BAF
	msgbox Johto_AzaleaTown_Text_83C407F, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_AzaleaTown_EventScript_83C3BAF::
	msgbox Johto_AzaleaTown_Text_83C40D6, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaTown object 5 (local id 5, gfx 0x2F) at 27,18
Johto_AzaleaTown_EventScript_83C3BBA::
	lock
	faceplayer
	msgbox Johto_AzaleaTown_Text_83C412F, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaTown object 4 (local id 4, gfx 0x23) at 16,23
Johto_AzaleaTown_EventScript_83C3BC7::
	lock
	faceplayer
	msgbox Johto_AzaleaTown_Text_83C417A, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaTown sign / bg event at 25,24
Johto_AzaleaTown_EventScript_83C3BD4::
	lock
	msgbox Johto_AzaleaTown_Text_83C4300, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_AzaleaTown sign / bg event at 17,11
Johto_AzaleaTown_EventScript_83C3BE0::
	lock
	msgbox Johto_AzaleaTown_Text_83C42F3, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_AzaleaTown sign / bg event at 35,23
Johto_AzaleaTown_EventScript_83C3BEC::
	lock
	msgbox Johto_AzaleaTown_Text_83C43F2, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_AzaleaTown sign / bg event at 13,15
Johto_AzaleaTown_EventScript_83C3BF8::
	lock
	msgbox Johto_AzaleaTown_Text_83C4400, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_AzaleaTown sign / bg event at 55,12
Johto_AzaleaTown_EventScript_83C3C04::
	lock
	msgbox Johto_AzaleaTown_Text_83C434B, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_AzaleaTown sign / bg event at 50,23
Johto_AzaleaTown_EventScript_83C3C10::
	lock
	msgbox Johto_AzaleaTown_Text_83C42B7, MSGBOX_SIGN
	closemessage
	release
	end

@ Johto_AzaleaTown trigger at 11,16 (VAR_QZ_0x5009 == 5)
Johto_AzaleaTown_EventScript_83C3C1C::
	lock
	clearflag FLAG_QZ_0x1086
	addobject 6
	playbgm MUS_QZ_648, 1
	applymovement 6, Johto_AzaleaTown_Movement_83C3D69
	waitmovement 0
	turnobject LOCALID_PLAYER, 4
	msgbox Johto_AzaleaTown_Text_83C3DA7, MSGBOX_DEFAULT
	closemessage
	call Johto_AzaleaTown_EventScript_83C3CCA
	msgbox Johto_AzaleaTown_Text_83C3E90, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_AzaleaTown_Movement_83C3D85
	waitmovement 0
	applymovement 6, Johto_AzaleaTown_Movement_83C3D7A
	waitmovement 0
	playse SE_EXIT
	setflag FLAG_QZ_0x1086
	removeobject 6
	setvar VAR_QZ_0x5009, 6
	fadedefaultbgm
	release
	end

@ Johto_AzaleaTown trigger at 11,17 (VAR_QZ_0x5009 == 5)
Johto_AzaleaTown_EventScript_83C3C71::
	lock
	clearflag FLAG_QZ_0x1086
	addobject 6
	playbgm MUS_QZ_648, 1
	applymovement 6, Johto_AzaleaTown_Movement_83C3D71
	waitmovement 0
	turnobject LOCALID_PLAYER, 4
	msgbox Johto_AzaleaTown_Text_83C3DA7, MSGBOX_DEFAULT
	closemessage
	call Johto_AzaleaTown_EventScript_83C3CCA
	playbgm MUS_QZ_649, 1
	msgbox Johto_AzaleaTown_Text_83C3E90, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, Johto_AzaleaTown_Movement_83C3D8A
	waitmovement 0
	applymovement 6, Johto_AzaleaTown_Movement_83C3D7F
	waitmovement 0
	playse SE_EXIT
	setflag FLAG_QZ_0x1086
	removeobject 6
	setvar VAR_QZ_0x5009, 6
	fadedefaultbgm
	release
	end

Johto_AzaleaTown_EventScript_83C3CCA::
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4721, 0, Johto_AzaleaTown_Text_83C3E3F
	return

Johto_AzaleaTown_EventScript_83C3CD5::
	lock
	applymovement LOCALID_PLAYER, Johto_AzaleaTown_Movement_83C3D9A
	waitmovement 0
	clearflag FLAG_QZ_0x108D
	addobject 8
	opendoor 0x13, 0xB
	waitdooranim
	waitmovement 0
	closedoor 0x13, 0xB
	waitdooranim
	applymovement 8, Johto_AzaleaTown_Movement_83C3D91
	playse SE_M_PETAL_DANCE
	waitmovement 0
	msgbox Johto_AzaleaTown_Text_83C423D, MSGBOX_DEFAULT
	closemessage
	playse SE_M_PETAL_DANCE
	msgbox Johto_AzaleaTown_Text_83C4268, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x36F
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	waitse
	playse SE_M_PETAL_DANCE
	turnobject 8, 3
	turnobject LOCALID_PLAYER, 3
	playse SE_M_PETAL_DANCE
	waitse
	turnobject 8, 1
	turnobject LOCALID_PLAYER, 2
	msgbox Johto_AzaleaTown_Text_83C4286, MSGBOX_DEFAULT
	closemessage
	applymovement 8, Johto_AzaleaTown_Movement_83C3DA3
	delay 0x3C
	opendoor 0x13, 0xB
	waitdooranim
	waitmovement 0
	setflag FLAG_QZ_0x108D
	removeobject 8
	closedoor 0x13, 0xB
	waitdooranim
	setvar VAR_QZ_0x5009, 9
	release
	end

Johto_AzaleaTown_Movement_83C3D69:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_AzaleaTown_Movement_83C3D71:
	walk_fast_down
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_AzaleaTown_Movement_83C3D7A:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Johto_AzaleaTown_Movement_83C3D7F:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_up
	walk_fast_left
	step_end

Johto_AzaleaTown_Movement_83C3D85:
	lock_facing_direction
	walk_fast_down
	unlock_facing_direction
	face_up
	step_end

Johto_AzaleaTown_Movement_83C3D8A:
	lock_facing_direction
	walk_fast_up
	unlock_facing_direction
	face_down
	step_end

Johto_AzaleaTown_Movement_83C3D91:
	walk_down
	walk_down
	walk_down
	face_right
	delay_8
	face_left
	delay_8
	face_down
	step_end

Johto_AzaleaTown_Movement_83C3D9A:
	walk_down
	walk_down
	walk_down
	face_left
	delay_16
	face_right
	delay_8
	face_up
	step_end

Johto_AzaleaTown_Movement_83C3DA3:
	walk_up
	walk_up
	walk_up
	step_end

Johto_AzaleaTown_Text_83C3DA7::
	.string "…Tell me something.\p"
	.string "Is it true that TEAM ROCKET\n"
	.string "has returned?\p"
	.string "What? You beat them? Hah! Quit\n"
	.string "lying.\p"
	.string "You’re not joking?\n"
	.string "Then let’s see how good you are.$"

Johto_AzaleaTown_Text_83C3E3F::
	.string "…Humph! Useless POKéMON!\p"
	.string "Listen, you. You only won because\n"
	.string "my POKéMON were weak.$"

Johto_AzaleaTown_Text_83C3E90::
	.string "I hate the weak.\p"
	.string "POKéMON, trainers. It doesn’t\n"
	.string "matter who or what.\p"
	.string "I’m going to be strong and wipe\n"
	.string "out the weak.\p"
	.string "That goes for TEAM ROCKET too.\p"
	.string "They act big and tough in a group.\p"
	.string "But get them alone, and they’re weak.\n"
	.string "I hate them all.\p"
	.string "You stay out of my way. A weakling\n"
	.string "like you is only a distraction.$"

Johto_AzaleaTown_Text_83C3FDC::
	.string "It’s unsafe to go in there,\n"
	.string "so I’m standing guard.\p"
	.string "Aren’t I a good Samaritan?$"

Johto_AzaleaTown_Text_83C402A::
	.string "Do you know about SLOWPOKETAIL?\n"
	.string "I heard it’s tasty!\p"
	.string "Aren’t you glad I told you that?$"

Johto_AzaleaTown_Text_83C407F::
	.string "The SLOWPOKE have disappeared\n"
	.string "from town…\p"
	.string "I heard their TAILS are being\n"
	.string "sold somewhere.$"

Johto_AzaleaTown_Text_83C40D6::
	.string "The SLOWPOKE have returned.\p"
	.string "Knowing them, they could’ve just\n"
	.string "been goofing off somewhere.$"

Johto_AzaleaTown_Text_83C412F::
	.string "Did you come to get KURT to make\n"
	.string "some BALLS?\p"
	.string "A lot of people do just that.$"

Johto_AzaleaTown_Text_83C417A::
	.string "Cut through AZALEA and you’ll be\n"
	.string "in ILEX FOREST.\p"
	.string "But these skinny trees make it\n"
	.string "impossible to get through.\p"
	.string "The CHARCOAL MAN’s POKéMON\n"
	.string "can CUT down trees.$"

Johto_AzaleaTown_Text_83C423D::
	.string "ILEX FOREST is restless!\p"
	.string "What is going on?$"

Johto_AzaleaTown_Text_83C4268::
	.string "{PLAYER}, here’s your GS BALL back!$"

Johto_AzaleaTown_Text_83C4286::
	.string "Could you go see why ILEX FOREST\n"
	.string "is so restless?$"

Johto_AzaleaTown_Text_83C42B7::
	.string "AZALEA TOWN\n"
	.string "Where People and POKéMON Live in\l"
	.string "Happy Harmony.$"

Johto_AzaleaTown_Text_83C42F3::
	.string "KURT’S HOUSE$"

Johto_AzaleaTown_Text_83C4300::
	.string "AZALEA TOWN\n"
	.string "POKéMON GYM\p"
	.string "LEADER: BUGSY\p"
	.string "The Walking Bug POKéMON\n"
	.string "Encyclopedia$"

Johto_AzaleaTown_Text_83C434B::
	.string "SLOWPOKE WELL\p"
	.string "Also known as the RAINMAKER WELL.\p"
	.string "Locals believe that a SLOWPOKE’s\n"
	.string "yawn summons rain.\p"
	.string "Records show that a SLOWPOKE’s\n"
	.string "yawn ended a drought 400 years ago.$"

Johto_AzaleaTown_Text_83C43F2::
	.string "CHARCOAL KILN$"

Johto_AzaleaTown_Text_83C4400::
	.string "ILEX FOREST\p"
	.string "Enter through the gate.$"
