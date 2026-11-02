Sinnoh_OreburghCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_OreburghCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_OreburghCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_OREBURGH_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_OreburghCityPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_OreburghCityPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person2::
	goto SinnohP_oreburgh_city_pokecenter_1f_3
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person3::
	goto SinnohP_oreburgh_city_pokecenter_1f_2
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person5::
	goto SinnohP_oreburgh_city_pokecenter_1f_5
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person8::
	goto SinnohP_oreburgh_city_pokecenter_1f_6
	end

Sinnoh_OreburghCityPokecenter1f_EventScript_Person9::
	goto SinnohP_oreburgh_city_pokecenter_1f_7
	end

Sinnoh_OreburghCityPokecenter1f_Text_Person1:
	.string "I can sense... that your MOéMON are\n"
	.string "tired. That's why you're here, right?$"

@ --- end of SINNOH words ---
