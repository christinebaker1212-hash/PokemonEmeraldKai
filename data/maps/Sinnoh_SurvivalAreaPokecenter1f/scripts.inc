Sinnoh_SurvivalAreaPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SurvivalAreaPokecenter1f_OnTransition
	.byte 0

Sinnoh_SurvivalAreaPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_SURVIVAL_AREA
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SurvivalAreaPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_SurvivalAreaPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_SurvivalAreaPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SurvivalAreaPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_SurvivalAreaPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SurvivalAreaPokecenter1f_EventScript_Person4::
	msgbox Sinnoh_SurvivalAreaPokecenter1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_SurvivalAreaPokecenter1f_Text_Person2:
	.string "The SURVIVAL AREA is where you prove\n"
	.string "you can keep going. Battle after\l"
	.string "battle!$"

Sinnoh_SurvivalAreaPokecenter1f_Text_Person3:
	.string "I've been training here for months. My\n"
	.string "MOéMON and I are tougher than ever.$"

Sinnoh_SurvivalAreaPokecenter1f_Text_Person4:
	.string "I can sense strong TRAINERS here.\n"
	.string "You're one of them.$"

@ --- end of SINNOH words ---
