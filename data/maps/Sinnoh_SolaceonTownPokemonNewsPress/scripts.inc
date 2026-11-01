Sinnoh_SolaceonTownPokemonNewsPress_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Bg1::
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Bg1, MSGBOX_SIGN
	end

Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person1::
	lock
	faceplayer
	goto_if_set 0x3DDA, Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person1_Given
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1, MSGBOX_DEFAULT
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1Gift, MSGBOX_DEFAULT
	giveitem ITEM_HEART_SCALE, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DDA
	release
	end

Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person1_Given::
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1, MSGBOX_DEFAULT
	release
	end

Sinnoh_SolaceonTownPokemonNewsPress_EventScript_Person2::
	msgbox Sinnoh_SolaceonTownPokemonNewsPress_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SolaceonTownPokemonNewsPress_Text_Bg1:
	.string "The PC is switched off.$"

Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1:
	.string "We report on MOéMON news from all over\n"
	.string "SINNOH. Got a story for us?$"

Sinnoh_SolaceonTownPokemonNewsPress_Text_Person1Gift:
	.string "Thanks for listening! Here's a pretty\n"
	.string "scale a reader sent us.$"

Sinnoh_SolaceonTownPokemonNewsPress_Text_Person2:
	.string "I write the column about raising\n"
	.string "MOéMON. Readers send me so many\l"
	.string "questions!$"

@ --- end of SINNOH words ---
