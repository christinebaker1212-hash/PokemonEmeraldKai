Sinnoh_GrandLakeValorLakefrontEastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_GrandLakeValorLakefrontEastHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DB5, Sinnoh_GrandLakeValorLakefrontEastHouse_EventScript_Person1_Given
	msgbox Sinnoh_GrandLakeValorLakefrontEastHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_GrandLakeValorLakefrontEastHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_WHITE_FLUTE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DB5
	release
	end

Sinnoh_GrandLakeValorLakefrontEastHouse_EventScript_Person1_Given::
	msgbox Sinnoh_GrandLakeValorLakefrontEastHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_GrandLakeValorLakefrontEastHouse_Text_Person1:
	.string "My view of the lake was the best in\n"
	.string "town. I just want the water back.$"

Sinnoh_GrandLakeValorLakefrontEastHouse_Text_Person1Gift:
	.string "Take this WHITE FLUTE. Its sound\n"
	.string "makes the grass rustle with POKéMON.$"

@ --- end of SINNOH words ---
