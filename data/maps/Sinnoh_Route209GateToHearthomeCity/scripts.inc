Sinnoh_Route209GateToHearthomeCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_Route209GateToHearthomeCity_OnTransition
	.byte 0

Sinnoh_Route209GateToHearthomeCity_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_Route209GateToHearthomeCity_EventScript_Person1::
	msgbox Sinnoh_Route209GateToHearthomeCity_Text_Person1, MSGBOX_NPC
	end

Sinnoh_Route209GateToHearthomeCity_Text_Person1:
	.string "The road to SOLACEON is long but\n"
	.string "pretty. Watch for the old tower!$"

@ --- end of SINNOH words ---
