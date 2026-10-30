Kanto_SafariZone_Center_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SafariZone_Center_EventScript_84120A9
	.byte 0

Kanto_SafariZone_Center_EventScript_84120A9::
	setflag FLAG_QZ_0x224F
	end

@ Kanto_SafariZone_Center sign / bg event at 30,26
Kanto_SafariZone_Center_EventScript_84120AD::
	msgbox Kanto_SafariZone_Center_Text_8427ED9, MSGBOX_SIGN
	end

@ Kanto_SafariZone_Center sign / bg event at 25,27
Kanto_SafariZone_Center_EventScript_84120B6::
	msgbox Kanto_SafariZone_Center_Text_8427EE4, MSGBOX_SIGN
	end

@ Kanto_SafariZone_Center sign / bg event at 33,20
Kanto_SafariZone_Center_EventScript_84120BF::
	msgbox Kanto_SafariZone_Center_Text_8427F24, MSGBOX_SIGN
	end

@ Kanto_SafariZone_Center object 1 (local id 1, gfx 0x3F0E) at 24,15
Kanto_SafariZone_Center_EventScript_846BC8D::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SafariZone_Center_Text_8427ED9::
	.string "REST HOUSE$"

Kanto_SafariZone_Center_Text_8427EE4::
	.string "TRAINER TIPS\p"
	.string "Wild POKéMON battles here follow\n"
	.string "the normal rules.$"

Kanto_SafariZone_Center_Text_8427F24::
	.string "CENTER AREA$"
