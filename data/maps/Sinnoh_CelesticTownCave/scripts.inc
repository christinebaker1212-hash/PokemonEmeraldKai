Sinnoh_CelesticTownCave_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_CelesticTownCave_OnTransition
	.byte 0

Sinnoh_CelesticTownCave_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CelesticTownCave_EventScript_Bg1::
	goto SinnohP_celestic_town_cave_1
	end

Sinnoh_CelesticTownCave_EventScript_Bg2::
	goto SinnohP_celestic_town_cave_1
	end

Sinnoh_CelesticTownCave_EventScript_Person1::
	msgbox Sinnoh_CelesticTownCave_Text_Person1, MSGBOX_NPC
	end

Sinnoh_CelesticTownCave_EventScript_Person3::
	msgbox Sinnoh_CelesticTownCave_Text_Person3, MSGBOX_NPC
	end

Sinnoh_CelesticTownCave_Text_Person1:
	.string "The painting in this cave shows the\n"
	.string "beginning of everything. Treat it\l"
	.string "gently.$"

Sinnoh_CelesticTownCave_Text_Person3:
	.string "CYNTHIA: Look at the wall. Two\n"
	.string "POKéMON, and one in the shadow\l"
	.string "between them.\p"
	.string "Every legend in SINNOH comes back to\n"
	.string "this one.$"

@ --- end of SINNOH words ---
