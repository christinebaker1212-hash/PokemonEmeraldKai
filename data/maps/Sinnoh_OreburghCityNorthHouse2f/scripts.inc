Sinnoh_OreburghCityNorthHouse2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityNorthHouse2f_EventScript_Person1::
	msgbox Sinnoh_OreburghCityNorthHouse2f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_OreburghCityNorthHouse2f_EventScript_Person2::
	msgbox Sinnoh_OreburghCityNorthHouse2f_Text_Person2, MSGBOX_NPC
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

Sinnoh_OreburghCityNorthHouse2f_Text_Person1:
	.string "I'm writing a song about OREBURGH. It\n"
	.string "goes “Dig, dig, dig, coal, coal, coal.”\l"
	.string "Needs work.$"

Sinnoh_OreburghCityNorthHouse2f_Text_Person2:
	.string "This is my quiet place. Downstairs\n"
	.string "gets so busy when the miners come\l"
	.string "home.$"

Sinnoh_OreburghCityNorthHouse2f_Text_Person3:
	.string "I collect rocks. These ones are from\n"
	.string "the mine. That one's from the beach. I\l"
	.string "like them all.$"

Sinnoh_OreburghCityNorthHouse2f_Text_Person3Gift:
	.string "I don't collect these, though. This\n"
	.string "BALL is yours.$"

@ --- end of SINNOH words ---
