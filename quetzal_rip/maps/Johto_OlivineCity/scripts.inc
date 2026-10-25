Johto_OlivineCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_83E3F77
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_OlivineCity_OnFrame_83E3F8F
	map_script MAP_SCRIPT_ON_LOAD, Quetzal_EventScript_83C2D44
	map_script MAP_SCRIPT_ON_RESUME, Quetzal_EventScript_83C2D44
	.byte 0

Johto_OlivineCity_OnFrame_83E3F8F:
	map_script_2 VAR_QZ_0x500F, 1, Johto_OlivineCity_EventScript_83E3FA1
	map_script_2 VAR_QZ_0x500C, 15, Quetzal_EventScript_83DF6B4
	.2byte 0

Johto_OlivineCity_EventScript_83E3FA1::
	lock
	msgbox Johto_OlivineCity_Text_83E3FB1, MSGBOX_DEFAULT
	setvar VAR_QZ_0x500F, 2
	release
	end

@ Johto_OlivineCity trigger at 15,28 (VAR_QZ_0x500D == 1)
@ Johto_OlivineCity trigger at 15,29 (VAR_QZ_0x500D == 1)
@ Johto_OlivineCity trigger at 15,30 (VAR_QZ_0x500D == 1)
Johto_OlivineCity_EventScript_83E4026::
	lock
	clearflag FLAG_QZ_0x10B5
	opendoor 0xB, 0x1B
	addobject 4
	playbgm MUS_QZ_648, 1
	delay 0x14
	closedoor 0xB, 0x1B
	getplayerxy VAR_TEMP_0, VAR_TEMP_1
	call_if_eq VAR_TEMP_1, 28, Johto_OlivineCity_EventScript_83E40A6
	call_if_eq VAR_TEMP_1, 29, Johto_OlivineCity_EventScript_83E40B1
	call_if_eq VAR_TEMP_1, 30, Johto_OlivineCity_EventScript_83E40BC
	msgbox Johto_OlivineCity_Text_83E40FC, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_TEMP_1, 28, Johto_OlivineCity_EventScript_83E40C7
	call_if_eq VAR_TEMP_1, 29, Johto_OlivineCity_EventScript_83E40D2
	call_if_eq VAR_TEMP_1, 30, Johto_OlivineCity_EventScript_83E40D2
	applymovement 4, Johto_OlivineCity_Movement_83E40EF
	waitmovement 0
	setvar VAR_QZ_0x500D, 2
	setflag FLAG_QZ_0x10B5
	removeobject 4
	fadedefaultbgm
	release
	end

Johto_OlivineCity_EventScript_83E40A6::
	applymovement 4, Johto_OlivineCity_Movement_83E40DD
	waitmovement 0
	return

Johto_OlivineCity_EventScript_83E40B1::
	applymovement 4, Johto_OlivineCity_Movement_83E40E2
	waitmovement 0
	return

Johto_OlivineCity_EventScript_83E40BC::
	applymovement 4, Johto_OlivineCity_Movement_83E40E8
	waitmovement 0
	return

Johto_OlivineCity_EventScript_83E40C7::
	applymovement 4, Quetzal_Movement_8360839
	waitmovement 0
	return

Johto_OlivineCity_EventScript_83E40D2::
	applymovement 4, Quetzal_Movement_8360837
	waitmovement 0
	return

@ Johto_OlivineCity object 3 (local id 3, gfx 0x31) at 11,44
Johto_OlivineCity_EventScript_83E4278::
	lock
	faceplayer
	msgbox Johto_OlivineCity_Text_83E4429, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineCity object 2 (local id 2, gfx 0x31) at 33,46
Johto_OlivineCity_EventScript_83E4285::
	lock
	faceplayer
	msgbox Johto_OlivineCity_Text_83E4532, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineCity object 1 (local id 1, gfx 0x23) at 21,28
Johto_OlivineCity_EventScript_83E4292::
	lock
	faceplayer
	msgbox Johto_OlivineCity_Text_83E44D3, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineCity sign / bg event at 22,24
Johto_OlivineCity_EventScript_83E429F::
	lock
	msgbox Johto_OlivineCity_Text_83E4617, MSGBOX_SIGN
	end

@ Johto_OlivineCity sign / bg event at 22,52
Johto_OlivineCity_EventScript_83E42A9::
	lock
	msgbox Johto_OlivineCity_Text_83E4646, MSGBOX_SIGN
	end

@ Johto_OlivineCity sign / bg event at 7,44
Johto_OlivineCity_EventScript_83E42B3::
	msgbox Johto_OlivineCity_Text_83E431C, MSGBOX_SIGN
	end

