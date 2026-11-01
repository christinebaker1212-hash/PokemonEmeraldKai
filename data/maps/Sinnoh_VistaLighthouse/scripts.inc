Sinnoh_VistaLighthouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VistaLighthouse_EventScript_Bg1::
	msgbox Sinnoh_VistaLighthouse_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_VistaLighthouse_EventScript_Person1::
	msgbox Sinnoh_VistaLighthouse_Text_Person1, MSGBOX_NPC
	end

Sinnoh_VistaLighthouse_EventScript_Person2::
	msgbox Sinnoh_VistaLighthouse_Text_Person2, MSGBOX_NPC
	end

Sinnoh_VistaLighthouse_Text_Bg1:
	.string "A pair of binoculars on a stand. You\n"
	.string "can see far across the water.$"

Sinnoh_VistaLighthouse_Text_Person1:
	.string "The lighthouse guides ships home.\n"
	.string "VOLKNER sometimes comes up here to\l"
	.string "think.$"

Sinnoh_VistaLighthouse_Text_Person2:
	.string "From the top you can see the MOéMON\n"
	.string "LEAGUE! It's so far away...$"

@ --- end of SINNOH words ---
