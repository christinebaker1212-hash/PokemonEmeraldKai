Johto_EcruteakItemfinderHouse_MapScripts::
	.byte 0

@ Johto_EcruteakItemfinderHouse object 1 (local id 1, gfx 0x3E84) at 5,4
Johto_EcruteakItemfinderHouse_EventScript_83CE937::
	lock
	faceplayer
	msgbox Johto_EcruteakItemfinderHouse_Text_83CE97B, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Johto_EcruteakItemfinderHouse sign / bg event at 6,4
@ Johto_EcruteakItemfinderHouse sign / bg event at 7,4
Johto_EcruteakItemfinderHouse_EventScript_83CE944::
	lock
	msgbox Johto_EcruteakItemfinderHouse_Text_83CE9D8, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_EcruteakItemfinderHouse_EventScript_83CE95B
	closemessage
	release
	end

Johto_EcruteakItemfinderHouse_EventScript_83CE95B::
	msgbox Johto_EcruteakItemfinderHouse_Text_83CE9FD, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 1, Johto_EcruteakItemfinderHouse_EventScript_83CE970
	release
	end

Johto_EcruteakItemfinderHouse_EventScript_83CE970::
	msgbox Johto_EcruteakItemfinderHouse_Text_83CEAB9, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakItemfinderHouse_Text_83CE97B::
	.string "ECRUTEAK is a very old city.\n"
	.string "Many generations passed down\l"
	.string "their history in books like these.$"

Johto_EcruteakItemfinderHouse_Text_83CE9D8::
	.string "HISTORY OF ECRUTEAK\p"
	.string "Want to read it?$"

Johto_EcruteakItemfinderHouse_Text_83CE9FD::
	.string "In ECRUTEAK, there were two towers.\p"
	.string "Each tower was the roost of\n"
	.string "powerful flying POKéMON.\p"
	.string "But one of the towers burned to\n"
	.string "the ground.\p"
	.string "The two POKéMON haven’t been seen\n"
	.string "since…\p"
	.string "Keep reading?$"

Johto_EcruteakItemfinderHouse_Text_83CEAB9::
	.string "ECRUTEAK was also home to three\n"
	.string "POKéMON that raced around the town.\p"
	.string "They were said to have been born\n"
	.string "of water, lightning and fire.\p"
	.string "But they could not contain their\n"
	.string "excessive power.\p"
	.string "So they say the three ran like the\n"
	.string "wind off into the grassland.$"
