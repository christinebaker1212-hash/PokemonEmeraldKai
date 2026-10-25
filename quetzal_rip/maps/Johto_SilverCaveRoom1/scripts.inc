Johto_SilverCaveRoom1_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SilverCaveRoom1_EventScript_83E02F7
	.byte 0

Johto_SilverCaveRoom1_EventScript_83E02F7::
	end

@ Johto_SilverCaveRoom1 object 2 (local id 2, gfx 0x3B) at 42,35
Johto_SilverCaveRoom1_EventScript_83E02F8::
	setorcopyvar VAR_0x8000, 0x1F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SilverCaveRoom1 object 3 (local id 3, gfx 0x3B) at 49,11
Johto_SilverCaveRoom1_EventScript_83E0305::
	setorcopyvar VAR_0x8000, 0x7F
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SilverCaveRoom1 object 1 (local id 1, gfx 0x3B) at 9,23
Johto_SilverCaveRoom1_EventScript_83E0312::
	setorcopyvar VAR_0x8000, 0x20
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SilverCaveRoom1 object 4 (local id 4, gfx 0x3B) at 9,35
Johto_SilverCaveRoom1_EventScript_83E031F::
	setorcopyvar VAR_0x8000, 3
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
