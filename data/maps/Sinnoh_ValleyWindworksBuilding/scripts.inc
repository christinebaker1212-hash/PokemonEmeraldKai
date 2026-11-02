Sinnoh_ValleyWindworksBuilding_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_ValleyWindworksBuilding_OnTransition
	.byte 0

Sinnoh_ValleyWindworksBuilding_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_ValleyWindworksBuilding_EventScript_Bg1::
	goto SinnohP_valley_windworks_building_7
	end

Sinnoh_ValleyWindworksBuilding_EventScript_Bg2::
	goto SinnohP_valley_windworks_building_8
	end

Sinnoh_ValleyWindworksBuilding_EventScript_Person1::
	msgbox Sinnoh_ValleyWindworksBuilding_Text_Person1, MSGBOX_NPC
	end

Sinnoh_ValleyWindworksBuilding_EventScript_Person3::
	goto SinnohP_valley_windworks_building_5
	end

Sinnoh_ValleyWindworksBuilding_EventScript_Person6::
	msgbox Sinnoh_ValleyWindworksBuilding_Text_Person6, MSGBOX_NPC
	end

Sinnoh_ValleyWindworksBuilding_EventScript_Person7::
	msgbox Sinnoh_ValleyWindworksBuilding_Text_Person7, MSGBOX_NPC
	end

Sinnoh_ValleyWindworksBuilding_EventScript_Person8::
	msgbox Sinnoh_ValleyWindworksBuilding_Text_Person8, MSGBOX_NPC
	end

Sinnoh_ValleyWindworksBuilding_Text_Person1:
	.string "Turn back! COMMANDER MARS is busy and\n"
	.string "doesn't want visitors.$"

Sinnoh_ValleyWindworksBuilding_Text_Person6:
	.string "My daddy's back! Thank you for\n"
	.string "chasing those mean people away!$"

Sinnoh_ValleyWindworksBuilding_Text_Person7:
	.string "This place makes so much energy.\n"
	.string "Imagine what the BOSS could do with\l"
	.string "it.$"

Sinnoh_ValleyWindworksBuilding_Text_Person8:
	.string "CHARON: Heh heh. Energy is the key to\n"
	.string "everything, you see.\p"
	.string "A little old scientist like me needs a\n"
	.string "LOT of it.$"

@ --- end of SINNOH words ---
