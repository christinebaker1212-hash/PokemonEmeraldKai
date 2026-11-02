Sinnoh_PastoriaCityNorthHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityNorthHouse_EventScript_Person1::
	goto SinnohP_pastoria_city_north_house_1
	end

Sinnoh_PastoriaCityNorthHouse_EventScript_Person2::
	msgbox Sinnoh_PastoriaCityNorthHouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_PastoriaCityNorthHouse_EventScript_Person3::
	goto SinnohP_pastoria_city_north_house_3
	end

Sinnoh_PastoriaCityNorthHouse_Text_Person2:
	.string "PSYDUCK: Psy psy...\p"
	.string "It's sitting in a puddle, perfectly\n"
	.string "content.$"

@ --- end of SINNOH words ---
