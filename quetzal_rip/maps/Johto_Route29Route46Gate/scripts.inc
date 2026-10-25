Johto_Route29Route46Gate_MapScripts::
	.byte 0

@ Johto_Route29Route46Gate object 1 (local id 1, gfx 0x3EA9) at 3,5
Johto_Route29Route46Gate_EventScript_83D1DB2::
	lock
	faceplayer
	msgbox Johto_Route29Route46Gate_Text_83D1DCC, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_Route29Route46Gate object 2 (local id 2, gfx 0x23) at 9,4
Johto_Route29Route46Gate_EventScript_83D1DBF::
	lock
	faceplayer
	msgbox Johto_Route29Route46Gate_Text_83D1E18, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_Route29Route46Gate_Text_83D1DCC::
	.string "You can’t climb ledges.\p"
	.string "But you can jump down from\n"
	.string "them to take a shortcut.$"

Johto_Route29Route46Gate_Text_83D1E18::
	.string "Different kinds of POKéMON\n"
	.string "appear past here.\p"
	.string "If you want to catch them all,\n"
	.string "you have to look everywhere.$"
