Sinnoh_SnowpointCity_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SnowpointCity_EventScript_Person1::
	goto SinnohP_snowpoint_city_5
	end

Sinnoh_SnowpointCity_EventScript_Person4::
	goto SinnohP_snowpoint_city_8
	end

Sinnoh_SnowpointCity_EventScript_Person5::
	goto SinnohP_snowpoint_city_6
	end

Sinnoh_SnowpointCity_EventScript_Person6::
	lock
	faceplayer
	msgbox Sinnoh_SnowpointCity_Text_Person6Ask0, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Sinnoh_SnowpointCity_EventScript_Person6_Sail0
	msgbox Sinnoh_SnowpointCity_Text_Person6Bye, MSGBOX_DEFAULT
	release
	end

Sinnoh_SnowpointCity_EventScript_Person6_Sail0::
	msgbox Sinnoh_SnowpointCity_Text_Person6Off, MSGBOX_DEFAULT
	closemessage
	warp MAP_SINNOH_FIGHT_AREA, 26, 27
	waitstate
	releaseall
	end

Sinnoh_SnowpointCity_EventScript_Person7::
	goto SinnohP_snowpoint_city_2
	end

Sinnoh_SnowpointCity_EventScript_Person8::
	msgbox Sinnoh_SnowpointCity_Text_Person8, MSGBOX_NPC
	end

Sinnoh_SnowpointCity_EventScript_Person10::
	goto SinnohP_snowpoint_city_7
	end

Sinnoh_SnowpointCity_EventScript_Sign2::
	goto SinnohP_snowpoint_city_10
	end

Sinnoh_SnowpointCity_EventScript_Sign3::
	goto SinnohP_snowpoint_city_11
	end

Sinnoh_SnowpointCity_Text_Person6Ask0:
	.string "Shall I take you to the BATTLE ZONE?$"

Sinnoh_SnowpointCity_Text_Person6Bye:
	.string "Come back any time. The sea's always\n"
	.string "here.$"

Sinnoh_SnowpointCity_Text_Person6Off:
	.string "All aboard! Hold on tight!$"

Sinnoh_SnowpointCity_Text_Person8:
	.string "CANDICE: A visitor in SNOWPOINT! I'm\n"
	.string "CANDICE, the GYM LEADER here.\p"
	.string "Snow's no reason to stay inside! Come\n"
	.string "show me what you've got at the GYM!$"

@ --- end of SINNOH words ---
