Sinnoh_JubilifeTv2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_JubilifeTv2f_EventScript_Person1::
	goto SinnohP_jubilife_tv_2f_1
	end

Sinnoh_JubilifeTv2f_EventScript_Person2::
	goto SinnohP_jubilife_tv_2f_2
	end

Sinnoh_JubilifeTv2f_EventScript_Person3::
	msgbox Sinnoh_JubilifeTv2f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_JubilifeTv2f_EventScript_Person4::
	goto SinnohP_jubilife_tv_2f_3
	end

Sinnoh_JubilifeTv2f_Text_Person3:
	.string "We're always looking for stories! A\n"
	.string "TRAINER who sailed all the way to\l"
	.string "SINNOH? That's a good one.$"

@ --- end of SINNOH words ---
