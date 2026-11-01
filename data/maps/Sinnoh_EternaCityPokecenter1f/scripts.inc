Sinnoh_EternaCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_EternaCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_EternaCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_ETERNA_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCityPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_EternaCityPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_EternaCityPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_EternaCityPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_EternaCityPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_EternaCityPokecenter1f_EventScript_Person5::
	msgbox Sinnoh_EternaCityPokecenter1f_Text_Person5, MSGBOX_NPC
	end

Sinnoh_EternaCityPokecenter1f_Text_Person1:
	.string "The statue in town is of a MOéMON from\n"
	.string "a long-ago legend. Nobody knows which\l"
	.string "one for sure.$"

Sinnoh_EternaCityPokecenter1f_Text_Person2:
	.string "I'm going to challenge GARDENIA! My\n"
	.string "FIGHTING MOéMON... wait, is that a good\l"
	.string "idea?$"

Sinnoh_EternaCityPokecenter1f_Text_Person5:
	.string "ETERNA's flowers are lovely, but the\n"
	.string "MOéMON CENTER's flowers are my\l"
	.string "favorite.$"

@ --- end of SINNOH words ---
