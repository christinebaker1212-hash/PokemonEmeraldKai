Sinnoh_Route214GateToVeilstoneCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route214GateToVeilstoneCity_EventScript_Person1::
	goto SinnohP_route_214_gate_to_veilstone_city_1
	end

Sinnoh_Route214GateToVeilstoneCity_EventScript_Person2::
	msgbox Sinnoh_Route214GateToVeilstoneCity_Text_Person2, MSGBOX_NPC
	end

Sinnoh_Route214GateToVeilstoneCity_Text_Person2:
	.string "I'm writing about VEILSTONE's huge\n"
	.string "store. It sells everything except\l"
	.string "quiet.$"

@ --- end of SINNOH words ---
