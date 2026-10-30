Johto_VictoryRoad1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_VictoryRoad1F object 4 (local id 4, gfx 0x3B) at 19,11
Johto_VictoryRoad1F_EventScript_83FE87C::
	setorcopyvar VAR_0x8000, ITEM_MAX_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_VictoryRoad1F object 5 (local id 5, gfx 0x3B) at 28,29
Johto_VictoryRoad1F_EventScript_83FE889::
	setorcopyvar VAR_0x8000, ITEM_FULL_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_VictoryRoad1F object 6 (local id 6, gfx 0x3B) at 22,24
Johto_VictoryRoad1F_EventScript_83FE896::
	setorcopyvar VAR_0x8000, ITEM_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
