Sinnoh_OreburghCityEastHouse2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityEastHouse2f_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DBD, Sinnoh_OreburghCityEastHouse2f_EventScript_Person1_Given
	msgbox Sinnoh_OreburghCityEastHouse2f_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_OreburghCityEastHouse2f_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_GREAT_BALL, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DBD
	release
	end

Sinnoh_OreburghCityEastHouse2f_EventScript_Person1_Given::
	msgbox Sinnoh_OreburghCityEastHouse2f_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghCityEastHouse2f_EventScript_Person2::
	msgbox Sinnoh_OreburghCityEastHouse2f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_OreburghCityEastHouse2f_EventScript_Person3::
	msgbox Sinnoh_OreburghCityEastHouse2f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_OreburghCityEastHouse2f_Text_Person1:
	.string "Did you know coal comes from plants? I\n"
	.string "didn't believe it either!$"

Sinnoh_OreburghCityEastHouse2f_Text_Person1Gift:
	.string "I traded a lump of coal for this BALL.\n"
	.string "You can have it!$"

Sinnoh_OreburghCityEastHouse2f_Text_Person2:
	.string "I've traveled all over SINNOH.\n"
	.string "OREBURGH's people are the hardest\l"
	.string "working of all.$"

Sinnoh_OreburghCityEastHouse2f_Text_Person3:
	.string "I'm researching how to burn coal more\n"
	.string "cleanly. It's slow work, but it matters.$"

@ --- end of SINNOH words ---
