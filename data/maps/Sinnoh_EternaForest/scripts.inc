Sinnoh_EternaForest_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaForest_EventScript_Person1::
	msgbox Sinnoh_EternaForest_Text_Person1, MSGBOX_NPC
	end

Sinnoh_EternaForest_EventScript_Person2::
	goto SinnohP_eterna_forest_6
	end

Sinnoh_EternaForest_EventScript_Person17::
	goto SinnohP_eterna_forest_10
	end

Sinnoh_EternaForest_EventScript_Person25::
	msgbox Sinnoh_EternaForest_Text_Person25, MSGBOX_NPC
	end

Sinnoh_EternaForest_EventScript_Sign13::
	goto SinnohP_eterna_forest_8
	end

Sinnoh_EternaForest_EventScript_Sign14::
	goto SinnohP_eterna_forest_9
	end

Sinnoh_EternaForest_Text_Person1:
	.string "CHERYL: The forest is so dark... Would\n"
	.string "you mind walking with me? My CHANSEY\l"
	.string "and I heal anyone who needs it.\p"
	.string "I'll patch up your team whenever you\n"
	.string "ask.$"

Sinnoh_EternaForest_Text_Person25:
	.string "GARDENIA: You're in ETERNA FOREST! I'm\n"
	.string "GARDENIA, ETERNA's GYM LEADER.\p"
	.string "I come here to listen to the trees.\n"
	.string "Visit my GYM soon!$"

@ --- end of SINNOH words ---
