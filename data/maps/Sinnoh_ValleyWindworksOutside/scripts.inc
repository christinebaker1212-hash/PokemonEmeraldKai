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

Sinnoh_ValleyWindworksOutside_EventScript_Sign2::
	msgbox Sinnoh_ValleyWindworksOutside_Text_Sign2, MSGBOX_SIGN
	end

Sinnoh_ValleyWindworksOutside_Text_Bg2:
	.string "The door is locked.$"

Sinnoh_ValleyWindworksOutside_Text_Sign2:
	.string "VALLEY WINDWORKS\p"
	.string "The wind turns the turbines, the\n"
	.string "turbines light the towns.$"

@ --- end of SINNOH words ---
