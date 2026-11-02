Sinnoh_CelesticTownPokecenter1f_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Sinnoh_CelesticTownPokecenter1f_OnTransition
	.byte 0

Sinnoh_CelesticTownPokecenter1f_OnTransition:
	setrespawn HEAL_LOCATION_SINNOH_CELESTIC_TOWN
	end

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CelesticTownPokecenter1f_EventScript_Person1::
	setvar VAR_0x800B, 1
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

Sinnoh_CelesticTownPokecenter1f_EventScript_Person2::
	goto SinnohP_celestic_town_pokecenter_1f_2
	end

Sinnoh_CelesticTownPokecenter1f_EventScript_Person3::
	goto SinnohP_celestic_town_pokecenter_1f_3
	end

Sinnoh_CelesticTownPokecenter1f_EventScript_Person4::
	lock
	faceplayer
	goto_if_set 0x3DA5, Sinnoh_CelesticTownPokecenter1f_EventScript_Person4_Given
	msgbox Sinnoh_CelesticTownPokecenter1f_Text_Person4, MSGBOX_DEFAULT
	msgbox Sinnoh_CelesticTownPokecenter1f_Text_Person4Gift, MSGBOX_DEFAULT
	giveitem ITEM_GREAT_BALL, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DA5
	release
	end

Sinnoh_CelesticTownPokecenter1f_EventScript_Person4_Given::
	msgbox Sinnoh_CelesticTownPokecenter1f_Text_Person4, MSGBOX_DEFAULT
	release
	end

Sinnoh_CelesticTownPokecenter1f_Text_Person4:
	.string "The shrine in the cave is said to honor\n"
	.string "a MOéMON of time and space.$"

Sinnoh_CelesticTownPokecenter1f_Text_Person4Gift:
	.string "Visitors to the shrine should have one\n"
	.string "of these. Take it!$"

@ --- end of SINNOH words ---
