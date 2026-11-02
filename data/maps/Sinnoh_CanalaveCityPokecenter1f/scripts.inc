Sinnoh_CanalaveCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_CanalaveCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_CanalaveCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_CANALAVE_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCityPokecenter1f_EventScript_Person1::
	goto SinnohP_canalave_city_pokecenter_1f_3
	end

Sinnoh_CanalaveCityPokecenter1f_EventScript_Person2::
	setvar VAR_0x800B, 2
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_CanalaveCityPokecenter1f_EventScript_Person3::
	goto SinnohP_canalave_city_pokecenter_1f_2
	end

Sinnoh_CanalaveCityPokecenter1f_EventScript_Person6::
	goto SinnohP_canalave_city_pokecenter_1f_5
	end

Sinnoh_CanalaveCityPokecenter1f_EventScript_Person7::
	goto SinnohP_canalave_city_pokecenter_1f_4
	end

@ --- end of SINNOH words ---
