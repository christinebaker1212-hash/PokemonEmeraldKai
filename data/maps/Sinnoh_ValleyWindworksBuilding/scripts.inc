Sinnoh_ValleyWindworksBuilding_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_ValleyWindworksBuilding_OnTransition
	.byte 0

Sinnoh_ValleyWindworksBuilding_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_ValleyWindworksBuilding_EventScript_Bg1::
	msgbox Sinnoh_ValleyWindworksBuilding_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_ValleyWindworksBuilding_EventScript_Bg2::
	msgbox Sinnoh_ValleyWindworksBuilding_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_ValleyWindworksBuilding_EventScript_Person3::
	msgbox Sinnoh_ValleyWindworksBuilding_Text_Person3, MSGBOX_NPC
	end

Sinnoh_ValleyWindworksBuilding_Text_Bg1:
	.string "The PC is switched off.$"

Sinnoh_ValleyWindworksBuilding_Text_Bg2:
	.string "The PC is switched off.$"

Sinnoh_ValleyWindworksBuilding_Text_Person3:
	.string "The windmills make power for the towns\n"
	.string "nearby. The wind here never stops!$"

@ --- end of SINNOH words ---
