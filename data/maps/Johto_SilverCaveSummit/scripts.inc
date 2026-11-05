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
	setmaplayoutindex LAYOUT_JOHTO_SILVER_CAVE_SUMMIT_ALT   @ (Quetzal's layout 0x232)
	return

Johto_SilverCaveSummit_EventScript_83E05E4::
	setmaplayoutindex LAYOUT_JOHTO_SILVER_CAVE_SUMMIT   @ (Quetzal's layout 0x231)
	return

@ Moemon Kai: Crystal's RED (pokecrystal maps/SilverCaveRoom3.asm; audit case 106): Crystal's team (mp_region.c
@ sCrystalTeams), no prize, one battle, then he's gone and the credits roll (he's back on the next climb, as in Crystal)
Johto_SilverCaveSummit_EventScript_Red::
	lock
	faceplayer
	msgbox Johto_SilverCaveSummit_Text_RedSeenText, MSGBOX_DEFAULT
	closemessage
	trainerbattle TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT, TRAINER_QZ_4327, 0, Johto_SilverCaveSummit_Text_RedWinLossText
	setflag FLAG_QZ_0x11B0
	msgbox Johto_SilverCaveSummit_Text_RedLeavesText, MSGBOX_DEFAULT
	closemessage
	setrespawn 0x17
	fadescreenswapbuffers 1
	setflag FLAG_QZ_0x1063
	removeobject 1
	fadescreenswapbuffers 0
	delay 0x1E
	clearflag FLAG_QZ_0x1063
	special QzAfterCreditsReset
	waitstate
	releaseall
	end

Johto_SilverCaveSummit_Text_RedSeenText::
	.string "…… ……$"

Johto_SilverCaveSummit_Text_RedWinLossText::
	.string "…$"

Johto_SilverCaveSummit_Text_RedLeavesText::
	.string "…… ……$"
