Kanto_SafariZone_West_MapScripts::
	.byte 0

@ Kanto_SafariZone_West sign / bg event at 22,19
Kanto_SafariZone_West_EventScript_841211D::
	msgbox Kanto_SafariZone_West_Text_842806F, MSGBOX_SIGN
	end

@ Kanto_SafariZone_West sign / bg event at 27,9
Kanto_SafariZone_West_EventScript_8412126::
	msgbox Kanto_SafariZone_West_Text_842807A, MSGBOX_SIGN
	end

@ Kanto_SafariZone_West sign / bg event at 36,10
Kanto_SafariZone_West_EventScript_841212F::
	msgbox Kanto_SafariZone_West_Text_84280F9, MSGBOX_SIGN
	end

@ Kanto_SafariZone_West sign / bg event at 35,26
Kanto_SafariZone_West_EventScript_8412138::
	msgbox Kanto_SafariZone_West_Text_8428142, MSGBOX_SIGN
	end

@ Kanto_SafariZone_West object 2 (local id 2, gfx 0x3F0E) at 17,13
Kanto_SafariZone_West_EventScript_846BCE8::
	setorcopyvar VAR_0x8000, ITEM_SHARP_BEAK
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SafariZone_West object 1 (local id 1, gfx 0x3F0E) at 28,14
Kanto_SafariZone_West_EventScript_846BCF5::
	checkitem ITEM_GOLD_TEETH, 1
	goto_if_eq VAR_RESULT, 1, Quetzal_EventScript_835FD2E
	setorcopyvar VAR_0x8000, ITEM_GOLD_TEETH
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SafariZone_West object 3 (local id 3, gfx 0x3F0E) at 16,27
Kanto_SafariZone_West_EventScript_846BD12::
	setorcopyvar VAR_0x8000, ITEM_MAX_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SafariZone_West object 4 (local id 4, gfx 0x3F0E) at 27,24
Kanto_SafariZone_West_EventScript_846BD1F::
	setorcopyvar VAR_0x8000, ITEM_MAX_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SafariZone_West_Text_842806F::
	.string "REST HOUSE$"

Kanto_SafariZone_West_Text_842807A::
	.string "REQUEST NOTICE\p"
	.string "Please find the SAFARI WARDEN’s\n"
	.string "lost GOLD TEETH.\l"
	.string "They’re around here somewhere.\p"
	.string "Reward offered!\n"
	.string "Contact: WARDEN$"

Kanto_SafariZone_West_Text_84280F9::
	.string "TRAINER TIPS\p"
	.string "Zone Exploration Campaign!\n"
	.string "The Search for the SECRET HOUSE!$"

Kanto_SafariZone_West_Text_8428142::
	.string "AREA 3\n"
	.string "EAST: CENTER AREA$"
