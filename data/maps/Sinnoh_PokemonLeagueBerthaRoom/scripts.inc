Sinnoh_PokemonLeagueBerthaRoom_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonLeagueBerthaRoom_EventScript_Person1::
	trainerbattle_single TRAINER_SINNOH_ELITE_FOUR_BERTHA, Sinnoh_PokemonLeagueBerthaRoom_Text_Person1Intro, Sinnoh_PokemonLeagueBerthaRoom_Text_Person1Defeat, Sinnoh_PokemonLeagueBerthaRoom_EventScript_Person1_Defeated, NO_MUSIC
	msgbox Sinnoh_PokemonLeagueBerthaRoom_Text_Person1After, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueBerthaRoom_EventScript_Person1_Defeated::
	msgbox Sinnoh_PokemonLeagueBerthaRoom_Text_Person1After, MSGBOX_DEFAULT
	release
	end

Sinnoh_PokemonLeagueBerthaRoom_Text_Person1Intro:
	.string "Well, well. Another young one made it\n"
	.string "past AARON. I'm BERTHA.\p"
	.string "My GROUND MOéMON have been with me a\n"
	.string "long, long time. Let's see if you can\l"
	.string "move a mountain!$"

Sinnoh_PokemonLeagueBerthaRoom_Text_Person1Defeat:
	.string "Oh my. The mountain moved.$"

Sinnoh_PokemonLeagueBerthaRoom_Text_Person1After:
	.string "You remind me of myself, long ago. Go\n"
	.string "on, dear. FLINT will keep you warm.$"

@ --- end of SINNOH words ---
