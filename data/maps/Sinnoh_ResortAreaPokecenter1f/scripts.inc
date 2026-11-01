Sinnoh_ResortAreaPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_ResortAreaPokecenter1f_OnTransition
	.byte 0

Sinnoh_ResortAreaPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_RESORT_AREA
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_ResortAreaPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_ResortAreaPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_ResortAreaPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_ResortAreaPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_ResortAreaPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_ResortAreaPokecenter1f_Text_Person2:
	.string "I'm on vacation with my family. They're\n"
	.string "at the beach. I'm at the MOéMON\l"
	.string "CENTER.$"

Sinnoh_ResortAreaPokecenter1f_Text_Person3:
	.string "I'm here to relax! No singing, no\n"
	.string "dancing, just sunshine.$"

@ --- end of SINNOH words ---
