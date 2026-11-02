Sinnoh_VerityCavern_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_VerityCavern_OnTransition
	.byte 0

Sinnoh_VerityCavern_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VerityCavern_EventScript_Person1::
	goto SinnohP_verity_cavern_2
	end

@ --- end of SINNOH words ---
