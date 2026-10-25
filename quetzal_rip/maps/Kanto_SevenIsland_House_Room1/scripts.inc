Kanto_SevenIsland_House_Room1_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SevenIsland_House_Room1_EventScript_84209D8
	.byte 0

Kanto_SevenIsland_House_Room1_EventScript_84209D8::
	setmaplayoutindex 0x3DB
	setobjectxyperm 1, 4, 4
	setobjectmovementtype 1, 0xA
	end

@ Kanto_SevenIsland_House_Room1 object 1 (local id 1, gfx 0x3F2F) at 4,4
Kanto_SevenIsland_House_Room1_EventScript_84209E7::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x2EB0, Kanto_SevenIsland_House_Room1_EventScript_8420A08
	goto_if_set FLAG_QZ_0x2EB1, Kanto_SevenIsland_House_Room1_EventScript_8420A11
	msgbox Kanto_SevenIsland_House_Room1_Text_845094B, MSGBOX_DEFAULT
	goto Kanto_SevenIsland_House_Room1_EventScript_8420A19

Kanto_SevenIsland_House_Room1_EventScript_8420A08::
	goto_if_set FLAG_QZ_0x2EB1, Kanto_SevenIsland_House_Room1_EventScript_8420A73

Kanto_SevenIsland_House_Room1_EventScript_8420A11::
	msgbox Kanto_SevenIsland_House_Room1_Text_8450A32, MSGBOX_DEFAULT

Kanto_SevenIsland_House_Room1_EventScript_8420A19::
	goto_if_set FLAG_QZ_0x2EB0, Kanto_SevenIsland_House_Room1_EventScript_8420A3C
	setorcopyvar VAR_0x8000, 0x23C
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_0x8007, 0, Kanto_SevenIsland_House_Room1_EventScript_8420A69
	setflag FLAG_QZ_0x2EB0

Kanto_SevenIsland_House_Room1_EventScript_8420A3C::
	goto_if_set FLAG_QZ_0x2EB1, Kanto_SevenIsland_House_Room1_EventScript_8420A5F
	setorcopyvar VAR_0x8000, 0x23D
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_0x8007, 0, Kanto_SevenIsland_House_Room1_EventScript_8420A69
	setflag FLAG_QZ_0x2EB1

Kanto_SevenIsland_House_Room1_EventScript_8420A5F::
	msgbox Kanto_SevenIsland_House_Room1_Text_8450A72, MSGBOX_DEFAULT
	release
	end

Kanto_SevenIsland_House_Room1_EventScript_8420A69::
	msgbox Kanto_SevenIsland_House_Room1_Text_8450AA3, MSGBOX_DEFAULT
	release
	end

Kanto_SevenIsland_House_Room1_EventScript_8420A73::
	msgbox Kanto_SevenIsland_House_Room1_Text_8450ADD, MSGBOX_DEFAULT
	release
	end

@ Kanto_SevenIsland_House_Room1 sign / bg event at 4,1
Kanto_SevenIsland_House_Room1_EventScript_8420A7D::
	msgbox Kanto_SevenIsland_House_Room1_Text_84509FB, MSGBOX_SIGN
	end

Kanto_SevenIsland_House_Room1_Text_845094B::
	.string "When I was young, I traveled the\n"
	.string "islands looking for new rivals.\p"
	.string "These Berries were with me\n"
	.string "through many battles.\p"
	.string "Now I’d like them to travel with\n"
	.string "you. Take good care of them!$"

Kanto_SevenIsland_House_Room1_Text_84509FB::
	.string "The box holds keepsakes from\n"
	.string "past travels and battles.$"

Kanto_SevenIsland_House_Room1_Text_8450A32::
	.string "I still have a gift for you.\n"
	.string "Let’s see if you can carry it now.$"

Kanto_SevenIsland_House_Room1_Text_8450A72::
	.string "May they serve you well on\n"
	.string "your next adventures!$"

Kanto_SevenIsland_House_Room1_Text_8450AA3::
	.string "I’ll keep the rest for you.\n"
	.string "Come back when you have room.$"

Kanto_SevenIsland_House_Room1_Text_8450ADD::
	.string "A held Berry can change\n"
	.string "the course of a battle.\p"
	.string "Choose one that suits each\n"
	.string "of your Pokémon.$"
