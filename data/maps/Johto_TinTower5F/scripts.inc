Johto_TinTower5F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_83FAE20
	.byte 0

@ Johto_TinTower5F object 1 (local id 1, gfx 0x3A9E) at 9,11
Johto_TinTower5F_EventScript_83FB554::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TinTower5F object 3 (local id 3, gfx 0x3A9E) at 16,2
Johto_TinTower5F_EventScript_83FB56E::
	setorcopyvar VAR_0x8000, ITEM_CARBOS
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
