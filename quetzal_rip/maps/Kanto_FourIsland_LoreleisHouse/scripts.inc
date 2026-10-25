Kanto_FourIsland_LoreleisHouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_FourIsland_LoreleisHouse_EventScript_8421967
	.byte 0

Kanto_FourIsland_LoreleisHouse_EventScript_8421967::
	special QzUpdateLoreleiDollCollection_774
	end

@ Kanto_FourIsland_LoreleisHouse object 1 (local id 1, gfx 0x3F19) at 7,4
Kanto_FourIsland_LoreleisHouse_EventScript_842196B::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2218, Kanto_FourIsland_LoreleisHouse_EventScript_8421996
	goto_if_set FLAG_QZ_0x2124, Kanto_FourIsland_LoreleisHouse_EventScript_8421989
	msgbox Kanto_FourIsland_LoreleisHouse_Text_8453DBE, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_LoreleisHouse_EventScript_8421989::
	setflag FLAG_QZ_0x2218
	msgbox Kanto_FourIsland_LoreleisHouse_Text_8453EA4, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_LoreleisHouse_EventScript_8421996::
	msgbox Kanto_FourIsland_LoreleisHouse_Text_8453F70, MSGBOX_DEFAULT
	release
	end

@ Kanto_FourIsland_LoreleisHouse object 2 (local id 2, gfx 0x3F56) at 0,2
@ Kanto_FourIsland_LoreleisHouse object 3 (local id 3, gfx 0x3F45) at 10,3
@ Kanto_FourIsland_LoreleisHouse object 4 (local id 4, gfx 0x3F34) at 9,3
@ Kanto_FourIsland_LoreleisHouse object 5 (local id 5, gfx 0x3F47) at 7,2
@ Kanto_FourIsland_LoreleisHouse object 6 (local id 6, gfx 0x3F46) at 6,2
@ Kanto_FourIsland_LoreleisHouse object 7 (local id 7, gfx 0x3F39) at 6,4
@ Kanto_FourIsland_LoreleisHouse object 8 (local id 8, gfx 0x3F1F) at 9,4
@ Kanto_FourIsland_LoreleisHouse object 9 (local id 9, gfx 0x3EF7) at 10,4
@ Kanto_FourIsland_LoreleisHouse object 10 (local id 10, gfx 0x3F0F) at 0,3
@ Kanto_FourIsland_LoreleisHouse object 11 (local id 11, gfx 0x3F27) at 0,5
@ Kanto_FourIsland_LoreleisHouse object 12 (local id 12, gfx 0x3F28) at 1,5
@ Kanto_FourIsland_LoreleisHouse object 13 (local id 13, gfx 0x3F32) at 9,0
@ Kanto_FourIsland_LoreleisHouse object 14 (local id 14, gfx 0x3F05) at 10,0
@ Kanto_FourIsland_LoreleisHouse object 15 (local id 15, gfx 0x3F15) at 9,6
Kanto_FourIsland_LoreleisHouse_EventScript_84219A0::
	lock
	msgbox Kanto_FourIsland_LoreleisHouse_Text_8453FDA, MSGBOX_DEFAULT
	release
	end

Kanto_FourIsland_LoreleisHouse_Text_8453DBE::
	.string "LORELEI: There’s something weighing\n"
	.string "heavily on my mind.\p"
	.string "If anything were to happen on\n"
	.string "the island where I was born…\p"
	.string "I wouldn’t know about it if I were\n"
	.string "in the POKéMON LEAGUE.\p"
	.string "I wonder if that would make me\n"
	.string "irresponsible to my home…$"

Kanto_FourIsland_LoreleisHouse_Text_8453EA4::
	.string "LORELEI: So, you managed to solve\n"
	.string "all the problems here?\p"
	.string "That’s wonderful.\p"
	.string "That means there isn’t any reason\n"
	.string "for me to be here all the time.\p"
	.string "Thank you…\p"
	.string "I’ll return to the POKéMON LEAGUE\n"
	.string "in a short while.$"

Kanto_FourIsland_LoreleisHouse_Text_8453F70::
	.string "I don’t know what will happen in\n"
	.string "the future, but…\p"
	.string "I will do what I can here and now.\n"
	.string "That’s all I can do.$"

Kanto_FourIsland_LoreleisHouse_Text_8453FDA::
	.string "Stuffed POKéMON dolls galore!$"
