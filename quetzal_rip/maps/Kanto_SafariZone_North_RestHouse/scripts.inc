Kanto_SafariZone_North_RestHouse_MapScripts::
	.byte 0

@ Kanto_SafariZone_North_RestHouse object 3 (local id 3, gfx 0x3F06) at 6,2
Kanto_SafariZone_North_RestHouse_EventScript_8412171::
	msgbox Kanto_SafariZone_North_RestHouse_Text_84282AC, MSGBOX_NPC
	end

@ Kanto_SafariZone_North_RestHouse object 2 (local id 2, gfx 0x3F43) at 7,5
Kanto_SafariZone_North_RestHouse_EventScript_841217A::
	msgbox Kanto_SafariZone_North_RestHouse_Text_8428239, MSGBOX_NPC
	end

@ Kanto_SafariZone_North_RestHouse object 1 (local id 1, gfx 0x3F09) at 3,4
Kanto_SafariZone_North_RestHouse_EventScript_8412183::
	msgbox Kanto_SafariZone_North_RestHouse_Text_84282ED, MSGBOX_NPC
	end

@ Kanto_SafariZone_North_RestHouse object 4 (local id 4, gfx 0x3EE6) at 11,2
Kanto_SafariZone_North_RestHouse_EventScript_845D2A4::
	lock
	faceplayer
	setvar VAR_0x8004, 6
	setvar VAR_0x8005, 5
	special QzQuestLogNop_737
	msgbox Kanto_SafariZone_North_RestHouse_Text_8428348, MSGBOX_DEFAULT
	release
	end

Kanto_SafariZone_North_RestHouse_Text_8428239::
	.string "You can keep any item you find in\n"
	.string "the SAFARI ZONE.\p"
	.string "There is no time limit, so explore\n"
	.string "every area at your own pace.$"

Kanto_SafariZone_North_RestHouse_Text_84282AC::
	.string "Go to the deepest part of the\n"
	.string "SAFARI ZONE. You will win a prize!$"

Kanto_SafariZone_North_RestHouse_Text_84282ED::
	.string "My EEVEE evolved into FLAREON.\p"
	.string "But, a friend’s EEVEE turned into a\n"
	.string "VAPOREON. I wonder why?$"

Kanto_SafariZone_North_RestHouse_Text_8428348::
	.string "The SAFARI ZONE’s huge, wouldn’t\n"
	.string "you say?\p"
	.string "FUCHSIA’s GYM LEADER, KOGA, \n"
	.string "patrols the grounds every so often.\p"
	.string "Thanks to him, we can play here\n"
	.string "knowing that we’re safe.$"
