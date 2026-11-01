Sinnoh_SolaceonTownPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SolaceonTownPokecenter1f_OnTransition
	.byte 0

Sinnoh_SolaceonTownPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_SOLACEON_TOWN
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SolaceonTownPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_SolaceonTownPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_SolaceonTownPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SolaceonTownPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_SolaceonTownPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SolaceonTownPokecenter1f_EventScript_Person6::
	msgbox Sinnoh_SolaceonTownPokecenter1f_Text_Person6, MSGBOX_NPC
	end

Sinnoh_SolaceonTownPokecenter1f_Text_Person2:
	.string "SOLACEON is the quietest town in\n"
	.string "SINNOH. That's why I retired here.$"

Sinnoh_SolaceonTownPokecenter1f_Text_Person3:
	.string "The DAY CARE is just east. They take\n"
	.string "great care of MOéMON!$"

Sinnoh_SolaceonTownPokecenter1f_Text_Person6:
	.string "I've been studying the ruins for\n"
	.string "years. Every room has new letters.$"

@ --- end of SINNOH words ---
