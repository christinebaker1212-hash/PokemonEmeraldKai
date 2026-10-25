Johto_SilverCaveSummit_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_SilverCaveSummit_EventScript_83E05C5
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Johto_SilverCaveSummit_OnFrame_83E05B2
	.byte 0

Johto_SilverCaveSummit_OnFrame_83E05B2:
	map_script_2 VAR_TEMP_0, 0, Johto_SilverCaveSummit_EventScript_83E05BC
	.2byte 0

Johto_SilverCaveSummit_EventScript_83E05BC::
	playse SE_M_BLIZZARD
	setvar VAR_TEMP_0, 1
	end

Johto_SilverCaveSummit_EventScript_83E05C5::
	setescapewarp MAP_JOHTO_SILVER_CAVE_OUTSIDE, 255, 17, 8
	call_if_set FLAG_QZ_0x1016, Johto_SilverCaveSummit_EventScript_83E05E0
	call_if_unset FLAG_QZ_0x1016, Johto_SilverCaveSummit_EventScript_83E05E4
	end

Johto_SilverCaveSummit_EventScript_83E05E0::
	setmaplayoutindex 0x232
	return

Johto_SilverCaveSummit_EventScript_83E05E4::
	setmaplayoutindex 0x231
	return
