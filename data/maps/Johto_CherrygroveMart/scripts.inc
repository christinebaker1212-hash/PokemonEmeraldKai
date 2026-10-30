Johto_CherrygroveMart_MapScripts::
	.byte 0

@ Johto_CherrygroveMart object 2 (local id 2, gfx 0x23) at 3,5
Johto_CherrygroveMart_EventScript_83C9947::
	lock
	faceplayer
	msgbox Johto_CherrygroveMart_Text_83C99F7, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_CherrygroveMart object 3 (local id 3, gfx 0x24) at 8,5
Johto_CherrygroveMart_EventScript_83C9954::
	lock
	faceplayer
	msgbox Johto_CherrygroveMart_Text_83C9988, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CherrygroveMart_Text_83C9988::
	.string "You can buy POKé BALLS even before\n"
	.string "earning your first Badge.$"

Johto_CherrygroveMart_Text_83C99F7::
	.string "When I was walking in the grass, a\n"
	.string "bug POKéMON poisoned my POKéMON!\p"
	.string "I just kept going, but then my\n"
	.string "POKéMON fainted.\p"
	.string "You should keep an ANTIDOTE\n"
	.string "with you.$"
