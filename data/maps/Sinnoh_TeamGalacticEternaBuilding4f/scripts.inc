Sinnoh_TeamGalacticEternaBuilding4f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_TeamGalacticEternaBuilding4f_OnTransition
	.byte 0

Sinnoh_TeamGalacticEternaBuilding4f_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_TeamGalacticEternaBuilding4f_EventScript_Bg1::
	msgbox Sinnoh_TeamGalacticEternaBuilding4f_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_TeamGalacticEternaBuilding4f_Text_Bg1:
	.string "A notice on the wall.$"

@ --- end of SINNOH words ---
