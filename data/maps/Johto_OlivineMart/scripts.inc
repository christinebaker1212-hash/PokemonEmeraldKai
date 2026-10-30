Johto_OlivineMart_MapScripts::
	.byte 0

@ Johto_OlivineMart object 1 (local id 1, gfx 0x3E8F) at 1,6
Johto_OlivineMart_EventScript_83E6709::
	lock
	faceplayer
	msgbox Johto_OlivineMart_Text_83E6723, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_OlivineMart object 2 (local id 2, gfx 0x18) at 6,2
Johto_OlivineMart_EventScript_83E6716::
	lock
	faceplayer
	msgbox Johto_OlivineMart_Text_83E676B, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_OlivineMart_Text_83E6723::
	.string "Do your POKéMON already know\n"
	.string "the move for carrying people\l"
	.string "across water?$"

Johto_OlivineMart_Text_83E676B::
	.string "My BUTTERFREE came from my\n"
	.string "boyfriend overseas.\p"
	.string "It carried some MAIL from him.\p"
	.string "Want to know what it says?\p"
	.string "Let’s see... Nope! It’s a\n"
	.string "secret!$"
