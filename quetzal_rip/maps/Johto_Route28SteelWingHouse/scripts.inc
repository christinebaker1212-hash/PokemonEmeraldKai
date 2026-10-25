Johto_Route28SteelWingHouse_MapScripts::
	.byte 0

@ Johto_Route28SteelWingHouse object 1 (local id 1, gfx 0x3E8F) at 4,4
Johto_Route28SteelWingHouse_EventScript_83ECA7C::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1062, Johto_Route28SteelWingHouse_EventScript_83ECAA9
	msgbox Johto_Route28SteelWingHouse_Text_83ECAB3, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x1B9
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_QZ_0x1062

Johto_Route28SteelWingHouse_EventScript_83ECAA9::
	msgbox Johto_Route28SteelWingHouse_Text_83ECB23, MSGBOX_DEFAULT
	release
	end

Johto_Route28SteelWingHouse_Text_83ECAB3::
	.string "Oh, dear. You’ve found me.\p"
	.string "Please don’t tell anyone about me.\p"
	.string "I’ll give you this for keeping my\n"
	.string "secret. Please?$"

Johto_Route28SteelWingHouse_Text_83ECB23::
	.string "It’s tough being a top celebrity.\p"
	.string "Everywhere I go, people chase me.\p"
	.string "I just want to be left alone…$"
