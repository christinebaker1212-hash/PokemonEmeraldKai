Sinnoh_PastoriaCityPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_PastoriaCityPokecenter1f_OnTransition
	.byte 0

Sinnoh_PastoriaCityPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_PASTORIA_CITY
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_PastoriaCityPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_PastoriaCityPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_PastoriaCityPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_PastoriaCityPokecenter1f_EventScript_Person3::
	msgbox Sinnoh_PastoriaCityPokecenter1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_PastoriaCityPokecenter1f_Text_Person2:
	.string "I train in the marsh. The mud slows you\n"
	.string "down, so your legs get stronger!$"

Sinnoh_PastoriaCityPokecenter1f_Text_Person3:
	.string "CRASHER WAKE waved at me in the\n"
	.string "street once. He's so loud and nice!$"

@ --- end of SINNOH words ---
