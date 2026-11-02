Sinnoh_JubilifeTv3f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_JubilifeTv3f_EventScript_Person1::
	msgbox Sinnoh_JubilifeTv3f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_JubilifeTv3f_EventScript_Person2::
	goto SinnohP_jubilife_tv_3f_2
	end

Sinnoh_JubilifeTv3f_EventScript_Person3::
	goto SinnohP_jubilife_tv_3f_3
	end

Sinnoh_JubilifeTv3f_EventScript_Person4::
	msgbox Sinnoh_JubilifeTv3f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_JubilifeTv3f_Text_Person1:
	.string "The ranking rooms show who's done the\n"
	.string "most in SINNOH. My name's on the list!\l"
	.string "Somewhere.$"

Sinnoh_JubilifeTv3f_Text_Person4:
	.string "The rankings change every day.\n"
	.string "TRAINERS all over SINNOH are working\l"
	.string "hard.$"

@ --- end of SINNOH words ---
