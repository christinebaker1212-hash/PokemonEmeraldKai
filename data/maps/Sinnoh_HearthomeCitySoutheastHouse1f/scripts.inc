Sinnoh_HearthomeCitySoutheastHouse1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_HearthomeCitySoutheastHouse1f_EventScript_Person1::
	goto SinnohP_hearthome_city_southeast_house_1f_1
	end

Sinnoh_HearthomeCitySoutheastHouse1f_EventScript_Person2::
	goto SinnohP_hearthome_city_southeast_house_1f_2
	end

Sinnoh_HearthomeCitySoutheastHouse1f_EventScript_Person3::
	msgbox Sinnoh_HearthomeCitySoutheastHouse1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_HearthomeCitySoutheastHouse1f_EventScript_Person4::
	goto SinnohP_hearthome_city_southeast_house_1f_3
	end

Sinnoh_HearthomeCitySoutheastHouse1f_Text_Person3:
	.string "SHROOMISH: Shroo...\p"
	.string "It's sleeping on the warm floor.$"

@ --- end of SINNOH words ---
