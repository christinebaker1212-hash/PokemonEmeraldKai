Sinnoh_SnowpointCityEastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SnowpointCityEastHouse_EventScript_Bg1::
	goto SinnohP_common_54
	end

Sinnoh_SnowpointCityEastHouse_EventScript_Person1::
	msgbox Sinnoh_SnowpointCityEastHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SnowpointCityEastHouse_EventScript_Person2::
	goto SinnohP_snowpoint_city_east_house_2
	end

Sinnoh_SnowpointCityEastHouse_Text_Person1:
	.string "The temple to the north is very old.\n"
	.string "They say a great MOéMON sleeps deep\l"
	.string "inside.$"

@ --- end of SINNOH words ---
