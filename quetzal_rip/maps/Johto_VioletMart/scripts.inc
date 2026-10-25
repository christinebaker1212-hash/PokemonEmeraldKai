Johto_VioletMart_MapScripts::
	.byte 0

@ Johto_VioletMart object 3 (local id 3, gfx 0x3E8E) at 2,3
Johto_VioletMart_EventScript_83FFBFA::
	lock
	faceplayer
	message Quetzal_Text_8360D02
	waitmessage
	pokemart 0x0
	msgbox Quetzal_Text_8360D20, MSGBOX_DEFAULT
	release
	end

@ Johto_VioletMart object 1 (local id 1, gfx 0x1E) at 8,5
Johto_VioletMart_EventScript_83FFC11::
	lock
	faceplayer
	msgbox Johto_VioletMart_Text_83FFC2B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_VioletMart object 2 (local id 2, gfx 0x3EAE) at 6,2
Johto_VioletMart_EventScript_83FFC1E::
	lock
	faceplayer
	msgbox Johto_VioletMart_Text_83FFCB0, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_VioletMart_Text_83FFC2B::
	.string "When you first catch a POKéMON,\n"
	.string "it may be weak.\p"
	.string "But it will eventually grow to\n"
	.string "be strong.\p"
	.string "It’s important to treat POKéMON\n"
	.string "with love.$"

Johto_VioletMart_Text_83FFCB0::
	.string "POKéMON can hold items like\n"
	.string "POTION and ANTIDOTE.\p"
	.string "But they don’t appear to know\n"
	.string "how to use man-made items.$"
