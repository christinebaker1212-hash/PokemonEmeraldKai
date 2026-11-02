Sinnoh_PastoriaCityMiddleHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityMiddleHouse_EventScript_Person1::
	goto SinnohP_pastoria_city_middle_house_1
	end

Sinnoh_PastoriaCityMiddleHouse_EventScript_Person2::
	lock
	faceplayer
	goto_if_set 0x3DC6, Sinnoh_PastoriaCityMiddleHouse_EventScript_Person2_Given
	msgbox Sinnoh_PastoriaCityMiddleHouse_Text_Person2, MSGBOX_DEFAULT
	msgbox Sinnoh_PastoriaCityMiddleHouse_Text_Person2Gift, MSGBOX_DEFAULT
	giveitem ITEM_MACHO_BRACE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DC6
	release
	end

Sinnoh_PastoriaCityMiddleHouse_EventScript_Person2_Given::
	msgbox Sinnoh_PastoriaCityMiddleHouse_Text_Person2, MSGBOX_DEFAULT
	release
	end

Sinnoh_PastoriaCityMiddleHouse_Text_Person2:
	.string "I caught a SKORUPI in the marsh! Okay,\n"
	.string "it caught me first. Then I caught it.$"

Sinnoh_PastoriaCityMiddleHouse_Text_Person2Gift:
	.string "It made me so strong, I don't need it\n"
	.string "anymore. Here!$"

@ --- end of SINNOH words ---
