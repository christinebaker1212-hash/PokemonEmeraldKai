Sinnoh_SandgemTownPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SandgemTownPokecenter1f_OnTransition
	.byte 0

Sinnoh_SandgemTownPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_SANDGEM_TOWN
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SandgemTownPokecenter1f_EventScript_Person1::
	goto SinnohP_sandgem_town_pokecenter_1f_2
	end

Sinnoh_SandgemTownPokecenter1f_EventScript_Person2::
	goto SinnohP_sandgem_town_pokecenter_1f_3
	end

Sinnoh_SandgemTownPokecenter1f_EventScript_Person3::
	goto SinnohP_sandgem_town_pokecenter_1f_4
	end

Sinnoh_SandgemTownPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

@ --- end of SINNOH words ---
