Sinnoh_Route209GateToHearthomeCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_Route209GateToHearthomeCity_OnTransition
	.byte 0

Sinnoh_Route209GateToHearthomeCity_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route209GateToHearthomeCity_EventScript_Person1::
	goto SinnohP_route_209_gate_to_hearthome_city_1
	end

@ --- end of SINNOH words ---
