Sinnoh_ResortAreaHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_ResortAreaHouse_EventScript_Person1::
	msgbox Sinnoh_ResortAreaHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_ResortAreaHouse_EventScript_Person2::
	goto SinnohP_resort_area_house_2
	end

Sinnoh_ResortAreaHouse_EventScript_Person3::
	msgbox Sinnoh_ResortAreaHouse_Text_Person3, MSGBOX_NPC
	end

Sinnoh_ResortAreaHouse_Text_Person1:
	.string "I collect seashells from the beach.\n"
	.string "This one is my favorite.$"

Sinnoh_ResortAreaHouse_Text_Person3:
	.string "I'm going to swim all the way to\n"
	.string "SUNYSHORE someday!$"

@ --- end of SINNOH words ---
