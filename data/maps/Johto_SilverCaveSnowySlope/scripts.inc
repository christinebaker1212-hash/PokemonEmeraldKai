Johto_SilverCaveSnowySlope_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SilverCaveSnowySlope_EventScript_83E0587
	.byte 0

Johto_SilverCaveSnowySlope_EventScript_83E0587::
	call Quetzal_EventScript_840EA62
	end

@ Johto_SilverCaveSnowySlope object 1 (local id 1, gfx 0x3B) at 10,40
Johto_SilverCaveSnowySlope_EventScript_83E058D::
	setorcopyvar VAR_0x8000, ITEM_RARE_CANDY
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end

@ Johto_SilverCaveSnowySlope object 2 (local id 2, gfx 0x3B) at 31,41
Johto_SilverCaveSnowySlope_EventScript_83E059A::
	setorcopyvar VAR_0x8000, ITEM_NEVER_MELT_ICE
	setorcopyvar VAR_0x8001, 1
	callstd STD_FIND_ITEM
	end
