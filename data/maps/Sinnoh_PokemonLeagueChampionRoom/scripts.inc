Sinnoh_PokemonLeagueChampionRoom_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonLeagueChampionRoom_EventScript_Person1::
	goto_if_unset 0x391B, Sinnoh_PokemonLeagueChampionRoom_EventScript_Person1_NotYet
	goto_if_unset 0x391C, Sinnoh_PokemonLeagueChampionRoom_EventScript_Person1_NotYet
	goto_if_unset 0x391D, Sinnoh_PokemonLeagueChampionRoom_EventScript_Person1_NotYet
	goto_if_unset 0x391E, Sinnoh_PokemonLeagueChampionRoom_EventScript_Person1_NotYet
	trainerbattle_single TRAINER_SINNOH_CHAMPION_CYNTHIA, Sinnoh_PokemonLeagueChampionRoom_Text_Person1Intro, Sinnoh_PokemonLeagueChampionRoom_Text_Person1Defeat, Sinnoh_PokemonLeagueChampionRoom_EventScript_Person1_Defeated, NO_MUSIC
	msgbox Sinnoh_PokemonLeagueChampionRoom_Text_Person1Again, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueChampionRoom_EventScript_Person1_NotYet::
	msgbox Sinnoh_PokemonLeagueChampionRoom_Text_Person1NotYet, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueChampionRoom_EventScript_Person1_Defeated::
	msgbox Sinnoh_PokemonLeagueChampionRoom_Text_Person1After, MSGBOX_DEFAULT
	closemessage
	call Quetzal_EventScript_835F6E4
	setrespawn HEAL_LOCATION_SINNOH_POKEMON_LEAGUE
	fadescreenspeed 1, 24
	special GameClear
	waitstate
	releaseall
	end

Sinnoh_PokemonLeagueChampionRoom_Text_Person1Intro:
	.string "Welcome. I'm CYNTHIA, the CHAMPION of\n"
	.string "SINNOH.\p"
	.string "You beat all four of the ELITE FOUR. I\n"
	.string "can tell your MOéMON trust you\l"
	.string "completely.\p"
	.string "I won't hold anything back. Neither\n"
	.string "should you!$"

Sinnoh_PokemonLeagueChampionRoom_Text_Person1Defeat:
	.string "Brilliant... truly brilliant.$"

Sinnoh_PokemonLeagueChampionRoom_Text_Person1After:
	.string "You are SINNOH's new CHAMPION. Your\n"
	.string "MOéMON fought with everything they\l"
	.string "had, and so did you.\p"
	.string "Come. The HALL OF FAME is waiting for\n"
	.string "your team.$"

Sinnoh_PokemonLeagueChampionRoom_Text_Person1NotYet:
	.string "Welcome to the CHAMPION's room. Come\n"
	.string "back once you've beaten all four of\l"
	.string "the ELITE FOUR. I'll be waiting.$"

Sinnoh_PokemonLeagueChampionRoom_Text_Person1Again:
	.string "That battle was one of my best. Come\n"
	.string "back and challenge me any time.$"

@ --- end of SINNOH words ---
