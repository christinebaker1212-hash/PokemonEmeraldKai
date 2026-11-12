Sinnoh_SurvivalArea_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SurvivalArea_OnTransition
	.byte 0

Sinnoh_SurvivalArea_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SurvivalArea_EventScript_Bg2::
	goto SinnohP_survival_area_7
	end

Sinnoh_SurvivalArea_EventScript_Person2::
	goto SinnohP_common_28
	end

Sinnoh_SurvivalArea_EventScript_Person3::
	goto SinnohP_common_28
	end

Sinnoh_SurvivalArea_EventScript_Person4::
	goto SinnohP_survival_area_3
	end

Sinnoh_SurvivalArea_EventScript_Person5::
	goto SinnohP_survival_area_2
	end

Sinnoh_SurvivalArea_EventScript_Person6::
	goto SinnohP_survival_area_1
	end

Sinnoh_SurvivalArea_EventScript_Person8::
	goto SinnohP_survival_area_4
	end

Sinnoh_SurvivalArea_EventScript_Person9::
	goto SinnohP_survival_area_6
	end

Sinnoh_SurvivalArea_EventScript_Sign1::
	goto SinnohP_survival_area_5
	end

@ --- end of SINNOH words ---
