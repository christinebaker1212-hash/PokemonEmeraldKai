Sinnoh_EternaCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_EternaCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_EternaCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_ETERNA_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCityPokecenter1f_EventScript_Person1::
	goto SinnohP_eterna_city_pokecenter_1f_3
	end

Sinnoh_EternaCityPokecenter1f_EventScript_Person2::
	goto SinnohP_eterna_city_pokecenter_1f_2
	end

Sinnoh_EternaCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_EternaCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_EternaCityPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_EternaCityPokecenter1f_EventScript_Person5::
	goto SinnohP_eterna_city_pokecenter_1f_5
	end

Sinnoh_EternaCityPokecenter1f_Text_Person3:
	.string "BUNEARY bounces on the spot, ears\n"
	.string "flopping.$"

@ --- end of SINNOH words ---
