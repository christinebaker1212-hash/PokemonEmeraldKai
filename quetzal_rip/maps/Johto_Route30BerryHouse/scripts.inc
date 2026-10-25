Johto_Route30BerryHouse_MapScripts::
	.byte 0

@ Johto_Route30BerryHouse object 1 (local id 1, gfx 0x3E84) at 4,4
Johto_Route30BerryHouse_EventScript_83ED953::
	lock
	faceplayer
	goto_if_set FLAG_QZ_0x1024, Johto_Route30BerryHouse_EventScript_83ED981
	msgbox Johto_Route30BerryHouse_Text_83ED98C, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, 0x202
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
	.string "When you follow that path up north,\n"
	.string "you will see MR. POKéMON’s house.\p"
	.string "But before you go, allow me to\n"
	.string "tell you about BERRIES!\p"
	.string "POKéMON get healthier by eating\n"
	.string "BERRIES.\p"
	.string "They can even eat BERRIES\n"
	.string "they’re holding in battle.\p"
	.string "If you need special BALLS,\n"
	.string "KURT sells them in AZALEA TOWN.\p"
	.string "Here. I’ll share one with you!$"

Johto_Route30BerryHouse_Text_83EDAC1::
	.string "Check trees for BERRIES.\p"
	.string "If respawning is enabled, defeating\n"
	.string "POKéMON can replenish them.\p"
	.string "Waiting a few hours won’t do it.$"
