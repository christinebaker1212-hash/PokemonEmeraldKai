Sinnoh_CanalaveCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_CanalaveCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_CanalaveCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_CANALAVE_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCityPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_CanalaveCityPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_CanalaveCityPokecenter1f_EventScript_Person2::
	setvar VAR_0x800B, 2
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_CanalaveCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_CanalaveCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_CanalaveCityPokecenter1f_EventScript_Person6::
	msgbox Sinnoh_CanalaveCityPokecenter1f_Text_Person6, MSGBOX_NPC
	end

Sinnoh_CanalaveCityPokecenter1f_EventScript_Person7::
	msgbox Sinnoh_CanalaveCityPokecenter1f_Text_Person7, MSGBOX_NPC
	end

Sinnoh_CanalaveCityPokecenter1f_Text_Person1:
	.string "Travelers from the boats always stop\n"
	.string "here first. The nurse has seen it all.$"

Sinnoh_CanalaveCityPokecenter1f_Text_Person3:
	.string "My MOéMON got seasick on the ferry. A\n"
	.string "rest here fixed it right up!$"

Sinnoh_CanalaveCityPokecenter1f_Text_Person6:
	.string "I'm going to IRON ISLAND to catch\n"
	.string "STEEL MOéMON. Wish me luck!$"

Sinnoh_CanalaveCityPokecenter1f_Text_Person7:
	.string "I study the old books in the library.\n"
	.string "The ones on the third floor are the\l"
	.string "oldest.$"

@ --- end of SINNOH words ---
