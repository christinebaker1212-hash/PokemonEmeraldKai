Sinnoh_CelesticTownCave_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_CelesticTownCave_OnTransition
	.byte 0

Sinnoh_CelesticTownCave_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CelesticTownCave_EventScript_Bg1::
	msgbox Sinnoh_CelesticTownCave_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_CelesticTownCave_EventScript_Bg2::
	msgbox Sinnoh_CelesticTownCave_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_CelesticTownCave_Text_Bg1:
	.string "A painting of a MOéMON in a field of\n"
	.string "flowers.$"

Sinnoh_CelesticTownCave_Text_Bg2:
	.string "A painting of a MOéMON in a field of\n"
	.string "flowers.$"

@ --- end of SINNOH words ---
