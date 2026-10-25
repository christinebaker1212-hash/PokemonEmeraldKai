Johto_RuinsOfAlphInnerChamber_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_RuinsOfAlphInnerChamber_OnFrame_83F6D14
	.byte 0

Johto_RuinsOfAlphInnerChamber_OnFrame_83F6D14:
	map_script_2 VAR_QZ_0x501C, 1, Johto_RuinsOfAlphInnerChamber_EventScript_83F6D1E
	.2byte 0

Johto_RuinsOfAlphInnerChamber_EventScript_83F6D1E::
	lock
	msgbox Johto_RuinsOfAlphInnerChamber_Text_83F6D97, MSGBOX_DEFAULT
	closemessage
	setvar VAR_QZ_0x501C, 0
	release
	end

@ Johto_RuinsOfAlphInnerChamber object 3 (local id 3, gfx 0x14) at 19,16
Johto_RuinsOfAlphInnerChamber_EventScript_83F6D2F::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphInnerChamber_Text_83F6DE7, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphInnerChamber object 1 (local id 1, gfx 0x37) at 18,10
Johto_RuinsOfAlphInnerChamber_EventScript_83F6D3B::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphInnerChamber_Text_83F6DB9, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphInnerChamber object 2 (local id 2, gfx 0x3EA6) at 13,23
Johto_RuinsOfAlphInnerChamber_EventScript_83F6D47::
	lock
	faceplayer
	msgbox Johto_RuinsOfAlphInnerChamber_Text_83F6E30, MSGBOX_DEFAULT
	release
	end

@ Johto_RuinsOfAlphInnerChamber object 6 (local id 6, gfx 0xC7) at 5,21
Johto_RuinsOfAlphInnerChamber_EventScript_83F6D53::
	setorcopyvar VAR_0x8000, 0xA5
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphInnerChamber object 4 (local id 4, gfx 0xC7) at 21,4
Johto_RuinsOfAlphInnerChamber_EventScript_83F6D60::
	setorcopyvar VAR_0x8000, 0xA6
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphInnerChamber object 5 (local id 5, gfx 0xC7) at 21,21
Johto_RuinsOfAlphInnerChamber_EventScript_83F6D6D::
	setorcopyvar VAR_0x8000, 0xA7
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphInnerChamber object 7 (local id 7, gfx 0x3A9E) at 5,4
Johto_RuinsOfAlphInnerChamber_EventScript_83F6D7A::
	checkitem ITEM_GS_BALL, 1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FD2E
	setorcopyvar VAR_0x8000, 0x36F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_RuinsOfAlphInnerChamber sign / bg event at 6,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 10,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 11,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 12,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 13,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 14,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 15,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 16,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 17,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 18,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 19,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 20,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 21,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 23,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 23,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 19,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 18,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 17,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 16,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 15,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 14,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 13,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 12,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 11,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 10,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 9,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 8,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 6,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 6,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 10,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 11,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 12,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 13,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 14,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 15,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 16,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 17,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 18,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 19,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 20,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 21,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 23,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 23,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 22,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 19,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 18,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 17,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 16,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 15,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 14,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 13,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 12,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 11,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 10,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 9,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 8,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 7,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 6,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 27,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 26,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 24,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 27,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 26,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 24,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 27,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 26,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 24,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 5,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 3,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 2,7
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 5,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 3,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 2,13
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 5,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 3,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 2,19
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 5,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 4,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 3,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 2,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 27,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 26,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 25,25
@ Johto_RuinsOfAlphInnerChamber sign / bg event at 24,25
Johto_RuinsOfAlphInnerChamber_EventScript_83F805A::
	lock
	msgbox Johto_RuinsOfAlphInnerChamber_Text_83F862E, MSGBOX_DEFAULT
	release
	end

Johto_RuinsOfAlphInnerChamber_Text_83F6D97::
	.string "There is a strange presence here.$"

Johto_RuinsOfAlphInnerChamber_Text_83F6DB9::
	.string "This is a big room, but there’s\n"
	.string "nothing here.$"

Johto_RuinsOfAlphInnerChamber_Text_83F6DE7::
	.string "This place has a mystical\n"
	.string "quality to it.\p"
	.string "It feels sort of ethereal\n"
	.string "even.$"

Johto_RuinsOfAlphInnerChamber_Text_83F6E30::
	.string "Ancient buildings are often\n"
	.string "tombs of kings.\p"
	.string "Like the pyramids, for\n"
	.string "instance.$"

Johto_RuinsOfAlphInnerChamber_Text_83F862E::
	.string "It’s UNOWN text!$"