@ Johto_OlivineCity sign / bg event at 13,27
Johto_OlivineCity_EventScript_83E42BC::
	lock
	msgbox Johto_OlivineCity_Text_83E4662, MSGBOX_SIGN
	end

@ Johto_OlivineCity sign / bg event at 36,48
Johto_OlivineCity_EventScript_83E42C6::
	lock
	msgbox Johto_OlivineCity_Text_83E46A7, MSGBOX_SIGN
	end

@ Johto_OlivineCity object 7 (local id 7, gfx 0x3E82) at 26,52
Johto_OlivineCity_EventScript_83E42DD::
	lock
	faceplayer
	checkitem ITEM_AURORA_TICKET, 1
	goto_if_eq VAR_RESULT, 1, Johto_OlivineCity_EventScript_83E4311
	msgbox Johto_OlivineCity_Text_83E435D, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineCity_EventScript_83E4311::
	msgbox Johto_OlivineCity_Text_83E4403, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineCity_Movement_83E40DD:
	walk_down
	walk_right
	walk_right
	walk_right
	step_end

Johto_OlivineCity_Movement_83E40E2:
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	step_end

Johto_OlivineCity_Movement_83E40E8:
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	step_end

Johto_OlivineCity_Movement_83E40EF:
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

Johto_OlivineCity_Text_83E3FB1::
	.string "News has spread that the new\n"
	.string "SAFARI ZONE is finally open!\p"
	.string "Its GATE is beyond the cavern\n"
	.string "near CIANWOOD CITY.$"

Johto_OlivineCity_Text_83E40FC::
	.string "You again?\n"
	.string "There’s no need to be alert.\p"
	.string "I don’t bother with wimps\n"
	.string "like you.\p"
	.string "Speaking of weaklings, the\n"
	.string "city’s GYM LEADER isn’t here,\l"
	.string "and is supposedly taking care\l"
	.string "of a sick POKéMON at the\l"
	.string "LIGHTHOUSE.\p"
	.string "Humph! Boo-hoo!\n"
	.string "Just let sick POKéMON go!\p"
	.string "A POKéMON that can’t battle\n"
	.string "is worthless!\p"
	.string "Why don’t you go train at the\n"
	.string "LIGHTHOUSE?\p"
	.string "Who knows? It may make you a\n"
	.string "bit of a better Trainer!$"

Johto_OlivineCity_Text_83E431C::
	.string "OLIVINE CAFE\n"
	.string "WHERE STRONG SAILORS ENJOY\l"
	.string "HEARTY MEALS OF STRENGTH$"

Johto_OlivineCity_Text_83E435D::
	.string "My name is MARY!\p"
	.string "Just kidding, but I love MARY’s\n"
	.string "radio show!$"

Johto_OlivineCity_Text_83E4403::
	.string "I can’t believe I met the\n"
	.string "champion!!!$"

Johto_OlivineCity_Text_83E4429::
	.string "Dark roads are dangerous\n"
	.string "at night.\p"
	.string "But in the pitch-black of\n"
	.string "night, the sea is even more\l"
	.string "treacherous!\p"
	.string "Without the beacon of the\n"
	.string "LIGHTHOUSE to guide it,\l"
	.string "no ship can sail.$"

Johto_OlivineCity_Text_83E44D3::
	.string "Wow, you have a POKéDEX!\n"
	.string "That’s really cool!$"

Johto_OlivineCity_Text_83E4532::
	.string "The league has JASMINE\n"
	.string "pegged as the sixth\l"
	.string "strongest GYM LEADER!\p"
	.string "Hah! That’s a load of crud.\p"
	.string "There’s no way PRYCE could\n"
	.string "hurt JASMINE’s STEEL-type\l"
	.string "POKéMON!\p"
	.string "Bureaucracy if you ask me.\p"
	.string "She’s something to be feared,\n"
	.string "that’s for sure!$"

Johto_OlivineCity_Text_83E4617::
	.string "OLIVINE CITY\p"
	.string "The Port Closest to Foreign Lands$"

Johto_OlivineCity_Text_83E4646::
	.string "OLIVINE PORT\n"
	.string "FAST SHIP PIER$"

Johto_OlivineCity_Text_83E4662::
	.string "OLIVINE CITY POKéMON GYM\n"
	.string "LEADER: JASMINE\p"
	.string "The Steel-Clad Defense Girl$"

Johto_OlivineCity_Text_83E46A7::
	.string "OLIVINE LIGHTHOUSE\n"
	.string "Also known as the GLITTER\l"
	.string "LIGHTHOUSE$"
