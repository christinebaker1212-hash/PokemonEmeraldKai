Kanto_SaffronCity_MrPsychicsHouse_MapScripts::
	.byte 0

@ Kanto_SaffronCity_MrPsychicsHouse object 1 (local id 1, gfx 0x3EE6) at 7,4
Kanto_SaffronCity_MrPsychicsHouse_EventScript_841F101::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x216F, Kanto_SaffronCity_MrPsychicsHouse_EventScript_841F15A
	msgbox Kanto_SaffronCity_MrPsychicsHouse_Text_844C09D, MSGBOX_DEFAULT
	@ Moemon Kai: FireRed's TM (tools/quetzal_port/fix_kanto_tms.py) (Quetzal gave MOON_STONE)
	checkitemspace ITEM_TM29, 1
	goto_if_eq VAR_RESULT, 0, Kanto_SaffronCity_MrPsychicsHouse_EventScript_841F150
	additem ITEM_TM29, 1
	loadword 0, Kanto_SaffronCity_MrPsychicsHouse_Text_844C0C7
	setorcopyvar VAR_0x8000, ITEM_TM29
	setorcopyvar VAR_0x8001, 1
	setorcopyvar VAR_0x8002, 0x16F
	call Quetzal_EventScript_845571E
	msgbox Kanto_SaffronCity_MrPsychicsHouse_Text_844C0F2, MSGBOX_DEFAULT
	setflag FLAG_QZ_0x216F
	release
	end

Kanto_SaffronCity_MrPsychicsHouse_EventScript_841F150::
	msgbox Kanto_SaffronCity_MrPsychicsHouse_Text_844C141, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_MrPsychicsHouse_EventScript_841F15A::
	msgbox Kanto_SaffronCity_MrPsychicsHouse_Text_844C0F2, MSGBOX_DEFAULT
	release
	end

Kanto_SaffronCity_MrPsychicsHouse_Text_844C09D::
	.string "…Wait! Don’t say a word!\p"
	.string "You wanted this!$"

Kanto_SaffronCity_MrPsychicsHouse_Text_844C0C7::
	.string "{PLAYER} received TM29\n"
	.string "from MR. PSYCHIC.$"

Kanto_SaffronCity_MrPsychicsHouse_Text_844C0F2::
	.string "You already know, don’t you?\n"
	.string "TM29 is PSYCHIC.\p"
	.string "It’s an attack that may also lower\n"
	.string "the target’s special stats.$"

Kanto_SaffronCity_MrPsychicsHouse_Text_844C141::
	.string "Where do you plan to put this?\n"
	.string "You’ve no room.$"
