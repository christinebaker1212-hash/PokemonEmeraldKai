Sinnoh_SnowpointCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SnowpointCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_SnowpointCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_SNOWPOINT_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SnowpointCityPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_SnowpointCityPokecenter1f_EventScript_Person2::
	goto SinnohP_snowpoint_city_pokecenter_1f_2
	end

Sinnoh_SnowpointCityPokecenter1f_EventScript_Person3::
	goto SinnohP_snowpoint_city_pokecenter_1f_3
	end

Sinnoh_SnowpointCityPokecenter1f_EventScript_Person4::
	msgbox Sinnoh_SnowpointCityPokecenter1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_SnowpointCityPokecenter1f_EventScript_Person5::
	msgbox Sinnoh_SnowpointCityPokecenter1f_Text_Person5, MSGBOX_NPC
	end

Sinnoh_SnowpointCityPokecenter1f_Text_Person4:
	.string "PSYDUCK: Psy...\p"
	.string "It's shivering a little, but happy to\n"
	.string "be inside.$"

Sinnoh_SnowpointCityPokecenter1f_Text_Person5:
	.string "MAYLENE: I came north to train in the\n"
	.string "snow. Cold is good for focus!\p"
	.string "Achoo! ...Mostly good.$"

@ --- end of SINNOH words ---
