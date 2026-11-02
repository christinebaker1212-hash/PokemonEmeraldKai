Sinnoh_OreburghCityWestHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityWestHouse_EventScript_Person1::
	goto SinnohP_oreburgh_city_west_house_1
	end

Sinnoh_OreburghCityWestHouse_EventScript_Person2::
	goto SinnohP_oreburgh_city_west_house_2
	end

Sinnoh_OreburghCityWestHouse_EventScript_Person3::
	msgbox Sinnoh_OreburghCityWestHouse_Text_Person3, MSGBOX_NPC
	end

Sinnoh_OreburghCityWestHouse_Text_Person3:
	.string "PIKACHU: Pika pi!\p"
	.string "It seems very determined.$"

@ --- end of SINNOH words ---
