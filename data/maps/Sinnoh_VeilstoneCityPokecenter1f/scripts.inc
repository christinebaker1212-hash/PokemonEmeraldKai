Sinnoh_VeilstoneCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_VeilstoneCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_VeilstoneCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_VEILSTONE_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_VeilstoneCityPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_VeilstoneCityPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_VeilstoneCityPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_VeilstoneCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_VeilstoneCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_VeilstoneCityPokecenter1f_Text_Person2:
	.string "The department store is the best\n"
	.string "thing in VEILSTONE. Well, after the\l"
	.string "MOéMON CENTER, darling.$"

Sinnoh_VeilstoneCityPokecenter1f_Text_Person3:
	.string "MAYLENE trains every day at the GYM. I\n"
	.string "watch through the window sometimes!$"

@ --- end of SINNOH words ---
