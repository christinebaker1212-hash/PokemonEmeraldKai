Kanto_CeruleanCave_2F_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, Kanto_CeruleanCave_2F_OnLoad
	.byte 0

@ Moemon Kai: Quetzal's ladder to its RAYQUAZA room is gone (FireRed's 2F has none)
Kanto_CeruleanCave_2F_OnLoad:
	setmetatile 4, 2, 665, TRUE
	setmetatile 4, 3, 641, FALSE
	end

@ Kanto_CeruleanCave_2F object 3 (local id 3, gfx 0x3F0E) at 33,12
Kanto_CeruleanCave_2F_EventScript_846BF47::
	setorcopyvar VAR_0x8000, ITEM_FULL_RESTORE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_CeruleanCave_2F object 1 (local id 1, gfx 0x3F0E) at 9,18
Kanto_CeruleanCave_2F_EventScript_846BF54::
	setorcopyvar VAR_0x8000, ITEM_PP_UP
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_CeruleanCave_2F object 2 (local id 2, gfx 0x3F0E) at 29,16
Kanto_CeruleanCave_2F_EventScript_846BF61::
	setorcopyvar VAR_0x8000, ITEM_ULTRA_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
