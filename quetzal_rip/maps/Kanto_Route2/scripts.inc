Kanto_Route2_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840FBB9
	.byte 0

@ Kanto_Route2 sign / bg event at 7,73
Kanto_Route2_EventScript_8417C12::
	msgbox Kanto_Route2_Text_8433725, MSGBOX_SIGN
	end

@ Kanto_Route2 sign / bg event at 14,12
Kanto_Route2_EventScript_8417C1B::
	msgbox Kanto_Route2_Text_8433749, MSGBOX_SIGN
	end

@ Kanto_Route2 object 5 (local id 5, gfx 0x3F0E) at 17,54
Kanto_Route2_EventScript_846BA44::
	setorcopyvar VAR_0x8000, 0x31
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_Route2 object 6 (local id 6, gfx 0x3F0E) at 17,64
Kanto_Route2_EventScript_846BA51::
	setorcopyvar VAR_0x8000, 0x2C
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_Route2_Text_8433725::
	.string "ROUTE 2\n"
	.string "VIRIDIAN CITY - PEWTER CITY$"

Kanto_Route2_Text_8433749::
	.string "DIGLETT’S CAVE$"
