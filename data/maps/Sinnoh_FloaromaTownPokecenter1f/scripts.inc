Sinnoh_FloaromaTownPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_FloaromaTownPokecenter1f_OnTransition
	.byte 0

Sinnoh_FloaromaTownPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_FLOAROMA_TOWN
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FloaromaTownPokecenter1f_EventScript_Person1::
	goto SinnohP_floaroma_town_pokecenter_1f_4
	end

Sinnoh_FloaromaTownPokecenter1f_EventScript_Person2::
	goto SinnohP_floaroma_town_pokecenter_1f_3
	end

Sinnoh_FloaromaTownPokecenter1f_EventScript_Person3::
	setvar VAR_0x800B, 3
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_FloaromaTownPokecenter1f_EventScript_Person4::
	goto SinnohP_floaroma_town_pokecenter_1f_2
	end

@ --- end of SINNOH words ---
