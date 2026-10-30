Kanto_SeafoamIslands_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Kanto_SeafoamIslands_1F_EventScript_8412C06
	.byte 0

Kanto_SeafoamIslands_1F_EventScript_8412C06::
	setflag FLAG_QZ_0x2251
	call Quetzal_EventScript_840FBB5
	end

@ Kanto_SeafoamIslands_1F object 3 (local id 3, gfx 0x3F0E) at 11,8
Kanto_SeafoamIslands_1F_EventScript_846C0CD::
	setorcopyvar VAR_0x8000, ITEM_ICE_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
