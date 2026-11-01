Sinnoh_HearthomeCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_HearthomeCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_HearthomeCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_HEARTHOME_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_HearthomeCityPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_HearthomeCityPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_HearthomeCityPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_HearthomeCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_HearthomeCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_HearthomeCityPokecenter1f_EventScript_Person4::
	msgbox Sinnoh_HearthomeCityPokecenter1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_HearthomeCityPokecenter1f_Text_Person2:
	.string "HEARTHOME is the friendliest city in\n"
	.string "SINNOH. Even the nurse says so!$"

Sinnoh_HearthomeCityPokecenter1f_Text_Person3:
	.string "I'm here to show off my MOéMON at the\n"
	.string "CONTEST HALL. Darling, isn't it\l"
	.string "beautiful?$"

Sinnoh_HearthomeCityPokecenter1f_Text_Person4:
	.string "I'm going to play a song for the FAN\n"
	.string "CLUB. They love it when I play!$"

@ --- end of SINNOH words ---
