Johto_MountMortar1FOutside_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_MountMortar1FOutside_EventScript_83DFE7F
	.byte 0

Johto_MountMortar1FOutside_EventScript_83DFE7F::
	call Quetzal_EventScript_840EA62
	end

@ Johto_MountMortar1FOutside object 2 (local id 2, gfx 0x3A9E) at 24,33
Johto_MountMortar1FOutside_EventScript_83DFE85::
	setorcopyvar VAR_0x8000, ITEM_REVIVE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_MountMortar1FOutside object 1 (local id 1, gfx 0x3A9E) at 41,33
Johto_MountMortar1FOutside_EventScript_83DFE92::
	setorcopyvar VAR_0x8000, ITEM_ELIXIR
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
