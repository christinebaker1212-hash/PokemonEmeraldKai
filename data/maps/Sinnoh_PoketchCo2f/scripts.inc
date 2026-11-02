Sinnoh_PoketchCo2f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PoketchCo2f_EventScript_Person1::
	goto SinnohP_poketch_co_2f_1
	end

Sinnoh_PoketchCo2f_EventScript_Person2::
	goto SinnohP_poketch_co_2f_2
	end

Sinnoh_PoketchCo2f_EventScript_Person3::
	msgbox Sinnoh_PoketchCo2f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PoketchCo2f_EventScript_Person4::
	msgbox Sinnoh_PoketchCo2f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_PoketchCo2f_Text_Person3:
	.string "CLEFAIRY: Pippi pi!\p"
	.string "It's tapping on a watch face,\n"
	.string "fascinated.$"

Sinnoh_PoketchCo2f_Text_Person4:
	.string "PACHIRISU: Chipa!\p"
	.string "It's chewing on a watch strap.$"

@ --- end of SINNOH words ---
