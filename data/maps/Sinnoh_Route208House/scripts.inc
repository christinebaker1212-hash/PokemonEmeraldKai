Sinnoh_Route208House_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route208House_EventScript_Person1::
	msgbox Sinnoh_Route208House_Text_Person1, MSGBOX_NPC
	end

Sinnoh_Route208House_EventScript_Person2::
	goto SinnohP_route_208_house_3
	end

Sinnoh_Route208House_EventScript_Person3::
	msgbox Sinnoh_Route208House_Text_Person3, MSGBOX_NPC
	end

Sinnoh_Route208House_EventScript_Sign4::
	goto SinnohP_route_208_house_5
	end

Sinnoh_Route208House_Text_Person1:
	.string "I've grown BERRIES my whole life. Each\n"
	.string "one has its own taste, and its own\l"
	.string "story.$"

Sinnoh_Route208House_Text_Person3:
	.string "BERRIES make MOéMON happy. And\n"
	.string "people too, if you bake them into a\l"
	.string "pie.$"

@ --- end of SINNOH words ---
