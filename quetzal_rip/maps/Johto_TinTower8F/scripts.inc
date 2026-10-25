Johto_TinTower8F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_TinTower8F_EventScript_83FB5FB
	.byte 0

@ Johto_TinTower8F object 2 (local id 2, gfx 0x3A9E) at 11,10
Johto_TinTower8F_EventScript_83FB595::
	setorcopyvar VAR_0x8000, 0x34
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_TinTower8F object 3 (local id 3, gfx 0x3A9E) at 6,15
Johto_TinTower8F_EventScript_83FB5A2::
	setorcopyvar VAR_0x8000, 0x88
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

Johto_TinTower8F_EventScript_83FB5FB::
	call Quetzal_EventScript_840EA62
	goto Quetzal_EventScript_83FAE20
