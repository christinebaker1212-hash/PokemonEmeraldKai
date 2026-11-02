Sinnoh_PastoriaCityNortheastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityNortheastHouse_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DC4, Sinnoh_PastoriaCityNortheastHouse_EventScript_Person1_Given
	msgbox Sinnoh_PastoriaCityNortheastHouse_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_PastoriaCityNortheastHouse_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_RED_SCARF, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DC4
	release
	end

Sinnoh_PastoriaCityNortheastHouse_EventScript_Person1_Given::
	msgbox Sinnoh_PastoriaCityNortheastHouse_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_PastoriaCityNortheastHouse_EventScript_Person2::
	goto SinnohP_pastoria_city_northeast_house_2
	end

Sinnoh_PastoriaCityNortheastHouse_EventScript_Person3::
	goto SinnohP_pastoria_city_northeast_house_3
	end

Sinnoh_PastoriaCityNortheastHouse_Text_Person1:
	.string "My sister and I both want to beat\n"
	.string "CRASHER WAKE. Whoever does it first\l"
	.string "gets dessert.$"

Sinnoh_PastoriaCityNortheastHouse_Text_Person1Gift:
	.string "My sister made scarves for every\n"
	.string "MOéMON we know. Here's a spare!$"

@ --- end of SINNOH words ---
