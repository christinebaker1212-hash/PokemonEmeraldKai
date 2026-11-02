Sinnoh_Route227_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route227_EventScript_Person4::
	msgbox Sinnoh_Route227_Text_Person4, MSGBOX_NPC
	end

Sinnoh_Route227_EventScript_Person5::
	msgbox Sinnoh_Route227_Text_Person5, MSGBOX_NPC
	end

Sinnoh_Route227_EventScript_Person10::
	goto SinnohP_common_28
	end

Sinnoh_Route227_EventScript_Person11::
	goto SinnohP_common_28
	end

Sinnoh_Route227_EventScript_Person12::
	msgbox Sinnoh_Route227_Text_Person12, MSGBOX_NPC
	end

Sinnoh_Route227_EventScript_Sign1::
	goto SinnohP_route_227_2
	end

Sinnoh_Route227_Text_Person4:
	.string "BARRY: STARK MOUNTAIN's up ahead! Its\n"
	.string "ash gets in your shoes. I'm gonna fine\l"
	.string "the volcano!$"

Sinnoh_Route227_Text_Person5:
	.string "WAKE: The ashy road makes for great\n"
	.string "training! Hah! My cape is gray now.$"

Sinnoh_Route227_Text_Person12:
	.string "BUCK: Up the road is STARK MOUNTAIN.\n"
	.string "TEAM GALACTIC's leftovers went in.\l"
	.string "Let's go!$"

@ --- end of SINNOH words ---
