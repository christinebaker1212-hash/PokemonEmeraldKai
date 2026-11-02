Sinnoh_ValleyWindworksOutside_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_ValleyWindworksOutside_OnTransition
	.byte 0

Sinnoh_ValleyWindworksOutside_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_ValleyWindworksOutside_EventScript_Bg2::
	msgbox Sinnoh_ValleyWindworksOutside_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_ValleyWindworksOutside_EventScript_Person1::
	msgbox Sinnoh_ValleyWindworksOutside_Text_Person1, MSGBOX_NPC
	end

Sinnoh_ValleyWindworksOutside_EventScript_Person7::
	msgbox Sinnoh_ValleyWindworksOutside_Text_Person7, MSGBOX_NPC
	end

Sinnoh_ValleyWindworksOutside_EventScript_Sign2::
	goto SinnohP_valley_windworks_outside_5
	end

Sinnoh_ValleyWindworksOutside_Text_Bg2:
	.string "The door is locked.$"

Sinnoh_ValleyWindworksOutside_Text_Person1:
	.string "The WINDWORKS is closed! TEAM\n"
	.string "GALACTIC orders. Shoo!$"

Sinnoh_ValleyWindworksOutside_Text_Person7:
	.string "LOOKER: TEAM GALACTIC has taken the\n"
	.string "WINDWORKS. Its energy could power\l"
	.string "something very large.\p"
	.string "Their leader inside is a woman called\n"
	.string "MARS. Be careful.$"

@ --- end of SINNOH words ---
