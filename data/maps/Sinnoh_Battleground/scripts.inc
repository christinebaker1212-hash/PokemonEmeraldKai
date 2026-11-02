Sinnoh_Battleground_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Battleground_EventScript_Person1::
	goto SinnohP_battleground_1
	end

Sinnoh_Battleground_EventScript_Person2::
	msgbox Sinnoh_Battleground_Text_Person2, MSGBOX_NPC
	end

Sinnoh_Battleground_Text_Person2:
	.string "BUCK: Battleground's my favorite\n"
	.string "place to warm up. Wanna see me warm\l"
	.string "up? It's very warm.$"

@ --- end of SINNOH words ---
