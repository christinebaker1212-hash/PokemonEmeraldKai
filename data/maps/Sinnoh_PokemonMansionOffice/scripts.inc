Sinnoh_PokemonMansionOffice_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonMansionOffice_EventScript_Bg1::
	msgbox Sinnoh_PokemonMansionOffice_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_PokemonMansionOffice_EventScript_Bg2::
	msgbox Sinnoh_PokemonMansionOffice_Text_Bg2, MSGBOX_SIGN
	end

Sinnoh_PokemonMansionOffice_EventScript_Person2::
	goto Sinnoh_EventScript_MrBacklot
	end

Sinnoh_PokemonMansionOffice_EventScript_Person3::
	msgbox Sinnoh_PokemonMansionOffice_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PokemonMansionOffice_Text_Bg1:
	.string "A stone statue of a MOéMON. It looks\n"
	.string "as if it's guarding something.$"

Sinnoh_PokemonMansionOffice_Text_Bg2:
	.string "It's full of handwriting. It doesn't\n"
	.string "seem polite to read it.$"

Sinnoh_PokemonMansionOffice_Text_Person3:
	.string "I've been the master's butler for\n"
	.string "forty years. He's never once been on\l"
	.string "time.$"

@ --- end of SINNOH words ---
