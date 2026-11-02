Sinnoh_VeilstoneCityNorthwestHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCityNorthwestHouse_EventScript_Person1::
	goto SinnohP_veilstone_city_northwest_house_1
	end

Sinnoh_VeilstoneCityNorthwestHouse_EventScript_Person2::
	goto SinnohP_veilstone_city_northwest_house_2
	end

Sinnoh_VeilstoneCityNorthwestHouse_EventScript_Person3::
	msgbox Sinnoh_VeilstoneCityNorthwestHouse_Text_Person3, MSGBOX_NPC
	end

Sinnoh_VeilstoneCityNorthwestHouse_Text_Person3:
	.string "BUNEARY: Bun!\p"
	.string "It's hopping in place.$"

@ --- end of SINNOH words ---
