Kanto_SaffronCity_MrPsychicsHouse_MapScripts::
	.byte 0

@ Kanto_SaffronCity_MrPsychicsHouse object 1 (local id 1, gfx 0x3EE6) at 7,4
Kanto_SaffronCity_MrPsychicsHouse_EventScript_841F101::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x216F, Kanto_SaffronCity_MrPsychicsHouse_EventScript_841F15A
	msgbox Kanto_SaffronCity_MrPsychicsHouse_Text_844C09D, MSGBOX_DEFAULT
	checkitemspace ITEM_MOON_STONE, 1
	goto_if_eq VAR_RESULT, 0, Kanto_SaffronCity_MrPsychicsHouse_EventScript_841F150
	additem ITEM_MOON_STONE, 1
	loadword 0, Kanto_SaffronCity_MrPsychicsHouse_Text_844C0C7
	setorcopyvar VAR_0x8000, 0xD9
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
	.string "{PLAYER} received a MOON STONE\n"
	.string "from MR. PSYCHIC.$"

Kanto_SaffronCity_MrPsychicsHouse_Text_844C0F2::
	.string "You already know, don’t you?\n"
	.string "A MOON STONE causes evolution in\l"
	.string "certain POKéMON.$"

Kanto_SaffronCity_MrPsychicsHouse_Text_844C141::
	.string "Where do you plan to put this?\n"
	.string "You’ve no room.$"
