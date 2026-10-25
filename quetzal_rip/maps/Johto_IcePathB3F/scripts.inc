Johto_IcePathB3F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IcePathB3F_EventScript_83DBF87
	.byte 0

Johto_IcePathB3F_EventScript_83DBF87::
	call Quetzal_EventScript_840EA62
	end

@ Johto_IcePathB3F object 1 (local id 1, gfx 0x3B) at 11,20
Johto_IcePathB3F_EventScript_83DBF8D::
	setorcopyvar VAR_0x8000, 0x1AE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
