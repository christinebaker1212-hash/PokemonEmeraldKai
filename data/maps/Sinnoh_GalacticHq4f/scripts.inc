Sinnoh_GalacticHq4f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_GalacticHq4f_OnTransition
	.byte 0

Sinnoh_GalacticHq4f_OnTransition:
	special Sinnoh_LegendsRefresh
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_GalacticHq4f_EventScript_Bg2::
	msgbox Sinnoh_GalacticHq4f_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_GalacticHq4f_EventScript_Bg3::
	msgbox Sinnoh_GalacticHq4f_Text_Bg3, MSGBOX_SIGN
	end

Sinnoh_GalacticHq4f_Text_Bg2:
	.string "A notice on the wall.$"

Sinnoh_GalacticHq4f_Text_Bg3:
	.string "It's full of handwriting. It doesn't\n"
	.string "seem polite to read it.$"

@ --- end of SINNOH words ---
