Kanto_SixIsland_WaterPath_House1_MapScripts::
	.byte 0

@ Kanto_SixIsland_WaterPath_House1 object 1 (local id 1, gfx 0x3EE8) at 3,4
Kanto_SixIsland_WaterPath_House1_EventScript_8421DCF::
	lock
	faceplayer
	specialvar VAR_RESULT, QzReturnZero
	goto_if_eq VAR_RESULT, 2, Quetzal_EventScript_8457FA3
	goto_if_set FLAG_QZ_0x2158, Kanto_SixIsland_WaterPath_House1_EventScript_8421E26
	setvar VAR_0x8004, SPECIES_HERACROSS
	specialvar VAR_RESULT, QzPartyHasSpecies
	goto_if_eq VAR_RESULT, 0, Kanto_SixIsland_WaterPath_House1_EventScript_8421E30
	msgbox Kanto_SixIsland_WaterPath_House1_Text_8421F1E, MSGBOX_DEFAULT
	setorcopyvar VAR_0x8000, ITEM_NEST_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Kanto_SixIsland_WaterPath_House1_EventScript_8421E98
	setflag FLAG_QZ_0x2158
	goto Kanto_SixIsland_WaterPath_House1_EventScript_8421E26

Kanto_SixIsland_WaterPath_House1_EventScript_8421E26::
	msgbox Kanto_SixIsland_WaterPath_House1_Text_8421F68, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_8421E30::
	msgbox Kanto_SixIsland_WaterPath_House1_Text_8421EBA, MSGBOX_DEFAULT
	release
	end

Kanto_SixIsland_WaterPath_House1_EventScript_8421E98::
	msgbox Kanto_SixIsland_WaterPath_House1_Text_845549B, MSGBOX_DEFAULT
	release
	end

@ Kanto_SixIsland_WaterPath_House1 sign / bg event at 9,1
Kanto_SixIsland_WaterPath_House1_EventScript_8421EA2::
	msgbox Kanto_SixIsland_WaterPath_House1_Text_8421FAC, MSGBOX_SIGN
	end

Kanto_SixIsland_WaterPath_House1_Text_8421EBA::
	.string "HERACROSS is my favorite POKéMON!\p"
	.string "Bring one in your party, and\n"
	.string "I’ll give you a gift for showing me!$"

Kanto_SixIsland_WaterPath_House1_Text_8421F1E::
	.string "What a lovely HERACROSS!\n"
	.string "Thank you for bringing it.\p"
	.string "This gift is for you!$"

Kanto_SixIsland_WaterPath_House1_Text_8421F68::
	.string "What a horn, and what strength!\n"
	.string "HERACROSS never ceases to amaze me.$"

Kanto_SixIsland_WaterPath_House1_Text_8421FAC::
	.string "A drawing of HERACROSS.\n"
	.string "Its horn is carefully detailed!$"

Kanto_SixIsland_WaterPath_House1_Text_845549B::
	.string "Your BAG is full.\n"
	.string "My reward won’t fit.$"
