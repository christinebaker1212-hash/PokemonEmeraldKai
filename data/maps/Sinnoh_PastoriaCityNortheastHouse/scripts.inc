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
	msgbox Sinnoh_PastoriaCityNortheastHouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_PastoriaCityNortheastHouse_EventScript_Person3::
	msgbox Sinnoh_PastoriaCityNortheastHouse_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PastoriaCityNortheastHouse_Text_Person1:
	.string "My sister and I both want to beat\n"
	.string "CRASHER WAKE. Whoever does it first\l"
	.string "gets dessert.$"

Sinnoh_PastoriaCityNortheastHouse_Text_Person1Gift:
	.string "My sister made scarves for every\n"
	.string "MOéMON we know. Here's a spare!$"

Sinnoh_PastoriaCityNortheastHouse_Text_Person2:
	.string "I'm going to win! My brother still can't\n"
	.string "get past WAKE's GYARADOS.$"

Sinnoh_PastoriaCityNortheastHouse_Text_Person3:
	.string "PACHIRISU: Chi!\p"
	.string "It's cheering for someone.$"

@ --- end of SINNOH words ---
