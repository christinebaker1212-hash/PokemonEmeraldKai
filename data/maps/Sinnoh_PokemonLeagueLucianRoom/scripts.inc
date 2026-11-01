Sinnoh_PokemonLeagueLucianRoom_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonLeagueLucianRoom_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_ELITE_FOUR_LUCIAN, Sinnoh_PokemonLeagueLucianRoom_Text_Person1Intro, Sinnoh_PokemonLeagueLucianRoom_Text_Person1Defeat, Sinnoh_PokemonLeagueLucianRoom_EventScript_Person1_Defeated, NO_MUSIC
	msgbox Sinnoh_PokemonLeagueLucianRoom_Text_Person1After, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueLucianRoom_EventScript_Person1_Defeated::
	msgbox Sinnoh_PokemonLeagueLucianRoom_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_PokemonLeagueLucianRoom_Text_Person1Intro:
	.string "I am LUCIAN. I was reading, but a\n"
	.string "battle with you will be a better story.\p"
	.string "Let's see how this chapter ends.$"

Sinnoh_PokemonLeagueLucianRoom_Text_Person1Defeat:
	.string "An unexpected ending.$"

Sinnoh_PokemonLeagueLucianRoom_Text_Person1After:
	.string "The CHAMPION is beyond this room. She\n"
	.string "is the strongest TRAINER I know. Go\l"
	.string "and write your own ending.$"

@ --- end of SINNOH words ---
