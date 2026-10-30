Kanto_CeruleanCave_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_CeruleanCave_1F_EventScript_8412212
	.byte 0

Kanto_CeruleanCave_1F_EventScript_8412212::
	setflag FLAG_QZ_0x2235
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_CeruleanCave_1F object 3 (local id 3, gfx 0x3F0E) at 25,5
Kanto_CeruleanCave_1F_EventScript_846BF20::
	setorcopyvar VAR_0x8000, ITEM_MAX_ELIXIR
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_CeruleanCave_1F object 1 (local id 1, gfx 0x3F0E) at 7,3
Kanto_CeruleanCave_1F_EventScript_846BF2D::
	setorcopyvar VAR_0x8000, ITEM_NUGGET
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_CeruleanCave_1F object 2 (local id 2, gfx 0x3F0E) at 11,16
Kanto_CeruleanCave_1F_EventScript_846BF3A::
	setorcopyvar VAR_0x8000, ITEM_FULL_RESTORE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
