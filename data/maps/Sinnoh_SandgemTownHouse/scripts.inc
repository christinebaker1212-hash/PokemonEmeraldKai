Sinnoh_SandgemTownHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SandgemTownHouse_EventScript_Person1::
	msgbox Sinnoh_SandgemTownHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SandgemTownHouse_EventScript_Person2::
	msgbox Sinnoh_SandgemTownHouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SandgemTownHouse_Text_Person1:
	.string "We came to SANDGEM for the beach and\n"
	.string "stayed for the people.\p"
	.string "Well, mostly for the beach.$"

Sinnoh_SandgemTownHouse_Text_Person2:
	.string "Our BUNEARY runs off to the shore\n"
	.string "every morning.\p"
	.string "It comes back with its ears full of\n"
	.string "sand, every single time.$"

@ --- end of SINNOH words ---
