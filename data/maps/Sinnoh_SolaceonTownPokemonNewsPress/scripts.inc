Sinnoh_SolaceonTownPokemonNewsPress_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Bg1::
	goto SinnohP_solaceon_town_pokemon_news_press_3
	end

Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DE3, Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person1_Given
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_HEART_SCALE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DE3
	release
	end

Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person1_Given::
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person2::
	goto SinnohP_solaceon_town_pokemon_news_press_2
	end

Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1:
	.string "We report on MOéMON news from all over\n"
	.string "SINNOH. Got a story for us?$"

Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1Gift:
	.string "Thanks for listening! Here's a pretty\n"
	.string "scale a reader sent us.$"

@ --- end of SINNOH words ---
