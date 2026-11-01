Sinnoh_SandgemTownCounterpartHouse1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SandgemTownCounterpartHouse1f_EventScript_Person1::
	msgbox Sinnoh_SandgemTownCounterpartHouse1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SandgemTownCounterpartHouse1f_EventScript_Person2::
	goto Sinnoh_EventScript_SwarmTwin
	end

Sinnoh_SandgemTownCounterpartHouse1f_Text_Person1:
	.string "My grandchild helps out at the MOéMON\n"
	.string "lab.\p"
	.string "Always running errands for the\n"
	.string "professor. I'm proud as can be.$"

@ --- end of SINNOH words ---
