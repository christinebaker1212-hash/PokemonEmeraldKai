Sinnoh_SurvivalAreaNorthHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SurvivalAreaNorthHouse_EventScript_Bg2::
	goto SinnohP_survival_area_north_house_3
	end

Sinnoh_SurvivalAreaNorthHouse_EventScript_Person1::
	msgbox Sinnoh_SurvivalAreaNorthHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SurvivalAreaNorthHouse_EventScript_Person2::
	goto SinnohP_survival_area_north_house_1
	end

Sinnoh_SurvivalAreaNorthHouse_Text_Person1:
	.string "I came here to climb STARK MOUNTAIN.\n"
	.string "The heat is no joke!$"

@ --- end of SINNOH words ---
