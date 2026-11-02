Sinnoh_GrandLakeRoute213Lobby_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_GrandLakeRoute213Lobby_EventScript_Person1::
	goto SinnohP_grand_lake_route_213_lobby_3
	end

Sinnoh_GrandLakeRoute213Lobby_EventScript_Person2::
	msgbox Sinnoh_GrandLakeRoute213Lobby_Text_Person2, MSGBOX_NPC
	end

Sinnoh_GrandLakeRoute213Lobby_EventScript_Person3::
	msgbox Sinnoh_GrandLakeRoute213Lobby_Text_Person3, MSGBOX_NPC
	end

Sinnoh_GrandLakeRoute213Lobby_Text_Person2:
	.string "I keep the peace around here. It's\n"
	.string "mostly lost sunglasses.$"

Sinnoh_GrandLakeRoute213Lobby_Text_Person3:
	.string "LOOKER: This hotel has a fine view of\n"
	.string "LAKE VALOR. And of anyone sneaking\l"
	.string "toward it.$"

@ --- end of SINNOH words ---
