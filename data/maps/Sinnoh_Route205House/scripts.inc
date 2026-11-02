Sinnoh_Route205House_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route205House_EventScript_Person1::
	msgbox Sinnoh_Route205House_Text_Person1, MSGBOX_NPC
	end

Sinnoh_Route205House_EventScript_Person2::
	goto SinnohP_route_205_house_2
	end

Sinnoh_Route205House_Text_Person1:
	.string "Welcome! Rest your feet. The forest\n"
	.string "path is long.$"

@ --- end of SINNOH words ---
