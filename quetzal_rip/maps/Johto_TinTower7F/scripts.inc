Johto_TinTower7F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_TinTower7F_EventScript_83FB5C7
	.byte 0

@ Johto_TinTower7F object 1 (local id 1, gfx 0x3A9E) at 14,4
Johto_TinTower7F_EventScript_83FB57B::
	setorcopyvar VAR_0x8000, 0x22
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_TinTower7F_EventScript_83FB5C7::
	call Quetzal_EventScript_840EA62
	goto Quetzal_EventScript_83FAE20
