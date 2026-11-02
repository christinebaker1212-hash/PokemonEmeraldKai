Sinnoh_Route206CyclingRoadSouthGate_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route206CyclingRoadSouthGate_EventScript_Person1::
	goto SinnohP_route_206_cycling_road_south_gate_2
	end

Sinnoh_Route206CyclingRoadSouthGate_EventScript_Person2::
	goto SinnohP_route_206_cycling_road_south_gate_2
	end

Sinnoh_Route206CyclingRoadSouthGate_EventScript_Person3::
	msgbox Sinnoh_Route206CyclingRoadSouthGate_Text_Person3, MSGBOX_NPC
	end

Sinnoh_Route206CyclingRoadSouthGate_Text_Person3:
	.string "The cycling road is downhill all the\n"
	.string "way south! Wheee!$"

@ --- end of SINNOH words ---
