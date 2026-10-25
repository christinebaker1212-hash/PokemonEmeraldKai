Kanto_PalletTown_RivalsHouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_PalletTown_RivalsHouse_EventScript_8418C12
	.byte 0

Kanto_PalletTown_RivalsHouse_EventScript_8418C12::
	call_if_lt VAR_QZ_0x511B, 2, Kanto_PalletTown_RivalsHouse_EventScript_8418C29
	call_if_ge VAR_QZ_0x511B, 2, Kanto_PalletTown_RivalsHouse_EventScript_8418C35
	end

Kanto_PalletTown_RivalsHouse_EventScript_8418C29::
	setobjectmovementtype 1, 0xA
	setobjectxyperm 1, 5, 4
	return

Kanto_PalletTown_RivalsHouse_EventScript_8418C35::
	setvar VAR_TEMP_1, 1
	return

@ Kanto_PalletTown_RivalsHouse object 1 (local id 1, gfx 0x3F00) at 10,6
Kanto_PalletTown_RivalsHouse_EventScript_8418C3B::
	lock
	faceplayer
	setvar VAR_0x8004, 1
	setvar VAR_0x8005, 2
	special QzQuestLogNop_737
	goto_if_set FLAG_QZ_0x2108, Kanto_PalletTown_RivalsHouse_EventScript_8418C9E
	goto_if_eq VAR_TEMP_1, 1, Kanto_PalletTown_RivalsHouse_EventScript_8418D62
	goto_if_eq VAR_QZ_0x511B, 2, Kanto_PalletTown_RivalsHouse_EventScript_8418D6C
	goto_if_eq VAR_QZ_0x511B, 1, Kanto_PalletTown_RivalsHouse_EventScript_8418D6C
	goto_if_ge VAR_QZ_0x511A, 1, Kanto_PalletTown_RivalsHouse_EventScript_8418C94
	msgbox Kanto_PalletTown_RivalsHouse_Text_843D8A4, MSGBOX_DEFAULT
	closemessage
	applymovement 1, Quetzal_Movement_8457AA8
	waitmovement 0
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_8418C94::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843D8DE, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_8418C9E::
	msgbox Kanto_PalletTown_RivalsHouse_Text_8418D96, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_8418D62::
	msgbox Kanto_PalletTown_RivalsHouse_Text_8418D96, MSGBOX_DEFAULT
	release
	end

Kanto_PalletTown_RivalsHouse_EventScript_8418D6C::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843D9CB, MSGBOX_DEFAULT
	setvar VAR_QZ_0x511B, 2
	release
	end

@ Kanto_PalletTown_RivalsHouse object 2 (local id 2, gfx 0x3F50) at 6,4
Kanto_PalletTown_RivalsHouse_EventScript_8418D7B::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843DA7F, MSGBOX_NPC
	end

@ Kanto_PalletTown_RivalsHouse sign / bg event at 12,1
@ Kanto_PalletTown_RivalsHouse sign / bg event at 11,1
Kanto_PalletTown_RivalsHouse_EventScript_8418D84::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843DABD, MSGBOX_SIGN
	end

@ Kanto_PalletTown_RivalsHouse sign / bg event at 9,1
Kanto_PalletTown_RivalsHouse_EventScript_8418D8D::
	msgbox Kanto_PalletTown_RivalsHouse_Text_843DF5F, MSGBOX_SIGN
	end

Kanto_PalletTown_RivalsHouse_Text_8418D96::
	.string "Want to know how your POKéMON\n"
	.string "evolves? Check the POKéDEX.\p"
	.string "It doesn’t depend on friendship\n"
	.string "here. The POKéDEX tells you how!$"

Kanto_PalletTown_RivalsHouse_Text_843D8A4::
	.string "DAISY: Hi, {PLAYER}!\p"
	.string "My brother, BLUE, is out at\n"
	.string "Grandpa’s LAB.$"

Kanto_PalletTown_RivalsHouse_Text_843D8DE::
	.string "DAISY: {PLAYER}, I heard you had\n"
	.string "a battle against BLUE.\p"
	.string "I wish I’d seen that!$"

Kanto_PalletTown_RivalsHouse_Text_843D9CB::
	.string "You can use the TOWN MAP to find\n"
	.string "out where you are, or check the\l"
	.string "names of places.$"

Kanto_PalletTown_RivalsHouse_Text_843DA7F::
	.string "It’s a big map of the KANTO region.\n"
	.string "Now this would be useful!$"

Kanto_PalletTown_RivalsHouse_Text_843DABD::
	.string "The shelves are crammed full of\n"
	.string "books on POKéMON.$"

Kanto_PalletTown_RivalsHouse_Text_843DF5F::
	.string "“The lovely and sweet\n"
	.string "CLEFAIRY”$"
