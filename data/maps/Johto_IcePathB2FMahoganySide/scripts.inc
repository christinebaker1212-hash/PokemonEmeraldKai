Johto_IcePathB2FMahoganySide_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_IcePathB2FMahoganySide_EventScript_83DBA0E
	.byte 0

Johto_IcePathB2FMahoganySide_EventScript_83DBA0E::
	end

@ Johto_IcePathB2FMahoganySide object 2 (local id 2, gfx 0x3B) at 7,6
Johto_IcePathB2FMahoganySide_EventScript_83DBA0F::
	setorcopyvar VAR_0x8000, ITEM_FULL_HEAL
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IcePathB2FMahoganySide object 1 (local id 1, gfx 0x3B) at 16,14
Johto_IcePathB2FMahoganySide_EventScript_83DBA1C::
	setorcopyvar VAR_0x8000, ITEM_MAX_POTION
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_IcePathB2FMahoganySide object 3 (local id 3, gfx 0x3B) at 26,5
Johto_IcePathB2FMahoganySide_EventScript_83DBA29::
	setorcopyvar VAR_0x8000, ITEM_CARBOS
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
