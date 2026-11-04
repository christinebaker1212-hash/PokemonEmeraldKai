Johto_Route30BerryHouse_MapScripts::
	.byte 0

@ Johto_Route30BerryHouse object 1 (local id 1, gfx 0x3E84) at 4,4
Johto_Route30BerryHouse_EventScript_83ED953::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1024, Johto_Route30BerryHouse_EventScript_83ED981
	msgbox Johto_Route30BerryHouse_Text_83ED98C, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_CHERI_BERRY
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	closemessage
	setflag FLAG_QZ_0x1024

Johto_Route30BerryHouse_EventScript_83ED981::
	msgbox Johto_Route30BerryHouse_Text_83EDAC1, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route30BerryHouse_Text_83ED98C::
	@ Moemon Kai: Crystal's words (pokecrystal maps/Route30BerryHouse.asm)
	.string "You know, MOéMON\n"
	.string "eat BERRIES.\p"
	.string "Well, my MOéMON\n"
	.string "got healthier by eating a BERRY.\p"
	.string "Here. I’ll share one with you!$"

Johto_Route30BerryHouse_Text_83EDAC1::
	.string "Check trees for BERRIES.\n"
	.string "They just drop right off.$"
