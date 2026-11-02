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
	@ Moemon Kai: FireRed's TM (tools/quetzal_port/fix_kanto_tms.py) (Quetzal gave SHINY_STONE)
	checkitemspace ITEM_TM27, 1
	goto_if_eq VAR_RESULT, 0, Kanto_Route12_NorthEntrance_2F_EventScript_841FE80
	additem ITEM_TM27, 1
	loadword 0, Kanto_Route12_NorthEntrance_2F_Text_844F351
	setorcopyvar VAR_0x8000, ITEM_TM27
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
	.string "My MOéMON’s ashes are stored in\n"
	.string "MOéMON TOWER.\p"
	.string "You can have this TM.\n"
	.string "I don’t need it anymore…$"

Kanto_Route12_NorthEntrance_2F_Text_844F2EA::
	.string "My MOéMON’s ashes are stored in\n"
	.string "MOéMON TOWER.\p"
	.string "You can have this TM.\n"
	.string "I don’t need it anymore…$"

Kanto_Route12_NorthEntrance_2F_Text_844F351::
	.string "{PLAYER} received TM27\n"
	.string "from the little girl.$"

Kanto_Route12_NorthEntrance_2F_Text_844F381::
	.string "TM27 is a move called RETURN…\p"
	.string "If you treat your MOéMON good,\n"
	.string "it will return your love by working\l"
	.string "its hardest in battle.$"

Kanto_Route12_NorthEntrance_2F_Text_844F3D9::
	.string "You don’t have room for this.$"

Kanto_Route12_NorthEntrance_2F_Text_844F3F7::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "There’s a man fishing!$"

Kanto_Route12_NorthEntrance_2F_Text_844F43A::
	.string "Let’s see what the binoculars have\n"
	.string "to show…\p"
	.string "It’s MOéMON TOWER!$"
