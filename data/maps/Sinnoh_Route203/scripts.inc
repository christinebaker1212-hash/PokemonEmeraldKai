Sinnoh_Route203_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_Route203_OnTransition
	.byte 0

Sinnoh_Route203_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route203_EventScript_Person13::
	goto SinnohP_route_203_1
	end

Sinnoh_Route203_EventScript_Sign7::
	goto SinnohP_route_203_2
	end

Sinnoh_Route203_EventScript_Sign8::
	goto SinnohP_route_203_3
	end

Sinnoh_Route203_EventScript_Sign9::
	goto SinnohP_route_203_4
	end

Sinnoh_Route203_EventScript_Sign10::
	goto SinnohP_route_203_5
	end

@ --- end of SINNOH words ---
