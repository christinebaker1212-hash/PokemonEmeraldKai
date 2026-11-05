Sinnoh_ValleyWindworksOutside_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_ValleyWindworksOutside_OnTransition
	.byte 0

Sinnoh_ValleyWindworksOutside_OnTransition:
	special Sinnoh_LegendsRefresh
	checkitem ITEM_WORKS_KEY   @ Moemon Kai: a WORKS KEY held opens the door (Platinum's flag for having it,
	call_if_eq VAR_RESULT, TRUE, Sinnoh_ValleyWindworksOutside_EventScript_HasWorksKey   @ missing on some saves)
	end

Sinnoh_ValleyWindworksOutside_EventScript_HasWorksKey:
	setflag 0x3564
	return

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_ValleyWindworksOutside_EventScript_Bg2::
	goto SinnohP_valley_windworks_outside_4
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
