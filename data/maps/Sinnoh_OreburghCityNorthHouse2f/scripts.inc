Sinnoh_OreburghCityNorthHouse2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityNorthHouse2f_EventScript_Person1::
	goto SinnohP_oreburgh_city_north_house_2f_1
	end

Sinnoh_OreburghCityNorthHouse2f_EventScript_Person2::
	goto SinnohP_oreburgh_city_north_house_2f_2
	end

Sinnoh_OreburghCityNorthHouse2f_EventScript_Person3::
	lock
	faceplayer
	goto_if_set 0x3DBF, Sinnoh_OreburghCityNorthHouse2f_EventScript_Person3_Given
	msgbox Sinnoh_OreburghCityNorthHouse2f_Text_Person3, MSGBOX_DEFAULT
	msgbox Sinnoh_OreburghCityNorthHouse2f_Text_Person3Gift, MSGBOX_DEFAULT
	giveitem ITEM_HEAL_BALL, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DBF
	release
	end

Sinnoh_OreburghCityNorthHouse2f_EventScript_Person3_Given::
	msgbox Sinnoh_OreburghCityNorthHouse2f_Text_Person3, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghCityNorthHouse2f_Text_Person3:
	.string "I collect rocks. These ones are from\n"
	.string "the mine. That one's from the beach. I\l"
	.string "like them all.$"

Sinnoh_OreburghCityNorthHouse2f_Text_Person3Gift:
	.string "I don't collect these, though. This\n"
	.string "BALL is yours.$"

@ --- end of SINNOH words ---
