Sinnoh_SolaceonTownPokemonNewsPress_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Bg1::
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person1::
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1, MSGBOX_NPC
	end

Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person2::
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SolaceonTownPokemonNewsPress_Text_Bg1:
	.string "The PC is switched off.$"

Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1:
	.string "We report on MOéMON news from all over\n"
	.string "SINNOH. Got a story for us?$"

Sinnoh_SolaceonTownPokemonNewsPress_Text_Person2:
	.string "I write the column about raising\n"
	.string "MOéMON. Readers send me so many\l"
	.string "questions!$"

@ --- end of SINNOH words ---
