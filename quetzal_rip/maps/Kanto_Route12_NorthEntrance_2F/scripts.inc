Kanto_Route12_NorthEntrance_2F_MapScripts::
	.byte 0

@ Kanto_Route12_NorthEntrance_2F sign / bg event at 4,1
Kanto_Route12_NorthEntrance_2F_EventScript_841FDFE::
	msgbox Kanto_Route12_NorthEntrance_2F_Text_844F3F7, MSGBOX_SIGN
	end

@ Kanto_Route12_NorthEntrance_2F sign / bg event at 8,1
Kanto_Route12_NorthEntrance_2F_EventScript_841FE07::
	msgbox Kanto_Route12_NorthEntrance_2F_Text_844F43A, MSGBOX_SIGN
	end

@ Kanto_Route12_NorthEntrance_2F object 1 (local id 1, gfx 0x3F16) at 7,5
Kanto_Route12_NorthEntrance_2F_EventScript_841FE10::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x216D, Kanto_Route12_NorthEntrance_2F_EventScript_841FE8A
	checkplayergender
	call_if_eq VAR_RESULT, 0, Kanto_Route12_NorthEntrance_2F_EventScript_841FE6E
	call_if_eq VAR_RESULT, 1, Kanto_Route12_NorthEntrance_2F_EventScript_841FE77
	checkitemspace ITEM_SHINY_STONE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_NorthEntrance_2F_EventScript_841FE80
	additem ITEM_SHINY_STONE, 1
	loadword 0, Kanto_Route12_NorthEntrance_2F_Text_844F351
	setorcopyvar VAR_0x8000, 0xDA
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	msgbox Kanto_Route12_NorthEntrance_2F_Text_844F381, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x216D
	release
	end

Kanto_Route12_NorthEntrance_2F_EventScript_841FE6E::
	msgbox Kanto_Route12_NorthEntrance_2F_Text_844F283, MSGBOX_DEFAULT
	return

Kanto_Route12_NorthEntrance_2F_EventScript_841FE77::
	msgbox Kanto_Route12_NorthEntrance_2F_Text_844F2EA, MSGBOX_DEFAULT
	return

Kanto_Route12_NorthEntrance_2F_EventScript_841FE80::
	msgbox Kanto_Route12_NorthEntrance_2F_Text_844F3D9, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_NorthEntrance_2F_EventScript_841FE8A::
	msgbox Kanto_Route12_NorthEntrance_2F_Text_844F381, MSGBOX_DEFAULT
	release
	end

Kanto_Route12_NorthEntrance_2F_Text_844F283::
	.string "My POKéMON’s ashes are stored in\n"
	.string "POKéMON TOWER.\p"
	.string "Please take this SHINY STONE.\n"
	.string "I don’t need it anymore…$"

Kanto_Route12_NorthEntrance_2F_Text_844F2EA::
	.string "My POKéMON’s ashes are stored in\n"
	.string "POKéMON TOWER.\p"
	.string "Please take this SHINY STONE.\n"
	.string "I don’t need it anymore…$"

Kanto_Route12_NorthEntrance_2F_Text_844F351::
	.string "{PLAYER} received a SHINY STONE\n"
	.string "from the little girl.$"

Kanto_Route12_NorthEntrance_2F_Text_844F381::
	.string "A SHINY STONE can make certain\n"
	.string "POKéMON evolve.\p"
	.string "Please treat your POKéMON with\n"
	.string "kindness.$"

Kanto_Route12_NorthEntrance_2F_Text_844F3D9::
	.string "You don’t have room for this.$"

Kanto_Route12_NorthEntrance_2F_Text_844F3F7::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "There’s a man fishing!$"

Kanto_Route12_NorthEntrance_2F_Text_844F43A::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "It’s POKéMON TOWER!$"
