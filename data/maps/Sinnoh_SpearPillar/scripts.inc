Sinnoh_SpearPillar_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SpearPillar_OnTransition
	.byte 0

Sinnoh_SpearPillar_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SpearPillar_EventScript_Person1::
	msgbox Sinnoh_SpearPillar_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SpearPillar_EventScript_Person2::
	msgbox Sinnoh_SpearPillar_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SpearPillar_EventScript_Person4::
	msgbox Sinnoh_SpearPillar_Text_Person4, MSGBOX_NPC
	end

Sinnoh_SpearPillar_EventScript_Person5::
	msgbox Sinnoh_SpearPillar_Text_Person5, MSGBOX_NPC
	end

Sinnoh_SpearPillar_EventScript_Person6::
	msgbox Sinnoh_SpearPillar_Text_Person6, MSGBOX_NPC
	end

Sinnoh_SpearPillar_Text_Person1:
	.string "SPEAR PILLAR is ours! The BOSS is\n"
	.string "calling the POKéMON of time and space!$"

Sinnoh_SpearPillar_Text_Person2:
	.string "CYRUS: You stopped my commanders. You\n"
	.string "will not stop the world from changing.\p"
	.string "...Yet the red chains are broken, and\n"
	.string "the sky is still the same sky. Perhaps\l"
	.string "you have won.$"

Sinnoh_SpearPillar_Text_Person4:
	.string "Did you feel that? The ground's\n"
	.string "humming. Something big is coming.$"

Sinnoh_SpearPillar_Text_Person5:
	.string "JUPITER: MARS, together! This one's\n"
	.string "too much for either of us alone.$"

Sinnoh_SpearPillar_Text_Person6:
	.string "BARRY: I made it! Whoa, look at them\n"
	.string "all. Let's take them on together!$"

@ --- end of SINNOH words ---
