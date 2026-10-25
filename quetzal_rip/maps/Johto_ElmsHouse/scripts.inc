Johto_ElmsHouse_MapScripts::
	.byte 0

@ Johto_ElmsHouse object 2 (local id 2, gfx 0x18) at 2,5
Johto_ElmsHouse_EventScript_83E1ADA::
	lock
	faceplayer
	msgbox Johto_ElmsHouse_Text_83E1AF4, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_ElmsHouse object 1 (local id 1, gfx 0xB) at 7,4
Johto_ElmsHouse_EventScript_83E1AE7::
	lock
	faceplayer
	msgbox Johto_ElmsHouse_Text_83E1B6E, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_ElmsHouse_Text_83E1AF4::
	.string "Hi, {PLAYER}! My husband’s always\n"
	.string "so busy--I hope he’s OK.\p"
	.string "When he’s caught up in his\n"
	.string "POKéMON research, he even\l"
	.string "forgets to eat.$"

Johto_ElmsHouse_Text_83E1B6E::
	.string "When I grow up, I’m going to\n"
	.string "help my dad!\p"
	.string "I’m going to be a great\n"
	.string "POKéMON professor!$"
