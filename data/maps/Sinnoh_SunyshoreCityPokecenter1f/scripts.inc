Sinnoh_SunyshoreCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_SunyshoreCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_SunyshoreCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_SUNYSHORE_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SunyshoreCityPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_SunyshoreCityPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_SunyshoreCityPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_SunyshoreCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_SunyshoreCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_SunyshoreCityPokecenter1f_EventScript_Person4::
	msgbox Sinnoh_SunyshoreCityPokecenter1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_SunyshoreCityPokecenter1f_Text_Person2:
	.string "I've lived by the sea all my life. The\n"
	.string "waves here are the gentlest in\l"
	.string "SINNOH.$"

Sinnoh_SunyshoreCityPokecenter1f_Text_Person3:
	.string "VOLKNER used to battle everyone who\n"
	.string "came. Now he just looks bored. Maybe\l"
	.string "you can help!$"

Sinnoh_SunyshoreCityPokecenter1f_Text_Person4:
	.string "The solar walkways power the whole\n"
	.string "city. Even this MOéMON CENTER!$"

@ --- end of SINNOH words ---
