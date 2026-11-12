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
	goto SinnohP_pokemon_mansion_office_2
	end

Sinnoh_PokemonMansionOffice_EventScript_Person3::
	goto SinnohP_pokemon_mansion_office_3
	end

Sinnoh_PokemonMansionOffice_EventScript_Person4::
	msgbox Sinnoh_PokemonMansionOffice_Text_Person4, MSGBOX_NPC
	end

Sinnoh_PokemonMansionOffice_Text_Person4:
	.string "I guard the owner's office. Mostly I\n"
	.string "guard it from boredom.$"

@ --- end of SINNOH words ---
