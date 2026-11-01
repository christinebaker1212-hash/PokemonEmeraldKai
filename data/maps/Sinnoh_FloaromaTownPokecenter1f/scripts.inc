Sinnoh_FloaromaTownPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_FloaromaTownPokecenter1f_OnTransition
	.byte 0

Sinnoh_FloaromaTownPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_FLOAROMA_TOWN
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_FloaromaTownPokecenter1f_EventScript_Person1::
	msgbox Sinnoh_FloaromaTownPokecenter1f_Text_Person1, MSGBOX_NPC
	end

Sinnoh_FloaromaTownPokecenter1f_EventScript_Person2::
	msgbox Sinnoh_FloaromaTownPokecenter1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_FloaromaTownPokecenter1f_EventScript_Person3::
	setvar VAR_0x800B, 3
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_FloaromaTownPokecenter1f_EventScript_Person4::
	msgbox Sinnoh_FloaromaTownPokecenter1f_Text_Person4, MSGBOX_NPC
	end

Sinnoh_FloaromaTownPokecenter1f_Text_Person1:
	.string "The flowers outside smell so good! I\n"
	.string "want to bring some to the nurse.$"

Sinnoh_FloaromaTownPokecenter1f_Text_Person2:
	.string "I trained in the meadow all day. My\n"
	.string "MOéMON smells like flowers now!$"

Sinnoh_FloaromaTownPokecenter1f_Text_Person4:
	.string "There's a windmill plant south of the\n"
	.string "meadow. It powers this whole town.$"

@ --- end of SINNOH words ---
