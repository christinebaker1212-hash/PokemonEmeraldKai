Sinnoh_PastoriaCityObservatoryGate1f_MapScripts::
	.byte 0

@ The GREAT MARSH's SAFARI GAME: Platinum's own gate (its map scripts run from sinnoh/platinum_scripts.inc;
@ StartSafariGame on the game's own SAFARI GAME: safari_zone.c, data/scripts/safari_zone.inc)
Sinnoh_PastoriaCityObservatoryGate1f_EventScript_Trigger1::
	goto SinnohP_pastoria_city_observatory_gate_1f_2

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_Person1::
	goto SinnohP_pastoria_city_observatory_gate_1f_5

Sinnoh_PastoriaCityObservatoryGate1f_EventScript_Person2::
	goto SinnohP_pastoria_city_observatory_gate_1f_1

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityObservatoryGate1f_EventScript_Person3::
	goto SinnohP_pastoria_city_observatory_gate_1f_6
	end

@ --- end of SINNOH words ---
