Kanto_SafariZone_East_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_SafariZone_East sign / bg event at 43,15
Kanto_SafariZone_East_EventScript_84120CE::
	msgbox Kanto_SafariZone_East_Text_8427F30, MSGBOX_SIGN
	end

@ Kanto_SafariZone_East sign / bg event at 17,8
Kanto_SafariZone_East_EventScript_84120D7::
	msgbox Kanto_SafariZone_East_Text_8427F3B, MSGBOX_SIGN
	end

@ Kanto_SafariZone_East sign / bg event at 18,27
Kanto_SafariZone_East_EventScript_84120E0::
	msgbox Kanto_SafariZone_East_Text_8427F83, MSGBOX_SIGN
	end

@ Kanto_SafariZone_East object 1 (local id 1, gfx 0x3F0E) at 13,12
Kanto_SafariZone_East_EventScript_846BC9A::
	setorcopyvar VAR_0x8000, ITEM_MAX_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SafariZone_East object 2 (local id 2, gfx 0x3F0E) at 37,15
Kanto_SafariZone_East_EventScript_846BCA7::
	setorcopyvar VAR_0x8000, ITEM_FULL_RESTORE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SafariZone_East object 3 (local id 3, gfx 0x3F0E) at 31,18
Kanto_SafariZone_East_EventScript_846BCB4::
	setorcopyvar VAR_0x8000, ITEM_SUN_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_SafariZone_East object 4 (local id 4, gfx 0x3F0E) at 36,17
Kanto_SafariZone_East_EventScript_846BCC1::
	setorcopyvar VAR_0x8000, ITEM_LEAF_STONE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_SafariZone_East_Text_8427F30::
	.string "REST HOUSE$"

Kanto_SafariZone_East_Text_8427F3B::
	.string "TRAINER TIPS\p"
	.string "Bring your own POKé BALLS. There\n"
	.string "is no time or step limit.$"

Kanto_SafariZone_East_Text_8427F83::
	.string "AREA 1\n"
	.string "WEST: CENTER AREA$"
