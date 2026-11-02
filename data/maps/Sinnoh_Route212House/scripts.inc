Sinnoh_Route212House_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route212House_EventScript_Bg1::
	goto SinnohP_route_212_house_4
	end

Sinnoh_Route212House_EventScript_Person1::
	goto SinnohP_route_212_house_3
	end

Sinnoh_Route212House_EventScript_Person2::
	msgbox Sinnoh_Route212House_Text_Person2, MSGBOX_NPC
	end

Sinnoh_Route212House_EventScript_Person3::
	goto SinnohP_route_212_house_2
	end

Sinnoh_Route212House_Text_Person2:
	.string "I came down from the mountains to\n"
	.string "escape the snow. Now it's rain\l"
	.string "instead!$"

@ --- end of SINNOH words ---
