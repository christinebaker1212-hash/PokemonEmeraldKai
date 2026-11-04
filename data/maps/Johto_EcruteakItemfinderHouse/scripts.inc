Johto_EcruteakItemfinderHouse_MapScripts::
	.byte 0

@ Johto_EcruteakItemfinderHouse object 1 (local id 1, gfx 0x3E84) at 5,4
Johto_EcruteakItemfinderHouse_EventScript_83CE937::
	@ Moemon Kai: Crystal's ITEMFINDER gift (docs/REGIONS_AUDIT.md 140)
	lock
	faceplayer
	goto_if_set FLAG_JOHTO_GOT_ITEMFINDER, Johto_EcruteakItemfinderHouse_EventScript_Explain
	msgbox Johto_EcruteakItemfinderHouse_Text_Adventure, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, Johto_EcruteakItemfinderHouse_EventScript_No
	msgbox Johto_EcruteakItemfinderHouse_Text_TrueSpirit, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_ITEMFINDER
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA
	setflag FLAG_JOHTO_GOT_ITEMFINDER
Johto_EcruteakItemfinderHouse_EventScript_Explain::
	msgbox Johto_EcruteakItemfinderHouse_Text_Explain, MSGBOX_DEFAULT
	closemessage
	release
	end

Johto_EcruteakItemfinderHouse_EventScript_No::
	msgbox Johto_EcruteakItemfinderHouse_Text_EachHisOwn, MSGBOX_DEFAULT
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

Johto_EcruteakItemfinderHouse_Text_83CE9D8::
	.string "HISTORY OF ECRUTEAK\p"
	.string "Want to read it?$"

Johto_EcruteakItemfinderHouse_Text_83CE9FD::
	.string "In ECRUTEAK, there were two towers.\p"
	.string "Each tower was the roost of\n"
	.string "powerful flying MOéMON.\p"
	.string "But one of the towers burned to\n"
	.string "the ground.\p"
	.string "The two MOéMON haven’t been seen\n"
	.string "since…\p"
	.string "Keep reading?$"

Johto_EcruteakItemfinderHouse_Text_83CEAB9::
	.string "ECRUTEAK was also home to three\n"
	.string "MOéMON that raced around the town.\p"
	.string "They were said to have been born\n"
	.string "of water, lightning and fire.\p"
	.string "But they could not contain their\n"
	.string "excessive power.\p"
	.string "So they say the three ran like the\n"
	.string "wind off into the grassland.$"

Johto_EcruteakItemfinderHouse_Text_Adventure::
	.string "Ah. You’re on an adventure with\n"
	.string "your MOéMON?\p"
	.string "Well, what’s an adventure without\n"
	.string "treasure hunting?\p"
	.string "Am I right, or am I right?$"

Johto_EcruteakItemfinderHouse_Text_TrueSpirit::
	.string "Good! You understand the true\n"
	.string "spirit of adventure.\p"
	.string "I like that! Take this with you.$"

Johto_EcruteakItemfinderHouse_Text_Explain::
	.string "There are many items lying about\n"
	.string "that aren’t obvious.\p"
	.string "Use ITEMFINDER to check if there\n"
	.string "is an item on the ground near you.\p"
	.string "It doesn’t show the exact spot, so\n"
	.string "you’ll have to look yourself.\p"
	.string "Oh yeah--I heard there are items\n"
	.string "in ECRUTEAK’s BURNED TOWER.$"

Johto_EcruteakItemfinderHouse_Text_EachHisOwn::
	.string "Oh… To each his own, I suppose…$"
