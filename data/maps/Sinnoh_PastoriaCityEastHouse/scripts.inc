Sinnoh_PastoriaCityEastHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityEastHouse_EventScript_Person1::
	msgbox Sinnoh_PastoriaCityEastHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_PastoriaCityEastHouse_EventScript_Person2::
	goto SinnohP_pastoria_city_east_house_2
	end

Sinnoh_PastoriaCityEastHouse_Text_Person1:
	.string "I know every move a MOéMON could ever\n"
	.string "learn. Ask me about any of them!\l"
	.string "...Later. I'm busy.$"

@ --- end of SINNOH words ---
