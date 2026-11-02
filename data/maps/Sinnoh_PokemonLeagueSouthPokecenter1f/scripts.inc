Sinnoh_PokemonLeagueSouthPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_PokemonLeagueSouthPokecenter1f_OnTransition
	.byte 0

Sinnoh_PokemonLeagueSouthPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_POKEMON_LEAGUE_2
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonLeagueSouthPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_PokemonLeagueSouthPokecenter1f_EventScript_Person2::
	goto SinnohP_pokemon_league_south_pokecenter_1f_5
	end

Sinnoh_PokemonLeagueSouthPokecenter1f_EventScript_Person3::
	goto SinnohP_pokemon_league_south_pokecenter_1f_6
	end

@ --- end of SINNOH words ---
