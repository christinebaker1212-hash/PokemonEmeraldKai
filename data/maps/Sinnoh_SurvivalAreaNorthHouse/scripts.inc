Sinnoh_SurvivalAreaNorthHouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SurvivalAreaNorthHouse_EventScript_Bg2::
	msgbox Sinnoh_SurvivalAreaNorthHouse_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_SurvivalAreaNorthHouse_EventScript_Person1::
	msgbox Sinnoh_SurvivalAreaNorthHouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SurvivalAreaNorthHouse_EventScript_Person2::
	msgbox Sinnoh_SurvivalAreaNorthHouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SurvivalAreaNorthHouse_Text_Bg2:
	.string "It's full of handwriting. It doesn't\n"
	.string "seem polite to read it.$"

Sinnoh_SurvivalAreaNorthHouse_Text_Person1:
	.string "I came here to climb STARK MOUNTAIN.\n"
	.string "The heat is no joke!$"

Sinnoh_SurvivalAreaNorthHouse_Text_Person2:
	.string "I sense a great power deep inside the\n"
	.string "mountain...$"

@ --- end of SINNOH words ---
