Sinnoh_PokemonLeagueNorthPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_PokemonLeagueNorthPokecenter1f_OnTransition
	.byte 0

Sinnoh_PokemonLeagueNorthPokecenter1f_OnTransition:
	special Sinnoh_LegendsRefresh
	setrespawn HEAL_LOCATION_SINNOH_POKEMON_LEAGUE
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_PokemonLeagueNorthPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person2::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_PokemonLeague
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person5::
	goto SinnohP_pokemon_league_north_pokecenter_1f_6
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_EventScript_Person7::
	goto SinnohP_pokemon_league_north_pokecenter_1f_7
	end

Sinnoh_PokemonLeagueNorthPokecenter1f_Text_Person1:
	.string "Beyond this door are the ELITE FOUR.\n"
	.string "Once you go in, there's no coming back\l"
	.string "until it's over.\p"
	.string "Make sure your team is ready!$"

@ --- end of SINNOH words ---
