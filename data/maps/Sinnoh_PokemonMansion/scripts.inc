Sinnoh_PokemonMansion_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonMansion_EventScript_Person1::
	goto SinnohP_pokemon_mansion_2
	end

Sinnoh_PokemonMansion_EventScript_Person2::
	goto SinnohP_pokemon_mansion_3
	end

Sinnoh_PokemonMansion_EventScript_Person3::
	msgbox Sinnoh_PokemonMansion_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PokemonMansion_EventScript_Person4::
	goto SinnohP_pokemon_mansion_5
	end

Sinnoh_PokemonMansion_EventScript_Person7::
	goto SinnohP_pokemon_mansion_6
	end

Sinnoh_PokemonMansion_Text_Person3:
	.string "Please enjoy the mansion. Don't touch\n"
	.string "the statues, though!$"

@ --- end of SINNOH words ---
