Sinnoh_VeilstoneCityNortheastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCityNortheastHouse_EventScript_Person1::
	msgbox Sinnoh_VeilstoneCityNortheastHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_VeilstoneCityNortheastHouse_EventScript_Person2::
	goto SinnohP_veilstone_city_northeast_house_2
	end

Sinnoh_VeilstoneCityNortheastHouse_Text_Person1:
	.string "The rocks in the city fell from the sky\n"
	.string "long ago. I wonder what's inside them.$"

@ --- end of SINNOH words ---
