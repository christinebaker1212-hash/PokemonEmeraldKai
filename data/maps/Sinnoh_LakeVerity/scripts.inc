Sinnoh_LakeVerity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_LakeVerity_OnTransition
	.byte 0

Sinnoh_LakeVerity_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_LakeVerity_EventScript_Person9::
	msgbox Sinnoh_LakeVerity_Text_Person9, MSGBOX_NPC
	end

Sinnoh_LakeVerity_Text_Person9:
	.string "COMMANDER MARS is busy catching the\n"
	.string "lake's POKéMON. Wait your turn!$"

@ --- end of SINNOH words ---
