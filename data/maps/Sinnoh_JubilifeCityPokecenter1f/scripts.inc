Sinnoh_JubilifeCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_JubilifeCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_JubilifeCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_JUBILIFE_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_JubilifeCityPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_JubilifeCityPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_JubilifeCityPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_JubilifeCityPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_JubilifeCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_JubilifeCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_JubilifeCityPokecenter1f_EventScript_Person4::
	setvar VAR_0x800B, 4
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_JubilifeCityPokecenter1f_Text_Person1:
	.string "The nurse here remembers every\n"
	.string "TRAINER's name. How does she do it?$"

Sinnoh_JubilifeCityPokecenter1f_Text_Person2:
	.string "I came here to watch TRAINERS heal\n"
	.string "up. It's how I learn which MOéMON are\l"
	.string "strong!$"

Sinnoh_JubilifeCityPokecenter1f_Text_Person3:
	.string "My MOéMON got tired from playing tag.\n"
	.string "Now it's resting, and I'm waiting!$"

@ --- end of SINNOH words ---
