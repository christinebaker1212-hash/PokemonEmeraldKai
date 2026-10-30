Kanto_FourIsland_IcefallCave_1F_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Kanto_FourIsland_IcefallCave_1F_EventScript_8413AF3
	map_script MAP_SCRIPT_ON_LOAD, Kanto_FourIsland_IcefallCave_1F_EventScript_8413AF6
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kanto_FourIsland_IcefallCave_1F_OnFrame_8413AFA
	.byte 0

Kanto_FourIsland_IcefallCave_1F_OnFrame_8413AFA:
	map_script_2 VAR_TEMP_1, 1, Kanto_FourIsland_IcefallCave_1F_EventScript_8413B04
	.2byte 0

Kanto_FourIsland_IcefallCave_1F_EventScript_8413AF3::
	setstepcallback 8
	end

Kanto_FourIsland_IcefallCave_1F_EventScript_8413AF6::
	special QzSetIcefallCaveCrackedIceMetatiles
	end

Kanto_FourIsland_IcefallCave_1F_EventScript_8413B04::
	lockall
	setvar VAR_TEMP_1, 0
	delay 0x14
	applymovement LOCALID_PLAYER, Kanto_FourIsland_IcefallCave_1F_Movement_8413B23
	waitmovement 0
	playse SE_FALL
	delay 0x3C
	warphole MAP_KANTO_FOUR_ISLAND_ICEFALL_CAVE_B1F
	waitstate
	releaseall
	end

@ Kanto_FourIsland_IcefallCave_1F object 1 (local id 1, gfx 0x3F0E) at 11,7
Kanto_FourIsland_IcefallCave_1F_EventScript_846C239::
	setorcopyvar VAR_0x8000, ITEM_ULTRA_BALL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Kanto_FourIsland_IcefallCave_1F object 2 (local id 2, gfx 0x3F0E) at 12,16
Kanto_FourIsland_IcefallCave_1F_EventScript_846C246::
	setorcopyvar VAR_0x8000, ITEM_DRAGON_SCALE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Kanto_FourIsland_IcefallCave_1F_Movement_8413B23:
	set_invisible
	step_end
