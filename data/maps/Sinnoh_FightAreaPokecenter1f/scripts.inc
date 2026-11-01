Sinnoh_FightAreaPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_FightAreaPokecenter1f_OnTransition
	.byte 0

Sinnoh_FightAreaPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_FIGHT_AREA
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FightAreaPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_FightAreaPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_FightAreaPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_FightAreaPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_FightAreaPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_FightAreaPokecenter1f_EventScript_Person4::
	msgbox Sinnoh_FightAreaPokecenter1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_FightAreaPokecenter1f_Text_Person2:
	.string "I sing to keep my MOéMON's spirits up.\n"
	.string "They battle better when they're\l"
	.string "happy!$"

Sinnoh_FightAreaPokecenter1f_Text_Person3:
	.string "Welcome! Everyone here is a TRAINER.\n"
	.string "Even the cooks!$"

Sinnoh_FightAreaPokecenter1f_Text_Person4:
	.string "The FRONTIER is the toughest place in\n"
	.string "SINNOH. I'm training here until I'm\l"
	.string "ready.$"

@ --- end of SINNOH words ---
