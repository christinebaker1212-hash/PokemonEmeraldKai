Johto_AzaleaMart_MapScripts::
	.byte 0

@ Johto_AzaleaMart object 2 (local id 2, gfx 0x3EAE) at 3,5
Johto_AzaleaMart_EventScript_83C63FF::
	lock
	faceplayer
	msgbox Johto_AzaleaMart_Text_83C6419, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_AzaleaMart object 1 (local id 1, gfx 0x24) at 6,2
Johto_AzaleaMart_EventScript_83C640C::
	lock
	faceplayer
	msgbox Johto_AzaleaMart_Text_83C64D7, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_AzaleaMart_Text_83C6419::
	.string "Are you able to buy a GREAT BALL?\n"
	.string "All I can buy are POKé BALLS.\p"
	.string "I heard that more items become\n"
	.string "available to buy when you earn\l"
	.string "GYM BADGES!\p"
	.string "I wish KURT would make me some\n"
	.string "of his custom BALLS.$"

Johto_AzaleaMart_Text_83C64D7::
	.string "A GREAT BALL is better for catch-\n"
	.string "ing POKéMON than a POKé BALL.\p"
	.string "But KURT’s might be better some-\n"
	.string "times.$"
