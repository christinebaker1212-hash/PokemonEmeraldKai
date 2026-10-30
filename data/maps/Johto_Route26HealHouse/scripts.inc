Johto_Route26HealHouse_MapScripts::
	.byte 0

@ Johto_Route26HealHouse object 1 (local id 1, gfx 0x14) at 4,4
Johto_Route26HealHouse_EventScript_83EC117::
	lock
	faceplayer
	msgbox Johto_Route26HealHouse_Text_83EC131, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_83602D9
	msgbox Johto_Route26HealHouse_Text_83EC171, MSGBOX_DEFAULT
	release
	end

Johto_Route26HealHouse_Text_83EC131::
	.string "Your POKéMON look a little tired.\p"
	.string "You should rest them a while.$"

Johto_Route26HealHouse_Text_83EC171::
	.string "There!\p"
	.string "Your POKéMON are looking good!\p"
	.string "Keep at it!$"
