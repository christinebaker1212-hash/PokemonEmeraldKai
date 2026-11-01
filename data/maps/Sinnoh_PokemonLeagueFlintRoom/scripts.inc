Sinnoh_PokemonLeagueFlintRoom_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonLeagueFlintRoom_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_ELITE_FOUR_FLINT, Sinnoh_PokemonLeagueFlintRoom_Text_Person1Intro, Sinnoh_PokemonLeagueFlintRoom_Text_Person1Defeat, Sinnoh_PokemonLeagueFlintRoom_EventScript_Person1_Defeated, NO_MUSIC
	msgbox Sinnoh_PokemonLeagueFlintRoom_Text_Person1After, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueFlintRoom_EventScript_Person1_Defeated::
	msgbox Sinnoh_PokemonLeagueFlintRoom_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_PokemonLeagueFlintRoom_Text_Person1Intro:
	.string "I'm FLINT! My FIRE MOéMON burn hot,\n"
	.string "and so do I!\p"
	.string "Are you ready to get scorched?$"

Sinnoh_PokemonLeagueFlintRoom_Text_Person1Defeat:
	.string "Burned out...$"

Sinnoh_PokemonLeagueFlintRoom_Text_Person1After:
	.string "That was hot! LUCIAN's next. He reads\n"
	.string "a lot. Don't let him bore you before\l"
	.string "the battle starts!$"

@ --- end of SINNOH words ---
