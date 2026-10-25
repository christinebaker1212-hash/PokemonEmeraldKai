Johto_GoldenrodNameRater_MapScripts::
	.byte 0

@ Johto_GoldenrodNameRater sign / bg event at 0,1
@ Johto_GoldenrodNameRater sign / bg event at 1,1
Johto_GoldenrodNameRater_EventScript_83D616F::
	lock
	msgbox Johto_GoldenrodNameRater_Text_83D617A, MSGBOX_DEFAULT
	release
	end

@ Johto_GoldenrodNameRater sign / bg event at 2,1
Johto_GoldenrodNameRater_EventScript_83D62A0::
	msgbox Johto_GoldenrodNameRater_Text_83D62A9, MSGBOX_DEFAULT
	end

@ Johto_GoldenrodNameRater object 1 (local id 1, gfx 0x3EA6) at 4,4
Johto_GoldenrodNameRater_EventScript_83D62CD::
	lock
	faceplayer
	msgbox Johto_GoldenrodNameRater_Text_83D62D9, MSGBOX_DEFAULT
	release
	end

Johto_GoldenrodNameRater_Text_83D617A::
	.string "Have you heard of RADIANT\n"
	.string "POKéMON?\p"
	.string "Their colors can be altered by\n"
	.string "many different palette filters.\p"
	.string "These filters can affect both\n"
	.string "normal and Shiny color palettes.$"

Johto_GoldenrodNameRater_Text_83D62A9::
	.string "The redheaded boy’s name\n"
	.string "is SILVER.$"

Johto_GoldenrodNameRater_Text_83D62D9::
	.string "You can change any nickname\n"
	.string "from the party menu.$"
