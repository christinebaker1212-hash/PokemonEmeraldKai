Sinnoh_FloaromaTownPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_FloaromaTownPokecenter1f_OnTransition
	.byte 0

Sinnoh_FloaromaTownPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_FLOAROMA_TOWN
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FloaromaTownPokecenter1f_EventScript_Person3::
	setvar VAR_0x800B, 3
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

@ --- end of SINNOH words ---
