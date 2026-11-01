Sinnoh_OreburghCityNorthwestHouse2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityNorthwestHouse2f_EventScript_Person1::
	msgbox Sinnoh_OreburghCityNorthwestHouse2f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_OreburghCityNorthwestHouse2f_EventScript_Person2::
	msgbox Sinnoh_OreburghCityNorthwestHouse2f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_OreburghCityNorthwestHouse2f_EventScript_Person3::
	lock
	faceplayer
	goto_if_set 0x3DC0, Sinnoh_OreburghCityNorthwestHouse2f_EventScript_Person3_Given
	msgbox Sinnoh_OreburghCityNorthwestHouse2f_Text_Person3, MSGBOX_DEFAULT
	msgbox Sinnoh_OreburghCityNorthwestHouse2f_Text_Person3Gift, MSGBOX_DEFAULT
	giveitem ITEM_DUSK_BALL, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DC0
	release
	end

Sinnoh_OreburghCityNorthwestHouse2f_EventScript_Person3_Given::
	msgbox Sinnoh_OreburghCityNorthwestHouse2f_Text_Person3, MSGBOX_DEFAULT
	release
	end

Sinnoh_OreburghCityNorthwestHouse2f_EventScript_Person4::
	msgbox Sinnoh_OreburghCityNorthwestHouse2f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_OreburghCityNorthwestHouse2f_Text_Person1:
	.string "After a long day in the mine, there's\n"
	.string "nothing like a nap with my PSYDUCK.$"

Sinnoh_OreburghCityNorthwestHouse2f_Text_Person2:
	.string "I'm drawing a map of the mine. It's\n"
	.string "mostly squiggly lines so far.$"

Sinnoh_OreburghCityNorthwestHouse2f_Text_Person3:
	.string "Did you see the GYM? It's carved right\n"
	.string "into the rock. I think it's so cool.$"

Sinnoh_OreburghCityNorthwestHouse2f_Text_Person3Gift:
	.string "Want this BALL? It works best in dark\n"
	.string "places, like the mine!$"

Sinnoh_OreburghCityNorthwestHouse2f_Text_Person4:
	.string "PSYDUCK: Psy...\p"
	.string "It's taking a nap. Or thinking very\n"
	.string "hard. Hard to tell.$"

@ --- end of SINNOH words ---
