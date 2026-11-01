Sinnoh_PokemonLeagueNorthPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_PokemonLeagueNorthPokecenter1f_OnTransition
	.byte 0

Sinnoh_PokemonLeagueNorthPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_POKEMON_LEAGUE
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_PokemonLeagueNorthPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person5::
	msgbox Sinnoh_PokemonLeagueNorthPokecenter1f_Text_Person5, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person7::
	msgbox Sinnoh_PokemonLeagueNorthPokecenter1f_Text_Person7, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_Text_Person1:
	.string "Beyond this door are the ELITE FOUR.\n"
	.string "Once you go in, there's no coming back\l"
	.string "until it's over.\p"
	.string "Make sure your team is ready!$"

Sinnoh_PokemonLeagueNorthPokecenter1f_Text_Person5:
	.string "I've challenged the ELITE FOUR five\n"
	.string "times. I'm getting closer every time!$"

Sinnoh_PokemonLeagueNorthPokecenter1f_Text_Person7:
	.string "I play a song for every TRAINER who\n"
	.string "goes in. Good luck!$"

@ --- end of SINNOH words ---
