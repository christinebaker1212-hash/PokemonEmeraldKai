Sinnoh_VeilstoneCitySouthwestHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCitySouthwestHouse_EventScript_Person1::
	msgbox Sinnoh_VeilstoneCitySouthwestHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_VeilstoneCitySouthwestHouse_EventScript_Person2::
	goto SinnohP_veilstone_city_southwest_house_2
	end

Sinnoh_VeilstoneCitySouthwestHouse_EventScript_Person3::
	goto SinnohP_veilstone_city_southwest_house_3
	end

Sinnoh_VeilstoneCitySouthwestHouse_Text_Person1:
	.string "I love living by the cliffs. The wind\n"
	.string "sounds like music at night.$"

@ --- end of SINNOH words ---
