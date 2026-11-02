Sinnoh_FullmoonIslandForest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_FullmoonIslandForest_OnTransition
	.byte 0

Sinnoh_FullmoonIslandForest_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FullmoonIslandForest_EventScript_Person1::
	goto SinnohP_fullmoon_island_forest_2
	end

@ --- end of SINNOH words ---
