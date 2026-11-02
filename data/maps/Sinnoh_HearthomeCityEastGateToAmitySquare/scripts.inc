Sinnoh_HearthomeCityEastGateToAmitySquare_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_HearthomeCityEastGateToAmitySquare_EventScript_Person1::
	goto SinnohP_hearthome_city_east_gate_to_amity_square_1
	end

Sinnoh_HearthomeCityEastGateToAmitySquare_EventScript_Person2::
	goto SinnohP_hearthome_city_east_gate_to_amity_square_2
	end

Sinnoh_HearthomeCityEastGateToAmitySquare_EventScript_Person3::
	msgbox Sinnoh_HearthomeCityEastGateToAmitySquare_Text_Person3, MSGBOX_NPC
	end

Sinnoh_HearthomeCityEastGateToAmitySquare_Text_Person3:
	.string "Have you been to AMITY SQUARE yet?\n"
	.string "My camera's full of happy POKéMON.$"

@ --- end of SINNOH words ---
