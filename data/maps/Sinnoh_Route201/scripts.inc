Sinnoh_Route201_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_Route201_OnTransition
	.byte 0

Sinnoh_Route201_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route201_EventScript_Person1::
	goto SinnohP_route_201_9
	end

Sinnoh_Route201_EventScript_Person2::
	goto SinnohP_route_201_10
	end

Sinnoh_Route201_EventScript_Person10::
	goto SinnohP_route_201_11
	end

Sinnoh_Route201_EventScript_Person12::
	goto SinnohP_route_201_12
	end

Sinnoh_Route201_EventScript_Person13::
	goto Sinnoh_EventScript_Briefcase
	end

Sinnoh_Route201_EventScript_Sign4::
	goto SinnohP_route_201_6
	end

Sinnoh_Route201_EventScript_Sign5::
	goto SinnohP_route_201_7
	end

Sinnoh_Route201_EventScript_Sign11::
	goto SinnohP_route_201_8
	end

@ --- end of SINNOH words ---
