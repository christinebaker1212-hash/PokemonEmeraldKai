Sinnoh_Route208GateToHearthomeCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route208GateToHearthomeCity_EventScript_Person1::
	goto SinnohP_route_208_gate_to_hearthome_city_1
	end

Sinnoh_Route208GateToHearthomeCity_EventScript_Person2::
	goto SinnohP_route_208_gate_to_hearthome_city_2
	end

Sinnoh_Route208GateToHearthomeCity_EventScript_Person3::
	msgbox Sinnoh_Route208GateToHearthomeCity_Text_Person3, MSGBOX_NPC
	end

Sinnoh_Route208GateToHearthomeCity_Text_Person3:
	.string "CLEFAIRY: Pi!\p"
	.string "It's bouncing up and down.$"

@ --- end of SINNOH words ---
