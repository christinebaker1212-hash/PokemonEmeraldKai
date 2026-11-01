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
	msgbox Sinnoh_PokemonLeagueSouthPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueSouthPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_PokemonLeagueSouthPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PokemonLeagueSouthPokecenter1f_Text_Person2:
	.string "VICTORY ROAD is no joke. Rest up here\n"
	.string "before you go in.$"

Sinnoh_PokemonLeagueSouthPokecenter1f_Text_Person3:
	.string "I made it through VICTORY ROAD once.\n"
	.string "My MOéMON were so tired they slept for\l"
	.string "a day.$"

@ --- end of SINNOH words ---
