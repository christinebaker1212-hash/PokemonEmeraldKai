Sinnoh_PokemonMansionOffice_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonMansionOffice_EventScript_Bg1::
	goto SinnohP_pokemon_mansion_office_6
	end

Sinnoh_PokemonMansionOffice_EventScript_Bg2::
	goto SinnohP_pokemon_mansion_office_7
	end

Sinnoh_PokemonMansionOffice_EventScript_Person2::
	goto Sinnoh_EventScript_MrBacklot
	end

Sinnoh_PokemonMansionOffice_EventScript_Person3::
	msgbox Sinnoh_PokemonMansionOffice_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PokemonMansionOffice_EventScript_Person4::
	msgbox Sinnoh_PokemonMansionOffice_Text_Person4, MSGBOX_NPC
	end

Sinnoh_PokemonMansionOffice_Text_Person3:
	.string "I've been the master's butler for\n"
	.string "forty years. He's never once been on\l"
	.string "time.$"

Sinnoh_PokemonMansionOffice_Text_Person4:
	.string "I guard the owner's office. Mostly I\n"
	.string "guard it from boredom.$"

@ --- end of SINNOH words ---
