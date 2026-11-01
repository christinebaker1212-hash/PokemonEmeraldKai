Sinnoh_PokemonLeagueAaronRoom_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonLeagueAaronRoom_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_ELITE_FOUR_AARON, Sinnoh_PokemonLeagueAaronRoom_Text_Person1Intro, Sinnoh_PokemonLeagueAaronRoom_Text_Person1Defeat, Sinnoh_PokemonLeagueAaronRoom_EventScript_Person1_Defeated, NO_MUSIC
	msgbox Sinnoh_PokemonLeagueAaronRoom_Text_Person1After, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueAaronRoom_EventScript_Person1_Defeated::
	msgbox Sinnoh_PokemonLeagueAaronRoom_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_PokemonLeagueAaronRoom_Text_Person1Intro:
	.string "I'm AARON of the ELITE FOUR. BUG\n"
	.string "MOéMON are small, and people think\l"
	.string "that makes them weak.\p"
	.string "They're wrong. Let me show you how\n"
	.string "wrong!$"

Sinnoh_PokemonLeagueAaronRoom_Text_Person1Defeat:
	.string "My bugs fought beautifully...$"

Sinnoh_PokemonLeagueAaronRoom_Text_Person1After:
	.string "Go on to the next room. BERTHA is\n"
	.string "waiting. She's tougher than she looks.$"

@ --- end of SINNOH words ---
