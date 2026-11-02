Sinnoh_CanalaveCitySailorEldritchHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCitySailorEldritchHouse_EventScript_Person1::
	goto SinnohP_canalave_city_sailor_eldritch_house_2
	end

Sinnoh_CanalaveCitySailorEldritchHouse_EventScript_Person2::
	goto SinnohP_canalave_city_sailor_eldritch_house_3
	end

Sinnoh_CanalaveCitySailorEldritchHouse_EventScript_Person3::
	msgbox Sinnoh_CanalaveCitySailorEldritchHouse_Text_Person3, MSGBOX_NPC
	end

Sinnoh_CanalaveCitySailorEldritchHouse_Text_Person3:
	.string "My son has nightmares every night.\n"
	.string "They say there's an island where\l"
	.string "nightmares come from...\p"
	.string "A crescent of light might chase them\n"
	.string "away.$"

@ --- end of SINNOH words ---
