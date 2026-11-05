Johto_CherrygroveMart_MapScripts::
	.byte 0

@ Johto_CherrygroveMart object 2 (local id 2, gfx 0x23) at 3,5
Johto_CherrygroveMart_EventScript_83C9947::
	lock
	faceplayer
	@ Moemon Kai: Crystal's lines (audit case 59): POKe BALLS come in with the POKeDEX, as the clerk's list does
	goto_if_set FLAG_SYS_POKEDEX_GET, Johto_CherrygroveMart_EventScript_PokeBallsInStock
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
	.string "When I was walking in the grass, a\n"
	.string "bug MOéMON poisoned my MOéMON!\p"
	.string "I just kept going, but then my\n"
	.string "MOéMON fainted.\p"
	.string "You should keep an ANTIDOTE with\n"
	.string "you.$"

Johto_CherrygroveMart_Text_83C99F7::
	.string "They’re fresh out of POKé BALLS!\p"
	.string "When will they get more of them?$"

Johto_CherrygroveMart_EventScript_PokeBallsInStock::
	msgbox Johto_CherrygroveMart_Text_PokeBallsInStock, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_CherrygroveMart_Text_PokeBallsInStock::
	.string "POKé BALLS are in stock! Now I can\n"
	.string "catch MOéMON!$"
