Sinnoh_SnowpointCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SnowpointCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_SnowpointCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_SNOWPOINT_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SnowpointCityPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_SnowpointCityPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_SnowpointCityPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SnowpointCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_SnowpointCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SnowpointCityPokecenter1f_EventScript_Person4::
	msgbox Sinnoh_SnowpointCityPokecenter1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_SnowpointCityPokecenter1f_Text_Person2:
	.string "It's so warm in here! I could stay all\n"
	.string "day.$"

Sinnoh_SnowpointCityPokecenter1f_Text_Person3:
	.string "CANDICE trains outside in the snow.\n"
	.string "She says it builds focus.$"

Sinnoh_SnowpointCityPokecenter1f_Text_Person4:
	.string "PSYDUCK: Psy...\p"
	.string "It's shivering a little, but happy to\n"
	.string "be inside.$"

@ --- end of SINNOH words ---
