Kanto_SafariZone_North_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_SafariZone_North sign / bg event at 44,9
Kanto_SafariZone_North_EventScript_84120EF::
	msgbox Kanto_SafariZone_North_Text_8427F9C, MSGBOX_SIGN
	end

@ Kanto_SafariZone_North sign / bg event at 13,27
Kanto_SafariZone_North_EventScript_84120F8::
	msgbox Kanto_SafariZone_North_Text_8427FA7, MSGBOX_SIGN
	end

@ Kanto_SafariZone_North sign / bg event at 23,31
Kanto_SafariZone_North_EventScript_8412101::
	msgbox Kanto_SafariZone_North_Text_8427FD5, MSGBOX_SIGN
	end

@ Kanto_SafariZone_North sign / bg event at 29,32
Kanto_SafariZone_North_EventScript_841210A::
	msgbox Kanto_SafariZone_North_Text_8427FDC, MSGBOX_SIGN
	end

@ Kanto_SafariZone_North sign / bg event at 34,28
Kanto_SafariZone_North_EventScript_8412113::
	msgbox Kanto_SafariZone_North_Text_8428034, MSGBOX_SIGN
	end

@ Kanto_SafariZone_North object 1 (local id 1, gfx 0x3F0E) at 33,5
Kanto_SafariZone_North_EventScript_846BCCE::
	setorcopyvar VAR_0x8000, ITEM_PROTEIN
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SafariZone_North object 2 (local id 2, gfx 0x3F0E) at 28,9
Kanto_SafariZone_North_EventScript_846BCDB::
	setorcopyvar VAR_0x8000, ITEM_METAL_COAT
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SafariZone_North object 3 (local id 3, gfx 0x3F0E) at 29,17
Kanto_SafariZone_North_EventScript_846C0C0::
	setorcopyvar VAR_0x8000, ITEM_QUICK_CLAW
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SafariZone_North_Text_8427F9C::
	.string "REST HOUSE$"

Kanto_SafariZone_North_Text_8427FA7::
	.string "TRAINER TIPS\p"
	.string "The SECRET HOUSE is still ahead.$"

Kanto_SafariZone_North_Text_8427FD5::
	.string "AREA 2$"

Kanto_SafariZone_North_Text_8427FDC::
	.string "TRAINER TIPS\p"
	.string "POKéMON hide in tall grass.\p"
	.string "Zigzag through grassy areas to\n"
	.string "flush them out.$"

Kanto_SafariZone_North_Text_8428034::
	.string "TRAINER TIPS\p"
	.string "Find the SECRET HOUSE to win a\n"
	.string "special prize.$"
