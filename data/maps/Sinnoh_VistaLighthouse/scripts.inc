Sinnoh_VistaLighthouse_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VistaLighthouse_EventScript_Bg1::
	msgbox Sinnoh_VistaLighthouse_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_VistaLighthouse_EventScript_Person1::
	goto SinnohP_vista_lighthouse_1
	end

Sinnoh_VistaLighthouse_EventScript_Person2::
	goto SinnohP_vista_lighthouse_2
	end

Sinnoh_VistaLighthouse_EventScript_Person3::
	msgbox Sinnoh_VistaLighthouse_Text_Person3, MSGBOX_NPC
	end

Sinnoh_VistaLighthouse_Text_Bg1:
	.string "A pair of binoculars on a stand. You\n"
	.string "can see far across the water.$"

Sinnoh_VistaLighthouse_Text_Person3:
	.string "VOLKNER: From up here you can see all\n"
	.string "the way to the LEAGUE.\p"
	.string "I've been waiting for a TRAINER who\n"
	.string "makes battles exciting again. Maybe\l"
	.string "it's you.$"

@ --- end of SINNOH words ---
