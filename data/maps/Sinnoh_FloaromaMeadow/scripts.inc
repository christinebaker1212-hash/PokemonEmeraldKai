Sinnoh_FloaromaMeadow_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FloaromaMeadow_EventScript_Person1::
	msgbox Sinnoh_FloaromaMeadow_Text_Person1, MSGBOX_NPC
	end

Sinnoh_FloaromaMeadow_EventScript_Person2::
	msgbox Sinnoh_FloaromaMeadow_Text_Person2, MSGBOX_NPC
	end

Sinnoh_FloaromaMeadow_EventScript_Person3::
	goto Sinnoh_EventScript_HoneySeller
	end

Sinnoh_FloaromaMeadow_Text_Person1:
	.string "This HONEY is ours now! TEAM GALACTIC\n"
	.string "needs it to lure rare POKéMON.$"

Sinnoh_FloaromaMeadow_Text_Person2:
	.string "We've got the WINDWORKS' key too. Not\n"
	.string "that I'll tell YOU that. Oops.$"

@ --- end of SINNOH words ---
