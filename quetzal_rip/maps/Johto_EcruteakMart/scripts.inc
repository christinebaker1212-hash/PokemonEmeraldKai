Johto_EcruteakMart_MapScripts::
	.byte 0

@ Johto_EcruteakMart object 2 (local id 2, gfx 0x23) at 5,2
Johto_EcruteakMart_EventScript_83CEBAF::
	lock
	faceplayer
	msgbox Johto_EcruteakMart_Text_83CEBC9, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakMart object 1 (local id 1, gfx 0x1E) at 7,5
Johto_EcruteakMart_EventScript_83CEBBC::
	lock
	faceplayer
	msgbox Johto_EcruteakMart_Text_83CEC56, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakMart_Text_83CEBC9::
	.string "My EEVEE evolved into an ESPEON.\p"
	.string "But my friend’s EEVEE turned into\n"
	.string "an UMBREON.\p"
	.string "I wonder why? We both were raising\n"
	.string "our EEVEE in the same way…$"

Johto_EcruteakMart_Text_83CEC56::
	.string "If you use REVIVE, a POKéMON that’s\n"
	.string "fainted will wake right up.$"
